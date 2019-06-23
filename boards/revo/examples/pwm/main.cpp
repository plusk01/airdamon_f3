#include <revo.h>

int main()
{
  board_init();

  airdamon::LED info, warn;
  info.init(GPIOB, GPIO_Pin_5, true);
  warn.init(GPIOB, GPIO_Pin_4, true);

  airdamon::VCP vcp;
  vcp.init();
  vcp.connect_to_printf();

  airdamon::PWM pwm[NUM_PWMS];
  pwm[0].init(&pwm_config[0], 490, 1000, 2000);
  pwm[1].init(&pwm_config[1], 490, 1000, 2000);
  pwm[2].init(&pwm_config[2], 490, 1000, 2000);
  pwm[3].init(&pwm_config[3], 490, 1000, 2000);

  printf("\n**** PWM Tester ****\n\n");

  pwm[0].write(0.0f);
  pwm[1].write(0.5f);
  pwm[2].write(1.0f);

  pwm[3].init(&pwm_config[3], 50, 1000, 2000);
  pwm[3].write(1.0f);

  while(1) {
    info.toggle();
    delay(250);
  }

  int i = 1000;

  while(1) {
    info.toggle();

    i += 100;
    if (i>2000) i = 1000;
    pwm[0].write_us(i);

    delay(500);
  }

  // PWM_OUT esc_out[PWM_NUM_OUTPUTS];
  // for (int i = 0; i < PWM_NUM_OUTPUTS; ++i)
  // {
  //   esc_out[i].init(&pwm_config[i], 490, 2000, 1000);
  //   esc_out[i].write(1.0);
  // }

  // // Calibrate ESC
  // while (millis() < 5000);

  // for (int i = 0; i < PWM_NUM_OUTPUTS; ++i)
  // {
  //   esc_out[i].write(0.0);
  // }

  // while (millis() < 1000);


  // bool use_us_driver = true;;
  // uint32_t throttle = 1000;
  // while(1)
  // {
  //   for (int i = 0; i < PWM_NUM_OUTPUTS; ++i)
  //   {
  //     if (use_us_driver)
  //     {
  //       esc_out[i].writeUs(throttle);
  //     }
  //     else
  //     {
  //       esc_out[i].write((float)(throttle - 1000) / 1000.0);
  //     }
  //   }
  //   throttle += 1;
  //   if (throttle > 2000)
  //   {
  //     throttle = 0;
  //     info.toggle();
  //     use_us_driver = !use_us_driver;
  //   }
  //   delay(2);
  // }
  
}
