#include <beecore.h>

int main()
{
  board_init();

  LED info;
  info.init(GPIOB, GPIO_Pin_8);

  airdamon::UART uart1;
  uart1.init(&uart_config[CFG_UART1]);
  uart1.connect_to_printf();

  VCP vcp;
  // vcp.connect_to_printf();

  airdamon::PWM m1, m2, m3, m4;
  m1.init(&pwm_config[0], 480, 1000, 2000);
  m2.init(&pwm_config[1], 480, 1000, 2000);
  m3.init(&pwm_config[2], 480, 1000, 2000);
  m4.init(&pwm_config[3], 480, 1000, 2000);

  m1.write_us(1000);
  m2.write_us(1000);
  m3.write_us(1000);
  m4.write_us(1000);


  int i = 1000;

  while(1)
  {
    info.toggle();

    if (i++>1100) i = 1000;
    m1.write_us(i);
    m2.write_us(i);
    m3.write_us(i);
    m4.write_us(i);

    delay(50);
  }
}