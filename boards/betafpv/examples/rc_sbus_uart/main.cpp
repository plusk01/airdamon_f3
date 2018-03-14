#include <led.h>
#include <uart.h>
#include <vcp.h>
#include <rc_sbus.h>

int main()
{
  systemInit();

  // LED info;
  // info.init(GPIOB, GPIO_Pin_8);

  UART uart2;
  uart2.init(USART2);

  sensors::RC_SBUS rc;
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
