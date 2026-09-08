/**
 * @file registers.c
 * @brief 依据 TRM 验证宿主寄存器镜像。
 *        Validate host register images against the TRM.
 *
 * @note 匿名内存页代替 MMIO，覆盖 TRM 表 11.5、11.20、11.21-11.26 和 21.48。
 *       Anonymous pages replace MMIO for TRM Tables 11.5, 11.20, 11.21-11.26, and 21.48.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "sgll.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

static void map_registers(uintptr_t address, size_t size)
{
    void *mapping = mmap(
        (void *)address, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
        0
    );
    assert(mapping != MAP_FAILED && mapping == (void *)address);
}

static uint32_t word_at(size_t offset) { return *(volatile uint32_t *)((unsigned char *)DMA + offset); }

static void check_abort(void)
{
    /** CHEN 低字和偏移 0x028 的占位字必须保持不变。 The low CHEN word and the reserved word at 0x028 must be
     * preserved. */
    DMA->GLOBAL.CHEN = 0x0FU;
    *(volatile uint32_t *)((unsigned char *)DMA + 0x028U) = 0x12345678U;
    for (unsigned channel = 0; channel < 8; ++channel)
    {
        sgll_dma_channel_abort(channel);
        assert(word_at(0x01CU) == ((1U << channel) | (1U << (channel + 8U))));
        assert(word_at(0x018U) == 0x0FU);
        assert(word_at(0x028U) == 0x12345678U);
    }
    sgll_dma_channels_abort(0xF0U);
    assert(word_at(0x01CU) == 0xF0F0U);
    sgll_dma_channel_abort(8U);
    assert(word_at(0x01CU) == 0xF0F0U);
}

static void check_descriptors(void)
{
    DMA_LLI_Type lli[2];
    assert(alignof(DMA_LLI_Type) == 64U);
    assert(sizeof(DMA_LLI_Type) == 64U);
    assert(((uintptr_t)&lli[0] & 63U) == 0U);
    assert(((uintptr_t)&lli[1] & 63U) == 0U);

    memset(lli, 0xA5, sizeof(lli));
    assert(sgll_dma_lli_build(
        &lli[0], 0x88001000U, 0x88002000U, 257U, SGLL_DMA_MEMORY_TO_MEMORY, SGLL_DMA_WIDTH_HALF_WORD,
        SGLL_DMA_MODE_NORMAL, 0U
    ));
    assert(lli[0].source == 0x88001000U && lli[0].destination == 0x88002000U);
    assert(lli[0].block_ts == 256U && lli[0].next == 0U);
    assert(lli[0].control_low == 0x00000905U);
    assert(lli[0].control_high == 0xC4000000U);
    assert(lli[0].source_status == 0U && lli[0].destination_status == 0U);
    assert(lli[0].status_low == 0U && lli[0].status_high == 0U);
    assert(lli[0].reserved_low == 0U && lli[0].reserved_high == 0U);

    assert(sgll_dma_lli_build(
        &lli[1], 0x041A0060U, 0x88002000U, 64U, SGLL_DMA_PERIPHERAL_TO_MEMORY, SGLL_DMA_WIDTH_BYTE,
        SGLL_DMA_MODE_CIRCULAR, (uintptr_t)&lli[1]
    ));
    assert(lli[1].next == (uintptr_t)&lli[1]);
    assert(lli[1].control_low == 0x00000015U);
    assert(lli[1].control_high == 0x84000000U);

    const DMA_LLI_Type before = lli[1];
    const uintptr_t invalid_links[] = {0U, (uintptr_t)&lli[0] + 8U};
    for (unsigned i = 0; i < sizeof(invalid_links) / sizeof(invalid_links[0]); ++i)
    {
        assert(!sgll_dma_lli_build(
            &lli[1], 0x041A0060U, 0x88002000U, 64U, SGLL_DMA_PERIPHERAL_TO_MEMORY, SGLL_DMA_WIDTH_BYTE,
            SGLL_DMA_MODE_CIRCULAR, invalid_links[i]
        ));
        assert(memcmp(&lli[1], &before, sizeof(before)) == 0);
    }
    alignas(64) unsigned char storage[128];
    memset(storage, 0xA5, sizeof(storage));
    assert(!sgll_dma_lli_build(
        (DMA_LLI_Type *)(void *)(storage + 8U), 0x88001000U, 0x88002000U, 64U, SGLL_DMA_MEMORY_TO_MEMORY,
        SGLL_DMA_WIDTH_BYTE, SGLL_DMA_MODE_NORMAL, 0U
    ));
    for (unsigned i = 0; i < sizeof(storage); ++i)
    {
        assert(storage[i] == 0xA5U);
    }
}

static void check_channel_configuration(void)
{
    const uint64_t config = sgll_dma_config_build(SGLL_DMA_PERIPHERAL_TO_MEMORY, 6U);
    const uintptr_t descriptor_address = (uintptr_t)UINT64_C(0x188003000);
    assert(sgll_dma_channel_configure(6U, config, descriptor_address));
    /** 通道 6 从 0x700 开始，CFG 与 LLP 占用 0x720 到 0x72F。 Channel 6 starts at 0x700; its CFG and LLP
     * occupy 0x720..0x72F. */
    assert(word_at(0x720U) == 0xFU);
    assert((word_at(0x724U) & 7U) == 2U);
    assert(((word_at(0x724U) >> 7U) & 31U) == 6U);
    assert(((word_at(0x724U) >> 12U) & 31U) == 6U);
    assert(word_at(0x728U) == 0x88003000U && word_at(0x72CU) == 1U);

    const DMA_Type before = *DMA;
    assert(!sgll_dma_channel_configure(6U, 0U, 0U));
    assert(!sgll_dma_channel_configure(6U, 0U, descriptor_address + 8U));
    assert(!sgll_dma_channel_configure(8U, 0U, descriptor_address));
    assert(memcmp(DMA, &before, sizeof(before)) == 0);
}

static void check_wait_disabled(void)
{
    DMA->GLOBAL.CHEN = 1U << 6U;
    assert(!sgll_dma_channel_wait_disabled(6U, 2U));
    assert(sgll_dma_channel_wait_disabled(7U, 1U));
    assert(!sgll_dma_channel_wait_disabled(7U, 0U));
    assert(!sgll_dma_channel_wait_disabled(8U, 2U));
    DMA->GLOBAL.CHEN = 0U;
    assert(sgll_dma_channel_wait_disabled(6U, 1U));
}

static void check_reset_lookup(void)
{
    const rstgen_reset_location_t spi2 = sgll_rcc_reset_loc_get(RESET_SPI2);
    assert(spi2.reg_index == 1U && spi2.bit == 10U);
    const rstgen_reset_location_t wdt2 = sgll_rcc_reset_loc_get(RESET_WDT2);
    assert(wdt2.reg_index == 3U && wdt2.bit == 5U);
    assert(!sgll_rcc_reset_target_exists(RESET_NONE));
    assert(!sgll_rcc_reset_target_exists(RESET_TARGET_COUNT));
    assert(!sgll_rcc_reset_target_exists((rstgen_reset_target_t)-1));

    RSTGEN->SOFT_RSTN[1] = UINT32_MAX;
    sgll_rcc_reset_assert(RESET_SPI2);
    assert(RSTGEN->SOFT_RSTN[1] == (UINT32_MAX & ~(1U << 10U)));
    assert(!sgll_rcc_reset_is_released(RESET_SPI2));
    sgll_rcc_reset_release(RESET_SPI2);
    assert(sgll_rcc_reset_is_released(RESET_SPI2));
    assert(RSTGEN->SOFT_RSTN[1] == UINT32_MAX);
}

static void check_uart_ids(void)
{
    const uint32_t ids[] = {0U, 1U, 2U, 4U, 6U, 7U, 12U};
    for (unsigned i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i)
    {
        assert(((0xC0U | ids[i]) & UART_IIR_IID_MASK) == ids[i]);
    }
}

static void check_pinmux(void)
{
    PINMUX->FUNCTION[0x0E4U / 4U] = 0xA5A50006U;
    assert(sgll_pinmux_function_set(PINMUX_SD1_CLK_OFFSET, PINMUX_SD1_CLK_SPI2_SCK_FUNCTION));
    assert(PINMUX->FUNCTION[0x0E4U / 4U] == 0xA5A50001U);
    assert(sgll_pinmux_function_get(PINMUX_SD1_CLK_OFFSET) == 1U);
    assert(sgll_pinmux_function_set(0x1D0U, 7U));
    assert(PINMUX->FUNCTION[116] == 7U);
    assert(!sgll_pinmux_function_set(0x1D4U, 1U));
    assert(!sgll_pinmux_function_set(0x0E5U, 1U));
    assert(!sgll_pinmux_function_set(0x0E4U, 8U));
    assert(PINMUX->FUNCTION[0x0E4U / 4U] == 0xA5A50001U);
    TOP->SD1_SELECT = UINT32_MAX;
    sgll_pinmux_select_sd1_pad_bank();
    assert(TOP->SD1_SELECT == (UINT32_MAX & ~(1U << 10U)));
    assert((uintptr_t)&TOP->SD1_SELECT == TOP_MISC_BASE + 0x294U);
}

static void check_clock_registers(void)
{
    assert(sgll_rcc_clkgen_reg(0x100U) == &CLKGEN->DIV_SPI);
    assert(sgll_rcc_clkgen_reg(0x048U) == &CLKGEN->DIV_CPU_AXI0);
    assert(sgll_rcc_clkgen_reg(0x07CU) == &CLKGEN->DIV_SD1);
    assert(sgll_rcc_clkgen_reg(0x138U) == &CLKGEN->DIV_C906[2]);
    assert(sgll_rcc_clkgen_reg(0x144U) == &CLKGEN->DIV_SRC_VIP_SYS_4);
    assert(sgll_rcc_clkgen_reg(0x014U) == NULL);
    assert(sgll_rcc_clkgen_reg(0x101U) == NULL);
    assert(sgll_rcc_clkgen_reg(0x148U) == NULL);
    assert(!sgll_rcc_field_write(0x014U, 0U, 1U, 0U));
    assert(!sgll_rcc_bit_write(0x014U, 0U, false));

    CLKGEN->CLK_EN[3] = 1U << 7U;
    sgll_rcc_gate_enable(CLKGEN_GATE_SPI);
    assert(CLKGEN->CLK_EN[3] == ((1U << 7U) | (1U << 6U)));
    sgll_rcc_gate_disable(CLKGEN_GATE_SPI);
    assert(CLKGEN->CLK_EN[3] == (1U << 7U));
    CLKGEN->DIV_SPI = 0x01000200U;
    sgll_rcc_div_factor_set(CLKGEN_DIV_OFF_SPI, CLKGEN_DIV_SPI_FACTOR_WIDTH, 8U);
    assert(CLKGEN->DIV_SPI == 0x01080209U);
    assert(sgll_rcc_div_factor_raw_get(CLKGEN_DIV_OFF_SPI, CLKGEN_DIV_SPI_FACTOR_WIDTH) == 8U);
    sgll_rcc_div_source_set(CLKGEN_DIV_OFF_SPI, CLKGEN_SRC_DISPPLL);
    assert(CLKGEN->DIV_SPI == 0x01080109U);
    sgll_rcc_div_reset_assert(CLKGEN_DIV_OFF_SPI);
    assert(!sgll_rcc_div_reset_is_deasserted(CLKGEN_DIV_OFF_SPI));
    sgll_rcc_div_reset_release(CLKGEN_DIV_OFF_SPI);
    assert(sgll_rcc_div_reset_is_deasserted(CLKGEN_DIV_OFF_SPI));
    PLL_G6->MPLL = 0x11111111U;
    PLL_G6->TPLL = 0x22222222U;
    PLL_G6->FPLL = 0x33333333U;
    assert(sgll_rcc_g6_pll_read(0x008U) == 0x11111111U);
    assert(sgll_rcc_g6_pll_read(0x00CU) == 0x22222222U);
    assert(sgll_rcc_g6_pll_read(0x010U) == 0x33333333U);
}

static void check_plic(void)
{
    sgll_plic_irq_priority_set(31U, 2U);
    sgll_plic_irq_priority_set(32U, 3U);
    assert(PLIC_C906L->PRIORITY[31] == 2U && PLIC_C906L->PRIORITY[32] == 3U);
    sgll_plic_irq_enable(31U);
    sgll_plic_irq_enable(32U);
    assert(PLIC_C906L->ENABLE[0] == 0x80000000U && PLIC_C906L->ENABLE[1] == 1U);
    sgll_plic_irq_disable(31U);
    assert(!sgll_plic_irq_is_enabled(31U) && sgll_plic_irq_is_enabled(32U));
    /** 通过匿名内存预置由硬件拥有的只读待处理中断字。 Seed hardware-owned, read-only pending words through
     * the anonymous memory. */
    *(volatile uint32_t *)(uintptr_t)(PLIC_BASE_ADDRESS + 0x1000U) = 0x80000000U;
    *(volatile uint32_t *)(uintptr_t)(PLIC_BASE_ADDRESS + 0x1004U) = 1U;
    assert(sgll_plic_irq_is_pending(31U) && sgll_plic_irq_is_pending(32U));
    assert(!sgll_plic_irq_is_pending(30U));
    sgll_plic_threshold_set(3U);
    assert(sgll_plic_threshold_get() == 3U);
    assert((uintptr_t)&PLIC_C906L->THRESHOLD == PLIC_BASE_ADDRESS + 0x200000U);
    PLIC_C906L->CLAIM_COMPLETE = 25U;
    assert(sgll_plic_irq_claim() == 25U);
    sgll_plic_irq_complete(25U);
    assert((uintptr_t)&PLIC_C906L->CLAIM_COMPLETE == PLIC_BASE_ADDRESS + 0x200004U);
    assert(PLIC_C906L->CLAIM_COMPLETE == 25U);
}

int main(void)
{
    map_registers(DMA_BASE, 4096U);
    map_registers(RSTGEN_BASE, 4096U);
    map_registers(TOP_MISC_BASE, 4096U);
    map_registers(PINMUX_BASE, 4096U);
    map_registers(CLKGEN_BASE, 4096U);
    map_registers(PLIC_BASE_ADDRESS, sizeof(PLIC_C906L_Type));
    check_abort();
    check_descriptors();
    check_channel_configuration();
    check_wait_disabled();
    check_reset_lookup();
    check_uart_ids();
    check_pinmux();
    check_clock_registers();
    check_plic();
    assert(munmap(DMA, 4096U) == 0);
    assert(munmap(RSTGEN, 4096U) == 0);
    assert(munmap(TOP, 4096U) == 0);
    assert(munmap(PINMUX, 4096U) == 0);
    assert(munmap(CLKGEN, 4096U) == 0);
    assert(munmap(PLIC_C906L, sizeof(PLIC_C906L_Type)) == 0);
    puts("DMA, RCC, UART, PINMUX, and PLIC register checks passed");
    return 0;
}
