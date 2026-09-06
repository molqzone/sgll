/*
 * sg200x_ll_plic.h - PLIC primitives for the C906L context.
 *
 * Dependencies: sg200x_ll_csr.h, sg200x_ll_utils.h, sg200x_ll_defs.h.
 *
 * The SG2002 TRM has NO PLIC register description.  The layout here is
 * the standard RISC-V PLIC memory map, grounded on:
 *   - SDK freertos/cvitek/arch/riscv64/include/irq.h (region offsets)
 *   - board DTS interrupt-controller@70000000 (riscv,ndev = 101,
 *     riscv,max-priority = 7 for the big-core instance)
 *   - driver/sg200x_dma.cpp, which claims/completes at 0x70200004 on
 *     the board (C906L, context 0, M-mode)
 *
 * The C906L ("CPU2") PLIC uses its own source numbering (see the
 * SG200X_LL_IRQ_* list in sg200x_ll_defs.h); it does NOT match the
 * big-core TRM Table 3.1 numbering.
 *
 * Region map (context 0), offsets from 0x7000_0000:
 *   0x0000_0000 + 4*irq   source priority (word per source, 1..ndev)
 *   0x0001_0000 + 4*word  pending (read-only, bits 31:0 per word)
 *   0x0002_0000 + 4*word  enable for context 0
 *   0x0020_0000           context-0 threshold
 *   0x0020_0004           context-0 claim/complete (read = claim,
 *                         write = complete)
 */
#pragma once

#include <stdbool.h>

#include "sg200x_ll_csr.h"
#include "sg200x_ll_defs.h"
#include "sg200x_ll_utils.h"

/* Region offsets from SG200X_PLIC_BASE (0x7000_0000). */
#define SG200X_LL_PLIC_PRIORITY_OFFSET 0x0000UL
#define SG200X_LL_PLIC_PENDING_OFFSET 0x1000UL
#define SG200X_LL_PLIC_ENABLE_OFFSET 0x2000UL
#define SG200X_LL_PLIC_THRESHOLD_OFFSET 0x200000UL
#define SG200X_LL_PLIC_CLAIM_OFFSET 0x200004UL

/* Source-level register access.  irq is the C906L PLIC source number
 * (1..SG200X_LL_NUM_IRQ-1; 0 means "no interrupt" on claim). */
#define SG200X_LL_PLIC_PRIORITY_REG(irq)                                                                     \
    SG200X_LL_REG32_AT(SG200X_PLIC_BASE, SG200X_LL_PLIC_PRIORITY_OFFSET + 4UL * (irq))
#define SG200X_LL_PLIC_PENDING_REG(irq)                                                                      \
    SG200X_LL_REG32_AT(SG200X_PLIC_BASE, SG200X_LL_PLIC_PENDING_OFFSET + 4UL * ((irq) / 32UL))
#define SG200X_LL_PLIC_ENABLE_REG(irq)                                                                       \
    SG200X_LL_REG32_AT(SG200X_PLIC_BASE, SG200X_LL_PLIC_ENABLE_OFFSET + 4UL * ((irq) / 32UL))
#define SG200X_LL_PLIC_THRESHOLD_REG() SG200X_LL_REG32_AT(SG200X_PLIC_BASE, SG200X_LL_PLIC_THRESHOLD_OFFSET)
#define SG200X_LL_PLIC_CLAIM_REG() SG200X_LL_REG32_AT(SG200X_PLIC_BASE, SG200X_LL_PLIC_CLAIM_OFFSET)

/* ------------------------------------------------------------------ */
/* Source priority                                                      */
/* ------------------------------------------------------------------ */

/* prio: 0 disables the source; 1..7 ascending priority
 * (SG200X_LL_PLIC_PRIORITY_MAX from the board DTS). */
static inline void sg200x_ll_plic_irq_priority_set(uint32_t irq, uint32_t prio)
{
    SG200X_LL_WRITE_REG(SG200X_LL_PLIC_PRIORITY_REG(irq), prio);
}

static inline uint32_t sg200x_ll_plic_irq_priority_get(uint32_t irq)
{
    return SG200X_LL_READ_REG(SG200X_LL_PLIC_PRIORITY_REG(irq));
}

/* ------------------------------------------------------------------ */
/* Source enable (context 0)                                            */
/* ------------------------------------------------------------------ */

static inline void sg200x_ll_plic_irq_enable(uint32_t irq)
{
    SG200X_LL_SET_BITS(SG200X_LL_PLIC_ENABLE_REG(irq), SG200X_LL_BIT((irq) % 32UL));
}

static inline void sg200x_ll_plic_irq_disable(uint32_t irq)
{
    SG200X_LL_CLEAR_BITS(SG200X_LL_PLIC_ENABLE_REG(irq), SG200X_LL_BIT((irq) % 32UL));
}

static inline bool sg200x_ll_plic_irq_is_enabled(uint32_t irq)
{
    return SG200X_LL_IS_BIT_SET(SG200X_LL_PLIC_ENABLE_REG(irq), SG200X_LL_BIT((irq) % 32UL));
}

static inline bool sg200x_ll_plic_irq_is_pending(uint32_t irq)
{
    return SG200X_LL_IS_BIT_SET(SG200X_LL_PLIC_PENDING_REG(irq), SG200X_LL_BIT((irq) % 32UL));
}

/* ------------------------------------------------------------------ */
/* Context threshold                                                    */
/* ------------------------------------------------------------------ */

/* Sources with priority <= threshold never signal this context. */
static inline void sg200x_ll_plic_threshold_set(uint32_t threshold)
{
    SG200X_LL_WRITE_REG(SG200X_LL_PLIC_THRESHOLD_REG(), threshold);
}

static inline uint32_t sg200x_ll_plic_threshold_get(void)
{
    return SG200X_LL_READ_REG(SG200X_LL_PLIC_THRESHOLD_REG());
}

/* ------------------------------------------------------------------ */
/* Claim / complete                                                     */
/* ------------------------------------------------------------------ */

/* Read the highest-priority pending source for context 0.  Returns 0
 * when no eligible interrupt is pending.  The read side-effect marks the
 * source as "in service" until completed. */
static inline uint32_t sg200x_ll_plic_irq_claim(void)
{
    return SG200X_LL_READ_REG(SG200X_LL_PLIC_CLAIM_REG());
}

/* Complete service for a source obtained from sg200x_ll_plic_irq_claim().
 * Writing 0 or a source that was never claimed has no effect on claim
 * state, so only pass claimed values. */
static inline void sg200x_ll_plic_irq_complete(uint32_t irq)
{
    SG200X_LL_WRITE_REG(SG200X_LL_PLIC_CLAIM_REG(), irq);
}

/* ------------------------------------------------------------------ */
/* Machine-side unmask (RISC-V only)                                   */
/* ------------------------------------------------------------------ */

#if defined(__riscv)

/* Enable the PLIC output at the core: mie.MEIE.  The global
 * mstatus.MIE is managed separately (sg200x_ll_csr_global_irq_enable)
 * or by the RTOS. */
static inline void sg200x_ll_plic_core_enable(void) { sg200x_ll_csr_external_irq_enable(); }

static inline void sg200x_ll_plic_core_disable(void) { sg200x_ll_csr_external_irq_disable(); }

#endif /* __riscv */

/* ------------------------------------------------------------------ */
/* Self checks                                                          */
/* ------------------------------------------------------------------ */

#include <stdbool.h>

#if defined(SG200X_LL_PLIC_ENABLE_REG)
/* The enable region must cover SG200X_LL_NUM_IRQ sources with whole
 * words; 62 sources use exactly words 0 and 1. */
SG200X_LL_STATIC_ASSERT(
    (SG200X_LL_NUM_IRQ + 31UL) / 32UL == 2UL, "C906L PLIC enable words changed; update region math"
);
#endif
