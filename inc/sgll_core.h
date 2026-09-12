/**
 * @file sgll_core.h
 * @brief C906L 核心内建原语与后端选择。
 *        C906L core built-ins and backend selection.
 * @ingroup SGLL_CORE
 *
 * @note 默认后端由本库自带内联汇编实现，RISC-V 构建不依赖任何 SDK 头文件。
 *       The default backend is implemented with in-library inline assembly; RISC-V builds need no SDK header.
 * @note 定义 SGLL_CORE_BACKEND_SDK 且 core_rv64.h 可见时，改用 SDK core_rv64.h/csi_rv64_gcc.h 作为对照后端。
 *       两个后端提供同名原语与相同的寄存器语义，可直接做 A/B 编译与指令级对比。
 *       Defining SGLL_CORE_BACKEND_SDK with core_rv64.h on the include path selects the SDK
 *       core_rv64.h/csi_rv64_gcc.h reference backend. Both backends expose identical primitives with identical
 * register semantics, so A/B builds and instruction-level comparison stay possible.
 * @note 缓存指令字、sync.s 与 fence.i 不属于本层，由 sg200x_ll_csr.h/.c 直接持有。
 *       Cache instruction words, sync.s, and fence.i are not part of this layer; sg200x_ll_csr.h/.c own them.
 * @note 非 RISC-V 构建不定义任何原语，宿主测试通过匿名页与链接期替身验证上层。
 *       Non-RISC-V builds define no primitive here; host tests exercise upper layers through anonymous pages
 * and link-time doubles.
 * @see RISC-V 特权规范 v1.12：mstatus.MIE 为位 3，mie 为机器中断使能寄存器。
 *      RISC-V privileged specification v1.12: mstatus.MIE is bit three and mie is the machine
 * interrupt-enable register.
 */

/**
 * @defgroup SGLL_CORE 核心内建原语 / Core built-ins
 * @ingroup SGLL
 */

#pragma once

#include <stdint.h>

#if defined(__riscv)

#if defined(SGLL_CORE_BACKEND_SDK)
#include "core_rv64.h"
#endif

/**
 * @brief 读取机器状态寄存器。
 *        Read the machine-status register.
 * @ingroup SGLL_CORE
 *
 * @return 当前 mstatus 值。
 *         Current mstatus value.
 */
static inline unsigned long sgll_core_mstatus_read(void)
{
#if defined(SGLL_CORE_BACKEND_SDK)
    return (unsigned long)__get_MSTATUS();
#else
    unsigned long value;
    __asm__ __volatile__("csrr %0, mstatus" : "=r"(value));
    return value;
#endif
}

/**
 * @brief 写入完整的机器状态寄存器。
 *        Write the complete machine-status register.
 * @ingroup SGLL_CORE
 *
 * @param value 要写入的 mstatus 值。
 *        mstatus value to write.
 */
static inline void sgll_core_mstatus_write(unsigned long value)
{
#if defined(SGLL_CORE_BACKEND_SDK)
    __set_MSTATUS((uint64_t)value);
#else
    __asm__ __volatile__("csrw mstatus, %0" ::"r"(value) : "memory");
#endif
}

/**
 * @brief 读取机器中断使能寄存器。
 *        Read the machine interrupt-enable register.
 * @ingroup SGLL_CORE
 *
 * @return 当前 mie 值。
 *         Current mie value.
 */
static inline unsigned long sgll_core_mie_read(void)
{
#if defined(SGLL_CORE_BACKEND_SDK)
    return (unsigned long)__get_MIE();
#else
    unsigned long value;
    __asm__ __volatile__("csrr %0, mie" : "=r"(value));
    return value;
#endif
}

/**
 * @brief 写入完整的机器中断使能寄存器。
 *        Write the complete machine interrupt-enable register.
 * @ingroup SGLL_CORE
 *
 * @param value 要写入的 mie 值。
 *        mie value to write.
 */
static inline void sgll_core_mie_write(unsigned long value)
{
#if defined(SGLL_CORE_BACKEND_SDK)
    __set_MIE((uint64_t)value);
#else
    __asm__ __volatile__("csrw mie, %0" ::"r"(value) : "memory");
#endif
}

/**
 * @brief 置位指定的 mie 中断使能位。
 *        Set the selected mie interrupt-enable bits.
 * @ingroup SGLL_CORE
 *
 * @param mask 要置位的位掩码。
 *        Bit mask to set.
 * @note 两个后端都实现为读改写，操作不原子；调用者应禁止中断或自行串行化。
 *       Both backends implement this as a read-modify-write, which is not atomic; disable interrupts or
 * serialize it externally.
 */
static inline void sgll_core_mie_set(unsigned long mask)
{
#if defined(SGLL_CORE_BACKEND_SDK)
    __set_MIE(__get_MIE() | (uint64_t)mask);
#else
    unsigned long value;
    __asm__ __volatile__("csrr %0, mie" : "=r"(value));
    value |= mask;
    __asm__ __volatile__("csrw mie, %0" ::"r"(value) : "memory");
#endif
}

/**
 * @brief 清除指定的 mie 中断使能位。
 *        Clear the selected mie interrupt-enable bits.
 * @ingroup SGLL_CORE
 *
 * @param mask 要清除的位掩码。
 *        Bit mask to clear.
 * @note 两个后端都实现为读改写，操作不原子；调用者应禁止中断或自行串行化。
 *       Both backends implement this as a read-modify-write, which is not atomic; disable interrupts or
 * serialize it externally.
 */
static inline void sgll_core_mie_clear(unsigned long mask)
{
#if defined(SGLL_CORE_BACKEND_SDK)
    __set_MIE(__get_MIE() & ~(uint64_t)mask);
#else
    unsigned long value;
    __asm__ __volatile__("csrr %0, mie" : "=r"(value));
    value &= ~mask;
    __asm__ __volatile__("csrw mie, %0" ::"r"(value) : "memory");
#endif
}

/**
 * @brief 使能机器级全局中断。
 *        Enable machine-mode global interrupts.
 * @ingroup SGLL_CORE
 *
 * @note 置位 mstatus.MIE（位 3），即 csrs mstatus, 8；两个后端一致。
 *       Sets mstatus.MIE (bit three), that is csrs mstatus, 8; both backends agree.
 */
static inline void sgll_core_irq_enable(void)
{
#if defined(SGLL_CORE_BACKEND_SDK)
    __enable_irq();
#else
    __asm__ __volatile__("csrs mstatus, 8" ::: "memory");
#endif
}

/**
 * @brief 禁止机器级全局中断。
 *        Disable machine-mode global interrupts.
 * @ingroup SGLL_CORE
 *
 * @note 清除 mstatus.MIE（位 3），即 csrc mstatus, 8；两个后端一致。
 *       Clears mstatus.MIE (bit three), that is csrc mstatus, 8; both backends agree.
 */
static inline void sgll_core_irq_disable(void)
{
#if defined(SGLL_CORE_BACKEND_SDK)
    __disable_irq();
#else
    __asm__ __volatile__("csrc mstatus, 8" ::: "memory");
#endif
}

#if __riscv_xlen == 64
/**
 * @brief 读取 64 位 time CSR 计数值。
 *        Read the 64-bit time CSR counter.
 * @ingroup SGLL_CORE
 *
 * @return 未经时间单位换算的计数值。
 *         Raw counter ticks without time-unit conversion.
 * @note 使用 rdtime；SDK 的时间访问器在固件中不可用，两个后端因此共用同一实现。
 *       Uses rdtime; the SDK time accessors are unusable in this firmware, so both backends share this
 * implementation.
 */
static inline uint64_t sgll_core_time_read(void)
{
    uint64_t value;
    __asm__ __volatile__("rdtime %0" : "=r"(value));
    return value;
}
#elif __riscv_xlen == 32
/**
 * @brief 读取 64 位 time CSR 计数值。
 *        Read the 64-bit time CSR counter.
 * @ingroup SGLL_CORE
 *
 * @return 未经时间单位换算的计数值。
 *         Raw counter ticks without time-unit conversion.
 * @note 通过高低字重读避免进位撕裂；C906L 使用 RV64 路径，此分支供 RV32 复用。
 *       The high word is reread to avoid rollover tearing; C906L uses the RV64 path and this branch keeps
 * RV32 reuse possible.
 */
static inline uint64_t sgll_core_time_read(void)
{
    uint32_t low;
    uint32_t high0;
    uint32_t high1;
    do
    {
        __asm__ __volatile__("rdtimeh %0" : "=r"(high0));
        __asm__ __volatile__("rdtime %0" : "=r"(low));
        __asm__ __volatile__("rdtimeh %0" : "=r"(high1));
    } while (high0 != high1);
    return ((uint64_t)high1 << 32) | low;
}
#else
#error "Unsupported __riscv_xlen"
#endif

#endif
