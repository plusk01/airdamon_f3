#include <discoveryf3.h>

#define NUM_I2C 2

int main()
{
  board_init();

  LED info;
  LED warn;
  info.init(GPIOE, GPIO_Pin_13);
  warn.init(GPIOE, GPIO_Pin_12);

  VCP vcp;
  vcp.init();
  vcp.connect_to_printf();

  printf("\n**** I2C Sniffer ****\n\n");

  warn.on();
  I2C i2c[NUM_I2C];
  i2c[0].init(I2C1);
  i2c[1].init(I2C2);
  warn.off();

  // There is no real data to send
  uint8_t data = 0;

  while(1)
  {
    info.toggle();
    for (uint8_t i = 0; i<NUM_I2C; i++)
      for (uint8_t j = 0; j<128; j++)
        if (i2c[i].write(j, 0xFF, data))
          printf("I2C%d: found device at 0x%X\n", i+1, j);
    printf("--------------------------\n");
  }
}