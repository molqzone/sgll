# sgll — SG200x 无状态外设库规划

SG200X 无状态外设库，类似 STM32 LL、nrfx。基于 C23。依赖外部 core_rv64.h。

本规划基于《SG2002 Technical Reference Manual v1.02》（2025-02-27，841 页，英文版）
通读后制定。全文提取文本见仓库根目录 `.tmp-sg2002-trm-en.txt`（pymupdf 提取）。

---

## 1. 定位与原则

sgll 是 SG200x（SG2002，C906L bare-metal 视角）的**寄存器契约层**：

- **无状态**：没有句柄、没有对象、没有回调注册，所有 API 都是读写寄存器的
  `static inline` 函数（STM32 LL 的 `LL_GPIO_SetOutputPin()` 风格）。
- **头文件 only**：不产出 .o/.a，只提供 include 路径。编译器跨翻译单元内联。
- **纯 C、零依赖**：只 include `<stdint.h>` `<stdbool.h>` `<stddef.h>`。不依赖
  LibXR、不依赖 vendor SDK、不依赖 FreeRTOS。`core_rv64.h` 仅用于 fence/CSR
  类操作，且必须是可选的（见 §3.4）。
- **与 `driver/` 的关系**：`driver/` 是已验证的 LibXR 有状态驱动层，保持不动。
  sgll 是更底一层；后续 `driver/` 可以逐步改为基于 sgll，但不是本次目标。
  sgll 的寄存器知识必须与 `driver/` 各头文件里的硬件契约一致（两处都注明
  TRM 表号，交叉校验）。
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
├── CMakeLists.txt             # INTERFACE 库，只导出 include 路径
└── inc/
    ├── sg200x.h               # 总入口（umbrella）：家族宏 + 按 §3.0 依赖序包含全部 LL 头
    ├── sg2002.h               # 芯片身份：SG200X_CHIP_* 宏 + 实例可用性（由 sg200x.h 包含）
    ├── sg200x_ll_csr.h        # mie/mstatus/fence/rdtime 内联 asm（§8.1，P0）
    ├── sg200x_ll_utils.h      # MMIO 原语、字段宏工具
    ├── sg200x_ll_defs.h       # 全部实例基址 + PLIC 中断号集中定义
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
    └── sg200x_ll_mbox.h       # (可选，§8.2) ap_mailbox/RTCSYS_MBOX 双核邮箱
    # 已删除：sg200x_ll_dac.h、sg200x_ll_exti.h（2026-09-05，理由见 §2.3）
    # 全部头文件为空骨架（2026-09-05 建齐），内容按 §4 阶段填充

### 3.0 头文件依赖序（单向，无循环 include）

与 STM32 LL"LL 头反向 include 器件头"的循环结构不同，sgll 做成**单向
依赖、每个头都可独立包含**（更接近 nrfx 的模块化）：

```
sg200x.h                 ← umbrella，P0 收尾时最后完成
 ├─ sg2002.h             ← 芯片身份，无依赖
 ├─ sg200x_ll_csr.h      ← 零依赖
 ├─ sg200x_ll_utils.h    ← 零依赖
 ├─ sg200x_ll_defs.h     ← 零依赖（基址/中断号/实例计数，引用 sg2002.h 的芯片宏）
 ├─ sg200x_ll_plic.h     ← csr + utils + defs
 ├─ sg200x_ll_rcc.h      ← utils + defs
 └─ 各外设 sg200x_ll_*.h ← utils + defs（外设之间互不依赖）
```

规则：每个 `sg200x_ll_*.h` 只允许包含 `<stdint.h>/<stdbool.h>/<stddef.h>`
和依赖图里更底层的 sgll 头；`sg200x.h` 按上图顺序一次性包含全部，业务代码
只需要 `#include "sg200x.h"`。因此 **sg200x.h 在 P0 内部最后写**：它的
内容（包含清单与顺序）就是 P0 各头依赖关系的固化，先行实现只能写出空壳。

### 3.0.1 sg2002.h 的角色

对标 STM32 的"家族头 + 具体型号头"两级：`sg200x.h` 是家族入口，
`sg2002.h` 定义芯片身份（`SG200X_CHIP_SG2002`）与该芯片的实例可用性
（如 SG2002 无 ETH1/少部分外设实例；实例计数宏供 defs 校验）。家族里若
将来支持 SG2005，新增 `sg2005.h` 并在 `sg200x.h` 顶部按芯片宏二选一。
```

### 3.1 两层结构（每模块头文件内部）

1. **寄存器映射层**：`typedef struct` 寄存器块 + 实例指针宏。
   结构体布局用 `_Static_assert(offsetof(...))` 锁死，防止填充错位。
2. **LL API 层**：`static inline` 的 `LL_XXX_*()` 函数 + `#define LL_XXX_YYY`
   位掩码枚举。

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
} SG200X_LL_GPIO_TypeDef;

#define GPIO0 ((SG200X_LL_GPIO_TypeDef *) 0x03020000u)
#define GPIO1 ((SG200X_LL_GPIO_TypeDef *) 0x03021000u)
#define GPIO2 ((SG200X_LL_GPIO_TypeDef *) 0x03022000u)
#define GPIO3 ((SG200X_LL_GPIO_TypeDef *) 0x03023000u)
#define RTCSYS_GPIO ((SG200X_LL_GPIO_TypeDef *) 0x05021000u)

/* —— LL API 层，命名照抄 STM32 LL 习惯 —— */
#define LL_GPIO_PIN_0            (1u << 0)  /* ... PIN_31 */
#define LL_GPIO_MODE_INPUT       (0u)
#define LL_GPIO_MODE_OUTPUT      (1u)

__STATIC_INLINE void LL_GPIO_SetOutputPin(SG200X_LL_GPIO_TypeDef *port, uint32_t pin)
{
  port->SWPORTA_DR |= pin;
}
```

### 3.2 命名规范（对照 STM32 LL）

| 元素 | 约定 | 示例 |
|---|---|---|
| 函数 | **全小写蛇形**（nrfx 风格，2026-09-05 用户拍板；弃用最初设想的 STM32 LL 驼峰），动词体系仍是 Set/Get/Enable/Disable/IsXxx | `sg200x_ll_rcc_gate_enable()`、`sg200x_ll_plic_irq_claim()` |
| 宏（寄存器/位/元组） | 大写蛇形，`SG200X_LL_` 前缀 | `SG200X_LL_RCC_GATE_APB_GPIO` |
| 枚举值 | 大写蛇形 | `SG200X_LL_RESET_WDT2` |
| 类型 | 小写蛇形 + `_t` | `sg200x_ll_rcc_reset_target_t` |
| 实例 | 沿用 SG200x 实例名的宏 | 后续 `SG200X_GPIO0`、`SG200X_UART0` |
| `LL_mDelay()` | 不提供 | 无状态库不管时间源；timebase 仍属 `driver/` |
| 类型 `InitTypeDef` | **不提供** | Init 结构体是有状态层的事，LL 只做原语 |

每个寄存器/字段宏注释里保留 TRM 表号与原始字段名（`reg_saradc_en` 等）。

### 3.3 MMIO 与屏障（sg200x_ll_utils.h）

- 统一 32 位访问。C906 是 RV64，对 APB/AXI 外设只用 `volatile uint32_t`
  读写，不做 8/16 位访问（DesignWare 块在跨 AXI6→APB 桥上，字节访问语义
  未被 TRM 保证；`driver/` 也全程 32 位）。
- 提供 `SG200X_LL_REG32(base, off)`、`SG200X_LL_SET_BITS/CLEAR_BITS/
  MODIFY_REG/READ_BIT` 原语（对齐 STM32 LL utils 的语义）。
- 提供 `SG200X_LL_FENCE()`（`fence io, io`）：在"写寄存器→读回状态"或
  "配置外设→触发"的路径上显式使用；TRM 未描述弱序风险，但跨核/TOP_MISC
  路由寄存器一律加 fence，宁多勿少。
- **CSR/缓存层依赖 SDK `core_rv64.h`（2026-09-05 用户批准，修正此前的
  "零依赖"决定）**：`sg200x_ll_csr.h` 是 CSI（`core_rv64.h` →
  `csi_rv64_gcc.h`）之上的薄适配——`__get/__set_MSTATUS`、
  `__enable/__disable_irq`、`__get/__set_MIE`、`__DSB/__ISB`。
  自留汇编：`rdtime`（CSI 的 timer 函数被 `#if 0` 编译掉了）和 **D-cache
  维护**——固件工具链的汇编器不认 `dcache.*` 助记符，CSI 的
  `__DCACHE_IPA/CPA/CIPA` 无法实例化；sgll 逐字采用 SDK
  `arch/riscv64/src/cache.c`（driver DMA 路径在板上验证的同一实现）的
  裸指令字编码（`dcache.ipa/cpa/cipa a0` = `.long 0x02a5/0x0295/0x02b5
  000b`，收尾 `sync.s`），64 字节行循环，函数签名与 SDK 的
  `inv/clean/flush_dcache_range` 对齐。全部内容保留 `__riscv` 守卫：
  RISC-V 构建需加 SDK `install/include/arch` 路径，宿主侧工具链不受影响。

### 3.4 C23 的务实口径

目标是 C23（`-std=gnu2x`），但**不使用任何 C23 才有的语法**（不用
`constexpr` 函数、`_BitInt`）。用到的只有：`static inline`、`_Static_assert`、
`alignas`、指定初始化器——C11/C17 语义下同样成立。理由：SDK 的 xPack
RISC-V GCC 对 gnu2x 支持不完整，头文件库最重要的指标是"任何一颗编译器
都能编过"。等工具链确认后，再考虑用 C23 特性收紧（如 `constexpr` 校验
基址常量）。

## 4. 模块清单与实施阶段

按"能点亮→能通信→能定时→能搬运→能测量"排序；每阶段都以板上 smoke test
收尾（走现有 `scripts/deploy_rtos_fw.sh`）。

| 阶段 | 模块 | TRM 依据 | 板上验证 |
|---|---|---|---|
| P0 | csr → utils → defs → plic → rcc → sg200x.h（umbrella 收尾，含 sg2002.h，见 §3.0） | Ch7/8 + clock_tree.hpp + PLIC 规范/DTS | PLIC 中断进出冒烟；复用 `driver/` 的时钟契约自检 |

> **P0 状态（2026-09-05）：代码已实现；已改用 nrfx 风格全小写命名；静态验证（含 xPack 固件工具链 -Werror 全探针）与一致性审计已通过；未上板。**
> 8 个头（sg200x.h、sg2002.h、csr/utils/defs/plic/rcc）+ INTERFACE CMake +
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
+ `_Static_assert` 偏移自检 + 对应 smoke test。

## 5. 构建集成

- `sgll/CMakeLists.txt`：`add_library(sgll INTERFACE)` +
  `target_include_directories(sgll INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/inc)`。
- 接入点：SDK overlay 的
  `sdk/sg200x/overlay/freertos/cvitek/task/xrobot/CMakeLists.txt` 里
  `add_subdirectory("${SG200X_BSP_ROOT}/sgll" sgll)`，并
  `target_link_libraries(xrobot PUBLIC sgll)`。C 文件零新增，构建时间不变。
- 编译告警基线：`-Wall -Wextra` 下零告警；与 `-fno-exceptions -fno-rtti`
  天然兼容（纯 C）。

## 6. 验证策略

1. **静态**：`_Static_assert(offsetof(SG200X_LL_UART_TypeDef, USR) == 0x07c)`
   式偏移锁定；基址宏与 TRM Table 3.4 逐一对照（可写一个宿主端脚本从
   `.tmp-sg2002-trm-en.txt` 抽表自动复核）。
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
| `sg200x_ll_csr.h` | 内联 asm，无外部依赖 | `mie/mstatus` 全局中断开关、`fence`、`rdtime` 只读封装，十几行。PLIC 原语与中断进出路径都依赖它；原方案把 CSR 排除在外是错的，修正之（不再依赖 SDK `core_rv64.h`） |

这两项建议提前到 **P0**：它们是 P1~P7 所有中断型 smoke test 的地基，
比先写 GPIO 再回头补中断层返工少。

### 8.2 可选（按需，不排期）

空文件 `sg200x_ll_efuse.h`、`sg200x_ll_mbox.h` 已创建（2026-09-05），待按需实施：

| 模块 | 基址 | 说明 |
|---|---|---|
| `sg200x_ll_efuse.h` | 0x03050000 | 芯片信息；TRM 21.7.3 的 SARADC trim 校准流程依赖它 |
| `sg200x_ll_mbox.h` | ap_mailbox 0x01900000、RTCSYS_MBOX 0x05024000 | 双核通信，需要与 Linux/另一核交互时再加 |
| `sg200x_ll_sdmmc.h` | 0x04300000/04310000/04320000 | 只做寄存器映射层；协议层已有 `driver/sg200x_sdio` 在开发 |
| `sg200x_ll_i2s.h` | 0x04100000..0x04130000 | 第 20 章音频；仅当出现音频场景 |

### 8.3 明确不做

USB（0x04340000）、ETH0（0x04070000）、DDR 控制器、SPI NOR/NAND 控制器、
8051 子系统、TPU/ISP/视频管线、CryptoDMA——这些的复杂度在协议与系统
初始化，寄存器原语不解决问题。TRNG（0x02070000）同样暂缓：当前固件没有
随机数需求，出现时再按 8.1 的格式补进排期。
