#include <betafpv_f3.h>
#include <led.h>
#include <uart.h>
#include <vcp.h>
#include <rc_sbus.h>

int main()
{
  systemInit();

  // LED info;
  // info.init(GPIOB, GPIO_Pin_8);

  airdamon::UART uart2;
  uart2.init(&uart_config[0]);

  airdamon::sensors::RC_SBUS rc;
  rc.init(&uart2);

  VCP vcp;
  vcp.connect_to_printf();


  while(1)
  {

    // info.on();
    // delay(30);
    // info.off();
    // delay(30);

    for (int i=0; i<8; i++)
    {
      float raw = rc.read(i);
      printf("%d, ", (uint32_t)(raw*1000));
    }

    printf("\n");
    delay(20);

  }
}
