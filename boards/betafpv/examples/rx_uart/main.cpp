#include <betafpv.h>

LED info;

uint8_t bytes[25];
uint8_t buffer_pos = 0;
bool data_ready = false;

void handle_byte(uint8_t byte)
{
  bytes[buffer_pos++] = byte;

  if (buffer_pos == 25)
  {
    info.toggle();
    buffer_pos = 0;
    data_ready = true;
  }
}

int main()
{
  board_init();

  // LED info;
  info.init(GPIOB, GPIO_Pin_8);

  airdamon::UART uart2;
  uart2.init(&uart_config[CFG_UART2], 100000, airdamon::UART::Mode::m8E2, true);

  uart2.register_rx_callback(handle_byte);

  VCP vcp;
  vcp.connect_to_printf();

  while(1)
  {
    if (data_ready)
    {
      vcp.write(bytes, 25);
      data_ready = false;
    }
  }
}
