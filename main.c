/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2024 Derek Fountain
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/*
 * This is a cut down, hopefully simplified version of the dev_hid_composite
 * example which comes with the Raspberry Pi Pico:
 *
 * https://github.com/raspberrypi/pico-examples/tree/master/usb/device/dev_hid_composite
 *
 * I only wanted the keyboard bit - i.e. a Pico to behave like a programmable
 * USB keyboard sending characters into the PC - so I cut out all the other stuff
 * in the example (which, initially at least, I couldn't get working).
 *
 * Build and program it, then connect the Pico to a PC using a suitable USB
 * cable. The PC will power the Pico via that cable; the cable also doubles
 * up as the data cable. Open a text editor or similar on the PC, then press
 * the Pico's white BOOTSEL button. It should send a letter 'a' to the PC
 * via USB which you'll see "typed" in the text editor.
 *
 * If that much works, you've got everything set up correctly.
 */

/*
 * cmake -DCMAKE_BUILD_TYPE=Debug ..
 * make -j10
 *
 * With the RPi Debug Probe:
 * 
 * sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" \
 *              -c "program pico_usb_keyboard.elf verify reset exit"
 * sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000"
 *
 * gdb-multiarch pico_usb_keyboard.elf
 *  target remote localhost:3333
 *  load
 *  monitor reset init
 *  continue
 *
 * With the home made Pico probe:
 *
 * sudo openocd -f interface/picoprobe.cfg -f target/rp2040.cfg \
 *              -c "program ./program pico_usb_keyboard.elf verify reset exit"
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"

static void hid_task(void);

int main(void)
{
  /* Initialise USB*/
  board_init();
  tusb_init();

  while (1)
  {
    /*
     * Periodically call Tiny USB main thread controller/delegater so it
     * can queue up events that need processing, despatch callbacks, etc
     */
    tud_task();

    /*
     * Call this program's HID task handler, which means sending out
     * key "presses" as required
     */
    hid_task();
  }
}

/*
 * Local handler code, called when a key press needs to be sent
 * out to the host PC. btn is the state of the Pico's button.
 * If the button is down we send the keypress. If the button
 * is not down we send a NULL keypress to confirm there is
 * no more for now.
 */
static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

	tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }
      else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key)
	  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);

        has_keyboard_key = false;
      }
    }
    break;

    default:
    break;
  }
}

/*
 * This is the local program's HID task. It is called repeatedly, but only
 * jumps into life every approx 10ms. It samples the button, which on a
 * Pico is the white BOOTSEL button (which can be read via the flash
 * hardware). If the button has been pressed, either wake up (if we're
 * suspended) or send out the next keypress report.
 */
static void hid_task(void)
{
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }
  else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_KEYBOARD, btn);
  }
}

/*
 * This callback is invoked by TinyUSB when the last keypress has
 * been successfully sent to the host PC.
 */
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

/*
 * Don't know what this is...
 */
// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

/*
 * This is the callback which handles a report from the PC that
 * something happened at the PC and has been sent here to the Pico.
 * Capslock, numlock and one or two other odd things like compose
 * mode are supported by this mechanism.
 */
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On
        board_led_write(true);
      }
      else
      {
        // Caplocks Off
        board_led_write(false);
      }
    }
  }
}
