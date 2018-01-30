#include <led.h>

int main()
{
  systemInit();

  LED info;
  info.init(GPIOB, GPIO_Pin_8);

  while(1)
  {
    delay(1000);
    info.toggle();
  }
}
