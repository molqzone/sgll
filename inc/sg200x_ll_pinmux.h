/*
 * sg200x_ll_pinmux.h - SG2002 pad function-select primitives.
 *
 * The SG2002 TRM section 10 intentionally points to the online pinout
 * workbook instead of listing the function table.  The offsets and function
 * values below therefore come from the SG2002 SDK's
 * cv181x_pinlist_swconfig.h/cv181x_reg_fmux_gpio.h and were checked against
 * the LicheeRV Nano board.  Keep the source distinction visible when adding
 * more pads.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sg200x_ll_defs.h"
#include "sg200x_ll_utils.h"

#define SG200X_LL_PINMUX_FUNCTION_MASK 0x07UL

/* FUNCSEL offsets from SG200X_PINMUX_BASE. */
#define SG200X_LL_PINMUX_EMMC_DAT2_OFFSET 0x04CUL /* EMMC_DAT2 / GPIOA26 */
#define SG200X_LL_PINMUX_SD1_D3_OFFSET 0x0D0UL
#define SG200X_LL_PINMUX_SD1_D2_OFFSET 0x0D4UL
#define SG200X_LL_PINMUX_SD1_D1_OFFSET 0x0D8UL
#define SG200X_LL_PINMUX_SD1_D0_OFFSET 0x0DCUL
#define SG200X_LL_PINMUX_SD1_CMD_OFFSET 0x0E0UL
#define SG200X_LL_PINMUX_SD1_CLK_OFFSET 0x0E4UL

/* Function numbers from cv181x_pinlist_swconfig.h. */
#define SG200X_LL_PINMUX_EMMC_DAT2_GPIOA26_FUNCTION 3U
#define SG200X_LL_PINMUX_SD1_D3_SPI2_CS_FUNCTION 1U
#define SG200X_LL_PINMUX_SD1_D3_GPIO18_FUNCTION 3U
#define SG200X_LL_PINMUX_SD1_D0_SPI2_SDI_FUNCTION 1U
#define SG200X_LL_PINMUX_SD1_D0_GPIO21_FUNCTION 3U
#define SG200X_LL_PINMUX_SD1_CMD_SPI2_SDO_FUNCTION 1U
#define SG200X_LL_PINMUX_SD1_CMD_GPIO22_FUNCTION 3U
#define SG200X_LL_PINMUX_SD1_CLK_SPI2_SCK_FUNCTION 1U
#define SG200X_LL_PINMUX_SD1_CLK_GPIO23_FUNCTION 3U

/* GPIO bit positions used when the three SD1 pads are temporarily owned by
 * the C906L diagnostic GPIO path instead of the SSI peripheral. */
#define SG200X_LL_PINMUX_SD1_D0_GPIO_MASK SG200X_LL_BIT(21)
#define SG200X_LL_PINMUX_SD1_CMD_GPIO_MASK SG200X_LL_BIT(22)
#define SG200X_LL_PINMUX_SD1_CLK_GPIO_MASK SG200X_LL_BIT(23)
#define SG200X_LL_PINMUX_SD1_GPIO_PAD_MASK \
    (SG200X_LL_PINMUX_SD1_D0_GPIO_MASK | SG200X_LL_PINMUX_SD1_CMD_GPIO_MASK | \
     SG200X_LL_PINMUX_SD1_CLK_GPIO_MASK)

/* TOP_MISC+0x294 bit 10 selects the SD1 pad bank on this board. */
#define SG200X_LL_PINMUX_TOP_SD1_SELECT_OFFSET 0x294UL
#define SG200X_LL_PINMUX_TOP_SD1_MIPI_SELECT_BIT SG200X_LL_BIT(10)

static inline bool sg200x_ll_pinmux_function_is_valid(uint32_t function)
{
    return function <= SG200X_LL_PINMUX_FUNCTION_MASK;
}

static inline bool sg200x_ll_pinmux_function_set(uint32_t offset, uint32_t function)
{
    if (!sg200x_ll_pinmux_function_is_valid(function))
    {
        return false;
    }
    volatile uint32_t *reg = &SG200X_LL_REG32_AT(SG200X_PINMUX_BASE, offset);
    *reg = (*reg & ~SG200X_LL_PINMUX_FUNCTION_MASK) | function;
    return true;
}

static inline uint32_t sg200x_ll_pinmux_function_get(uint32_t offset)
{
    return SG200X_LL_REG32_AT(SG200X_PINMUX_BASE, offset) & SG200X_LL_PINMUX_FUNCTION_MASK;
}

static inline void sg200x_ll_pinmux_select_sd1_pad_bank(void)
{
    /* 0 selects SD1; 1 selects the alternate MIPI lane path. */
    SG200X_LL_CLEAR_BITS(
        SG200X_LL_REG32_AT(SG200X_TOP_MISC_BASE, SG200X_LL_PINMUX_TOP_SD1_SELECT_OFFSET),
        SG200X_LL_PINMUX_TOP_SD1_MIPI_SELECT_BIT);
}
