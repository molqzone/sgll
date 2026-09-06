/*
 * sg200x_ll_csr.h - C906L CSR/interrupt primitives on the SDK core layer.
 *
 * Adapter over the SDK CSI core headers:
 *   core_rv64.h -> csi_rv64_gcc.h  (__get/__set_MSTATUS, __enable_irq,
 *                  __disable_irq, __get/__set_MIE, __DSB/__ISB)
 *
 * The dependency on the SDK core layer was re-approved for sgll on
 * 2026-09-05 ("you may rely on core_rv64.h" - PLAN section 3.3).  Terms:
 *   - RISC-V builds need the SDK include path (<SDK>/install/include/arch).
 *   - Everything stays guarded by __riscv, so host-side tooling can still
 *     compile every other sgll header unchanged.
 *   - Own asm remains for `rdtime` (the CSI timer accessors are compiled
 *     out, #if 0, in this SDK copy) and for the D-cache maintenance ops
 *     (the CSI __DCACHE_* mnemonics cannot be assembled by the firmware
 *     toolchain; raw instruction words from the SDK's arch/riscv64/src/
 *     cache.c are used instead - see the section comment below).
 *   - Range cache maintenance loops are implemented here (64-byte lines,
 *     mirroring the SDK cache.c pattern).
 *
 * The C906L is RV64 here; the RV32 rdtimeh path is kept for completeness.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(__riscv)

#include "core_rv64.h"

/* ------------------------------------------------------------------ */
/* CSR bit definitions                                                 */
/* ------------------------------------------------------------------ */

/* mstatus bits (RISC-V privileged spec). */
#define SG200X_LL_MSTATUS_MIE_BIT (1UL << 3) /* machine global irq enable */

/* mie bits (RISC-V privileged spec). */
#define SG200X_LL_MIE_SSIE_BIT (1UL << 1)  /* supervisor sw irq */
#define SG200X_LL_MIE_MSIE_BIT (1UL << 3)  /* machine sw irq */
#define SG200X_LL_MIE_STIE_BIT (1UL << 5)  /* supervisor timer irq */
#define SG200X_LL_MIE_MTIE_BIT (1UL << 7)  /* machine timer irq */
#define SG200X_LL_MIE_SEIE_BIT (1UL << 9)  /* supervisor external irq */
#define SG200X_LL_MIE_MEIE_BIT (1UL << 11) /* machine external irq (PLIC) */

/* C906L D-cache line size used by the range maintenance loops. */
#define SG200X_LL_DCACHE_LINE_SIZE 64U

/* ------------------------------------------------------------------ */
/* Raw CSR access via the CSI core layer                               */
/* ------------------------------------------------------------------ */

static inline unsigned long sg200x_ll_csr_mstatus_read(void) { return (unsigned long)__get_MSTATUS(); }

static inline void sg200x_ll_csr_mstatus_write(unsigned long value) { __set_MSTATUS((uint64_t)value); }

static inline unsigned long sg200x_ll_csr_mie_read(void) { return (unsigned long)__get_MIE(); }

static inline void sg200x_ll_csr_mie_write(unsigned long value) { __set_MIE((uint64_t)value); }

/* mie set/clear are read-modify-write through the CSI accessors.  They
 * are not atomic: call them with interrupts disabled (or at init) so a
 * concurrent ISR cannot lose a bit between __get_MIE and __set_MIE. */
static inline void sg200x_ll_csr_mie_set(unsigned long mask) { __set_MIE(__get_MIE() | (uint64_t)mask); }

static inline void sg200x_ll_csr_mie_clear(unsigned long mask) { __set_MIE(__get_MIE() & ~(uint64_t)mask); }

/* ------------------------------------------------------------------ */
/* Global interrupt control (mstatus.MIE via __enable_irq/__disable_irq)*/
/* ------------------------------------------------------------------ */

/* Enable the machine-global interrupt (mstatus.MIE).  IRQ sources still
 * need their mie bits and PLIC enables. */
static inline void sg200x_ll_csr_global_irq_enable(void) { __enable_irq(); }

static inline void sg200x_ll_csr_global_irq_disable(void) { __disable_irq(); }

/* mstatus snapshot for critical sections; returns the prior mstatus
 * (same pattern as the SDK csi_irq_save, which is compiled out there). */
static inline unsigned long sg200x_ll_csr_critical_enter(void)
{
    unsigned long prior = (unsigned long)__get_MSTATUS();
    __disable_irq();
    return prior;
}

/* Restore an mstatus snapshot produced by sg200x_ll_csr_critical_enter(). */
static inline void sg200x_ll_csr_critical_exit(unsigned long prior) { sg200x_ll_csr_mstatus_write(prior); }

/* Per-source machine interrupt enables (mie bits). */
static inline void sg200x_ll_csr_mie_enable(unsigned long mie_mask) { sg200x_ll_csr_mie_set(mie_mask); }

static inline void sg200x_ll_csr_mie_disable(unsigned long mie_mask) { sg200x_ll_csr_mie_clear(mie_mask); }

/* Convenience for PLIC users: the PLIC output wires into machine external
 * interrupt, mie bit 11. */
static inline void sg200x_ll_csr_external_irq_enable(void) { sg200x_ll_csr_mie_set(SG200X_LL_MIE_MEIE_BIT); }

static inline void sg200x_ll_csr_external_irq_disable(void)
{
    sg200x_ll_csr_mie_clear(SG200X_LL_MIE_MEIE_BIT);
}

/* ------------------------------------------------------------------ */
/* Time CSR                                                            */
/* ------------------------------------------------------------------ */

/* Read the `time` CSR (25 MHz on this board, see sg2002.h).  The firmware
 * SDK explicitly has no CLINT mtime MMIO for non-QEMU targets and the TRM
 * marks the 0x74000000 area reserved; the CSR is the only verified source
 * (driver/sg200x_timebase.cpp uses exactly this form). */
#if __riscv_xlen == 64
static inline uint64_t sg200x_ll_csr_time_read(void)
{
    uint64_t value;
    __ASM volatile("rdtime %0" : "=r"(value));
    return value;
}
#elif __riscv_xlen == 32
static inline uint64_t sg200x_ll_csr_time_read(void)
{
    uint32_t low;
    uint32_t high0;
    uint32_t high1;
    do
    {
        __ASM volatile("rdtimeh %0" : "=r"(high0));
        __ASM volatile("rdtime %0" : "=r"(low));
        __ASM volatile("rdtimeh %0" : "=r"(high1));
    } while (high0 != high1);
    return ((uint64_t)high1 << 32) | low;
}
#else
#error "Unsupported __riscv_xlen"
#endif

/* ------------------------------------------------------------------ */
/* Fences                                                              */
/*                                                                     */
/* On RISC-V a bare `fence` (what __DSB()/__ISB() emit) defaults to     */
/* fence iorw, iorw.  MMIO write-then-poll paths and the TOP_MISC       */
/* cross-core routing registers go through sg200x_ll_csr_fence_io().        */
/* ------------------------------------------------------------------ */

static inline void sg200x_ll_csr_fence_io(void) { __DSB(); }

static inline void sg200x_ll_csr_fence_i(void) { __ISB(); }

/* ------------------------------------------------------------------ */
/* D-cache maintenance (raw T-HEAD C906 instruction words)             */
/*                                                                     */
/* The toolchain assembler does NOT accept the dcache.* mnemonics, so  */
/* the CSI per-line helpers (__DCACHE_IPA/CPA/CIPA) can never be       */
/* instantiated.  These functions copy the encoding used by the SDK's  */
/* board-verified out-of-line implementation (arch/riscv64/src/        */
/* cache.c), which the driver/ DMA paths already call: raw .long       */
/* instruction words with rs1 hardwired to a0, one per 64-byte line,   */
/* closed by sync.s:                                                   */
/*   dcache.ipa  a0  ->  .long 0x02a5000b   (invalidate)               */
/*   dcache.cpa  a0  ->  .long 0x0295000b   (clean)                    */
/*   dcache.cipa a0  ->  .long 0x02b5000b   (clean then invalidate)    */
/*   sync.s          ->  .long 0x0190000b                              */
/* Callers keep DMA buffers cache-line aligned (driver/ contract).     */
/* ------------------------------------------------------------------ */

#define SG200X_LL_DCACHE_IPA_A0 ".long 0x02a5000b"
#define SG200X_LL_DCACHE_CPA_A0 ".long 0x0295000b"
#define SG200X_LL_DCACHE_CIPA_A0 ".long 0x02b5000b"
#define SG200X_LL_SYNC_S ".long 0x0190000b"

#define SG200X_LL_DCACHE_OP_RANGE(OP, addr, size)                                                            \
    do                                                                                                       \
    {                                                                                                        \
        register uintptr_t line_ asm("a0") =                                                                 \
            (uintptr_t)(addr) & ~(uintptr_t)(SG200X_LL_DCACHE_LINE_SIZE - 1U);                               \
        const uintptr_t end_ =                                                                               \
            (((uintptr_t)(addr) + (size_t)(size) + (SG200X_LL_DCACHE_LINE_SIZE - 1U)) &                      \
             ~(uintptr_t)(SG200X_LL_DCACHE_LINE_SIZE - 1U));                                                 \
        for (; line_ < end_; line_ += SG200X_LL_DCACHE_LINE_SIZE)                                            \
        {                                                                                                    \
            __asm__ __volatile__(OP);                                                                        \
        }                                                                                                    \
        __asm__ __volatile__(SG200X_LL_SYNC_S);                                                              \
    } while (0)

static inline void sg200x_ll_csr_dcache_invalidate_range(uintptr_t addr, size_t size)
{
    SG200X_LL_DCACHE_OP_RANGE(SG200X_LL_DCACHE_IPA_A0, addr, size);
}

static inline void sg200x_ll_csr_dcache_clean_range(uintptr_t addr, size_t size)
{
    SG200X_LL_DCACHE_OP_RANGE(SG200X_LL_DCACHE_CPA_A0, addr, size);
}

static inline void sg200x_ll_csr_dcache_clean_invalidate_range(uintptr_t addr, size_t size)
{
    SG200X_LL_DCACHE_OP_RANGE(SG200X_LL_DCACHE_CIPA_A0, addr, size);
}

#endif /* __riscv */
