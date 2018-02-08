#include <led.h>
#include <vcp.h>
#include <uart.h>
#include <spi.h>
#include <l3gd20.h>

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

  printf("\n**** SPI Gyro L3GD20 ****\n\n");

  SPI spi1;
  sensors::L3GD20 gyro;

  sensors::L3GD20::DeviceParams gyro_params;
  sensors::L3GD20::FilterParams filt_params;

  gyro_params.power_mode = sensors::L3GD20::Power_Mode::ACTIVE;

  warn.on();
  spi1.init(SPI1);
  gyro.init(&spi1, &gyro_params);
  // gyro.enable_filter(&filt_params);
  warn.off();

  while(1)
  {
    info.toggle();
    delay(200);
  }
}