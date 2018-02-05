#include <led.h>
#include <uart.h>
#include <vcp.h>

int main()
{
  systemInit();

  LED info;
  info.init(GPIOB, GPIO_Pin_8);

  int i = 0;

  UART uart2;
  uart2.init(USART2);

  VCP vcp;
  vcp.connect_to_printf();

  printf("\n**** VCP (Rx / Tx capable) Example ****\n\n");

  while(1)
  {

    info.on();
    delay(30);
    info.off();
    delay(30);

    if (uart2.rx_bytes_waiting())
      printf("Read: %c\n", uart2.read_byte());
    else
      printf("Nothing to read! %d\n", i++);

  }
}
