#include <revo.h>

int main()
{
  board_init();

  LED info;
  info.init(GPIOB, GPIO_Pin_4);

  airdamon::UART uart3;
  uart3.init(&uart_config[CFG_UART3]);
  uart3.connect_to_printf();

  int i = 0;

  printf("\n**** UART (Rx / Tx capable) Example ****\n\n");

  while(1)
  {
    info.toggle();
    delay(50);

    if (uart3.rx_bytes_waiting())
      printf("Read: %c\n", uart3.read_byte());
    else
      printf("Nothing to read! %d\n", i++);
  }
}
