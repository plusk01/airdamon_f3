#include <led.h>
#include <uart.h>

int main()
{
  systemInit();

  LED info;
  LED warn;
  info.init(GPIOE, GPIO_Pin_13);
  warn.init(GPIOE, GPIO_Pin_12);

  // UART uart1(USART1);

  int i = 0;

  while(1)
  {
    warn.on();
    info.on();
    delay(100);
    info.off();
    warn.off();
    delay(100);

    // printf("hellooo! %d\n", i++);
  }
}
