/**
 * @file peripherals.c
 * @brief 驱动迁移使用的外设契约回归。
 *        Peripheral-contract regressions for the driver migration.
 * @note 宿主寄存器镜像不模拟总线时序、ADC 转换或硬件复位副作用。
 *       Host register images do not simulate bus timing, ADC conversion, or hardware reset effects.
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
    assert(
        mmap(
            (void *)address, 4096U, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
            -1, 0
        ) == (void *)address
    );
}

/* Populate a read-only hardware field in the anonymous register image. */
static void seed(const volatile uint32_t *reg, uint32_t value)
{
    *(volatile uint32_t *)(uintptr_t)reg = value;
}

static void check_gpio(void)
{
    assert(sgll_gpio_get(0U) == GPIO0_REGS && sgll_gpio_get(3U) == GPIO3_REGS);
    assert(sgll_gpio_get(GPIO_COUNT) == NULL && sgll_gpio_index_get(NULL) == GPIO_COUNT);
    assert(sgll_gpio_index_get(GPIO3_REGS) == 3U);
    assert(sgll_gpio_index_get(RTC_GPIO_REGS) == GPIO_COUNT);

    GPIO_Type *gpio = RTC_GPIO_REGS;
    const uint32_t pin = 1U << 18;
    gpio->SWPORTA_DR = 0xA5A00000U;
    gpio->SWPORTA_DDR = 0x5A500000U;
    sgll_gpio_output_write(gpio, pin, true);
    sgll_gpio_output_enable(gpio, pin, true);
    assert(sgll_gpio_output_get(gpio) == (0xA5A00000U | pin));
    assert(gpio->SWPORTA_DDR == (0x5A500000U | pin));
    sgll_gpio_output_enable(gpio, pin, false);
    sgll_gpio_output_write(gpio, pin, false);
    assert(gpio->SWPORTA_DDR == (0x5A500000U & ~pin));
    assert(gpio->SWPORTA_DR == (0xA5A00000U & ~pin));
    seed(&gpio->EXT_PORTA, 0xC0010000U);
    assert(sgll_gpio_input_get(gpio) == 0xC0010000U);

    gpio->INTEN = 2U;
    gpio->INTTYPE_LEVEL = 2U;
    gpio->INT_POLARITY = 2U;
    gpio->INTMASK = UINT32_MAX;
    sgll_gpio_interrupt_edge_set(gpio, pin, true);
    sgll_gpio_interrupt_polarity_set(gpio, pin, false);
    sgll_gpio_interrupt_mask(gpio, pin, false);
    sgll_gpio_interrupt_enable(gpio, pin, true);
    assert(gpio->INTTYPE_LEVEL == (2U | pin) && gpio->INT_POLARITY == 2U);
    assert(gpio->INTMASK == (UINT32_MAX & ~pin) && gpio->INTEN == (2U | pin));
    seed(&gpio->INTSTATUS, pin);
    sgll_gpio_interrupt_clear(gpio, sgll_gpio_interrupt_status_get(gpio));
    assert(gpio->PORTA_EOI == pin);
    sgll_gpio_interrupt_enable(gpio, pin, false);
    assert(gpio->INTEN == 2U);
}

static void check_pwm(void)
{
    assert(sgll_pwm_get(0U) == PWM0_REGS && sgll_pwm_get(3U) == PWM3_REGS);
    assert(sgll_pwm_get(PWM_CONTROLLER_COUNT) == NULL);
    PWM_Type *pwm = PWM2_REGS;
    sgll_pwm_init_t config;
    sgll_pwm_struct_init(&config);
    config.period_ticks = 5000U;
    config.inactive_ticks = 3750U;
    pwm->POLARITY = 0xCAFE0008U;
    pwm->PWMSTART = 8U;
    pwm->PWM_OE = 8U;
    pwm->PWMUPDATE = 4U;
    assert(sgll_pwm_init(pwm, 1U, &config));
    assert(pwm->CHANNEL[1].PERIOD == 5000U && pwm->CHANNEL[1].HLPERIOD == 3750U);
    assert(pwm->PWMSTART == 8U && pwm->PWM_OE == 8U);
    assert(sgll_pwm_start(pwm, 2U, false));
    assert(pwm->POLARITY == 0xCAFE000AU && pwm->PWMSTART == 10U && pwm->PWM_OE == 10U);
    const PWM_Type running = *pwm;
    assert(!sgll_pwm_init(pwm, 1U, &config));
    assert(!sgll_pwm_period_set(pwm, 1U, 5000U, 5000U));
    assert(!sgll_pwm_period_set(pwm, 4U, 5000U, 1U));
    assert(!sgll_pwm_period_set(pwm, 1U, 0x40000000U, 1U));
    assert(!sgll_pwm_period_set(pwm, 1U, 5000U, 0U));
    assert(!sgll_pwm_start(pwm, 16U, true));
    assert(!sgll_pwm_update(pwm, 0U));
    assert(!sgll_pwm_stop(NULL, 1U));
    assert(memcmp(pwm, &running, sizeof(running)) == 0);
    assert(sgll_pwm_period_set(pwm, 1U, 8000U, 2000U));
    assert(sgll_pwm_update(pwm, 2U));
    assert(pwm->CHANNEL[1].PERIOD == 8000U && pwm->CHANNEL[1].HLPERIOD == 2000U);
    assert(pwm->PWMUPDATE == 4U && pwm->PWMSTART == 10U);
    assert(sgll_pwm_stop(pwm, 2U));
    assert(pwm->PWM_OE == 8U && pwm->PWMSTART == 8U);
    assert(sgll_pwm_init(pwm, 1U, &config));
    assert(pwm->POLARITY == 0xCAFE0008U);
}

static void check_adc(void)
{
    assert(sgll_adc_get(0U) == SARADC_REGS && sgll_adc_get(1U) == RTC_SARADC_REGS);
    assert(sgll_adc_get(2U) == NULL);
    sgll_adc_init_t config;
    sgll_adc_struct_init(&config);
    assert(config.clock_divider == 15U && !config.external_reference);
    config.external_reference = true;
    for (uint32_t domain = 0U; domain < 2U; ++domain)
    {
        SARADC_Type *adc = sgll_adc_get(domain);
        adc->CYC_SET = 0xABC001E0U;
        adc->TEST = 0xFFFFFFFBU;
        assert(sgll_adc_init(adc, &config));
        assert(adc->CYC_SET == 0xABCBF3EFU && adc->TEST == UINT32_MAX);
        assert(adc->INTR_EN == 0U && adc->INTR_CLR == 1U);
        for (uint32_t channel = 1U; channel <= 3U; ++channel)
        {
            adc->CTRL = 0xA50000F1U;
            assert(sgll_adc_start(adc, channel));
            assert(adc->CTRL == (0xA5000001U | (1U << (4U + channel))));
            assert(sgll_adc_wait_idle(adc, 1U));
            seed(&adc->RESULT[channel - 1U], 0xFFFFF123U);
            uint16_t value = 0xFFFFU;
            assert(sgll_adc_result_read(adc, channel, &value) && value == 0x123U);
            seed(&adc->RESULT[channel - 1U], 0x7000U);
            assert(!sgll_adc_result_read(adc, channel, &value) && value == 0x123U);
        }
        seed(&adc->STATUS, 1U);
        const SARADC_Type busy = *adc;
        assert(!sgll_adc_wait_idle(adc, 8U));
        assert(!sgll_adc_init(adc, &config) && !sgll_adc_start(adc, 1U));
        assert(memcmp(adc, &busy, sizeof(busy)) == 0);
        seed(&adc->STATUS, 0U);
        const SARADC_Type before = *adc;
        config.clock_divider = 16U;
        assert(!sgll_adc_init(adc, &config));
        assert(!sgll_adc_start(adc, 0U) && !sgll_adc_start(adc, 4U));
        assert(!sgll_adc_wait_idle(adc, 0U));
        assert(memcmp(adc, &before, sizeof(before)) == 0);
        config.clock_divider = 15U;
    }
    assert(!sgll_adc_init(NULL, &config) && !sgll_adc_wait_idle(NULL, 1U));
    RTC_CTRL_REGS->CLOCK_MUX = UINT32_MAX;
    RTC_CTRL_REGS->RESET = 0x80000001U;
    sgll_rcc_rtc_saradc_enable();
    assert(RTC_CTRL_REGS->CLOCK_MUX == 0xFFEFFFFFU);
    assert(RTC_CTRL_REGS->RESET == 0x80020001U);
}

static void check_wdt(void)
{
    assert(sgll_wdt_get(0U) == WDT0_REGS && sgll_wdt_get(2U) == WDT2_REGS);
    assert(sgll_wdt_get(3U) == NULL);
    assert(sgll_wdt_timeout_cycles(0U) == 65536ULL);
    assert(sgll_wdt_timeout_cycles(15U) == 2147483648ULL);
    WDT_Type *wdt = WDT2_REGS;
    sgll_wdt_init_t config;
    sgll_wdt_struct_init(&config);
    config.top = 8U;
    config.interrupt_first = true;
    assert(sgll_wdt_init(wdt, &config));
    assert(wdt->TORR == 0x88U && wdt->TOC == 0U && wdt->CR == 2U);
    const WDT_Type before = *wdt;
    config.top = 16U;
    assert(!sgll_wdt_init(wdt, &config) && !sgll_wdt_start(wdt, 16U, false));
    assert(memcmp(wdt, &before, sizeof(before)) == 0);
    assert(sgll_wdt_start(wdt, 9U, true));
    assert(wdt->TORR == 0x99U && wdt->CRR == 0x76U && wdt->CR == 3U);
    const WDT_Type running = *wdt;
    config.top = 1U;
    assert(!sgll_wdt_init(wdt, &config) && !sgll_wdt_start(wdt, 1U, false));
    assert(memcmp(wdt, &running, sizeof(running)) == 0);

    TOP->SYS_CTRL = 0x80000001U;
    TOP->WDT_CTRL = 0xA5000717U;
    assert(sgll_wdt_reset_route_set(2U, true));
    assert(TOP->SYS_CTRL == 0x80000101U && TOP->WDT_CTRL == 0xA5000753U);
    sgll_rcc_watchdog_clock_select(true);
    assert(TOP->WDT_CTRL == 0xA5000153U);
    assert(sgll_wdt_reset_route_set(2U, false));
    assert(TOP->WDT_CTRL == 0xA5000117U);
    sgll_rcc_watchdog_clock_select(false);
    assert(TOP->WDT_CTRL == 0xA5000017U);
    assert(!sgll_wdt_reset_route_set(3U, true) && TOP->WDT_CTRL == 0xA5000017U);
}

static void check_i2c_timing(void)
{
    static const uint32_t clocks[] = {25000000U, 50000000U, 100000000U, 187500000U};
    /* Frozen counts from the driver's nanosecond timing contract, independent of TRM presets. */
    static const sgll_i2c_timing_t expected[] = {
        {105U, 124U, 20U, 39U, 8U, 25U, 2U},
        {212U, 249U, 42U, 79U, 15U, 50U, 3U},
        {427U, 499U, 87U, 159U, 30U, 100U, 5U},
        {804U, 937U, 166U, 299U, 57U, 188U, 10U},
    };
    sgll_i2c_timing_t timing;
    sgll_i2c_init_t config;
    sgll_i2c_struct_init(&config);
    config.speed = I2C_SPEED_FAST;
    config.ten_bit_addressing = true;
    config.target_address = 0x2ABU;
    for (size_t i = 0U; i < sizeof(clocks) / sizeof(clocks[0]); ++i)
    {
        assert(sgll_i2c_timing_calculate(clocks[i], &timing));
        assert(memcmp(&timing, &expected[i], sizeof(timing)) == 0);
        assert(sgll_i2c_init_with_timing(I2C3_REGS, &config, &timing));
        assert(I2C3_REGS->CON == 0x75U && I2C3_REGS->TAR == 0x12ABU);
        assert(I2C3_REGS->SS_SCL_HCNT == expected[i].standard_high);
        assert(I2C3_REGS->SS_SCL_LCNT == expected[i].standard_low);
        assert(I2C3_REGS->FS_SCL_HCNT == expected[i].fast_high);
        assert(I2C3_REGS->FS_SCL_LCNT == expected[i].fast_low);
        assert(I2C3_REGS->SDA_HOLD == expected[i].sda_hold);
        assert(
            I2C3_REGS->SDA_SETUP == expected[i].sda_setup && I2C3_REGS->SPKLEN == expected[i].spike_length
        );
        assert(I2C3_REGS->ENABLE == 0U && I2C3_REGS->INTR_MASK == 0U && I2C3_REGS->DMA_CR == 0U);
    }
    const sgll_i2c_timing_t prior = timing;
    assert(!sgll_i2c_timing_calculate(0U, &timing));
    assert(!sgll_i2c_timing_calculate(1000000U, &timing));
    assert(!sgll_i2c_timing_calculate(256000000U, &timing));
    assert(!sgll_i2c_timing_calculate(UINT32_MAX, &timing));
    assert(!sgll_i2c_timing_calculate(50000000U, NULL));
    assert(memcmp(&timing, &prior, sizeof(timing)) == 0);
    const I2C_Type image = *I2C3_REGS;
    timing.sda_setup = 256U;
    assert(!sgll_i2c_init_with_timing(I2C3_REGS, &config, &timing));
    assert(memcmp(I2C3_REGS, &image, sizeof(image)) == 0);
    timing = prior;
    seed(&I2C3_REGS->ENABLE_STATUS, 1U);
    assert(!sgll_i2c_init_with_timing(I2C3_REGS, &config, &timing));
    assert(sgll_i2c_enable_wait(I2C3_REGS, true, 1U));
    assert(!sgll_i2c_enable_wait(I2C3_REGS, false, 4U));
    assert(I2C3_REGS->ENABLE == 0U);
    seed(&I2C3_REGS->ENABLE_STATUS, 0U);
    assert(sgll_i2c_enable_wait(I2C3_REGS, false, 1U));
    assert(!sgll_i2c_enable_wait(NULL, true, 1U));
    sgll_i2c_master_address_set(I2C3_REGS, 0x42U, false);
    assert(I2C3_REGS->CON == 0x65U && I2C3_REGS->TAR == 0x42U);
    sgll_i2c_dma_threshold_set(I2C3_REGS, 5U, 7U);
    assert(I2C3_REGS->DMA_TDLR == 5U && I2C3_REGS->DMA_RDLR == 7U);
    assert(sgll_i2c_data_address(I2C3_REGS) == I2C3_BASE + 0x10U);
}

static void check_dma_routes_and_waits(void)
{
    assert(sgll_dmamux_spi_request_get(0U, false) == 16U);
    assert(sgll_dmamux_spi_request_get(3U, true) == 23U);
    assert(sgll_dmamux_spi_request_get(4U, true) == DMA_REQUEST_NONE);
    assert(sgll_dmamux_i2c_request_get(0U, false) == 24U);
    assert(sgll_dmamux_i2c_request_get(4U, true) == 33U);
    assert(sgll_dmamux_i2c_request_get(UINT32_MAX, false) == DMA_REQUEST_NONE);
    DMA->GLOBAL.CHEN = 1U;
    assert(sgll_dma_channels_wait_disabled(0xF0U, 1U));
    assert(!sgll_dma_channels_wait_disabled(0xF1U, 4U));
    assert(!sgll_dma_channels_wait_disabled(0U, 1U));
    assert(!sgll_dma_channels_wait_disabled(0x100U, 1U));
    assert(!sgll_dma_channels_wait_disabled(0xF0U, 0U));
    assert(sgll_spi_wait_idle(SPI1_REGS, 1U));
    SPI1_REGS->SR = 1U;
    assert(!sgll_spi_wait_idle(SPI1_REGS, 4U));
    assert(!sgll_spi_wait_idle(NULL, 1U));
}

int main(void)
{
    map_block(RTC_GPIO_BASE);
    map_block(PWM2_BASE);
    map_block(SARADC_BASE);
    map_block(RTC_SARADC_BASE);
    map_block(RTC_CTRL_BASE);
    map_block(WDT2_BASE);
    map_block(TOP_MISC_BASE);
    map_block(I2C3_BASE);
    map_block(DMA_BASE);
    map_block(SPI1_BASE);
    check_gpio();
    check_pwm();
    check_adc();
    check_wdt();
    check_i2c_timing();
    check_dma_routes_and_waits();
    puts("Peripheral isolation, validation, timing, and bounded-wait contracts passed");
    return 0;
}
