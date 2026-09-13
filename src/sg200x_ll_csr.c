#include "sg200x_ll_csr.h"

#if defined(__riscv)

/**
 * @def DCACHE_IPA_A0
 * @brief 使用 a0 地址的缓存失效指令编码 / Cache-invalidate instruction encoding using the address in a0.
 */
#define DCACHE_IPA_A0 ".long 0x02a5000b"
/**
 * @def DCACHE_CPA_A0
 * @brief 使用 a0 地址的缓存写回指令编码 / Cache-clean instruction encoding using the address in a0.
 */
#define DCACHE_CPA_A0 ".long 0x0295000b"
/**
 * @def DCACHE_CIPA_A0
 * @brief 使用 a0 地址的缓存写回并失效指令编码 / Cache-clean-and-invalidate encoding using the address in a0.
 */
#define DCACHE_CIPA_A0 ".long 0x02b5000b"
/**
 * @def SYNC_S
 * @brief 等待 C906 缓存操作完成的 sync.s 指令编码。
 *        sync.s instruction encoding completing C906 cache operations.
 */
#define SYNC_S ".long 0x0190000b"

/**
 * @def DCACHE_OP_RANGE(OP, addr, size)
 * @brief 对覆盖地址范围的缓存行执行指定指令。
 *        Execute a selected cache instruction over lines covering a range.
 *
 * @param OP 使用 a0 的指令字符串 / Instruction string using a0.
 * @param addr 范围起始地址 / Range start address.
 * @param size 范围长度，单位为字节 / Range length in bytes.
 * @note 显式绑定 a0 并保留 memory clobber，避免优化和 LTO 改变操作数；末尾执行 sync.s。
 *       Bind a0 explicitly and retain the memory clobber so optimization and LTO cannot detach the operand;
 *       finish with sync.s.
 */
#define DCACHE_OP_RANGE(OP, addr, size)                                                                      \
    do                                                                                                       \
    {                                                                                                        \
        register uintptr_t line_ __asm__("a0") = (uintptr_t)(addr) & ~(uintptr_t)(LL_DCACHE_LINE_SIZE - 1U); \
        const uintptr_t end_ =                                                                               \
            (((uintptr_t)(addr) + (size_t)(size) + (LL_DCACHE_LINE_SIZE - 1U)) &                             \
             ~(uintptr_t)(LL_DCACHE_LINE_SIZE - 1U));                                                        \
        for (; line_ < end_; line_ += LL_DCACHE_LINE_SIZE)                                                   \
        {                                                                                                    \
            __asm__ __volatile__(OP : : "r"(line_) : "memory");                                              \
        }                                                                                                    \
        __asm__ __volatile__(SYNC_S ::: "memory");                                                           \
    } while (0)

void sg200x_ll_csr_dcache_invalidate_range(uintptr_t addr, size_t size)
{
    DCACHE_OP_RANGE(DCACHE_IPA_A0, addr, size);
}

void sg200x_ll_csr_dcache_clean_range(uintptr_t addr, size_t size)
{
    DCACHE_OP_RANGE(DCACHE_CPA_A0, addr, size);
}

void sg200x_ll_csr_dcache_clean_invalidate_range(uintptr_t addr, size_t size)
{
    DCACHE_OP_RANGE(DCACHE_CIPA_A0, addr, size);
}

void sg200x_ll_csr_delay_nops(uint32_t iterations)
{
    for (uint32_t count = 0U; count < iterations; ++count)
    {
        __asm__ __volatile__("nop" ::: "memory");
    }
}

#endif
