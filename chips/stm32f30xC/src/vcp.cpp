#include "vcp.h"

#define USB_TIMEOUT  5

// printf function for putting a single character to screen
static void _putc(void* p, char c)
{
  VCP* pVCP = static_cast<VCP*>(p);
  pVCP->write(reinterpret_cast<uint8_t*>(&c), 1);
}

// ----------------------------------------------------------------------------

VCP::VCP() {}

// ----------------------------------------------------------------------------

void VCP::init()
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

// ----------------------------------------------------------------------------

void VCP::connect_to_printf()
{
  init_printf(this, _putc);
}

// ----------------------------------------------------------------------------

void VCP::write(const uint8_t* ch, uint8_t len)
{
  if (!usbIsConnected() || !usbIsConfigured()) return;

  uint32_t start = millis();
  while (len > 0)
  {
    uint32_t num_bytes_sent = CDC_Send_DATA(const_cast<uint8_t*>(ch), len);
    len -= num_bytes_sent;
    ch += num_bytes_sent;

    if (len == 0 || millis() > (start + USB_TIMEOUT))
      break;
  }
}

// ----------------------------------------------------------------------------

uint8_t VCP::read_byte()
{
  uint8_t data;

  if (CDC_Receive_DATA(&data, 1))
    return data;
  else
    return 0;
}

// ----------------------------------------------------------------------------

uint32_t VCP::tx_bytes_free()
{
  return CDC_Send_FreeBytes();
}

// ----------------------------------------------------------------------------

bool VCP::tx_buffer_empty()
{
  return CDC_Send_FreeBytes() > 0;
}

// ----------------------------------------------------------------------------

uint32_t VCP::rx_bytes_waiting()
{
  return CDC_Receive_BytesAvailable();
}

// ----------------------------------------------------------------------------

bool VCP::in_bulk_mode()
{
  return false;
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void VCP::send_disconnect_signal()
{
  tx_pin_.set_mode(GPIO::OUTPUT);
  tx_pin_.write(GPIO::LOW);
  delay(200);
  tx_pin_.write(GPIO::HIGH);
  tx_pin_.set_mode(GPIO::PERIPH_IN_OUT);
}
