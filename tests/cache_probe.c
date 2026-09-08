/**
 * @file cache_probe.c
 * @brief 使用固件工具链编译的缓存与邮箱探针入口。
 *        Cache and mailbox probe entry points compiled with the firmware toolchain.
 *
 * @note 使用实际 SDK 头；cache_linux.c 在 QEMU 捕获 C906 指令时观察 a0。
 *       Use real SDK headers; cache_linux.c observes a0 when QEMU traps C906 instructions.
 */
#include "inc/sg200x_ll_csr.h"
#include "inc/sg200x_ll_mbox.h"

static sgll_mbox_config_t mailbox_config(uintptr_t base)
{
    const sgll_mbox_config_t config = {
        .base = base,
        .size = 256U,
        .header_size = 128U,
        .state_offset = 68U,
        .empty_state = 7U,
        .ready_state = 11U,
    };
    return config;
}

#ifdef __cplusplus
extern "C"
{
#endif

    void sgll_test_clean_fixed(void) { sgll_csr_dcache_clean_range(0x88001000UL, 128U); }

    void sgll_test_invalidate_fixed(void) { sgll_csr_dcache_invalidate_range(0x88002000UL, 192U); }

    void sgll_test_flush_fixed(void) { sgll_csr_dcache_clean_invalidate_range(0x88003000UL, 64U); }

    uintptr_t sgll_test_clean_dynamic(uintptr_t cookie, uintptr_t address, size_t size)
    {
        sgll_csr_dcache_clean_range(address, size);
        return cookie;
    }

    void sgll_test_instruction_fence(void) { sgll_csr_fence_i(); }
    void sgll_test_io_fence(void) { sgll_csr_fence_io(); }

    int sgll_test_mbox_tx_acquire(uintptr_t base)
    {
        const sgll_mbox_config_t config = mailbox_config(base);
        return sgll_mbox_tx_acquire(&config) == SGLL_MBOX_OK;
    }

    int sgll_test_mbox_publish(uintptr_t base, size_t length)
    {
        const sgll_mbox_config_t config = mailbox_config(base);
        return sgll_mbox_tx_publish(&config, length) == SGLL_MBOX_OK;
    }

    int sgll_test_mbox_rx_acquire(uintptr_t base, void *header)
    {
        const sgll_mbox_config_t config = mailbox_config(base);
        return sgll_mbox_rx_acquire(&config, header, 128U) == SGLL_MBOX_OK;
    }

    int sgll_test_mbox_payload(uintptr_t base, size_t length)
    {
        const sgll_mbox_config_t config = mailbox_config(base);
        const uint8_t *payload = nullptr;
        return sgll_mbox_rx_payload(&config, length, &payload) == SGLL_MBOX_OK;
    }

    int sgll_test_mbox_release(uintptr_t base)
    {
        const sgll_mbox_config_t config = mailbox_config(base);
        return sgll_mbox_rx_release(&config) == SGLL_MBOX_OK;
    }

#ifdef __cplusplus
}
#endif
