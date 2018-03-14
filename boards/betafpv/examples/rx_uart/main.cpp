#include <memory>

#include <led.h>
#include <uart.h>
#include <vcp.h>

LED info;

uint8_t bytes[25];
uint8_t buffer_pos = 0;
bool data_ready = false;

bool once = false;

void handle_byte(uint8_t byte)
{

  // info.toggle();
  bytes[buffer_pos++] = byte;

  if (buffer_pos == 25)
  {
    buffer_pos = 0;
    data_ready = true;
  }
}

int main()
{
  systemInit();

  // LED info;
  info.init(GPIOB, GPIO_Pin_8);

  UART uart2;
  USART_InvPinCmd(USART2, USART_InvPin_Rx, ENABLE);
  uart2.init(USART2, 100000, UART::Mode::m8E2);


  // uart2.register_rx_callback(std::bind(handle_byte, std::placeholders::_1));
  uart2.register_rx_callback(handle_byte);

  VCP vcp;
  vcp.connect_to_printf();

  // uint8_t tmp[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  int i = 0;
  while(1)
  {

    // info.on();
    // delay(30);
    // info.off();
    // delay(30);

    // if (uart2.rx_bytes_waiting())
    //   printf("Read: %c\n", uart2.read_byte());
    // else
    //   printf("Nothing to read! %d\n", i++);

    if (once || data_ready)
    {
      vcp.write(bytes, 25);
      data_ready = false;

      // once = true;
    }

    // vcp.write(tmp, 20);

  }
}
