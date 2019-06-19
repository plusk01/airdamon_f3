#include <revo.h>

int main()
{
  board_init();

  LED info;
  info.init(GPIOB, GPIO_Pin_4);

  int i = 0;

  VCP vcp;
  vcp.init();
  vcp.connect_to_printf();

  printf("\n**** VCP (Rx / Tx capable) Example ****\n\n");

  while(1)
  {

    info.toggle();

    if (vcp.rx_bytes_waiting())
      printf("Read: %c\n", vcp.read_byte());
    else
      printf("Nothing to read! %d\n", i++);

    delay(100);
  }
}
