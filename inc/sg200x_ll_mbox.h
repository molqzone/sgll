/**
 * @file sg200x_ll_mbox.h
 * @brief C906L 共享 DDR 邮箱传输接口。
 *        C906L shared-DDR mailbox transport interfaces.
 * @ingroup SGLL_MBOX
 *
 * @note 单生产者与单消费者通过一个消息槽通信；empty 归生产者，ready 归消费者，头部格式和状态值由调用者定义。
 *       One producer and one consumer exchange a single slot; empty belongs to the producer and ready to the
 * consumer, with caller-defined header format and state values.
 * @note 双方必须遵守所有权协议；另一端持有槽时不得复位或覆写，交还槽前须完成相关 DMA。
 *       Both peers must honor ownership; do not reset or overwrite a peer-owned slot, and complete related
 * DMA before returning it.
 * @note 这是共享 DDR 传输，不配置 APB/RTC 硬件邮箱中断。宿主构建假定一致性内存。
 *       This is shared-DDR transport and does not configure APB/RTC hardware mailbox interrupts. Host builds
 * assume coherent memory.
 */

/**
 * @defgroup SGLL_MBOX 共享内存邮箱 / Shared-memory mailbox
 * @ingroup SGLL
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief 共享邮箱槽的布局和所有权状态编码。
 *        Layout and ownership-state encodings of a shared mailbox slot.
 * @ingroup SGLL_MBOX
 *
 * @note 基址、总大小和头部大小按 64 字节缓存行对齐；配置和输出对象位于共享区之外。
 *       Align the base, total size, and header size to 64-byte cache lines; keep configuration and output
 * objects outside the shared region.
 */
typedef struct
{
    uintptr_t base;     ///< 共享区的本地映射地址。 Locally mapped address of the shared region.
    size_t size;        ///< 包括头部在内的共享区字节数。 Shared-region bytes including the header.
    size_t header_size; ///< 头部字节数，载荷紧随其后。 Header bytes; the payload follows immediately.
    size_t state_offset; ///< 头部内按 4 字节对齐的状态字偏移。 Four-byte-aligned state-word offset within the
                         ///< header.
    uint32_t empty_state; ///< 将槽交给生产者的状态值。 State value granting the slot to the producer.
    uint32_t ready_state; ///< 将槽交给消费者的状态值。 State value granting the slot to the consumer.
} sgll_mbox_config_t;

/**
 * @brief 非阻塞邮箱操作的结果。
 *        Result of a nonblocking mailbox operation.
 * @ingroup SGLL_MBOX
 */
typedef enum
{
    SGLL_MBOX_OK,      ///< 操作已完成。 Operation completed.
    SGLL_MBOX_BUSY,    ///< 槽不处于所需状态。 Slot is not in the required state.
    SGLL_MBOX_INVALID, ///< 布局、缓冲区或长度无效。 Invalid layout, buffer, or length.
} sgll_mbox_result_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 检查共享邮箱布局而不访问共享区。
     *        Validate a shared mailbox layout without accessing the shared region.
     * @ingroup SGLL_MBOX
     *
     * @param[in] config 调用者持有的不可变邮箱布局描述。
     *        Caller-owned immutable mailbox layout description.
     * @return 对齐、大小、状态位置及地址范围均有效时返回 true。
     *         True when alignment, sizes, state location, and address bounds are valid.
     */
    bool sgll_mbox_config_valid(const sgll_mbox_config_t *config);

    /**
     * @brief 取得就绪槽并把头部快照复制到本地。
     *        Acquire a ready slot and copy its header snapshot locally.
     * @ingroup SGLL_MBOX
     *
     * @param[in] config 调用者持有的不可变邮箱布局描述。
     *        Caller-owned immutable mailbox layout description.
     * @param[out] header 接收头部快照的本地缓冲区，失败时不变。
     *        Local destination for the header snapshot; unchanged on failure.
     * @param capacity 本地缓冲区容量，至少为 header_size 字节。
     *        Local buffer capacity, at least header_size bytes.
     * @note 成功后槽保持 ready，直至 rx_release；只有一个消费者，不分配或保存接收句柄。
     *       The slot remains ready until rx_release; there is one consumer and no receive handle is allocated
     * or retained.
     * @note 先刷新并观察状态字，再刷新整个头部，避免将发布前缓存的行复制进快照。
     *       Refresh and observe the state word before refreshing the complete header, so lines cached before
     * publication cannot enter the snapshot.
     * @retval SGLL_MBOX_OK 操作完成。
     *         Operation completed.
     * @retval SGLL_MBOX_BUSY 槽不处于要求的所有权状态。
     *         The slot is not in the required ownership state.
     * @retval SGLL_MBOX_INVALID 布局、输出缓冲区或长度不合法。
     *         Invalid layout, output buffer, or length.
     */
    sgll_mbox_result_t sgll_mbox_rx_acquire(const sgll_mbox_config_t *config, void *header, size_t capacity);

    /**
     * @brief 同步已取得槽的载荷并返回只读视图。
     *        Synchronize an acquired slot's payload and return a read-only view.
     * @ingroup SGLL_MBOX
     *
     * @param[in] config 调用者持有的不可变邮箱布局描述。
     *        Caller-owned immutable mailbox layout description.
     * @param length 从已取得的本地头部快照读取的载荷长度。
     *        Payload length read from the acquired local header snapshot.
     * @param[out] payload 接收载荷地址的本地变量，失败时不变。
     *        Local output for the payload address; unchanged on failure.
     * @note 允许零长度消息；载荷视图在释放槽之前有效。
     *       Zero-length messages are allowed; the payload view remains valid until release.
     * @retval SGLL_MBOX_OK 操作完成。
     *         Operation completed.
     * @retval SGLL_MBOX_BUSY 槽不处于要求的所有权状态。
     *         The slot is not in the required ownership state.
     * @retval SGLL_MBOX_INVALID 布局、输出缓冲区或长度不合法。
     *         Invalid layout, output buffer, or length.
     */
    sgll_mbox_result_t
    sgll_mbox_rx_payload(const sgll_mbox_config_t *config, size_t length, const uint8_t **payload);

    /**
     * @brief 在所有读取完成后把消息槽交还生产者。
     *        Return the message slot to the producer after all reads complete.
     * @ingroup SGLL_MBOX
     *
     * @param[in] config 调用者持有的不可变邮箱布局描述。
     *        Caller-owned immutable mailbox layout description.
     * @note 只修改状态字，并只回写该状态字所在缓存行；相关 DMA 读取须已完成。
     *       Only the state word changes and only its cache line is written back; related DMA reads must
     * already be complete.
     * @retval SGLL_MBOX_OK 操作完成。
     *         Operation completed.
     * @retval SGLL_MBOX_BUSY 槽不处于要求的所有权状态。
     *         The slot is not in the required ownership state.
     * @retval SGLL_MBOX_INVALID 布局、输出缓冲区或长度不合法。
     *         Invalid layout, output buffer, or length.
     */
    sgll_mbox_result_t sgll_mbox_rx_release(const sgll_mbox_config_t *config);

    /**
     * @brief 取得空槽以准备就地写入消息。
     *        Acquire an empty slot to prepare a message in place.
     * @ingroup SGLL_MBOX
     *
     * @param[in] config 调用者持有的不可变邮箱布局描述。
     *        Caller-owned immutable mailbox layout description.
     * @note 只有一个生产者；保持状态字为 empty，填写头部和载荷后调用 tx_publish。
     *       There is one producer; keep the state word empty while filling the header and payload, then call
     * tx_publish.
     * @retval SGLL_MBOX_OK 操作完成。
     *         Operation completed.
     * @retval SGLL_MBOX_BUSY 槽不处于要求的所有权状态。
     *         The slot is not in the required ownership state.
     * @retval SGLL_MBOX_INVALID 布局、输出缓冲区或长度不合法。
     *         Invalid layout, output buffer, or length.
     */
    sgll_mbox_result_t sgll_mbox_tx_acquire(const sgll_mbox_config_t *config);

    /**
     * @brief 写回头部和载荷后发布就绪状态。
     *        Publish ready state after cleaning the header and payload.
     * @ingroup SGLL_MBOX
     *
     * @param[in] config 调用者持有的不可变邮箱布局描述。
     *        Caller-owned immutable mailbox layout description.
     * @param length 生产者已写入的载荷字节数。
     *        Number of payload bytes written by the producer.
     * @note 仅在消息写回完成后设置 ready；发布后生产者不得修改该槽。
     *       Ready is set only after message writeback completes; the producer must not modify the slot after
     * publication.
     * @retval SGLL_MBOX_OK 操作完成。
     *         Operation completed.
     * @retval SGLL_MBOX_BUSY 槽不处于要求的所有权状态。
     *         The slot is not in the required ownership state.
     * @retval SGLL_MBOX_INVALID 布局、输出缓冲区或长度不合法。
     *         Invalid layout, output buffer, or length.
     */
    sgll_mbox_result_t sgll_mbox_tx_publish(const sgll_mbox_config_t *config, size_t length);

#ifdef __cplusplus
}
#endif
