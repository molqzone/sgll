#!/usr/bin/env python3
"""Run SGLL header, register-image, and C906 code-generation regressions."""
import argparse
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile


def run(command, source=None):
    result = subprocess.run(command, input=source, text=True, capture_output=True, timeout=60)
    if result.returncode:
        raise RuntimeError(" ".join(map(str, command)) + "\n" + result.stdout + result.stderr)
    return result.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cross-prefix", default=os.environ.get("SG200X_CROSS_COMPILE", "riscv-none-elf-"))
    parser.add_argument("--core-include", type=Path, required=True, help="SDK directory containing core_rv64.h")
    parser.add_argument("--cc", default=os.environ.get("CC", "gcc"))
    parser.add_argument("--cxx", default=os.environ.get("CXX", "g++"))
    parser.add_argument("--linux-cc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--linux-sysroot", type=Path, help="Optional RISC-V libc tree with include/ and lib/")
    parser.add_argument("--qemu", default="qemu-riscv64")
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    tests = root / "tests"
    common = ["-Wall", "-Wextra", "-Werror", "-I", str(root)]
    # Match the BSP's C906 selection and explicit base ISA. Cache instructions
    # remain raw words and trap under QEMU's ordinary RV64 userspace CPU.
    target_flags = ["-mcpu=thead-c906", "-march=rv64imafdc_zicsr", "-mabi=lp64d"]
    arch = [*target_flags, "-isystem", str(args.core_include.resolve())]
    linux_flags = []
    if args.linux_sysroot:
        sysroot = args.linux_sysroot.resolve()
        linux_flags = ["--sysroot=" + str(sysroot), "-isystem", str(sysroot / "include"),
                       "-B" + str(sysroot / "lib") + "/", "-L" + str(sysroot / "lib")]
    modes = [("host C23", args.cc, "c", "c23", []),
             ("host C++20", args.cxx, "c++", "c++20", []),
             ("RV64 C23", args.cross_prefix + "gcc", "c", "c23", arch),
             ("RV64 C++20", args.cross_prefix + "g++", "c++", "c++20", arch)]
    headers = ["sgll.h"] + ["inc/" + p.name for p in sorted((root / "inc").glob("*.h"))]
    for label, compiler, language, standard, flags in modes:
        for header in headers:
            run([compiler, *common, *flags, "-std=" + standard, "-fsyntax-only", "-x", language, "-"],
                '#include "' + header + '"\n')
        print(f"{label}: {len(headers)} standalone headers passed", flush=True)

    with tempfile.TemporaryDirectory(prefix="sgll-tests-") as directory:
        build = Path(directory)
        host = build / "host"
        run(["cmake", "-S", str(tests), "-B", str(host), "-G", "Ninja",
             "-DCMAKE_C_COMPILER=" + args.cc, "-DCMAKE_CXX_COMPILER=" + args.cxx,
             "-DCMAKE_C_FLAGS=-O2 -Wall -Wextra -Werror",
             "-DCMAKE_CXX_FLAGS=-O2 -Wall -Wextra -Werror"])
        run(["cmake", "--build", str(host)])
        print(run(["ctest", "--test-dir", str(host), "--output-on-failure"]).strip(), flush=True)
        for optimization in (["-O2"], ["-O3", "-funroll-loops"]):
            cross = build / ("cross_" + optimization[0][1:])
            run(["cmake", "-S", str(root), "-B", str(cross), "-G", "Ninja",
                 "-DCMAKE_SYSTEM_NAME=Generic", "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
                 "-DCMAKE_C_COMPILER=" + args.cross_prefix + "gcc",
                 "-DCMAKE_C_FLAGS=" + shlex.join([*target_flags, *optimization, "-Wall", "-Wextra", "-Werror"]),
                 "-DSGLL_CORE_INCLUDE_DIR=" + str(args.core_include.resolve())])
            run(["cmake", "--build", str(cross)])
            library = str(cross / "libsgll.a")
            symbols = run([args.cross_prefix + "nm", "--defined-only", library])
            for name in ("sgll_dma_lli_build", "sgll_dma_channel_configure", "sgll_dma_channel_wait_disabled",
                         "sgll_dma_channels_wait_disabled", "sgll_csr_delay_nops",
                         "sgll_rcc_reset_loc_get", "sgll_rcc_clkgen_reg", "sgll_csr_dcache_clean_range",
                         "sgll_rcc_rtc_saradc_enable", "sgll_spi_wait_idle",
                         "sgll_i2c_timing_calculate", "sgll_i2c_init_with_timing", "sgll_i2c_enable_wait",
                         "sgll_adc_start", "sgll_adc_wait_idle", "sgll_pwm_period_set",
                         "sgll_pwm_update", "sgll_pwm_start", "sgll_pwm_stop",
                         "sgll_wdt_start", "sgll_wdt_reset_route_set",
                         "sgll_csr_dcache_invalidate_range", "sgll_csr_dcache_clean_invalidate_range",
                         "sgll_mbox_config_valid", "sgll_mbox_rx_acquire", "sgll_mbox_rx_payload",
                         "sgll_mbox_rx_release", "sgll_mbox_tx_acquire", "sgll_mbox_tx_publish",
                         *(f"sgll_{module}_{operation}" for module in ("spi", "i2c", "uart", "dma")
                           for operation in ("struct_init", "init", "deinit")),
                         *(f"sgll_{module}_{operation}" for module in ("adc", "pwm", "wdt")
                           for operation in ("struct_init", "init"))):
                if len(re.findall(r"\bT " + name + r"$", symbols, re.M)) != 1:
                    raise RuntimeError("Expected one compiled C definition for " + name)
            print(f"CMake RV64 {' '.join(optimization)}: static library and C symbols passed", flush=True)
            for label, compiler, language, standard, flags in modes[2:]:
                probe = str(build / "cache_probe.o")
                run([compiler, *common, *flags, "-std=" + standard, *optimization,
                     "-x", language, "-c", str(tests / "cache_probe.c"), "-o", probe])
                disassembly = run([args.cross_prefix + "objdump", "-d", probe])
                fence = re.search(r"<sgll_test_instruction_fence>:(.*?)(?:\n\s*\n|\Z)", disassembly, re.S)
                if fence is None or "0000100f" not in fence.group(1):
                    raise RuntimeError("Instruction fence must emit the fence.i opcode 0x0000100f")
                io_fence = re.search(r"<sgll_test_io_fence>:(.*?)(?:\n\s*\n|\Z)", disassembly, re.S)
                if io_fence is None or "0ff0000f" not in io_fence.group(1):
                    raise RuntimeError("I/O barrier must emit fence iorw, iorw (0x0ff0000f)")
                executable = str(build / "cache_linux")
                run([args.linux_cc, *linux_flags, "-std=c23", "-O2", "-Wall", "-Wextra", "-Werror", "-static",
                     str(tests / "cache_linux.c"), probe, library, "-o", executable])
                output = run([args.qemu, executable]).strip()
                print(f"{label} {' '.join(optimization)}: {output}; fence.i opcode passed", flush=True)


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.TimeoutExpired) as error:
        raise SystemExit(str(error)) from error
