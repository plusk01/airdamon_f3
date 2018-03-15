#include <discoveryf3.h>

int main()
{
  board_init();

  LED info;
  info.init(GPIOE, GPIO_Pin_13);

  while(1)
  {
    delay(1000);
    info.toggle();
  }
}
