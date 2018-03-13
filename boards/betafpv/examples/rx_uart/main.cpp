#include <memory>

#include <led.h>
#include <uart.h>
#include <vcp.h>

LED info;
std::unique_ptr<VCP> vcp;

void handle_byte(uint8_t byte)
{
  // info.toggle();
  vcp->write(&byte, 1);
}

int main()
{
  systemInit();

  // LED info;
  info.init(GPIOB, GPIO_Pin_8);

  vcp.reset(new VCP);

  UART uart2;
  uart2.init(USART2);

  // uart2.register_rx_callback(std::bind(handle_byte, std::placeholders::_1));
  uart2.register_rx_callback(handle_byte);

  // vcp.connect_to_printf();

  int i = 0;
  while(1)
  {

    info.on();
    delay(30);
    info.off();
    delay(30);

    // if (uart2.rx_bytes_waiting())
    //   printf("Read: %c\n", uart2.read_byte());
    // else
    //   printf("Nothing to read! %d\n", i++);

  }
}
