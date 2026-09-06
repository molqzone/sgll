/*
 * sg200x_ll_spi.h - SG2002 DesignWare SSI low-level API.
 *
 * The SG2002 SPI block is a Synopsys DesignWare APB SSI master.  This header
 * deliberately keeps the register names and field meanings visible: the
 * caller owns the transfer state, while sgll supplies only typed register
 * access and field encodings.
 *
 * References:
 *   - SG2002 TRM v1.02, section 21.3.7, Tables 21.72-21.98.
 *   - SG2002 SDK hal/cv181x/config/cv181x_pinlist_swconfig.h for the related
 *     SD1 pad functions (the pinmux table is outside the TRM).
 *
 * Important hardware contract:
 *   - CTRLR0, BAUDR, FIFO thresholds, interrupt and DMA controls must be
 *     programmed while SPIENR is 0.  The TRM initialization sequence starts
 *     by disabling SPI and ends by enabling it.
 *   - BAUDR is an even divisor in the inclusive range 2..65534. The value
 *     zero is also defined by the TRM and disables the serial output clock;
 *     it is kept separate from a usable transfer divisor below.
 *   - SG2002 exposes one external chip-select per SSI instance in the SDK
 *     board descriptions; SER bit 0 is the only documented select bit.
 *   - CTRLR0 SCPH is not an abstract edge number.  SCPH=0 captures on the
 *     first edge; SCPH=1 delays the first clock and captures on the second
 *     edge.  Therefore standard SPI mode 1 is CTRLR0 value 0x47 for an
 *     8-bit Motorola full-duplex transfer (the SG2002-to-ESP32-C5 wiring
 *     uses this setting).
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sg200x_ll_defs.h"
#include "sg200x_ll_utils.h"

/* ------------------------------------------------------------------ */
/* Register map                                                       */
/* ------------------------------------------------------------------ */

#define SG200X_LL_SPI_INSTANCE_STRIDE 0x10000UL
#define SG200X_LL_SPI_FIFO_DEPTH 8U
#define SG200X_LL_SPI_DR_COUNT 36U
#define SG200X_LL_SPI_CHIP_SELECT_COUNT 1U

#define SG200X_LL_SPI_CTRLR0_OFFSET 0x000UL
#define SG200X_LL_SPI_CTRLR1_OFFSET 0x004UL
#define SG200X_LL_SPI_SPIENR_OFFSET 0x008UL
#define SG200X_LL_SPI_MWCR_OFFSET 0x00CUL
#define SG200X_LL_SPI_SER_OFFSET 0x010UL
#define SG200X_LL_SPI_BAUDR_OFFSET 0x014UL
#define SG200X_LL_SPI_TXFTLR_OFFSET 0x018UL
#define SG200X_LL_SPI_RXFTLR_OFFSET 0x01CUL
#define SG200X_LL_SPI_TXFLR_OFFSET 0x020UL
#define SG200X_LL_SPI_RXFLR_OFFSET 0x024UL
#define SG200X_LL_SPI_SR_OFFSET 0x028UL
#define SG200X_LL_SPI_IMR_OFFSET 0x02CUL
#define SG200X_LL_SPI_ISR_OFFSET 0x030UL
#define SG200X_LL_SPI_RISR_OFFSET 0x034UL
#define SG200X_LL_SPI_TXOICR_OFFSET 0x038UL
#define SG200X_LL_SPI_RXOICR_OFFSET 0x03CUL
#define SG200X_LL_SPI_RXUICR_OFFSET 0x040UL
#define SG200X_LL_SPI_MSTICR_OFFSET 0x044UL
#define SG200X_LL_SPI_ICR_OFFSET 0x048UL
#define SG200X_LL_SPI_DMACR_OFFSET 0x04CUL
#define SG200X_LL_SPI_DMATDLR_OFFSET 0x050UL
#define SG200X_LL_SPI_DMARDLR_OFFSET 0x054UL
#define SG200X_LL_SPI_DR_OFFSET 0x060UL
#define SG200X_LL_SPI_RX_SAMPLE_DLY_OFFSET 0x0F0UL

typedef struct
{
    volatile uint32_t CTRLR0;       /* 0x000, Table 21.73 */
    volatile uint32_t CTRLR1;       /* 0x004, Table 21.76 */
    volatile uint32_t SPIENR;       /* 0x008, Table 21.77 */
    volatile uint32_t MWCR;         /* 0x00C, Table 21.78 */
    volatile uint32_t SER;          /* 0x010, Table 21.79 */
    volatile uint32_t BAUDR;        /* 0x014, Table 21.80 */
    volatile uint32_t TXFTLR;        /* 0x018, Table 21.81 */
    volatile uint32_t RXFTLR;        /* 0x01C, Table 21.82 */
    volatile uint32_t TXFLR;         /* 0x020, Table 21.83 */
    volatile uint32_t RXFLR;         /* 0x024, Table 21.84 */
    volatile uint32_t SR;            /* 0x028, Table 21.85 */
    volatile uint32_t IMR;           /* 0x02C, Table 21.86 */
    volatile uint32_t ISR;           /* 0x030, Table 21.87 */
    volatile uint32_t RISR;          /* 0x034, Table 21.88 */
    volatile uint32_t TXOICR;        /* 0x038, Table 21.89 */
    volatile uint32_t RXOICR;        /* 0x03C, Table 21.90 */
    volatile uint32_t RXUICR;        /* 0x040, Table 21.91 */
    volatile uint32_t MSTICR;        /* 0x044, Table 21.92 */
    volatile uint32_t ICR;           /* 0x048, Table 21.93 */
    volatile uint32_t DMACR;         /* 0x04C, Table 21.94 */
    volatile uint32_t DMATDLR;       /* 0x050, Table 21.95 */
    volatile uint32_t DMARDLR;       /* 0x054, Table 21.96 */
    uint32_t _reserved0[2];           /* 0x058-0x05C */
    volatile uint32_t DR[SG200X_LL_SPI_DR_COUNT]; /* 0x060, Table 21.97 */
    volatile uint32_t RX_SAMPLE_DLY; /* 0x0F0, Table 21.98 */
} sg200x_ll_spi_t;

SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, CTRLR0) == SG200X_LL_SPI_CTRLR0_OFFSET,
                        "SG200X SPI CTRLR0 offset changed");
SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, CTRLR1) == SG200X_LL_SPI_CTRLR1_OFFSET,
                        "SG200X SPI CTRLR1 offset changed");
SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, SPIENR) == SG200X_LL_SPI_SPIENR_OFFSET,
                        "SG200X SPI SPIENR offset changed");
SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, SER) == SG200X_LL_SPI_SER_OFFSET,
                        "SG200X SPI SER offset changed");
SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, BAUDR) == SG200X_LL_SPI_BAUDR_OFFSET,
                        "SG200X SPI BAUDR offset changed");
SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, TXFLR) == SG200X_LL_SPI_TXFLR_OFFSET,
                        "SG200X SPI TXFLR offset changed");
SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, SR) == SG200X_LL_SPI_SR_OFFSET,
                        "SG200X SPI SR offset changed");
SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, DMACR) == SG200X_LL_SPI_DMACR_OFFSET,
                        "SG200X SPI DMACR offset changed");
SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, DR) == SG200X_LL_SPI_DR_OFFSET,
                        "SG200X SPI DR offset changed");
SG200X_LL_STATIC_ASSERT(offsetof(sg200x_ll_spi_t, RX_SAMPLE_DLY) == SG200X_LL_SPI_RX_SAMPLE_DLY_OFFSET,
                        "SG200X SPI RX_SAMPLE_DLY offset changed");
SG200X_LL_STATIC_ASSERT(sizeof(sg200x_ll_spi_t) == SG200X_LL_SPI_RX_SAMPLE_DLY_OFFSET + sizeof(uint32_t),
                        "SG200X SPI register block layout changed");

#define SG200X_SPI0 ((sg200x_ll_spi_t *)(uintptr_t)SG200X_SPI0_BASE)
#define SG200X_SPI1 ((sg200x_ll_spi_t *)(uintptr_t)SG200X_SPI1_BASE)
#define SG200X_SPI2 ((sg200x_ll_spi_t *)(uintptr_t)SG200X_SPI2_BASE)
#define SG200X_SPI3 ((sg200x_ll_spi_t *)(uintptr_t)SG200X_SPI3_BASE)

static inline sg200x_ll_spi_t *sg200x_ll_spi_get(uint32_t index)
{
    switch (index)
    {
    case 0U:
        return SG200X_SPI0;
    case 1U:
        return SG200X_SPI1;
    case 2U:
        return SG200X_SPI2;
    case 3U:
        return SG200X_SPI3;
    default:
        return (sg200x_ll_spi_t *)0;
    }
}

static inline uintptr_t sg200x_ll_spi_reg_address(const sg200x_ll_spi_t *spi,
                                                  uint32_t offset)
{
    return (uintptr_t)spi + (uintptr_t)offset;
}

static inline uintptr_t sg200x_ll_spi_data_address(const sg200x_ll_spi_t *spi)
{
    return sg200x_ll_spi_reg_address(spi, SG200X_LL_SPI_DR_OFFSET);
}

/* ------------------------------------------------------------------ */
/* CTRLR0 semantic fields                                             */
/* ------------------------------------------------------------------ */

#define SG200X_LL_SPI_CTRLR0_CFS_SHIFT 12U
#define SG200X_LL_SPI_CTRLR0_CFS_WIDTH 4U
#define SG200X_LL_SPI_CTRLR0_CFS_MASK (0x0FUL << SG200X_LL_SPI_CTRLR0_CFS_SHIFT)
#define SG200X_LL_SPI_CTRLR0_SRL_BIT SG200X_LL_BIT(11)
#define SG200X_LL_SPI_CTRLR0_TMOD_SHIFT 8U
#define SG200X_LL_SPI_CTRLR0_TMOD_WIDTH 2U
#define SG200X_LL_SPI_CTRLR0_TMOD_MASK (0x03UL << SG200X_LL_SPI_CTRLR0_TMOD_SHIFT)
#define SG200X_LL_SPI_CTRLR0_SCPOL_BIT SG200X_LL_BIT(7)
#define SG200X_LL_SPI_CTRLR0_SCPH_BIT SG200X_LL_BIT(6)
#define SG200X_LL_SPI_CTRLR0_FRF_SHIFT 4U
#define SG200X_LL_SPI_CTRLR0_FRF_WIDTH 2U
#define SG200X_LL_SPI_CTRLR0_FRF_MASK (0x03UL << SG200X_LL_SPI_CTRLR0_FRF_SHIFT)
#define SG200X_LL_SPI_CTRLR0_DFS_SHIFT 0U
#define SG200X_LL_SPI_CTRLR0_DFS_WIDTH 4U
#define SG200X_LL_SPI_CTRLR0_DFS_MASK 0x0FUL

typedef enum
{
    /* CTRLR0.CFS encoding: value is control-word length minus one. */
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_1_BIT = 0U,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_2_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_3_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_4_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_5_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_6_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_7_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_8_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_9_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_10_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_11_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_12_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_13_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_14_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_15_BIT,
    SG200X_LL_SPI_CONTROL_FRAME_SIZE_16_BIT,
} sg200x_ll_spi_control_frame_size_t;

typedef enum
{
    SG200X_LL_SPI_TRANSFER_MODE_TX_RX = 0U,
    SG200X_LL_SPI_TRANSFER_MODE_TX_ONLY = 1U,
    SG200X_LL_SPI_TRANSFER_MODE_RX_ONLY = 2U,
    SG200X_LL_SPI_TRANSFER_MODE_EEPROM_READ = 3U,
} sg200x_ll_spi_transfer_mode_t;

typedef enum
{
    SG200X_LL_SPI_CLOCK_POLARITY_LOW = 0U,
    SG200X_LL_SPI_CLOCK_POLARITY_HIGH = 1U,
} sg200x_ll_spi_clock_polarity_t;

typedef enum
{
    /* FIRST_EDGE means SCPH=0; SECOND_EDGE means SCPH=1. */
    SG200X_LL_SPI_CLOCK_PHASE_FIRST_EDGE = 0U,
    SG200X_LL_SPI_CLOCK_PHASE_SECOND_EDGE = 1U,
} sg200x_ll_spi_clock_phase_t;

typedef enum
{
    SG200X_LL_SPI_FRAME_FORMAT_MOTOROLA = 0U,
    SG200X_LL_SPI_FRAME_FORMAT_TI_SSP = 1U,
    SG200X_LL_SPI_FRAME_FORMAT_NS_MICROWIRE = 2U,
} sg200x_ll_spi_frame_format_t;

typedef enum
{
    /* CTRLR0.DFS encoding: 4-bit data is 0b0011. */
    SG200X_LL_SPI_DATA_FRAME_SIZE_4_BIT = 3U,
    SG200X_LL_SPI_DATA_FRAME_SIZE_5_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_6_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_7_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_8_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_9_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_10_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_11_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_12_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_13_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_14_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_15_BIT,
    SG200X_LL_SPI_DATA_FRAME_SIZE_16_BIT,
} sg200x_ll_spi_data_frame_size_t;

typedef enum
{
    SG200X_LL_SPI_MODE_0 = 0U, /* CPOL=0, SCPH=0 */
    SG200X_LL_SPI_MODE_1 = 1U, /* CPOL=0, SCPH=1; CTRLR0 8-bit = 0x47 */
    SG200X_LL_SPI_MODE_2 = 2U, /* CPOL=1, SCPH=0 */
    SG200X_LL_SPI_MODE_3 = 3U, /* CPOL=1, SCPH=1 */
} sg200x_ll_spi_mode_t;

static inline uint32_t sg200x_ll_spi_ctrlr0_build(
    sg200x_ll_spi_control_frame_size_t control_frame_size,
    bool shift_register_loop,
    sg200x_ll_spi_transfer_mode_t transfer_mode,
    sg200x_ll_spi_clock_polarity_t clock_polarity,
    sg200x_ll_spi_clock_phase_t clock_phase,
    sg200x_ll_spi_frame_format_t frame_format,
    sg200x_ll_spi_data_frame_size_t data_frame_size)
{
    uint32_t value = ((uint32_t)control_frame_size << SG200X_LL_SPI_CTRLR0_CFS_SHIFT) &
                     SG200X_LL_SPI_CTRLR0_CFS_MASK;
    value |= ((uint32_t)transfer_mode << SG200X_LL_SPI_CTRLR0_TMOD_SHIFT) &
             SG200X_LL_SPI_CTRLR0_TMOD_MASK;
    value |= ((uint32_t)frame_format << SG200X_LL_SPI_CTRLR0_FRF_SHIFT) &
             SG200X_LL_SPI_CTRLR0_FRF_MASK;
    value |= (uint32_t)data_frame_size & SG200X_LL_SPI_CTRLR0_DFS_MASK;
    if (shift_register_loop)
    {
        value |= SG200X_LL_SPI_CTRLR0_SRL_BIT;
    }
    if (clock_polarity == SG200X_LL_SPI_CLOCK_POLARITY_HIGH)
    {
        value |= SG200X_LL_SPI_CTRLR0_SCPOL_BIT;
    }
    if (clock_phase == SG200X_LL_SPI_CLOCK_PHASE_SECOND_EDGE)
    {
        value |= SG200X_LL_SPI_CTRLR0_SCPH_BIT;
    }
    return value;
}

static inline uint32_t sg200x_ll_spi_ctrlr0_build_motorola_8bit(
    sg200x_ll_spi_mode_t mode, bool shift_register_loop)
{
    const sg200x_ll_spi_clock_polarity_t polarity =
        (mode & 2U) != 0U ? SG200X_LL_SPI_CLOCK_POLARITY_HIGH
                           : SG200X_LL_SPI_CLOCK_POLARITY_LOW;
    const sg200x_ll_spi_clock_phase_t phase =
        (mode & 1U) != 0U ? SG200X_LL_SPI_CLOCK_PHASE_SECOND_EDGE
                           : SG200X_LL_SPI_CLOCK_PHASE_FIRST_EDGE;
    return sg200x_ll_spi_ctrlr0_build(
        SG200X_LL_SPI_CONTROL_FRAME_SIZE_1_BIT, shift_register_loop,
        SG200X_LL_SPI_TRANSFER_MODE_TX_RX, polarity, phase,
        SG200X_LL_SPI_FRAME_FORMAT_MOTOROLA,
        SG200X_LL_SPI_DATA_FRAME_SIZE_8_BIT);
}

static inline sg200x_ll_spi_mode_t sg200x_ll_spi_mode_get(uint32_t ctrlr0)
{
    return (sg200x_ll_spi_mode_t)(((ctrlr0 & SG200X_LL_SPI_CTRLR0_SCPOL_BIT) != 0U ? 2U : 0U) |
                                  ((ctrlr0 & SG200X_LL_SPI_CTRLR0_SCPH_BIT) != 0U ? 1U : 0U));
}

static inline void sg200x_ll_spi_ctrlr0_set(sg200x_ll_spi_t *spi, uint32_t value)
{
    /* CTRLR0 is a shadow/configuration register; SPI must be disabled. */
    spi->CTRLR0 = value & 0xFFFFUL;
}

static inline uint32_t sg200x_ll_spi_ctrlr0_get(const sg200x_ll_spi_t *spi)
{
    return spi->CTRLR0 & 0xFFFFUL;
}

/* ------------------------------------------------------------------ */
/* Enable, chip select and frame count                                */
/* ------------------------------------------------------------------ */

#define SG200X_LL_SPI_SPIENR_ENABLE_BIT SG200X_LL_BIT(0)
#define SG200X_LL_SPI_SER_CS0_BIT SG200X_LL_BIT(0)
#define SG200X_LL_SPI_SER_MASK SG200X_LL_SPI_SER_CS0_BIT
#define SG200X_LL_SPI_CTRLR1_NDF_MASK 0xFFFFUL

static inline void sg200x_ll_spi_enable(sg200x_ll_spi_t *spi)
{
    spi->SPIENR = SG200X_LL_SPI_SPIENR_ENABLE_BIT;
}

static inline void sg200x_ll_spi_disable(sg200x_ll_spi_t *spi)
{
    spi->SPIENR = 0U;
}

static inline bool sg200x_ll_spi_is_enabled(const sg200x_ll_spi_t *spi)
{
    return (spi->SPIENR & SG200X_LL_SPI_SPIENR_ENABLE_BIT) != 0U;
}

static inline void sg200x_ll_spi_slave_select_set(sg200x_ll_spi_t *spi, uint32_t mask)
{
    spi->SER = mask & SG200X_LL_SPI_SER_MASK;
}

static inline void sg200x_ll_spi_slave_select_enable(sg200x_ll_spi_t *spi)
{
    sg200x_ll_spi_slave_select_set(spi, SG200X_LL_SPI_SER_CS0_BIT);
}

static inline void sg200x_ll_spi_slave_select_disable(sg200x_ll_spi_t *spi)
{
    spi->SER = 0U;
}

static inline bool sg200x_ll_spi_slave_is_selected(const sg200x_ll_spi_t *spi)
{
    return (spi->SER & SG200X_LL_SPI_SER_CS0_BIT) != 0U;
}

static inline bool sg200x_ll_spi_data_frame_count_set(sg200x_ll_spi_t *spi,
                                                      uint32_t count)
{
    /* CTRLR1.NDF stores count - 1 and is used by receive-only modes. */
    if (count == 0U || count > 65536UL)
    {
        return false;
    }
    spi->CTRLR1 = (count - 1U) & SG200X_LL_SPI_CTRLR1_NDF_MASK;
    return true;
}

static inline uint32_t sg200x_ll_spi_data_frame_count_get(const sg200x_ll_spi_t *spi)
{
    return (spi->CTRLR1 & SG200X_LL_SPI_CTRLR1_NDF_MASK) + 1U;
}

/* ------------------------------------------------------------------ */
/* Clock divider and FIFO thresholds                                  */
/* ------------------------------------------------------------------ */

#define SG200X_LL_SPI_BAUDR_MIN 2U
#define SG200X_LL_SPI_BAUDR_MAX 65534U
#define SG200X_LL_SPI_BAUDR_CLOCK_DISABLED 0U
#define SG200X_LL_SPI_BAUDR_MASK 0xFFFFUL
#define SG200X_LL_SPI_FIFO_LEVEL_MASK 0x0FUL
#define SG200X_LL_SPI_FIFO_THRESHOLD_MASK 0x07UL
#define SG200X_LL_SPI_FIFO_THRESHOLD_MAX SG200X_LL_SPI_FIFO_THRESHOLD_MASK
#define SG200X_LL_SPI_DMA_LEVEL_MASK 0x07UL

static inline bool sg200x_ll_spi_baud_divider_is_valid(uint32_t divider)
{
    return divider == SG200X_LL_SPI_BAUDR_CLOCK_DISABLED ||
           (divider >= SG200X_LL_SPI_BAUDR_MIN && divider <= SG200X_LL_SPI_BAUDR_MAX &&
            (divider & 1U) == 0U);
}

static inline bool sg200x_ll_spi_baud_divider_set(sg200x_ll_spi_t *spi, uint32_t divider)
{
    if (!sg200x_ll_spi_baud_divider_is_valid(divider))
    {
        return false;
    }
    spi->BAUDR = divider & SG200X_LL_SPI_BAUDR_MASK;
    return true;
}

static inline uint32_t sg200x_ll_spi_baud_divider_get(const sg200x_ll_spi_t *spi)
{
    return spi->BAUDR & SG200X_LL_SPI_BAUDR_MASK;
}

static inline bool sg200x_ll_spi_tx_fifo_threshold_set(sg200x_ll_spi_t *spi, uint32_t level)
{
    if (level > SG200X_LL_SPI_FIFO_THRESHOLD_MAX)
    {
        return false;
    }
    spi->TXFTLR = level & SG200X_LL_SPI_FIFO_THRESHOLD_MASK;
    return true;
}

static inline bool sg200x_ll_spi_rx_fifo_threshold_set(sg200x_ll_spi_t *spi, uint32_t level)
{
    /* The TRM text says "greater than depth", but this field is only [2:0]. */
    if (level > SG200X_LL_SPI_FIFO_THRESHOLD_MAX)
    {
        return false;
    }
    spi->RXFTLR = level & SG200X_LL_SPI_FIFO_THRESHOLD_MASK;
    return true;
}

static inline bool sg200x_ll_spi_rx_fifo_trigger_level_set(sg200x_ll_spi_t *spi,
                                                           uint32_t entries)
{
    /* RXFTLR triggers at RXFTLR + 1 entries; express that contract directly. */
    if (entries == 0U || entries > SG200X_LL_SPI_FIFO_DEPTH)
    {
        return false;
    }
    return sg200x_ll_spi_rx_fifo_threshold_set(spi, entries - 1U);
}

static inline uint32_t sg200x_ll_spi_tx_fifo_level_get(const sg200x_ll_spi_t *spi)
{
    return spi->TXFLR & SG200X_LL_SPI_FIFO_LEVEL_MASK;
}

static inline uint32_t sg200x_ll_spi_rx_fifo_level_get(const sg200x_ll_spi_t *spi)
{
    return spi->RXFLR & SG200X_LL_SPI_FIFO_LEVEL_MASK;
}

/* ------------------------------------------------------------------ */
/* Status and interrupt semantics                                     */
/* ------------------------------------------------------------------ */

#define SG200X_LL_SPI_SR_BUSY_BIT SG200X_LL_BIT(0)
#define SG200X_LL_SPI_SR_TFNF_BIT SG200X_LL_BIT(1)
#define SG200X_LL_SPI_SR_TFE_BIT SG200X_LL_BIT(2)
#define SG200X_LL_SPI_SR_RFNE_BIT SG200X_LL_BIT(3)
#define SG200X_LL_SPI_SR_RFF_BIT SG200X_LL_BIT(4)
#define SG200X_LL_SPI_SR_TX_ERROR_BIT SG200X_LL_BIT(5)
#define SG200X_LL_SPI_SR_COLLISION_ERROR_BIT SG200X_LL_BIT(6)
#define SG200X_LL_SPI_SR_ERROR_MASK \
    (SG200X_LL_SPI_SR_TX_ERROR_BIT | SG200X_LL_SPI_SR_COLLISION_ERROR_BIT)

#define SG200X_LL_SPI_INTERRUPT_TX_EMPTY_BIT SG200X_LL_BIT(0)
#define SG200X_LL_SPI_INTERRUPT_TX_OVERFLOW_BIT SG200X_LL_BIT(1)
#define SG200X_LL_SPI_INTERRUPT_RX_UNDERFLOW_BIT SG200X_LL_BIT(2)
#define SG200X_LL_SPI_INTERRUPT_RX_OVERFLOW_BIT SG200X_LL_BIT(3)
#define SG200X_LL_SPI_INTERRUPT_RX_FULL_BIT SG200X_LL_BIT(4)
#define SG200X_LL_SPI_INTERRUPT_MULTI_MASTER_BIT SG200X_LL_BIT(5)
#define SG200X_LL_SPI_INTERRUPT_MASK 0x3FUL
#define SG200X_LL_SPI_RISR_ERROR_MASK \
    (SG200X_LL_SPI_INTERRUPT_TX_OVERFLOW_BIT | SG200X_LL_SPI_INTERRUPT_RX_UNDERFLOW_BIT | \
     SG200X_LL_SPI_INTERRUPT_RX_OVERFLOW_BIT | SG200X_LL_SPI_INTERRUPT_MULTI_MASTER_BIT)

static inline uint32_t sg200x_ll_spi_status_get(const sg200x_ll_spi_t *spi)
{
    /* SR[5:6] are clear-on-read according to TRM Table 21.85. */
    return spi->SR;
}

static inline bool sg200x_ll_spi_is_busy(const sg200x_ll_spi_t *spi)
{
    return (sg200x_ll_spi_status_get(spi) & SG200X_LL_SPI_SR_BUSY_BIT) != 0U;
}

static inline bool sg200x_ll_spi_tx_fifo_not_full(const sg200x_ll_spi_t *spi)
{
    return (sg200x_ll_spi_status_get(spi) & SG200X_LL_SPI_SR_TFNF_BIT) != 0U;
}

static inline bool sg200x_ll_spi_rx_fifo_not_empty(const sg200x_ll_spi_t *spi)
{
    return (sg200x_ll_spi_status_get(spi) & SG200X_LL_SPI_SR_RFNE_BIT) != 0U;
}

static inline uint32_t sg200x_ll_spi_interrupt_mask_get(const sg200x_ll_spi_t *spi)
{
    return spi->IMR & SG200X_LL_SPI_INTERRUPT_MASK;
}

static inline void sg200x_ll_spi_interrupt_mask_set(sg200x_ll_spi_t *spi, uint32_t mask)
{
    spi->IMR = mask & SG200X_LL_SPI_INTERRUPT_MASK;
}

static inline uint32_t sg200x_ll_spi_interrupt_status_get(const sg200x_ll_spi_t *spi)
{
    return spi->ISR & SG200X_LL_SPI_INTERRUPT_MASK;
}

static inline uint32_t sg200x_ll_spi_raw_interrupt_status_get(const sg200x_ll_spi_t *spi)
{
    return spi->RISR & SG200X_LL_SPI_INTERRUPT_MASK;
}

static inline uint32_t sg200x_ll_spi_interrupt_clear(sg200x_ll_spi_t *spi)
{
    /* ICR is a read-to-clear register; writes have no effect. */
    return spi->ICR;
}

static inline uint32_t sg200x_ll_spi_tx_overflow_clear(sg200x_ll_spi_t *spi)
{
    return spi->TXOICR;
}

static inline uint32_t sg200x_ll_spi_rx_overflow_clear(sg200x_ll_spi_t *spi)
{
    return spi->RXOICR;
}

static inline uint32_t sg200x_ll_spi_rx_underflow_clear(sg200x_ll_spi_t *spi)
{
    return spi->RXUICR;
}

static inline uint32_t sg200x_ll_spi_multi_master_clear(sg200x_ll_spi_t *spi)
{
    return spi->MSTICR;
}

/* ------------------------------------------------------------------ */
/* DMA, data and sample delay                                         */
/* ------------------------------------------------------------------ */

#define SG200X_LL_SPI_DMACR_RX_ENABLE_BIT SG200X_LL_BIT(0)
#define SG200X_LL_SPI_DMACR_TX_ENABLE_BIT SG200X_LL_BIT(1)
#define SG200X_LL_SPI_DMACR_MASK 0x03UL
#define SG200X_LL_SPI_DMATDLR_MASK SG200X_LL_SPI_DMA_LEVEL_MASK
#define SG200X_LL_SPI_DMARDLR_MASK SG200X_LL_SPI_DMA_LEVEL_MASK
#define SG200X_LL_SPI_DR_MASK 0xFFFFUL
#define SG200X_LL_SPI_RX_SAMPLE_DLY_MASK 0xFFUL
#define SG200X_LL_SPI_RX_SAMPLE_DLY_MAX 7U

static inline void sg200x_ll_spi_dma_control_set(sg200x_ll_spi_t *spi, uint32_t mask)
{
    spi->DMACR = mask & SG200X_LL_SPI_DMACR_MASK;
}

static inline void sg200x_ll_spi_dma_disable(sg200x_ll_spi_t *spi)
{
    sg200x_ll_spi_dma_control_set(spi, 0U);
}

static inline void sg200x_ll_spi_dma_enable(sg200x_ll_spi_t *spi, bool receive, bool transmit)
{
    uint32_t mask = 0U;
    if (receive)
    {
        mask |= SG200X_LL_SPI_DMACR_RX_ENABLE_BIT;
    }
    if (transmit)
    {
        mask |= SG200X_LL_SPI_DMACR_TX_ENABLE_BIT;
    }
    sg200x_ll_spi_dma_control_set(spi, mask);
}

static inline bool sg200x_ll_spi_dma_tx_level_set(sg200x_ll_spi_t *spi, uint32_t level)
{
    if (level >= SG200X_LL_SPI_FIFO_DEPTH)
    {
        return false;
    }
    spi->DMATDLR = level & SG200X_LL_SPI_DMATDLR_MASK;
    return true;
}

static inline bool sg200x_ll_spi_dma_rx_level_set(sg200x_ll_spi_t *spi, uint32_t level)
{
    if (level >= SG200X_LL_SPI_FIFO_DEPTH)
    {
        return false;
    }
    spi->DMARDLR = level & SG200X_LL_SPI_DMARDLR_MASK;
    return true;
}

static inline void sg200x_ll_spi_data_write(sg200x_ll_spi_t *spi, uint16_t value)
{
    /* DR occupies 36 aliases so AHB bursts can target the same FIFO port. */
    spi->DR[0] = (uint32_t)value & SG200X_LL_SPI_DR_MASK;
}

static inline uint16_t sg200x_ll_spi_data_read(sg200x_ll_spi_t *spi)
{
    return (uint16_t)(spi->DR[0] & SG200X_LL_SPI_DR_MASK);
}

static inline bool sg200x_ll_spi_rx_sample_delay_set(sg200x_ll_spi_t *spi, uint32_t delay)
{
    if (delay > SG200X_LL_SPI_RX_SAMPLE_DLY_MAX)
    {
        /* Values above the internal depth silently select zero delay. */
        return false;
    }
    spi->RX_SAMPLE_DLY = delay & SG200X_LL_SPI_RX_SAMPLE_DLY_MASK;
    return true;
}

static inline uint32_t sg200x_ll_spi_rx_sample_delay_get(const sg200x_ll_spi_t *spi)
{
    return spi->RX_SAMPLE_DLY & SG200X_LL_SPI_RX_SAMPLE_DLY_MASK;
}

/* ------------------------------------------------------------------ */
/* MWCR (only meaningful for NS Microwire)                            */
/* ------------------------------------------------------------------ */

#define SG200X_LL_SPI_MWCR_TRANSFER_MODE_BIT SG200X_LL_BIT(0)
#define SG200X_LL_SPI_MWCR_CONTROL_BIT SG200X_LL_BIT(1)
#define SG200X_LL_SPI_MWCR_HANDSHAKE_BIT SG200X_LL_BIT(2)
#define SG200X_LL_SPI_MWCR_MASK 0x07UL

static inline void sg200x_ll_spi_microwire_control_set(sg200x_ll_spi_t *spi, uint32_t value)
{
    spi->MWCR = value & SG200X_LL_SPI_MWCR_MASK;
}

static inline uint32_t sg200x_ll_spi_microwire_control_get(const sg200x_ll_spi_t *spi)
{
    return spi->MWCR & SG200X_LL_SPI_MWCR_MASK;
}
