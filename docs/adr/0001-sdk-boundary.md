# ADR-0001：sgll 与厂商 SDK 的边界

- 状态：已采纳（2026-09-12）
- 影响范围：`inc/sgll_core.h`（新增）、`inc/sg200x_ll_csr.h`、`CMakeLists.txt`、`tests/`
- 相关：`PLAN.md` §3.0/§5、BSP 的 `sdk/sg200x/overlay/freertos/cvitek/task/xrobot/CMakeLists.txt`

## 背景

sgll 定位是"无状态底层库"（对标 STM32 LL / nrfx），并作为独立仓库分发；但历史上
RISC-V 构建需要厂商 SDK 的 `core_rv64.h`（经 `csi_rv64_gcc.h`）才能编译。实际依赖面很小：

| 依赖 | 位置 | 内容 |
|---|---|---|
| `#include "core_rv64.h"` | `inc/sg200x_ll_csr.h` | `__ASM`、`__get/__set_MSTATUS`、`__get/__set_MIE`、`__enable/__disable_irq`，均为 1–3 行 `csrr/csrw/csrs/csrc` 包装 |

而库中最"厂商特有"的部分（C906 缓存指令字 `.long 0x02a5000b` 等、`sync.s`、`fence.i`、
`rdtime`）**早已由 sgll 自己持有**，并没有依赖 SDK。这带来三个具体问题：

1. SDK 的 `install/include/arch` 是**构建产物**：外部工程要用 sgll 必须先把 GB 级 SDK 跑到 staging。
2. 测试与审计脚本被迫要求 SDK 路径（`tests/run.py --core-include` 为必需参数），CI 无法只 checkout sgll。
3. 第三方仓库（`sdk/sg200x/sdk.env` 固定版本）的头文件变更会影响 sgll 的编译面。

## 决策

**在"原语层"独立，在"固件集成层"不独立。**

1. 新增 `inc/sgll_core.h`：以库内内联汇编实现 CSR 读写、`mstatus.MIE` 开关、`mie` 置位/清除、
   64/32 位 `rdtime` 读取；RISC-V 构建不再需要任何 SDK 头。
2. 保留 SDK 对照后端：定义 `SGLL_CORE_BACKEND_SDK`（CMake 选项 `SGLL_CORE_BACKEND_SDK=ON`
   配合 `SGLL_CORE_INCLUDE_DIR`）即把同一批原语编译到厂商头实现上，用于 A/B 比对。
   **两个后端语义完全一致**（`mstatus`/`mie` 读改写、`csrs mstatus, 8`/`csrc mstatus, 8`），
   切换不改变行为。
3. 公开 API 不变：`sgll_csr_*` 名称、注释与契约保持原样，只是内部委托给 `sgll_core_*`。
4. 固件集成层**继续由 SDK 拥有**：`start.S`、trap/异常、FreeRTOS port、tick、
   `cv181x_lscript.ld`、`request_irq()` 分发与 remoteproc 镜像约定都不在本 ADR 范围内，
   也不建议复制（与 Sophgo Debian 集成/FSBL 的契约保持一致）。

## 理由

- 独立成本极低（约 150 行头文件），收益明确：sgll 可被任何 RISC-V 工具链直接编译，
  测试/审计/CI 只需交叉编译器。
- 不独立的部分重写成本极高（启动、trap、RTOS、链接脚本），且与厂商镜像流程耦合，
  重写只会增加与 SDK 行为漂移的风险。
- 缓存指令等真正易错的编码本来就在 sgll 手里，且已有 QEMU 操作数验证；
  为了 6 个标准 CSR 包装继续依赖厂商头，性价比是反的。

## 后果

正向：

- RISC-V 构建可在**没有 SDK 头**的情况下通过；`run.py` 默认路径即验证这一点。
- 可做 A/B 构建：`run.py --core-include <sdk arch>` 编译 `tests/core_probe.c` 两侧并逐符号
  比较助记符序列与 CSR 操作数；`--sdk-cache <sdk cache.c>` 比对缓存指令字。
- 宿主构建行为不变（非 RISC-V 仍不定义任何原语）。

代价与约束：

- sgll 现在自己拥有 CSR 语义；变更需对照 RISC-V 特权规范，且必须保持两个后端一致
  （A/B 测试是硬门槛）。
- `mie` 置位/清除仍为读改写（与 SDK 一致），因此 `sgll_csr_mie_set/clear` 的"不原子"注释
  依旧成立。若要改为原子的 `csrs/csrc`，需要单独立项并同步更新文档与调用方预期。

## 备选方案

- **保持现状（依赖 SDK 头）**：对"本 BSP 能编过"没有影响，但 sgll 作为独立组件的价值打折，
  测试/CI 受制于 SDK staging。否决。
- **完全脱离 SDK（自建启动/trap/RTOS/链接脚本）**：与厂商镜像流程脱钩，收益只有"少一个仓库"。
  否决。
- **把 CSR 常量一并搬进 `sgll_core.h`**：会造成与 `sg2002.h` 的重复定义，且在途的
  `driver/`、`tests/` 都在用现有名字。改为让 `sgll_core.h` 只依赖 `<stdint.h>` 与编译器宏，
  设备字段常量仍留在 `sg2002.h`。

## 后续（下一个架构缝）

真正的 SDK 耦合不在头文件而在**中断分发**：`driver/*.cpp` 仍全部通过 SDK 的 `request_irq()`
注册 ISR，sgll 只在 claim/complete 被调用。若要 sgll 成为可独立使用的裸机底座，需要一层
**薄且可选**的中断层（trap 入口 + ISR 表 + `mstatus.MIE` 初始化，接口与 `request_irq` 同形），
由板级/RTOS 决定是否接管。该工作不在本 ADR 内。
