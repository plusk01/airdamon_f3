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

#ifndef RC_SBUS_H
#define RC_SBUS_H

#include "rc.h"
#include "uart.h"

namespace airdamon { namespace sensors {

  class RC_SBUS : public RC
  {
  public:
    RC_SBUS();

    void init(UART* uart);

    // read the raw RC value of a channel
    float read(uint8_t channel) override;

    // Is communication with the RC transmitter considered lost?
    bool lost() override;

    // How many malformed packets were received?
    uint32_t num_errors() const { return errors_; }

  private:
    // How many ms can there be without an RC signal before going into failsafe
    static constexpr int ALLOWABLE_RC_DELAY_MS = 100;

    // The UART that is connected to the SBUS RC RX
    UART* uart_;

    uint8_t buffer_[25];  // buffer for the SBUS frame
    uint8_t prev_byte_;   // the last byte we received from the UART
    uint8_t buffer_pos_;

    bool frame_started_;  // Have we seen an SBUS start byte?

    uint32_t frame_start_ms_;   // The time (in ms) of the start of the last frame
    uint32_t errors_;           // how many garbled frames have we received?
    uint32_t raw_[18] = { 0 };  // raw RC values to be read for each channel

    // SBUS protocol magic bytes
    enum { END_BYTE = 0x00, START_BYTE = 0x0F };

    // What is the state of SBUS connectivity?
    enum class Status { OK, LOST, FAILSAFE };

    Status status_;

    struct sbusFrame_s {
      uint8_t startByte;
      // 176 bits of data (11 bits per channel * 16 channels) = 22 bytes.
      unsigned int chan0 : 11;
      unsigned int chan1 : 11;
      unsigned int chan2 : 11;
      unsigned int chan3 : 11;
      unsigned int chan4 : 11;
      unsigned int chan5 : 11;
      unsigned int chan6 : 11;
      unsigned int chan7 : 11;
      unsigned int chan8 : 11;
      unsigned int chan9 : 11;
      unsigned int chan10 : 11;
      unsigned int chan11 : 11;
      unsigned int chan12 : 11;
      unsigned int chan13 : 11;
      unsigned int chan14 : 11;
      unsigned int chan15 : 11;
      uint8_t flags;
      /**
       * The endByte is 0x00 on FrSky and some futaba RX's, on Some SBUS2 RX's the value indicates the telemetry byte that is sent after every 4th sbus frame.
       *
       * See https://github.com/cleanflight/cleanflight/issues/590#issuecomment-101027349
       * and
       * https://github.com/cleanflight/cleanflight/issues/590#issuecomment-101706023
       */
      uint8_t endByte;
    } __attribute__ ((__packed__));

    typedef union {
        uint8_t data[25];
        struct sbusFrame_s frame;
    } sbusFrame_t;

    sbusFrame_t sbus_frame_;

    // Callback that is fired whenever there is a byte to read from the UART
    void handle_byte_cb(uint8_t byte);

    // Once a full SBUS frame is received, decode the buffer for later
    void decode_buffer();
  };
  
}}

#endif