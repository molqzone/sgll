# SGLL regression checks

From the BSP root, run:

```sh
python3 sgll/tests/run.py \
  --cross-prefix "$SG200X_CROSS_COMPILE" \
  --core-include build/sdk-worktree/freertos/cvitek/install/include/arch
python3 sgll/tools/audit_vs_trm.py
```

The audit defaults to the enclosing BSP and its staged SDK. For a standalone
SGLL checkout or a second firmware workspace, pass `--bsp-root`, `--sdk-root`,
`--trm`, and/or `--clock-tree` as needed. The SDK argument accepts either the
SDK repository root or its `freertos/cvitek` directory; no workstation path is
embedded in the audit script.

The regression runner needs a Linux host, CMake/Ninja, GCC/G++ with C23
support, the firmware's xPack RISC-V compiler, `riscv64-linux-gnu-gcc` with static libc, and
`qemu-riscv64`. All generated files go into a temporary directory.
An unpacked RISC-V libc tree can be supplied with `--linux-sysroot PATH`;
`PATH` must contain its `include/` and `lib/` directories.

It compiles every header independently in strict C23 and C++20 for the host
and C906. CMake builds the C23 static library and links both C and C++
consumers. Anonymous pages at the register addresses let the same checks
exercise inline primitives and compiled operations without hardware. They
cover DMA register writes, LLI alignment, invalid addresses, bounded waits,
RCC reset lookup and clocks, UART interrupt IDs, PINMUX bounds and pad-bank
selection, and PLIC priority/pending/enable/claim registers against the TRM
and SDK. Cache probes
link the separately compiled library built with the real SDK headers and
firmware compiler at `-O2` and `-O3 -funroll-loops`. Each public compiled
operation must appear exactly once as a C symbol in the archive.
A QEMU SIGILL handler records the actual `a0` operand
of each C906 cache instruction and verifies the requested cache-line sequence
and final `sync.s`. The runner separately verifies the `fence.i` and
`fence iorw, iorw` opcodes.
These checks validate register encoding and compiler behavior, not physical
cache coherence or peripheral operation on a board.

`mbox.c` runs through both C and C++ callers. It covers single-slot send/receive,
header snapshots, empty and busy states, zero-length and full-size payloads,
invalid alignment and address overflow, overlapping outputs, and preservation
of outputs and shared data on rejection. The RV64 cache probe also checks that
the state line is refreshed before the header snapshot, payload/header writes
complete before ready is published, and release writes back only the state line.

`init.c` is also linked and run as both C and C++. It checks SPI/I2C/UART/DMA
default configurations, documented wire formats and I2C timing profiles,
UART divisor rounding and DLAB exit, invalid-input and active-device rejection,
circular descriptors, per-instance reset selection, and preservation of other
DMA channels and global routing. The register image records the last write to
aliased UART addresses; it does not emulate IER/DLH banking or hardware reset
side effects.

`peripherals.c` checks the GPIO/PWM/ADC/watchdog interfaces used by the migrated
drivers: preservation of neighboring pins/channels and reserved fields, rejection
of invalid or active configurations without writes, ADC domain/channel selection,
result validity, watchdog routing and shared clock selection, and bounded waits.
I2C timing is checked against frozen driver counts at 25, 50, 100, and 187.5 MHz,
including clocks that cannot be represented, ten-bit target transitions, and
enable timeouts. Both C23 and C++20 consumers run the same checks.

The incomplete `sgll_dma_lli_program()` and its configuration type were
removed. Build descriptors with `sgll_dma_lli_build()`, clean them for DMA,
then call `sgll_dma_channel_configure()` with a configuration from
`sgll_dma_config_build()`. Channel configuration now returns `false` for
invalid channels or descriptor addresses without writing any registers.
