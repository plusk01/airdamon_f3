#include <discoveryf3.h>

int main()
{
  board_init();

  LED info;
  LED warn;
  info.init(GPIOE, GPIO_Pin_13);
  warn.init(GPIOE, GPIO_Pin_12);

  airdamon::UART uart1;
  uart1.init(&uart_config[CFG_UART1]);
  uart1.connect_to_printf();

  VCP vcp;
  // vcp.connect_to_printf();

  printf("\n**** SPI Gyro L3GD20 ****\n\n");

  SPI spi1;
  GPIO cs;
  sensors::L3GD20 gyro;

  sensors::L3GD20::DeviceParams gyro_params;
  sensors::L3GD20::FilterParams filt_params;

  gyro_params.power_mode        = sensors::L3GD20::Power_Mode::ACTIVE;
  gyro_params.output_datarate   = sensors::L3GD20::Output_DataRate::RATE1;
  gyro_params.axes_enable       = sensors::L3GD20::Axes::ENABLE;
  gyro_params.bandwidth         = sensors::L3GD20::Bandwidth::BW4;
  gyro_params.blockdata_update  = sensors::L3GD20::BD_Update::CONTINUOUS;
  gyro_params.endianness        = sensors::L3GD20::Endianness::LSB;
  gyro_params.full_scale        = sensors::L3GD20::FS_Selection::FS500;

  filt_params.hpf_mode          = sensors::L3GD20::HPF_Mode::NORMAL_RES;
  filt_params.hpf_cutoff_freq   = sensors::L3GD20::HPF_CF::F0;

  warn.on();
  spi1.init(SPI1);
  cs.init(GPIOE, GPIO_Pin_3, GPIO::OUTPUT);
  gyro.init(&spi1, &cs, &gyro_params);
  gyro.enable_hpfilter(&filt_params);
  warn.off();

  while(1)
  {
    info.toggle();

    float gx = 0.0f, gy = 0.0f, gz = 0.0f;
    gyro.read_rates(gx, gy, gz);

    printf("Gyro (x, y, z): %d\t%d\t%d\n",
        static_cast<int>(gx*1000),
        static_cast<int>(gy*1000),
        static_cast<int>(gz*1000));

    delay(50);
  }
}