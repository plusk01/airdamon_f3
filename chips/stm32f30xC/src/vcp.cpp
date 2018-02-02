#include "vcp.h"

#define USB_TIMEOUT  5

VCP::VCP()
{
  // Initialize the GPIOs for the pins
  rx_pin_.init(GPIOA, GPIO_Pin_11, GPIO::PERIPH_IN_OUT);
  tx_pin_.init(GPIOA, GPIO_Pin_12, GPIO::PERIPH_IN_OUT);

  // resets the connection for the host
  send_disconnect_signal();

  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
}

void VCP::write(uint8_t* ch, uint8_t len)
{
  if (!usbIsConnected() || !usbIsConfigured()) return;

  printf("about to send bytes...");
  uint32_t start = millis();
  while (len > 0)
  {
    uint32_t num_bytes_sent = CDC_Send_DATA(ch, len);
    len -= num_bytes_sent;
    ch += num_bytes_sent;

    printf("Sent %d bytes\n", num_bytes_sent);

    if (millis() > start + USB_TIMEOUT)
      break;
  }
  printf("sent!\n");
}


uint32_t VCP::rx_bytes_waiting()
{
  return CDC_Receive_BytesAvailable();
}


uint32_t VCP::tx_bytes_free()
{
  return CDC_Send_FreeBytes();
}


uint8_t VCP::read_byte()
{
  uint8_t data;

  if (CDC_Receive_DATA(&data, 1))
    return data;
  else
    return 0;
}


bool VCP::set_baud_rate(uint32_t baud){}

bool VCP::tx_buffer_empty()
{
  return CDC_Send_FreeBytes() > 0;
}

bool VCP::set_mode(uint8_t mode)
{
  (void)mode;
}

void VCP::put_byte(uint8_t ch)
{
  CDC_Send_DATA(&ch, 1);
}

bool VCP::flush()
{
  // CDC_flush();
  // return false;
}
void VCP::begin_write(){}
void VCP::end_write(){}


void VCP::register_rx_callback(void (*rx_callback_ptr)(uint8_t data))
{
  // rx_callback_ = rx_callback_ptr;
  // Register_CDC_RxCallback(rx_callback_ptr);
}


bool VCP::in_bulk_mode()
{
  return false;
}


void VCP::send_disconnect_signal()
{
  tx_pin_.set_mode(GPIO::OUTPUT);
  tx_pin_.write(GPIO::LOW);
  delay(200);
  tx_pin_.write(GPIO::HIGH);
  tx_pin_.set_mode(GPIO::PERIPH_IN_OUT);
}
