#include <revo.h>

int main()
{
  board_init();

  LED info;
  LED warn;
  info.init(GPIOB, GPIO_Pin_5);
  warn.init(GPIOB, GPIO_Pin_4);

  airdamon::VCP vcp;
  vcp.init();
  vcp.connect_to_printf();

  printf("\n**** I2C Sniffer ****\n\n");


  warn.on();
  airdamon::I2C i2c[NUM_I2CS];
  i2c[0].init(&i2c_config[CFG_I2C1], airdamon::I2C::ClockSpeed::FastMode400kHz);
  i2c[1].init(&i2c_config[CFG_I2C2], airdamon::I2C::ClockSpeed::StandardMode100kHz);
  warn.off();

  // There is no real data to send
  uint8_t data = 0;

  while(1)
  {
    info.toggle();
    for (uint8_t i = 0; i<NUM_I2CS; i++)
      for (uint8_t j = 0; j<128; j++)
        if (i2c[i].write(j, 0xFF, data))
          printf("I2C%d: found device at 0x%X\n", i+1, j);
    printf("--------------------------\n");
    delay(100);
  }
}