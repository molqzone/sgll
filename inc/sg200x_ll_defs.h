/*
 * sg200x_ll_defs.h - Instance base addresses and C906L PLIC IRQ numbers.
 *
 * Zero dependencies besides sg2002.h and <stdint.h>.
 *
 * Address sources:
 *   - TRM v1.02 Table 3.4 (memory mapping) and the per-instance tables
 *     in chapters 11-13 and 21 (e.g. Table 21.2 I2C, Table 21.44 UART,
 *     Table 21.71 SPI, Table 21.140 GPIO, ch. 13.5 WDT, ch. 12.5 TIMER).
 *   - PLIC base and geometry: NOT documented in the TRM.  Taken from the
 *     board DTS (interrupt-controller@70000000, riscv,ndev = 101,
 *     riscv,max-priority = 7) and the SDK
 *     freertos/cvitek/arch/riscv64/include/irq.h, cross-checked against
 *     driver/sg200x_dma.cpp (PLIC claim/complete 0x70200004).
 *
 * IRQ number sources (C906L view):
 *   - SDK hal/cv181x/config/intr_conf.h ("CPU2" column; the C906L is
 *     CPU2 in the TOP mux naming).  These match the board-verified
 *     driver/ values: SDMA=25, WDT2=39, GPIO0..3=41..44.
 *   - NOTE: the big-core (Linux) PLIC uses a DIFFERENT numbering
 *     (TRM Table 3.1: e.g. GPIO0=76, System DMA=45, UART0=60).  Do not
 *     mix the two tables.
 */
#pragma once

#include <stdint.h>

#include "sg2002.h"

/* ------------------------------------------------------------------ */
/* System control and buses                                            */
/* ------------------------------------------------------------------ */

#define SG200X_APB_MAILBOX_BASE 0x01900000UL /* ap_mailbox, TRM Table 3.4 */
#define SG200X_AP_SYSTEM_CTRL_BASE 0x01901000UL
#define SG200X_TOP_MISC_BASE 0x03000000UL /* incl. SDMA CPU mux 0x03000298 */
#define SG200X_PINMUX_BASE 0x03001000UL
#define SG200X_CLKGEN_BASE 0x03002000UL /* TRM Table 8.51 */
#define SG200X_PLL_G2_BASE 0x03002800UL /* TRM Table 8.5 */
#define SG200X_PLL_G6_BASE 0x03002900UL /* TRM Table 8.6 */
#define SG200X_RSTGEN_BASE 0x03003000UL /* TRM Table 7.1 */

/* ------------------------------------------------------------------ */
/* Active-domain peripherals                                           */
/* ------------------------------------------------------------------ */

#define SG200X_WDT0_BASE 0x03010000UL
#define SG200X_WDT1_BASE 0x03011000UL
#define SG200X_WDT2_BASE 0x03012000UL /* C906L-owned (driver contract) */

#define SG200X_GPIO0_BASE 0x03020000UL
#define SG200X_GPIO1_BASE 0x03021000UL
#define SG200X_GPIO2_BASE 0x03022000UL
#define SG200X_GPIO3_BASE 0x03023000UL

#define SG200X_WGN0_BASE 0x03030000UL /* Wiegand generator 0 */
#define SG200X_WGN1_BASE 0x03031000UL
#define SG200X_WGN2_BASE 0x03032000UL

#define SG200X_KEYSCAN_BASE 0x03040000UL
#define SG200X_EFUSE_BASE 0x03050000UL

#define SG200X_PWM0_BASE 0x03060000UL
#define SG200X_PWM1_BASE 0x03061000UL
#define SG200X_PWM2_BASE 0x03062000UL
#define SG200X_PWM3_BASE 0x03063000UL

#define SG200X_TIMER_BASE 0x030A0000UL /* 8 timers, TRM Table 12.1 */

#define SG200X_TEMPSEN_BASE 0x030E0000UL
#define SG200X_SARADC_BASE 0x030F0000UL /* active-domain channels 1..3 */

#define SG200X_I2C0_BASE 0x04000000UL
#define SG200X_I2C1_BASE 0x04010000UL
#define SG200X_I2C2_BASE 0x04020000UL
#define SG200X_I2C3_BASE 0x04030000UL
#define SG200X_I2C4_BASE 0x04040000UL

#define SG200X_SPI_NAND_BASE 0x04060000UL
#define SG200X_ETH0_BASE 0x04070000UL

#define SG200X_I2S0_BASE 0x04100000UL
#define SG200X_I2S_GLOBAL_BASE 0x04108000UL
#define SG200X_I2S1_BASE 0x04110000UL
#define SG200X_I2S2_BASE 0x04120000UL
#define SG200X_I2S3_BASE 0x04130000UL

#define SG200X_UART0_BASE 0x04140000UL
#define SG200X_UART1_BASE 0x04150000UL
#define SG200X_UART2_BASE 0x04160000UL
#define SG200X_UART3_BASE 0x04170000UL

#define SG200X_SPI0_BASE 0x04180000UL
#define SG200X_SPI1_BASE 0x04190000UL
#define SG200X_SPI2_BASE 0x041A0000UL
#define SG200X_SPI3_BASE 0x041B0000UL

#define SG200X_UART4_BASE 0x041C0000UL
#define SG200X_AUDSRC_BASE 0x041D0000UL

#define SG200X_EMMC_BASE 0x04300000UL
#define SG200X_SD0_BASE 0x04310000UL
#define SG200X_SD1_BASE 0x04320000UL

/* System DMA: 8-channel DesignWare AXI DMAC.  Board contract: channels
 * 0-3 are owned by Linux, channels 4-7 by C906L (PLAN section 7). */
#define SG200X_DMA_BASE 0x04330000UL

#define SG200X_USB_BASE 0x04340000UL

/* ------------------------------------------------------------------ */
/* RTC (No-die) domain                                                 */
/* ------------------------------------------------------------------ */

#define SG200X_RTC_TIMER_BASE 0x05020000UL
#define SG200X_RTC_GPIO_BASE 0x05021000UL
#define SG200X_RTC_UART_BASE 0x05022000UL
#define SG200X_RTC_INTR_BASE 0x05023000UL
#define SG200X_RTC_MBOX_BASE 0x05024000UL
#define SG200X_RTC_CTRL_BASE 0x05025000UL
#define SG200X_RTC_CORE_BASE 0x05026000UL
#define SG200X_RTC_IO_BASE 0x05027000UL
#define SG200X_RTC_OSC_BASE 0x05028000UL
#define SG200X_RTC_32KLESS_BASE 0x0502A000UL
#define SG200X_RTC_I2C_BASE 0x0502B000UL
#define SG200X_RTC_SARADC_BASE 0x0502C000UL /* PWR_ADC1..3 */
#define SG200X_RTC_WDT_BASE 0x0502D000UL

/* ------------------------------------------------------------------ */
/* Security subsystem                                                  */
/* ------------------------------------------------------------------ */

#define SG200X_CRYPTO_DMA_BASE 0x02060000UL
#define SG200X_TRNG_BASE 0x02070000UL

/* ------------------------------------------------------------------ */
/* PLIC (C906L context)                                                */
/* ------------------------------------------------------------------ */

/* Standard RISC-V PLIC at 0x70000000; the C906L uses context 0
 * (M-mode): threshold 0x70200000, claim/complete 0x70200004. */
#define SG200X_PLIC_BASE 0x70000000UL

/* Board DTS: riscv,max-priority = 7. */
#define SG200X_LL_PLIC_PRIORITY_MAX 7UL

/* SDK intr_conf.h: NUM_IRQ = 62 (C906L/"CPU2" view). */
#define SG200X_LL_NUM_IRQ 62UL

/* ------------------------------------------------------------------ */
/* C906L PLIC IRQ numbers                                              */
/*                                                                     */
/* From SDK hal/cv181x/config/intr_conf.h (CPU2 column).  Entries the   */
/* SDK marks NA (not routed to C906L) are omitted; the list is only     */
/* what C906L firmware may claim.                                      */
/* ------------------------------------------------------------------ */

#define SG200X_LL_IRQ_SDMA 25UL /* System DMA via TOP CPU2 mux */
#define SG200X_LL_IRQ_I2S0 26UL
#define SG200X_LL_IRQ_I2S1 27UL
#define SG200X_LL_IRQ_I2S2 28UL
#define SG200X_LL_IRQ_I2S3 29UL
#define SG200X_LL_IRQ_UART0 30UL
#define SG200X_LL_IRQ_UART1 31UL
#define SG200X_LL_IRQ_I2C0 32UL
#define SG200X_LL_IRQ_I2C1 33UL
#define SG200X_LL_IRQ_I2C2 34UL
#define SG200X_LL_IRQ_I2C3 35UL
#define SG200X_LL_IRQ_I2C4 36UL
#define SG200X_LL_IRQ_SPI0 37UL
#define SG200X_LL_IRQ_SPI1 38UL
#define SG200X_LL_IRQ_WDT2 39UL
#define SG200X_LL_IRQ_KEYSCAN 40UL
#define SG200X_LL_IRQ_GPIO0 41UL
#define SG200X_LL_IRQ_GPIO1 42UL
#define SG200X_LL_IRQ_GPIO2 43UL
#define SG200X_LL_IRQ_GPIO3 44UL
#define SG200X_LL_IRQ_WGN0 45UL
#define SG200X_LL_IRQ_MBOX1 46UL
#define SG200X_LL_IRQ_IRRX 47UL
#define SG200X_LL_IRQ_RTC_GPIO 48UL
#define SG200X_LL_IRQ_RTC_UART 49UL
#define SG200X_LL_IRQ_RTC_I2C 50UL
#define SG200X_LL_IRQ_RTC_WDT 51UL
#define SG200X_LL_IRQ_TDMA 52UL
#define SG200X_LL_IRQ_SW0_CPU2 53UL
#define SG200X_LL_IRQ_SW1_CPU2 54UL
#define SG200X_LL_IRQ_TIMER4 55UL
#define SG200X_LL_IRQ_TIMER5 56UL
#define SG200X_LL_IRQ_TIMER6 57UL
#define SG200X_LL_IRQ_TIMER7 58UL
#define SG200X_LL_IRQ_SPACC 59UL
#define SG200X_LL_IRQ_IVE 60UL
#define SG200X_LL_IRQ_MBOX_C906_2ND 61UL
