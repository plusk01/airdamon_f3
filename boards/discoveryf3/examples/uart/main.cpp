#include <discoveryf3.h>

int main()
{
  board_init();

  LED info;
  LED warn;
  info.init(GPIOE, GPIO_Pin_13);
  warn.init(GPIOE, GPIO_Pin_12);

  airdamon::UART uart1;
  uart1.init(&uart_config[CFG_UART1]);
  uart1.connect_to_printf();

  int i = 0;

  printf("\n**** UART (Rx / Tx capable) Example ****\n\n");

  while(1)
  {
    warn.on();
    info.on();
    delay(300);
    info.off();
    warn.off();
    delay(300);

    if (uart1.rx_bytes_waiting())
      printf("Read: %c\n", uart1.read_byte());
    else
      printf("Nothing to read! %d\n", i++);
  }
}
