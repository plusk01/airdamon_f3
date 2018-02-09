#include <led.h>
#include <vcp.h>
#include <uart.h>
#include <spi.h>
#include <mpu6500.h>

int main()
{
  systemInit();

  LED info;
  LED warn;
  info.init(GPIOE, GPIO_Pin_13);
  warn.init(GPIOE, GPIO_Pin_12);

  UART uart1;
  uart1.init(USART1);
  uart1.connect_to_printf();

  VCP vcp;
  // vcp.connect_to_printf();

  printf("\n**** SPI IMU MPU6500 ****\n\n");

  SPI spi1;
  GPIO cs;
  sensors::MPU6500 imu;

  warn.on();
  spi1.init(SPI1);
  cs.init(GPIOE, GPIO_Pin_3, GPIO::OUTPUT);
  // imu.init(&spi1, &cs, &imu_params);
  // imu.enable_hpfilter(&filt_params);
  warn.off();

  while(1)
  {
    info.toggle();

    float gx = 0.0f, gy = 0.0f, gz = 0.0f;
    imu.read(gx, gy, gz);

    printf("Gyro (x, y, z): %d\t%d\t%d\n",
        static_cast<int>(gx*1000),
        static_cast<int>(gy*1000),
        static_cast<int>(gz*1000));

    delay(50);
  }
}