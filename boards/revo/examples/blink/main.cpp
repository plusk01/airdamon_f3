#include <revo.h>

int main()
{
  board_init();

  LED warn, stat;
  warn.init(GPIOB, GPIO_Pin_4);
  stat.init(GPIOB, GPIO_Pin_5);

  warn.off();
  stat.on();

  while(1)
  {
    delay(1000);
    warn.toggle();
    stat.toggle();
  }
}
