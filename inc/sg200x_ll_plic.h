/**
 * @file sg200x_ll_plic.h
 * @brief C906L 上下文的 PLIC 中断原语。
 *        PLIC interrupt primitives for the C906L context.
 * @ingroup SGLL_PLIC
 *
 * @note TRM 不提供 PLIC 寄存器描述；布局采用 SDK irq.h 的标准 RISC-V PLIC 地址映射。
 *       The TRM does not describe PLIC registers; the layout follows the SDK irq.h standard RISC-V PLIC
 * address map.
 * @note 大核 DTS 描述 101 个源、最大优先级 7；C906L CPU2 使用 sg2002.h 的独立 IRQ 编号，不采用大核 TRM 表 3.1
 * 编号。 The big-core DTS describes 101 sources and maximum priority seven; C906L CPU2 uses the separate IRQ
 * numbering in sg2002.h rather than big-core TRM Table 3.1.
 * @note 板级 DMA 适配器在 0x70200004 领取和完成 C906L 机器态上下文 0 的中断。
 *       The board DMA adapter claims and completes C906L machine-mode context-zero interrupts at 0x70200004.
 * @note 调用者检查源编号并串行化使能寄存器的读改写。
 *       Callers validate source IDs and serialize enable-register read-modify-write operations.
 * @see SG2002 SDK arch/riscv64/include/irq.h；PLIC 地址映射。PLIC address map.
 */

/**
 * @defgroup SGLL_PLIC PLIC 中断控制器 / PLIC interrupt controller
 * @ingroup SGLL
 */

#pragma once

#include <stdbool.h>

#include "sg2002.h"
#include "sg200x_ll_csr.h"
#include "sg200x_ll_utils.h"

/**
 * @brief 设置 C906L PLIC 中断源的优先级。
 *        Set a C906L PLIC interrupt source priority.
 * @ingroup SGLL_PLIC
 *
 * @param irq 器件头中定义的 PLIC 中断源编号。
 *        PLIC source ID defined by the device header.
 * @param prio 优先级 0 到 7；0 屏蔽该源，数值越大优先级越高。
 *        Priority from 0 through 7; zero suppresses the source and larger values have higher priority.
 * @pre irq 必须是器件头定义的有效源编号且小于 NUM_IRQ；原语不检查边界。
 *      irq must be a valid device-defined source below NUM_IRQ; the primitive does not check bounds.
 */
static inline void sgll_plic_irq_priority_set(uint32_t irq, uint32_t prio)
{
    PLIC_C906L->PRIORITY[irq] = prio;
}

/**
 * @brief 读取 C906L PLIC 中断源的优先级。
 *        Read a C906L PLIC interrupt source priority.
 * @ingroup SGLL_PLIC
 *
 * @param irq 器件头中定义的 PLIC 中断源编号。
 *        PLIC source ID defined by the device header.
 * @return 当前优先级值。
 *         Current priority value.
 * @pre irq 必须是器件头定义的有效源编号且小于 NUM_IRQ；原语不检查边界。
 *      irq must be a valid device-defined source below NUM_IRQ; the primitive does not check bounds.
 */
static inline uint32_t sgll_plic_irq_priority_get(uint32_t irq) { return PLIC_C906L->PRIORITY[irq]; }

/**
 * @brief 使能中断源到 C906L 上下文的路由。
 *        Enable an interrupt source for the C906L context.
 * @ingroup SGLL_PLIC
 *
 * @param irq 器件头中定义的 PLIC 中断源编号。
 *        PLIC source ID defined by the device header.
 * @pre irq 必须是器件头定义的有效源编号且小于 NUM_IRQ；原语不检查边界。
 *      irq must be a valid device-defined source below NUM_IRQ; the primitive does not check bounds.
 */
static inline void sgll_plic_irq_enable(uint32_t irq)
{
    PLIC_C906L->ENABLE[irq / PLIC_IRQS_PER_WORD] |= SGLL_BIT(irq % PLIC_IRQS_PER_WORD);
}

/**
 * @brief 禁止中断源到 C906L 上下文的路由。
 *        Disable an interrupt source for the C906L context.
 * @ingroup SGLL_PLIC
 *
 * @param irq 器件头中定义的 PLIC 中断源编号。
 *        PLIC source ID defined by the device header.
 * @pre irq 必须是器件头定义的有效源编号且小于 NUM_IRQ；原语不检查边界。
 *      irq must be a valid device-defined source below NUM_IRQ; the primitive does not check bounds.
 */
static inline void sgll_plic_irq_disable(uint32_t irq)
{
    PLIC_C906L->ENABLE[irq / PLIC_IRQS_PER_WORD] &= ~(uint32_t)SGLL_BIT(irq % PLIC_IRQS_PER_WORD);
}

/**
 * @brief 查询中断源在 C906L 上下文中是否使能。
 *        Query whether a source is enabled for the C906L context.
 * @ingroup SGLL_PLIC
 *
 * @param irq 器件头中定义的 PLIC 中断源编号。
 *        PLIC source ID defined by the device header.
 * @return 源使能位为 1 时返回 true。
 *         True when the source enable bit is set.
 * @pre irq 必须是器件头定义的有效源编号且小于 NUM_IRQ；原语不检查边界。
 *      irq must be a valid device-defined source below NUM_IRQ; the primitive does not check bounds.
 */
static inline bool sgll_plic_irq_is_enabled(uint32_t irq)
{
    return (PLIC_C906L->ENABLE[irq / PLIC_IRQS_PER_WORD] & SGLL_BIT(irq % PLIC_IRQS_PER_WORD)) != 0U;
}

/**
 * @brief 查询中断源是否处于待处理状态。
 *        Query whether an interrupt source is pending.
 * @ingroup SGLL_PLIC
 *
 * @param irq 器件头中定义的 PLIC 中断源编号。
 *        PLIC source ID defined by the device header.
 * @return 待处理位为 1 时返回 true。
 *         True when the pending bit is set.
 * @pre irq 必须是器件头定义的有效源编号且小于 NUM_IRQ；原语不检查边界。
 *      irq must be a valid device-defined source below NUM_IRQ; the primitive does not check bounds.
 */
static inline bool sgll_plic_irq_is_pending(uint32_t irq)
{
    return (PLIC_C906L->PENDING[irq / PLIC_IRQS_PER_WORD] & SGLL_BIT(irq % PLIC_IRQS_PER_WORD)) != 0U;
}

/**
 * @brief 设置 C906L PLIC 上下文的优先级阈值。
 *        Set the C906L PLIC context priority threshold.
 * @ingroup SGLL_PLIC
 *
 * @param threshold 阈值 0 到 7；优先级不大于该值的源不会通知此上下文。
 *        Threshold from 0 through 7; sources at or below it do not signal this context.
 */
static inline void sgll_plic_threshold_set(uint32_t threshold) { PLIC_C906L->THRESHOLD = threshold; }

/**
 * @brief 读取 C906L PLIC 上下文的优先级阈值。
 *        Read the C906L PLIC context priority threshold.
 * @ingroup SGLL_PLIC
 *
 * @return 当前阈值。
 *         Current threshold.
 */
static inline uint32_t sgll_plic_threshold_get(void) { return PLIC_C906L->THRESHOLD; }

/**
 * @brief 领取优先级最高的可处理中断源。
 *        Claim the highest-priority eligible interrupt source.
 * @ingroup SGLL_PLIC
 *
 * @return 领取的源编号；没有可处理中断时返回 0。
 *         Claimed source ID, or zero when no interrupt is eligible.
 * @note 读 CLAIM_COMPLETE 会把该源标记为服务中，完成后必须调用 irq_complete。
 *       Reading CLAIM_COMPLETE marks the source in service; call irq_complete after servicing it.
 */
static inline uint32_t sgll_plic_irq_claim(void) { return PLIC_C906L->CLAIM_COMPLETE; }

/**
 * @brief 提交一个已领取中断源的处理完成通知。
 *        Complete service of a previously claimed interrupt source.
 * @ingroup SGLL_PLIC
 *
 * @param irq 先前由 irq_claim 领取的非零中断源编号。
 *        Nonzero source ID previously returned by irq_claim.
 */
static inline void sgll_plic_irq_complete(uint32_t irq) { PLIC_C906L->CLAIM_COMPLETE = irq; }

#if defined(__riscv)

/**
 * @brief 使能核心的 PLIC 外部中断输入。
 *        Enable the core's PLIC external-interrupt input.
 * @ingroup SGLL_PLIC
 *
 * @note 只修改 mie.MEIE；全局中断状态和 PLIC 各源使能独立管理。
 *       Only mie.MEIE changes; global interrupt state and per-source PLIC enables are managed separately.
 */
static inline void sgll_plic_core_enable(void) { sgll_csr_external_irq_enable(); }

/**
 * @brief 禁止核心的 PLIC 外部中断输入。
 *        Disable the core's PLIC external-interrupt input.
 * @ingroup SGLL_PLIC
 *
 * @note 只修改 mie.MEIE；全局中断状态和 PLIC 各源使能独立管理。
 *       Only mie.MEIE changes; global interrupt state and per-source PLIC enables are managed separately.
 */
static inline void sgll_plic_core_disable(void) { sgll_csr_external_irq_disable(); }

#endif

static_assert(PLIC_IRQ_WORD_COUNT == 2UL, "C906L PLIC enable words changed; update region math");
