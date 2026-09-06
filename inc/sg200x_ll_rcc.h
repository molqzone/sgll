/*
 * sg200x_ll_rcc.h - CLKGEN/PLL and RSTGEN register primitives.
 *
 * Dependencies: sg200x_ll_utils.h, sg200x_ll_defs.h (no csr usage).
 *
 * References (all TRM v1.02):
 *   - Clock Gen register overview: Table 8.51 (base 0x03002000).
 *   - clk_en_0..4 bit maps: Tables 8.52..8.56.
 *   - clk_sel_0 / clk_byp_0 / clk_byp_1: Tables 8.57..8.59.
 *   - Divider register layout: Tables 8.79 (axi4), 8.88..8.90 (1m/spi/i2c),
 *     8.93 (pwm_src_0); the layout is uniform:
 *       [0]     divider reset (0 = assert, 1 = de-assert)
 *       [3]     factor source (0 = hardware initial value, 1 = [20:16])
 *       [9:8]   clk_src on muxed leaves (0 = fpll, 1 = disppll)
 *       [20:16] divider factor
 *   - Reset CRG: Tables 7.1..7.5 (base 0x03003000, active low).
 *
 * Ownership contract (PLAN section 7, mirrors driver/sg200x_clock_tree.hpp):
 *   - Gate, bypass, source, divider and reset bits of the single-path
 *     peripheral branches are C906L-managed and exposed for write here.
 *   - PLL CSRs, clk_sel_0 (A53/C906 core source selects), the C906 core
 *     dividers (0x130..0x13c) and clk_en_4 core-enable bits belong to the
 *     boot/system contract: read-only here.  Decode logic for PLL rates
 *     stays in driver/sg200x_clock_tree.hpp.
 *   - These are startup/thread control-plane calls, never ISR APIs.  The
 *     TRM (7.3) requires a module's bus and DMA activity to be idle
 *     before its reset is asserted.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sg200x_ll_defs.h"
#include "sg200x_ll_utils.h"

/* ------------------------------------------------------------------ */
/* CLKGEN register offsets (TRM Table 8.51)                            */
/* ------------------------------------------------------------------ */

#define SG200X_LL_RCC_CLKEN_REG(n) SG200X_LL_REG32_AT(SG200X_CLKGEN_BASE, 0x000UL + 4UL * (n))

#define SG200X_LL_RCC_CLK_SEL0_REG SG200X_LL_REG32_AT(SG200X_CLKGEN_BASE, 0x020UL)
#define SG200X_LL_RCC_CLK_BYP0_REG SG200X_LL_REG32_AT(SG200X_CLKGEN_BASE, 0x030UL)
#define SG200X_LL_RCC_CLK_BYP1_REG SG200X_LL_REG32_AT(SG200X_CLKGEN_BASE, 0x034UL)

/* Divider registers (offsets from SG200X_CLKGEN_BASE). */
#define SG200X_LL_RCC_DIV_OFF_GPIO_DB 0x094UL
#define SG200X_LL_RCC_DIV_OFF_SDMA_AUD0 0x098UL
#define SG200X_LL_RCC_DIV_OFF_SDMA_AUD1 0x09cUL
#define SG200X_LL_RCC_DIV_OFF_SDMA_AUD2 0x0a0UL
#define SG200X_LL_RCC_DIV_OFF_SDMA_AUD3 0x0a4UL
#define SG200X_LL_RCC_DIV_OFF_AXI4 0x0b8UL
#define SG200X_LL_RCC_DIV_OFF_AXI6 0x0bcUL
#define SG200X_LL_RCC_DIV_OFF_1M 0x0fcUL
#define SG200X_LL_RCC_DIV_OFF_SPI 0x100UL
#define SG200X_LL_RCC_DIV_OFF_I2C 0x104UL
#define SG200X_LL_RCC_DIV_OFF_PWM_SRC 0x120UL
#define SG200X_LL_RCC_DIV_OFF_C906_0_0 0x130UL /* read-only contract */
#define SG200X_LL_RCC_DIV_OFF_C906_0_1 0x134UL /* read-only contract */
#define SG200X_LL_RCC_DIV_OFF_C906_1_0 0x138UL /* read-only contract */
#define SG200X_LL_RCC_DIV_OFF_C906_1_1 0x13cUL /* read-only contract */

#define SG200X_LL_RCC_DIV_REG(offset) SG200X_LL_REG32_AT(SG200X_CLKGEN_BASE, (offset))

/* Divider field layout shared by every div_* register. */
#define SG200X_LL_RCC_DIV_RESET_BIT SG200X_LL_BIT(0)
#define SG200X_LL_RCC_DIV_USE_REG_FACTOR_BIT SG200X_LL_BIT(3)
#define SG200X_LL_RCC_DIV_SRC_SHIFT 8U
#define SG200X_LL_RCC_DIV_SRC_WIDTH 2U
#define SG200X_LL_RCC_DIV_FACTOR_SHIFT 16U

/* clk_src encodings for leaves with a [9:8] source field
 * (TRM Tables 8.79/8.93). */
#define SG200X_LL_RCC_SRC_FPLL 0UL
#define SG200X_LL_RCC_SRC_DISPPLL 1UL

/* ------------------------------------------------------------------ */
/* Clock gates: (clk_en register index, bit) tuples                    */
/*                                                                     */
/* Each SG200X_LL_RCC_GATE_* macro expands to two arguments for         */
/* sg200x_ll_rcc_gate_enable/Disable/IsEnabled, so a bit can never be    */
/* paired with the wrong clk_en register.                               */
/* Bit sources: TRM Tables 8.52..8.56; cross-checked against            */
/* driver/sg200x_clock_tree.hpp Field() entries.                        */
/* ------------------------------------------------------------------ */

/* clk_en_0 (Table 8.52) */
#define SG200X_LL_RCC_GATE_RTC_25M 0U, 8U
#define SG200X_LL_RCC_GATE_TEMPSEN 0U, 9U
#define SG200X_LL_RCC_GATE_SARADC 0U, 10U
#define SG200X_LL_RCC_GATE_EFUSE 0U, 11U
#define SG200X_LL_RCC_GATE_APB_EFUSE 0U, 12U
#define SG200X_LL_RCC_GATE_XTAL_MISC 0U, 14U
#define SG200X_LL_RCC_GATE_APB_GPIO 0U, 29U
#define SG200X_LL_RCC_GATE_APB_GPIO_INTR 0U, 30U
#define SG200X_LL_RCC_GATE_GPIO_DB 0U, 31U

/* clk_en_1 (Table 8.53) */
#define SG200X_LL_RCC_GATE_SDMA_AXI 1U, 1U
#define SG200X_LL_RCC_GATE_SDMA_AUD0 1U, 2U
#define SG200X_LL_RCC_GATE_SDMA_AUD1 1U, 3U
#define SG200X_LL_RCC_GATE_SDMA_AUD2 1U, 4U
#define SG200X_LL_RCC_GATE_SDMA_AUD3 1U, 5U
#define SG200X_LL_RCC_GATE_APB_I2C 1U, 6U
#define SG200X_LL_RCC_GATE_APB_WDT 1U, 7U
#define SG200X_LL_RCC_GATE_APB_PWM 1U, 8U
#define SG200X_LL_RCC_GATE_APB_SPI0 1U, 9U
#define SG200X_LL_RCC_GATE_APB_SPI1 1U, 10U
#define SG200X_LL_RCC_GATE_APB_SPI2 1U, 11U
#define SG200X_LL_RCC_GATE_APB_SPI3 1U, 12U
#define SG200X_LL_RCC_GATE_UART0 1U, 14U
#define SG200X_LL_RCC_GATE_APB_UART0 1U, 15U
#define SG200X_LL_RCC_GATE_UART1 1U, 16U
#define SG200X_LL_RCC_GATE_APB_UART1 1U, 17U
#define SG200X_LL_RCC_GATE_UART2 1U, 18U
#define SG200X_LL_RCC_GATE_APB_UART2 1U, 19U
#define SG200X_LL_RCC_GATE_UART3 1U, 20U
#define SG200X_LL_RCC_GATE_APB_UART3 1U, 21U
#define SG200X_LL_RCC_GATE_UART4 1U, 22U
#define SG200X_LL_RCC_GATE_APB_UART4 1U, 23U
#define SG200X_LL_RCC_GATE_APB_I2S0 1U, 24U
#define SG200X_LL_RCC_GATE_APB_I2S1 1U, 25U
#define SG200X_LL_RCC_GATE_APB_I2S2 1U, 26U
#define SG200X_LL_RCC_GATE_APB_I2S3 1U, 27U
#define SG200X_LL_RCC_GATE_AXI4_USB 1U, 28U
#define SG200X_LL_RCC_GATE_APB_USB 1U, 29U

/* clk_en_2 (Table 8.54) - fabric roots, video block omitted (Linux-owned) */
#define SG200X_LL_RCC_GATE_AXI4 2U, 1U
#define SG200X_LL_RCC_GATE_AXI6 2U, 2U

/* clk_en_3 (Table 8.55) */
#define SG200X_LL_RCC_GATE_1M 3U, 5U
#define SG200X_LL_RCC_GATE_SPI 3U, 6U
#define SG200X_LL_RCC_GATE_I2C 3U, 7U
#define SG200X_LL_RCC_GATE_PM 3U, 8U
#define SG200X_LL_RCC_GATE_TIMER0 3U, 9U
#define SG200X_LL_RCC_GATE_TIMER1 3U, 10U
#define SG200X_LL_RCC_GATE_TIMER2 3U, 11U
#define SG200X_LL_RCC_GATE_TIMER3 3U, 12U
#define SG200X_LL_RCC_GATE_TIMER4 3U, 13U
#define SG200X_LL_RCC_GATE_TIMER5 3U, 14U
#define SG200X_LL_RCC_GATE_TIMER6 3U, 15U
#define SG200X_LL_RCC_GATE_TIMER7 3U, 16U
#define SG200X_LL_RCC_GATE_APB_I2C0 3U, 17U
#define SG200X_LL_RCC_GATE_APB_I2C1 3U, 18U
#define SG200X_LL_RCC_GATE_APB_I2C2 3U, 19U
#define SG200X_LL_RCC_GATE_APB_I2C3 3U, 20U
#define SG200X_LL_RCC_GATE_APB_I2C4 3U, 21U
#define SG200X_LL_RCC_GATE_WGN 3U, 22U
#define SG200X_LL_RCC_GATE_WGN0 3U, 23U
#define SG200X_LL_RCC_GATE_WGN1 3U, 24U
#define SG200X_LL_RCC_GATE_WGN2 3U, 25U
#define SG200X_LL_RCC_GATE_KEYSCAN 3U, 26U
#define SG200X_LL_RCC_GATE_AHB_SF1 3U, 27U

/* clk_en_4 (Table 8.56) - core enable bits 13/14 are boot-contract only */
#define SG200X_LL_RCC_GATE_AUDSRC 4U, 1U
#define SG200X_LL_RCC_GATE_APB_AUDSRC 4U, 2U
#define SG200X_LL_RCC_GATE_PWM_SRC 4U, 4U
#define SG200X_LL_RCC_GATE_RTCSYS_SRC0 4U, 6U

/* ------------------------------------------------------------------ */
/* Gate API                                                            */
/* ------------------------------------------------------------------ */

static inline void sg200x_ll_rcc_gate_enable(unsigned gate_reg, unsigned gate_bit)
{
    SG200X_LL_SET_BITS(SG200X_LL_RCC_CLKEN_REG(gate_reg), SG200X_LL_BIT(gate_bit));
}

static inline void sg200x_ll_rcc_gate_disable(unsigned gate_reg, unsigned gate_bit)
{
    SG200X_LL_CLEAR_BITS(SG200X_LL_RCC_CLKEN_REG(gate_reg), SG200X_LL_BIT(gate_bit));
}

static inline bool sg200x_ll_rcc_gate_is_enabled(unsigned gate_reg, unsigned gate_bit)
{
    return SG200X_LL_IS_BIT_SET(SG200X_LL_RCC_CLKEN_REG(gate_reg), SG200X_LL_BIT(gate_bit));
}

/* ------------------------------------------------------------------ */
/* Bypass-to-xtal bits (TRM Tables 8.58/8.59; 1 = parent bypassed to   */
/* 25 MHz xtal, 0 = normal parent path).  Reset value is 1 (bypassed), */
/* boot firmware clears them after PLL bring-up.                       */
/* ------------------------------------------------------------------ */

#define SG200X_LL_RCC_BYPASS_PWM_SRC 0U, 15U
#define SG200X_LL_RCC_BYPASS_AXI4 0U, 19U
#define SG200X_LL_RCC_BYPASS_AXI6 0U, 20U
#define SG200X_LL_RCC_BYPASS_SPI 0U, 30U
#define SG200X_LL_RCC_BYPASS_I2C 0U, 31U

static inline void sg200x_ll_rcc_bypass_enable(unsigned bypass_reg, unsigned bypass_bit)
{
    if (bypass_reg == 0U)
    {
        SG200X_LL_SET_BITS(SG200X_LL_RCC_CLK_BYP0_REG, SG200X_LL_BIT(bypass_bit));
    }
    else
    {
        SG200X_LL_SET_BITS(SG200X_LL_RCC_CLK_BYP1_REG, SG200X_LL_BIT(bypass_bit));
    }
}

static inline void sg200x_ll_rcc_bypass_disable(unsigned bypass_reg, unsigned bypass_bit)
{
    if (bypass_reg == 0U)
    {
        SG200X_LL_CLEAR_BITS(SG200X_LL_RCC_CLK_BYP0_REG, SG200X_LL_BIT(bypass_bit));
    }
    else
    {
        SG200X_LL_CLEAR_BITS(SG200X_LL_RCC_CLK_BYP1_REG, SG200X_LL_BIT(bypass_bit));
    }
}

static inline bool sg200x_ll_rcc_bypass_is_enabled(unsigned bypass_reg, unsigned bypass_bit)
{
    const uint32_t reg = (bypass_reg == 0U) ? SG200X_LL_READ_REG(SG200X_LL_RCC_CLK_BYP0_REG)
                                            : SG200X_LL_READ_REG(SG200X_LL_RCC_CLK_BYP1_REG);
    return SG200X_LL_IS_BIT_SET(reg, SG200X_LL_BIT(bypass_bit));
}

/* ------------------------------------------------------------------ */
/* clk_sel_0 (Table 8.57) - read-only contract                         */
/*                                                                     */
/* Bits 0/23/24 select divider path 0 vs 1 for clk_a53/clk_c906_0/     */
/* clk_c906_1.  Switching the running core's path is a boot-time       */
/* decision; sgll only reports the current selection.                  */
/* ------------------------------------------------------------------ */

#define SG200X_LL_RCC_SEL0_A53_BIT SG200X_LL_BIT(0)
#define SG200X_LL_RCC_SEL0_C906_0_BIT SG200X_LL_BIT(23)
#define SG200X_LL_RCC_SEL0_C906_1_BIT SG200X_LL_BIT(24)

static inline bool sg200x_ll_rcc_c906_0_uses_path0(void)
{
    return SG200X_LL_IS_BIT_SET(
        SG200X_LL_READ_REG(SG200X_LL_RCC_CLK_SEL0_REG), SG200X_LL_RCC_SEL0_C906_0_BIT
    );
}

static inline bool sg200x_ll_rcc_c906_1_uses_path0(void)
{
    return SG200X_LL_IS_BIT_SET(
        SG200X_LL_READ_REG(SG200X_LL_RCC_CLK_SEL0_REG), SG200X_LL_RCC_SEL0_C906_1_BIT
    );
}

/* ------------------------------------------------------------------ */
/* Divider leaves                                                      */
/*                                                                     */
/* Generic contract per div register:                                  */
/*   [0] 0=assert divider reset, 1=de-assert                           */
/*   [3] 1=use the factor field, 0=use hardware initial value          */
/*   [9:8] parent select on muxed leaves (0=fpll, 1=disppll)           */
/*   factor field starting at [16]                                     */
/*                                                                     */
/* Factor field WIDTH: the TRM tables print "[20:16] Clock Divider     */
/* Factor" for every divider without stating per-clock widths.  The    */
/* widths used here (axi4/axi6/i2c = 4 bits [19:16], 1m/spi/pwm_src =  */
/* 6 bits [21:16]) come from driver/sg200x_clock_tree.hpp Divide()     */
/* entries, which the on-board SG200XRCC rate plan exercises.  They    */
/* supersede the TRM's generic [20:16] notation.                       */
/*                                                                     */
/* "Effective factor" mirrors the hardware rule implemented by         */
/* SG200XRCC: while [3] is clear the branch runs on the hardware       */
/* initial value, so the factor field is not authoritative.            */
/* ------------------------------------------------------------------ */

static inline uint32_t sg200x_ll_rcc_div_factor_raw_get(uint32_t div_offset, uint32_t factor_width)
{
    return sg200x_ll_field_get(
        SG200X_LL_READ_REG(SG200X_LL_RCC_DIV_REG(div_offset)), SG200X_LL_RCC_DIV_FACTOR_SHIFT, factor_width
    );
}

static inline bool sg200x_ll_rcc_div_uses_register_factor(uint32_t div_offset)
{
    return SG200X_LL_IS_BIT_SET(
        SG200X_LL_READ_REG(SG200X_LL_RCC_DIV_REG(div_offset)), SG200X_LL_RCC_DIV_USE_REG_FACTOR_BIT
    );
}

static inline bool sg200x_ll_rcc_div_reset_is_deasserted(uint32_t div_offset)
{
    return SG200X_LL_IS_BIT_SET(
        SG200X_LL_READ_REG(SG200X_LL_RCC_DIV_REG(div_offset)), SG200X_LL_RCC_DIV_RESET_BIT
    );
}

static inline void sg200x_ll_rcc_div_factor_set(uint32_t div_offset, uint32_t factor_width, uint32_t factor)
{
    uint32_t value = SG200X_LL_READ_REG(SG200X_LL_RCC_DIV_REG(div_offset));
    value = sg200x_ll_field_prepare(value, SG200X_LL_RCC_DIV_FACTOR_SHIFT, factor_width, factor);
    value |= SG200X_LL_RCC_DIV_USE_REG_FACTOR_BIT;
    value |= SG200X_LL_RCC_DIV_RESET_BIT; /* never leave a divider in reset */
    SG200X_LL_WRITE_REG(SG200X_LL_RCC_DIV_REG(div_offset), value);
}

static inline void sg200x_ll_rcc_div_reset_assert(uint32_t div_offset)
{
    SG200X_LL_CLEAR_BITS(SG200X_LL_RCC_DIV_REG(div_offset), SG200X_LL_RCC_DIV_RESET_BIT);
}

static inline void sg200x_ll_rcc_div_reset_release(uint32_t div_offset)
{
    SG200X_LL_SET_BITS(SG200X_LL_RCC_DIV_REG(div_offset), SG200X_LL_RCC_DIV_RESET_BIT);
}

static inline uint32_t sg200x_ll_rcc_div_source_get(uint32_t div_offset)
{
    return sg200x_ll_field_get(
        SG200X_LL_READ_REG(SG200X_LL_RCC_DIV_REG(div_offset)), SG200X_LL_RCC_DIV_SRC_SHIFT,
        SG200X_LL_RCC_DIV_SRC_WIDTH
    );
}

static inline void sg200x_ll_rcc_div_source_set(uint32_t div_offset, uint32_t source)
{
    uint32_t value = SG200X_LL_READ_REG(SG200X_LL_RCC_DIV_REG(div_offset));
    value = sg200x_ll_field_prepare(value, SG200X_LL_RCC_DIV_SRC_SHIFT, SG200X_LL_RCC_DIV_SRC_WIDTH, source);
    SG200X_LL_WRITE_REG(SG200X_LL_RCC_DIV_REG(div_offset), value);
}

/* Named leaves.  Two macro families, matched to the generic functions:
 *   - SG200X_LL_RCC_DIV_<LEAF>     -> (offset, factor_width) tuple for
 *      sg200x_ll_rcc_div_factor_set / DivFactorRaw
 *   - SG200X_LL_RCC_DIV_OFF_<LEAF> -> bare offset for single-offset
 *      functions (reset, source, use-register-factor probes)
 * Source ([9:8]) accessors are only meaningful on leaves that have the
 * field (axi4, pwm_src); on spi/i2c/axi6/1m the xtal-vs-PLL parent is
 * selected by the bypass bit instead. */

/* clk_axi4: factor width 4, src [9:8], bypass clk_byp_0[19],
 * gate clk_en_2[1].  Initial factor 5 (driver Divide reset value);
 * board plan: FPLL 1500 MHz / 5 = 300 MHz. */
#define SG200X_LL_RCC_DIV_AXI4 SG200X_LL_RCC_DIV_OFF_AXI4, 4U
#define SG200X_LL_RCC_AXI4_MAX_FACTOR 15U
#define SG200X_LL_RCC_AXI4_INITIAL_FACTOR 5U

/* clk_axi6: factor width 4, no src field, bypass clk_byp_0[20],
 * gate clk_en_2[2].  Initial factor 15; board plan: FPLL / 15 = 100 MHz. */
#define SG200X_LL_RCC_DIV_AXI6 SG200X_LL_RCC_DIV_OFF_AXI6, 4U
#define SG200X_LL_RCC_AXI6_MAX_FACTOR 15U
#define SG200X_LL_RCC_AXI6_INITIAL_FACTOR 15U

/* clk_1m: factor width 6, initial value 25 (25 MHz / 25), no src field,
 * gate clk_en_3[5]. */
#define SG200X_LL_RCC_DIV_1M SG200X_LL_RCC_DIV_OFF_1M, 6U
#define SG200X_LL_RCC_1M_INITIAL_FACTOR 25U

/* clk_spi: factor width 6, initial value 8, parents xtal/fpll selected by
 * bypass clk_byp_0[30], gate clk_en_3[6].  Board plan: FPLL / 8 =
 * 187.5 MHz. */
#define SG200X_LL_RCC_DIV_SPI SG200X_LL_RCC_DIV_OFF_SPI, 6U
#define SG200X_LL_RCC_SPI_INITIAL_FACTOR 8U

/* clk_i2c: factor width 4, initial value 1, parents xtal/axi6 selected by
 * bypass clk_byp_0[31], gate clk_en_3[7]. */
#define SG200X_LL_RCC_DIV_I2C SG200X_LL_RCC_DIV_OFF_I2C, 4U
#define SG200X_LL_RCC_I2C_INITIAL_FACTOR 1U

/* clk_pwm_src: factor width 6, initial value 10, src [9:8]
 * (0=fpll, 1=disppll), bypass clk_byp_0[15], gate clk_en_4[4]. */
#define SG200X_LL_RCC_DIV_PWM_SRC SG200X_LL_RCC_DIV_OFF_PWM_SRC, 6U
#define SG200X_LL_RCC_PWM_SRC_INITIAL_FACTOR 10U

/* ------------------------------------------------------------------ */
/* RSTGEN: soft reset controller (TRM Tables 7.1..7.5)                 */
/*                                                                     */
/* Bits are ACTIVE LOW and NOT self-clearing: write 0 to assert,       */
/* write 1 to release.  TRM 7.3: the target's bus and built-in DMA     */
/* must be idle before asserting or the system may hang.               */
/* ------------------------------------------------------------------ */

#define SG200X_LL_RCC_SOFT_RSTN_REG(n) SG200X_LL_REG32_AT(SG200X_RSTGEN_BASE, 0x000UL + 4UL * (n))
#define SG200X_LL_RCC_SOFT_CPUAC_RSTN_REG SG200X_LL_REG32_AT(SG200X_RSTGEN_BASE, 0x020UL)
#define SG200X_LL_RCC_SOFT_CPU_RSTN_REG SG200X_LL_REG32_AT(SG200X_RSTGEN_BASE, 0x024UL)

/* C906L-relevant reset targets; (SOFT_RSTN index, bit). */
typedef enum
{
    SG200X_LL_RESET_NONE = 0,
    SG200X_LL_RESET_SDMA,    /* rstn0[18] */
    SG200X_LL_RESET_I2S0,    /* rstn0[19] */
    SG200X_LL_RESET_I2S1,    /* rstn0[20] */
    SG200X_LL_RESET_I2S2,    /* rstn0[21] */
    SG200X_LL_RESET_I2S3,    /* rstn0[22] */
    SG200X_LL_RESET_UART0,   /* rstn0[23] */
    SG200X_LL_RESET_UART1,   /* rstn0[24] */
    SG200X_LL_RESET_UART2,   /* rstn0[25] */
    SG200X_LL_RESET_UART3,   /* rstn0[26] */
    SG200X_LL_RESET_I2C0,    /* rstn0[27] */
    SG200X_LL_RESET_I2C1,    /* rstn0[28] */
    SG200X_LL_RESET_I2C2,    /* rstn0[29] */
    SG200X_LL_RESET_I2C3,    /* rstn0[30] */
    SG200X_LL_RESET_I2C4,    /* rstn0[31] */
    SG200X_LL_RESET_PWM0,    /* rstn1[0]  */
    SG200X_LL_RESET_PWM1,    /* rstn1[1]  */
    SG200X_LL_RESET_PWM2,    /* rstn1[2]  */
    SG200X_LL_RESET_PWM3,    /* rstn1[3]  */
    SG200X_LL_RESET_SPI0,    /* rstn1[8]  */
    SG200X_LL_RESET_SPI1,    /* rstn1[9]  */
    SG200X_LL_RESET_SPI2,    /* rstn1[10] */
    SG200X_LL_RESET_SPI3,    /* rstn1[11] */
    SG200X_LL_RESET_GPIO0,   /* rstn1[12] */
    SG200X_LL_RESET_GPIO1,   /* rstn1[13] */
    SG200X_LL_RESET_GPIO2,   /* rstn1[14] */
    SG200X_LL_RESET_EFUSE,   /* rstn1[15] */
    SG200X_LL_RESET_WDT0,    /* rstn1[16] */
    SG200X_LL_RESET_TEMPSEN, /* rstn1[19] */
    SG200X_LL_RESET_SARADC,  /* rstn1[20] */
    SG200X_LL_RESET_UART4,   /* rstn2[10] */
    SG200X_LL_RESET_GPIO3,   /* rstn2[11] */
    SG200X_LL_RESET_TIMER,   /* rstn2[13] timer block */
    SG200X_LL_RESET_TIMER0,  /* rstn2[14] */
    SG200X_LL_RESET_TIMER1,  /* rstn2[15] */
    SG200X_LL_RESET_TIMER2,  /* rstn2[16] */
    SG200X_LL_RESET_TIMER3,  /* rstn2[17] */
    SG200X_LL_RESET_TIMER4,  /* rstn2[18] */
    SG200X_LL_RESET_TIMER5,  /* rstn2[19] */
    SG200X_LL_RESET_TIMER6,  /* rstn2[20] */
    SG200X_LL_RESET_TIMER7,  /* rstn2[21] */
    SG200X_LL_RESET_WGN0,    /* rstn2[22] */
    SG200X_LL_RESET_WGN1,    /* rstn2[23] */
    SG200X_LL_RESET_WGN2,    /* rstn2[24] */
    SG200X_LL_RESET_KEYSCAN, /* rstn2[25] */
    SG200X_LL_RESET_WDT1,    /* rstn3[4]  */
    SG200X_LL_RESET_WDT2,    /* rstn3[5]  C906L-owned watchdog */
} sg200x_ll_rcc_reset_target_t;

typedef struct
{
    uint8_t reg_index;
    uint8_t bit;
} sg200x_ll_rcc_reset_loc_t;

static inline sg200x_ll_rcc_reset_loc_t sg200x_ll_rcc_reset_loc_get(sg200x_ll_rcc_reset_target_t target)
{
    sg200x_ll_rcc_reset_loc_t loc = {0U, 0U};
    switch (target)
    {
    case SG200X_LL_RESET_SDMA:
        loc.reg_index = 0U;
        loc.bit = 18U;
        break;
    case SG200X_LL_RESET_I2S0:
        loc.reg_index = 0U;
        loc.bit = 19U;
        break;
    case SG200X_LL_RESET_I2S1:
        loc.reg_index = 0U;
        loc.bit = 20U;
        break;
    case SG200X_LL_RESET_I2S2:
        loc.reg_index = 0U;
        loc.bit = 21U;
        break;
    case SG200X_LL_RESET_I2S3:
        loc.reg_index = 0U;
        loc.bit = 22U;
        break;
    case SG200X_LL_RESET_UART0:
        loc.reg_index = 0U;
        loc.bit = 23U;
        break;
    case SG200X_LL_RESET_UART1:
        loc.reg_index = 0U;
        loc.bit = 24U;
        break;
    case SG200X_LL_RESET_UART2:
        loc.reg_index = 0U;
        loc.bit = 25U;
        break;
    case SG200X_LL_RESET_UART3:
        loc.reg_index = 0U;
        loc.bit = 26U;
        break;
    case SG200X_LL_RESET_I2C0:
        loc.reg_index = 0U;
        loc.bit = 27U;
        break;
    case SG200X_LL_RESET_I2C1:
        loc.reg_index = 0U;
        loc.bit = 28U;
        break;
    case SG200X_LL_RESET_I2C2:
        loc.reg_index = 0U;
        loc.bit = 29U;
        break;
    case SG200X_LL_RESET_I2C3:
        loc.reg_index = 0U;
        loc.bit = 30U;
        break;
    case SG200X_LL_RESET_I2C4:
        loc.reg_index = 0U;
        loc.bit = 31U;
        break;
    case SG200X_LL_RESET_PWM0:
        loc.reg_index = 1U;
        loc.bit = 0U;
        break;
    case SG200X_LL_RESET_PWM1:
        loc.reg_index = 1U;
        loc.bit = 1U;
        break;
    case SG200X_LL_RESET_PWM2:
        loc.reg_index = 1U;
        loc.bit = 2U;
        break;
    case SG200X_LL_RESET_PWM3:
        loc.reg_index = 1U;
        loc.bit = 3U;
        break;
    case SG200X_LL_RESET_SPI0:
        loc.reg_index = 1U;
        loc.bit = 8U;
        break;
    case SG200X_LL_RESET_SPI1:
        loc.reg_index = 1U;
        loc.bit = 9U;
        break;
    case SG200X_LL_RESET_SPI2:
        loc.reg_index = 1U;
        loc.bit = 10U;
        break;
    case SG200X_LL_RESET_SPI3:
        loc.reg_index = 1U;
        loc.bit = 11U;
        break;
    case SG200X_LL_RESET_GPIO0:
        loc.reg_index = 1U;
        loc.bit = 12U;
        break;
    case SG200X_LL_RESET_GPIO1:
        loc.reg_index = 1U;
        loc.bit = 13U;
        break;
    case SG200X_LL_RESET_GPIO2:
        loc.reg_index = 1U;
        loc.bit = 14U;
        break;
    case SG200X_LL_RESET_EFUSE:
        loc.reg_index = 1U;
        loc.bit = 15U;
        break;
    case SG200X_LL_RESET_WDT0:
        loc.reg_index = 1U;
        loc.bit = 16U;
        break;
    case SG200X_LL_RESET_TEMPSEN:
        loc.reg_index = 1U;
        loc.bit = 19U;
        break;
    case SG200X_LL_RESET_SARADC:
        loc.reg_index = 1U;
        loc.bit = 20U;
        break;
    case SG200X_LL_RESET_UART4:
        loc.reg_index = 2U;
        loc.bit = 10U;
        break;
    case SG200X_LL_RESET_GPIO3:
        loc.reg_index = 2U;
        loc.bit = 11U;
        break;
    case SG200X_LL_RESET_TIMER:
        loc.reg_index = 2U;
        loc.bit = 13U;
        break;
    case SG200X_LL_RESET_TIMER0:
        loc.reg_index = 2U;
        loc.bit = 14U;
        break;
    case SG200X_LL_RESET_TIMER1:
        loc.reg_index = 2U;
        loc.bit = 15U;
        break;
    case SG200X_LL_RESET_TIMER2:
        loc.reg_index = 2U;
        loc.bit = 16U;
        break;
    case SG200X_LL_RESET_TIMER3:
        loc.reg_index = 2U;
        loc.bit = 17U;
        break;
    case SG200X_LL_RESET_TIMER4:
        loc.reg_index = 2U;
        loc.bit = 18U;
        break;
    case SG200X_LL_RESET_TIMER5:
        loc.reg_index = 2U;
        loc.bit = 19U;
        break;
    case SG200X_LL_RESET_TIMER6:
        loc.reg_index = 2U;
        loc.bit = 20U;
        break;
    case SG200X_LL_RESET_TIMER7:
        loc.reg_index = 2U;
        loc.bit = 21U;
        break;
    case SG200X_LL_RESET_WGN0:
        loc.reg_index = 2U;
        loc.bit = 22U;
        break;
    case SG200X_LL_RESET_WGN1:
        loc.reg_index = 2U;
        loc.bit = 23U;
        break;
    case SG200X_LL_RESET_WGN2:
        loc.reg_index = 2U;
        loc.bit = 24U;
        break;
    case SG200X_LL_RESET_KEYSCAN:
        loc.reg_index = 2U;
        loc.bit = 25U;
        break;
    case SG200X_LL_RESET_WDT1:
        loc.reg_index = 3U;
        loc.bit = 4U;
        break;
    case SG200X_LL_RESET_WDT2:
        loc.reg_index = 3U;
        loc.bit = 5U;
        break;
    case SG200X_LL_RESET_NONE:
    default:
        loc.reg_index = 0xFFU;
        loc.bit = 0xFFU;
        break;
    }
    return loc;
}

static inline bool sg200x_ll_rcc_reset_target_exists(sg200x_ll_rcc_reset_target_t target)
{
    sg200x_ll_rcc_reset_loc_t loc = sg200x_ll_rcc_reset_loc_get(target);
    return loc.reg_index != 0xFFU;
}

/* Assert the module soft reset (clears the active-low bit). */
static inline void sg200x_ll_rcc_reset_assert(sg200x_ll_rcc_reset_target_t target)
{
    sg200x_ll_rcc_reset_loc_t loc = sg200x_ll_rcc_reset_loc_get(target);
    SG200X_LL_CLEAR_BITS(SG200X_LL_RCC_SOFT_RSTN_REG(loc.reg_index), SG200X_LL_BIT(loc.bit));
}

/* Release the module soft reset (sets the active-low bit). */
static inline void sg200x_ll_rcc_reset_release(sg200x_ll_rcc_reset_target_t target)
{
    sg200x_ll_rcc_reset_loc_t loc = sg200x_ll_rcc_reset_loc_get(target);
    SG200X_LL_SET_BITS(SG200X_LL_RCC_SOFT_RSTN_REG(loc.reg_index), SG200X_LL_BIT(loc.bit));
}

static inline bool sg200x_ll_rcc_reset_is_released(sg200x_ll_rcc_reset_target_t target)
{
    sg200x_ll_rcc_reset_loc_t loc = sg200x_ll_rcc_reset_loc_get(target);
    return SG200X_LL_IS_BIT_SET(SG200X_LL_RCC_SOFT_RSTN_REG(loc.reg_index), SG200X_LL_BIT(loc.bit));
}

/* ------------------------------------------------------------------ */
/* Static self checks                                                  */
/* ------------------------------------------------------------------ */

SG200X_LL_STATIC_ASSERT(SG200X_RSTGEN_BASE == 0x03003000UL, "RSTGEN base moved");
SG200X_LL_STATIC_ASSERT(SG200X_CLKGEN_BASE == 0x03002000UL, "CLKGEN base moved");
