/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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
                // uint16_t level = ((uint32_t)value * PWM_WRAP) / 65535u;
                uint16_t level = value >> 3;

                pwm_set_chan_level(fanPwmSlices[1], fanPwmChannels[1], level);
                // printf("Number of steps set to: %lu\n", fanRange);
                // printf("Clock set: %s\n", okay ? "yes" : "no");

                have_high = false;
            }
        }

        tight_loop_contents();
    }
}



// #include <stdio.h>
// #include <inttypes.h>

// #include "pico/stdlib.h"
// #include "hardware/clocks.h"

// #define FAN_PWM_HZ      20600u
// #define PWM_STEPS       16384u
// #define PWM_TOP         (PWM_STEPS - 1u)

// #define START_KHZ       409600u
// #define END_KHZ         500000u
// #define STEP_KHZ        1u

// int main() {
//     stdio_init_all();
//     sleep_ms(2000);

//     printf("\nFinding first accepted clock...\n");

//     bool found = false;

//     uint32_t accepted_khz = 0;
//     uint32_t accepted_vco = 0;
//     uint32_t accepted_postdiv1 = 0;
//     uint32_t accepted_postdiv2 = 0;
//     float accepted_pwm_div = 0.0f;

//     for (uint32_t khz = START_KHZ; khz <= END_KHZ; khz += STEP_KHZ) {
//         uint vco;
//         uint postdiv1;
//         uint postdiv2;

//         bool ok = check_sys_clock_khz(khz, &vco, &postdiv1, &postdiv2);

//         if (!ok) {
//             continue;
//         }

//         float pwm_div =
//             ((float)khz * 1000.0f) /
//             ((float)FAN_PWM_HZ * (float)PWM_STEPS);

//         printf(
//             "ACCEPTED: %" PRIu32 " kHz  "
//             "clkdiv=%.8f  "
//             "VCO=%u kHz  PD1=%u  PD2=%u\n",
//             khz,
//             pwm_div,
//             vco,
//             postdiv1,
//             postdiv2
//         );

//         accepted_khz = khz;
//         accepted_vco = vco;
//         accepted_postdiv1 = postdiv1;
//         accepted_postdiv2 = postdiv2;
//         accepted_pwm_div = pwm_div;
//         found = true;

//         break;  // stop at the first accepted clock
//     }

//     printf("\nScan complete.\n");

//     while (true) {
//         if (found) {
//             printf(
//                 "FIRST ACCEPTED: %" PRIu32 " kHz  "
//                 "PWM wrap=%u  "
//                 "PWM steps=%u  "
//                 "clkdiv=%.8f  "
//                 "VCO=%" PRIu32 " kHz  "
//                 "PD1=%" PRIu32 "  "
//                 "PD2=%" PRIu32 "\n",
//                 accepted_khz,
//                 PWM_TOP,
//                 PWM_STEPS,
//                 accepted_pwm_div,
//                 accepted_vco,
//                 accepted_postdiv1,
//                 accepted_postdiv2
//             );
//         } else {
//             printf(
//                 "NO ACCEPTED CLOCK FOUND from %" PRIu32 " kHz to %" PRIu32 " kHz\n",
//                 (uint32_t)START_KHZ,
//                 (uint32_t)END_KHZ
//             );
//         }

//         sleep_ms(1000);
//     }
// }

// #include <stdio.h>
// #include <inttypes.h>

// #include "pico/stdlib.h"
// #include "hardware/vreg.h"
// #include "hardware/clocks.h"

// #define FAN_PWM_HZ      25000u
// #define PWM_STEPS       16384u
// #define PWM_TOP         (PWM_STEPS - 1u)

// #define START_KHZ       204800u
// #define END_KHZ         500000u
// #define STEP_KHZ        1u

// #define TEST_HOLD_MS    250u

// int main() {
//     stdio_init_all();

//     // Give USB serial plenty of time to enumerate.
//     sleep_ms(15000);

//     printf("\nRP2350 clock acceptance + live set test\n");
//     printf("Waiting done. Setting VREG to 1.30 V...\n");

//     // vreg_set_voltage(VREG_VOLTAGE_1_30);
//     sleep_ms(50);

//     printf("VREG set to 1.30 V\n");
//     printf("Target PWM: %u Hz\n", FAN_PWM_HZ);
//     printf("PWM steps: %u\n", PWM_STEPS);
//     printf("PWM wrap: %u\n", PWM_TOP);
//     printf("Scanning %" PRIu32 " kHz to %" PRIu32 " kHz, step %" PRIu32 " kHz\n\n",
//            (uint32_t)START_KHZ,
//            (uint32_t)END_KHZ,
//            (uint32_t)STEP_KHZ);

//     uint32_t last_set_khz = 0;
//     uint32_t last_actual_hz = 0;
//     uint32_t accepted_count = 0;

//     for (uint32_t khz = START_KHZ; khz <= END_KHZ; khz += STEP_KHZ) {
//         uint vco;
//         uint postdiv1;
//         uint postdiv2;

//         bool accepted = check_sys_clock_khz(khz, &vco, &postdiv1, &postdiv2);

//         if (!accepted) {
//             continue;
//         }

//         float pwm_div =
//             ((float)khz * 1000.0f) /
//             ((float)FAN_PWM_HZ * (float)PWM_STEPS);

//         accepted_count++;

//         printf(
//             "\nACCEPTED #%" PRIu32 ": %" PRIu32 " kHz  "
//             "clkdiv=%.8f  "
//             "VCO=%u kHz  PD1=%u  PD2=%u\n",
//             accepted_count,
//             khz,
//             pwm_div,
//             vco,
//             postdiv1,
//             postdiv2
//         );

//         printf("Trying set_sys_clock_khz(%" PRIu32 ")...\n", khz);

//         bool set_ok = set_sys_clock_khz(khz, false);
//         uint32_t actual_hz = clock_get_hz(clk_sys);

//         printf("set_sys_clock_khz: %s\n", set_ok ? "OK" : "FAILED");
//         printf("actual clk_sys: %" PRIu32 " Hz\n", actual_hz);

//         last_set_khz = khz;
//         last_actual_hz = actual_hz;

//         if (!set_ok) {
//             printf("Set failed even though check accepted it. Continuing anyway, because apparently consistency was optional.\n");
//             sleep_ms(TEST_HOLD_MS);
//             continue;
//         }

//         printf("Holding at %" PRIu32 " kHz for %u ms...\n", khz, TEST_HOLD_MS);
//         sleep_ms(TEST_HOLD_MS);
//     }

//     printf("\nScan finished without crashing.\n");

//     while (true) {
//         printf(
//             "ALIVE: accepted_count=%" PRIu32 "  "
//             "last_set=%" PRIu32 " kHz  "
//             "actual_clk_sys=%" PRIu32 " Hz\n",
//             accepted_count,
//             last_set_khz,
//             last_actual_hz
//         );

//         sleep_ms(1000);
//     }
// }
