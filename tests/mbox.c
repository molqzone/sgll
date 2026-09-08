/**
 * @file mbox.c
 * @brief 共享内存邮箱的所有权和边界回归。
 *        Shared-memory mailbox ownership and bounds regressions.
 */
#include "sgll.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    uint32_t tag;
    uint32_t state;
    uint32_t length;
    uint32_t words[29];
} test_header_t;

typedef struct
{
    alignas(64) test_header_t header;
    uint8_t data[128];
} test_slot_t;

static_assert(sizeof(test_header_t) == 128U);
static_assert(sizeof(test_slot_t) == 256U);

static sgll_mbox_config_t layout(test_slot_t *slot)
{
    const sgll_mbox_config_t config = {
        .base = (uintptr_t)slot,
        .size = sizeof(*slot),
        .header_size = sizeof(slot->header),
        .state_offset = offsetof(test_header_t, state),
        .empty_state = 7U,
        .ready_state = 11U,
    };
    return config;
}

static void round_trip(void)
{
    test_slot_t slot;
    memset(&slot, 0xA5, sizeof(slot));
    const sgll_mbox_config_t config = layout(&slot);
    slot.header.state = config.empty_state;
    assert(sgll_mbox_config_valid(&config));

    test_header_t snapshot;
    memset(&snapshot, 0xCC, sizeof(snapshot));
    const test_header_t untouched = snapshot;
    const uint8_t *payload = nullptr;
    assert(sgll_mbox_rx_acquire(&config, &snapshot, sizeof(snapshot)) == SGLL_MBOX_BUSY);
    assert(memcmp(&snapshot, &untouched, sizeof(snapshot)) == 0);
    assert(sgll_mbox_rx_payload(&config, 1U, &payload) == SGLL_MBOX_BUSY);
    assert(payload == nullptr);
    assert(sgll_mbox_rx_release(&config) == SGLL_MBOX_BUSY);

    assert(sgll_mbox_tx_acquire(&config) == SGLL_MBOX_OK);
    slot.header.tag = 0x12345678U;
    slot.header.length = sizeof(slot.data);
    for (size_t i = 0; i < sizeof(slot.data); ++i)
    {
        slot.data[i] = (uint8_t)i;
    }
    assert(sgll_mbox_tx_publish(&config, sizeof(slot.data)) == SGLL_MBOX_OK);
    assert(slot.header.state == config.ready_state);
    assert(sgll_mbox_tx_acquire(&config) == SGLL_MBOX_BUSY);
    assert(sgll_mbox_tx_publish(&config, 0U) == SGLL_MBOX_BUSY);
    assert(sgll_mbox_rx_acquire(&config, &snapshot, sizeof(snapshot)) == SGLL_MBOX_OK);
    assert(snapshot.tag == 0x12345678U && snapshot.state == config.ready_state);
    assert(snapshot.length == sizeof(slot.data));
    assert(sgll_mbox_rx_payload(&config, snapshot.length, &payload) == SGLL_MBOX_OK);
    assert(payload == slot.data);
    for (size_t i = 0; i < sizeof(slot.data); ++i)
    {
        assert(payload[i] == (uint8_t)i);
    }

    const test_slot_t before_release = slot;
    assert(sgll_mbox_rx_release(&config) == SGLL_MBOX_OK);
    assert(slot.header.state == config.empty_state);
    slot.header.state = config.ready_state;
    assert(memcmp(&slot, &before_release, sizeof(slot)) == 0);
    slot.header.state = config.empty_state;
    assert(snapshot.state == config.ready_state);

    assert(sgll_mbox_tx_acquire(&config) == SGLL_MBOX_OK);
    assert(sgll_mbox_tx_publish(&config, 0U) == SGLL_MBOX_OK);
    assert(sgll_mbox_rx_acquire(&config, &snapshot, sizeof(snapshot)) == SGLL_MBOX_OK);
    assert(sgll_mbox_rx_payload(&config, 0U, &payload) == SGLL_MBOX_OK);
    assert(sgll_mbox_rx_release(&config) == SGLL_MBOX_OK);

    slot.header.state = 99U;
    assert(sgll_mbox_rx_acquire(&config, &snapshot, sizeof(snapshot)) == SGLL_MBOX_BUSY);
    assert(sgll_mbox_rx_release(&config) == SGLL_MBOX_BUSY);
    assert(sgll_mbox_tx_acquire(&config) == SGLL_MBOX_BUSY);
    assert(sgll_mbox_tx_publish(&config, 0U) == SGLL_MBOX_BUSY);
    assert(slot.header.state == 99U);
}

static void invalid_arguments(void)
{
    test_slot_t slot;
    memset(&slot, 0x5A, sizeof(slot));
    sgll_mbox_config_t config = layout(&slot);
    slot.header.state = config.ready_state;
    const test_slot_t before = slot;
    test_header_t snapshot;
    memset(&snapshot, 0xCC, sizeof(snapshot));
    const test_header_t untouched = snapshot;
    const uint8_t *payload = nullptr;

    assert(!sgll_mbox_config_valid(nullptr));
    assert(sgll_mbox_rx_acquire(nullptr, &snapshot, sizeof(snapshot)) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_rx_payload(nullptr, 0U, &payload) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_rx_release(nullptr) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_tx_acquire(nullptr) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_tx_publish(nullptr, 0U) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_rx_acquire(&config, nullptr, sizeof(snapshot)) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_rx_acquire(&config, &snapshot, sizeof(snapshot) - 1U) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_rx_acquire(&config, &slot.header, sizeof(snapshot)) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_rx_payload(&config, 1U, nullptr) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_rx_payload(&config, sizeof(slot.data) + 1U, &payload) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_rx_payload(&config, SIZE_MAX, &payload) == SGLL_MBOX_INVALID);
    assert(sgll_mbox_rx_payload(&config, 1U, (const uint8_t **)(void *)&slot.header) == SGLL_MBOX_INVALID);
    assert(payload == nullptr);
    assert(memcmp(&snapshot, &untouched, sizeof(snapshot)) == 0);
    assert(memcmp(&slot, &before, sizeof(slot)) == 0);

    for (unsigned failure = 0; failure < 10U; ++failure)
    {
        config = layout(&slot);
        switch (failure)
        {
        case 0:
            config.base = 0U;
            break;
        case 1:
            ++config.base;
            break;
        case 2:
            config.size = sizeof(slot.header) - 64U;
            break;
        case 3:
            --config.size;
            break;
        case 4:
            config.header_size = 0U;
            break;
        case 5:
            --config.header_size;
            break;
        case 6:
            config.state_offset = sizeof(slot.header);
            break;
        case 7:
            ++config.state_offset;
            break;
        case 8:
            config.ready_state = config.empty_state;
            break;
        case 9:
            config.base = UINTPTR_MAX & ~(uintptr_t)63U;
            break;
        }
        assert(!sgll_mbox_config_valid(&config));
        assert(sgll_mbox_rx_acquire(&config, &snapshot, sizeof(snapshot)) == SGLL_MBOX_INVALID);
        assert(sgll_mbox_rx_payload(&config, 1U, &payload) == SGLL_MBOX_INVALID);
        assert(sgll_mbox_rx_release(&config) == SGLL_MBOX_INVALID);
        assert(sgll_mbox_tx_acquire(&config) == SGLL_MBOX_INVALID);
        assert(sgll_mbox_tx_publish(&config, 1U) == SGLL_MBOX_INVALID);
    }
    assert(memcmp(&slot, &before, sizeof(slot)) == 0);

    config = layout(&slot);
    slot.header.state = config.empty_state;
    assert(sgll_mbox_tx_publish(&config, sizeof(slot.data) + 1U) == SGLL_MBOX_INVALID);
    assert(slot.header.state == config.empty_state);
}

int main(void)
{
    round_trip();
    invalid_arguments();
    puts("mailbox ownership, snapshots, bounds and rejected-input preservation passed");
    return 0;
}
