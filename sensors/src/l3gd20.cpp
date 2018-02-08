#include "l3gd20.h"

namespace sensors {

void L3GD20::init(SPI* spi, DeviceParams* params)
{
  spi_ = spi;

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
// Private Methods
// ----------------------------------------------------------------------------

void L3GD20::write(RegAddr addr, uint8_t* buffer, uint16_t len)
{

}

// ----------------------------------------------------------------------------

void L3GD20::read(RegAddr addr, uint8_t* buffer, uint16_t len)
{

}

// ----------------------------------------------------------------------------

void L3GD20::chip_select(bool enable)
{

}

// ----------------------------------------------------------------------------

void L3GD20::reboot()
{
  uint8_t tmpreg;

  // Reboot the memory content of the L3GD20
  read(RegAddr::CTRL_REG5, &tmpreg, 1);
  tmpreg |= uv(Boot_Mode::REBOOTMEMORY);
  write(RegAddr::CTRL_REG5, &tmpreg, 1);
}

// ----------------------------------------------------------------------------

// void L3GD20::

// ----------------------------------------------------------------------------

}