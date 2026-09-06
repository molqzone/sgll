/*
 * sg200x.h - SG200X family umbrella header.
 *
 * This is the single include business code uses:
 *
 *     #include "sg200x.h"
 *
 * Include graph (PLAN section 3.0, strictly one-way, every LL header is
 * also individually includable):
 *
 *   sg200x.h
 *    +- sg2002.h              chip identity, instance availability
 *    +- sg200x_ll_csr.h       CSR/asm primitives (zero deps)
 *    +- sg200x_ll_utils.h     MMIO/bit-field primitives (zero deps)
 *    +- sg200x_ll_defs.h      base addresses, C906L IRQ numbers
 *    +- sg200x_ll_plic.h      PLIC primitives          (P0)
 *    +- sg200x_ll_rcc.h       CLKGEN + RSTGEN          (P0)
 *    +- peripheral headers    P1..P7, filled in phase order
 *
 * Do NOT include SDK or LibXR headers from here; sgll is free-standing.
 */
#pragma once

/* Chip selection: the family umbrella includes exactly one chip header.
 * Default is SG2002 (this BSP's board); define SG200X_CHIP_SG2005 before
 * including to override once a sg2005.h exists. */
#if !defined(SG200X_CHIP_SG2002) && !defined(SG200X_CHIP_SG2005)
#define SG200X_CHIP_SG2002 1
#endif

#if defined(SG200X_CHIP_SG2002)
#include "sg2002.h"
#elif defined(SG200X_CHIP_SG2005)
#error "sgll: SG2005 not supported yet; add sg2005.h first"
#else
#error "sgll: no SG200X chip selected"
#endif

/* P0 foundation (order matters for later headers). */
#include "sg200x_ll_csr.h"
#include "sg200x_ll_defs.h"
#include "sg200x_ll_plic.h"
#include "sg200x_ll_rcc.h"
#include "sg200x_ll_utils.h"

/* ------------------------------------------------------------------ */
/* Peripheral modules (P1..P7; currently empty skeletons, included so  */
/* the umbrella contract is stable as they get implemented).           */
/* ------------------------------------------------------------------ */

#include "sg200x_ll_adc.h"
#include "sg200x_ll_dma.h"
#include "sg200x_ll_dmamux.h"
#include "sg200x_ll_gpio.h"
#include "sg200x_ll_i2c.h"
#include "sg200x_ll_pinmux.h"
#include "sg200x_ll_pwm.h"
#include "sg200x_ll_spi.h"
#include "sg200x_ll_tempsen.h"
#include "sg200x_ll_tim.h"
#include "sg200x_ll_uart.h"
#include "sg200x_ll_wdt.h"

/* Optional modules (on-demand). */
#include "sg200x_ll_efuse.h"
#include "sg200x_ll_mbox.h"
