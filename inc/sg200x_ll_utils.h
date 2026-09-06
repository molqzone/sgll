/*
 * sg200x_ll_utils.h - MMIO primitives and bit-field helpers.
 *
 * Zero dependencies; only <stdint.h>.  Pure C, no asm (fences live in
 * sg200x_ll_csr.h).
 *
 * Access contract (driver/sg200x_mmio.hpp keeps the same rule): every
 * peripheral access is a 32-bit volatile access.  The DesignWare blocks
 * sit behind the AXI6-to-APB bridge and the TRM does not document byte or
 * halfword access semantics, so sgll never emits them.
 *
 * The read-modify-write macros are NOT atomic across cores.  Register
 * ownership follows the board contract: Linux owns its clock/reset/DMA
 * partitions and C906L firmware owns its own; single-writer discipline is
 * the caller's responsibility (see PLAN section 7).
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
#define SG200X_LL_STATIC_ASSERT(condition, message) static_assert((condition), message)
#else
#define SG200X_LL_STATIC_ASSERT(condition, message) _Static_assert((condition), message)
#endif

/* ------------------------------------------------------------------ */
/* Register access                                                     */
/* ------------------------------------------------------------------ */

#define SG200X_LL_REG32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))

#define SG200X_LL_REG32_AT(base, offset) SG200X_LL_REG32((uintptr_t)(base) + (uintptr_t)(offset))

#define SG200X_LL_READ_REG(reg) (reg)
#define SG200X_LL_WRITE_REG(reg, value) ((reg) = (value))

/* ------------------------------------------------------------------ */
/* Bit and field construction                                          */
/* ------------------------------------------------------------------ */

#define SG200X_LL_BIT(n) (1UL << (n))
#define SG200X_LL_MASK(shift, width) (((1UL << (width)) - 1UL) << (shift))

/* Read-modify-write helpers, CMSIS-LL style: REG is a volatile lvalue. */
#define SG200X_LL_SET_BITS(reg, mask) ((reg) |= (mask))
#define SG200X_LL_CLEAR_BITS(reg, mask) ((reg) &= ~(uint32_t)(mask))
#define SG200X_LL_MODIFY_REG(reg, clear_mask, set_mask)                                                      \
    ((reg) = (((reg) & ~(uint32_t)(clear_mask)) | (set_mask)))
#define SG200X_LL_IS_BIT_SET(reg, mask) (((reg) & (mask)) != 0U)

/* Extract a bit field from a register value. */
static inline uint32_t sg200x_ll_field_get(uint32_t value, uint32_t shift, uint32_t width)
{
    return (value >> shift) & ((1UL << width) - 1UL);
}

/* Build a register value with `field_value` placed into one field,
 * preserving everything outside the field mask. */
static inline uint32_t
sg200x_ll_field_prepare(uint32_t reg_value, uint32_t shift, uint32_t width, uint32_t field_value)
{
    const uint32_t mask = ((1UL << width) - 1UL) << shift;
    return (reg_value & ~mask) | ((field_value << shift) & mask);
}
