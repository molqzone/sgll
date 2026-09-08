/**
 * @file sg200x_ll_mbox.c
 * @brief 共享 DDR 邮箱的所有权交接和缓存同步。
 *        Shared-DDR mailbox ownership transfer and cache synchronization.
 *
 * @ingroup SGLL_MBOX
 */

#include "sg200x_ll_mbox.h"

#include <string.h>

#include "sg200x_ll_csr.h"

/**
 * @brief 同步并使邮箱范围的本地缓存失效。
 *        Synchronize and invalidate local cache for a mailbox range.
 *
 * @param address 范围起始地址。
 *        Range start address.
 * @param length 范围字节数；0 时跳过缓存指令。
 *        Range bytes; zero skips cache instructions.
 * @note 宿主构建使用一致性内存，仅执行屏障。
 *       Host builds use coherent memory and only issue barriers.
 */
static void invalidate(uintptr_t address, size_t length)
{
    sgll_csr_fence_io();
#if defined(__riscv)
    if (length != 0U)
    {
        sgll_csr_dcache_invalidate_range(address, length);
    }
#else

    (void)address;
    (void)length;
#endif
    sgll_csr_fence_io();
}

/**
 * @brief 写回邮箱范围并等待完成。
 *        Clean a mailbox range and wait for completion.
 *
 * @param address 范围起始地址。
 *        Range start address.
 * @param length 范围字节数；0 时跳过缓存指令。
 *        Range bytes; zero skips cache instructions.
 * @note 宿主构建使用一致性内存，仅执行屏障。
 *       Host builds use coherent memory and only issue barriers.
 */
static void clean(uintptr_t address, size_t length)
{
#if defined(__riscv)
    if (length != 0U)
    {
        sgll_csr_dcache_clean_range(address, length);
    }
#else
    (void)address;
    (void)length;
#endif
    sgll_csr_fence_io();
}

/**
 * @brief 解析已验证邮箱布局的状态字地址。
 *        Resolve the state-word address of a validated mailbox layout.
 *
 * @param[in] config 已验证的邮箱布局。
 *        Validated mailbox layout.
 * @return 易失的 32 位状态字指针。
 *         Pointer to the volatile 32-bit state word.
 */
static volatile uint32_t *state_word(const sgll_mbox_config_t *config)
{
    return (volatile uint32_t *)(config->base + config->state_offset);
}

/**
 * @brief 按屏障及缓存顺序发布邮箱所有权状态。
 *        Publish mailbox ownership state with ordered barriers and cache maintenance.
 *
 * @param[in] config 已验证的邮箱布局。
 *        Validated mailbox layout.
 * @param value 将发布的 empty 或 ready 状态值。
 *        Empty or ready state value to publish.
 * @note 只回写状态字所在行；对端看到新状态后即可使用其他头部及载荷缓存行。
 *       Only the state line is written back; the peer may use other header and payload lines once it observes
 * the new state.
 */
static void publish_state(const sgll_mbox_config_t *config, uint32_t value)
{
    sgll_csr_fence_io();
    *state_word(config) = value;
    sgll_csr_fence_io();
#if defined(__riscv)

    sgll_csr_dcache_clean_invalidate_range(config->base + config->state_offset, sizeof(uint32_t));
#endif
    sgll_csr_fence_io();
}

bool sgll_mbox_config_valid(const sgll_mbox_config_t *config)
{
    if (config == nullptr || config->base == 0U || config->header_size == 0U ||
        config->header_size > config->size || config->size > UINTPTR_MAX - config->base ||
        config->empty_state == config->ready_state)
    {
        return false;
    }
    const uintptr_t mask = SGLL_DCACHE_LINE_SIZE - 1U;
    return ((config->base | config->size | config->header_size) & mask) == 0U &&
           config->state_offset % alignof(uint32_t) == 0U &&
           config->state_offset <= config->header_size - sizeof(uint32_t);
}

/**
 * @brief 检查输出缓冲区是否非空、地址不溢出且位于共享区之外。
 *        Check that an output buffer is nonnull, does not overflow, and lies outside shared memory.
 *
 * @param[in] config 已验证的邮箱布局。
 *        Validated mailbox layout.
 * @param buffer 待检查的本地缓冲区地址。
 *        Local buffer address to check.
 * @param length 写入缓冲区的字节数。
 *        Number of bytes written to the buffer.
 * @return 所述地址范围有效且不与共享区重叠时为 true。
 *         True when the range is valid and does not overlap shared memory.
 */
static bool local_buffer(const sgll_mbox_config_t *config, const void *buffer, size_t length)
{
    const uintptr_t address = (uintptr_t)buffer;
    return address != 0U && length <= UINTPTR_MAX - address &&
           (address >= config->base + config->size ||
            (address <= config->base && length <= config->base - address));
}

sgll_mbox_result_t sgll_mbox_rx_acquire(const sgll_mbox_config_t *config, void *header, size_t capacity)
{
    if (!sgll_mbox_config_valid(config) || capacity < config->header_size ||
        !local_buffer(config, header, config->header_size))
    {
        return SGLL_MBOX_INVALID;
    }
    invalidate(config->base + config->state_offset, sizeof(uint32_t));
    if (*state_word(config) != config->ready_state)
    {
        return SGLL_MBOX_BUSY;
    }

    invalidate(config->base, config->header_size);
    memcpy(header, (const void *)config->base, config->header_size);
    return SGLL_MBOX_OK;
}

sgll_mbox_result_t
sgll_mbox_rx_payload(const sgll_mbox_config_t *config, size_t length, const uint8_t **payload)
{
    if (!sgll_mbox_config_valid(config) || length > config->size - config->header_size ||
        !local_buffer(config, payload, sizeof(*payload)))
    {
        return SGLL_MBOX_INVALID;
    }
    if (*state_word(config) != config->ready_state)
    {
        return SGLL_MBOX_BUSY;
    }
    const uintptr_t address = config->base + config->header_size;
    invalidate(address, length);
    *payload = (const uint8_t *)address;
    return SGLL_MBOX_OK;
}

sgll_mbox_result_t sgll_mbox_rx_release(const sgll_mbox_config_t *config)
{
    if (!sgll_mbox_config_valid(config))
    {
        return SGLL_MBOX_INVALID;
    }
    if (*state_word(config) != config->ready_state)
    {
        return SGLL_MBOX_BUSY;
    }
    publish_state(config, config->empty_state);
    return SGLL_MBOX_OK;
}

sgll_mbox_result_t sgll_mbox_tx_acquire(const sgll_mbox_config_t *config)
{
    if (!sgll_mbox_config_valid(config))
    {
        return SGLL_MBOX_INVALID;
    }
    invalidate(config->base + config->state_offset, sizeof(uint32_t));
    if (*state_word(config) != config->empty_state)
    {
        return SGLL_MBOX_BUSY;
    }
    invalidate(config->base, config->header_size);
    return SGLL_MBOX_OK;
}

sgll_mbox_result_t sgll_mbox_tx_publish(const sgll_mbox_config_t *config, size_t length)
{
    if (!sgll_mbox_config_valid(config) || length > config->size - config->header_size)
    {
        return SGLL_MBOX_INVALID;
    }
    if (*state_word(config) != config->empty_state)
    {
        return SGLL_MBOX_BUSY;
    }
    sgll_csr_fence_io();
    clean(config->base + config->header_size, length);
    clean(config->base, config->header_size);
    publish_state(config, config->ready_state);
    return SGLL_MBOX_OK;
}
