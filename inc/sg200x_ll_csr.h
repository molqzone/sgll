/**
 * @file sg200x_ll_csr.h
 * @brief C906L CSR、中断、屏障和缓存维护接口。
 *        C906L CSR, interrupt, barrier, and cache-maintenance interfaces.
 * @ingroup SGLL_CSR
 *
 * @note CSR、中断与时间原语由 sgll_core.h 提供：默认后端自带内联汇编，RISC-V 构建不需要 SDK 头文件；
 *       定义 SGLL_CORE_BACKEND_SDK 可切换到 SDK core_rv64.h/csi_rv64_gcc.h 对照后端。
 *       CSR, interrupt, and time primitives come from sgll_core.h: the default backend is in-library inline
 * assembly and RISC-V builds need no SDK header, while SGLL_CORE_BACKEND_SDK selects the SDK
 * core_rv64.h/csi_rv64_gcc.h reference backend.
 * @note RISC-V 专用接口由 __riscv 守卫；I/O 屏障提供宿主实现。C906L 为 RV64，并保留 RV32 time 读取分支。
 *       RISC-V-only interfaces are guarded by __riscv; the I/O fence has a host implementation. C906L is RV64
 * and the RV32 time-read path is retained.
 * @note SDK 的 time 访问器被禁用，故保留 rdtime；SDK __ISB 不提供指令屏障，故显式使用 fence.i。
 *       The SDK time accessors are disabled, so rdtime is retained; SDK __ISB lacks an instruction barrier,
 * so fence.i is explicit.
 * @note 固件汇编器不识别 SDK 缓存助记符；src 中使用 SDK cache.c 对应的裸指令字及 64 字节范围循环，
 *       tests/run.py 在提供 SDK cache.c 时比对两者的指令字。
 *       The firmware assembler does not recognize SDK cache mnemonics; src uses the matching raw instruction
 * words and 64-byte range loops from SDK cache.c, and tests/run.py compares the words when that file is
 * available.
 * @see SG2002 SDK arch/riscv64/src/cache.c；缓存指令来源。Cache instruction source.
 */

/**
 * @defgroup SGLL_CSR 核心寄存器与缓存 / Core registers and cache
 * @ingroup SGLL
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "sg2002.h"
#include "sgll_core.h"

/**
 * @brief 执行设备 I/O 与编译器内存屏障。
 *        Issue a device I/O and compiler memory barrier.
 * @ingroup SGLL_CSR
 *
 * @note RISC-V 使用 fence iorw, iorw；宿主构建使用顺序一致的线程屏障。
 *       RISC-V uses fence iorw, iorw; host builds use a sequentially consistent thread fence.
 */
static inline void sgll_csr_fence_io(void)
{
#if defined(__riscv)
    __asm__ __volatile__("fence iorw, iorw" ::: "memory");
#else
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
#endif
}

#if defined(__riscv)


/**
 * @brief 读取机器状态寄存器。
 *        Read the machine-status register.
 * @ingroup SGLL_CSR
 *
 * @return 当前 mstatus 值。
 *         Current mstatus value.
 */
static inline unsigned long sgll_csr_mstatus_read(void) { return sgll_core_mstatus_read(); }

/**
 * @brief 写入完整的机器状态寄存器。
 *        Write the complete machine-status register.
 * @ingroup SGLL_CSR
 *
 * @param value 要写入或编码的数值。
 *        Value to write or encode.
 */
static inline void sgll_csr_mstatus_write(unsigned long value) { sgll_core_mstatus_write(value); }

/**
 * @brief 读取机器中断使能寄存器。
 *        Read the machine interrupt-enable register.
 * @ingroup SGLL_CSR
 *
 * @return 当前 mie 值。
 *         Current mie value.
 */
static inline unsigned long sgll_csr_mie_read(void) { return sgll_core_mie_read(); }

/**
 * @brief 写入完整的机器中断使能寄存器。
 *        Write the complete machine interrupt-enable register.
 * @ingroup SGLL_CSR
 *
 * @param value 要写入或编码的数值。
 *        Value to write or encode.
 */
static inline void sgll_csr_mie_write(unsigned long value) { sgll_core_mie_write(value); }

/**
 * @brief 置位指定的 mie 中断使能位。
 *        Set the selected mie interrupt-enable bits.
 * @ingroup SGLL_CSR
 *
 * @param mask 要操作的位掩码。
 *        Bit mask to operate on.
 * @note 此读改写操作不原子；调用者应禁止中断或自行串行化。
 *       This read-modify-write operation is not atomic; disable interrupts or serialize it externally.
 */
static inline void sgll_csr_mie_set(unsigned long mask) { sgll_core_mie_set(mask); }

/**
 * @brief 清除指定的 mie 中断使能位。
 *        Clear the selected mie interrupt-enable bits.
 * @ingroup SGLL_CSR
 *
 * @param mask 要操作的位掩码。
 *        Bit mask to operate on.
 * @note 此读改写操作不原子；调用者应禁止中断或自行串行化。
 *       This read-modify-write operation is not atomic; disable interrupts or serialize it externally.
 */
static inline void sgll_csr_mie_clear(unsigned long mask) { sgll_core_mie_clear(mask); }

/**
 * @brief 使能机器级全局中断。
 *        Enable machine-mode global interrupts.
 * @ingroup SGLL_CSR
 *
 * @note 各中断源的 mie 位和 PLIC 门控仍需单独配置。
 *       Per-source mie bits and PLIC enables must still be configured separately.
 */
static inline void sgll_csr_global_irq_enable(void) { sgll_core_irq_enable(); }

/**
 * @brief 禁止机器级全局中断。
 *        Disable machine-mode global interrupts.
 * @ingroup SGLL_CSR
 */
static inline void sgll_csr_global_irq_disable(void) { sgll_core_irq_disable(); }

/**
 * @brief 保存 mstatus 并禁止机器级全局中断。
 *        Save mstatus and disable machine-mode global interrupts.
 * @ingroup SGLL_CSR
 *
 * @return 供 critical_exit 恢复的完整 mstatus 快照。
 *         Complete mstatus snapshot to restore with critical_exit.
 */
static inline unsigned long sgll_csr_critical_enter(void)
{
    unsigned long prior = sgll_core_mstatus_read();
    sgll_core_irq_disable();
    return prior;
}

/**
 * @brief 恢复进入临界区前的 mstatus。
 *        Restore the mstatus saved before entering a critical section.
 * @ingroup SGLL_CSR
 *
 * @param prior critical_enter 返回的 mstatus 快照。
 *        The mstatus snapshot returned by critical_enter.
 */
static inline void sgll_csr_critical_exit(unsigned long prior) { sgll_csr_mstatus_write(prior); }

/**
 * @brief 使能掩码指定的机器中断源。
 *        Enable the machine interrupt sources selected by a mask.
 * @ingroup SGLL_CSR
 *
 * @param mie_mask 要操作的 mie 中断使能位掩码。
 *        mie interrupt-enable bit mask.
 * @note 此读改写操作不原子；调用者应禁止中断或自行串行化。
 *       This read-modify-write operation is not atomic; disable interrupts or serialize it externally.
 */
static inline void sgll_csr_mie_enable(unsigned long mie_mask) { sgll_csr_mie_set(mie_mask); }

/**
 * @brief 禁止掩码指定的机器中断源。
 *        Disable the machine interrupt sources selected by a mask.
 * @ingroup SGLL_CSR
 *
 * @param mie_mask 要操作的 mie 中断使能位掩码。
 *        mie interrupt-enable bit mask.
 * @note 此读改写操作不原子；调用者应禁止中断或自行串行化。
 *       This read-modify-write operation is not atomic; disable interrupts or serialize it externally.
 */
static inline void sgll_csr_mie_disable(unsigned long mie_mask) { sgll_csr_mie_clear(mie_mask); }

/**
 * @brief 使能核心的机器外部中断输入。
 *        Enable the core's machine external-interrupt input.
 * @ingroup SGLL_CSR
 *
 * @note 此读改写操作不原子；调用者应禁止中断或自行串行化。
 *       This read-modify-write operation is not atomic; disable interrupts or serialize it externally.
 * @note 仅设置 mie.MEIE；mstatus.MIE 和 PLIC 源使能由调用者或 RTOS 管理。
 *       Only mie.MEIE is set; the caller or RTOS manages mstatus.MIE and PLIC source enables.
 */
static inline void sgll_csr_external_irq_enable(void) { sgll_csr_mie_set(SGLL_MIE_MEIE_BIT); }

/**
 * @brief 禁止核心的机器外部中断输入。
 *        Disable the core's machine external-interrupt input.
 * @ingroup SGLL_CSR
 *
 * @note 此读改写操作不原子；调用者应禁止中断或自行串行化。
 *       This read-modify-write operation is not atomic; disable interrupts or serialize it externally.
 * @note 仅清除 mie.MEIE，不修改 PLIC 中的各源使能位。
 *       Only mie.MEIE is cleared; per-source PLIC enables are unchanged.
 */
static inline void sgll_csr_external_irq_disable(void) { sgll_csr_mie_clear(SGLL_MIE_MEIE_BIT); }

/**
 * @brief 读取 64 位 time CSR 计数值。
 *        Read the 64-bit time CSR counter.
 * @ingroup SGLL_CSR
 *
 * @return 未经时间单位换算的计数值。
 *         Raw counter ticks without time-unit conversion.
 * @note 本板 time 时基为 25 MHz。使用 CSR，不访问非 QEMU SDK 未提供的 CLINT mtime 地址。
 *       The board time base is 25 MHz. Use the CSR rather than a CLINT mtime address absent from the non-QEMU
 * SDK.
 * @note RV32 路径通过高低字重读避免进位撕裂；C906L 使用 RV64 路径，两者都在 sgll_core.h 中实现。
 *       The RV32 path rereads the high word to avoid rollover tearing; C906L uses the RV64 path, and both
 * live in sgll_core.h.
 */
static inline uint64_t sgll_csr_time_read(void) { return sgll_core_time_read(); }

/**
 * @brief 同步本核心后续的指令取指。
 *        Synchronize subsequent instruction fetches on this core.
 * @ingroup SGLL_CSR
 *
 * @note 使用真正的 fence.i；SDK 的 __ISB 仅提供数据屏障。
 *       Use a real fence.i; the SDK __ISB only provides a data barrier.
 */
static inline void sgll_csr_fence_i(void)
{

    __asm__ __volatile__(".option push\n\t"
                         ".option arch, +zifencei\n\t"
                         "fence.i\n\t"
                         ".option pop" ::
                             : "memory");
}

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 使指定范围的数据缓存失效。
     *        Invalidate data-cache lines covering a range.
     * @ingroup SGLL_CSR
     *
     * @param addr 范围起始地址。
     *        Start address of the range.
     * @param size 范围长度，单位为字节。
     *        Range length in bytes.
     * @pre 地址加长度再加 63 不得溢出 uintptr_t；调用者拥有覆盖的完整缓存行。
     *      Address plus length plus 63 must not overflow uintptr_t; the caller owns every covered cache line.
     * @note 起始地址向下、末地址向上对齐至 64 字节边界，末尾执行 sync.s 等待完成。
     *       The start is rounded down and the end up to 64-byte boundaries; sync.s completes the operation.
     * @note 失效不写回脏数据；仅用于可丢弃本地缓存内容的范围。
     *       Invalidation does not write back dirty data; use it only where local cached contents may be
     * discarded.
     */
    void sgll_csr_dcache_invalidate_range(uintptr_t addr, size_t size);
    /**
     * @brief 写回指定范围的脏数据缓存行。
     *        Clean dirty data-cache lines covering a range.
     * @ingroup SGLL_CSR
     *
     * @param addr 范围起始地址。
     *        Start address of the range.
     * @param size 范围长度，单位为字节。
     *        Range length in bytes.
     * @pre 地址加长度再加 63 不得溢出 uintptr_t；调用者拥有覆盖的完整缓存行。
     *      Address plus length plus 63 must not overflow uintptr_t; the caller owns every covered cache line.
     * @note 起始地址向下、末地址向上对齐至 64 字节边界，末尾执行 sync.s 等待完成。
     *       The start is rounded down and the end up to 64-byte boundaries; sync.s completes the operation.
     */
    void sgll_csr_dcache_clean_range(uintptr_t addr, size_t size);
    /**
     * @brief 写回并使指定范围的数据缓存失效。
     *        Clean and invalidate data-cache lines covering a range.
     * @ingroup SGLL_CSR
     *
     * @param addr 范围起始地址。
     *        Start address of the range.
     * @param size 范围长度，单位为字节。
     *        Range length in bytes.
     * @pre 地址加长度再加 63 不得溢出 uintptr_t；调用者拥有覆盖的完整缓存行。
     *      Address plus length plus 63 must not overflow uintptr_t; the caller owns every covered cache line.
     * @note 起始地址向下、末地址向上对齐至 64 字节边界，末尾执行 sync.s 等待完成。
     *       The start is rounded down and the end up to 64-byte boundaries; sync.s completes the operation.
     */
    void sgll_csr_dcache_clean_invalidate_range(uintptr_t addr, size_t size);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 执行指定数量的 NOP 延迟迭代。
     *        Execute a specified number of NOP delay iterations.
     * @ingroup SGLL_CSR
     * @param iterations NOP 迭代次数。
     *        Number of NOP iterations.
     * @note 仅 RISC-V；每次包含循环开销，不能作为精确时钟周期或时间单位。
     *        RISC-V only; each iteration includes loop overhead and is not an exact cycle or time unit.
     */
    void sgll_csr_delay_nops(uint32_t iterations);

#ifdef __cplusplus
}
#endif

#endif
