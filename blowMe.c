/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Note i need pwm pin 16 always own to provide power to the fan in my
// implementation, it's my variable voltage channel

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

// #define SYS_KHZ 206000u
#define SYS_KHZ 390000u
#define FAN_PWM_FREQ_HZ 25000u
// #define PWM_STEPS       16384u
#define PWM_STEPS       8192u
// #define PWM_STEPS       16384u
#define PWM_WRAP        (PWM_STEPS - 1u)

// uint32_t sys_hz = 0;
// uint32_t fanRange = 0;
// uint32_t pwm_wrap = 0;

const unsigned int fanPwmPins[]={16,17};

#define FAN_COUNT (sizeof(fanPwmPins) / sizeof(fanPwmPins[0]))
unsigned int fanPwmSlices[FAN_COUNT];
unsigned int fanPwmChannels[FAN_COUNT];

int main() {
    stdio_init_all();
    sleep_ms(2000);

    vreg_set_voltage(VREG_VOLTAGE_1_30);
    
    sleep_ms(10);

    bool okay = set_sys_clock_khz(SYS_KHZ, false);

    sleep_ms(10);

    uint32_t sys_hz = clock_get_hz(clk_sys);

    float pwm_clkdiv =
        (float)sys_hz/
        ((float)FAN_PWM_FREQ_HZ * (float)PWM_STEPS);

    for(int i=0;i<FAN_COUNT;++i){
        gpio_set_function(fanPwmPins[i], GPIO_FUNC_PWM);

        fanPwmSlices[i] = pwm_gpio_to_slice_num(fanPwmPins[i]);
        fanPwmChannels[i] = pwm_gpio_to_channel(fanPwmPins[i]);

        pwm_set_clkdiv(fanPwmSlices[i], pwm_clkdiv);
        pwm_set_wrap(fanPwmSlices[i], PWM_WRAP);

        pwm_set_chan_level(fanPwmSlices[i], fanPwmChannels[i], 0);

        pwm_set_enabled(fanPwmSlices[i], true);
    }

    pwm_set_chan_level(fanPwmSlices[0], fanPwmChannels[0], PWM_WRAP);

    static bool have_high = false;
    static uint8_t high_byte = 0;

    while (true) {
        int c;

        while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
            uint8_t b = (uint8_t)c;

            if (!have_high) {
                high_byte = b;
                have_high = true;
            } else {
                uint16_t value = ((uint16_t)high_byte << 8) | b;
                uint16_t level = value >> 3;
                pwm_set_chan_level(fanPwmSlices[1], fanPwmChannels[1], level);
                have_high = false;
            }
        }

        tight_loop_contents();
    }
}
