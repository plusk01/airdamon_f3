#include "l3gd20.h"

namespace sensors {

void L3GD20::init(airdamon::SPI* spi, GPIO* cs, DeviceParams* params)
{
  spi_ = spi;
  cs_ = cs;

  // ensure that the SPI line is inactive
  chip_select(false);

  // configure the gyro
  uint8_t ctrl1 = 0x00, ctrl4 = 0x00;

  // Configure MEMS: data rate, power mode, full scale and axes
  ctrl1 |= uv(params->power_mode)
        |  uv(params->output_datarate)
        |  uv(params->axes_enable)
        |  uv(params->bandwidth);
  
  ctrl4 |= uv(params->blockdata_update)
        |  uv(params->endianness)
        |  uv(params->full_scale);
                    
  // Write value to MEMS CTRL_REG1 regsister
  write(RegAddr::CTRL_REG1, &ctrl1, 1);
  
  // Write value to MEMS CTRL_REG4 regsister
  write(RegAddr::CTRL_REG4, &ctrl4, 1);
}

// ----------------------------------------------------------------------------

void L3GD20::enable_hpfilter(FilterParams* params)
{
  uint8_t ctrl2 = 0x00, ctrl5 = 0x00;

  //
  // High Pass Filter configuration
  //

  read(RegAddr::CTRL_REG2, &ctrl2, 1);

  // clear the filter configuration bits
  ctrl2 &= 0xC0;

  // write the filter configuration bits
  ctrl2 |= uv(params->hpf_mode)
        |  uv(params->hpf_cutoff_freq);

  // write the value to the reg
  write(RegAddr::CTRL_REG2, &ctrl2, 1);

  //
  // Filter state control
  //

  read(RegAddr::CTRL_REG5, &ctrl5, 1);

  // clear the enable/disable bit
  ctrl5 &= 0xEF;

  // enable the HP filter
  ctrl5 |= uv(HPF_State::ENABLE);

  // write the value to the reg
  write(RegAddr::CTRL_REG5, &ctrl5, 1);

}

// ----------------------------------------------------------------------------

void L3GD20::read_rates(float& x, float& y, float& z)
{
  // read the current configuration for Endianness and sensitivity
  uint8_t ctrl4 = 0x00;
  read(RegAddr::CTRL_REG4, &ctrl4, 1);

  // read the 6 registers containing x, y, z data (16 bits each)
  uint8_t buffer[6] = {0};
  read(RegAddr::OUT_X_L, buffer, 6);


  // reconstruct high and low bits. Note that the value is stored
  // as two's complement, so make sure to cast to signed at the end.
  // Make sure to cast BEFORE shifting.
  int16_t raw[3] = {0};

  // check if Big Endian or Little Endian
  if (!(ctrl4 & uv(Endianness::MSB)))
  { // little endian
    for (int i=0; i<3; i++)
    {
      const uint16_t high = static_cast<uint16_t>(buffer[2*i+1])<<8;
      raw[i] = static_cast<int16_t>(high | buffer[2*i]);
    }
  }
  else
  { // big endian
    for (int i=0; i<3; i++)
    {
      const uint16_t high = static_cast<uint16_t>(buffer[2*i])<<8;
      raw[i] = static_cast<int16_t>(high | buffer[2*i+1]);
    }
  }

  // Find the right sensitivity value based on CTRL4
  float sensitivity = 0;
  constexpr uint8_t FS_Selection_Mask = uv(FS_Selection::FS250) | uv(FS_Selection::FS500) | uv(FS_Selection::FS2000);
  switch (ctrl4 & FS_Selection_Mask)
  {
    case uv(FS_Selection::FS250):
      sensitivity = 114.285f; // 250 dps full scale (LSB/dps)
      break;
    case uv(FS_Selection::FS500):
      sensitivity = 57.1429f; // 500 dps full scale (LSB/dps)
      break;
    case uv(FS_Selection::FS2000):
      sensitivity = 14.285f; // 2000 dps full scale (LSB/dps)
      break;
  }

  // divide by the sensitivity
  x = static_cast<float>(raw[0])/sensitivity;
  y = static_cast<float>(raw[1])/sensitivity;
  z = static_cast<float>(raw[2])/sensitivity;
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void L3GD20::write(RegAddr addr, uint8_t* buffer, uint16_t len)
{
  uint8_t write_addr = uv(addr);

  if (len > 1)
    write_addr |= uv(RegAddr::MULTIPLEBYTE_CMD);

  // signal beginning of transmission to the L3GD20 device
  chip_select(true);

  // send address of register to write to
  spi_->transfer_byte(write_addr);

  // Send the data to device (MSB first)
  while (len > 0)
  {
    spi_->transfer_byte(*buffer);
    len--;
    buffer++;
  }

  // signal end of transmission to the L3GD20 device
  chip_select(false);
}

// ----------------------------------------------------------------------------

void L3GD20::read(RegAddr addr, uint8_t* buffer, uint16_t len)
{
  uint8_t read_addr = uv(addr);

  // let the device know that we will write
  // dummy bytes so that we can read
  read_addr |= uv(RegAddr::READWRITE_CMD);

  if (len > 1)
    read_addr |= uv(RegAddr::MULTIPLEBYTE_CMD);

  // signal beginning of transmission to the L3GD20 device
  chip_select(true);

  // send address of register to write to
  spi_->transfer_byte(read_addr);

  // Send the data to device (MSB first)
  while (len > 0)
  {
    *buffer = spi_->transfer_byte(uv(RegAddr::DUMMY_BYTE));
    len--;
    buffer++;
  }

  // signal end of transmission to the L3GD20 device
  chip_select(false);
}

// ----------------------------------------------------------------------------

void L3GD20::chip_select(bool enable)
{
  if (enable)
    cs_->write(GPIO::LOW);
  else
    cs_->write(GPIO::HIGH);
}

// ----------------------------------------------------------------------------

void L3GD20::reboot()
{
  uint8_t ctrl5;

  // Reboot the memory content of the L3GD20
  read(RegAddr::CTRL_REG5, &ctrl5, 1);
  ctrl5 |= uv(Boot_Mode::REBOOTMEMORY);
  write(RegAddr::CTRL_REG5, &ctrl5, 1);
}

// ----------------------------------------------------------------------------

}