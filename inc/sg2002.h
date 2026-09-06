/*
 * sg2002.h - SG2002 chip identity and instance availability.
 *
 * References:
 *   - SOPHGO SG2002 Technical Reference Manual v1.02 (2025-02-27).
 *   - This header carries no register knowledge; base addresses live in
 *     sg200x_ll_defs.h.
 *
 * The sg200x.h family umbrella includes exactly one chip header selected by
 * the SG200X_CHIP_* macros.  When a second family member (e.g. SG2005) is
 * added, add sg2005.h and make sg200x.h choose via the chip macros.
 */
#pragma once

/* ------------------------------------------------------------------ */
/* Chip identity                                                       */
/* ------------------------------------------------------------------ */

#define SG200X_FAMILY "SG200X"
#define SG200X_CHIP_SG2002 1
#define SG200X_CHIP_NAME "SG2002"

/* Xtal frequencies fixed by the board contract (TRM ch. 8: XTAL_XIN 25 MHz,
 * RTC_XIN 32.768 kHz).  The C906L firmware device tree documents
 * timebase-frequency = 25000000, read through the `time` CSR. */
#define SG200X_XTAL_FREQ_HZ 25000000UL
#define SG200X_RTC_XTAL_FREQ_HZ 32768UL

/* ------------------------------------------------------------------ */
/* Instance availability on SG2002 (TRM ch. 2.3.11, ch. 3 Table 3.4)   */
/* ------------------------------------------------------------------ */

/* GPIO: 4 controllers in the Active domain, 1 in the RTC (No-die) domain. */
#define SG200X_GPIO_COUNT 4
#define SG200X_RTC_GPIO_COUNT 1

/* UART: 5 in the Active domain (UART0..UART4), 1 RTCSYS_UART. */
#define SG200X_UART_COUNT 5
#define SG200X_RTC_UART_COUNT 1

/* I2C: 5 masters in the Active domain, 1 RTCSYS_I2C (TRM 21.1.6: 6 sets). */
#define SG200X_I2C_COUNT 5
#define SG200X_RTC_I2C_COUNT 1

/* SPI: 4 DesignWare SSI masters in the Active domain (TRM 21.3). */
#define SG200X_SPI_COUNT 4

/* PWM: 4 controllers x 4 channels = PWM0..PWM15 (TRM 21.9.1). */
#define SG200X_PWM_CONTROLLER_COUNT 4
#define SG200X_PWM_CHANNEL_COUNT 16

/* Timers: 8 down-counters in one block, plus RTCSYS_Timer (TRM ch. 12). */
#define SG200X_TIMER_COUNT 8

/* Watchdogs: 3 in the Active domain, 1 RTCSYS_WDT (TRM ch. 13.5). */
#define SG200X_WDT_COUNT 3
#define SG200X_RTC_WDT_COUNT 1

/* SARADC: 1 controller in the Active domain, 1 RTCSYS_SARADC, 3 channels
 * each (TRM 21.7.1). */
#define SG200X_SARADC_COUNT 2
#define SG200X_SARADC_CHANNEL_COUNT 3

/* System DMA: one 8-channel DesignWare AXI DMAC (TRM ch. 11.2). */
#define SG200X_DMA_CHANNEL_COUNT 8
