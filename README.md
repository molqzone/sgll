# sgll

SG2002 无状态底层库，使用 C23，支持 C++17 及以上调用（回归使用 C++20）。

按 STM32 LL / nrfx 的职责划分组织代码：

| 位置 | 内容 |
|---|---|
| `inc/sg2002.h` | 器件地址、寄存器布局与实例、位域、复位坐标和类型定义 |
| `inc/sg200x_ll_*.h` | 寄存器读写、位域编码、CSR／屏障等内联原语，以及外部函数声明 |
| `src/sg200x_ll_adc.c` | SARADC 默认配置、初始化、单次转换触发和有界等待 |
| `src/sg200x_ll_csr.c` | 缓存范围维护、NOP 延迟循环和私有指令编码 |
| `src/sg200x_ll_dma.c` | DMA 初始化／反初始化、描述符构造、通道配置和有界等待 |
| `src/sg200x_ll_spi.c` | SPI 默认配置、初始化、单实例反初始化和空闲等待 |
| `src/sg200x_ll_i2c.c` | I2C 默认配置、运行时时序计算、初始化／反初始化和使能等待 |
| `src/sg200x_ll_mbox.c` | 共享 DDR 邮箱的头部快照、缓存同步和状态交接 |
| `src/sg200x_ll_pwm.c` | PWM 默认配置、初始化、周期写入、动态更新和有序启动／停止 |
| `src/sg200x_ll_uart.c` | UART 默认配置、波特率／帧格式配置和单实例反初始化 |
| `src/sg200x_ll_rcc.c` | 根据器件定义生成的复位表、时钟寄存器选择和 RTC SARADC 时钟／复位准备 |
| `src/sg200x_ll_wdt.c` | 看门狗默认配置、初始化、启动和 TOP 复位路由 |

划分依据是操作职责。位域编码即使包含多个条件，仍保留内联以便编译器
折叠常量；包含循环、描述符初始化或共享数据的操作独立编译。
库不保存通道所有权、回调或 RTOS 状态。尚未实现的外设不建立空 `.c` 文件。

LL 直接访问 `sg2002.h` 声明的设备成员，例如 `SPI2_REGS->DR[0]`、
`PINMUX->FUNCTION[index]`、`PLIC_C906L->ENABLE[word]`。
CLKGEN 的时钟树偏移通过具名成员选择器解析；未定义或保留的偏移不进行访问。
`sg200x_ll_utils.h` 仅提供数值位运算，通用 MMIO 函数、寄存器读写宏和
C++ 引用适配器已移除。I/O 排序统一调用 `sgll_csr_fence_io()`。

BSP 的全部生产驱动已通过 SGLL 操作硬件。GPIO 数据、方向、中断和引脚复用
采用短内联接口；PWM、ADC、看门狗和 I2C 的复合操作放在对应 `.c` 中。
C++ 层保留资源所有权、LibXR 回调、锁、频率／电压换算和时钟树规划。
SGLL 不保存外设运行状态，不注册 RTOS 中断，也不替调用者分配 DMA 通道。

`sg2002.h` 的基址、偏移、位掩码、计数和 IRQ 使用 `static constexpr auto`
声明，类型由保留的 `U`／`UL`／`ULL` 字面量后缀推导；长数字使用分隔符，
例如 `0x0300'2000UL`、`25'000'000UL`。布局 `static_assert` 紧邻对应寄存器块。
实例指针、功能开关和展开列表／坐标元组仍使用宏。数值常量可用于数组长度、
枚举初始化和静态断言，但不能用于预处理器 `#if`／`#ifdef` 判断。

## 构建

```cmake
add_subdirectory(sgll)
target_link_libraries(firmware PRIVATE sgll)
```

`sgll` 目标生成静态库 `libsgll.a`，公开头文件路径和 C23 要求。RISC-V 构建
不需要任何 SDK 头文件：CSR、中断与时间原语由 `inc/sgll_core.h` 的内联汇编提供。
只使用内联寄存器原语时，静态链接器无需提取未引用的实现对象。

需要与厂商定义做对照时，可切换到 SDK 后端编译同一批原语：

```cmake
# A/B 对照构建：-DSGLL_CORE_BACKEND_SDK=ON
#                -DSGLL_CORE_INCLUDE_DIR=/path/to/sdk/arch/riscv64/include
```

两个后端提供同名原语与相同的寄存器语义，`sgll/tests/run.py` 会用 `--core-include`
编译 `tests/core_probe.c` 两侧并逐符号比较生成的指令。

使用复合操作的工程需链接 `sgll`。驱动中的旧通用 MMIO 调用改为对应外设 LL 接口，
旧屏障调用改用 `sgll_csr_fence_io()`。
头文件通过 `extern "C"` 保证 C++ 调用者使用 C 库的符号。

## 初始化接口

参考 [STM32 LL SPI](https://github.com/STMicroelectronics/stm32g4xx-hal-driver/blob/master/Src/stm32g4xx_ll_spi.c)
的 `StructInit` / `Init` / `DeInit` 划分，各模块按硬件能力提供
`sgll_*_struct_init()`、`sgll_*_init()`；SPI、I2C、UART 和 DMA 还提供
`sgll_*_deinit()`。看门狗使能后只能通过系统复位停止，不提供软件 DeInit。
配置结构体只在调用期间使用，不是句柄，库不保留其地址。

| 模块 | 配置与约束 |
|---|---|
| SPI0–3 | `sgll_spi_init_t`：模式、帧格式／位宽、BAUDR 分频、FIFO／DMA 阈值、采样延迟。Init 要求 SSI 已禁用，完成后保持禁用并关闭 CS、DMA 请求和中断。 |
| I2C0–4 | `sgll_i2c_init_t`：标准／快速主机模式、7/10 位目标地址、重启和阈值。Init 另接收实际输入时钟，使用 TRM 表 21.1 的 25/100 MHz 时序；ENABLE 与 ENABLE_STATUS 都必须为零。 |
| UART0–4 | `sgll_uart_init_t`：波特率、5–8 位数据、奇偶校验、停止位、FIFO 和 RTS/CTS。Init 另接收实际输入时钟，四舍五入计算分频；1.5 停止位限 5 位数据，2 停止位限 6–8 位数据。 |
| DMA | `sgll_dma_init_t`：地址、元素数、方向、宽度、模式、请求和优先级。Init 另接收调用者拥有的通道与 64 字节对齐描述符；循环模式的空 `next` 表示自链接。 |
| PWM0–3 | `sgll_pwm_init_t`：总周期、非有效电平计数和极性。Init 只配置一个已停止的通道，保留其他通道；更新使用置位再清零脉冲，启动按极性、START 清零、OE 置位、START 置位排序。 |
| SARADC／RTC SARADC | `sgll_adc_init_t`：时钟分频编码和参考电压。Init 要求空闲，默认十六分频及内部参考；`sgll_adc_start()` 使用域内 1–3 通道编号，等待与结果读取分开。 |
| WDT0–2 | `sgll_wdt_init_t`：超时指数和响应模式。Init 要求尚未使能；`sgll_wdt_start()` 按设置超时、喂狗、使能排序。TOP 路由和共享计数时钟由调用者显式准备。 |

I2C 的 `sgll_i2c_timing_calculate()` 支持实际输入时钟，计算标准／快速模式
计数并补偿 DesignWare 内部延迟；无法表示时返回 `false` 且保留输出。
`sgll_i2c_init_with_timing()` 接收显式时序。原 `sgll_i2c_init()` 继续使用
TRM 的 25/100 MHz 固定配置，两个入口的时序来源不同。

看门狗的 `sgll_rcc_watchdog_clock_select()` 整体替换共享 `WDT_CTRL[10:8]`，
调用者必须协调所有主域实例。器件头标注两处沿用 SDK 的兼容行为：
`cvitek_wdt` 使用的 `SYS_CTRL[8]` 使能，以及 `cv181x-resets.h` 的 SD1
复位位 17；TRM 对这两位的描述为保留位。

`Init` / `DeInit` 返回 `bool`。参数无效或外设仍处于要求之外的活动状态时
返回 `false`，不修改寄存器；DMA Init 在此情况下也不修改描述符。
UART 的输入端和 DMA 必须先由调用者静止，波特率可能因整数分频产生误差。

调用者负责外设所有权、时钟、引脚和 DMA 生命周期。Init 不注册 ISR、不分配
DMA 通道、不启动传输。SPI/I2C/UART 的 DeInit 要求外设和相关 DMA 已停止，
只脉冲对应复位位。DMA DeInit 只清理已停止的指定通道，不复位共享控制器，
也不修改其他通道、请求映射或 TOP 中断路由。

例如，在板级代码准备好 SPI2 的时钟、复位和引脚后：

```c
sgll_spi_init_t config;
sgll_spi_struct_init(&config);
config.mode = SGLL_SPI_MODE_1;
config.baud_divider = 8;
if (!sgll_spi_init(SPI2_REGS, &config)) {
    /* 检查参数及 SPIENR。 */
    return;
}
/* 配置传输所需的 DMA／中断通路后，再显式启用 SSI。 */
sgll_spi_enable(SPI2_REGS);
```

DMA Init 不维护缓存。配置完成后，按传输方向清理或失效数据缓冲区，清理
描述符缓存，再配置中断并调用 `sgll_dma_channel_enable()`。共享 DMA 的全局
使能由板级初始化负责。

## 共享内存邮箱

`sg200x_ll_mbox.h` 提供单生产者、单消费者、单消息槽的无状态传输接口。
`sgll_mbox_config_t` 描述共享区、头部大小、32 位状态字位置及 empty/ready 值，
不保存运行状态，不分配内存，也不重置对端正在使用的槽。

- 发送：`sgll_mbox_tx_acquire()` → 写头部和载荷 → `sgll_mbox_tx_publish()`。
  调用者保持状态字为 empty，库在头部和载荷全部写回后才发布 ready。
- 接收：`sgll_mbox_rx_acquire()` 获取本地头部快照 → 校验协议字段 →
  `sgll_mbox_rx_payload()` 同步载荷 → 处理完成后 `sgll_mbox_rx_release()`。
  接收端只修改状态字，并只回写该状态字所在的缓存行。

共享区基址、总大小和头部大小必须按 64 字节缓存行对齐；状态字按 4 字节
对齐并位于头部内。接口检查地址溢出、载荷边界和输出缓冲区，失败时不修改
输出或共享数据。调用者负责协议、CRC、单生产者／消费者约束及 DMA 完成等待。
重启任一端之前应先停止双方访问；未发布的发送内容仍由生产者持有。

业务固件可在应用层封装该接口，对接 Linux 相机桥等共享 DDR 协议，保留
既有头部和地址约定。APB/RTC 硬件邮箱的中断和寄存器
通道不属于这条共享内存协议。宿主测试使用一致性内存，C906L 构建执行实际
缓存维护和 I/O 屏障。

## 双语 API 文档 / Bilingual API documentation

全部库头文件和实现使用 LibXR 风格的中英对照 Doxygen 注释。`@brief`、
`@param`、`@return`、`@note` 和 `@pre` 先写中文，再给出对应英文；
枚举和结构成员使用 `///< 中文。English.`；常量说明放在声明前，保持初始化
表达式完整。预留接口会明确标注尚未实现。

All library headers and implementations use LibXR-style bilingual Doxygen comments.
Descriptions, parameters, return values, notes, and preconditions pair Chinese
with English. Enumerators and members use trailing `///<` comments; constant
descriptions precede declarations to keep initializer expressions intact.
Reserved interfaces explicitly state that operations are not implemented yet.

安装 Doxygen 1.15 或更新版本后，在 `sgll` 目录运行：
With Doxygen 1.15 or newer installed, run from the `sgll` directory:

```sh
mkdir -p build
doxygen Doxyfile
```

HTML 入口为 `build/doxygen/html/index.html`，XML 位于 `build/doxygen/xml/`。
配置检查缺失的接口、参数、枚举值文档和无效标签，任何告警都会使生成失败。
文档预处理选择 C906L/RV64 接口；实际编译仍由工程目标决定。

HTML starts at `build/doxygen/html/index.html`; XML is in `build/doxygen/xml/`.
Missing interface, parameter, or enumerator documentation and invalid tags fail
generation. Documentation preprocessing selects C906L/RV64 interfaces; actual
compilation remains controlled by the project target.

## 验证

见 [tests/README.md](tests/README.md)：包含头文件独立编译、C/C++ 链接、
寄存器与描述符检查，以及 QEMU 下的缓存指令参数验证。
