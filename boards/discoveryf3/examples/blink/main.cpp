#include <led.h>

int main()
{
  systemInit();

  LED info;
  info.init(GPIOE, GPIO_Pin_13);

  while(1)
  {
    delay(1000);
    info.toggle();
  }
}
