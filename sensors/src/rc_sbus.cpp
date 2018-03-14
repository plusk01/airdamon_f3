/*
 * Copyright (c) 2017, James Jackson, Parker Lusk
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "rc_sbus.h"

namespace sensors {

RC_SBUS::RC_SBUS() {}

// ----------------------------------------------------------------------------

void RC_SBUS::init(UART* uart)
{
  uart_ = uart;

  uart_->set_mode(100000, UART::Mode::m8E2);
  uart_->register_rx_callback(std::bind(&RC_SBUS::read_cb, this, std::placeholders::_1));

  errors_ = 0;
  frame_started_ = false;

  memset(raw_, 0, sizeof(raw_));

  // USART_InvPinCmd(USART2, USART_InvPin_Rx, ENABLE);

  info.init(GPIOB, GPIO_Pin_8);
}

// ----------------------------------------------------------------------------

float RC_SBUS::read(uint8_t channel)
{
  return (static_cast<float>(raw_[channel]) - 172.0)/1639.0;
}

// ----------------------------------------------------------------------------

bool RC_SBUS::lost()
{
  return false;
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void RC_SBUS::read_cb(uint8_t byte)
{
  // Look for the beginning of a new SBUS frame
  if (byte == START_BYTE && prev_byte_ == END_BYTE)
  {
    buffer_pos_ = 0;
    frame_started_ = true;
    frame_start_ms_ = millis();

  }

  if (frame_started_)
  {
    sbus_frame_.data[buffer_pos_++] = byte;

    // is the buffer full?
    if (buffer_pos_ == 25)
    {
      // If the buffer contains a complete frame, then decode the RC data
      if (sbus_frame_.frame.startByte == START_BYTE
            && sbus_frame_.frame.endByte == END_BYTE)
      {
        decode_buffer();
      }
      else
      {
        errors_++;
      }

      // begin looking for next frame
      frame_started_ = false;
    }
    info.toggle();
  }


  prev_byte_ = byte;
}

// ----------------------------------------------------------------------------

void RC_SBUS::decode_buffer()
{
  frame_start_ms_ = millis();

  // process actual sbus data, use union to decode
  // (Map to AETR to keep with ROSflight convention)
  raw_[0]  = sbus_frame_.frame.chan1;
  raw_[1]  = sbus_frame_.frame.chan2;
  raw_[2]  = sbus_frame_.frame.chan0;
  raw_[3]  = sbus_frame_.frame.chan3;
  raw_[4]  = sbus_frame_.frame.chan4;
  raw_[5]  = sbus_frame_.frame.chan5;
  raw_[6]  = sbus_frame_.frame.chan6;
  raw_[7]  = sbus_frame_.frame.chan7;
  raw_[8]  = sbus_frame_.frame.chan8;
  raw_[9]  = sbus_frame_.frame.chan9;
  raw_[10] = sbus_frame_.frame.chan10;
  raw_[11] = sbus_frame_.frame.chan11;
  raw_[12] = sbus_frame_.frame.chan12;
  raw_[13] = sbus_frame_.frame.chan13;
  raw_[14] = sbus_frame_.frame.chan14;
  raw_[15] = sbus_frame_.frame.chan15;

  // Digital Channel 1
  if (sbus_frame_.frame.flags & 0x0001)
    raw_[16] = 1811;
  else
    raw_[16] = 172;

  // Digital Channel 2
  if (sbus_frame_.frame.flags & 0x0010)
    raw_[17] = 1811;
  else
    raw_[17] = 172;

//   // Failsafe
//   failsafe_status_ = SBUS_SIGNAL_OK;
// //  if (sbus_frame_.frame.flags & 0x0100)
// //    failsafe_status_ = SBUS_SIGNAL_LOST;
//   if (sbus_frame_.frame.flags & 0x1000)
//     failsafe_status_ = SBUS_SIGNAL_FAILSAFE;
}

}