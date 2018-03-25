#include <cmath>
#include <limits>

#include <beecore.h>

void findAxis(float* data, char* sign, char* axis)
{
  // first, find the axis with the largets norm
  int idx = -1;
  float val = std::numeric_limits<float>::min();
  for (int i=0; i<3; i++)
  {
    if (std::abs(data[i]) > val)
    {
      idx = i;
      val = std::abs(data[i]);
    }
  }

  // find out if that axis is positive or negative
  *sign = (data[idx] > 0) ? '+' : '-';

  // find the ASCII char associated with this axis
  *axis = 'X' + idx;
}

int main()
{
  board_init();

  LED info;
  info.init(GPIOB, GPIO_Pin_8);

  airdamon::UART uart1;
  uart1.init(&uart_config[CFG_UART1]);
  uart1.connect_to_printf();

  VCP vcp;
  vcp.init();
  // vcp.connect_to_printf();

  printf("\n**** SPI IMU MPU6500 ****\n\n");

  airdamon::SPI spi1;
  GPIO cs;
  airdamon::sensors::MPU6500 imu;

  info.on();
  spi1.init(&spi_config[CFG_SPI1]);
  cs.init(GPIOB, GPIO_Pin_9, GPIO::OUTPUT);
  imu.init(&spi1, &cs);
  info.off();

  delay(2000);

  float acc[3];
  float gyro[3];
  float temp;

  bool showAccel = true;
  bool showGyro = false;
  bool showTemp = false;

  while(1)
  {
    info.toggle();

    imu.read(acc, gyro, &temp);

    // Allow user to select a different data stream
    if (uart1.rx_bytes_waiting()) {
      uint8_t byte = uart1.read_byte();
      showAccel = (byte == 'a');
      showGyro = (byte == 'g');
      showTemp = (byte == 't');
    }

    if (showAccel)
    {
      // Which has the greatest magnitude?
      char sign, axis;
      findAxis(acc, &sign, &axis);

      printf("Accel (%c%c): %d\t%d\t%d\n",
          sign, axis,
          static_cast<int>(acc[0]*1000),
          static_cast<int>(acc[1]*1000),
          static_cast<int>(acc[2]*1000));
    }

    if (showGyro)
    {
      // Which has the greatest magnitude?
      char sign, axis;
      findAxis(gyro, &sign, &axis);

      printf("Gyro (%c%c): %d\t%d\t%d\n",
                sign, axis,
                static_cast<int>(gyro[0]*1000),
                static_cast<int>(gyro[1]*1000),
                static_cast<int>(gyro[2]*1000));
    }

    if (showTemp)
      printf("Temp: %d\n",
                static_cast<int>(temp*1000));

    delay(50);
  }
}