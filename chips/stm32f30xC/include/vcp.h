#ifndef VCP_H
#define VCP_H

// #include "serial.h"
#include "gpio.h"

extern "C" {
// #include "stm32f4xx_conf.h"
// #include "usbd_cdc_core.h"
// #include "usb_conf.h"
// #include "usbd_desc.h"
// #include "usbd_cdc_vcp.h"
// #include "usbd_usr.h"
// #include "usbd_ioreq.h"

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


  uint32_t tx_bytes_free();
  bool set_baud_rate(uint32_t baud);
  bool set_mode(uint8_t mode_);
  void put_byte(uint8_t ch);
  bool flush();
  void begin_write();
  void end_write();
  void register_rx_callback(void (*rx_callback_ptr)(uint8_t data));
  bool in_bulk_mode();

private:

  void send_disconnect_signal();

  void (*rx_callback_)(uint8_t data);

  uint8_t bulk_mode_buffer[64];
  uint8_t bulk_mode_buffer_index;
  bool bulk_mode;

  GPIO rx_pin_;
  GPIO tx_pin_;
};

#endif // VCP_H
