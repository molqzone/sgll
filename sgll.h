/**
 * @file sgll.h
 * @brief SG2002 设备定义和全部 LL 接口的统一入口。
 *        Unified entry point for SG2002 device definitions and all LL interfaces.
 */

/**
 * @defgroup SGLL SG2002 低层库 / SG2002 low-level library
 * @brief C23 无状态硬件接口，支持 C++ 调用。
 *        Stateless C23 hardware interfaces with C++ interoperability.
 */
#pragma once

#include "inc/sg2002.h"
#include "inc/sgll_core.h"

#include "inc/sg200x_ll_adc.h"
#include "inc/sg200x_ll_csr.h"
#include "inc/sg200x_ll_dma.h"
#include "inc/sg200x_ll_dmamux.h"
#include "inc/sg200x_ll_efuse.h"
#include "inc/sg200x_ll_gpio.h"
#include "inc/sg200x_ll_i2c.h"
#include "inc/sg200x_ll_mbox.h"
#include "inc/sg200x_ll_pinmux.h"
#include "inc/sg200x_ll_plic.h"
#include "inc/sg200x_ll_pwm.h"
#include "inc/sg200x_ll_rcc.h"
#include "inc/sg200x_ll_spi.h"
#include "inc/sg200x_ll_tempsen.h"
#include "inc/sg200x_ll_tim.h"
#include "inc/sg200x_ll_uart.h"
#include "inc/sg200x_ll_utils.h"
#include "inc/sg200x_ll_wdt.h"
