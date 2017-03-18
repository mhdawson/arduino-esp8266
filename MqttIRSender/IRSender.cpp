// Copyright 2014-2017 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.
#include <string.h>
#include "IRSender.h"

IRSender::IRSender(int pin) {
  _pin = pin;
  pinMode(pin, OUTPUT);
}


/* Send one pulse from the ir code.
 * Modulate the pin output at ~38khz for
 * the length of the pulse.
 */
void IRSender::sendPulse(int length) {
  while(length > 0) {
    digitalWrite(_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_pin, LOW);
    delayMicroseconds(10);
    // adjust to get timing for specific arduino
    // this is for a node mcu D1
    length = length - 24;
  }
}


/* Send an ir code. The ir code
 * For a mark the pin is modulated at ~38khz for
 * the length of the mark by sendPulse.
 * For a space we simply  set the pin low.
 * We pass the ir code as a string of alternating space and
 * mark lengths, separated by commas
 */
void IRSender::sendIrCode(char* code) {
  cli();
  boolean mark = true;
  char* next = strtok(code,",");
  while(true) {
    if (nullptr != next) {
      int length = atoi(next);
      if (mark) {
        sendPulse(length);
        mark = false;
      } else {
        delayMicroseconds(length);
        mark = true;
      }
      next = strtok(nullptr,",");
      continue;
    }
    break;
  }
  sei();
}
