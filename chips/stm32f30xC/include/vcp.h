#ifndef VCP_H
#define VCP_H

#include "system.h"
#include "gpio.h"

extern "C" {
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"

#include "printf.h"
}


class VCP
{
public:
  VCP();

  // Use this object for printf
  void connect_to_printf();

  //
  // Rx functions
  //

  // Get a single byte from buffer. Must call
  //  `rx_bytes_waiting()` beforehand.
  uint8_t read_byte();

  // Check to see if there are bytes to be read.
  uint32_t rx_bytes_waiting();


  //
  // Tx functions
  //

  // send a byte array through the VCP peripheral
  void write(uint8_t* c, uint8_t len);

  // Is there any data in the buffer that needs to be sent?
  bool tx_buffer_empty();

  // How many bytes are available in the buffer to be transferred?
  uint32_t tx_bytes_free();


  // USB bulk mode
  bool in_bulk_mode();

private:
  // Toggle the tx_pin so the host sees us as a new connection
  void send_disconnect_signal();

  // USB pins
  GPIO rx_pin_;
  GPIO tx_pin_;
};

#endif // VCP_H
