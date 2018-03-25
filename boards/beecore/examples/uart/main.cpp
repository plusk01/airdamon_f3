#include <beecore.h>

int main()
{
  board_init();

  LED info;
  info.init(GPIOB, GPIO_Pin_8);

  airdamon::UART uart1;
  uart1.init(&uart_config[CFG_UART1]);
  uart1.connect_to_printf();

  int i = 0;

  printf("\n**** UART (Rx / Tx capable) Example ****\n\n");

  while(1)
  {
    info.toggle();
    delay(50);

    if (uart1.rx_bytes_waiting())
      printf("Read: %c\n", uart1.read_byte());
    else
      printf("Nothing to read! %d\n", i++);
  }
}
