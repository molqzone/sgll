/**
 * @file sg200x_ll_pwm.c
 * @brief PWM 初始化、周期更新和有序启动停止。
 *        PWM initialization, period updates, and ordered start/stop.
 * @ingroup SGLL_PWM
 */
#include "sg200x_ll_pwm.h"
#include "sg200x_ll_csr.h"

/**
 * @brief 检查 PWM 实例及通道掩码。
 *        Validate a PWM instance and channel mask.
 * @param pwm PWM 寄存器实例。
 *        PWM register instance.
 * @param channels 控制器内通道掩码，仅低四位有效。
 *        Channel mask within the controller; only the low four bits are valid.
 * @return 实例和掩码均有效时返回 true。
 *        True when both the instance and mask are valid.
 */
static bool pwm_channels_valid(const PWM_Type *pwm, uint32_t channels)
{
    return (pwm == PWM0_REGS || pwm == PWM1_REGS || pwm == PWM2_REGS || pwm == PWM3_REGS) && channels != 0U &&
           (channels & ~PWM_CHANNEL_MASK) == 0U;
}

void sgll_pwm_struct_init(sgll_pwm_init_t *config)
{
    if (config != NULL)
    {
        config->period_ticks = 2U;
        config->inactive_ticks = 1U;
        config->active_high = true;
    }
}

bool sgll_pwm_period_set(PWM_Type *pwm, uint32_t channel, uint32_t period_ticks, uint32_t inactive_ticks)
{
    if (channel >= PWM_CHANNELS_PER_CONTROLLER || !pwm_channels_valid(pwm, 1U << channel) ||
        period_ticks < 2U || period_ticks > PWM_PERIOD_MAX || inactive_ticks == 0U ||
        inactive_ticks >= period_ticks)
    {
        return false;
    }
    pwm->CHANNEL[channel].HLPERIOD = inactive_ticks;
    pwm->CHANNEL[channel].PERIOD = period_ticks;
    return true;
}

bool sgll_pwm_init(PWM_Type *pwm, uint32_t channel, const sgll_pwm_init_t *config)
{
    if (config == NULL || channel >= PWM_CHANNELS_PER_CONTROLLER || !pwm_channels_valid(pwm, 1U << channel))
    {
        return false;
    }
    const uint32_t mask = 1U << channel;
    if (((pwm->PWMSTART | pwm->PWM_OE) & mask) != 0U ||
        !sgll_pwm_period_set(pwm, channel, config->period_ticks, config->inactive_ticks))
    {
        return false;
    }
    sgll_pwm_polarity_set(pwm, mask, config->active_high);
    sgll_csr_fence_io();
    return true;
}

bool sgll_pwm_update(PWM_Type *pwm, uint32_t channels)
{
    if (!pwm_channels_valid(pwm, channels))
        return false;
    sgll_csr_fence_io();
    pwm->PWMUPDATE |= channels;
    sgll_csr_fence_io();
    pwm->PWMUPDATE &= ~channels;
    sgll_csr_fence_io();
    return true;
}

bool sgll_pwm_start(PWM_Type *pwm, uint32_t channels, bool active_high)
{
    if (!pwm_channels_valid(pwm, channels))
        return false;
    sgll_pwm_polarity_set(pwm, channels, active_high);
    pwm->PWMSTART &= ~channels;
    sgll_csr_fence_io();
    pwm->PWM_OE |= channels;
    sgll_csr_fence_io();
    pwm->PWMSTART |= channels;
    sgll_csr_fence_io();
    return true;
}

bool sgll_pwm_stop(PWM_Type *pwm, uint32_t channels)
{
    if (!pwm_channels_valid(pwm, channels))
        return false;
    pwm->PWM_OE &= ~channels;
    sgll_csr_fence_io();
    pwm->PWMSTART &= ~channels;
    sgll_csr_fence_io();
    return true;
}
