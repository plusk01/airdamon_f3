#include <discoveryf3.h>

int main()
{
  board_init();

  LED info;
  LED warn;
  info.init(GPIOE, GPIO_Pin_13);
  warn.init(GPIOE, GPIO_Pin_12);

  VCP vcp;
  vcp.connect_to_printf();

  printf("\n**** Time Example ****\n\n");

  info.off();
  warn.on();


  int i = 0;
  while(1)
  {
    warn.toggle();
    info.toggle();

    printf("time = %d s, %d ms, %d us\n", i++, millis(), (uint32_t)micros());

    delay(1000);
  }
}
