# sgll — SG200x 无状态外设库规划

SG200X 无状态外设库，类似 STM32 LL、nrfx。基于 C23。依赖外部 core_rv64.h。

本规划基于《SG2002 Technical Reference Manual v1.02》（2025-02-27，841 页，英文版）
通读后制定。全文提取文本见 BSP 的 `docs/reference/sg2002_trm_en_v1.02.txt`（pymupdf 提取）。

---

## 1. 定位与原则

sgll 是 SG200x（SG2002，C906L bare-metal 视角）的**寄存器契约层**：

- **无状态**：没有句柄、没有对象、没有回调注册。寄存器原语为
  `static inline`；复合操作由调用者提供参数，在独立 C 源文件中实现。
- **头文件与源文件分层**：参考 STM32 LL / nrfx，`inc/` 保留寄存器访问、
  位域编码和 CPU 内建操作；`src/` 实现描述符构造、成组配置、等待循环、
  缓存范围维护和共享查表。CMake 生成静态库 `libsgll.a`。
- **纯 C、明确依赖**：标准头使用 `<stdint.h>`、`<stdbool.h>`、`<stddef.h>`。
  不依赖 LibXR 或 FreeRTOS；RISC-V 的 CSR 层依赖 SDK `core_rv64.h`，
  通过构建配置提供头文件路径。宿主构建由 `__riscv` 守卫排除该依赖。
- **与 `driver/` 的关系**：全部生产驱动已经通过 sgll 操作硬件。`driver/`
  保留 LibXR 状态、资源所有权、回调、同步和时钟规划；寄存器操作、硬件等待
  与初始化序列交给 sgll。器件地址、字段和复位坐标只在 `sg2002.h` 定义。
- **忠实 TRM、不猜硬件**：TRM 没写的字段/引脚复用表，不发明。与 `driver/`
  同样的纪律（例如 pad bias 未文档化就返回 NOT_SUPPORT，这里则是不定义）。

## 2. TRM v1.02 阅读结论

### 2.1 外设地址映射（TRM Table 3.4，sgll 基址宏的来源）

| 模块 | 基址 | 备注 |
|---|---|---|
| TOP_MISC | 0x03000000 | 含 DMA 中断路由（如 0x03000298 SDMA CPU2 mux） |
| PINMUX | 0x03001000 | ⚠️ TRM 第 10 章是空壳，功能表不在本文档 |
| CLKGEN/PLL | 0x03002000 | clk_en_0..4 / clk_sel_0 / clk_byp_0,1 / div_clk_*（Table 8.51） |
| RSTGEN | 0x03003000 | SOFT_RSTN_0..3、SOFT_CPUAC_RSTN、SOFT_CPU_RSTN，低有效 |
| WDT0/1/2 | 0x03010000/11000/12000 | DesignWare WDT；RTCSYS_WDT 0x0502D000 |
| GPIO0..3 | 0x03020000..0x03023000 | DesignWare GPIO 单 PortA；RTCSYS_GPIO 0x05021000 |
| PWM0..3 | 0x03060000..0x03063000 | 每控制器 4 通道，共 PWM0..15 |
| TIMER | 0x030A0000 | 8× DesignWare APB down-timer；RTCSYS_Timer 0x05020000 |
| TEMPSEN | 0x030E0000 | |
| SARADC | 0x030F0000 | 3ch；RTCSYS_SARADC 0x0502C000（PWR_ADC1..3） |
| I2C0..4 | 0x04000000..0x04040000 | DesignWare I2C；RTCSYS_I2C 0x0502B000 |
| UART0..4 | 0x04140000..0x041C0000(UART4) | DesignWare 8250；RTCSYS_UART 0x05022000 |
| SPI0..3 | 0x04180000..0x041B0000 | DesignWare SSI |
| eMMC/SD0/SD1 | 0x04300000/04310000/04320000 | |
| DMA | 0x04330000 | DesignWare 8ch AXI DMA（LLI 链表） |
| USB | 0x04340000 | |
| I2S0..3 | 0x04100000..0x04130000 | 含 I2S Global 0x04108000 |

### 2.2 寄存器描述约定（决定 sgll 宏命名）

TRM 每个寄存器都是 `Bits / Name / Access / Description / Reset` 五列表，
字段名保留 vendor 前缀：时钟 `reg_*`、SARADC `reg_/sta_*`。sgll 宏采用
STM32 LL 命名（`LL_XXX_YYY`），但在每个宏后注释保留 TRM 原始字段名，方便回查。

时钟块结构是 `enable / bypass / mux(sel) / divider` 四类寄存器，正好对应
`sg200x_clock_tree.hpp` 已建模的 gate/bypass/mux/divider。复位块低有效、
不自清零（置 0 触发、置 1 释放），复位前须确认该模块 DMA 已 idle（TRM 7.3）。

### 2.3 TRM 的三个缺口（规划必须显式应对）

1. **PINMUX 功能表不在 TRM 里**（第 10 章只有标题页）。引脚复用知识必须来自
   vendor SDK（`cv181x` pinlist）、设备树或 `driver/` 已验证的条目
   （如 `GPIOA_14` @ 0x38、`PWM0_BUCK` @ 0xEC）。sgll 只提供读写 PINMUX
   寄存器的原语，引脚功能枚举单独建表并注明来源。
2. **PLIC 中断号不在 TRM 里**。中断号沿用 SDK/DT 值并在 sgll 头文件里集中
   定义、注明出处（GPIO0..3=41..44、WDT2=39、DMA 通道经 TOP mux 0x03000298
   到 PLIC 25 —— 这些已被 `driver/` 在板上验证）。
3. **没有独立 DAC 外设**（只有音频 Codec 内置 DAC，属第 20 章音频子系统）。
   空文件 `sg200x_ll_dac.h` 已删除（2026-09-05）；同理**没有独立 EXTI 块**，
   GPIO 中断在 DesignWare GPIO 块内、跨核路由在 TOP_MISC——后者已归
   `sg200x_ll_dmamux.h`，`sg200x_ll_exti.h` 同日删除。

## 3. 架构与文件组织

```
sgll/
├── PLAN.md                    # 本文件
├── CMakeLists.txt             # STATIC 库，导出头文件路径及 C23 要求
├── sgll.h                     # 唯一公共入口，按依赖序包含 SG2002 和全部 LL 头
├── src/
│   ├── sg200x_ll_adc.c         # SARADC 初始化、单次触发和有界等待
│   ├── sg200x_ll_csr.c         # C906 缓存范围维护和 NOP 延迟循环
│   ├── sg200x_ll_dma.c         # 初始化、LLI 构造、通道清理和有界等待
│   ├── sg200x_ll_i2c.c         # I2C 时序计算、初始化、反初始化和有界等待
│   ├── sg200x_ll_mbox.c        # 共享 DDR 邮箱快照、缓存同步和状态交接
│   ├── sg200x_ll_pwm.c         # PWM 初始化、周期配置、更新和启动／停止
│   ├── sg200x_ll_rcc.c         # 复位位置表、时钟选择及 RTC SARADC 准备
│   ├── sg200x_ll_spi.c         # SPI 默认配置、初始化和反初始化
│   ├── sg200x_ll_uart.c        # UART 默认配置、初始化和反初始化
│   └── sg200x_ll_wdt.c         # 看门狗初始化、启动和复位路由
└── inc/
    ├── sg2002.h               # SG2002 设备头：基址、寄存器块、字段、实例和 IRQ
    ├── sg200x_ll_csr.h        # mie/mstatus/fence/rdtime 内联 asm（§8.1，P0）
    ├── sg200x_ll_utils.h      # 纯数值位操作与字段工具
    # sg2002.h 同时集中承载全部实例基址、寄存器布局、字段和 PLIC 中断号
    ├── sg200x_ll_plic.h       # PLIC 原语，0x70000000（§8.1，P0）
    ├── sg200x_ll_rcc.h        # CLKGEN/PLL + RSTGEN（使能/旁路/选源/分频/复位）
    ├── sg200x_ll_gpio.h       # GPIO0..3 + RTCSYS_GPIO
    ├── sg200x_ll_pinmux.h     # PINMUX 寄存器原语 + 已验证引脚功能表
    ├── sg200x_ll_uart.h       # UART0..4 + RTCSYS_UART
    ├── sg200x_ll_spi.h        # SPI0..3
    ├── sg200x_ll_i2c.h        # I2C0..4 + RTCSYS_I2C
    ├── sg200x_ll_tim.h        # 8× system timer + RTCSYS_Timer
    ├── sg200x_ll_pwm.h        # PWM0..15
    ├── sg200x_ll_dma.h        # DW AXI DMA 通道/LLI
    ├── sg200x_ll_dmamux.h     # 外设请求映射 + TOP 中断路由（0x03000298 等）
    ├── sg200x_ll_adc.h        # SARADC（含 RTCSYS_SARADC）
    ├── sg200x_ll_wdt.h        # DesignWare WDT（新增；无 wwdg 前身，初版规划笔误）
    ├── sg200x_ll_tempsen.h    # TEMPSEN 0x030E0000
    ├── sg200x_ll_efuse.h      # (可选，§8.2) EFUSE 0x03050000
    └── sg200x_ll_mbox.h       # 共享 DDR 邮箱；APB/RTC 硬件通道待按需实现
    # 已删除：sg200x_ll_dac.h、sg200x_ll_exti.h（2026-09-05，理由见 §2.3）
    # 未实现的外设头暂保留骨架；仅为已有复合实现建立 src 文件

### 3.0 头文件依赖序（单向，无循环 include）

与 STM32 LL"LL 头反向 include 器件头"的循环结构不同，sgll 做成**单向
依赖、每个头都可独立包含**（更接近 nrfx 的模块化）：

```
sgll.h                    ← umbrella，包含 SG2002 设备头和全部 LL 头
 ├─ inc/sg2002.h          ← 唯一芯片设备头
 ├─ sg200x_ll_csr.h      ← RISC-V 构建依赖 CSI core_rv64.h
 ├─ sg200x_ll_utils.h    ← 零依赖
 ├─ sg200x_ll_plic.h     ← csr + utils + sg2002
 ├─ sg200x_ll_rcc.h      ← csr + utils + sg2002
 └─ 各外设 sg200x_ll_*.h ← utils + sg2002（外设之间互不依赖）
```

规则：每个 `sg200x_ll_*.h` 只允许包含 `<stdint.h>/<stdbool.h>/<stddef.h>`
和依赖图里更底层的 sgll 头；`sgll.h` 按上图顺序一次性包含全部，业务代码
只需要 `#include "sgll.h"`。芯片寄存器和字段定义归 `sg2002.h`，LL 头只提供
操作语义。

### 3.0.1 sg2002.h 的角色

当前只支持 SG2002，因此不引入无实际用途的 family/chip 选择宏。后续新增
芯片时，应增加独立设备头和明确的库入口，而不是把型号选择隐藏在公共头中。
```

### 3.1 两层结构（每模块头文件内部）

1. **器件层**：`sg2002.h` 定义寄存器布局、实例地址、位域、枚举和类型，
   用 `static_assert` 验证布局；不在器件头中分配查表存储。
2. **内联原语**：各 LL 头保留寄存器读写、位域编码、短小校验、CSR 和屏障。
3. **独立实现**：缓存范围循环、DMA 描述符构造／配置／等待及 RCC 复位
   查表放入对应 `.c` 文件；头文件只保留带 C linkage 的声明。
   保持现有函数名及调用参数，C++ 调用者与 C23 库共用同一组外部符号。

参考 STM32 LL：复位原语声明在 `Inc/stm32g4xx_ll_bus.h`，各外设的
`DeInit()` 在对应 `.c` 中直接调用 ForceReset / ReleaseReset。
SGLL 当前由 `inc/sg200x_ll_rcc.h` 提供对应复位原语，SPI/I2C/UART 的
`DeInit()` 直接组合这些操作并保留 C906 I/O 屏障。

```c
/* —— 寄存器映射层（sg200x_ll_gpio.h）—— */
typedef struct {
  volatile uint32_t SWPORTA_DR;        /* 0x000 TRM Table 21.142 */
  volatile uint32_t SWPORTA_DDR;       /* 0x004 */
  uint32_t _reserved0[0x030u / 4u - 2u];
  volatile uint32_t INTEN;             /* 0x030 */
  volatile uint32_t INTMASK;           /* 0x034 */
  volatile uint32_t INTTYPE_LEVEL;     /* 0x038 */
  volatile uint32_t INT_POLARITY;      /* 0x03c */
  volatile uint32_t INTSTATUS;         /* 0x040 */
  volatile uint32_t RAW_INTSTATUS;     /* 0x044 */
  volatile uint32_t DEBOUNCE;          /* 0x048 */
  volatile uint32_t PORTA_EOI;         /* 0x04c */
  volatile uint32_t EXT_PORTA;         /* 0x050 */
} GPIO_Type;

#define GPIO0 ((GPIO_Type *) 0x03020000u)
#define GPIO1 ((GPIO_Type *) 0x03021000u)
#define GPIO2 ((GPIO_Type *) 0x03022000u)
#define GPIO3 ((GPIO_Type *) 0x03023000u)
#define RTCSYS_GPIO ((GPIO_Type *) 0x05021000u)

/* —— LL API 层，命名照抄 STM32 LL 习惯 —— */
#define LL_GPIO_PIN_0            (1u << 0)  /* ... PIN_31 */
#define LL_GPIO_MODE_INPUT       (0u)
#define LL_GPIO_MODE_OUTPUT      (1u)

static inline void sgll_gpio_set_output_pin(GPIO_Type *port, uint32_t pin)
{
  port->SWPORTA_DR |= pin;
}
```

### 3.2 命名规范（对照 STM32 LL）

| 元素 | 约定 | 示例 |
|---|---|---|
| 函数 | **全小写蛇形**（nrfx 风格，2026-09-05 用户拍板；弃用最初设想的 STM32 LL 驼峰），动词体系仍是 Set/Get/Enable/Disable/IsXxx | `sgll_rcc_gate_enable()`、`sgll_plic_irq_claim()` |
| 设备常量（基址/偏移/位/字段） | 放在 `sg2002.h`，使用 `static constexpr auto` 和外设语义前缀 | `CLKGEN_DIV_OFF_SPI`、`CLKGEN_DIV_SPI_FACTOR_WIDTH`、`SPI_DR_OFFSET` |
| LL 宏（库级工具） | 大写蛇形，`SGLL_` 前缀 | `SGLL_BIT` |
| 枚举值 | 大写蛇形，按所属 API 命名 | `SGLL_SPI_MODE_1`、`RESET_WDT2` |
| 类型 | 小写蛇形 + `_t` | `rstgen_reset_target_t` |
| 实例 | 设备语义实例宏 | `GPIO0`、`SPI2`、`DMA` |
| `LL_mDelay()` | 不提供 | 无状态库不管时间源；timebase 仍属 `driver/` |
| 初始化配置类型 | 提供无状态 `sgll_*_init_t` | 只承载调用参数，配套 `struct_init/init/deinit`，不保留配置指针、句柄或回调 |

每个寄存器/字段宏注释里保留 TRM 表号与原始字段名（`reg_saradc_en` 等）。

### 3.3 寄存器访问与屏障

- 统一 32 位访问。C906 是 RV64，对 APB/AXI 外设只用 `volatile uint32_t`
  读写，不做 8/16 位访问（DesignWare 块在跨 AXI6→APB 桥上，字节访问语义
  未被 TRM 保证；`driver/` 也全程 32 位）。
- 所有寄存器布局、实例、偏移和位定义集中到 `sg2002.h`。LL 直接读写
  设备结构成员；PINMUX、PLIC、CLKGEN/PLL 与其他外设使用同一约定。
  `sg200x_ll_utils.h` 仅保留纯数值位操作，移除通用 MMIO 访问函数及宏。
- I/O 屏障统一为 `sgll_csr_fence_io()`（`fence iorw, iorw`，带编译器
  memory clobber）；在配置→触发和跨核 TOP 路由更新路径显式调用。
- **CSR/缓存层依赖 SDK `core_rv64.h`（2026-09-05 用户批准，修正此前的
  "零依赖"决定）**：`sg200x_ll_csr.h` 是 CSI（`core_rv64.h` →
  `csi_rv64_gcc.h`）之上的薄适配——`__get/__set_MSTATUS`、
  `__enable/__disable_irq`、`__get/__set_MIE`；I/O 屏障和指令屏障直接发出
  `fence.i`，不使用 SDK 中只发出数据屏障的 `__ISB`。
  自留汇编：`rdtime`（CSI 的 timer 函数被 `#if 0` 编译掉了）和 **D-cache
  维护**——固件工具链的汇编器不认 `dcache.*` 助记符，CSI 的
  `__DCACHE_IPA/CPA/CIPA` 无法实例化；sgll 逐字采用 SDK
  `arch/riscv64/src/cache.c`（driver DMA 路径在板上验证的同一实现）的
  裸指令字编码（`dcache.ipa/cpa/cipa a0` = `.long 0x02a5/0x0295/0x02b5
  000b`，收尾 `sync.s`），64 字节行循环，函数签名与 SDK 的
  `inv/clean/flush_dcache_range` 对齐。循环及私有指令宏存放在
  `src/sg200x_ll_csr.c`，显式约束汇编输入寄存器并保留 memory clobber。
  头文件声明与实现均保留 `__riscv` 守卫：
  RISC-V 构建需加 SDK `install/include/arch` 路径，宿主侧工具链不受影响。

### 3.4 C23 语言要求

sgll 统一要求 C23（`-std=c23` 或等效选项），并在 C、C++ 两种消费模式中
使用同一个 `static_assert` 关键字。设备头的布局断言紧邻对应寄存器块，
使用单参数形式，由编译器报告失败表达式。C++ 消费者要求 C++17 及以上，
当前回归使用 C++20；C 消费者必须启用 C23。

设备数值常量使用 `static constexpr auto`，保留原有字面量后缀所表达的
整数类型；地址按四位十六进制分组，例如 `0x0300'2000UL`，长十进制值
按三位分组，例如 `25'000'000UL`。这些常量可参与数组长度、枚举初始化
和静态断言，不能用于预处理器条件。实例地址转换、功能开关、复位／寄存器
展开列表和时钟坐标元组仍保留宏形式。

## 4. 模块清单与实施阶段

按"能点亮→能通信→能定时→能搬运→能测量"排序；每阶段都以板上 smoke test
收尾（走现有 `scripts/deploy_rtos_fw.sh`）。

| 阶段 | 模块 | TRM 依据 | 板上验证 |
|---|---|---|---|
| P0 | csr → utils → sg2002 → plic → rcc → sgll.h | Ch7/8 + clock_tree.hpp + PLIC 规范/DTS | PLIC 中断进出冒烟；复用 `driver/` 的时钟契约自检 |

> **P0 状态（2026-09-05）：代码已实现；已改用 nrfx 风格全小写命名；静态验证（含 xPack 固件工具链 -Werror 全探针）与一致性审计已通过；未上板。**
> 7 个头（sgll.h、sg2002.h、csr/utils/plic/rcc）+ INTERFACE CMake +
> overlay 接入。验证：宿主 gcc（gnu11/gnu2x，无 SDK 头）与 riscv64 交叉 gcc
> （带 SDK arch 头）均零告警通过；反汇编确认 CSR/fence/rdtime 生成；CMake
> 冒烟通过。
>
> **一致性审计（`sgll/tools/audit_vs_trm.py`，从 TRM 提取文本与 SDK 头
> 程序化比对）**：门控位 66/66、bypass 位 5/5、复位位 46/46、div 偏移
> 15/15、分频宽度与初始值 6/6（对照 driver clock tree）、基址 70/70、
> C906L 中断号 37/37，exit 0。审计中修复的库差异：补 WGN0/1/2 复位项
> （rstn2[22-24]）；补 AXI4/AXI6 初始分频值宏与因子宽度来源说明（TRM 只写
> [20:16]，宽度以 driver 已验证值为准：axi4/axi6/i2c=4 位、
> 1m/spi/pwm_src=6 位）。
> 待做：板上 PLIC 进出/时钟分支冒烟（走 firmware/deploy 流水线）。
| P1 | pinmux 原语 + gpio | Ch21.5 | LL_GPIO 点 LED（GPIOA_14）+ 按键输入 |
| P2 | uart | Ch21.2 | 轮询/中断收发打印（板已用 UART0/1 出日志） |
| P3 | tim | Ch12 | 1ms 中断翻转 LED；读CurrentValue核对 |
| P4 | pwm | Ch21.9 | PWM8 输出，示波器看占空比（driver 已验流程） |
| P5 | spi / i2c | Ch21.3/21.1 | SPI 回环；I2C 扫描板载器件 |

> **P5 SPI 部分状态（2026-09-06）：已实现并接入。** `sg200x_ll_spi.h` 固化
> SG2002 DesignWare SSI 的寄存器布局、CTRLR0 四种 Motorola mode、BAUDR/FIFO、
> SER、状态/清除、中断、DMA、DR 和 RX sample delay 语义；`driver/sg200x_spi.*`
> 已改用这些原语。SPI2 + ESP32-C5 D7 的板上验证使用 `CTRLR0=0x47`
> （mode 1 / `SCPH=1`），收到 `4F 4B 01 00` ACK。I2C 仍待 P5 的后续实现。
| P6 | dma + dmamux | Ch11 | mem-to-mem；SPI RX circular |
| P7 | adc + tempsen + wdt | Ch21.7/21.8/Ch13 | ADC 读电压；WDT 喂狗/复位 |

每模块交付清单：寄存器结构体 + 基址宏 + LL API + 字段宏（含复位值注释）
+ `static_assert` 偏移自检 + 对应 smoke test。

## 5. 构建集成

- `sgll/CMakeLists.txt`：`add_library(sgll STATIC ...)`，编译 CSR、DMA、RCC
  以及 SPI、I2C、UART、MBOX 的 C23 源文件，公开根目录和 `inc/` 头文件路径。`SGLL_CORE_INCLUDE_DIR`
  可指定 CSI core 头目录，也可由父项目通过 `target_include_directories` 提供。
- 接入点：SDK overlay 的
  `sdk/sg200x/overlay/freertos/cvitek/task/xrobot/CMakeLists.txt` 里
  `add_subdirectory("${SG200X_BSP_ROOT}/sgll" sgll)`，并
  `target_link_libraries(xrobot PUBLIC sgll)`；同时将 SDK `install/include/arch`
  设为 sgll 的 PUBLIC SYSTEM include，使库和调用者使用相同的 core 头。
- 编译告警基线：`-Wall -Wextra` 下零告警；与 `-fno-exceptions -fno-rtti`
  天然兼容（纯 C）。

## 6. 验证策略

1. **静态**：`static_assert(offsetof(UART_Type, USR) == 0x07c)`
   式偏移锁定；基址宏与 TRM Table 3.4 逐一对照（可写一个宿主端脚本从
   `docs/reference/sg2002_trm_en_v1.02.txt` 抽表自动复核）。
2. **交叉源**：每模块实现前，对照 `driver/` 同外设头文件和 vendor SDK
   （duo-buildroot-sdk cv181x hal）寄存器定义；三方一致才落笔。
3. **板上**：每阶段 smoke test 直接写进 `User/main.cpp`（或临时 task），
   经现有 firmware/deploy 流水线跑真机，结果人工判定。
4. **回归**：smoke test 保留为可重复入口（脚本化），后续改 sgll 可一键复测。

## 7. 风险与开放问题

- **PINMUX 功能表无权威来源**：只能逐步从 SDK/DT/实测积累，sgll 的引脚
  功能表宁缺勿滥，每条注明出处；未验证的 pad bias 字段一律不碰。
- **与 Linux 共享外设**：DMA 通道 0-3 归 Linux、4-7 归 C906L 的所有权约定
  必须在 sgll 的 dmamux 头里以注释/宏形式固化，防止绕过 `driver/` 直接用
  sgll 时误抢通道。
- **RTCSYS 域寄存器**（0x0502xxxx）访问路径与 Active 域可能不同，P1 只做
  宏定义，行为验证放到单独阶段。
- **PLL 配置**：sgll 只读 PLL CSR、不写（与 `SG200XRCC` 同一契约），改 PLL
  属于系统级操作，不进无状态库。
- **TRM 版本演进**：所有头文件头部标注 "TRM v1.02" 与表号，换版本时 diff 有据可查。

## 8. 补充模块评估（第二轮）

判据：LL 层只做**裸机寄存器原语**。凡是需要协议栈、状态机或成片运行时状态
的模块（USB、以太网、SD 协议、音视频）都不是 LL 层的事，归 `driver/` 或上层。

### 8.1 建议补充（csr/plic 已并入 P0）

空文件 `sg200x_ll_plic.h`、`sg200x_ll_csr.h` 已创建（2026-09-05），
规划职责如下；实施随 P0 进行：

| 模块 | 基址/依据 | 理由 |
|---|---|---|
| `sg200x_ll_plic.h` | 0x70000000（板级 DTS `interrupt-controller@70000000`；标准 RISC-V PLIC 规范；TRM 无寄存器描述，头文件须注明出处） | priority/enable/threshold/claim/complete 原语。目前中断使能依赖 SDK `request_irq()`，与 sgll"零依赖"定位冲突；有了它，GPIO/UART/TIMER 的 LL 中断路径才能脱离 SDK 独立运行 |
| `sg200x_ll_csr.h` | SDK `core_rv64.h` 薄适配 + 必要内联 asm | `mie/mstatus` 全局中断开关、`fence`、`rdtime` 和 C906L D-cache 原语。PLIC 原语与中断进出路径都依赖它。 |

这两项建议提前到 **P0**：它们是 P1~P7 所有中断型 smoke test 的地基，
比先写 GPIO 再回头补中断层返工少。

### 8.2 可选（按需，不排期）

`sg200x_ll_efuse.h` 保留骨架。`sg200x_ll_mbox.h/.c` 已实现共享 DDR 单槽传输，
由调用者定义头部、状态字和值；相机 ABI 和 CRC 校验在 `User/camera_mailbox.cpp`。
APB/RTC 硬件邮箱通道仍按实际需要单独实现：

| 模块 | 基址 | 说明 |
|---|---|---|
| `sg200x_ll_efuse.h` | 0x03050000 | 芯片信息；TRM 21.7.3 的 SARADC trim 校准流程依赖它 |
| `sg200x_ll_mbox.h` | 共享 DDR；硬件通道基址为 ap_mailbox 0x01900000、RTCSYS_MBOX 0x05024000 | 已接入 Linux/C906L 相机共享内存通信；硬件通知通道未启用 |
| `sg200x_ll_sdmmc.h` | 0x04300000/04310000/04320000 | 只做寄存器映射层；协议层已有 `driver/sg200x_sdio` 在开发 |
| `sg200x_ll_i2s.h` | 0x04100000..0x04130000 | 第 20 章音频；仅当出现音频场景 |

### 8.3 明确不做

USB（0x04340000）、ETH0（0x04070000）、DDR 控制器、SPI NOR/NAND 控制器、
8051 子系统、TPU/ISP/视频管线、CryptoDMA——这些的复杂度在协议与系统
初始化，寄存器原语不解决问题。TRNG（0x02070000）同样暂缓：当前固件没有
随机数需求，出现时再按 8.1 的格式补进排期。
