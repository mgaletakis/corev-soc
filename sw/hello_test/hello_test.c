// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "ibex_soc.h"
#include "timer.h"
#include "gpio.h"
#include "pwm.h"
#include <stdbool.h>


#define USE_GPIO_SHIFT_REG 1

void test_uart_irq_handler(void) __attribute__((interrupt));

void test_uart_irq_handler(void) {
  int uart_in_char;

  while ((uart_in_char = uart_in(DEFAULT_UART)) != -1) {
    uart_out(DEFAULT_UART, uart_in_char);
    uart_out(DEFAULT_UART, '\r');
    uart_out(DEFAULT_UART, '\n');
  }
}

int main(void) {
  install_exception_handler(UART_IRQ_NUM, &test_uart_irq_handler);
  uart_enable_rx_int();

  // This indicates how often the timer gets updated.
  timer_init();
  timer_enable(5000000);

  uint64_t last_elapsed_time = get_elapsed_time();

  // Reset green LEDs to off
  set_outputs(GPIO_OUT, 0x0);

  // PWM variables
  uint32_t counter = UINT8_MAX;
  uint32_t brightness = 0;
  bool ascending = true;
  // The three least significant bits correspond to RGB, where B is the leas significant.
  uint8_t color = 7;

  puts("Hello World! \n");
  set_outputs(GPIO_OUT, 0x0F);
  sim_halt();

  while(1) {
    uint64_t cur_time = get_elapsed_time();

    if (cur_time != last_elapsed_time) {
      last_elapsed_time = cur_time;

      // Disable interrupts whilst outputting to prevent output for RX IRQ
      // happening in the middle
      set_global_interrupt_enable(0);

      // Print this to UART (use the screen command to see it).
      puts("Hello World! ");
      puthex(last_elapsed_time);
      puts("   Input Value: ");
      uint32_t in_val = read_gpio(GPIO_IN_DBNC);
      puthex(in_val);
      putchar('\n');

      // Re-enable interrupts with output complete
      set_global_interrupt_enable(1);

      // Cycling through green LEDs when BTN0 is pressed
      if (USE_GPIO_SHIFT_REG) {
        set_outputs(GPIO_OUT_SHIFT, in_val);
      } else {
        uint32_t out_val = read_gpio(GPIO_OUT);
        out_val = ((out_val << 1) & GPIO_OUT_MASK) | (in_val & 0x1);
        set_outputs(GPIO_OUT, out_val);
      }

      // Going from bright to dim on PWM
      for(int i = 0; i < NUM_PWM_MODULES; i++) {
        set_pwm(PWM_FROM_ADDR_AND_INDEX(PWM_BASE, i),
            ((1 << (i%3)) & color) ? counter : 0,
            brightness ? 1 << (brightness - 1) : 0);
      }
      if (ascending) {
        brightness++;
        if (brightness >= 5) {
          ascending = false;
        }
      } else {
        brightness--;
        // When LEDs are off cycle through the colors
        if (brightness == 0) {
          ascending = true;
          color++;
          if (color >= 8) {
            color = 1;
          }
        }
      }
    }

    asm volatile ("wfi");
  }
}



#if 0
int main(int argc, char **argv) {
  pcount_enable(0);
  pcount_reset();
  pcount_enable(1);

  puts("Hello World!!!!!\n");
  puthex(0xDEADBEEF);
  putchar('\n');
  puthex(0xBAADF00D);
  putchar('\n');

  pcount_enable(0);

  // Enable periodic timer interrupt
  // (the actual timebase is a bit meaningless in simulation)
  timer_enable(2000);

  uint64_t last_elapsed_time = get_elapsed_time();

  while (last_elapsed_time <= 4) {
    uint64_t cur_time = get_elapsed_time();
    if (cur_time != last_elapsed_time) {
      last_elapsed_time = cur_time;

      if (last_elapsed_time & 1) {
        puts("Tick!\n");
      } else {
        puts("Tock!\n");
      }
    }
    asm volatile("wfi");
  }

  return 0;
}
#endif