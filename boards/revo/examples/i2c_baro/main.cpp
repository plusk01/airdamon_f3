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

  printf("\n**** I2C Baro ****\n\n");

  airdamon::I2C i2c1;
  i2c1.init(&i2c_config[CFG_I2C1], airdamon::I2C::ClockSpeed::FastMode400kHz);

  airdamon::sensors::MS5611 baro;
  baro.init(&i2c1, GPIO::LOW); // CSB pin is tied low on U7
  if (baro.present()) info.on();

  // There is no real data to send
  uint8_t data = 0;

  while(1) {
    warn.toggle();
    
    for (uint8_t i=0; i<8; ++i) {
      // printf("I2C%d: found device at 0x%X\n", i+1, j);
      printf("%d: %d (0x%X)\n", i, baro.prom_[i], baro.prom_[i]);
    }

    printf("---------------------------------------\n\n");
    delay(1000);
  }
}