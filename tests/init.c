/**
 * @file init.c
 * @brief 基于寄存器镜像验证初始化契约。
 *        Validate initialization contracts against register images.
 *
 * @note 硬件复位副作用和寄存器读写银行仍需板级验证。
 *       Hardware reset side effects and read/write register banks still require board validation.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "sgll.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

static void map_block(uintptr_t address)
{
    void *memory = mmap(
        (void *)address, 4096U, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
        0
    );
    assert(memory == (void *)address);
}

static void check_spi_init(void)
{
    sgll_spi_init_t config;
    sgll_spi_struct_init(&config);
    assert(config.data_frame_size == SGLL_SPI_DATA_FRAME_SIZE_8_BIT);
    assert(config.baud_divider == 2U && config.receive_frame_count == 1U);
    config.mode = SGLL_SPI_MODE_1;
    config.baud_divider = 8U;
    SPI2_REGS->IMR = 0x3FU;
    SPI2_REGS->DMACR = 3U;
    SPI2_REGS->SER = 1U;
    assert(sgll_spi_init(SPI2_REGS, &config));
    assert(SPI2_REGS->CTRLR0 == 0x47U && SPI2_REGS->BAUDR == 8U);
    assert(SPI2_REGS->SPIENR == 0U && SPI2_REGS->SER == 0U);
    assert(SPI2_REGS->DMACR == 0U && SPI2_REGS->IMR == 0U);

    config.mode = SGLL_SPI_MODE_3;
    config.transfer_mode = SGLL_SPI_TRANSFER_MODE_RX_ONLY;
    config.data_frame_size = SGLL_SPI_DATA_FRAME_SIZE_16_BIT;
    config.receive_frame_count = 65536U;
    config.baud_divider = 65534U;
    config.rx_fifo_threshold = 7U;
    config.rx_sample_delay = 7U;
    assert(sgll_spi_init(SPI2_REGS, &config));
    assert(SPI2_REGS->CTRLR0 == 0x2CFU && SPI2_REGS->CTRLR1 == 0xFFFFU);
    assert(SPI2_REGS->BAUDR == 65534U && SPI2_REGS->RXFTLR == 7U);
    const SPI_Type before = *SPI2_REGS;
    config.baud_divider = 3U;
    assert(!sgll_spi_init(SPI2_REGS, &config));
    config.baud_divider = 2U;
    config.receive_frame_count = 65537U;
    assert(!sgll_spi_init(SPI2_REGS, &config));
    config.receive_frame_count = 1U;
    config.mode = (sgll_spi_mode_t)4;
    assert(!sgll_spi_init(SPI2_REGS, &config));
    assert(!sgll_spi_init(SPI2_REGS, NULL));
    assert(!sgll_spi_init(NULL, &config));
    assert(memcmp(SPI2_REGS, &before, sizeof(before)) == 0);

    sgll_spi_struct_init(&config);
    SPI2_REGS->SPIENR = 1U;
    const SPI_Type running = *SPI2_REGS;
    assert(!sgll_spi_init(SPI2_REGS, &config));
    assert(!sgll_spi_deinit(SPI2_REGS));
    assert(memcmp(SPI2_REGS, &running, sizeof(running)) == 0);
    SPI2_REGS->SPIENR = 0U;
    RSTGEN->SOFT_RSTN[1] = 0xA5A50000U;
    assert(sgll_spi_deinit(SPI2_REGS));
    assert(RSTGEN->SOFT_RSTN[1] == 0xA5A50400U);
    assert(!sgll_spi_deinit(NULL));
}

static void check_i2c_init(void)
{
    sgll_i2c_init_t config;
    sgll_i2c_struct_init(&config);
    assert(sgll_i2c_init(I2C1_REGS, &config, 25000000U));
    assert(I2C1_REGS->CON == 0x63U && I2C1_REGS->TAR == 0x55U);
    assert(I2C1_REGS->SS_SCL_HCNT == 115U && I2C1_REGS->SS_SCL_LCNT == 135U);
    assert(I2C1_REGS->FS_SCL_HCNT == 21U && I2C1_REGS->FS_SCL_LCNT == 42U);
    assert(I2C1_REGS->SDA_HOLD == 1U && I2C1_REGS->SDA_SETUP == 6U && I2C1_REGS->SPKLEN == 2U);
    config.speed = I2C_SPEED_FAST;
    config.ten_bit_addressing = true;
    config.target_address = 0x2ABU;
    config.rx_fifo_threshold = 63U;
    assert(sgll_i2c_init(I2C1_REGS, &config, 100000000U));
    assert(I2C1_REGS->CON == 0x75U && I2C1_REGS->TAR == 0x12ABU);
    assert(I2C1_REGS->SS_SCL_HCNT == 460U && I2C1_REGS->SS_SCL_LCNT == 540U);
    assert(I2C1_REGS->FS_SCL_HCNT == 90U && I2C1_REGS->FS_SCL_LCNT == 160U);
    assert(I2C1_REGS->SDA_SETUP == 25U && I2C1_REGS->SPKLEN == 5U);
    assert(I2C1_REGS->ENABLE == 0U && I2C1_REGS->DMA_CR == 0U && I2C1_REGS->INTR_MASK == 0U);
    const I2C_Type before = *I2C1_REGS;
    assert(!sgll_i2c_init(I2C1_REGS, &config, 50000000U));
    config.rx_fifo_threshold = 64U;
    assert(!sgll_i2c_init(I2C1_REGS, &config, 100000000U));
    config.rx_fifo_threshold = 0U;
    config.ten_bit_addressing = false;
    assert(!sgll_i2c_init(I2C1_REGS, &config, 100000000U));
    assert(memcmp(I2C1_REGS, &before, sizeof(before)) == 0);

    sgll_i2c_struct_init(&config);
    I2C1_REGS->ENABLE_STATUS = 1U;
    const I2C_Type disabling = *I2C1_REGS;
    assert(!sgll_i2c_init(I2C1_REGS, &config, 25000000U));
    assert(!sgll_i2c_deinit(I2C1_REGS));
    assert(memcmp(I2C1_REGS, &disabling, sizeof(disabling)) == 0);
    I2C1_REGS->ENABLE_STATUS = 0U;
    I2C1_REGS->ENABLE = 1U;
    assert(!sgll_i2c_init(I2C1_REGS, &config, 25000000U));
    I2C1_REGS->ENABLE = 0U;
    RSTGEN->SOFT_RSTN[0] = 0x1234U;
    assert(sgll_i2c_deinit(I2C1_REGS));
    assert(RSTGEN->SOFT_RSTN[0] == 0x10001234U);
    assert(!sgll_i2c_deinit(NULL));
}

static void check_uart_init(void)
{
    sgll_uart_init_t config;
    sgll_uart_struct_init(&config);
    assert(sgll_uart_init(UART1_REGS, &config, 25000000U));
    assert(UART1_REGS->RBR_THR_DLL == 14U && UART1_REGS->IER_DLH == 0U);
    assert(UART1_REGS->LCR == 3U && UART1_REGS->IIR_FCR == 7U && UART1_REGS->MCR == 0U);
    config.baud_rate = 100U;
    config.data_bits = SGLL_UART_DATA_BITS_7;
    config.stop_bits = SGLL_UART_STOP_BITS_2;
    config.parity = SGLL_UART_PARITY_EVEN;
    config.auto_flow_control = true;
    assert(sgll_uart_init(UART1_REGS, &config, 25000000U));
    /** 寄存器镜像记录最后一次 DLL/DLH 写入；真实硬件中的 IER 使用独立银行。 Register image records the last
     * DLL/DLH writes; real hardware banks IER separately. */
    assert(UART1_REGS->RBR_THR_DLL == 9U && UART1_REGS->IER_DLH == 61U);
    assert(UART1_REGS->LCR == 0x1EU && UART1_REGS->MCR == 0x22U);
    const UART_Type before = *UART1_REGS;
    config.stop_bits = SGLL_UART_STOP_BITS_1_5;
    assert(!sgll_uart_init(UART1_REGS, &config, 25000000U));
    sgll_uart_struct_init(&config);
    config.baud_rate = 0U;
    assert(!sgll_uart_init(UART1_REGS, &config, 25000000U));
    config.baud_rate = UINT32_MAX;
    assert(!sgll_uart_init(UART1_REGS, &config, 25000000U));
    assert(memcmp(UART1_REGS, &before, sizeof(before)) == 0);

    sgll_uart_struct_init(&config);
    UART1_REGS->USR = 1U;
    const UART_Type running = *UART1_REGS;
    assert(!sgll_uart_init(UART1_REGS, &config, 25000000U));
    assert(!sgll_uart_deinit(UART1_REGS));
    assert(memcmp(UART1_REGS, &running, sizeof(running)) == 0);
    UART1_REGS->USR = 0U;
    RSTGEN->SOFT_RSTN[0] = 0x1234U;
    assert(sgll_uart_deinit(UART1_REGS));
    assert(RSTGEN->SOFT_RSTN[0] == 0x01001234U);
    assert((UART1_REGS->LCR & 0x80U) == 0U && UART1_REGS->IER_DLH == 0U);
    assert(!sgll_uart_deinit(NULL));
}

static void check_dma_init(void)
{
    sgll_dma_init_t config;
    DMA_LLI_Type lli;
    sgll_dma_struct_init(&config);
    config.source = 0x88001000U;
    config.destination = (uintptr_t)&SPI2_REGS->DR[0];
    config.count = 16U;
    config.direction = SGLL_DMA_MEMORY_TO_PERIPHERAL;
    config.request = SGLL_DMA_REQUEST_SPI2_TX;
    config.priority = 3U;
    DMA->GLOBAL.CFG = 3U;
    DMA->GLOBAL.CHEN = 0x0FU;
    TOP->DMA_REMAP1 = 0x12345678U;
    TOP->DMA_INTERRUPT_MUX = 0xABCDEU;
    DMA->CHANNEL[0].CFG_LOW = 0x55667788U;
    assert(sgll_dma_init(6U, &lli, &config));
    assert(lli.source == config.source && lli.destination == config.destination && lli.block_ts == 15U);
    assert(lli.control_low == 0x45U && lli.control_high == 0xC4000000U);
    assert(DMA->CHANNEL[6].CFG_LOW == 0xFU);
    assert(((DMA->CHANNEL[6].CFG_HIGH >> 17U) & 7U) == 3U);
    assert(DMA->CHANNEL[6].LLP_LOW == (uint32_t)(uintptr_t)&lli);
    assert(sgll_dma_request_route_get(6U) == 21U);
    assert((TOP->DMA_REMAP1 & ~0x803F0000U) == (0x12345678U & ~0x803F0000U));
    assert(DMA->GLOBAL.CFG == 3U && DMA->GLOBAL.CHEN == 0x0FU);
    assert(DMA->CHANNEL[6].INTSIGNAL_EN == 0U);
    const DMA_LLI_Type saved_lli = lli;
    const DMA_Type saved_dma = *DMA;
    config.request = SGLL_DMA_REQUEST_SPI2_RX;
    assert(!sgll_dma_init(6U, &lli, &config));
    config.request = SGLL_DMA_REQUEST_SPI2_TX;
    config.width = SGLL_DMA_WIDTH_HALF_WORD;
    config.source |= 1U;
    assert(!sgll_dma_init(6U, &lli, &config));
    assert(memcmp(&lli, &saved_lli, sizeof(lli)) == 0);
    assert(memcmp(DMA, &saved_dma, sizeof(saved_dma)) == 0);
    config.source &= ~(uintptr_t)1U;
    config.width = SGLL_DMA_WIDTH_BYTE;
    DMA->GLOBAL.CHEN |= 1U << 6U;
    const DMA_Type running = *DMA;
    assert(!sgll_dma_init(6U, &lli, &config));
    assert(!sgll_dma_deinit(6U));
    assert(memcmp(&lli, &saved_lli, sizeof(lli)) == 0);
    assert(memcmp(DMA, &running, sizeof(running)) == 0);
    DMA->GLOBAL.CHEN = 0x0FU;
    config.mode = SGLL_DMA_MODE_CIRCULAR;
    config.request = SGLL_DMA_REQUEST_UART4_TX;
    config.destination = (uintptr_t)&UART4_REGS->RBR_THR_DLL;
    assert(sgll_dma_init(6U, &lli, &config));
    assert(lli.next == (uintptr_t)&lli && lli.control_high == 0x84000000U);
    const uint32_t remap = TOP->DMA_REMAP1;
    assert(sgll_dma_deinit(6U));
    assert(DMA->CHANNEL[6].CFG_LOW == 0U && DMA->CHANNEL[6].LLP_LOW == 0U);
    assert(DMA->GLOBAL.CFG == 3U && DMA->GLOBAL.CHEN == 0x0FU);
    assert(DMA->CHANNEL[0].CFG_LOW == 0x55667788U);
    assert(TOP->DMA_REMAP1 == remap && TOP->DMA_INTERRUPT_MUX == 0xABCDEU);
    assert(!sgll_dma_deinit(8U));
}

int main(void)
{
    map_block(TOP_MISC_BASE);
    map_block(RSTGEN_BASE);
    map_block(DMA_BASE);
    map_block(SPI2_BASE);
    map_block(I2C1_BASE);
    map_block(UART1_BASE);
    check_spi_init();
    check_i2c_init();
    check_uart_init();
    check_dma_init();
    sgll_spi_struct_init(NULL);
    sgll_i2c_struct_init(NULL);
    sgll_uart_struct_init(NULL);
    sgll_dma_struct_init(NULL);
    puts("SPI/I2C/UART/DMA defaults, initialization, rejection, and per-instance deinit passed");
    return 0;
}
