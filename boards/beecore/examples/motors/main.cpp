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

  airdamon::PWM motor1;
  motor1.init(&pwm_config[0], 480, 1000, 2000);

  while(1)
  {
    info.toggle();

    delay(50);
  }
}