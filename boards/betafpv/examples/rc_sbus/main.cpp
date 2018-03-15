#include <betafpv.h>

int main()
{
  board_init();

  LED info;
  info.init(GPIOB, GPIO_Pin_8);

  airdamon::UART uart2;
  uart2.init(&uart_config[CFG_UART2]);

  airdamon::sensors::RC_SBUS rc;
  rc.init(&uart2);

  VCP vcp;
  vcp.connect_to_printf();


  while(1)
  {
    info.toggle();

    for (int i=0; i<8; i++)
    {
      float raw = rc.read(i);
      printf("%d, ", (uint32_t)(raw*1000));
    }

    if (rc.lost())
      printf("\tlost\n");
    else
      printf("\tOK\n");
    delay(20);
  }
}
