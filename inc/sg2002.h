/**
 * @file sg2002.h
 * @brief SG2002 芯片设备定义。
 *        SG2002 chip device definitions.
 *
 * @note 集中定义地址映射、寄存器布局、实例指针、中断号和硬件位；外设 LL 头在此之上提供操作语义。
 *       Centralize address maps, register layouts, instance pointers, interrupt IDs, and hardware bits;
 * peripheral LL headers build semantic operations on top.
 * @note C23 constexpr 常量保留字面量整数类型；实例转换、预处理功能开关和展开列表使用宏。
 *       C23 constexpr constants retain literal integer types; macros serve instance casts, preprocessor
 * feature switches, and expansion lists.
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02，表 3.4、第 7/8/11/21 章。SG2002 TRM v1.02, Table 3.4 and Chapters
 * 7/8/11/21.
 * @see sg2002-licheerv-nano-b.svd；I2C/UART 寄存器描述。I2C/UART register descriptions.
 * @see Sophgo SDK C906L 中断配置。Sophgo SDK C906L interrupt configuration.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup SG2002_DEVICE SG2002 设备定义 / SG2002 device
 * @ingroup SGLL
 * @brief 寄存器布局、实例指针、硬件常量及中断源。
 *        Register layouts, instance pointers, hardware constants, and interrupt sources.
 */

/**
 * @defgroup SG2002_INFO 芯片参数与实例数量 / Chip identity and instance counts
 * @ingroup SG2002_DEVICE
 * @{
 */

/// @brief 主晶振频率，单位为 Hz。
///        Main crystal frequency in Hz.
static constexpr auto XTAL_FREQ_HZ = 25'000'000UL;
/// @brief RTC 晶振频率，单位为 Hz。
///        RTC crystal frequency in Hz.
static constexpr auto RTC_XTAL_FREQ_HZ = 32'768UL;

/// @brief 主域 GPIO 控制器数量。
///        Number of main-domain GPIO controllers.
static constexpr auto GPIO_COUNT = 4U;

/// @brief 每个 GPIO 端口的引脚数。
///        Pins per GPIO port.
static constexpr auto GPIO_PIN_COUNT = 32U;

/// @brief RTC 域 GPIO 控制器数量。
///        Number of RTC-domain GPIO controllers.
static constexpr auto RTC_GPIO_COUNT = 1U;
/// @brief 主域 UART 控制器数量。
///        Number of main-domain UART controllers.
static constexpr auto UART_COUNT = 5U;
/// @brief RTC 域 UART 控制器数量。
///        Number of RTC-domain UART controllers.
static constexpr auto RTC_UART_COUNT = 1U;
/// @brief 主域 I2C 控制器数量。
///        Number of main-domain I2C controllers.
static constexpr auto I2C_COUNT = 5U;
/// @brief RTC 域 I2C 控制器数量。
///        Number of RTC-domain I2C controllers.
static constexpr auto RTC_I2C_COUNT = 1U;
/// @brief SSI 控制器数量。
///        Number of SSI controllers.
static constexpr auto SPI_COUNT = 4U;
/// @brief PWM 控制器数量。
///        Number of PWM controllers.
static constexpr auto PWM_CONTROLLER_COUNT = 4U;

/// @brief 每个 PWM 控制器的通道数。
///        Channels per PWM controller.
static constexpr auto PWM_CHANNELS_PER_CONTROLLER = 4U;

/// @brief 全部 PWM 控制器的通道总数。
///        Total channels across all PWM controllers.
static constexpr auto PWM_CHANNEL_COUNT = 16U;
/// @brief 系统定时器数量。
///        Number of system timers.
static constexpr auto TIMER_COUNT = 8U;
/// @brief 主域看门狗数量。
///        Number of main-domain watchdogs.
static constexpr auto WDT_COUNT = 3U;
/// @brief RTC 域看门狗数量。
///        Number of RTC-domain watchdogs.
static constexpr auto RTC_WDT_COUNT = 1U;
/// @brief 主域与 RTC 域 SARADC 实例总数。
///        Total SARADC instances across main and RTC domains.
static constexpr auto SARADC_COUNT = 2U;
/// @brief 每个 SARADC 实例的输入通道数。
///        Input channels per SARADC instance.
static constexpr auto SARADC_CHANNEL_COUNT = 3U;
/// @brief AXI DMA 通道总数。
///        Total AXI DMA channels.
static constexpr auto DMA_CHANNEL_COUNT = 8U;

/** @} */

/**
 * @defgroup SG2002_CORE C906L 核心寄存器字段 / C906L core register fields
 * @ingroup SG2002_DEVICE
 * @{
 */

/**
 * @name mstatus 中断字段 / mstatus interrupt fields
 * @see RISC-V 特权规范，mstatus CSR。RISC-V privileged specification, mstatus CSR.
 * @{
 */
/// @brief 机器级全局中断使能位。
///        Machine-mode global interrupt-enable bit.
static constexpr auto SGLL_MSTATUS_MIE_BIT = (1UL << 3);
/** @} */

/**
 * @name mie 中断字段 / mie interrupt fields
 * @see RISC-V 特权规范，mie CSR。RISC-V privileged specification, mie CSR.
 * @{
 */
/// @brief 监管者软件中断使能位。
///        Supervisor software interrupt-enable bit.
static constexpr auto SGLL_MIE_SSIE_BIT = (1UL << 1);
/// @brief 机器软件中断使能位。
///        Machine software interrupt-enable bit.
static constexpr auto SGLL_MIE_MSIE_BIT = (1UL << 3);
/// @brief 监管者定时器中断使能位。
///        Supervisor timer interrupt-enable bit.
static constexpr auto SGLL_MIE_STIE_BIT = (1UL << 5);
/// @brief 机器定时器中断使能位。
///        Machine timer interrupt-enable bit.
static constexpr auto SGLL_MIE_MTIE_BIT = (1UL << 7);
/// @brief 监管者外部中断使能位。
///        Supervisor external interrupt-enable bit.
static constexpr auto SGLL_MIE_SEIE_BIT = (1UL << 9);
/// @brief 机器外部中断使能位，用于 PLIC。
///        Machine external interrupt-enable bit used by the PLIC.
static constexpr auto SGLL_MIE_MEIE_BIT = (1UL << 11);
/** @} */

/// @brief C906L 数据缓存行大小，单位为字节。
///        C906L data-cache line size in bytes.
static constexpr auto SGLL_DCACHE_LINE_SIZE = 64U;

/** @} */

/**
 * @defgroup SG2002_MEMORY_MAP SG2002 地址映射 / SG2002 memory map
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02, 表 3.4。SG2002 TRM v1.02, Table 3.4.
 * @{
 */

/// @brief APB_MAILBOX 寄存器基地址。
///        Base address of APB_MAILBOX registers.
static constexpr auto APB_MAILBOX_BASE = 0x0190'0000UL;
/// @brief AP_SYSTEM_CTRL 寄存器基地址。
///        Base address of AP_SYSTEM_CTRL registers.
static constexpr auto AP_SYSTEM_CTRL_BASE = 0x0190'1000UL;
/// @brief CRYPTO_DMA 寄存器基地址。
///        Base address of CRYPTO_DMA registers.
static constexpr auto CRYPTO_DMA_BASE = 0x0206'0000UL;
/// @brief TRNG 寄存器基地址。
///        Base address of TRNG registers.
static constexpr auto TRNG_BASE = 0x0207'0000UL;

/// @brief TOP_MISC 寄存器基地址。
///        Base address of TOP_MISC registers.
static constexpr auto TOP_MISC_BASE = 0x0300'0000UL;
/// @brief PINMUX 寄存器基地址。
///        Base address of PINMUX registers.
static constexpr auto PINMUX_BASE = 0x0300'1000UL;
/// @brief CLKGEN 寄存器基地址。
///        Base address of CLKGEN registers.
static constexpr auto CLKGEN_BASE = 0x0300'2000UL;

/// @brief CLKGEN 寄存器地址窗口大小。
///        Size of the CLKGEN register-address window.
static constexpr auto CLKGEN_REGISTER_BYTES = 0x1'000U;

/// @brief PLL_G2 寄存器基地址。
///        Base address of PLL_G2 registers.
static constexpr auto PLL_G2_BASE = 0x0300'2800UL;
/// @brief PLL_G6 寄存器基地址。
///        Base address of PLL_G6 registers.
static constexpr auto PLL_G6_BASE = 0x0300'2900UL;
/// @brief RSTGEN 寄存器基地址。
///        Base address of RSTGEN registers.
static constexpr auto RSTGEN_BASE = 0x0300'3000UL;

/// @brief WDT0 寄存器基地址。
///        Base address of WDT0 registers.
static constexpr auto WDT0_BASE = 0x0301'0000UL;
/// @brief WDT1 寄存器基地址。
///        Base address of WDT1 registers.
static constexpr auto WDT1_BASE = 0x0301'1000UL;
/// @brief WDT2 寄存器基地址。
///        Base address of WDT2 registers.
static constexpr auto WDT2_BASE = 0x0301'2000UL;
/// @brief GPIO0 寄存器基地址。
///        Base address of GPIO0 registers.
static constexpr auto GPIO0_BASE = 0x0302'0000UL;
/// @brief GPIO1 寄存器基地址。
///        Base address of GPIO1 registers.
static constexpr auto GPIO1_BASE = 0x0302'1000UL;
/// @brief GPIO2 寄存器基地址。
///        Base address of GPIO2 registers.
static constexpr auto GPIO2_BASE = 0x0302'2000UL;
/// @brief GPIO3 寄存器基地址。
///        Base address of GPIO3 registers.
static constexpr auto GPIO3_BASE = 0x0302'3000UL;
/// @brief WGN0 寄存器基地址。
///        Base address of WGN0 registers.
static constexpr auto WGN0_BASE = 0x0303'0000UL;
/// @brief WGN1 寄存器基地址。
///        Base address of WGN1 registers.
static constexpr auto WGN1_BASE = 0x0303'1000UL;
/// @brief WGN2 寄存器基地址。
///        Base address of WGN2 registers.
static constexpr auto WGN2_BASE = 0x0303'2000UL;
/// @brief KEYSCAN 寄存器基地址。
///        Base address of KEYSCAN registers.
static constexpr auto KEYSCAN_BASE = 0x0304'0000UL;
/// @brief EFUSE 寄存器基地址。
///        Base address of EFUSE registers.
static constexpr auto EFUSE_BASE = 0x0305'0000UL;
/// @brief PWM0 寄存器基地址。
///        Base address of PWM0 registers.
static constexpr auto PWM0_BASE = 0x0306'0000UL;
/// @brief PWM1 寄存器基地址。
///        Base address of PWM1 registers.
static constexpr auto PWM1_BASE = 0x0306'1000UL;
/// @brief PWM2 寄存器基地址。
///        Base address of PWM2 registers.
static constexpr auto PWM2_BASE = 0x0306'2000UL;
/// @brief PWM3 寄存器基地址。
///        Base address of PWM3 registers.
static constexpr auto PWM3_BASE = 0x0306'3000UL;
/// @brief TIMER 寄存器基地址。
///        Base address of TIMER registers.
static constexpr auto TIMER_BASE = 0x030A'0000UL;
/// @brief TEMPSEN 寄存器基地址。
///        Base address of TEMPSEN registers.
static constexpr auto TEMPSEN_BASE = 0x030E'0000UL;
/// @brief SARADC 寄存器基地址。
///        Base address of SARADC registers.
static constexpr auto SARADC_BASE = 0x030F'0000UL;

/// @brief I2C0 寄存器基地址。
///        Base address of I2C0 registers.
static constexpr auto I2C0_BASE = 0x0400'0000UL;
/// @brief I2C1 寄存器基地址。
///        Base address of I2C1 registers.
static constexpr auto I2C1_BASE = 0x0401'0000UL;
/// @brief I2C2 寄存器基地址。
///        Base address of I2C2 registers.
static constexpr auto I2C2_BASE = 0x0402'0000UL;
/// @brief I2C3 寄存器基地址。
///        Base address of I2C3 registers.
static constexpr auto I2C3_BASE = 0x0403'0000UL;
/// @brief I2C4 寄存器基地址。
///        Base address of I2C4 registers.
static constexpr auto I2C4_BASE = 0x0404'0000UL;
/// @brief SPI_NAND 寄存器基地址。
///        Base address of SPI_NAND registers.
static constexpr auto SPI_NAND_BASE = 0x0406'0000UL;
/// @brief ETH0 寄存器基地址。
///        Base address of ETH0 registers.
static constexpr auto ETH0_BASE = 0x0407'0000UL;
/// @brief I2S0 寄存器基地址。
///        Base address of I2S0 registers.
static constexpr auto I2S0_BASE = 0x0410'0000UL;
/// @brief I2S_GLOBAL 寄存器基地址。
///        Base address of I2S_GLOBAL registers.
static constexpr auto I2S_GLOBAL_BASE = 0x0410'8000UL;
/// @brief I2S1 寄存器基地址。
///        Base address of I2S1 registers.
static constexpr auto I2S1_BASE = 0x0411'0000UL;
/// @brief I2S2 寄存器基地址。
///        Base address of I2S2 registers.
static constexpr auto I2S2_BASE = 0x0412'0000UL;
/// @brief I2S3 寄存器基地址。
///        Base address of I2S3 registers.
static constexpr auto I2S3_BASE = 0x0413'0000UL;
/// @brief UART0 寄存器基地址。
///        Base address of UART0 registers.
static constexpr auto UART0_BASE = 0x0414'0000UL;
/// @brief UART1 寄存器基地址。
///        Base address of UART1 registers.
static constexpr auto UART1_BASE = 0x0415'0000UL;
/// @brief UART2 寄存器基地址。
///        Base address of UART2 registers.
static constexpr auto UART2_BASE = 0x0416'0000UL;
/// @brief UART3 寄存器基地址。
///        Base address of UART3 registers.
static constexpr auto UART3_BASE = 0x0417'0000UL;
/// @brief SPI0 寄存器基地址。
///        Base address of SPI0 registers.
static constexpr auto SPI0_BASE = 0x0418'0000UL;
/// @brief SPI1 寄存器基地址。
///        Base address of SPI1 registers.
static constexpr auto SPI1_BASE = 0x0419'0000UL;
/// @brief SPI2 寄存器基地址。
///        Base address of SPI2 registers.
static constexpr auto SPI2_BASE = 0x041A'0000UL;
/// @brief SPI3 寄存器基地址。
///        Base address of SPI3 registers.
static constexpr auto SPI3_BASE = 0x041B'0000UL;
/// @brief UART4 寄存器基地址。
///        Base address of UART4 registers.
static constexpr auto UART4_BASE = 0x041C'0000UL;
/// @brief AUDSRC 寄存器基地址。
///        Base address of AUDSRC registers.
static constexpr auto AUDSRC_BASE = 0x041D'0000UL;
/// @brief EMMC 寄存器基地址。
///        Base address of EMMC registers.
static constexpr auto EMMC_BASE = 0x0430'0000UL;
/// @brief SD0 寄存器基地址。
///        Base address of SD0 registers.
static constexpr auto SD0_BASE = 0x0431'0000UL;
/// @brief SD1 寄存器基地址。
///        Base address of SD1 registers.
static constexpr auto SD1_BASE = 0x0432'0000UL;
/// @brief DMA 寄存器基地址。
///        Base address of DMA registers.
static constexpr auto DMA_BASE = 0x0433'0000UL;
/// @brief USB 寄存器基地址。
///        Base address of USB registers.
static constexpr auto USB_BASE = 0x0434'0000UL;

/// @brief RTC_TIMER 寄存器基地址。
///        Base address of RTC_TIMER registers.
static constexpr auto RTC_TIMER_BASE = 0x0502'0000UL;
/// @brief RTC_GPIO 寄存器基地址。
///        Base address of RTC_GPIO registers.
static constexpr auto RTC_GPIO_BASE = 0x0502'1000UL;
/// @brief RTC_UART 寄存器基地址。
///        Base address of RTC_UART registers.
static constexpr auto RTC_UART_BASE = 0x0502'2000UL;
/// @brief RTC_INTR 寄存器基地址。
///        Base address of RTC_INTR registers.
static constexpr auto RTC_INTR_BASE = 0x0502'3000UL;
/// @brief RTC_MBOX 寄存器基地址。
///        Base address of RTC_MBOX registers.
static constexpr auto RTC_MBOX_BASE = 0x0502'4000UL;
/// @brief RTC_CTRL 寄存器基地址。
///        Base address of RTC_CTRL registers.
static constexpr auto RTC_CTRL_BASE = 0x0502'5000UL;
/// @brief RTC_CORE 寄存器基地址。
///        Base address of RTC_CORE registers.
static constexpr auto RTC_CORE_BASE = 0x0502'6000UL;
/// @brief RTC_IO 寄存器基地址。
///        Base address of RTC_IO registers.
static constexpr auto RTC_IO_BASE = 0x0502'7000UL;
/// @brief RTC_OSC 寄存器基地址。
///        Base address of RTC_OSC registers.
static constexpr auto RTC_OSC_BASE = 0x0502'8000UL;
/// @brief RTC_32KLESS 寄存器基地址。
///        Base address of RTC_32KLESS registers.
static constexpr auto RTC_32KLESS_BASE = 0x0502'A000UL;
/// @brief RTC_I2C 寄存器基地址。
///        Base address of RTC_I2C registers.
static constexpr auto RTC_I2C_BASE = 0x0502'B000UL;
/// @brief RTC_SARADC 寄存器基地址。
///        Base address of RTC_SARADC registers.
static constexpr auto RTC_SARADC_BASE = 0x0502'C000UL;
/// @brief RTC_WDT 寄存器基地址。
///        Base address of RTC_WDT registers.
static constexpr auto RTC_WDT_BASE = 0x0502'D000UL;

/// @brief PLIC 寄存器基地址。
///        Base address of PLIC registers.
static constexpr auto PLIC_BASE_ADDRESS = 0x7000'0000UL;
/** @} */

/**
 * @defgroup SG2002_PLIC C906L PLIC 寄存器 / C906L PLIC registers
 * @ingroup SG2002_DEVICE
 * @see Sophgo SDK arch/riscv64/include/irq.h（寄存器定义来源 / register-definition source）。
 * @{
 */

/// @brief PLIC 允许的最大优先级值。
///        Maximum permitted PLIC priority value.
static constexpr auto PLIC_PRIORITY_MAX = 7UL;
/// @brief C906L PLIC 源编号空间大小，包含保留编号 0。
///        Size of the C906L PLIC source-ID space, including reserved ID zero.
static constexpr auto NUM_IRQ = 62UL;
/// @brief PLIC 优先级寄存器组的字节偏移。
///        Byte offset of PLIC priority register array.
static constexpr auto PLIC_PRIORITY_OFFSET = 0x0000UL;
/// @brief PLIC 待处理中断位数组的字节偏移。
///        Byte offset of PLIC pending-interrupt bit array.
static constexpr auto PLIC_PENDING_OFFSET = 0x1000UL;
/// @brief PLIC 上下文源使能位数组的字节偏移。
///        Byte offset of PLIC context source-enable bit array.
static constexpr auto PLIC_ENABLE_OFFSET = 0x2000UL;
/// @brief PLIC 上下文优先级阈值的字节偏移。
///        Byte offset of PLIC context priority threshold.
static constexpr auto PLIC_THRESHOLD_OFFSET = 0x20'0000UL;
/// @brief PLIC 领取和完成寄存器的字节偏移。
///        Byte offset of PLIC claim-and-completion register.
static constexpr auto PLIC_CLAIM_OFFSET = 0x20'0004UL;

/// @brief 一个 PLIC 位图字容纳的中断源数量。
///        Interrupt sources represented by one PLIC bitmap word.
static constexpr auto PLIC_IRQS_PER_WORD = 32U;
/// @brief 覆盖 C906L 源编号空间所需的位图字数。
///        Bitmap words needed to cover the C906L source-ID space.
static constexpr auto PLIC_IRQ_WORD_COUNT = ((NUM_IRQ + PLIC_IRQS_PER_WORD - 1U) / PLIC_IRQS_PER_WORD);

/**
 * @brief C906L 上下文 0 的 PLIC 寄存器块。
 *        PLIC register block for C906L context zero.
 *
 * @note SDK 也定义 PLIC_Type/PLIC，本视图通过名称区分 C906L 上下文。
 *       The SDK also defines PLIC_Type/PLIC; this view names the C906L context explicitly.
 * @see Sophgo SDK arch/riscv64/include/irq.h（寄存器定义来源 / register-definition source）。
 */
typedef struct
{
    volatile uint32_t
        PRIORITY[NUM_IRQ]; ///< 偏移 0x000：中断源优先级数组。Offset 0x000: interrupt-source priority array.
    uint32_t RESERVED_PRIORITY
        [(PLIC_PENDING_OFFSET - NUM_IRQ * sizeof(uint32_t)) /
         sizeof(uint32_t)]; ///< 偏移 0x0F8：未公开区域的占位空间。Offset 0x0F8: padding for an unexposed
                            ///< area.
    volatile const uint32_t PENDING[PLIC_IRQ_WORD_COUNT]; ///< 偏移 0x1000：待处理中断位数组。Offset 0x1000:
                                                          ///< pending-interrupt bit array.
    uint32_t RESERVED_PENDING
        [(PLIC_ENABLE_OFFSET - PLIC_PENDING_OFFSET) / sizeof(uint32_t) -
         PLIC_IRQ_WORD_COUNT]; ///< 偏移 0x1008：未公开区域的占位空间。Offset 0x1008: padding for an unexposed
                               ///< area.
    volatile uint32_t ENABLE[PLIC_IRQ_WORD_COUNT]; ///< 偏移 0x2000：上下文 0 的源使能位数组。Offset 0x2000:
                                                   ///< context-zero source-enable bit array.
    uint32_t RESERVED_ENABLE
        [(PLIC_THRESHOLD_OFFSET - PLIC_ENABLE_OFFSET) / sizeof(uint32_t) -
         PLIC_IRQ_WORD_COUNT]; ///< 偏移 0x2008：未公开区域的占位空间。Offset 0x2008: padding for an unexposed
                               ///< area.
    volatile uint32_t
        THRESHOLD; ///< 偏移 0x200000：上下文优先级阈值。Offset 0x200000: context priority threshold.
    volatile uint32_t CLAIM_COMPLETE; ///< 偏移 0x200004：读领取、写完成寄存器。Offset 0x200004:
                                      ///< read-to-claim, write-to-complete register.
} PLIC_C906L_Type;

static_assert(offsetof(PLIC_C906L_Type, PENDING) == PLIC_PENDING_OFFSET);
static_assert(offsetof(PLIC_C906L_Type, ENABLE) == PLIC_ENABLE_OFFSET);
static_assert(offsetof(PLIC_C906L_Type, THRESHOLD) == PLIC_THRESHOLD_OFFSET);
static_assert(offsetof(PLIC_C906L_Type, CLAIM_COMPLETE) == PLIC_CLAIM_OFFSET);

/**
 * @brief PLIC_C906L 寄存器实例指针。
 *        PLIC_C906L register-instance pointer.
 */
#define PLIC_C906L ((PLIC_C906L_Type *)(uintptr_t)PLIC_BASE_ADDRESS)

/** @} */

/**
 * @defgroup SG2002_PINMUX 引脚功能选择 / PINMUX function selectors
 * @ingroup SG2002_DEVICE
 * @see Sophgo SDK cv181x_reg_fmux_gpio.h and SG2002 pinlist（寄存器定义来源 / register-definition source）。
 * @{
 */

/**
 * @brief 连续的 32 位引脚功能选择寄存器数量。
 *        Number of contiguous 32-bit pin-function selectors.
 *
 * @note 选择器从偏移 0x000 的 CAM_MCLK0 连续排列到 0x1D0 的 GPIO_ZQ。
 *       Selectors run contiguously from CAM_MCLK0 at offset 0x000 to GPIO_ZQ at 0x1D0.
 * @see Sophgo SDK cv181x_reg_fmux_gpio.h（寄存器定义来源 / register-definition source）。
 */
static constexpr auto PINMUX_FUNCTION_COUNT = 117U;

/**
 * @brief 连续的引脚功能选择寄存器。
 *        Contiguous pin-function selector registers.
 *
 * @see Sophgo SDK cv181x_reg_fmux_gpio.h（寄存器定义来源 / register-definition source）。
 */
typedef struct
{
    volatile uint32_t FUNCTION[PINMUX_FUNCTION_COUNT]; ///< 偏移 0x000：引脚功能选择寄存器数组。Offset 0x000:
                                                       ///< pin-function selector register array.
} PINMUX_Type;

static_assert(sizeof(PINMUX_Type) == 0x1D4U);

/**
 * @brief PINMUX 寄存器实例指针。
 *        PINMUX register-instance pointer.
 */
#define PINMUX ((PINMUX_Type *)(uintptr_t)PINMUX_BASE)

/**
 * @name LicheeRV Nano SD1/SPI2 引脚功能 / LicheeRV Nano SD1/SPI2 pad functions
 * @note TRM 未给出功能表；这些值采用 SG2002 SDK 引脚列表。
 *        The TRM omits the function table; these values follow the SG2002 SDK pinlist.
 * @{
 */
/// @brief 引脚复用功能字段的位掩码。
///        Bit mask for pin-function selector field.
static constexpr auto PINMUX_FUNCTION_MASK = 0x07UL;

/// @brief SD0_PWR_EN 引脚功能选择偏移。
///        Function-selector offset of SD0_PWR_EN.
static constexpr auto PINMUX_SD0_PWR_EN_OFFSET = 0x038UL;
/// @brief EMMC_CLK 引脚功能选择偏移。
///        Function-selector offset of EMMC_CLK.
static constexpr auto PINMUX_EMMC_CLK_OFFSET = 0x050UL;
/// @brief EMMC_DAT0 引脚功能选择偏移。
///        Function-selector offset of EMMC_DAT0.
static constexpr auto PINMUX_EMMC_DAT0_OFFSET = 0x054UL;
/// @brief EMMC_CMD 引脚功能选择偏移。
///        Function-selector offset of EMMC_CMD.
static constexpr auto PINMUX_EMMC_CMD_OFFSET = 0x05CUL;
/// @brief EMMC_DAT1 引脚功能选择偏移。
///        Function-selector offset of EMMC_DAT1.
static constexpr auto PINMUX_EMMC_DAT1_OFFSET = 0x060UL;
/// @brief PWM0_BUCK 引脚功能选择偏移。
///        Function-selector offset of PWM0_BUCK.
static constexpr auto PINMUX_PWM0_BUCK_OFFSET = 0x0ECUL;
/// @brief SD0_PWR_EN 选择 GPIOA14 的编码。
///        GPIOA14 function encoding on SD0_PWR_EN.
static constexpr auto PINMUX_SD0_PWR_EN_GPIOA14_FUNCTION = 3U;
/// @brief EMMC_CLK 选择 GPIOA22 的编码。
///        GPIOA22 function encoding on EMMC_CLK.
static constexpr auto PINMUX_EMMC_CLK_GPIOA22_FUNCTION = 3U;
/// @brief EMMC_CMD 选择 GPIOA23 的编码。
///        GPIOA23 function encoding on EMMC_CMD.
static constexpr auto PINMUX_EMMC_CMD_GPIOA23_FUNCTION = 3U;
/// @brief EMMC_DAT1 选择 GPIOA24 的编码。
///        GPIOA24 function encoding on EMMC_DAT1.
static constexpr auto PINMUX_EMMC_DAT1_GPIOA24_FUNCTION = 3U;
/// @brief EMMC_DAT0 选择 GPIOA25 的编码。
///        GPIOA25 function encoding on EMMC_DAT0.
static constexpr auto PINMUX_EMMC_DAT0_GPIOA25_FUNCTION = 3U;
/// @brief PWM0_BUCK 选择 PWM0 的编码。
///        PWM0 function encoding on PWM0_BUCK.
static constexpr auto PINMUX_PWM0_BUCK_PWM0_FUNCTION = 0U;
/// @brief SD1_D3 对应的 RTC GPIO18 位掩码。
///        RTC GPIO18 mask corresponding to SD1_D3.
static constexpr auto PINMUX_SD1_D3_GPIO_MASK = (1UL << 18);

/// @brief EMMC_DAT2 引脚选择字段的字节偏移。
///        Byte offset of EMMC_DAT2 pad-selector field.
static constexpr auto PINMUX_EMMC_DAT2_OFFSET = 0x04CUL;
/// @brief SD1_D3 引脚选择字段的字节偏移。
///        Byte offset of SD1_D3 pad-selector field.
static constexpr auto PINMUX_SD1_D3_OFFSET = 0x0D0UL;
/// @brief SD1_D2 引脚选择字段的字节偏移。
///        Byte offset of SD1_D2 pad-selector field.
static constexpr auto PINMUX_SD1_D2_OFFSET = 0x0D4UL;
/// @brief SD1_D1 引脚选择字段的字节偏移。
///        Byte offset of SD1_D1 pad-selector field.
static constexpr auto PINMUX_SD1_D1_OFFSET = 0x0D8UL;
/// @brief SD1_D0 引脚选择字段的字节偏移。
///        Byte offset of SD1_D0 pad-selector field.
static constexpr auto PINMUX_SD1_D0_OFFSET = 0x0DCUL;
/// @brief SD1_CMD 引脚选择字段的字节偏移。
///        Byte offset of SD1_CMD pad-selector field.
static constexpr auto PINMUX_SD1_CMD_OFFSET = 0x0E0UL;
/// @brief SD1_CLK 引脚选择字段的字节偏移。
///        Byte offset of SD1_CLK pad-selector field.
static constexpr auto PINMUX_SD1_CLK_OFFSET = 0x0E4UL;
/// @brief EMMC_DAT2 选择 GPIOA26 的复用编码。
///        Mux encoding selecting GPIOA26 on EMMC_DAT2.
static constexpr auto PINMUX_EMMC_DAT2_GPIOA26_FUNCTION = 3U;
/// @brief SD1_D3 选择 SPI2_CS 的复用编码。
///        Mux encoding selecting SPI2_CS on SD1_D3.
static constexpr auto PINMUX_SD1_D3_SPI2_CS_FUNCTION = 1U;
/// @brief SD1_D3 选择 GPIO18 的复用编码。
///        Mux encoding selecting GPIO18 on SD1_D3.
static constexpr auto PINMUX_SD1_D3_GPIO18_FUNCTION = 3U;
/// @brief SD1_D0 选择 SPI2_SDI 的复用编码。
///        Mux encoding selecting SPI2_SDI on SD1_D0.
static constexpr auto PINMUX_SD1_D0_SPI2_SDI_FUNCTION = 1U;
/// @brief SD1_D0 选择 GPIO21 的复用编码。
///        Mux encoding selecting GPIO21 on SD1_D0.
static constexpr auto PINMUX_SD1_D0_GPIO21_FUNCTION = 3U;
/// @brief SD1_CMD 选择 SPI2_SDO 的复用编码。
///        Mux encoding selecting SPI2_SDO on SD1_CMD.
static constexpr auto PINMUX_SD1_CMD_SPI2_SDO_FUNCTION = 1U;
/// @brief SD1_CMD 选择 GPIO22 的复用编码。
///        Mux encoding selecting GPIO22 on SD1_CMD.
static constexpr auto PINMUX_SD1_CMD_GPIO22_FUNCTION = 3U;
/// @brief SD1_CLK 选择 SPI2_SCK 的复用编码。
///        Mux encoding selecting SPI2_SCK on SD1_CLK.
static constexpr auto PINMUX_SD1_CLK_SPI2_SCK_FUNCTION = 1U;
/// @brief SD1_CLK 选择 GPIO23 的复用编码。
///        Mux encoding selecting GPIO23 on SD1_CLK.
static constexpr auto PINMUX_SD1_CLK_GPIO23_FUNCTION = 3U;
/// @brief SD1_D0 对应的 GPIO 引脚位掩码。
///        GPIO pin mask corresponding to SD1_D0.
static constexpr auto PINMUX_SD1_D0_GPIO_MASK = (1UL << 21);
/// @brief SD1_CMD 对应的 GPIO 引脚位掩码。
///        GPIO pin mask corresponding to SD1_CMD.
static constexpr auto PINMUX_SD1_CMD_GPIO_MASK = (1UL << 22);
/// @brief SD1_CLK 对应的 GPIO 引脚位掩码。
///        GPIO pin mask corresponding to SD1_CLK.
static constexpr auto PINMUX_SD1_CLK_GPIO_MASK = (1UL << 23);
/// @brief SD1 输入输出复用引脚组的位掩码。
///        Bit mask for SD1 GPIO pad group.
static constexpr auto PINMUX_SD1_GPIO_PAD_MASK =
    (PINMUX_SD1_D0_GPIO_MASK | PINMUX_SD1_CMD_GPIO_MASK | PINMUX_SD1_CLK_GPIO_MASK);
/// @brief TOP SD1 引脚组选源寄存器的字节偏移。
///        Byte offset of TOP SD1 pad-bank selector register.
static constexpr auto TOP_SD1_SELECT_OFFSET = 0x294UL;
/// @brief TOP SD1 的替代 MIPI 通路选择位。
///        Bit for TOP SD1 alternate-MIPI-path selection.
static constexpr auto TOP_SD1_MIPI_SELECT_BIT = (1UL << 10);
/** @} */

/** @} */

/**
 * @defgroup SG2002_GPIO GPIO 端口 A 寄存器 / GPIO Port A registers
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02, 表 21.140-21.141。SG2002 TRM v1.02, Tables 21.140-21.141.
 * @{
 */

/**
 * @brief GPIO 端口 A 数据、方向和中断寄存器。
 *        GPIO Port A data, direction, and interrupt registers.
 */
typedef struct
{
    volatile uint32_t
        SWPORTA_DR; ///< 偏移 0x000：端口 A 输出数据寄存器。Offset 0x000: Port A output-data register.
    volatile uint32_t SWPORTA_DDR; ///< 偏移 0x004：端口 A 输出方向使能寄存器。Offset 0x004: Port A
                                   ///< output-direction enable register.
    uint32_t
        RESERVED_008[10]; ///< 偏移 0x008：未公开区域的占位空间。Offset 0x008: padding for an unexposed area.
    volatile uint32_t INTEN;   ///< 偏移 0x030：中断使能寄存器。Offset 0x030: interrupt-enable register.
    volatile uint32_t INTMASK; ///< 偏移 0x034：中断屏蔽寄存器。Offset 0x034: interrupt-mask register.
    volatile uint32_t
        INTTYPE_LEVEL; ///< 偏移 0x038：边沿或电平触发选择寄存器。Offset 0x038: edge/level trigger selector.
    volatile uint32_t
        INT_POLARITY; ///< 偏移 0x03C：中断极性选择寄存器。Offset 0x03C: interrupt-polarity selector.
    volatile const uint32_t
        INTSTATUS; ///< 偏移 0x040：端口 A 中断状态。Offset 0x040: Port A interrupt status.
    volatile const uint32_t RAW_INTSTATUS; ///< 偏移 0x044：屏蔽前的端口 A 中断状态。Offset 0x044: unmasked
                                           ///< Port A interrupt status.
    volatile uint32_t
        DEBOUNCE; ///< 偏移 0x048：输入消抖使能寄存器。Offset 0x048: input-debounce enable register.
    volatile uint32_t
        PORTA_EOI; ///< 偏移 0x04C：端口 A 中断清除寄存器。Offset 0x04C: Port A interrupt-clear register.
    volatile const uint32_t
        EXT_PORTA; ///< 偏移 0x050：端口 A 输入电平。Offset 0x050: Port A input pin levels.
    uint32_t
        RESERVED_054[3]; ///< 偏移 0x054：未公开区域的占位空间。Offset 0x054: padding for an unexposed area.
    volatile uint32_t LS_SYNC; ///< 偏移 0x060：电平敏感输入同步使能寄存器。Offset 0x060: level-sensitive
                               ///< input synchronization enable register.
} GPIO_Type;

static_assert(offsetof(GPIO_Type, SWPORTA_DR) == 0x000U);
static_assert(offsetof(GPIO_Type, SWPORTA_DDR) == 0x004U);
static_assert(offsetof(GPIO_Type, INTEN) == 0x030U);
static_assert(offsetof(GPIO_Type, PORTA_EOI) == 0x04CU);
static_assert(offsetof(GPIO_Type, EXT_PORTA) == 0x050U);
static_assert(offsetof(GPIO_Type, LS_SYNC) == 0x060U);

/**
 * @brief GPIO0 寄存器实例指针。
 *        GPIO0 register-instance pointer.
 */
#define GPIO0_REGS ((GPIO_Type *)(uintptr_t)GPIO0_BASE)
/**
 * @brief GPIO1 寄存器实例指针。
 *        GPIO1 register-instance pointer.
 */
#define GPIO1_REGS ((GPIO_Type *)(uintptr_t)GPIO1_BASE)
/**
 * @brief GPIO2 寄存器实例指针。
 *        GPIO2 register-instance pointer.
 */
#define GPIO2_REGS ((GPIO_Type *)(uintptr_t)GPIO2_BASE)
/**
 * @brief GPIO3 寄存器实例指针。
 *        GPIO3 register-instance pointer.
 */
#define GPIO3_REGS ((GPIO_Type *)(uintptr_t)GPIO3_BASE)
/**
 * @brief RTC_GPIO 寄存器实例指针。
 *        RTC_GPIO register-instance pointer.
 */
#define RTC_GPIO_REGS ((GPIO_Type *)(uintptr_t)RTC_GPIO_BASE)

/** @} */

/**
 * @defgroup SG2002_TOP TOP 系统控制与 DMA 路由 / TOP system control and DMA routing
 * @ingroup SG2002_DEVICE
 * @{
 */

/// @brief TOP DMA 请求重映射寄存器 0的字节偏移。
///        Byte offset of TOP DMA request-remap register zero.
static constexpr auto TOP_DMA_REMAP0_OFFSET = 0x154U;
/// @brief TOP DMA 请求重映射寄存器 1的字节偏移。
///        Byte offset of TOP DMA request-remap register one.
static constexpr auto TOP_DMA_REMAP1_OFFSET = 0x158U;
/// @brief TOP DMA 中断复用寄存器的字节偏移。
///        Byte offset of TOP DMA interrupt-multiplexer register.
static constexpr auto TOP_DMA_INTERRUPT_MUX_OFFSET = 0x298U;
/// @brief CPU0 DMA 中断路由字段的最低位号。
///        Least-significant bit position of the CPU0 DMA routing field.
static constexpr auto TOP_DMA_INTERRUPT_CPU0_SHIFT = 0U;
/// @brief CPU1 DMA 中断路由字段的最低位号。
///        Least-significant bit position of the CPU1 DMA routing field.
static constexpr auto TOP_DMA_INTERRUPT_CPU1_SHIFT = 10U;
/// @brief CPU2 DMA 中断路由字段的最低位号。
///        Least-significant bit position of the CPU2 DMA routing field.
static constexpr auto TOP_DMA_INTERRUPT_CPU2_SHIFT = 20U;
/// @brief TOP 每 CPU 的 DMA 中断路由字段的位掩码。
///        Bit mask for TOP per-CPU DMA interrupt-routing field.
static constexpr auto TOP_DMA_INTERRUPT_FIELD_MASK = 0x1FFU;

/**
 * @brief TOP 系统控制与 DMA 路由寄存器。
 *        TOP system-control and DMA-routing registers.
 */
typedef struct
{
    uint32_t
        RESERVED_000[2]; ///< 偏移 0x000：未公开区域的占位空间。Offset 0x000: padding for an unexposed area.
    volatile uint32_t SYS_CTRL; ///< 偏移 0x008：系统控制寄存器。Offset 0x008: system-control register.
    uint32_t RESERVED_00C[(0x154U - 0x00CU) / sizeof(uint32_t)]; ///< 偏移 0x00C：未公开区域的占位空间。Offset
                                                                 ///< 0x00C: padding for an unexposed area.
    volatile uint32_t
        DMA_REMAP0; ///< 偏移 0x154：DMA 请求重映射寄存器 0。Offset 0x154: DMA request-remap register zero.
    volatile uint32_t
        DMA_REMAP1; ///< 偏移 0x158：DMA 请求重映射寄存器 1。Offset 0x158: DMA request-remap register one.
    uint32_t RESERVED_15C[(0x1A8U - 0x15CU) / sizeof(uint32_t)]; ///< 偏移 0x15C：未公开区域的占位空间。Offset
                                                                 ///< 0x15C: padding for an unexposed area.
    volatile uint32_t
        WDT_CTRL; ///< 偏移 0x1A8：看门狗复位路由控制。Offset 0x1A8: watchdog reset-routing control.
    uint32_t RESERVED_1AC
        [(TOP_SD1_SELECT_OFFSET - 0x1ACU) / sizeof(uint32_t)]; ///< 偏移 0x1AC：未公开区域的占位空间。Offset
                                                               ///< 0x1AC: padding for an unexposed area.
    volatile uint32_t SD1_SELECT; ///< 偏移 0x294：SD1 与替代 MIPI 引脚通路选择。Offset 0x294: SD1 versus
                                  ///< alternate MIPI pad-path selector.
    volatile uint32_t DMA_INTERRUPT_MUX; ///< 偏移 0x298：DMA 中断 CPU 路由字段。Offset 0x298: DMA interrupt
                                         ///< CPU-routing fields.
} TOP_Type;

/**
 * @brief TOP 寄存器实例指针。
 *        TOP register-instance pointer.
 */
#define TOP ((TOP_Type *)(uintptr_t)TOP_MISC_BASE)
static_assert(offsetof(TOP_Type, SYS_CTRL) == 0x008U);

/// @brief TOP 系统控制寄存器偏移。
///        TOP system-control register offset.
static constexpr auto TOP_SYS_CTRL_OFFSET = 0x008UL;
/// @brief 看门狗系统复位路由字段起始位。
///        First bit of the watchdog system-reset route field.
static constexpr auto TOP_WDT_SYSTEM_ROUTE_SHIFT = 0U;
/// @brief 看门狗 CPU 复位路由字段起始位。
///        First bit of the watchdog CPU-reset route field.
static constexpr auto TOP_WDT_CPU_ROUTE_SHIFT = 4U;
/// @brief 主域看门狗共享时钟选择字段。
///        Shared clock-selection field for main-domain watchdogs.
static constexpr auto TOP_WDT_CLOCK_SELECT_MASK = (7UL << 8);
/// @brief 主域看门狗共享时钟选择 32 kHz 的编码。
///        32 kHz encoding of the shared main-domain watchdog clock selector.
static constexpr auto TOP_WDT_CLOCK_32K = (1UL << 8);

/**
 * @brief Sophgo 看门狗驱动使用的系统控制兼容使能位。
 *        System-control compatibility enable used by the Sophgo watchdog driver.
 * @note 沿用 cvitek_wdt 的 CV_TOP_WDT_VAL=0x100；TRM 表 9.4 将此位列为保留位。
 *       Retains cvitek_wdt CV_TOP_WDT_VAL=0x100; TRM Table 9.4 lists this bit as reserved.
 */
static constexpr auto TOP_SYS_CTRL_WDT_COMPAT_ENABLE_BIT = (1UL << 8);

static_assert(offsetof(TOP_Type, WDT_CTRL) == 0x1A8U);
static_assert(offsetof(TOP_Type, SD1_SELECT) == TOP_SD1_SELECT_OFFSET);
static_assert(offsetof(TOP_Type, DMA_REMAP0) == 0x154U);
static_assert(offsetof(TOP_Type, DMA_REMAP1) == TOP_DMA_REMAP1_OFFSET);
static_assert(offsetof(TOP_Type, DMA_INTERRUPT_MUX) == TOP_DMA_INTERRUPT_MUX_OFFSET);

/** @} */

/**
 * @defgroup SG2002_CLKGEN CLKGEN 门控、选源与分频 / CLKGEN clock gates, muxes and dividers
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02, Chapter 8。SG2002 TRM v1.02, Chapter 8.
 * @{
 */

/**
 * @name 时钟控制寄存器偏移 / Clock control register offsets
 * 时钟树使用的寄存器偏移。
 *        Offsets used by the clock tree.
 * @{
 */
/// @brief CLK_EN0 时钟控制寄存器的字节偏移。
///        Byte offset of CLK_EN0 clock-control register.
static constexpr auto CLKGEN_CLK_EN0_OFFSET = 0x000UL;
/// @brief CLK_EN1 时钟控制寄存器的字节偏移。
///        Byte offset of CLK_EN1 clock-control register.
static constexpr auto CLKGEN_CLK_EN1_OFFSET = 0x004UL;
/// @brief CLK_EN2 时钟控制寄存器的字节偏移。
///        Byte offset of CLK_EN2 clock-control register.
static constexpr auto CLKGEN_CLK_EN2_OFFSET = 0x008UL;
/// @brief CLK_EN3 时钟控制寄存器的字节偏移。
///        Byte offset of CLK_EN3 clock-control register.
static constexpr auto CLKGEN_CLK_EN3_OFFSET = 0x00CUL;
/// @brief CLK_EN4 时钟控制寄存器的字节偏移。
///        Byte offset of CLK_EN4 clock-control register.
static constexpr auto CLKGEN_CLK_EN4_OFFSET = 0x010UL;
/// @brief CLK_SEL0 时钟控制寄存器的字节偏移。
///        Byte offset of CLK_SEL0 clock-control register.
static constexpr auto CLKGEN_CLK_SEL0_OFFSET = 0x020UL;
/// @brief CLK_BYP0 时钟控制寄存器的字节偏移。
///        Byte offset of CLK_BYP0 clock-control register.
static constexpr auto CLKGEN_CLK_BYP0_OFFSET = 0x030UL;
/// @brief CLK_BYP1 时钟控制寄存器的字节偏移。
///        Byte offset of CLK_BYP1 clock-control register.
static constexpr auto CLKGEN_CLK_BYP1_OFFSET = 0x034UL;
/** @} */

/**
 * @name 分频器寄存器偏移 / Divider register offsets
 * 这些器件定义由 LL RCC 操作共同使用。
 *        These device definitions are shared with the LL RCC operations.
 * @see SG2002 技术参考手册 v1.02, 表 8.51, 8.79, 8.88-8.90, and 8.93。SG2002 TRM v1.02,
 * Tables 8.51, 8.79, 8.88-8.90, and 8.93.
 * @{
 */
/// @brief CPU_AXI0 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the CPU_AXI0 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_CPU_AXI0 = 0x048UL;
/// @brief GPIO_DB 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the GPIO_DB divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_GPIO_DB = 0x094UL;
/// @brief SDMA_AUD0 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the SDMA_AUD0 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_SDMA_AUD0 = 0x098UL;
/// @brief SDMA_AUD1 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the SDMA_AUD1 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_SDMA_AUD1 = 0x09CUL;
/// @brief SDMA_AUD2 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the SDMA_AUD2 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_SDMA_AUD2 = 0x0A0UL;
/// @brief SDMA_AUD3 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the SDMA_AUD3 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_SDMA_AUD3 = 0x0A4UL;
/// @brief AXI4 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the AXI4 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_AXI4 = 0x0B8UL;
/// @brief AXI6 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the AXI6 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_AXI6 = 0x0BCUL;
/// @brief 1M 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the 1M divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_1M = 0x0FCUL;
/// @brief SPI 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the SPI divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_SPI = 0x100UL;
/// @brief I2C 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the I2C divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_I2C = 0x104UL;
/// @brief SD0 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the SD0 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_SD0 = 0x070UL;
/// @brief 100K_SD0 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the 100K_SD0 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_100K_SD0 = 0x078UL;
/// @brief SD1 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the SD1 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_SD1 = 0x07CUL;
/// @brief 100K_SD1 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the 100K_SD1 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_100K_SD1 = 0x084UL;
/// @brief PWM_SRC 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the PWM_SRC divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_PWM_SRC = 0x120UL;
/// @brief C906_0_0 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the C906_0_0 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_C906_0_0 = 0x130UL;
/// @brief C906_0_1 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the C906_0_1 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_C906_0_1 = 0x134UL;
/// @brief C906_1_0 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the C906_1_0 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_C906_1_0 = 0x138UL;
/// @brief C906_1_1 分频寄存器相对 CLKGEN 的字节偏移。
///        Byte offset of the C906_1_1 divider register from CLKGEN.
static constexpr auto CLKGEN_DIV_OFF_C906_1_1 = 0x13CUL;
/** @} */

/// @brief 分频器复位释放位。
///        Bit for divider reset release.
static constexpr auto CLKGEN_DIV_RESET_BIT = (1UL << 0);
/// @brief 分频器寄存器因子选择位。
///        Bit for divider register-factor selection.
static constexpr auto CLKGEN_DIV_USE_REG_FACTOR_BIT = (1UL << 3);
/// @brief 分频器父时钟选择字段的最低有效位位置。
///        Least-significant bit position of divider parent-clock selector.
static constexpr auto CLKGEN_DIV_SRC_SHIFT = 8U;
/// @brief 分频器父时钟选择字段的位宽。
///        Bit width of divider parent-clock selector.
static constexpr auto CLKGEN_DIV_SRC_WIDTH = 2U;
/// @brief 分频因子字段的最低有效位位置。
///        Least-significant bit position of divider-factor field.
static constexpr auto CLKGEN_DIV_FACTOR_SHIFT = 16U;

/// @brief FPLL 父时钟选择编码。
///        Parent-clock selector encoding for FPLL.
static constexpr auto CLKGEN_SRC_FPLL = 0UL;
/// @brief DISPPLL 父时钟选择编码。
///        Parent-clock selector encoding for DISPPLL.
static constexpr auto CLKGEN_SRC_DISPPLL = 1UL;

/**
 * @name 分频字段位宽与硬件初始因子 / Divider field widths and hardware initial factors
 * @note TRM 将因子字段通用标注为 [20:16]；以下各叶节点位宽采用 SG2002 时钟提供者的定义，属于器件契约。
 *        The TRM describes the factor field generically as [20:16]. The per-leaf widths below are the values
 * used by the SG2002 clock provider and form part of the device contract.
 * @{
 */
/// @brief AXI4 分频因子字段的位宽。
///        Bit width of the AXI4 divider-factor field.
static constexpr auto CLKGEN_DIV_AXI4_FACTOR_WIDTH = 4U;
/// @brief AXI4 分支的最大分频因子。
///        Maximum divider factor of the AXI4 branch.
static constexpr auto CLKGEN_AXI4_MAX_FACTOR = 15U;
/// @brief AXI4 分支的硬件初始分频因子。
///        Hardware initial divider factor of the AXI4 branch.
static constexpr auto CLKGEN_AXI4_INITIAL_FACTOR = 5U;
/// @brief CPU_AXI0 分频因子字段的位宽。
///        Bit width of the CPU_AXI0 divider-factor field.
static constexpr auto CLKGEN_DIV_CPU_AXI0_FACTOR_WIDTH = 4U;
/// @brief CPU_AXI0 分支的硬件初始分频因子。
///        Hardware initial divider factor of the CPU_AXI0 branch.
static constexpr auto CLKGEN_CPU_AXI0_INITIAL_FACTOR = 3U;
/// @brief AXI6 分频因子字段的位宽。
///        Bit width of the AXI6 divider-factor field.
static constexpr auto CLKGEN_DIV_AXI6_FACTOR_WIDTH = 4U;
/// @brief AXI6 分支的最大分频因子。
///        Maximum divider factor of the AXI6 branch.
static constexpr auto CLKGEN_AXI6_MAX_FACTOR = 15U;
/// @brief AXI6 分支的硬件初始分频因子。
///        Hardware initial divider factor of the AXI6 branch.
static constexpr auto CLKGEN_AXI6_INITIAL_FACTOR = 15U;
/// @brief SD 分频因子字段的位宽。
///        Bit width of the SD divider-factor field.
static constexpr auto CLKGEN_DIV_SD_FACTOR_WIDTH = 5U;
/// @brief SD 分支的硬件初始分频因子。
///        Hardware initial divider factor of the SD branch.
static constexpr auto CLKGEN_SD_INITIAL_FACTOR = 15U;
/// @brief SD_100K 分频因子字段的位宽。
///        Bit width of the SD_100K divider-factor field.
static constexpr auto CLKGEN_DIV_SD_100K_FACTOR_WIDTH = 8U;
/// @brief SD_100K 分支的硬件初始分频因子。
///        Hardware initial divider factor of the SD_100K branch.
static constexpr auto CLKGEN_SD_100K_INITIAL_FACTOR = 10U;
/// @brief 1M 分频因子字段的位宽。
///        Bit width of the 1M divider-factor field.
static constexpr auto CLKGEN_DIV_1M_FACTOR_WIDTH = 6U;
/// @brief 1M 分支的硬件初始分频因子。
///        Hardware initial divider factor of the 1M branch.
static constexpr auto CLKGEN_1M_INITIAL_FACTOR = 25U;
/// @brief SPI 分频因子字段的位宽。
///        Bit width of the SPI divider-factor field.
static constexpr auto CLKGEN_DIV_SPI_FACTOR_WIDTH = 6U;
/// @brief SPI 分支的硬件初始分频因子。
///        Hardware initial divider factor of the SPI branch.
static constexpr auto CLKGEN_SPI_INITIAL_FACTOR = 8U;
/// @brief I2C 分频因子字段的位宽。
///        Bit width of the I2C divider-factor field.
static constexpr auto CLKGEN_DIV_I2C_FACTOR_WIDTH = 4U;
/// @brief I2C 分支的硬件初始分频因子。
///        Hardware initial divider factor of the I2C branch.
static constexpr auto CLKGEN_I2C_INITIAL_FACTOR = 1U;
/// @brief PWM_SRC 分频因子字段的位宽。
///        Bit width of the PWM_SRC divider-factor field.
static constexpr auto CLKGEN_DIV_PWM_SRC_FACTOR_WIDTH = 6U;
/// @brief PWM_SRC 分支的硬件初始分频因子。
///        Hardware initial divider factor of the PWM_SRC branch.
static constexpr auto CLKGEN_PWM_SRC_INITIAL_FACTOR = 10U;
/// @brief C906 分频因子字段的位宽。
///        Bit width of the C906 divider-factor field.
static constexpr auto CLKGEN_DIV_C906_FACTOR_WIDTH = 4U;
/// @brief C906_0_0 分支的硬件初始分频因子。
///        Hardware initial divider factor of the C906_0_0 branch.
static constexpr auto CLKGEN_C906_0_0_INITIAL_FACTOR = 1U;
/// @brief C906_0_1 分支的硬件初始分频因子。
///        Hardware initial divider factor of the C906_0_1 branch.
static constexpr auto CLKGEN_C906_0_1_INITIAL_FACTOR = 2U;
/// @brief C906_1_0 分支的硬件初始分频因子。
///        Hardware initial divider factor of the C906_1_0 branch.
static constexpr auto CLKGEN_C906_1_0_INITIAL_FACTOR = 2U;
/// @brief C906_1_1 分支的硬件初始分频因子。
///        Hardware initial divider factor of the C906_1_1 branch.
static constexpr auto CLKGEN_C906_1_1_INITIAL_FACTOR = 2U;
/** @} */

/**
 * @name 时钟门控坐标 / Clock gate locations
 * 每个宏展开为（CLK_EN 寄存器偏移，位号）元组。
 *        Each macro expands to a (CLK_EN register offset, bit) tuple.
 * @see SG2002 技术参考手册 v1.02, 表 8.52-8.56。SG2002 TRM v1.02, Tables 8.52-8.56.
 * @{
 */
/**
 * @brief RTC_25M 时钟门控坐标（寄存器偏移，位号）。
 *        RTC_25M clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_RTC_25M CLKGEN_CLK_EN0_OFFSET, 8U
/**
 * @brief TEMPSEN 时钟门控坐标（寄存器偏移，位号）。
 *        TEMPSEN clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_TEMPSEN CLKGEN_CLK_EN0_OFFSET, 9U
/**
 * @brief SARADC 时钟门控坐标（寄存器偏移，位号）。
 *        SARADC clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_SARADC CLKGEN_CLK_EN0_OFFSET, 10U
/**
 * @brief EFUSE 时钟门控坐标（寄存器偏移，位号）。
 *        EFUSE clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_EFUSE CLKGEN_CLK_EN0_OFFSET, 11U
/**
 * @brief APB_EFUSE 时钟门控坐标（寄存器偏移，位号）。
 *        APB_EFUSE clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_EFUSE CLKGEN_CLK_EN0_OFFSET, 12U
/**
 * @brief XTAL_MISC 时钟门控坐标（寄存器偏移，位号）。
 *        XTAL_MISC clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_XTAL_MISC CLKGEN_CLK_EN0_OFFSET, 14U
/**
 * @brief APB_GPIO 时钟门控坐标（寄存器偏移，位号）。
 *        APB_GPIO clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_GPIO CLKGEN_CLK_EN0_OFFSET, 29U
/**
 * @brief APB_GPIO_INTR 时钟门控坐标（寄存器偏移，位号）。
 *        APB_GPIO_INTR clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_GPIO_INTR CLKGEN_CLK_EN0_OFFSET, 30U
/**
 * @brief GPIO_DB 时钟门控坐标（寄存器偏移，位号）。
 *        GPIO_DB clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_GPIO_DB CLKGEN_CLK_EN0_OFFSET, 31U
/**
 * @brief SDMA_AXI 时钟门控坐标（寄存器偏移，位号）。
 *        SDMA_AXI clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_SDMA_AXI CLKGEN_CLK_EN1_OFFSET, 1U
/**
 * @brief SDMA_AUD0 时钟门控坐标（寄存器偏移，位号）。
 *        SDMA_AUD0 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_SDMA_AUD0 CLKGEN_CLK_EN1_OFFSET, 2U
/**
 * @brief SDMA_AUD1 时钟门控坐标（寄存器偏移，位号）。
 *        SDMA_AUD1 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_SDMA_AUD1 CLKGEN_CLK_EN1_OFFSET, 3U
/**
 * @brief SDMA_AUD2 时钟门控坐标（寄存器偏移，位号）。
 *        SDMA_AUD2 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_SDMA_AUD2 CLKGEN_CLK_EN1_OFFSET, 4U
/**
 * @brief SDMA_AUD3 时钟门控坐标（寄存器偏移，位号）。
 *        SDMA_AUD3 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_SDMA_AUD3 CLKGEN_CLK_EN1_OFFSET, 5U
/**
 * @brief APB_I2C 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2C clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2C CLKGEN_CLK_EN1_OFFSET, 6U
/**
 * @brief APB_WDT 时钟门控坐标（寄存器偏移，位号）。
 *        APB_WDT clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_WDT CLKGEN_CLK_EN1_OFFSET, 7U
/**
 * @brief APB_PWM 时钟门控坐标（寄存器偏移，位号）。
 *        APB_PWM clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_PWM CLKGEN_CLK_EN1_OFFSET, 8U
/**
 * @brief APB_SPI0 时钟门控坐标（寄存器偏移，位号）。
 *        APB_SPI0 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_SPI0 CLKGEN_CLK_EN1_OFFSET, 9U
/**
 * @brief APB_SPI1 时钟门控坐标（寄存器偏移，位号）。
 *        APB_SPI1 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_SPI1 CLKGEN_CLK_EN1_OFFSET, 10U
/**
 * @brief APB_SPI2 时钟门控坐标（寄存器偏移，位号）。
 *        APB_SPI2 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_SPI2 CLKGEN_CLK_EN1_OFFSET, 11U
/**
 * @brief APB_SPI3 时钟门控坐标（寄存器偏移，位号）。
 *        APB_SPI3 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_SPI3 CLKGEN_CLK_EN1_OFFSET, 12U
/**
 * @brief UART0 时钟门控坐标（寄存器偏移，位号）。
 *        UART0 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_UART0 CLKGEN_CLK_EN1_OFFSET, 14U
/**
 * @brief APB_UART0 时钟门控坐标（寄存器偏移，位号）。
 *        APB_UART0 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_UART0 CLKGEN_CLK_EN1_OFFSET, 15U
/**
 * @brief UART1 时钟门控坐标（寄存器偏移，位号）。
 *        UART1 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_UART1 CLKGEN_CLK_EN1_OFFSET, 16U
/**
 * @brief APB_UART1 时钟门控坐标（寄存器偏移，位号）。
 *        APB_UART1 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_UART1 CLKGEN_CLK_EN1_OFFSET, 17U
/**
 * @brief UART2 时钟门控坐标（寄存器偏移，位号）。
 *        UART2 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_UART2 CLKGEN_CLK_EN1_OFFSET, 18U
/**
 * @brief APB_UART2 时钟门控坐标（寄存器偏移，位号）。
 *        APB_UART2 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_UART2 CLKGEN_CLK_EN1_OFFSET, 19U
/**
 * @brief UART3 时钟门控坐标（寄存器偏移，位号）。
 *        UART3 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_UART3 CLKGEN_CLK_EN1_OFFSET, 20U
/**
 * @brief APB_UART3 时钟门控坐标（寄存器偏移，位号）。
 *        APB_UART3 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_UART3 CLKGEN_CLK_EN1_OFFSET, 21U
/**
 * @brief UART4 时钟门控坐标（寄存器偏移，位号）。
 *        UART4 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_UART4 CLKGEN_CLK_EN1_OFFSET, 22U
/**
 * @brief APB_UART4 时钟门控坐标（寄存器偏移，位号）。
 *        APB_UART4 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_UART4 CLKGEN_CLK_EN1_OFFSET, 23U
/**
 * @brief APB_I2S0 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2S0 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2S0 CLKGEN_CLK_EN1_OFFSET, 24U
/**
 * @brief APB_I2S1 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2S1 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2S1 CLKGEN_CLK_EN1_OFFSET, 25U
/**
 * @brief APB_I2S2 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2S2 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2S2 CLKGEN_CLK_EN1_OFFSET, 26U
/**
 * @brief APB_I2S3 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2S3 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2S3 CLKGEN_CLK_EN1_OFFSET, 27U
/**
 * @brief AXI4_USB 时钟门控坐标（寄存器偏移，位号）。
 *        AXI4_USB clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_AXI4_USB CLKGEN_CLK_EN1_OFFSET, 28U
/**
 * @brief APB_USB 时钟门控坐标（寄存器偏移，位号）。
 *        APB_USB clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_USB CLKGEN_CLK_EN1_OFFSET, 29U
/**
 * @brief AXI4 时钟门控坐标（寄存器偏移，位号）。
 *        AXI4 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_AXI4 CLKGEN_CLK_EN2_OFFSET, 1U
/**
 * @brief AXI6 时钟门控坐标（寄存器偏移，位号）。
 *        AXI6 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_AXI6 CLKGEN_CLK_EN2_OFFSET, 2U
/**
 * @brief 1M 时钟门控坐标（寄存器偏移，位号）。
 *        1M clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_1M CLKGEN_CLK_EN3_OFFSET, 5U
/**
 * @brief SPI 时钟门控坐标（寄存器偏移，位号）。
 *        SPI clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_SPI CLKGEN_CLK_EN3_OFFSET, 6U
/**
 * @brief I2C 时钟门控坐标（寄存器偏移，位号）。
 *        I2C clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_I2C CLKGEN_CLK_EN3_OFFSET, 7U
/**
 * @brief PM 时钟门控坐标（寄存器偏移，位号）。
 *        PM clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_PM CLKGEN_CLK_EN3_OFFSET, 8U
/**
 * @brief TIMER0 时钟门控坐标（寄存器偏移，位号）。
 *        TIMER0 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_TIMER0 CLKGEN_CLK_EN3_OFFSET, 9U
/**
 * @brief TIMER1 时钟门控坐标（寄存器偏移，位号）。
 *        TIMER1 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_TIMER1 CLKGEN_CLK_EN3_OFFSET, 10U
/**
 * @brief TIMER2 时钟门控坐标（寄存器偏移，位号）。
 *        TIMER2 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_TIMER2 CLKGEN_CLK_EN3_OFFSET, 11U
/**
 * @brief TIMER3 时钟门控坐标（寄存器偏移，位号）。
 *        TIMER3 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_TIMER3 CLKGEN_CLK_EN3_OFFSET, 12U
/**
 * @brief TIMER4 时钟门控坐标（寄存器偏移，位号）。
 *        TIMER4 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_TIMER4 CLKGEN_CLK_EN3_OFFSET, 13U
/**
 * @brief TIMER5 时钟门控坐标（寄存器偏移，位号）。
 *        TIMER5 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_TIMER5 CLKGEN_CLK_EN3_OFFSET, 14U
/**
 * @brief TIMER6 时钟门控坐标（寄存器偏移，位号）。
 *        TIMER6 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_TIMER6 CLKGEN_CLK_EN3_OFFSET, 15U
/**
 * @brief TIMER7 时钟门控坐标（寄存器偏移，位号）。
 *        TIMER7 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_TIMER7 CLKGEN_CLK_EN3_OFFSET, 16U
/**
 * @brief APB_I2C0 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2C0 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2C0 CLKGEN_CLK_EN3_OFFSET, 17U
/**
 * @brief APB_I2C1 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2C1 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2C1 CLKGEN_CLK_EN3_OFFSET, 18U
/**
 * @brief APB_I2C2 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2C2 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2C2 CLKGEN_CLK_EN3_OFFSET, 19U
/**
 * @brief APB_I2C3 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2C3 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2C3 CLKGEN_CLK_EN3_OFFSET, 20U
/**
 * @brief APB_I2C4 时钟门控坐标（寄存器偏移，位号）。
 *        APB_I2C4 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_I2C4 CLKGEN_CLK_EN3_OFFSET, 21U
/**
 * @brief WGN 时钟门控坐标（寄存器偏移，位号）。
 *        WGN clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_WGN CLKGEN_CLK_EN3_OFFSET, 22U
/**
 * @brief WGN0 时钟门控坐标（寄存器偏移，位号）。
 *        WGN0 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_WGN0 CLKGEN_CLK_EN3_OFFSET, 23U
/**
 * @brief WGN1 时钟门控坐标（寄存器偏移，位号）。
 *        WGN1 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_WGN1 CLKGEN_CLK_EN3_OFFSET, 24U
/**
 * @brief WGN2 时钟门控坐标（寄存器偏移，位号）。
 *        WGN2 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_WGN2 CLKGEN_CLK_EN3_OFFSET, 25U
/**
 * @brief KEYSCAN 时钟门控坐标（寄存器偏移，位号）。
 *        KEYSCAN clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_KEYSCAN CLKGEN_CLK_EN3_OFFSET, 26U
/**
 * @brief AHB_SF1 时钟门控坐标（寄存器偏移，位号）。
 *        AHB_SF1 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_AHB_SF1 CLKGEN_CLK_EN3_OFFSET, 27U
/**
 * @brief AUDSRC 时钟门控坐标（寄存器偏移，位号）。
 *        AUDSRC clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_AUDSRC CLKGEN_CLK_EN4_OFFSET, 1U
/**
 * @brief APB_AUDSRC 时钟门控坐标（寄存器偏移，位号）。
 *        APB_AUDSRC clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_APB_AUDSRC CLKGEN_CLK_EN4_OFFSET, 2U
/**
 * @brief PWM_SRC 时钟门控坐标（寄存器偏移，位号）。
 *        PWM_SRC clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_PWM_SRC CLKGEN_CLK_EN4_OFFSET, 4U
/**
 * @brief RTCSYS_SRC0 时钟门控坐标（寄存器偏移，位号）。
 *        RTCSYS_SRC0 clock-gate coordinates (register offset, bit index).
 */
#define CLKGEN_GATE_RTCSYS_SRC0 CLKGEN_CLK_EN4_OFFSET, 6U
/** @} */

/**
 * @name 父时钟旁路与选源 / Clock parent bypass and source selection
 * @{
 */
/**
 * @brief PWM_SRC 晶振旁路坐标（寄存器偏移，位号）。
 *        PWM_SRC crystal-bypass coordinates (register offset, bit index).
 */
#define CLKGEN_BYPASS_PWM_SRC CLKGEN_CLK_BYP0_OFFSET, 15U
/**
 * @brief AXI4 晶振旁路坐标（寄存器偏移，位号）。
 *        AXI4 crystal-bypass coordinates (register offset, bit index).
 */
#define CLKGEN_BYPASS_AXI4 CLKGEN_CLK_BYP0_OFFSET, 19U
/**
 * @brief AXI6 晶振旁路坐标（寄存器偏移，位号）。
 *        AXI6 crystal-bypass coordinates (register offset, bit index).
 */
#define CLKGEN_BYPASS_AXI6 CLKGEN_CLK_BYP0_OFFSET, 20U
/**
 * @brief SPI 晶振旁路坐标（寄存器偏移，位号）。
 *        SPI crystal-bypass coordinates (register offset, bit index).
 */
#define CLKGEN_BYPASS_SPI CLKGEN_CLK_BYP0_OFFSET, 30U
/**
 * @brief I2C 晶振旁路坐标（寄存器偏移，位号）。
 *        I2C crystal-bypass coordinates (register offset, bit index).
 */
#define CLKGEN_BYPASS_I2C CLKGEN_CLK_BYP0_OFFSET, 31U
/// @brief A53 核心时钟选源位掩码。
///        A53 core-clock selector bit mask.
static constexpr auto CLKGEN_SEL0_A53_BIT = (1UL << 0);
/// @brief C906_0 核心时钟选源位掩码。
///        C906_0 core-clock selector bit mask.
static constexpr auto CLKGEN_SEL0_C906_0_BIT = (1UL << 23);
/// @brief C906_1 核心时钟选源位掩码。
///        C906_1 core-clock selector bit mask.
static constexpr auto CLKGEN_SEL0_C906_1_BIT = (1UL << 24);
/// @brief A53 核心时钟选源字段位号。
///        A53 core-clock selector bit position.
static constexpr auto CLKGEN_SEL0_A53_SHIFT = 0U;
/// @brief C906_0 核心时钟选源字段位号。
///        C906_0 core-clock selector bit position.
static constexpr auto CLKGEN_SEL0_C906_0_SHIFT = 23U;
/// @brief C906_1 核心时钟选源字段位号。
///        C906_1 core-clock selector bit position.
static constexpr auto CLKGEN_SEL0_C906_1_SHIFT = 24U;
/** @} */

/**
 * @brief 时钟发生器寄存器块。
 *        Clock-generator register block.
 *
 * @note 占位区域不作为可访问寄存器公开。
 *       Padding areas are not exposed as accessible registers.
 * @see SG2002 技术参考手册 v1.02, 表 8.51。SG2002 TRM v1.02, Table 8.51
 */
typedef struct
{
    volatile uint32_t CLK_EN[5]; ///< 偏移 0x000：时钟门控寄存器数组。Offset 0x000: clock-gate register array.
    uint32_t
        RESERVED_014[3]; ///< 偏移 0x014：未公开区域的占位空间。Offset 0x014: padding for an unexposed area.
    volatile uint32_t CLK_SEL0; ///< 偏移 0x020：核心时钟选源寄存器。Offset 0x020: core clock-source selector.
    uint32_t
        RESERVED_024[3]; ///< 偏移 0x024：未公开区域的占位空间。Offset 0x024: padding for an unexposed area.
    volatile uint32_t CLK_BYP0; ///< 偏移 0x030：时钟旁路寄存器 0。Offset 0x030: clock-bypass register zero.
    volatile uint32_t CLK_BYP1; ///< 偏移 0x034：时钟旁路寄存器 1。Offset 0x034: clock-bypass register one.
    uint32_t
        RESERVED_038[2]; ///< 偏移 0x038：未公开区域的占位空间。Offset 0x038: padding for an unexposed area.
    volatile uint32_t
        DIV_A53_0; ///< 偏移 0x040：A53_0 时钟分频寄存器。Offset 0x040: clock-divider register for A53_0.
    volatile uint32_t
        DIV_A53_1; ///< 偏移 0x044：A53_1 时钟分频寄存器。Offset 0x044: clock-divider register for A53_1.
    volatile uint32_t DIV_CPU_AXI0; ///< 偏移 0x048：CPU_AXI0 时钟分频寄存器。Offset 0x048: clock-divider
                                    ///< register for CPU_AXI0.
    uint32_t RESERVED_04C; ///< 偏移 0x04C：未公开区域的占位空间。Offset 0x04C: padding for an unexposed area.
    volatile uint32_t DIV_CPU_GIC; ///< 偏移 0x050：CPU_GIC 时钟分频寄存器。Offset 0x050: clock-divider
                                   ///< register for CPU_GIC.
    volatile uint32_t
        DIV_TPU; ///< 偏移 0x054：TPU 时钟分频寄存器。Offset 0x054: clock-divider register for TPU.
    uint32_t
        RESERVED_058[3]; ///< 偏移 0x058：未公开区域的占位空间。Offset 0x058: padding for an unexposed area.
    volatile uint32_t
        DIV_EMMC; ///< 偏移 0x064：EMMC 时钟分频寄存器。Offset 0x064: clock-divider register for EMMC.
    uint32_t RESERVED_068; ///< 偏移 0x068：未公开区域的占位空间。Offset 0x068: padding for an unexposed area.
    volatile uint32_t DIV_100K_EMMC; ///< 偏移 0x06C：100K_EMMC 时钟分频寄存器。Offset 0x06C: clock-divider
                                     ///< register for 100K_EMMC.
    volatile uint32_t
        DIV_SD0;           ///< 偏移 0x070：SD0 时钟分频寄存器。Offset 0x070: clock-divider register for SD0.
    uint32_t RESERVED_074; ///< 偏移 0x074：未公开区域的占位空间。Offset 0x074: padding for an unexposed area.
    volatile uint32_t DIV_100K_SD0; ///< 偏移 0x078：100K_SD0 时钟分频寄存器。Offset 0x078: clock-divider
                                    ///< register for 100K_SD0.
    volatile uint32_t
        DIV_SD1;           ///< 偏移 0x07C：SD1 时钟分频寄存器。Offset 0x07C: clock-divider register for SD1.
    uint32_t RESERVED_080; ///< 偏移 0x080：未公开区域的占位空间。Offset 0x080: padding for an unexposed area.
    volatile uint32_t DIV_100K_SD1;  ///< 偏移 0x084：100K_SD1 时钟分频寄存器。Offset 0x084: clock-divider
                                     ///< register for 100K_SD1.
    volatile uint32_t DIV_SPI_NAND;  ///< 偏移 0x088：SPI_NAND 时钟分频寄存器。Offset 0x088: clock-divider
                                     ///< register for SPI_NAND.
    volatile uint32_t DIV_500M_ETH0; ///< 偏移 0x08C：500M_ETH0 时钟分频寄存器。Offset 0x08C: clock-divider
                                     ///< register for 500M_ETH0.
    uint32_t RESERVED_090; ///< 偏移 0x090：未公开区域的占位空间。Offset 0x090: padding for an unexposed area.
    volatile uint32_t DIV_GPIO_DB;     ///< 偏移 0x094：GPIO_DB 时钟分频寄存器。Offset 0x094: clock-divider
                                       ///< register for GPIO_DB.
    volatile uint32_t DIV_SDMA_AUD[4]; ///< 偏移 0x098：SDMA_AUD 时钟分频寄存器。Offset 0x098: clock-divider
                                       ///< register for SDMA_AUD.
    volatile uint32_t DIV_CAM0_200;    ///< 偏移 0x0A8：CAM0_200 时钟分频寄存器。Offset 0x0A8: clock-divider
                                       ///< register for CAM0_200.
    uint32_t
        RESERVED_0AC[3]; ///< 偏移 0x0AC：未公开区域的占位空间。Offset 0x0AC: padding for an unexposed area.
    volatile uint32_t
        DIV_AXI4; ///< 偏移 0x0B8：AXI4 时钟分频寄存器。Offset 0x0B8: clock-divider register for AXI4.
    volatile uint32_t
        DIV_AXI6; ///< 偏移 0x0BC：AXI6 时钟分频寄存器。Offset 0x0BC: clock-divider register for AXI6.
    uint32_t RESERVED_0C0; ///< 偏移 0x0C0：未公开区域的占位空间。Offset 0x0C0: padding for an unexposed area.
    volatile uint32_t DIV_DSI_ESC; ///< 偏移 0x0C4：DSI_ESC 时钟分频寄存器。Offset 0x0C4: clock-divider
                                   ///< register for DSI_ESC.
    volatile uint32_t DIV_AXI_VIP; ///< 偏移 0x0C8：AXI_VIP 时钟分频寄存器。Offset 0x0C8: clock-divider
                                   ///< register for AXI_VIP.
    uint32_t RESERVED_0CC; ///< 偏移 0x0CC：未公开区域的占位空间。Offset 0x0CC: padding for an unexposed area.
    volatile uint32_t DIV_SRC_VIP_SYS_0; ///< 偏移 0x0D0：SRC_VIP_SYS_0 时钟分频寄存器。Offset 0x0D0:
                                         ///< clock-divider register for SRC_VIP_SYS_0.
    uint32_t RESERVED_0D4; ///< 偏移 0x0D4：未公开区域的占位空间。Offset 0x0D4: padding for an unexposed area.
    volatile uint32_t DIV_SRC_VIP_SYS_1; ///< 偏移 0x0D8：SRC_VIP_SYS_1 时钟分频寄存器。Offset 0x0D8:
                                         ///< clock-divider register for SRC_VIP_SYS_1.
    uint32_t RESERVED_0DC; ///< 偏移 0x0DC：未公开区域的占位空间。Offset 0x0DC: padding for an unexposed area.
    volatile uint32_t DIV_DISP_SRC_VIP;    ///< 偏移 0x0E0：DISP_SRC_VIP 时钟分频寄存器。Offset 0x0E0:
                                           ///< clock-divider register for DISP_SRC_VIP.
    volatile uint32_t DIV_AXI_VIDEO_CODEC; ///< 偏移 0x0E4：AXI_VIDEO_CODEC 时钟分频寄存器。Offset 0x0E4:
                                           ///< clock-divider register for AXI_VIDEO_CODEC.
    uint32_t RESERVED_0E8; ///< 偏移 0x0E8：未公开区域的占位空间。Offset 0x0E8: padding for an unexposed area.
    volatile uint32_t DIV_VC_SRC0; ///< 偏移 0x0EC：VC_SRC0 时钟分频寄存器。Offset 0x0EC: clock-divider
                                   ///< register for VC_SRC0.
    uint32_t
        RESERVED_0F0[3]; ///< 偏移 0x0F0：未公开区域的占位空间。Offset 0x0F0: padding for an unexposed area.
    volatile uint32_t DIV_1M; ///< 偏移 0x0FC：1M 时钟分频寄存器。Offset 0x0FC: clock-divider register for 1M.
    volatile uint32_t
        DIV_SPI; ///< 偏移 0x100：SPI 时钟分频寄存器。Offset 0x100: clock-divider register for SPI.
    volatile uint32_t
        DIV_I2C; ///< 偏移 0x104：I2C 时钟分频寄存器。Offset 0x104: clock-divider register for I2C.
    uint32_t
        RESERVED_108[2]; ///< 偏移 0x108：未公开区域的占位空间。Offset 0x108: padding for an unexposed area.
    volatile uint32_t DIV_SRC_VIP_SYS_2; ///< 偏移 0x110：SRC_VIP_SYS_2 时钟分频寄存器。Offset 0x110:
                                         ///< clock-divider register for SRC_VIP_SYS_2.
    uint32_t RESERVED_114; ///< 偏移 0x114：未公开区域的占位空间。Offset 0x114: padding for an unexposed area.
    volatile uint32_t
        DIV_AUDSRC; ///< 偏移 0x118：AUDSRC 时钟分频寄存器。Offset 0x118: clock-divider register for AUDSRC.
    uint32_t RESERVED_11C; ///< 偏移 0x11C：未公开区域的占位空间。Offset 0x11C: padding for an unexposed area.
    volatile uint32_t DIV_PWM_SRC; ///< 偏移 0x120：PWM_SRC 时钟分频寄存器。Offset 0x120: clock-divider
                                   ///< register for PWM_SRC.
    uint32_t RESERVED_124; ///< 偏移 0x124：未公开区域的占位空间。Offset 0x124: padding for an unexposed area.
    volatile uint32_t DIV_AP_DEBUG;     ///< 偏移 0x128：AP_DEBUG 时钟分频寄存器。Offset 0x128: clock-divider
                                        ///< register for AP_DEBUG.
    volatile uint32_t DIV_RTCSYS_SRC_0; ///< 偏移 0x12C：RTCSYS_SRC_0 时钟分频寄存器。Offset 0x12C:
                                        ///< clock-divider register for RTCSYS_SRC_0.
    volatile uint32_t
        DIV_C906[4]; ///< 偏移 0x130：C906 时钟分频寄存器。Offset 0x130: clock-divider register for C906.
    volatile uint32_t DIV_SRC_VIP_SYS_3; ///< 偏移 0x140：SRC_VIP_SYS_3 时钟分频寄存器。Offset 0x140:
                                         ///< clock-divider register for SRC_VIP_SYS_3.
    volatile uint32_t DIV_SRC_VIP_SYS_4; ///< 偏移 0x144：SRC_VIP_SYS_4 时钟分频寄存器。Offset 0x144:
                                         ///< clock-divider register for SRC_VIP_SYS_4.
} CLKGEN_Type;

/**
 * @brief CLKGEN 寄存器实例指针。
 *        CLKGEN register-instance pointer.
 */
#define CLKGEN ((CLKGEN_Type *)(uintptr_t)CLKGEN_BASE)
static_assert(offsetof(CLKGEN_Type, CLK_EN[0]) == 0x000U);
static_assert(offsetof(CLKGEN_Type, CLK_EN[1]) == 0x004U);
static_assert(offsetof(CLKGEN_Type, CLK_EN[2]) == 0x008U);
static_assert(offsetof(CLKGEN_Type, CLK_EN[3]) == 0x00CU);
static_assert(offsetof(CLKGEN_Type, CLK_EN[4]) == 0x010U);
static_assert(offsetof(CLKGEN_Type, CLK_SEL0) == 0x020U);
static_assert(offsetof(CLKGEN_Type, CLK_BYP0) == 0x030U);
static_assert(offsetof(CLKGEN_Type, CLK_BYP1) == 0x034U);
static_assert(offsetof(CLKGEN_Type, DIV_A53_0) == 0x040U);
static_assert(offsetof(CLKGEN_Type, DIV_A53_1) == 0x044U);
static_assert(offsetof(CLKGEN_Type, DIV_CPU_AXI0) == 0x048U);
static_assert(offsetof(CLKGEN_Type, DIV_CPU_GIC) == 0x050U);
static_assert(offsetof(CLKGEN_Type, DIV_TPU) == 0x054U);
static_assert(offsetof(CLKGEN_Type, DIV_EMMC) == 0x064U);
static_assert(offsetof(CLKGEN_Type, DIV_100K_EMMC) == 0x06CU);
static_assert(offsetof(CLKGEN_Type, DIV_SD0) == 0x070U);
static_assert(offsetof(CLKGEN_Type, DIV_100K_SD0) == 0x078U);
static_assert(offsetof(CLKGEN_Type, DIV_SD1) == 0x07CU);
static_assert(offsetof(CLKGEN_Type, DIV_100K_SD1) == 0x084U);
static_assert(offsetof(CLKGEN_Type, DIV_SPI_NAND) == 0x088U);
static_assert(offsetof(CLKGEN_Type, DIV_500M_ETH0) == 0x08CU);
static_assert(offsetof(CLKGEN_Type, DIV_GPIO_DB) == 0x094U);
static_assert(offsetof(CLKGEN_Type, DIV_SDMA_AUD[0]) == 0x098U);
static_assert(offsetof(CLKGEN_Type, DIV_SDMA_AUD[1]) == 0x09CU);
static_assert(offsetof(CLKGEN_Type, DIV_SDMA_AUD[2]) == 0x0A0U);
static_assert(offsetof(CLKGEN_Type, DIV_SDMA_AUD[3]) == 0x0A4U);
static_assert(offsetof(CLKGEN_Type, DIV_CAM0_200) == 0x0A8U);
static_assert(offsetof(CLKGEN_Type, DIV_AXI4) == 0x0B8U);
static_assert(offsetof(CLKGEN_Type, DIV_AXI6) == 0x0BCU);
static_assert(offsetof(CLKGEN_Type, DIV_DSI_ESC) == 0x0C4U);
static_assert(offsetof(CLKGEN_Type, DIV_AXI_VIP) == 0x0C8U);
static_assert(offsetof(CLKGEN_Type, DIV_SRC_VIP_SYS_0) == 0x0D0U);
static_assert(offsetof(CLKGEN_Type, DIV_SRC_VIP_SYS_1) == 0x0D8U);
static_assert(offsetof(CLKGEN_Type, DIV_DISP_SRC_VIP) == 0x0E0U);
static_assert(offsetof(CLKGEN_Type, DIV_AXI_VIDEO_CODEC) == 0x0E4U);
static_assert(offsetof(CLKGEN_Type, DIV_VC_SRC0) == 0x0ECU);
static_assert(offsetof(CLKGEN_Type, DIV_1M) == 0x0FCU);
static_assert(offsetof(CLKGEN_Type, DIV_SPI) == 0x100U);
static_assert(offsetof(CLKGEN_Type, DIV_I2C) == 0x104U);
static_assert(offsetof(CLKGEN_Type, DIV_SRC_VIP_SYS_2) == 0x110U);
static_assert(offsetof(CLKGEN_Type, DIV_AUDSRC) == 0x118U);
static_assert(offsetof(CLKGEN_Type, DIV_PWM_SRC) == 0x120U);
static_assert(offsetof(CLKGEN_Type, DIV_AP_DEBUG) == 0x128U);
static_assert(offsetof(CLKGEN_Type, DIV_RTCSYS_SRC_0) == 0x12CU);
static_assert(offsetof(CLKGEN_Type, DIV_C906[0]) == 0x130U);
static_assert(offsetof(CLKGEN_Type, DIV_C906[1]) == 0x134U);
static_assert(offsetof(CLKGEN_Type, DIV_C906[2]) == 0x138U);
static_assert(offsetof(CLKGEN_Type, DIV_C906[3]) == 0x13CU);
static_assert(offsetof(CLKGEN_Type, DIV_SRC_VIP_SYS_3) == 0x140U);
static_assert(offsetof(CLKGEN_Type, DIV_SRC_VIP_SYS_4) == 0x144U);
static_assert(offsetof(CLKGEN_Type, DIV_AXI4) == 0x0B8U);
static_assert(offsetof(CLKGEN_Type, DIV_C906) == 0x130U);

/**
 * @brief 为静态校验和运行时选择展开具名 CLKGEN 寄存器。
 *        Expand named CLKGEN registers for static validation and runtime selection.
 *
 * @param X 接受一个 CLKGEN_Type 成员表达式的展开宏。
 *        Expansion macro accepting one CLKGEN_Type member designator.
 */
#define SG2002_CLKGEN_REGISTERS(X)                                                                           \
    X(CLK_EN[0])                                                                                             \
    X(CLK_EN[1])                                                                                             \
    X(CLK_EN[2])                                                                                             \
    X(CLK_EN[3])                                                                                             \
    X(CLK_EN[4])                                                                                             \
    X(CLK_SEL0)                                                                                              \
    X(CLK_BYP0)                                                                                              \
    X(CLK_BYP1)                                                                                              \
    X(DIV_A53_0)                                                                                             \
    X(DIV_A53_1)                                                                                             \
    X(DIV_CPU_AXI0)                                                                                          \
    X(DIV_CPU_GIC)                                                                                           \
    X(DIV_TPU)                                                                                               \
    X(DIV_EMMC)                                                                                              \
    X(DIV_100K_EMMC)                                                                                         \
    X(DIV_SD0)                                                                                               \
    X(DIV_100K_SD0)                                                                                          \
    X(DIV_SD1)                                                                                               \
    X(DIV_100K_SD1)                                                                                          \
    X(DIV_SPI_NAND)                                                                                          \
    X(DIV_500M_ETH0)                                                                                         \
    X(DIV_GPIO_DB)                                                                                           \
    X(DIV_SDMA_AUD[0])                                                                                       \
    X(DIV_SDMA_AUD[1])                                                                                       \
    X(DIV_SDMA_AUD[2])                                                                                       \
    X(DIV_SDMA_AUD[3])                                                                                       \
    X(DIV_CAM0_200)                                                                                          \
    X(DIV_AXI4)                                                                                              \
    X(DIV_AXI6)                                                                                              \
    X(DIV_DSI_ESC)                                                                                           \
    X(DIV_AXI_VIP)                                                                                           \
    X(DIV_SRC_VIP_SYS_0)                                                                                     \
    X(DIV_SRC_VIP_SYS_1)                                                                                     \
    X(DIV_DISP_SRC_VIP)                                                                                      \
    X(DIV_AXI_VIDEO_CODEC)                                                                                   \
    X(DIV_VC_SRC0)                                                                                           \
    X(DIV_1M)                                                                                                \
    X(DIV_SPI)                                                                                               \
    X(DIV_I2C)                                                                                               \
    X(DIV_SRC_VIP_SYS_2)                                                                                     \
    X(DIV_AUDSRC)                                                                                            \
    X(DIV_PWM_SRC)                                                                                           \
    X(DIV_AP_DEBUG)                                                                                          \
    X(DIV_RTCSYS_SRC_0)                                                                                      \
    X(DIV_C906[0])                                                                                           \
    X(DIV_C906[1])                                                                                           \
    X(DIV_C906[2])                                                                                           \
    X(DIV_C906[3])                                                                                           \
    X(DIV_SRC_VIP_SYS_3)                                                                                     \
    X(DIV_SRC_VIP_SYS_4)

/** @} */

/**
 * @defgroup SG2002_PLL_G6 G6 PLL / G6 PLL
 * @ingroup SG2002_DEVICE
 * @{
 */
/// @brief G6 PLL 前置分频字段的最低有效位位置。
///        Least-significant bit position of G6 PLL predivider field.
static constexpr auto PLL_G6_PREDIV_SHIFT = 0U;
/// @brief G6 PLL 后置分频字段的最低有效位位置。
///        Least-significant bit position of G6 PLL postdivider field.
static constexpr auto PLL_G6_POSTDIV_SHIFT = 8U;
/// @brief G6 PLL 倍频字段的最低有效位位置。
///        Least-significant bit position of G6 PLL multiplier field.
static constexpr auto PLL_G6_MULTIPLIER_SHIFT = 17U;
/// @brief G6 PLL 分频字段的位宽。
///        Bit width of G6 PLL divider field.
static constexpr auto PLL_G6_DIVIDER_WIDTH = 7U;

/// @brief G6 MPLL 控制寄存器的字节偏移。
///        Byte offset of G6 MPLL control register.
static constexpr auto PLL_G6_MPLL_OFFSET = 0x008UL;
/// @brief G6 TPLL 控制寄存器的字节偏移。
///        Byte offset of G6 TPLL control register.
static constexpr auto PLL_G6_TPLL_OFFSET = 0x00CUL;
/// @brief G6 FPLL 控制寄存器的字节偏移。
///        Byte offset of G6 FPLL control register.
static constexpr auto PLL_G6_FPLL_OFFSET = 0x010UL;

/**
 * @brief G6 组 MPLL、TPLL 和 FPLL 控制寄存器。
 *        G6 MPLL, TPLL, and FPLL control registers.
 */
typedef struct
{
    uint32_t
        RESERVED_000[2]; ///< 偏移 0x000：未公开区域的占位空间。Offset 0x000: padding for an unexposed area.
    volatile uint32_t MPLL; ///< 偏移 0x008：MPLL 控制寄存器。Offset 0x008: MPLL control register.
    volatile uint32_t TPLL; ///< 偏移 0x00C：TPLL 控制寄存器。Offset 0x00C: TPLL control register.
    volatile uint32_t FPLL; ///< 偏移 0x010：FPLL 控制寄存器。Offset 0x010: FPLL control register.
} PLL_G6_Type;

/**
 * @brief PLL_G6 寄存器实例指针。
 *        PLL_G6 register-instance pointer.
 */
#define PLL_G6 ((PLL_G6_Type *)(uintptr_t)PLL_G6_BASE)
static_assert(offsetof(PLL_G6_Type, MPLL) == PLL_G6_MPLL_OFFSET);
static_assert(offsetof(PLL_G6_Type, TPLL) == PLL_G6_TPLL_OFFSET);
static_assert(offsetof(PLL_G6_Type, FPLL) == PLL_G6_FPLL_OFFSET);

/** @} */

/**
 * @defgroup SG2002_RSTGEN 复位控制 / RSTGEN reset control
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02, Chapter 7。SG2002 TRM v1.02, Chapter 7.
 * @{
 */

/// @brief SOFT_RSTN 寄存器视图的元素数量。
///        Element count of the SOFT_RSTN register view.
static constexpr auto RSTGEN_SOFT_RSTN_COUNT = 8U;
/// @brief SOFT_RSTN0 软复位寄存器的字节偏移。
///        Byte offset of SOFT_RSTN0 soft-reset register.
static constexpr auto RSTGEN_SOFT_RSTN0_OFFSET = 0x000UL;
/// @brief SOFT_RSTN 软复位寄存器的字节步长。
///        Byte stride of SOFT_RSTN soft-reset register.
static constexpr auto RSTGEN_SOFT_RSTN_STRIDE = 0x004UL;
/// @brief 表示软复位位采用低有效语义。
///        Indicates active-low soft-reset semantics.
static constexpr auto RSTGEN_SOFT_RSTN_ACTIVE_LOW = 1U;
/// @brief 无效复位寄存器或位坐标的哨兵值。
///        Sentinel for an invalid reset-register or bit coordinate.
static constexpr auto RSTGEN_RESET_LOCATION_INVALID = 0xFFU;

/**
 * @brief 复位控制器寄存器块。
 *        Reset-controller register block.
 */
typedef struct
{
    volatile uint32_t SOFT_RSTN[RSTGEN_SOFT_RSTN_COUNT]; ///< 偏移 0x000：低有效软复位寄存器数组。Offset
                                                         ///< 0x000: active-low soft-reset register array.
    volatile uint32_t
        SOFT_CPUAC_RSTN; ///< 偏移 0x020：CPUAC 软复位寄存器。Offset 0x020: CPUAC soft-reset register.
    volatile uint32_t SOFT_CPU_RSTN; ///< 偏移 0x024：CPU 软复位寄存器。Offset 0x024: CPU soft-reset register.
} RSTGEN_Type;

/**
 * @brief RSTGEN 寄存器实例指针。
 *        RSTGEN register-instance pointer.
 */
#define RSTGEN ((RSTGEN_Type *)(uintptr_t)RSTGEN_BASE)
static_assert(offsetof(RSTGEN_Type, SOFT_RSTN) == RSTGEN_SOFT_RSTN0_OFFSET);
static_assert(
    offsetof(RSTGEN_Type, SOFT_CPUAC_RSTN) ==
    RSTGEN_SOFT_RSTN0_OFFSET + RSTGEN_SOFT_RSTN_COUNT * RSTGEN_SOFT_RSTN_STRIDE
);

/**
 * @brief 展开复位目标名称及寄存器、位坐标。
 *        Expand reset-target names and their register/bit coordinates.
 *
 * @param X 接受 name、reg_index、bit 三个参数的展开宏。
 *        Expansion macro accepting name, reg_index, and bit.
 * @note RCC 实现由此生成内部复位位置表。
 *       The RCC implementation builds its internal reset-location table from this map.
 * @see SG2002 TRM 表 7.2-7.5；复位坐标。Tables 7.2-7.5, reset coordinates.
 */
#define SG2002_RESET_MAP(X)                                                                                  \
    X(NONE, RSTGEN_RESET_LOCATION_INVALID, RSTGEN_RESET_LOCATION_INVALID)                                    \
    X(SDMA, 0U, 18U)                                                                                         \
    X(I2S0, 0U, 19U)                                                                                         \
    X(I2S1, 0U, 20U)                                                                                         \
    X(I2S2, 0U, 21U)                                                                                         \
    X(I2S3, 0U, 22U)                                                                                         \
    X(UART0, 0U, 23U)                                                                                        \
    X(UART1, 0U, 24U)                                                                                        \
    X(UART2, 0U, 25U)                                                                                        \
    X(UART3, 0U, 26U)                                                                                        \
    X(I2C0, 0U, 27U)                                                                                         \
    X(I2C1, 0U, 28U)                                                                                         \
    X(I2C2, 0U, 29U)                                                                                         \
    X(I2C3, 0U, 30U)                                                                                         \
    X(I2C4, 0U, 31U)                                                                                         \
    X(PWM0, 1U, 0U)                                                                                          \
    X(PWM1, 1U, 1U)                                                                                          \
    X(PWM2, 1U, 2U)                                                                                          \
    X(PWM3, 1U, 3U)                                                                                          \
    X(SPI0, 1U, 8U)                                                                                          \
    X(SPI1, 1U, 9U)                                                                                          \
    X(SPI2, 1U, 10U)                                                                                         \
    X(SPI3, 1U, 11U)                                                                                         \
    X(GPIO0, 1U, 12U)                                                                                        \
    X(GPIO1, 1U, 13U)                                                                                        \
    X(GPIO2, 1U, 14U)                                                                                        \
    X(EFUSE, 1U, 15U)                                                                                        \
    X(WDT0, 1U, 16U)                                                                                         \
    X(TEMPSEN, 1U, 19U)                                                                                      \
    X(SARADC, 1U, 20U)                                                                                       \
    X(UART4, 2U, 10U)                                                                                        \
    X(GPIO3, 2U, 11U)                                                                                        \
    X(TIMER, 2U, 13U)                                                                                        \
    X(TIMER0, 2U, 14U)                                                                                       \
    X(TIMER1, 2U, 15U)                                                                                       \
    X(TIMER2, 2U, 16U)                                                                                       \
    X(TIMER3, 2U, 17U)                                                                                       \
    X(TIMER4, 2U, 18U)                                                                                       \
    X(TIMER5, 2U, 19U)                                                                                       \
    X(TIMER6, 2U, 20U)                                                                                       \
    X(TIMER7, 2U, 21U)                                                                                       \
    X(WGN0, 2U, 22U)                                                                                         \
    X(WGN1, 2U, 23U)                                                                                         \
    X(WGN2, 2U, 24U)                                                                                         \
    X(KEYSCAN, 2U, 25U)                                                                                      \
    X(WDT1, 3U, 4U)                                                                                          \
    X(WDT2, 3U, 5U)                                                                                          \
    X(SD0, 0U, 16U)                                                                                          \
    X(SD1, 0U, 17U)

/**
 * @brief 由器件映射生成的复位目标编号。
 *        Reset-target identifiers generated from the device map.
 */
typedef enum
{
/**
 * @brief 由复位映射生成枚举项。
 *        Generate an enumerator from the reset map.
 *
 * @param name 复位目标名称后缀。
 *        Reset-target name suffix.
 * @param reg_index 映射中的寄存器编号，此展开不使用。
 *        Register index in the map; unused by this expansion.
 * @param bit 映射中的位编号，此展开不使用。
 *        Bit index in the map; unused by this expansion.
 */
#define SG2002_RESET_ENUM(name, reg_index, bit) RESET_##name,
    SG2002_RESET_MAP(SG2002_RESET_ENUM)
#undef SG2002_RESET_ENUM
        RESET_TARGET_COUNT ///< 复位目标编号空间大小。Size of the reset-target ID space.
} rstgen_reset_target_t;

/**
 * @var rstgen_reset_target_t::RESET_NONE
 * @brief 无复位目标。 No reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_SD0
 * @brief SD0 复位目标。 SD0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_SD1
 * @brief SD1 复位目标。 SD1 reset target.
 * @note 坐标沿用 cv181x-resets.h 的 RST_SD1=17；TRM 表 7.2 将该位列为保留位。
 *       Coordinates retain RST_SD1=17 from cv181x-resets.h; TRM Table 7.2 lists this bit as reserved.
 */
/**
 * @var rstgen_reset_target_t::RESET_SDMA
 * @brief SDMA 复位目标。 SDMA reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_I2S0
 * @brief I2S0 复位目标。 I2S0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_I2S1
 * @brief I2S1 复位目标。 I2S1 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_I2S2
 * @brief I2S2 复位目标。 I2S2 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_I2S3
 * @brief I2S3 复位目标。 I2S3 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_UART0
 * @brief UART0 复位目标。 UART0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_UART1
 * @brief UART1 复位目标。 UART1 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_UART2
 * @brief UART2 复位目标。 UART2 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_UART3
 * @brief UART3 复位目标。 UART3 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_I2C0
 * @brief I2C0 复位目标。 I2C0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_I2C1
 * @brief I2C1 复位目标。 I2C1 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_I2C2
 * @brief I2C2 复位目标。 I2C2 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_I2C3
 * @brief I2C3 复位目标。 I2C3 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_I2C4
 * @brief I2C4 复位目标。 I2C4 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_PWM0
 * @brief PWM0 复位目标。 PWM0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_PWM1
 * @brief PWM1 复位目标。 PWM1 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_PWM2
 * @brief PWM2 复位目标。 PWM2 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_PWM3
 * @brief PWM3 复位目标。 PWM3 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_SPI0
 * @brief SPI0 复位目标。 SPI0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_SPI1
 * @brief SPI1 复位目标。 SPI1 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_SPI2
 * @brief SPI2 复位目标。 SPI2 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_SPI3
 * @brief SPI3 复位目标。 SPI3 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_GPIO0
 * @brief GPIO0 复位目标。 GPIO0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_GPIO1
 * @brief GPIO1 复位目标。 GPIO1 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_GPIO2
 * @brief GPIO2 复位目标。 GPIO2 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_EFUSE
 * @brief EFUSE 复位目标。 EFUSE reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_WDT0
 * @brief WDT0 复位目标。 WDT0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TEMPSEN
 * @brief TEMPSEN 复位目标。 TEMPSEN reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_SARADC
 * @brief SARADC 复位目标。 SARADC reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_UART4
 * @brief UART4 复位目标。 UART4 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_GPIO3
 * @brief GPIO3 复位目标。 GPIO3 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TIMER
 * @brief TIMER 复位目标。 TIMER reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TIMER0
 * @brief TIMER0 复位目标。 TIMER0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TIMER1
 * @brief TIMER1 复位目标。 TIMER1 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TIMER2
 * @brief TIMER2 复位目标。 TIMER2 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TIMER3
 * @brief TIMER3 复位目标。 TIMER3 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TIMER4
 * @brief TIMER4 复位目标。 TIMER4 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TIMER5
 * @brief TIMER5 复位目标。 TIMER5 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TIMER6
 * @brief TIMER6 复位目标。 TIMER6 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_TIMER7
 * @brief TIMER7 复位目标。 TIMER7 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_WGN0
 * @brief WGN0 复位目标。 WGN0 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_WGN1
 * @brief WGN1 复位目标。 WGN1 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_WGN2
 * @brief WGN2 复位目标。 WGN2 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_KEYSCAN
 * @brief KEYSCAN 复位目标。 KEYSCAN reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_WDT1
 * @brief WDT1 复位目标。 WDT1 reset target.
 */
/**
 * @var rstgen_reset_target_t::RESET_WDT2
 * @brief WDT2 复位目标。 WDT2 reset target.
 */

/**
 * @brief 低有效复位的寄存器编号和位坐标。
 *        Register index and bit coordinate of an active-low reset.
 */
typedef struct
{
    uint8_t reg_index; ///< 偏移 0x000：SOFT_RSTN 编号，无效时为 RSTGEN_RESET_LOCATION_INVALID。Offset 0x000:
                       ///< SOFT_RSTN index, or RSTGEN_RESET_LOCATION_INVALID.
    uint8_t bit; ///< 偏移 0x001：寄存器位号，无效时为 RSTGEN_RESET_LOCATION_INVALID。Offset 0x001: register
                 ///< bit index, or RSTGEN_RESET_LOCATION_INVALID.
} rstgen_reset_location_t;

/** @} */

/**
 * @defgroup SG2002_DMA DesignWare AXI DMA / DesignWare AXI DMA
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02, Chapter 11。SG2002 TRM v1.02, Chapter 11.
 * @{
 */
/// @brief DMA 链表描述符所需的字节对齐。
///        Required byte alignment of DMA linked-list items.
static constexpr auto DMA_LLI_ALIGNMENT = 64U;
/// @brief 单个 DMA 块允许的数据元素最大数量。
///        Maximum number of data items in one DMA block.
static constexpr auto DMA_BLOCK_TRANSFER_MAX = 0x40'0000U;
/// @brief 内存复制使用的无外设请求编码。
///        No-peripheral-request encoding used for memory copies.
static constexpr auto DMA_REQUEST_NONE = 0U;
/// @brief UART0 的 DMA 接收请求编号。
///        DMA receive request ID for UART0.
static constexpr auto DMA_REQUEST_UART0_RX = 8U;
/// @brief UART0 的 DMA 发送请求编号。
///        DMA transmit request ID for UART0.
static constexpr auto DMA_REQUEST_UART0_TX = 9U;
/// @brief UART1 的 DMA 接收请求编号。
///        DMA receive request ID for UART1.
static constexpr auto DMA_REQUEST_UART1_RX = 10U;
/// @brief UART1 的 DMA 发送请求编号。
///        DMA transmit request ID for UART1.
static constexpr auto DMA_REQUEST_UART1_TX = 11U;
/// @brief UART2 的 DMA 接收请求编号。
///        DMA receive request ID for UART2.
static constexpr auto DMA_REQUEST_UART2_RX = 12U;
/// @brief UART2 的 DMA 发送请求编号。
///        DMA transmit request ID for UART2.
static constexpr auto DMA_REQUEST_UART2_TX = 13U;
/// @brief UART3 的 DMA 接收请求编号。
///        DMA receive request ID for UART3.
static constexpr auto DMA_REQUEST_UART3_RX = 14U;
/// @brief UART3 的 DMA 发送请求编号。
///        DMA transmit request ID for UART3.
static constexpr auto DMA_REQUEST_UART3_TX = 15U;
/// @brief SPI0 的 DMA 接收请求编号。
///        DMA receive request ID for SPI0.
static constexpr auto DMA_REQUEST_SPI0_RX = 16U;
/// @brief SPI0 的 DMA 发送请求编号。
///        DMA transmit request ID for SPI0.
static constexpr auto DMA_REQUEST_SPI0_TX = 17U;
/// @brief SPI1 的 DMA 接收请求编号。
///        DMA receive request ID for SPI1.
static constexpr auto DMA_REQUEST_SPI1_RX = 18U;
/// @brief SPI1 的 DMA 发送请求编号。
///        DMA transmit request ID for SPI1.
static constexpr auto DMA_REQUEST_SPI1_TX = 19U;
/// @brief SPI2 的 DMA 接收请求编号。
///        DMA receive request ID for SPI2.
static constexpr auto DMA_REQUEST_SPI2_RX = 20U;
/// @brief SPI2 的 DMA 发送请求编号。
///        DMA transmit request ID for SPI2.
static constexpr auto DMA_REQUEST_SPI2_TX = 21U;
/// @brief SPI3 的 DMA 接收请求编号。
///        DMA receive request ID for SPI3.
static constexpr auto DMA_REQUEST_SPI3_RX = 22U;
/// @brief SPI3 的 DMA 发送请求编号。
///        DMA transmit request ID for SPI3.
static constexpr auto DMA_REQUEST_SPI3_TX = 23U;
/// @brief I2C0 的 DMA 接收请求编号。
///        DMA receive request ID for I2C0.
static constexpr auto DMA_REQUEST_I2C0_RX = 24U;
/// @brief I2C0 的 DMA 发送请求编号。
///        DMA transmit request ID for I2C0.
static constexpr auto DMA_REQUEST_I2C0_TX = 25U;
/// @brief I2C1 的 DMA 接收请求编号。
///        DMA receive request ID for I2C1.
static constexpr auto DMA_REQUEST_I2C1_RX = 26U;
/// @brief I2C1 的 DMA 发送请求编号。
///        DMA transmit request ID for I2C1.
static constexpr auto DMA_REQUEST_I2C1_TX = 27U;
/// @brief I2C2 的 DMA 接收请求编号。
///        DMA receive request ID for I2C2.
static constexpr auto DMA_REQUEST_I2C2_RX = 28U;
/// @brief I2C2 的 DMA 发送请求编号。
///        DMA transmit request ID for I2C2.
static constexpr auto DMA_REQUEST_I2C2_TX = 29U;
/// @brief I2C3 的 DMA 接收请求编号。
///        DMA receive request ID for I2C3.
static constexpr auto DMA_REQUEST_I2C3_RX = 30U;
/// @brief I2C3 的 DMA 发送请求编号。
///        DMA transmit request ID for I2C3.
static constexpr auto DMA_REQUEST_I2C3_TX = 31U;
/// @brief I2C4 的 DMA 接收请求编号。
///        DMA receive request ID for I2C4.
static constexpr auto DMA_REQUEST_I2C4_RX = 32U;
/// @brief I2C4 的 DMA 发送请求编号。
///        DMA transmit request ID for I2C4.
static constexpr auto DMA_REQUEST_I2C4_TX = 33U;
/// @brief UART4 的 DMA 接收请求编号。
///        DMA receive request ID for UART4.
static constexpr auto DMA_REQUEST_UART4_RX = 40U;
/// @brief UART4 的 DMA 发送请求编号。
///        DMA transmit request ID for UART4.
static constexpr auto DMA_REQUEST_UART4_TX = 41U;
/// @brief 相邻 DMA 请求映射字段的位步长。
///        Bit stride between adjacent DMA request-remap fields.
static constexpr auto DMA_REMAP_REQUEST_SHIFT = 8U;
/// @brief 每个重映射寄存器容纳的通道字段数。
///        Channel fields held in each remap register.
static constexpr auto DMA_REMAP_CHANNELS_PER_REGISTER = 4U;
/// @brief DMA 请求映射字段的位掩码。
///        Bit mask for DMA request-remap field.
static constexpr auto DMA_REMAP_REQUEST_MASK = 0x3FU;
/// @brief DMA 请求重映射使能位。
///        Bit for DMA request-remap enable.
static constexpr auto DMA_REMAP_ENABLE_BIT = (1UL << 31);
/// @brief 通道请求位到对应写使能位的位移。
///        Shift from a channel-request bit to its write-enable bit.
static constexpr auto DMA_CHANNEL_WRITE_ENABLE_SHIFT = 8U;

/// @brief DMA 全局配置寄存器的字节偏移。
///        Byte offset of DMA global configuration register.
static constexpr auto DMA_CFG_OFFSET = 0x010U;
/// @brief DMA 通道使能寄存器低字的字节偏移。
///        Byte offset of DMA channel-enable register low word.
static constexpr auto DMA_CHEN_OFFSET = 0x018U;

/**
 * @brief DMA 通道使能寄存器高字的字节偏移。
 *        Byte offset of DMA channel-enable register high word.
 *
 * @note DMAC_CHENREG[39:32] 为中止请求，[47:40] 为写使能；本字段是同一寄存器的高字。
 *       DMAC_CHENREG[39:32] holds abort requests and [47:40] their write enables; this is the high word of
 * the same register.
 * @see SG2002 TRM 表 11.5；通道使能寄存器。Table 11.5, channel-enable register.
 */
static constexpr auto DMA_CHABORT_OFFSET = 0x01CU;
/// @brief DMA 通道寄存器块的字节偏移。
///        Byte offset of DMA channel register block.
static constexpr auto DMA_CHANNEL_OFFSET = 0x100U;
/// @brief DMA 通道寄存器块的字节步长。
///        Byte stride of DMA channel register block.
static constexpr auto DMA_CHANNEL_STRIDE = 0x100U;
/// @brief DMA 通道配置寄存器的字节偏移。
///        Byte offset of DMA channel configuration register.
static constexpr auto DMA_CH_CFG_OFFSET = 0x020U;
/// @brief DMA 通道链表指针寄存器的字节偏移。
///        Byte offset of DMA channel linked-list-pointer register.
static constexpr auto DMA_CH_LLP_OFFSET = 0x028U;
/// @brief DMA 通道中断状态使能寄存器的字节偏移。
///        Byte offset of DMA channel interrupt-status-enable register.
static constexpr auto DMA_CH_INTSTATUS_EN_OFFSET = 0x080U;
/// @brief DMA 通道中断状态寄存器的字节偏移。
///        Byte offset of DMA channel interrupt-status register.
static constexpr auto DMA_CH_INTSTATUS_OFFSET = 0x088U;
/// @brief DMA 通道中断信号使能寄存器的字节偏移。
///        Byte offset of DMA channel interrupt-signal-enable register.
static constexpr auto DMA_CH_INTSIGNAL_EN_OFFSET = 0x090U;
/// @brief DMA 通道中断清除寄存器的字节偏移。
///        Byte offset of DMA channel interrupt-clear register.
static constexpr auto DMA_CH_INTCLEAR_OFFSET = 0x098U;

/// @brief DMA 控制器使能位。
///        Bit for DMA controller enable.
static constexpr auto DMA_CFG_DMAC_ENABLE_BIT = (1UL << 0);
/// @brief DMA 全局中断使能位。
///        Bit for DMA global interrupt enable.
static constexpr auto DMA_CFG_INTERRUPT_ENABLE_BIT = (1UL << 1);
/// @brief DMA 源主端口选择位。
///        Bit for DMA source-master selection.
static constexpr auto DMA_CTL_SMS_BIT = (1ULL << 0);
/// @brief DMA 目的主端口选择位。
///        Bit for DMA destination-master selection.
static constexpr auto DMA_CTL_DMS_BIT = (1ULL << 2);
/// @brief DMA 源地址递增控制位。
///        Bit for DMA source-address increment control.
static constexpr auto DMA_CTL_SINC_BIT = (1ULL << 4);
/// @brief DMA 目的地址递增控制位。
///        Bit for DMA destination-address increment control.
static constexpr auto DMA_CTL_DINC_BIT = (1ULL << 6);
/// @brief DMA 源数据宽度字段的最低有效位位置。
///        Least-significant bit position of DMA source-data-width field.
static constexpr auto DMA_CTL_SRC_WIDTH_SHIFT = 8U;
/// @brief DMA 目的数据宽度字段的最低有效位位置。
///        Least-significant bit position of DMA destination-data-width field.
static constexpr auto DMA_CTL_DST_WIDTH_SHIFT = 11U;
/// @brief DMA 块完成中断使能位。
///        Bit for DMA block-completion interrupt enable.
static constexpr auto DMA_CTL_IOC_BLOCK_BIT = (1ULL << 58);
/// @brief DMA 最后一个链表描述符标志位。
///        Bit for DMA last-linked-list-item flag.
static constexpr auto DMA_CTL_LLI_LAST_BIT = (1ULL << 62);
/// @brief DMA 链表描述符有效标志位。
///        Bit for DMA linked-list-item valid flag.
static constexpr auto DMA_CTL_LLI_VALID_BIT = (1ULL << 63);
/// @brief DMA 传输类型与流控字段的最低有效位位置。
///        Least-significant bit position of DMA transfer-type and flow-control field.
static constexpr auto DMA_CFG_TTFC_SHIFT = 32U;
/// @brief DMA 源握手选择字段的最低有效位位置。
///        Least-significant bit position of DMA source-handshake selector.
static constexpr auto DMA_CFG_SRC_PER_SHIFT = 39U;
/// @brief DMA 目的握手选择字段的最低有效位位置。
///        Least-significant bit position of DMA destination-handshake selector.
static constexpr auto DMA_CFG_DST_PER_SHIFT = 44U;
/// @brief DMA 优先级字段的最低有效位位置。
///        Least-significant bit position of DMA priority field.
static constexpr auto DMA_CFG_PRIORITY_SHIFT = 49U;
/// @brief 最高 DMA 通道优先级编码。
///        Highest DMA channel-priority encoding.
static constexpr auto DMA_CFG_PRIORITY_MAX = 7U;
/// @brief DMA 优先级字段的位掩码。
///        Bit mask for DMA priority field.
static constexpr auto DMA_CFG_PRIORITY_MASK = (7ULL << DMA_CFG_PRIORITY_SHIFT);
/// @brief 最大未完成请求限制的字段编码。
///        Field encoding for the maximum outstanding-request limit.
static constexpr auto DMA_CFG_OSR_MAX = 15U;
/// @brief DMA 源未完成请求限制字段的最低有效位位置。
///        Least-significant bit position of DMA source outstanding-request limit field.
static constexpr auto DMA_CFG_SRC_OSR_SHIFT = 55U;
/// @brief DMA 目的未完成请求限制字段的最低有效位位置。
///        Least-significant bit position of DMA destination outstanding-request limit field.
static constexpr auto DMA_CFG_DST_OSR_SHIFT = 59U;
/// @brief 源端采用链表多块传输的编码。
///        Source linked-list multi-block transfer encoding.
static constexpr auto DMA_CFG_SRC_MULTIBLK_LLI = (3ULL << 0);
/// @brief 目的端采用链表多块传输的编码。
///        Destination linked-list multi-block transfer encoding.
static constexpr auto DMA_CFG_DST_MULTIBLK_LLI = (3ULL << 2);
/// @brief DMA 块完成中断位。
///        Bit for DMA block-completion interrupt.
static constexpr auto DMA_INT_BLOCK_DONE_BIT = (1UL << 0);
/// @brief DMA 传输完成中断位。
///        Bit for DMA transfer-completion interrupt.
static constexpr auto DMA_INT_TRANSFER_DONE_BIT = (1UL << 1);
/// @brief DMA 错误中断字段的位掩码。
///        Bit mask for DMA error-interrupt fields.
static constexpr auto DMA_INT_ERROR_MASK = 0xF8FF'7FE0UL;
/// @brief DMA 已定义中断字段的位掩码。
///        Bit mask for DMA defined interrupt fields.
static constexpr auto DMA_INT_ALL_MASK =
    (DMA_INT_BLOCK_DONE_BIT | DMA_INT_TRANSFER_DONE_BIT | DMA_INT_ERROR_MASK);

/**
 * @brief AXI DMA 全局控制寄存器。
 *        AXI DMA global control registers.
 */
typedef struct
{
    volatile uint32_t
        RESERVED_000[4];   ///< 偏移 0x000：未公开区域的占位空间。Offset 0x000: padding for an unexposed area.
    volatile uint32_t CFG; ///< 偏移 0x010：DMA 全局配置寄存器低字。Offset 0x010: DMA global
                           ///< configuration-register low word.
    volatile uint32_t
        RESERVED_014; ///< 偏移 0x014：未公开区域的占位空间。Offset 0x014: padding for an unexposed area.
    volatile uint32_t
        CHEN; ///< 偏移 0x018：DMA 通道使能寄存器低字。Offset 0x018: DMA channel-enable register low word.
    volatile uint32_t CHABORT; ///< 偏移 0x01C：DMAC_CHENREG 高字，包含中止请求和写使能。Offset 0x01C:
                               ///< DMAC_CHENREG high word containing abort requests and write enables.
    volatile uint32_t
        RESERVED_020[(DMA_CHANNEL_OFFSET - 0x020U) / 4U]; ///< 偏移 0x020：未公开区域的占位空间。Offset 0x020:
                                                          ///< padding for an unexposed area.
} DMA_GLOBAL_Type;

static_assert(offsetof(DMA_GLOBAL_Type, CFG) == DMA_CFG_OFFSET);
static_assert(offsetof(DMA_GLOBAL_Type, CHEN) == DMA_CHEN_OFFSET);
static_assert(offsetof(DMA_GLOBAL_Type, CHABORT) == DMA_CHABORT_OFFSET);
static_assert(sizeof(DMA_GLOBAL_Type) == DMA_CHANNEL_OFFSET);

/**
 * @brief 单个 AXI DMA 通道的寄存器块。
 *        Register block for one AXI DMA channel.
 */
typedef struct
{
    volatile uint32_t
        SAR_LOW; ///< 偏移 0x000：源地址低 32 位。Offset 0x000: low 32 bits of the source address.
    volatile uint32_t
        SAR_HIGH; ///< 偏移 0x004：源地址高 32 位。Offset 0x004: high 32 bits of the source address.
    volatile uint32_t
        DAR_LOW; ///< 偏移 0x008：目的地址低 32 位。Offset 0x008: low 32 bits of the destination address.
    volatile uint32_t
        DAR_HIGH; ///< 偏移 0x00C：目的地址高 32 位。Offset 0x00C: high 32 bits of the destination address.
    volatile uint32_t BLOCK_TS_LOW;  ///< 偏移 0x010：块传输元素计数字段低字。Offset 0x010: low word of the
                                     ///< block item-count field.
    volatile uint32_t BLOCK_TS_HIGH; ///< 偏移 0x014：块传输元素计数字段高字。Offset 0x014: high word of the
                                     ///< block item-count field.
    volatile uint32_t CTL_LOW;       ///< 偏移 0x018：传输控制低字。Offset 0x018: transfer-control low word.
    volatile uint32_t CTL_HIGH;      ///< 偏移 0x01C：传输控制高字。Offset 0x01C: transfer-control high word.
    volatile uint32_t CFG_LOW;  ///< 偏移 0x020：通道配置低字。Offset 0x020: channel-configuration low word.
    volatile uint32_t CFG_HIGH; ///< 偏移 0x024：通道配置高字。Offset 0x024: channel-configuration high word.
    volatile uint32_t LLP_LOW;  ///< 偏移 0x028：链表指针低字。Offset 0x028: linked-list-pointer low word.
    volatile uint32_t LLP_HIGH; ///< 偏移 0x02C：链表指针高字。Offset 0x02C: linked-list-pointer high word.
    volatile uint32_t
        RESERVED_030[20]; ///< 偏移 0x030：未公开区域的占位空间。Offset 0x030: padding for an unexposed area.
    volatile uint32_t
        INTSTATUS_EN; ///< 偏移 0x080：中断状态锁存使能。Offset 0x080: interrupt-status latch enable.
    volatile uint32_t
        RESERVED_084; ///< 偏移 0x084：未公开区域的占位空间。Offset 0x084: padding for an unexposed area.
    volatile uint32_t INTSTATUS; ///< 偏移 0x088：中断状态寄存器。Offset 0x088: interrupt-status register.
    volatile uint32_t
        RESERVED_08C; ///< 偏移 0x08C：未公开区域的占位空间。Offset 0x08C: padding for an unexposed area.
    volatile uint32_t
        INTSIGNAL_EN; ///< 偏移 0x090：中断信号输出使能。Offset 0x090: interrupt-signal output enable.
    volatile uint32_t
        RESERVED_094; ///< 偏移 0x094：未公开区域的占位空间。Offset 0x094: padding for an unexposed area.
    volatile uint32_t INTCLEAR; ///< 偏移 0x098：写 1 清除的中断状态位。Offset 0x098: write-one-to-clear
                                ///< interrupt status bits.
    volatile uint32_t
        RESERVED_09C[25]; ///< 偏移 0x09C：未公开区域的占位空间。Offset 0x09C: padding for an unexposed area.
} DMA_Channel_Type;

static_assert(sizeof(DMA_Channel_Type) == DMA_CHANNEL_STRIDE);
static_assert(offsetof(DMA_Channel_Type, INTSTATUS_EN) == DMA_CH_INTSTATUS_EN_OFFSET);
static_assert(offsetof(DMA_Channel_Type, INTSTATUS) == DMA_CH_INTSTATUS_OFFSET);
static_assert(offsetof(DMA_Channel_Type, INTSIGNAL_EN) == DMA_CH_INTSIGNAL_EN_OFFSET);
static_assert(offsetof(DMA_Channel_Type, INTCLEAR) == DMA_CH_INTCLEAR_OFFSET);

/**
 * @brief 包含全局和通道寄存器的 DMA 控制器视图。
 *        DMA controller view containing global and channel registers.
 */
typedef struct
{
    DMA_GLOBAL_Type
        GLOBAL; ///< 偏移 0x000：DMA 全局控制寄存器视图。Offset 0x000: DMA global control-register view.
    DMA_Channel_Type CHANNEL[DMA_CHANNEL_COUNT]; ///< 偏移 0x100：DMA 通道寄存器数组。Offset 0x100: DMA
                                                 ///< channel-register array.
} DMA_Type;

/**
 * @brief DMA 寄存器实例指针。
 *        DMA register-instance pointer.
 */
#define DMA ((DMA_Type *)(uintptr_t)DMA_BASE)

/**
 * @brief AXI DMA 链表描述符。
 *        AXI DMA linked-list item.
 *
 * @note LLP[5:1] 保留，每项必须从 64 字节边界开始。
 *       LLP[5:1] are reserved; each item must begin on a 64-byte boundary.
 * @see SG2002 技术参考手册 v1.02, 表 11.26。SG2002 TRM v1.02, Table 11.26
 */
typedef struct
{
    alignas(
        DMA_LLI_ALIGNMENT
    ) uint64_t source;    ///< 偏移 0x000：源数据地址。Offset 0x000: source data address.
    uint64_t destination; ///< 偏移 0x008：目的数据地址。Offset 0x008: destination data address.
    uint64_t block_ts;    ///< 偏移 0x010：数据元素数量减 1。Offset 0x010: data-item count minus one.
    uint64_t next; ///< 偏移 0x018：下一描述符地址与链接字段。Offset 0x018: next-descriptor address and link
                   ///< fields.
    uint32_t control_low;        ///< 偏移 0x020：传输控制低字。Offset 0x020: transfer-control low word.
    uint32_t control_high;       ///< 偏移 0x024：传输控制高字。Offset 0x024: transfer-control high word.
    uint32_t source_status;      ///< 偏移 0x028：源状态字。Offset 0x028: source-status word.
    uint32_t destination_status; ///< 偏移 0x02C：目的状态字。Offset 0x02C: destination-status word.
    uint32_t status_low;         ///< 偏移 0x030：描述符状态低字。Offset 0x030: descriptor-status low word.
    uint32_t status_high;        ///< 偏移 0x034：描述符状态高字。Offset 0x034: descriptor-status high word.
    uint32_t reserved_low;       ///< 偏移 0x038：保留低字。Offset 0x038: reserved low word.
    uint32_t reserved_high;      ///< 偏移 0x03C：保留高字。Offset 0x03C: reserved high word.
} DMA_LLI_Type;

static_assert(sizeof(DMA_LLI_Type) == 64U);
static_assert(alignof(DMA_LLI_Type) == DMA_LLI_ALIGNMENT);

/** @} */

/**
 * @defgroup SG2002_SPI DesignWare SSI / DesignWare SSI register block
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02, 表 21.73-21.98。SG2002 TRM v1.02, Tables 21.73-21.98.
 * @{
 */

/// @brief 相邻 SSI 实例基址之间的字节间隔。
///        Byte stride between adjacent SSI instance bases.
static constexpr auto SPI_INSTANCE_STRIDE = 0x1'0000UL;
/// @brief SSI 收发 FIFO 的条目深度。
///        Entry depth of the SSI receive/transmit FIFOs.
static constexpr auto SPI_FIFO_DEPTH = 8U;
/// @brief 同一 SPI FIFO 数据端口的地址别名数量。
///        Address aliases for the same SPI FIFO data port.
static constexpr auto SPI_DR_COUNT = 36U;
/// @brief 每个 SSI 实例已定义的外部片选数量。
///        Defined external slave selects per SSI instance.
static constexpr auto SPI_CHIP_SELECT_COUNT = 1U;
/// @brief SPI 控制寄存器 0的字节偏移。
///        Byte offset of SPI control register zero.
static constexpr auto SPI_CTRLR0_OFFSET = 0x000U;
/// @brief SPI 控制寄存器 1的字节偏移。
///        Byte offset of SPI control register one.
static constexpr auto SPI_CTRLR1_OFFSET = 0x004U;
/// @brief SSI 使能寄存器的字节偏移。
///        Byte offset of SSI enable register.
static constexpr auto SPI_SPIENR_OFFSET = 0x008U;
/// @brief Microwire 控制寄存器的字节偏移。
///        Byte offset of Microwire control register.
static constexpr auto SPI_MWCR_OFFSET = 0x00CU;
/// @brief SPI 片选寄存器的字节偏移。
///        Byte offset of SPI slave-select register.
static constexpr auto SPI_SER_OFFSET = 0x010U;
/// @brief SPI 波特率分频寄存器的字节偏移。
///        Byte offset of SPI baud-divisor register.
static constexpr auto SPI_BAUDR_OFFSET = 0x014U;
/// @brief SPI 发送 FIFO 阈值寄存器的字节偏移。
///        Byte offset of SPI transmit FIFO threshold register.
static constexpr auto SPI_TXFTLR_OFFSET = 0x018U;
/// @brief SPI 接收 FIFO 阈值寄存器的字节偏移。
///        Byte offset of SPI receive FIFO threshold register.
static constexpr auto SPI_RXFTLR_OFFSET = 0x01CU;
/// @brief SPI 发送 FIFO 条目计数寄存器的字节偏移。
///        Byte offset of SPI transmit FIFO level register.
static constexpr auto SPI_TXFLR_OFFSET = 0x020U;
/// @brief SPI 接收 FIFO 条目计数寄存器的字节偏移。
///        Byte offset of SPI receive FIFO level register.
static constexpr auto SPI_RXFLR_OFFSET = 0x024U;
/// @brief SPI 状态寄存器的字节偏移。
///        Byte offset of SPI status register.
static constexpr auto SPI_SR_OFFSET = 0x028U;
/// @brief SPI 中断使能掩码寄存器的字节偏移。
///        Byte offset of SPI interrupt-enable mask register.
static constexpr auto SPI_IMR_OFFSET = 0x02CU;
/// @brief SPI 屏蔽后中断状态寄存器的字节偏移。
///        Byte offset of SPI masked interrupt-status register.
static constexpr auto SPI_ISR_OFFSET = 0x030U;
/// @brief SPI 原始中断状态寄存器的字节偏移。
///        Byte offset of SPI raw interrupt-status register.
static constexpr auto SPI_RISR_OFFSET = 0x034U;
/// @brief SPI 发送溢出清除寄存器的字节偏移。
///        Byte offset of SPI transmit-overflow clear register.
static constexpr auto SPI_TXOICR_OFFSET = 0x038U;
/// @brief SPI 接收溢出清除寄存器的字节偏移。
///        Byte offset of SPI receive-overflow clear register.
static constexpr auto SPI_RXOICR_OFFSET = 0x03CU;
/// @brief SPI 接收下溢清除寄存器的字节偏移。
///        Byte offset of SPI receive-underflow clear register.
static constexpr auto SPI_RXUICR_OFFSET = 0x040U;
/// @brief SPI 多主机竞争清除寄存器的字节偏移。
///        Byte offset of SPI multi-master contention clear register.
static constexpr auto SPI_MSTICR_OFFSET = 0x044U;
/// @brief SPI 中断清除寄存器的字节偏移。
///        Byte offset of SPI interrupt-clear register.
static constexpr auto SPI_ICR_OFFSET = 0x048U;
/// @brief SPI DMA 请求控制寄存器的字节偏移。
///        Byte offset of SPI DMA request-control register.
static constexpr auto SPI_DMACR_OFFSET = 0x04CU;
/// @brief SPI 发送 DMA 请求阈值的字节偏移。
///        Byte offset of SPI transmit DMA request threshold.
static constexpr auto SPI_DMATDLR_OFFSET = 0x050U;
/// @brief SPI 接收 DMA 请求阈值的字节偏移。
///        Byte offset of SPI receive DMA request threshold.
static constexpr auto SPI_DMARDLR_OFFSET = 0x054U;
/// @brief SPI FIFO 数据端口的字节偏移。
///        Byte offset of SPI FIFO data port.
static constexpr auto SPI_DR_OFFSET = 0x060U;
/// @brief SPI 接收采样延迟的字节偏移。
///        Byte offset of SPI receive-sample delay.
static constexpr auto SPI_RX_SAMPLE_DLY_OFFSET = 0x0F0U;

/**
 * @brief DesignWare SSI 寄存器块。
 *        DesignWare SSI register block.
 *
 * @see SG2002 技术参考手册 v1.02, 表 21.73-21.98。SG2002 TRM v1.02, Tables 21.73-21.98
 */
typedef struct
{
    volatile uint32_t CTRLR0; ///< 偏移 0x000：SPI 控制寄存器 0。Offset 0x000: SPI control register zero.
    volatile uint32_t CTRLR1; ///< 偏移 0x004：SPI 控制寄存器 1。Offset 0x004: SPI control register one.
    volatile uint32_t SPIENR; ///< 偏移 0x008：SSI 使能寄存器。Offset 0x008: SSI enable register.
    volatile uint32_t MWCR;   ///< 偏移 0x00C：Microwire 控制寄存器。Offset 0x00C: Microwire control register.
    volatile uint32_t SER;    ///< 偏移 0x010：SPI 片选寄存器。Offset 0x010: SPI slave-select register.
    volatile uint32_t BAUDR;  ///< 偏移 0x014：SPI 波特率分频寄存器。Offset 0x014: SPI baud-divisor register.
    volatile uint32_t
        TXFTLR; ///< 偏移 0x018：SPI 发送 FIFO 阈值寄存器。Offset 0x018: SPI transmit FIFO threshold register.
    volatile uint32_t
        RXFTLR; ///< 偏移 0x01C：SPI 接收 FIFO 阈值寄存器。Offset 0x01C: SPI receive FIFO threshold register.
    volatile uint32_t
        TXFLR; ///< 偏移 0x020：SPI 发送 FIFO 条目计数寄存器。Offset 0x020: SPI transmit FIFO level register.
    volatile uint32_t
        RXFLR; ///< 偏移 0x024：SPI 接收 FIFO 条目计数寄存器。Offset 0x024: SPI receive FIFO level register.
    volatile uint32_t SR; ///< 偏移 0x028：SPI 状态寄存器。Offset 0x028: SPI status register.
    volatile uint32_t
        IMR; ///< 偏移 0x02C：SPI 中断使能掩码寄存器。Offset 0x02C: SPI interrupt-enable mask register.
    volatile uint32_t
        ISR; ///< 偏移 0x030：SPI 屏蔽后中断状态寄存器。Offset 0x030: SPI masked interrupt-status register.
    volatile uint32_t
        RISR; ///< 偏移 0x034：SPI 原始中断状态寄存器。Offset 0x034: SPI raw interrupt-status register.
    volatile uint32_t
        TXOICR; ///< 偏移 0x038：SPI 发送溢出清除寄存器。Offset 0x038: SPI transmit-overflow clear register.
    volatile uint32_t
        RXOICR; ///< 偏移 0x03C：SPI 接收溢出清除寄存器。Offset 0x03C: SPI receive-overflow clear register.
    volatile uint32_t
        RXUICR; ///< 偏移 0x040：SPI 接收下溢清除寄存器。Offset 0x040: SPI receive-underflow clear register.
    volatile uint32_t MSTICR; ///< 偏移 0x044：SPI 多主机竞争清除寄存器。Offset 0x044: SPI multi-master
                              ///< contention clear register.
    volatile uint32_t ICR;    ///< 偏移 0x048：SPI 中断清除寄存器。Offset 0x048: SPI interrupt-clear register.
    volatile uint32_t
        DMACR; ///< 偏移 0x04C：SPI DMA 请求控制寄存器。Offset 0x04C: SPI DMA request-control register.
    volatile uint32_t
        DMATDLR; ///< 偏移 0x050：SPI 发送 DMA 请求阈值。Offset 0x050: SPI transmit DMA request threshold.
    volatile uint32_t
        DMARDLR; ///< 偏移 0x054：SPI 接收 DMA 请求阈值。Offset 0x054: SPI receive DMA request threshold.
    uint32_t
        RESERVED_058[2]; ///< 偏移 0x058：未公开区域的占位空间。Offset 0x058: padding for an unexposed area.
    volatile uint32_t DR[SPI_DR_COUNT]; ///< 偏移 0x060：同一 FIFO 数据端口的 36 个地址别名。Offset 0x060: 36
                                        ///< address aliases of the same FIFO data port.
    volatile uint32_t
        RX_SAMPLE_DLY; ///< 偏移 0x0F0：SPI 接收采样延迟。Offset 0x0F0: SPI receive-sample delay.
} SPI_Type;

static_assert(offsetof(SPI_Type, DR) == SPI_DR_OFFSET);
static_assert(offsetof(SPI_Type, RX_SAMPLE_DLY) == SPI_RX_SAMPLE_DLY_OFFSET);

/**
 * @brief 表示设备头已提供 SPI 寄存器类型。
 *        Indicate that the device header provides the SPI register type.
 */
#define DEVICE_HAS_SPI_TYPE 1

/**
 * @brief SPI 寄存器块类型的别名。
 *        Alias of the SPI register-block type.
 */
typedef SPI_Type spi_t;

/**
 * @brief SPI0 寄存器实例指针。
 *        SPI0 register-instance pointer.
 */
#define SPI0_REGS ((SPI_Type *)(uintptr_t)SPI0_BASE)

/**
 * @brief SPI1 寄存器实例指针。
 *        SPI1 register-instance pointer.
 */
#define SPI1_REGS ((SPI_Type *)(uintptr_t)SPI1_BASE)

/**
 * @brief SPI2 寄存器实例指针。
 *        SPI2 register-instance pointer.
 */
#define SPI2_REGS ((SPI_Type *)(uintptr_t)SPI2_BASE)

/**
 * @brief SPI3 寄存器实例指针。
 *        SPI3 register-instance pointer.
 */
#define SPI3_REGS ((SPI_Type *)(uintptr_t)SPI3_BASE)

/**
 * @name SSI 寄存器字段 / SSI register fields
 * @see SG2002 技术参考手册 v1.02, 表 21.73-21.98。SG2002 TRM v1.02, Tables 21.73-21.98.
 * @{
 */
/// @brief SPI 控制帧长度字段的最低有效位位置。
///        Least-significant bit position of SPI control-frame-length field.
static constexpr auto SPI_CTRLR0_CFS_SHIFT = 12U;
/// @brief SPI 控制帧长度字段的位宽。
///        Bit width of SPI control-frame-length field.
static constexpr auto SPI_CTRLR0_CFS_WIDTH = 4U;
/// @brief SPI 控制帧长度字段的位掩码。
///        Bit mask for SPI control-frame-length field.
static constexpr auto SPI_CTRLR0_CFS_MASK = (0x0FUL << SPI_CTRLR0_CFS_SHIFT);
/// @brief SPI 移位寄存器回环位。
///        Bit for SPI shift-register loopback.
static constexpr auto SPI_CTRLR0_SRL_BIT = (1UL << 11);
/// @brief SPI 收发模式字段的最低有效位位置。
///        Least-significant bit position of SPI transfer-mode field.
static constexpr auto SPI_CTRLR0_TMOD_SHIFT = 8U;
/// @brief SPI 收发模式字段的位宽。
///        Bit width of SPI transfer-mode field.
static constexpr auto SPI_CTRLR0_TMOD_WIDTH = 2U;
/// @brief SPI 收发模式字段的位掩码。
///        Bit mask for SPI transfer-mode field.
static constexpr auto SPI_CTRLR0_TMOD_MASK = (0x03UL << SPI_CTRLR0_TMOD_SHIFT);
/// @brief SPI 时钟极性位。
///        Bit for SPI clock polarity.
static constexpr auto SPI_CTRLR0_SCPOL_BIT = (1UL << 7);
/// @brief SPI 时钟相位位。
///        Bit for SPI clock phase.
static constexpr auto SPI_CTRLR0_SCPH_BIT = (1UL << 6);
/// @brief SPI 串行帧格式字段的最低有效位位置。
///        Least-significant bit position of SPI serial frame-format field.
static constexpr auto SPI_CTRLR0_FRF_SHIFT = 4U;
/// @brief SPI 串行帧格式字段的位宽。
///        Bit width of SPI serial frame-format field.
static constexpr auto SPI_CTRLR0_FRF_WIDTH = 2U;
/// @brief SPI 串行帧格式字段的位掩码。
///        Bit mask for SPI serial frame-format field.
static constexpr auto SPI_CTRLR0_FRF_MASK = (0x03UL << SPI_CTRLR0_FRF_SHIFT);
/// @brief SPI 数据帧长度字段的最低有效位位置。
///        Least-significant bit position of SPI data-frame-length field.
static constexpr auto SPI_CTRLR0_DFS_SHIFT = 0U;
/// @brief SPI 数据帧长度字段的位宽。
///        Bit width of SPI data-frame-length field.
static constexpr auto SPI_CTRLR0_DFS_WIDTH = 4U;
/// @brief SPI 数据帧长度字段的位掩码。
///        Bit mask for SPI data-frame-length field.
static constexpr auto SPI_CTRLR0_DFS_MASK = 0x0FUL;

/// @brief SSI 控制器使能位。
///        Bit for SSI controller enable.
static constexpr auto SPI_SPIENR_ENABLE_BIT = (1UL << 0);
/// @brief SPI 片选 0 使能位。
///        Bit for SPI slave-select-zero enable.
static constexpr auto SPI_SER_CS0_BIT = (1UL << 0);
/// @brief SPI 片选寄存器的位掩码。
///        Bit mask for SPI slave-select register.
static constexpr auto SPI_SER_MASK = SPI_SER_CS0_BIT;
/// @brief SPI 接收帧数减 1 字段的位掩码。
///        Bit mask for SPI receive-frame-count-minus-one field.
static constexpr auto SPI_CTRLR1_NDF_MASK = 0xFFFFUL;

/// @brief 可传输的最小 BAUDR 分频值。
///        Minimum BAUDR divisor usable for transfers.
static constexpr auto SPI_BAUDR_MIN = 2U;
/// @brief 可传输的最大偶数 BAUDR 分频值。
///        Maximum even BAUDR divisor usable for transfers.
static constexpr auto SPI_BAUDR_MAX = 65'534U;
/// @brief 关闭串行输出时钟的 BAUDR 编码。
///        BAUDR encoding that disables the serial output clock.
static constexpr auto SPI_BAUDR_CLOCK_DISABLED = 0U;
/// @brief SPI 波特率分频寄存器的位掩码。
///        Bit mask for SPI baud-divisor register.
static constexpr auto SPI_BAUDR_MASK = 0xFFFFUL;
/// @brief SPI FIFO 条目计数的位掩码。
///        Bit mask for SPI FIFO entry count.
static constexpr auto SPI_FIFO_LEVEL_MASK = 0x0FUL;
/// @brief SPI FIFO 阈值的位掩码。
///        Bit mask for SPI FIFO threshold.
static constexpr auto SPI_FIFO_THRESHOLD_MASK = 0x07UL;
/// @brief FIFO 阈值字段的最大编码。
///        Maximum FIFO threshold-field encoding.
static constexpr auto SPI_FIFO_THRESHOLD_MAX = SPI_FIFO_THRESHOLD_MASK;
/// @brief SPI DMA 请求阈值的位掩码。
///        Bit mask for SPI DMA request threshold.
static constexpr auto SPI_DMA_LEVEL_MASK = 0x07UL;

/// @brief SSI 忙状态位。
///        Bit for SSI busy status.
static constexpr auto SPI_SR_BUSY_BIT = (1UL << 0);
/// @brief SPI 发送 FIFO 非满状态位。
///        Bit for SPI transmit FIFO not-full status.
static constexpr auto SPI_SR_TFNF_BIT = (1UL << 1);
/// @brief SPI 发送 FIFO 空状态位。
///        Bit for SPI transmit FIFO empty status.
static constexpr auto SPI_SR_TFE_BIT = (1UL << 2);
/// @brief SPI 接收 FIFO 非空状态位。
///        Bit for SPI receive FIFO not-empty status.
static constexpr auto SPI_SR_RFNE_BIT = (1UL << 3);
/// @brief SPI 接收 FIFO 满状态位。
///        Bit for SPI receive FIFO full status.
static constexpr auto SPI_SR_RFF_BIT = (1UL << 4);
/// @brief SPI 发送错误状态位。
///        Bit for SPI transmit-error status.
static constexpr auto SPI_SR_TX_ERROR_BIT = (1UL << 5);
/// @brief SPI 冲突错误状态位。
///        Bit for SPI collision-error status.
static constexpr auto SPI_SR_COLLISION_ERROR_BIT = (1UL << 6);
/// @brief SPI 状态寄存器错误字段的位掩码。
///        Bit mask for SPI status-register error fields.
static constexpr auto SPI_SR_ERROR_MASK = (SPI_SR_TX_ERROR_BIT | SPI_SR_COLLISION_ERROR_BIT);

/// @brief SPI 发送 FIFO 空中断位。
///        Bit for SPI transmit FIFO empty interrupt.
static constexpr auto SPI_INTERRUPT_TX_EMPTY_BIT = (1UL << 0);
/// @brief SPI 发送 FIFO 溢出中断位。
///        Bit for SPI transmit FIFO overflow interrupt.
static constexpr auto SPI_INTERRUPT_TX_OVERFLOW_BIT = (1UL << 1);
/// @brief SPI 接收 FIFO 下溢中断位。
///        Bit for SPI receive FIFO underflow interrupt.
static constexpr auto SPI_INTERRUPT_RX_UNDERFLOW_BIT = (1UL << 2);
/// @brief SPI 接收 FIFO 溢出中断位。
///        Bit for SPI receive FIFO overflow interrupt.
static constexpr auto SPI_INTERRUPT_RX_OVERFLOW_BIT = (1UL << 3);
/// @brief SPI 接收 FIFO 满中断位。
///        Bit for SPI receive FIFO full interrupt.
static constexpr auto SPI_INTERRUPT_RX_FULL_BIT = (1UL << 4);
/// @brief SPI 多主机竞争中断位。
///        Bit for SPI multi-master contention interrupt.
static constexpr auto SPI_INTERRUPT_MULTI_MASTER_BIT = (1UL << 5);
/// @brief SPI 已定义中断字段的位掩码。
///        Bit mask for SPI defined interrupt fields.
static constexpr auto SPI_INTERRUPT_MASK = 0x3FUL;
/// @brief SPI 原始错误中断字段的位掩码。
///        Bit mask for SPI raw error-interrupt fields.
static constexpr auto SPI_RISR_ERROR_MASK =
    (SPI_INTERRUPT_TX_OVERFLOW_BIT | SPI_INTERRUPT_RX_UNDERFLOW_BIT | SPI_INTERRUPT_RX_OVERFLOW_BIT |
     SPI_INTERRUPT_MULTI_MASTER_BIT);

/// @brief SPI 接收 DMA 请求使能位。
///        Bit for SPI receive DMA request enable.
static constexpr auto SPI_DMACR_RX_ENABLE_BIT = (1UL << 0);
/// @brief SPI 发送 DMA 请求使能位。
///        Bit for SPI transmit DMA request enable.
static constexpr auto SPI_DMACR_TX_ENABLE_BIT = (1UL << 1);
/// @brief SPI DMA 请求控制寄存器的位掩码。
///        Bit mask for SPI DMA request-control register.
static constexpr auto SPI_DMACR_MASK = 0x03UL;
/// @brief SPI 发送 DMA 请求阈值的位掩码。
///        Bit mask for SPI transmit DMA request threshold.
static constexpr auto SPI_DMATDLR_MASK = SPI_DMA_LEVEL_MASK;
/// @brief SPI 接收 DMA 请求阈值的位掩码。
///        Bit mask for SPI receive DMA request threshold.
static constexpr auto SPI_DMARDLR_MASK = SPI_DMA_LEVEL_MASK;
/// @brief SPI FIFO 数据端口的位掩码。
///        Bit mask for SPI FIFO data port.
static constexpr auto SPI_DR_MASK = 0xFFFFUL;
/// @brief SPI 接收采样延迟的位掩码。
///        Bit mask for SPI receive-sample delay.
static constexpr auto SPI_RX_SAMPLE_DLY_MASK = 0xFFUL;
/// @brief 硬件内部支持的最大接收采样延迟编码。
///        Maximum receive-sample-delay encoding supported internally.
static constexpr auto SPI_RX_SAMPLE_DLY_MAX = 7U;

/// @brief Microwire 传输模式位。
///        Bit for Microwire transfer mode.
static constexpr auto SPI_MWCR_TRANSFER_MODE_BIT = (1UL << 0);
/// @brief Microwire 控制方向位。
///        Bit for Microwire control direction.
static constexpr auto SPI_MWCR_CONTROL_BIT = (1UL << 1);
/// @brief Microwire 握手使能位。
///        Bit for Microwire handshake enable.
static constexpr auto SPI_MWCR_HANDSHAKE_BIT = (1UL << 2);
/// @brief Microwire 控制寄存器的位掩码。
///        Bit mask for Microwire control register.
static constexpr auto SPI_MWCR_MASK = 0x07UL;
/** @} */

/** @} */

/**
 * @defgroup SG2002_I2C DesignWare APB I2C / DesignWare APB I2C register block
 * @ingroup SG2002_DEVICE
 * @see sg2002-licheerv-nano-b.svd.（SVD 寄存器描述 / SVD register descriptions）。
 * @{
 */

/**
 * @brief DesignWare APB I2C 寄存器块。
 *        DesignWare APB I2C register block.
 *
 * @see sg2002-licheerv-nano-b.svd（SVD 寄存器描述 / SVD register descriptions）。
 */
typedef struct
{
    volatile uint32_t CON; ///< 偏移 0x000：I2C 控制寄存器。Offset 0x000: I2C control register.
    volatile uint32_t TAR; ///< 偏移 0x004：主机目标地址寄存器。Offset 0x004: master target-address register.
    volatile uint32_t SAR; ///< 偏移 0x008：从机自身地址寄存器。Offset 0x008: slave own-address register.
    volatile uint32_t
        RESERVED_00C; ///< 偏移 0x00C：未公开区域的占位空间。Offset 0x00C: padding for an unexposed area.
    volatile uint32_t
        DATA_CMD; ///< 偏移 0x010：FIFO 数据与传输命令端口。Offset 0x010: FIFO data and transfer-command port.
    volatile uint32_t
        SS_SCL_HCNT; ///< 偏移 0x014：标准模式 SCL 高电平计数。Offset 0x014: standard-mode SCL high count.
    volatile uint32_t
        SS_SCL_LCNT; ///< 偏移 0x018：标准模式 SCL 低电平计数。Offset 0x018: standard-mode SCL low count.
    volatile uint32_t
        FS_SCL_HCNT; ///< 偏移 0x01C：快速模式 SCL 高电平计数。Offset 0x01C: fast-mode SCL high count.
    volatile uint32_t
        FS_SCL_LCNT; ///< 偏移 0x020：快速模式 SCL 低电平计数。Offset 0x020: fast-mode SCL low count.
    volatile uint32_t
        HS_SCL_HCNT; ///< 偏移 0x024：高速模式 SCL 高电平计数。Offset 0x024: high-speed-mode SCL high count.
    volatile uint32_t
        HS_SCL_LCNT; ///< 偏移 0x028：高速模式 SCL 低电平计数。Offset 0x028: high-speed-mode SCL low count.
    volatile uint32_t INTR_STAT;     ///< 偏移 0x02C：屏蔽后中断状态。Offset 0x02C: masked interrupt status.
    volatile uint32_t INTR_MASK;     ///< 偏移 0x030：中断使能掩码。Offset 0x030: interrupt-enable mask.
    volatile uint32_t RAW_INTR_STAT; ///< 偏移 0x034：屏蔽前中断状态。Offset 0x034: unmasked interrupt status.
    volatile uint32_t RX_TL;         ///< 偏移 0x038：接收 FIFO 阈值。Offset 0x038: receive FIFO threshold.
    volatile uint32_t TX_TL;         ///< 偏移 0x03C：发送 FIFO 阈值。Offset 0x03C: transmit FIFO threshold.
    volatile uint32_t
        CLR_INTR; ///< 偏移 0x040：读取以清除全部中断状态。Offset 0x040: read to clear all interrupts status.
    volatile uint32_t CLR_RX_UNDER; ///< 偏移 0x044：读取以清除接收下溢状态。Offset 0x044: read to clear
                                    ///< receive underflow status.
    volatile uint32_t CLR_RX_OVER;  ///< 偏移 0x048：读取以清除接收溢出状态。Offset 0x048: read to clear
                                    ///< receive overflow status.
    volatile uint32_t CLR_TX_OVER;  ///< 偏移 0x04C：读取以清除发送溢出状态。Offset 0x04C: read to clear
                                    ///< transmit overflow status.
    volatile uint32_t
        CLR_RD_REQ; ///< 偏移 0x050：读取以清除读请求状态。Offset 0x050: read to clear read request status.
    volatile uint32_t CLR_TX_ABRT;   ///< 偏移 0x054：读取以清除发送中止状态。Offset 0x054: read to clear
                                     ///< transmit abort status.
    volatile uint32_t CLR_RX_DONE;   ///< 偏移 0x058：读取以清除接收完成状态。Offset 0x058: read to clear
                                     ///< receive completion status.
    volatile uint32_t CLR_ACTIVITY;  ///< 偏移 0x05C：读取以清除活动状态状态。Offset 0x05C: read to clear
                                     ///< activity status status.
    volatile uint32_t CLR_STOP_DET;  ///< 偏移 0x060：读取以清除停止条件检测状态。Offset 0x060: read to clear
                                     ///< STOP detection status.
    volatile uint32_t CLR_START_DET; ///< 偏移 0x064：读取以清除起始条件检测状态。Offset 0x064: read to clear
                                     ///< START detection status.
    volatile uint32_t CLR_GEN_CALL;  ///< 偏移 0x068：读取以清除通用呼叫状态。Offset 0x068: read to clear
                                     ///< general call status.
    volatile uint32_t
        ENABLE; ///< 偏移 0x06C：控制器使能与中止控制。Offset 0x06C: controller enable and abort control.
    volatile uint32_t STATUS;   ///< 偏移 0x070：控制器状态。Offset 0x070: controller status.
    volatile uint32_t TXFLR;    ///< 偏移 0x074：发送 FIFO 条目数。Offset 0x074: transmit FIFO entry count.
    volatile uint32_t RXFLR;    ///< 偏移 0x078：接收 FIFO 条目数。Offset 0x078: receive FIFO entry count.
    volatile uint32_t SDA_HOLD; ///< 偏移 0x07C：SDA 保持时间。Offset 0x07C: SDA hold time.
    volatile uint32_t TX_ABRT_SOURCE; ///< 偏移 0x080：发送中止原因。Offset 0x080: transmit-abort source.
    volatile uint32_t
        RESERVED_084; ///< 偏移 0x084：未公开区域的占位空间。Offset 0x084: padding for an unexposed area.
    volatile uint32_t DMA_CR; ///< 偏移 0x088：DMA 请求使能。Offset 0x088: DMA request enables.
    volatile uint32_t
        DMA_TDLR; ///< 偏移 0x08C：发送 DMA 请求阈值。Offset 0x08C: transmit DMA request threshold.
    volatile uint32_t
        DMA_RDLR; ///< 偏移 0x090：接收 DMA 请求阈值。Offset 0x090: receive DMA request threshold.
    volatile uint32_t SDA_SETUP; ///< 偏移 0x094：SDA 建立时间。Offset 0x094: SDA setup time.
    volatile uint32_t
        RESERVED_098; ///< 偏移 0x098：未公开区域的占位空间。Offset 0x098: padding for an unexposed area.
    volatile uint32_t ENABLE_STATUS; ///< 偏移 0x09C：实际使能状态。Offset 0x09C: actual enable status.
    volatile uint32_t SPKLEN;        ///< 偏移 0x0A0：毛刺抑制长度。Offset 0x0A0: spike-suppression length.
    volatile uint32_t
        RESERVED_0A4; ///< 偏移 0x0A4：未公开区域的占位空间。Offset 0x0A4: padding for an unexposed area.
    volatile uint32_t CLR_RESTART_DET; ///< 偏移 0x0A8：读取以清除重复起始检测状态。Offset 0x0A8: read to
                                       ///< clear RESTART detection status.
    uint32_t
        RESERVED_0AC[18]; ///< 偏移 0x0AC：未公开区域的占位空间。Offset 0x0AC: padding for an unexposed area.
    volatile uint32_t
        COMP_PARAM_1; ///< 偏移 0x0F4：组件参数寄存器。Offset 0x0F4: component-parameter register.
    volatile uint32_t COMP_VERSION; ///< 偏移 0x0F8：组件版本寄存器。Offset 0x0F8: component-version register.
    volatile uint32_t COMP_TYPE;    ///< 偏移 0x0FC：组件类型寄存器。Offset 0x0FC: component-type register.
} I2C_Type;

static_assert(offsetof(I2C_Type, DATA_CMD) == 0x010U);
static_assert(offsetof(I2C_Type, DMA_CR) == 0x088U);
static_assert(offsetof(I2C_Type, ENABLE_STATUS) == 0x09CU);
static_assert(offsetof(I2C_Type, CLR_RESTART_DET) == 0x0A8U);
static_assert(offsetof(I2C_Type, COMP_PARAM_1) == 0x0F4U);

/**
 * @brief I2C0 寄存器实例指针。
 *        I2C0 register-instance pointer.
 */
#define I2C0_REGS ((I2C_Type *)(uintptr_t)I2C0_BASE)

/**
 * @brief I2C1 寄存器实例指针。
 *        I2C1 register-instance pointer.
 */
#define I2C1_REGS ((I2C_Type *)(uintptr_t)I2C1_BASE)

/**
 * @brief I2C2 寄存器实例指针。
 *        I2C2 register-instance pointer.
 */
#define I2C2_REGS ((I2C_Type *)(uintptr_t)I2C2_BASE)

/**
 * @brief I2C3 寄存器实例指针。
 *        I2C3 register-instance pointer.
 */
#define I2C3_REGS ((I2C_Type *)(uintptr_t)I2C3_BASE)

/**
 * @brief I2C4 寄存器实例指针。
 *        I2C4 register-instance pointer.
 */
#define I2C4_REGS ((I2C_Type *)(uintptr_t)I2C4_BASE)

/**
 * @name I2C 速率编码 / I2C speed encodings
 * @see sg2002-licheerv-nano-b.svd, DesignWare APB I2C registers.（SVD 寄存器描述 / SVD register
 * descriptions）。
 * @{
 */
/// @brief I2C 标准速率模式编码。
///        I2C standard-speed mode encoding.
static constexpr auto I2C_SPEED_STANDARD = 1U;
/// @brief I2C 快速模式编码。
///        I2C fast-mode encoding.
static constexpr auto I2C_SPEED_FAST = 2U;
/// @brief I2C 高速模式编码。
///        I2C high-speed mode encoding.
static constexpr auto I2C_SPEED_HIGH = 3U;
/** @} */

/**
 * @name I2C 时序配置 / I2C timing profiles
 * 计数值以 IP 时钟周期为单位。
 *        Counts are expressed in IP-clock cycles.
 * @see SG2002 技术参考手册 v1.02, 表 21.1。SG2002 TRM v1.02, Table 21.1.
 * @{
 */
/// @brief 25 MHz 配置的输入时钟频率，单位为 Hz。
///        Input clock frequency in hz for the 25 MHz profile.
static constexpr auto I2C_TIMING_25M_CLOCK_HZ = 25'000'000UL;
/// @brief 25 MHz 配置的标准模式 SCL 高电平计数。
///        Standard-mode scl high count for the 25 MHz profile.
static constexpr auto I2C_TIMING_25M_SS_HCNT = 115U;
/// @brief 25 MHz 配置的标准模式 SCL 低电平计数。
///        Standard-mode scl low count for the 25 MHz profile.
static constexpr auto I2C_TIMING_25M_SS_LCNT = 135U;
/// @brief 25 MHz 配置的快速模式 SCL 高电平计数。
///        Fast-mode scl high count for the 25 MHz profile.
static constexpr auto I2C_TIMING_25M_FS_HCNT = 21U;
/// @brief 25 MHz 配置的快速模式 SCL 低电平计数。
///        Fast-mode scl low count for the 25 MHz profile.
static constexpr auto I2C_TIMING_25M_FS_LCNT = 42U;
/// @brief 25 MHz 配置的SDA 建立时间计数。
///        Sda setup-time count for the 25 MHz profile.
static constexpr auto I2C_TIMING_25M_SDA_SETUP = 6U;
/// @brief 25 MHz 配置的毛刺抑制周期数。
///        Spike-suppression cycle count for the 25 MHz profile.
static constexpr auto I2C_TIMING_25M_SPKLEN = 2U;
/// @brief 100 MHz 配置的输入时钟频率，单位为 Hz。
///        Input clock frequency in hz for the 100 MHz profile.
static constexpr auto I2C_TIMING_100M_CLOCK_HZ = 100'000'000UL;
/// @brief 100 MHz 配置的标准模式 SCL 高电平计数。
///        Standard-mode scl high count for the 100 MHz profile.
static constexpr auto I2C_TIMING_100M_SS_HCNT = 460U;
/// @brief 100 MHz 配置的标准模式 SCL 低电平计数。
///        Standard-mode scl low count for the 100 MHz profile.
static constexpr auto I2C_TIMING_100M_SS_LCNT = 540U;
/// @brief 100 MHz 配置的快速模式 SCL 高电平计数。
///        Fast-mode scl high count for the 100 MHz profile.
static constexpr auto I2C_TIMING_100M_FS_HCNT = 90U;
/// @brief 100 MHz 配置的快速模式 SCL 低电平计数。
///        Fast-mode scl low count for the 100 MHz profile.
static constexpr auto I2C_TIMING_100M_FS_LCNT = 160U;
/// @brief 100 MHz 配置的SDA 建立时间计数。
///        Sda setup-time count for the 100 MHz profile.
static constexpr auto I2C_TIMING_100M_SDA_SETUP = 25U;
/// @brief 100 MHz 配置的毛刺抑制周期数。
///        Spike-suppression cycle count for the 100 MHz profile.
static constexpr auto I2C_TIMING_100M_SPKLEN = 5U;
/// @brief SDA 保持时间配置，单位为 IP 时钟周期。
///        SDA hold-time setting in IP-clock cycles.
static constexpr auto I2C_TIMING_SDA_HOLD = 1U;
/** @} */

/// @brief I2C 收发 FIFO 的条目深度。
///        Entry depth of the I2C receive/transmit FIFOs.
static constexpr auto I2C_FIFO_DEPTH = 64U;

/// @brief I2C SCL 高低电平计数上限。
///        Maximum I2C SCL high/low count.
static constexpr auto I2C_SCL_COUNT_MAX = 0xFFFFU;
/// @brief I2C SDA 发送保持计数上限。
///        Maximum I2C SDA transmit-hold count.
static constexpr auto I2C_SDA_HOLD_TX_MAX = 0xFFFFU;
/// @brief I2C SDA 建立计数下限。
///        Minimum I2C SDA setup count.
static constexpr auto I2C_SDA_SETUP_MIN = 2U;
/// @brief I2C SDA 建立计数上限。
///        Maximum I2C SDA setup count.
static constexpr auto I2C_SDA_SETUP_MAX = 0xFFU;
/// @brief I2C 毛刺抑制计数上限。
///        Maximum I2C spike-suppression count.
static constexpr auto I2C_SPKLEN_MAX = 0xFFU;
/// @brief I2C 标准模式最小 SCL 高电平时间，单位 ns。
///        Minimum standard-mode I2C SCL high time in ns.
static constexpr auto I2C_STANDARD_HIGH_NS = 4'000U;
/// @brief I2C 标准模式最小 SCL 低电平时间，单位 ns。
///        Minimum standard-mode I2C SCL low time in ns.
static constexpr auto I2C_STANDARD_LOW_NS = 4'700U;
/// @brief I2C 快速模式最小 SCL 高电平时间，单位 ns。
///        Minimum fast-mode I2C SCL high time in ns.
static constexpr auto I2C_FAST_HIGH_NS = 600U;
/// @brief I2C 快速模式最小 SCL 低电平时间，单位 ns。
///        Minimum fast-mode I2C SCL low time in ns.
static constexpr auto I2C_FAST_LOW_NS = 1'300U;
/// @brief I2C 默认 SCL 下降时间预算，单位 ns。
///        Default I2C SCL fall-time budget in ns.
static constexpr auto I2C_SCL_FALL_NS = 300U;
/// @brief I2C 默认 SDA 保持时间，单位 ns。
///        Default I2C SDA hold time in ns.
static constexpr auto I2C_SDA_HOLD_NS = 300U;
/// @brief I2C 默认 SDA 建立时间，单位 ns。
///        Default I2C SDA setup time in ns.
static constexpr auto I2C_SDA_SETUP_NS = 1'000U;
/// @brief I2C 默认毛刺抑制时间，单位 ns。
///        Default I2C spike-suppression time in ns.
static constexpr auto I2C_SPIKE_SUPPRESSION_NS = 50U;
/// @brief DesignWare SCL 高电平内部延迟，单位 IP 周期。
///        DesignWare internal SCL high latency in IP cycles.
static constexpr auto I2C_SCL_HIGH_LATENCY = 3U;
/// @brief DesignWare SCL 低电平内部延迟，单位 IP 周期。
///        DesignWare internal SCL low latency in IP cycles.
static constexpr auto I2C_SCL_LOW_LATENCY = 1U;

/// @brief I2C 7 位目标地址字段的位掩码。
///        Bit mask for I2C seven-bit target-address field.
static constexpr auto I2C_TAR_7BIT_ADDRESS_MASK = 0x7FU;
/// @brief I2C TAR 的复位目标地址。
///        Reset target address of I2C TAR.
static constexpr auto I2C_TAR_RESET_ADDRESS = 0x55U;
/// @brief I2C 主机模式使能位。
///        Bit for I2C master-mode enable.
static constexpr auto I2C_CON_MASTER_BIT = (1UL << 0);
/// @brief I2C 速率模式字段的最低有效位位置。
///        Least-significant bit position of I2C speed-mode field.
static constexpr auto I2C_CON_SPEED_SHIFT = 1U;
/// @brief I2C 速率模式字段的位掩码。
///        Bit mask for I2C speed-mode field.
static constexpr auto I2C_CON_SPEED_MASK = (3UL << I2C_CON_SPEED_SHIFT);
/// @brief I2C 从机 10 位寻址位。
///        Bit for I2C slave ten-bit addressing.
static constexpr auto I2C_CON_SLAVE_10BIT_BIT = (1UL << 3);
/// @brief I2C 主机 10 位寻址位。
///        Bit for I2C master ten-bit addressing.
static constexpr auto I2C_CON_MASTER_10BIT_BIT = (1UL << 4);
/// @brief I2C 重复起始使能位。
///        Bit for I2C repeated-start enable.
static constexpr auto I2C_CON_RESTART_ENABLE_BIT = (1UL << 5);
/// @brief I2C 从机禁止位。
///        Bit for I2C slave disable.
static constexpr auto I2C_CON_SLAVE_DISABLE_BIT = (1UL << 6);
/// @brief I2C 被寻址时的停止检测选择位。
///        Bit for I2C addressed-only STOP detection selection.
static constexpr auto I2C_CON_STOP_DET_IF_ADDRESSED_BIT = (1UL << 7);
/// @brief I2C 发送空中断控制位。
///        Bit for I2C transmit-empty interrupt control.
static constexpr auto I2C_CON_TX_EMPTY_CTRL_BIT = (1UL << 8);
/// @brief I2C 接收 FIFO 满时保持控制位。
///        Bit for I2C receive-FIFO-full hold control.
static constexpr auto I2C_CON_RX_FIFO_FULL_HOLD_BIT = (1UL << 9);
/// @brief I2C 总线清除控制位。
///        Bit for I2C bus-clear control.
static constexpr auto I2C_CON_BUS_CLEAR_CTRL_BIT = (1UL << 11);
/// @brief I2C 目标地址字段的位掩码。
///        Bit mask for I2C target-address field.
static constexpr auto I2C_TAR_ADDRESS_MASK = 0x3FFUL;
/// @brief I2C TAR 的 10 位寻址选择位。
///        Bit for I2C TAR ten-bit addressing selection.
static constexpr auto I2C_TAR_10BIT_BIT = (1UL << 12);
/// @brief I2C 命令数据字段的位掩码。
///        Bit mask for I2C command-data field.
static constexpr auto I2C_DATA_CMD_DATA_MASK = 0xFFUL;
/// @brief I2C 读请求命令位。
///        Bit for I2C read-request command.
static constexpr auto I2C_DATA_CMD_READ_BIT = (1UL << 8);
/// @brief I2C 停止条件命令位。
///        Bit for I2C STOP command.
static constexpr auto I2C_DATA_CMD_STOP_BIT = (1UL << 9);
/// @brief I2C 重复起始命令位。
///        Bit for I2C RESTART command.
static constexpr auto I2C_DATA_CMD_RESTART_BIT = (1UL << 10);
/// @brief I2C 首字节标志位。
///        Bit for I2C first-byte flag.
static constexpr auto I2C_DATA_CMD_FIRST_BYTE_BIT = (1UL << 11);
/// @brief I2C 发送中止中断位。
///        Bit for I2C transmit-abort interrupt.
static constexpr auto I2C_INTR_TX_ABRT_BIT = (1UL << 6);
/// @brief I2C 停止条件检测中断位。
///        Bit for I2C STOP-detection interrupt.
static constexpr auto I2C_INTR_STOP_DET_BIT = (1UL << 9);
/// @brief I2C 控制器使能位。
///        Bit for I2C controller enable.
static constexpr auto I2C_ENABLE_BIT = (1UL << 0);
/// @brief I2C 中止请求位。
///        Bit for I2C abort request.
static constexpr auto I2C_ENABLE_ABORT_BIT = (1UL << 1);
/// @brief I2C 控制器实际使能状态位。
///        Bit for I2C controller actual enable status.
static constexpr auto I2C_ENABLE_STATUS_BIT = (1UL << 0);
/// @brief I2C 接收 DMA 请求使能位。
///        Bit for I2C receive DMA request enable.
static constexpr auto I2C_DMA_RX_ENABLE_BIT = (1UL << 0);
/// @brief I2C 发送 DMA 请求使能位。
///        Bit for I2C transmit DMA request enable.
static constexpr auto I2C_DMA_TX_ENABLE_BIT = (1UL << 1);

/** @} */

/**
 * @defgroup SG2002_UART DesignWare APB UART / DesignWare APB UART register block
 * @ingroup SG2002_DEVICE
 * @see sg2002-licheerv-nano-b.svd.（SVD 寄存器描述 / SVD register descriptions）。
 * @{
 */

/**
 * @brief DesignWare APB UART 寄存器块。
 *        DesignWare APB UART register block.
 *
 * @see sg2002-licheerv-nano-b.svd, DW_apb_uart views（SVD 寄存器描述 / SVD register descriptions）。
 */
typedef struct
{
    volatile uint32_t RBR_THR_DLL; ///< 偏移 0x000：接收缓冲、发送保持或分频低字节，按访问方向和 DLAB
                                   ///< 选择。Offset 0x000: receive buffer, transmit holding, or divisor low
                                   ///< byte, selected by access direction and DLAB.
    volatile uint32_t IER_DLH; ///< 偏移 0x004：中断使能或分频高字节，按 DLAB 选择。Offset 0x004: interrupt
                               ///< enable or divisor high byte, selected by DLAB.
    volatile uint32_t IIR_FCR; ///< 偏移 0x008：读中断标识、写 FIFO 控制。Offset 0x008: read interrupt
                               ///< identification, write FIFO control.
    volatile uint32_t LCR; ///< 偏移 0x00C：线路格式与 DLAB 控制。Offset 0x00C: line-format and DLAB control.
    volatile uint32_t MCR; ///< 偏移 0x010：调制解调器控制。Offset 0x010: modem control.
    volatile uint32_t LSR; ///< 偏移 0x014：线路状态。Offset 0x014: line status.
    volatile uint32_t MSR; ///< 偏移 0x018：调制解调器状态。Offset 0x018: modem status.
    volatile uint32_t SCR; ///< 偏移 0x01C：暂存寄存器。Offset 0x01C: scratch register.
    volatile uint32_t
        LPDLL; ///< 偏移 0x020：低功耗分频锁存器低字节。Offset 0x020: low-power divisor-latch low byte.
    volatile uint32_t
        LPDLH; ///< 偏移 0x024：低功耗分频锁存器高字节。Offset 0x024: low-power divisor-latch high byte.
    uint32_t
        RESERVED_028[2]; ///< 偏移 0x028：未公开区域的占位空间。Offset 0x028: padding for an unexposed area.
    volatile uint32_t
        SHADOW[16];         ///< 偏移 0x030：FIFO 影子数据端口数组。Offset 0x030: shadow FIFO data-port array.
    volatile uint32_t FAR;  ///< 偏移 0x070：FIFO 访问控制。Offset 0x070: FIFO access control.
    volatile uint32_t TFR;  ///< 偏移 0x074：发送 FIFO 读取端口。Offset 0x074: transmit FIFO read port.
    volatile uint32_t RFW;  ///< 偏移 0x078：接收 FIFO 写入端口。Offset 0x078: receive FIFO write port.
    volatile uint32_t USR;  ///< 偏移 0x07C：UART 状态寄存器。Offset 0x07C: UART status register.
    volatile uint32_t TFL;  ///< 偏移 0x080：发送 FIFO 条目数。Offset 0x080: transmit FIFO entry count.
    volatile uint32_t RFL;  ///< 偏移 0x084：接收 FIFO 条目数。Offset 0x084: receive FIFO entry count.
    volatile uint32_t SRR;  ///< 偏移 0x088：软件复位寄存器。Offset 0x088: software-reset register.
    volatile uint32_t SRTS; ///< 偏移 0x08C：RTS 影子寄存器。Offset 0x08C: shadow RTS register.
    volatile uint32_t SBCR; ///< 偏移 0x090：间断控制影子寄存器。Offset 0x090: shadow break-control register.
    volatile uint32_t SDMAM; ///< 偏移 0x094：DMA 模式影子寄存器。Offset 0x094: shadow DMA-mode register.
    volatile uint32_t SFE;   ///< 偏移 0x098：FIFO 使能影子寄存器。Offset 0x098: shadow FIFO-enable register.
    volatile uint32_t SRT; ///< 偏移 0x09C：接收触发影子寄存器。Offset 0x09C: shadow receive-trigger register.
    volatile uint32_t
        STET; ///< 偏移 0x0A0：发送空触发影子寄存器。Offset 0x0A0: shadow transmit-empty trigger register.
    volatile uint32_t HTX;  ///< 偏移 0x0A4：暂停发送控制。Offset 0x0A4: halt-transmit control.
    volatile uint32_t DMSA; ///< 偏移 0x0A8：DMA 软件应答。Offset 0x0A8: DMA software acknowledgement.
    uint32_t
        RESERVED_0AC[18];  ///< 偏移 0x0AC：未公开区域的占位空间。Offset 0x0AC: padding for an unexposed area.
    volatile uint32_t CPR; ///< 偏移 0x0F4：组件参数寄存器。Offset 0x0F4: component-parameter register.
    volatile uint32_t UCV_CTR; ///< 偏移 0x0F8：UCV/CTR 寄存器视图。Offset 0x0F8: UCV/CTR register view.
} UART_Type;

static_assert(offsetof(UART_Type, SHADOW) == 0x030U);
static_assert(offsetof(UART_Type, USR) == 0x07CU);
static_assert(offsetof(UART_Type, DMSA) == 0x0A8U);
static_assert(offsetof(UART_Type, CPR) == 0x0F4U);

/**
 * @brief UART0 寄存器实例指针。
 *        UART0 register-instance pointer.
 */
#define UART0_REGS ((UART_Type *)(uintptr_t)UART0_BASE)

/**
 * @brief UART1 寄存器实例指针。
 *        UART1 register-instance pointer.
 */
#define UART1_REGS ((UART_Type *)(uintptr_t)UART1_BASE)

/**
 * @brief UART2 寄存器实例指针。
 *        UART2 register-instance pointer.
 */
#define UART2_REGS ((UART_Type *)(uintptr_t)UART2_BASE)

/**
 * @brief UART3 寄存器实例指针。
 *        UART3 register-instance pointer.
 */
#define UART3_REGS ((UART_Type *)(uintptr_t)UART3_BASE)

/**
 * @brief UART4 寄存器实例指针。
 *        UART4 register-instance pointer.
 */
#define UART4_REGS ((UART_Type *)(uintptr_t)UART4_BASE)

/**
 * @name UART 寄存器字段 / UART register fields
 * @see sg2002-licheerv-nano-b.svd, DW_apb_uart register views.（SVD 寄存器描述 / SVD register
 * descriptions）。
 * @{
 */
/// @brief UART 接收数据中断使能位。
///        Bit for UART received-data interrupt enable.
static constexpr auto UART_IER_ERBFI_BIT = (1UL << 0);
/// @brief UART 发送保持寄存器空中断使能位。
///        Bit for UART transmit-holding-empty interrupt enable.
static constexpr auto UART_IER_ETBEI_BIT = (1UL << 1);
/// @brief UART 接收线路状态中断使能位。
///        Bit for UART receive-line-status interrupt enable.
static constexpr auto UART_IER_ELSI_BIT = (1UL << 2);
/// @brief UART 调制解调器状态中断使能位。
///        Bit for UART modem-status interrupt enable.
static constexpr auto UART_IER_EDSSI_BIT = (1UL << 3);
/// @brief UART 可编程 THRE 中断模式位。
///        Bit for UART programmable THRE interrupt mode.
static constexpr auto UART_IER_PTIME_BIT = (1UL << 7);
/**
 * @brief UART 中断标识字段的位掩码。
 *        Bit mask for UART interrupt-identification field.
 *
 * @note 中断 ID 占 IIR[3:0]，见 TRM 表 21.48。
 *       The interrupt ID occupies IIR[3:0]; see TRM Table 21.48.
 */
static constexpr auto UART_IIR_IID_MASK = 0x0FUL;
/// @brief UART FIFO 状态字段的位掩码。
///        Bit mask for UART FIFO-status field.
static constexpr auto UART_IIR_FIFO_STATUS_MASK = (3UL << 6);
/// @brief UART FIFO 使能位。
///        Bit for UART FIFO enable.
static constexpr auto UART_FCR_FIFO_ENABLE_BIT = (1UL << 0);
/// @brief UART 接收 FIFO 复位位。
///        Bit for UART receive FIFO reset.
static constexpr auto UART_FCR_RX_FIFO_RESET_BIT = (1UL << 1);
/// @brief UART 发送 FIFO 复位位。
///        Bit for UART transmit FIFO reset.
static constexpr auto UART_FCR_TX_FIFO_RESET_BIT = (1UL << 2);
/// @brief UART DMA 模式选择位。
///        Bit for UART DMA-mode selection.
static constexpr auto UART_FCR_DMA_MODE_BIT = (1UL << 3);
/// @brief UART 发送 FIFO 触发字段的最低有效位位置。
///        Least-significant bit position of UART transmit FIFO trigger field.
static constexpr auto UART_FCR_TX_TRIGGER_SHIFT = 4U;
/// @brief UART 接收 FIFO 触发字段的最低有效位位置。
///        Least-significant bit position of UART receive FIFO trigger field.
static constexpr auto UART_FCR_RX_TRIGGER_SHIFT = 6U;
/// @brief UART FIFO 触发字段的最大编码。
///        Maximum UART FIFO trigger-field encoding.
static constexpr auto UART_FCR_TRIGGER_MAX = 3U;
/// @brief UART 波特率发生器的过采样倍数。
///        UART baud-generator oversampling factor.
static constexpr auto UART_BAUD_OVERSAMPLING = 16U;
/// @brief UART 波特率分频锁存器的最大值。
///        Maximum UART baud-divisor latch value.
static constexpr auto UART_BAUD_DIVISOR_MAX = 65'535U;
/// @brief UART 数据长度字段的位掩码。
///        Bit mask for UART data-length field.
static constexpr auto UART_LCR_DATA_LENGTH_MASK = 3UL;
/// @brief UART 停止位选择位。
///        Bit for UART stop-bit selection.
static constexpr auto UART_LCR_STOP_BIT = (1UL << 2);
/// @brief UART 奇偶校验使能位。
///        Bit for UART parity enable.
static constexpr auto UART_LCR_PARITY_ENABLE_BIT = (1UL << 3);
/// @brief UART 偶校验选择位。
///        Bit for UART even-parity selection.
static constexpr auto UART_LCR_EVEN_PARITY_BIT = (1UL << 4);
/// @brief UART 固定校验位选择位。
///        Bit for UART stick-parity selection.
static constexpr auto UART_LCR_STICK_PARITY_BIT = (1UL << 5);
/// @brief UART 间断控制位。
///        Bit for UART break control.
static constexpr auto UART_LCR_BREAK_BIT = (1UL << 6);
/// @brief UART 分频锁存器访问选择位。
///        Bit for UART divisor-latch access selection.
static constexpr auto UART_LCR_DLAB_BIT = (1UL << 7);
/// @brief UART 请求发送输出位。
///        Bit for UART request-to-send output.
static constexpr auto UART_MCR_RTS_BIT = (1UL << 1);
/// @brief UART 自动流控使能位。
///        Bit for UART automatic flow-control enable.
static constexpr auto UART_MCR_AUTO_FLOW_CONTROL_BIT = (1UL << 5);
/// @brief UART 接收数据就绪状态位。
///        Bit for UART received-data-ready status.
static constexpr auto UART_LSR_DATA_READY_BIT = (1UL << 0);
/// @brief UART 发送保持寄存器空状态位。
///        Bit for UART transmit-holding-register empty status.
static constexpr auto UART_LSR_THR_EMPTY_BIT = (1UL << 5);
/// @brief UART 发送器空状态位。
///        Bit for UART transmitter empty status.
static constexpr auto UART_LSR_TEMT_BIT = (1UL << 6);
/// @brief UART 忙状态位。
///        Bit for UART busy status.
static constexpr auto UART_USR_BUSY_BIT = (1UL << 0);
/// @brief UART 发送 FIFO 非满状态位。
///        Bit for UART transmit FIFO not-full status.
static constexpr auto UART_USR_TFNF_BIT = (1UL << 1);
/// @brief UART 发送 FIFO 空状态位。
///        Bit for UART transmit FIFO empty status.
static constexpr auto UART_USR_TFE_BIT = (1UL << 2);
/// @brief UART 接收 FIFO 非空状态位。
///        Bit for UART receive FIFO not-empty status.
static constexpr auto UART_USR_RFNE_BIT = (1UL << 3);
/// @brief UART 接收 FIFO 满状态位。
///        Bit for UART receive FIFO full status.
static constexpr auto UART_USR_RFF_BIT = (1UL << 4);
/** @} */

/** @} */

/**
 * @defgroup SG2002_PWM PWM 寄存器 / PWM registers
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02, Section 21.9。SG2002 TRM v1.02, Section 21.9.
 * @{
 */

/**
 * @brief 单个 PWM 通道的周期寄存器。
 *        Period registers for one PWM channel.
 */
typedef struct
{
    volatile uint32_t HLPERIOD; ///< 偏移 0x000：高低电平周期寄存器，单位为时钟周期。Offset 0x000: high/low
                                ///< period register in clock ticks.
    volatile uint32_t PERIOD;   ///< 偏移 0x004：完整 PWM 周期，单位为时钟周期。Offset 0x004: complete PWM
                                ///< period in clock ticks.
} PWM_Channel_Type;

/**
 * @brief PWM 周期、极性、启动、更新与输出使能寄存器。
 *        PWM period, polarity, start, update, and output-enable registers.
 */
typedef struct
{
    PWM_Channel_Type CHANNEL[PWM_CHANNELS_PER_CONTROLLER]; ///< 偏移 0x000：PWM 通道周期寄存器数组，通道步长 8
                                                           ///< 字节。Offset 0x000: PWM channel-period
                                                           ///< register array with an eight-byte stride.
    uint32_t
        RESERVED_020[8]; ///< 偏移 0x020：未公开区域的占位空间。Offset 0x020: padding for an unexposed area.
    volatile uint32_t POLARITY; ///< 偏移 0x040：输出极性选择。Offset 0x040: output-polarity selection.
    volatile uint32_t PWMSTART; ///< 偏移 0x044：通道启动控制。Offset 0x044: channel start control.
    uint32_t RESERVED_048; ///< 偏移 0x048：未公开区域的占位空间。Offset 0x048: padding for an unexposed area.
    volatile uint32_t
        PWMUPDATE; ///< 偏移 0x04C：通道动态更新控制。Offset 0x04C: channel dynamic-update control.
    uint32_t
        RESERVED_050[32]; ///< 偏移 0x050：未公开区域的占位空间。Offset 0x050: padding for an unexposed area.
    volatile uint32_t PWM_OE; ///< 偏移 0x0D0：通道输出使能。Offset 0x0D0: channel output enables.
} PWM_Type;

static_assert(sizeof(PWM_Channel_Type) == 0x008U);
static_assert(offsetof(PWM_Channel_Type, HLPERIOD) == 0x000U);
static_assert(offsetof(PWM_Channel_Type, PERIOD) == 0x004U);
static_assert(offsetof(PWM_Type, POLARITY) == 0x040U);
static_assert(offsetof(PWM_Type, PWMSTART) == 0x044U);
static_assert(offsetof(PWM_Type, PWMUPDATE) == 0x04CU);
static_assert(offsetof(PWM_Type, PWM_OE) == 0x0D0U);

/** @brief PWM0 寄存器实例。 PWM0 register instance. */
#define PWM0_REGS ((PWM_Type *)(uintptr_t)PWM0_BASE)
/** @brief PWM1 寄存器实例。 PWM1 register instance. */
#define PWM1_REGS ((PWM_Type *)(uintptr_t)PWM1_BASE)
/** @brief PWM2 寄存器实例。 PWM2 register instance. */
#define PWM2_REGS ((PWM_Type *)(uintptr_t)PWM2_BASE)
/** @brief PWM3 寄存器实例。 PWM3 register instance. */
#define PWM3_REGS ((PWM_Type *)(uintptr_t)PWM3_BASE)

/// @brief PWM 周期字段可表示的最大时钟计数。
///        Maximum clock count in a PWM period field.
static constexpr auto PWM_PERIOD_MAX = 0x3FFF'FFFFUL;
/// @brief 单个 PWM 控制器的有效通道位掩码。
///        Valid channel mask for one PWM controller.
static constexpr auto PWM_CHANNEL_MASK = (1UL << PWM_CHANNELS_PER_CONTROLLER) - 1UL;

/** @} */

/**
 * @defgroup SG2002_SARADC SARADC 寄存器 / SARADC registers
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02, Section 21.7。SG2002 TRM v1.02, Section 21.7.
 * @{
 */

/**
 * @brief SARADC 转换控制、采样结果和时序寄存器。
 *        SARADC conversion-control, result, and timing registers.
 */
typedef struct
{
    uint32_t RESERVED_000; ///< 偏移 0x000：未公开区域的占位空间。Offset 0x000: padding for an unexposed area.
    volatile uint32_t CTRL; ///< 偏移 0x004：转换触发和通道选择控制。Offset 0x004: conversion-trigger and
                            ///< channel-select control.
    volatile const uint32_t STATUS; ///< 偏移 0x008：转换忙状态。Offset 0x008: conversion-busy status.
    volatile uint32_t CYC_SET; ///< 偏移 0x00C：转换周期配置。Offset 0x00C: conversion-cycle configuration.
    uint32_t RESERVED_010; ///< 偏移 0x010：未公开区域的占位空间。Offset 0x010: padding for an unexposed area.
    volatile const uint32_t
        RESULT[SARADC_CHANNEL_COUNT]; ///< 偏移 0x014：输入通道 1 到 3 的采样结果数组。Offset 0x014:
                                      ///< sample-result array for input channels one through three.
    volatile uint32_t INTR_EN;        ///< 偏移 0x020：中断使能。Offset 0x020: interrupt enable.
    volatile uint32_t INTR_CLR;       ///< 偏移 0x024：中断清除。Offset 0x024: interrupt clear.
    uint32_t
        RESERVED_028[2]; ///< 偏移 0x028：未公开区域的占位空间。Offset 0x028: padding for an unexposed area.
    volatile uint32_t
        TEST; ///< 偏移 0x030：测试与参考电压选择。Offset 0x030: test and reference-voltage selection.
} SARADC_Type;

static_assert(offsetof(SARADC_Type, CTRL) == 0x004U);
static_assert(offsetof(SARADC_Type, STATUS) == 0x008U);
static_assert(offsetof(SARADC_Type, CYC_SET) == 0x00CU);
static_assert(offsetof(SARADC_Type, RESULT) == 0x014U);
static_assert(offsetof(SARADC_Type, INTR_EN) == 0x020U);
static_assert(offsetof(SARADC_Type, INTR_CLR) == 0x024U);
static_assert(offsetof(SARADC_Type, TEST) == 0x030U);

/// @brief SARADC 单次转换触发位。
///        SARADC single-conversion trigger bit.
static constexpr auto SARADC_CTRL_TRIGGER_BIT = (1UL << 0);
/// @brief SARADC 单热通道选择字段起始位，通道 1 对应位 5。
///        Start of the SARADC one-hot channel field; channel one uses bit five.
static constexpr auto SARADC_CTRL_CHANNEL_SHIFT = 4U;
/// @brief SARADC 单热通道选择字段掩码。
///        Mask of the SARADC one-hot channel-selection field.
static constexpr auto SARADC_CTRL_CHANNEL_MASK = (0xFUL << SARADC_CTRL_CHANNEL_SHIFT);
/// @brief SARADC 转换忙标志。
///        SARADC conversion-busy flag.
static constexpr auto SARADC_STATUS_BUSY_BIT = (1UL << 0);
/// @brief SARADC 结果有效标志。
///        SARADC result-valid flag.
static constexpr auto SARADC_RESULT_VALID_BIT = (1UL << 15);
/// @brief SARADC 12 位采样值掩码。
///        Mask of the twelve-bit SARADC sample value.
static constexpr auto SARADC_RESULT_DATA_MASK = 0xFFFUL;
/// @brief SARADC 转换中断使能及清除位。
///        SARADC conversion interrupt enable and clear bit.
static constexpr auto SARADC_INTERRUPT_BIT = (1UL << 0);
/// @brief SARADC 外部 VDD18A 参考电压选择位。
///        SARADC external VDD18A reference-selection bit.
static constexpr auto SARADC_TEST_EXTERNAL_REFERENCE_BIT = (1UL << 2);
/// @brief SARADC 启动稳定周期字段掩码。
///        SARADC settling-cycle field mask.
static constexpr auto SARADC_CYCLE_SETTLE_MASK = 0x1FUL;
/// @brief SARADC 采样周期字段起始位。
///        SARADC sampling-cycle field shift.
static constexpr auto SARADC_CYCLE_SAMPLE_SHIFT = 8U;
/// @brief SARADC 采样周期字段掩码。
///        SARADC sampling-cycle field mask.
static constexpr auto SARADC_CYCLE_SAMPLE_MASK = (0xFUL << SARADC_CYCLE_SAMPLE_SHIFT);
/// @brief SARADC 时钟分频字段起始位。
///        SARADC clock-divider field shift.
static constexpr auto SARADC_CYCLE_DIVIDER_SHIFT = 12U;
/// @brief SARADC 时钟分频字段掩码。
///        SARADC clock-divider field mask.
static constexpr auto SARADC_CYCLE_DIVIDER_MASK = (0xFUL << SARADC_CYCLE_DIVIDER_SHIFT);
/// @brief SARADC 分频编码上限，实际分频为编码加一。
///        Maximum SARADC divider encoding; the divisor is the encoding plus one.
static constexpr auto SARADC_CYCLE_DIVIDER_MAX = 15U;
/// @brief SARADC 比较周期字段起始位。
///        SARADC comparison-cycle field shift.
static constexpr auto SARADC_CYCLE_COMPARE_SHIFT = 16U;
/// @brief SARADC 比较周期字段掩码。
///        SARADC comparison-cycle field mask.
static constexpr auto SARADC_CYCLE_COMPARE_MASK = (0xFUL << SARADC_CYCLE_COMPARE_SHIFT);
/// @brief SARADC 默认稳定周期编码。
///        Default SARADC settling-cycle encoding.
static constexpr auto SARADC_CYCLE_SETTLE_DEFAULT = 15U;
/// @brief SARADC 默认采样周期编码。
///        Default SARADC sampling-cycle encoding.
static constexpr auto SARADC_CYCLE_SAMPLE_DEFAULT = 3U;
/// @brief SARADC 默认比较周期编码。
///        Default SARADC comparison-cycle encoding.
static constexpr auto SARADC_CYCLE_COMPARE_DEFAULT = 11U;

/** @brief 主域 SARADC 寄存器实例。 Main-domain SARADC register instance. */
#define SARADC_REGS ((SARADC_Type *)(uintptr_t)SARADC_BASE)

/** @brief RTC 域 SARADC 寄存器实例。 RTC-domain SARADC register instance. */
#define RTC_SARADC_REGS ((SARADC_Type *)(uintptr_t)RTC_SARADC_BASE)

/**
 * @brief SARADC 使用的 RTC 域复位和时钟选源寄存器。
 *        RTC-domain reset and clock-selector registers used by SARADC.
 *
 * @see SG2002 技术参考手册 v1.02, 表 6.3。SG2002 TRM v1.02, Table 6.3
 */
typedef struct
{
    uint32_t
        RESERVED_000[6]; ///< 偏移 0x000：未公开区域的占位空间。Offset 0x000: padding for an unexposed area.
    volatile uint32_t RESET; ///< 偏移 0x018：RTC 域软复位控制。Offset 0x018: RTC-domain soft-reset control.
    volatile uint32_t CLOCK_MUX; ///< 偏移 0x01C：RTC 域时钟选择。Offset 0x01C: RTC-domain clock selection.
} RTC_CTRL_Type;

static_assert(offsetof(RTC_CTRL_Type, RESET) == 0x018U);
static_assert(offsetof(RTC_CTRL_Type, CLOCK_MUX) == 0x01CU);

/**
 * @brief RTC_CTRL 寄存器实例指针。
 *        RTC_CTRL register-instance pointer.
 */
#define RTC_CTRL_REGS ((RTC_CTRL_Type *)(uintptr_t)RTC_CTRL_BASE)

/// @brief RTC SARADC 低有效复位释放位。
///        RTC SARADC active-low reset-release bit.
static constexpr auto RTC_CTRL_SARADC_RESETN_BIT = (1UL << 17);
/// @brief RTC SARADC 分频振荡器选择位；清零选择 XTAL。
///        RTC SARADC divided-oscillator selector; clear to select XTAL.
static constexpr auto RTC_CTRL_SARADC_OSC_DIV_BIT = (1UL << 20);

/** @} */

/**
 * @defgroup SG2002_WDT 看门狗寄存器 / Watchdog registers
 * @ingroup SG2002_DEVICE
 * @see SG2002 技术参考手册 v1.02, Chapter 13。SG2002 TRM v1.02, Chapter 13.
 * @{
 */

/**
 * @brief DesignWare 看门狗控制与计数寄存器。
 *        DesignWare watchdog control and counter registers.
 */
typedef struct
{
    volatile uint32_t CR;         ///< 偏移 0x000：看门狗控制寄存器。Offset 0x000: watchdog control register.
    volatile uint32_t TORR;       ///< 偏移 0x004：超时范围配置。Offset 0x004: timeout-range configuration.
    volatile const uint32_t CCVR; ///< 偏移 0x008：当前计数器值。Offset 0x008: current counter value.
    volatile uint32_t CRR;        ///< 偏移 0x00C：计数器重启寄存器。Offset 0x00C: counter-restart register.
    volatile const uint32_t STAT; ///< 偏移 0x010：中断状态。Offset 0x010: interrupt status.
    volatile const uint32_t
        EOI; ///< 偏移 0x014：读取以清除中断的寄存器。Offset 0x014: read-to-clear interrupt register.
    uint32_t RESERVED_018; ///< 偏移 0x018：未公开区域的占位空间。Offset 0x018: padding for an unexposed area.
    volatile uint32_t TOC; ///< 偏移 0x01C：超时计数配置。Offset 0x01C: timeout-count configuration.
} WDT_Type;

static_assert(offsetof(WDT_Type, CR) == 0x000U);
static_assert(offsetof(WDT_Type, TORR) == 0x004U);
static_assert(offsetof(WDT_Type, CRR) == 0x00CU);
static_assert(offsetof(WDT_Type, TOC) == 0x01CU);

/** @brief WDT0 寄存器实例。 WDT0 register instance. */
#define WDT0_REGS ((WDT_Type *)(uintptr_t)WDT0_BASE)
/** @brief WDT1 寄存器实例。 WDT1 register instance. */
#define WDT1_REGS ((WDT_Type *)(uintptr_t)WDT1_BASE)
/** @brief WDT2 寄存器实例。 WDT2 register instance. */
#define WDT2_REGS ((WDT_Type *)(uintptr_t)WDT2_BASE)

/// @brief 看门狗使能位，置位后仅系统复位可清除。
///        Watchdog enable bit; only a system reset can clear it once set.
static constexpr auto WDT_CR_ENABLE_BIT = (1UL << 0);
/// @brief 看门狗先中断后复位响应选择位。
///        Watchdog interrupt-then-reset response selector.
static constexpr auto WDT_CR_INTERRUPT_RESPONSE_BIT = (1UL << 1);
/// @brief 看门狗超时范围编码上限。
///        Maximum watchdog timeout-range encoding.
static constexpr auto WDT_TOP_MAX = 15U;
/// @brief 看门狗初始超时范围字段起始位。
///        Watchdog initial timeout-range field shift.
static constexpr auto WDT_TOP_INITIAL_SHIFT = 4U;
/// @brief 指数计数模式的超时指数基数。
///        Base timeout exponent in exponential counter mode.
static constexpr auto WDT_TIMEOUT_EXPONENT_MIN = 16U;
/// @brief 重载看门狗并清除中断的写入密钥。
///        Write key that reloads the watchdog and clears its interrupt.
static constexpr auto WDT_RESTART_KEY = 0x76U;

/** @} */

/**
 * @defgroup SG2002_IRQ C906L PLIC 中断源编号 / C906L PLIC source numbers
 * @ingroup SG2002_DEVICE
 * @see Sophgo SDK hal/cv181x/config/intr_conf.h, CPU2 routing（寄存器定义来源 / register-definition
 * source）。
 * @{
 */

/// @brief SDMA 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for SDMA.
static constexpr auto IRQ_SDMA = 25UL;
/// @brief I2S0 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for I2S0.
static constexpr auto IRQ_I2S0 = 26UL;
/// @brief I2S1 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for I2S1.
static constexpr auto IRQ_I2S1 = 27UL;
/// @brief I2S2 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for I2S2.
static constexpr auto IRQ_I2S2 = 28UL;
/// @brief I2S3 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for I2S3.
static constexpr auto IRQ_I2S3 = 29UL;
/// @brief UART0 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for UART0.
static constexpr auto IRQ_UART0 = 30UL;
/// @brief UART1 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for UART1.
static constexpr auto IRQ_UART1 = 31UL;
/// @brief I2C0 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for I2C0.
static constexpr auto IRQ_I2C0 = 32UL;
/// @brief I2C1 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for I2C1.
static constexpr auto IRQ_I2C1 = 33UL;
/// @brief I2C2 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for I2C2.
static constexpr auto IRQ_I2C2 = 34UL;
/// @brief I2C3 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for I2C3.
static constexpr auto IRQ_I2C3 = 35UL;
/// @brief I2C4 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for I2C4.
static constexpr auto IRQ_I2C4 = 36UL;
/// @brief SPI0 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for SPI0.
static constexpr auto IRQ_SPI0 = 37UL;
/// @brief SPI1 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for SPI1.
static constexpr auto IRQ_SPI1 = 38UL;
/// @brief WDT2 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for WDT2.
static constexpr auto IRQ_WDT2 = 39UL;
/// @brief KEYSCAN 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for KEYSCAN.
static constexpr auto IRQ_KEYSCAN = 40UL;
/// @brief GPIO0 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for GPIO0.
static constexpr auto IRQ_GPIO0 = 41UL;
/// @brief GPIO1 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for GPIO1.
static constexpr auto IRQ_GPIO1 = 42UL;
/// @brief GPIO2 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for GPIO2.
static constexpr auto IRQ_GPIO2 = 43UL;
/// @brief GPIO3 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for GPIO3.
static constexpr auto IRQ_GPIO3 = 44UL;
/// @brief WGN0 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for WGN0.
static constexpr auto IRQ_WGN0 = 45UL;
/// @brief MBOX1 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for MBOX1.
static constexpr auto IRQ_MBOX1 = 46UL;
/// @brief IRRX 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for IRRX.
static constexpr auto IRQ_IRRX = 47UL;
/// @brief RTC_GPIO 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for RTC_GPIO.
static constexpr auto IRQ_RTC_GPIO = 48UL;
/// @brief RTC_UART 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for RTC_UART.
static constexpr auto IRQ_RTC_UART = 49UL;
/// @brief RTC_I2C 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for RTC_I2C.
static constexpr auto IRQ_RTC_I2C = 50UL;
/// @brief RTC_WDT 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for RTC_WDT.
static constexpr auto IRQ_RTC_WDT = 51UL;
/// @brief TDMA 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for TDMA.
static constexpr auto IRQ_TDMA = 52UL;
/// @brief SW0_CPU2 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for SW0_CPU2.
static constexpr auto IRQ_SW0_CPU2 = 53UL;
/// @brief SW1_CPU2 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for SW1_CPU2.
static constexpr auto IRQ_SW1_CPU2 = 54UL;
/// @brief TIMER4 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for TIMER4.
static constexpr auto IRQ_TIMER4 = 55UL;
/// @brief TIMER5 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for TIMER5.
static constexpr auto IRQ_TIMER5 = 56UL;
/// @brief TIMER6 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for TIMER6.
static constexpr auto IRQ_TIMER6 = 57UL;
/// @brief TIMER7 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for TIMER7.
static constexpr auto IRQ_TIMER7 = 58UL;
/// @brief SPACC 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for SPACC.
static constexpr auto IRQ_SPACC = 59UL;
/// @brief IVE 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for IVE.
static constexpr auto IRQ_IVE = 60UL;
/// @brief MBOX_C906_2ND 的 C906L PLIC 中断源编号。
///        C906L PLIC interrupt-source ID for MBOX_C906_2ND.
static constexpr auto IRQ_MBOX_C906_2ND = 61UL;

/** @} */
