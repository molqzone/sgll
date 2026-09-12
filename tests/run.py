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


CORE_PROBE_SYMBOLS = ("sgll_test_core_mstatus_read", "sgll_test_core_mstatus_write",
                      "sgll_test_core_mie_read", "sgll_test_core_mie_write", "sgll_test_core_mie_set",
                      "sgll_test_core_mie_clear", "sgll_test_core_irq_enable", "sgll_test_core_irq_disable",
                      "sgll_test_core_time_read")

# objdump prints the immediate forms (csrsi/csrci) and keeps csrr/csrw for
# register operands, so the CSR operand is picked by elimination rather than by
# position: whatever is neither a register nor a number names the CSR.
CSR_MNEMONICS = ("csrr", "csrw", "csrs", "csrc", "csrsi", "csrci", "csrrsi", "csrrci")
REGISTER_NAMES = re.compile(r"^(x\d+|a\d+|t\d+|s\d+|zero|ra|sp|gp|tp|fp)$")
NUMBER = re.compile(r"^(-?\d+|0x[0-9a-f]+)$")


def csr_operand(operands):
    """Return the CSR name in an instruction operand list, or None."""
    for token in (part.strip() for part in operands.split(",")):
        if token and not REGISTER_NAMES.match(token) and not NUMBER.match(token):
            return token
    return None


def core_operations(disassembly):
    """Return per-symbol (mnemonic list, CSR name list) for the core probe."""
    operations = {}
    for symbol in CORE_PROBE_SYMBOLS:
        body = re.search(r"<" + symbol + r">:(.*?)(?:\n\s*\n|\Z)", disassembly, re.S)
        if body is None:
            raise RuntimeError("Core probe symbol missing from the disassembly: " + symbol)
        mnemonics = []
        csrs = []
        for line in body.group(1).split("\n"):
            match = re.match(r"\s*[0-9a-f]+:\s+[0-9a-f]+\s+(\S+)\s*(.*)$", line)
            if match is None:
                continue
            mnemonic, operands = match.group(1), match.group(2).strip()
            mnemonics.append(mnemonic)
            if mnemonic in CSR_MNEMONICS:
                name = csr_operand(operands)
                if name:
                    csrs.append(name)
        operations[symbol] = (mnemonics, csrs)
    return operations


def compare_core_backends(build, tests, common, target_flags, args, optimization):
    """Compile the core probe with both backends and compare emitted operations."""
    emitted = {}
    for label, extra in (("builtin", []),
                         ("sdk", ["-DSGLL_CORE_BACKEND_SDK", "-isystem", str(args.core_include.resolve())])):
        probe = str(build / ("core_probe_" + label + ".o"))
        run([args.cross_prefix + "gcc", *common, *target_flags, *extra, *optimization,
             "-std=c23", "-c", str(tests / "core_probe.c"), "-o", probe])
        emitted[label] = core_operations(run([args.cross_prefix + "objdump", "-d", probe]))
    for symbol in CORE_PROBE_SYMBOLS:
        if emitted["builtin"][symbol] != emitted["sdk"][symbol]:
            raise RuntimeError("Core backends differ for " + symbol + ": " + repr(emitted["builtin"][symbol]) +
                               " vs " + repr(emitted["sdk"][symbol]))
    if not any(csrs for _, csrs in emitted["builtin"].values()):
        raise RuntimeError("Core probe produced no CSR operations")
    if "rdtime" not in emitted["builtin"]["sgll_test_core_time_read"][0]:
        raise RuntimeError("Time read must emit rdtime")
    if not [name for name in emitted["builtin"]["sgll_test_core_irq_enable"][1] if name == "mstatus"]:
        raise RuntimeError("Global IRQ enable must touch mstatus")
    print(f"core backend A/B {' '.join(optimization)}: {len(CORE_PROBE_SYMBOLS)} primitives identical", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cross-prefix", default=os.environ.get("SG200X_CROSS_COMPILE", "riscv-none-elf-"))
    parser.add_argument("--core-include", type=Path,
                        help="Optional SDK arch directory containing core_rv64.h; enables the SDK core-backend "
                             "header checks and the A/B instruction comparison")
    parser.add_argument("--sdk-cache", type=Path,
                        help="Optional SDK arch/riscv64/src/cache.c; compares its cache instruction words with src")
    parser.add_argument("--target-flags", default=None,
                        help="Override the RV64 target flags (default: -mcpu=thead-c906 -march=rv64imafdc_zicsr "
                             "-mabi=lp64d); use this for a generic RISC-V cross toolchain")
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
    target_flags = (shlex.split(args.target_flags) if args.target_flags
                    else ["-mcpu=thead-c906", "-march=rv64imafdc_zicsr", "-mabi=lp64d"])
    arch = [*target_flags]
    if args.core_include:
        arch += ["-isystem", str(args.core_include.resolve())]
    linux_flags = []
    if args.linux_sysroot:
        sysroot = args.linux_sysroot.resolve()
        linux_flags = ["--sysroot=" + str(sysroot), "-isystem", str(sysroot / "include"),
                       "-B" + str(sysroot / "lib") + "/", "-L" + str(sysroot / "lib")]
    modes = [("host C23", args.cc, "c", "c23", []),
             ("host C++20", args.cxx, "c++", "c++20", []),
             ("RV64 C23", args.cross_prefix + "gcc", "c", "c23", arch),
             ("RV64 C++20", args.cross_prefix + "g++", "c++", "c++20", arch)]
    if args.core_include:
        modes.append(("RV64 C23 SDK core backend", args.cross_prefix + "gcc", "c", "c23",
                      [*arch, "-DSGLL_CORE_BACKEND_SDK"]))
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
                 "-DCMAKE_C_FLAGS=" + shlex.join([*target_flags, *optimization, "-Wall", "-Wextra", "-Werror"])])
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
            print(f"CMake RV64 {' '.join(optimization)} (no SDK headers): static library and C symbols passed",
                  flush=True)
            if args.sdk_cache:
                sgll_words = sorted(set(re.findall(r"\.long (0x[0-9a-f]{8})",
                                                   (root / "src/sg200x_ll_csr.c").read_text())))
                sdk_words = set(re.findall(r"\.long (0x[0-9a-f]{8})", args.sdk_cache.read_text()))
                missing = [word for word in sgll_words if word not in sdk_words]
                if missing:
                    raise RuntimeError("Cache instruction words absent from the SDK source: " + ", ".join(missing))
                print(f"cache encodings: {len(sgll_words)} words match {args.sdk_cache.name}", flush=True)
            if args.core_include:
                compare_core_backends(build, tests, common, target_flags, args, optimization)
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
