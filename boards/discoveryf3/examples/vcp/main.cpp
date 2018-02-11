#include <led.h>
#include <uart.h>
#include <vcp.h>

int main()
{
  systemInit();

  LED info;
  LED warn;
  info.init(GPIOE, GPIO_Pin_13);
  warn.init(GPIOE, GPIO_Pin_12);

  UART uart1;
  uart1.init(USART1);
  uart1.connect_to_printf();

  int i = 0;

  VCP vcp;

  printf("\n**** VCP (Rx / Tx capable) Example ****\n\n");

  while(1)
  {

    warn.on();
    info.on();
    delay(300);
    info.off();
    warn.off();
    delay(300);

    uint8_t msg[5] = "Hey\n";
    vcp.write(msg, 4);

    if (vcp.rx_bytes_waiting())
      printf("Read: %c\n", vcp.read_byte());
    else
      printf("Nothing to read! %d\n", i++);

  }
}
