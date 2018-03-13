#include <led.h>
#include <uart.h>
#include <vcp.h>

LED info;
UART uart1;

void handle_byte(uint8_t byte)
{
  info.toggle();
  uart1.write(&byte, 1);
}

int main()
{
  systemInit();

  // LED info;
  LED warn;
  info.init(GPIOE, GPIO_Pin_13);
  warn.init(GPIOE, GPIO_Pin_12);

  // UART uart1;
  uart1.init(USART1);
  uart1.register_rx_callback(handle_byte);

  int i = 0;

  VCP vcp;
  vcp.connect_to_printf();

  while(1)
  {
    warn.on();
    delay(1000);
    warn.off();
    delay(1000);
  }
}
