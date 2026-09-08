/**
 * @file cache_linux.c
 * @brief QEMU 下的 C906 缓存操作数与邮箱发布顺序回归。
 *        C906 cache-operand and mailbox publication-order regression under QEMU.
 *
 * @note SIGILL 记录缓存指令参数；不接触硬件或模拟缓存，指令屏障编码单独检查。
 *       SIGILL records cache operands without hardware access or cache emulation; instruction-fence encoding
 * is checked separately.
 */
#define _GNU_SOURCE
#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>

void sgll_test_clean_fixed(void);
void sgll_test_invalidate_fixed(void);
void sgll_test_flush_fixed(void);
uintptr_t sgll_test_clean_dynamic(uintptr_t cookie, uintptr_t address, size_t size);
void sgll_test_instruction_fence(void);
void sgll_test_io_fence(void);
int sgll_test_mbox_tx_acquire(uintptr_t base);
int sgll_test_mbox_publish(uintptr_t base, size_t length);
int sgll_test_mbox_rx_acquire(uintptr_t base, void *header);
int sgll_test_mbox_payload(uintptr_t base, size_t length);
int sgll_test_mbox_release(uintptr_t base);

static volatile sig_atomic_t event_count;
static const volatile uint32_t *watched_state;
static volatile struct
{
    uint32_t opcode;
    uintptr_t address;
    uint32_t mailbox_state;
} events[16];

static void record_cache_instruction(int signal, siginfo_t *info, void *context)
{
    (void)info;
    ucontext_t *state = context;
    const uintptr_t pc = state->uc_mcontext.__gregs[REG_PC];
    const uint32_t opcode = *(const uint32_t *)pc;
    if (event_count >= 16 ||
        (opcode != 0x0295000BU && opcode != 0x02A5000BU && opcode != 0x02B5000BU && opcode != 0x0190000BU))
    {
        _exit(128 + signal);
    }
    events[event_count].opcode = opcode;
    events[event_count].address = state->uc_mcontext.__gregs[REG_A0];
    events[event_count].mailbox_state = watched_state == NULL ? 0U : *watched_state;
    ++event_count;
    state->uc_mcontext.__gregs[REG_PC] = pc + 4U;
}

static void check_events(uint32_t opcode, uintptr_t start, unsigned lines)
{
    assert(event_count == (sig_atomic_t)(lines + 1U));
    for (unsigned i = 0; i < lines; ++i)
    {
        assert(events[i].opcode == opcode);
        if (events[i].address != start + 64U * i)
        {
            fprintf(
                stderr, "cache operand %u: got 0x%lx, expected 0x%lx\n", i, (unsigned long)events[i].address,
                (unsigned long)(start + 64U * i)
            );
            _exit(1);
        }
    }
    assert(events[lines].opcode == 0x0190000BU);
    event_count = 0;
}

static void check_mailbox_acquire(uintptr_t base)
{
    assert(event_count == 5);
    assert(events[0].opcode == 0x02A5000BU && events[0].address == base + 64U);
    assert(events[1].opcode == 0x0190000BU);
    assert(events[2].opcode == 0x02A5000BU && events[2].address == base);
    assert(events[3].opcode == 0x02A5000BU && events[3].address == base + 64U);
    assert(events[4].opcode == 0x0190000BU);
    event_count = 0;
}

static void check_mailbox(void)
{
    alignas(64) uint32_t slot[64] = {0};
    uint8_t header[128];
    const uintptr_t base = (uintptr_t)slot;
    watched_state = &slot[17];
    slot[17] = 7U;
    assert(sgll_test_mbox_tx_acquire(base));
    check_mailbox_acquire(base);
    assert(sgll_test_mbox_publish(base, 65U));
    assert(event_count == 8);
    assert(events[0].opcode == 0x0295000BU && events[0].address == base + 128U);
    assert(events[1].opcode == 0x0295000BU && events[1].address == base + 192U);
    assert(events[2].opcode == 0x0190000BU);
    assert(events[3].opcode == 0x0295000BU && events[3].address == base);
    assert(events[4].opcode == 0x0295000BU && events[4].address == base + 64U);
    assert(events[5].opcode == 0x0190000BU);
    assert(events[6].opcode == 0x02B5000BU && events[6].address == base + 64U);
    assert(events[7].opcode == 0x0190000BU);
    for (unsigned i = 0; i < 8U; ++i)
    {
        assert(events[i].mailbox_state == (i < 6U ? 7U : 11U));
    }
    event_count = 0;
    assert(sgll_test_mbox_rx_acquire(base, header));
    check_mailbox_acquire(base);
    assert(sgll_test_mbox_payload(base, 65U));
    check_events(0x02A5000BU, base + 128U, 2U);
    assert(sgll_test_mbox_release(base));
    assert(events[0].mailbox_state == 7U && events[1].mailbox_state == 7U);
    check_events(0x02B5000BU, base + 64U, 1U);
    watched_state = NULL;
}

int main(void)
{
    struct sigaction action = {.sa_sigaction = record_cache_instruction, .sa_flags = SA_SIGINFO};
    sigemptyset(&action.sa_mask);
    assert(sigaction(SIGILL, &action, NULL) == 0);

    sgll_test_clean_fixed();
    check_events(0x0295000BU, 0x88001000UL, 2U);
    sgll_test_invalidate_fixed();
    check_events(0x02A5000BU, 0x88002000UL, 3U);
    sgll_test_flush_fixed();
    check_events(0x02B5000BU, 0x88003000UL, 1U);
    assert(sgll_test_clean_dynamic(0x12345678UL, 0x88001001UL, 65U) == 0x12345678UL);
    check_events(0x0295000BU, 0x88001000UL, 2U);
    sgll_test_instruction_fence();
    sgll_test_io_fence();
    assert(event_count == 0);
    check_mailbox();
    puts("C906 cache operands, completion barriers and mailbox publication order passed");
    return 0;
}
