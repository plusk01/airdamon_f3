#include "l3gd20.h"

namespace sensors {

void L3GD20::init(SPI* spi, DeviceParams* params)
{
  spi_ = spi;

  // configure the gyro
  uint8_t ctrl1 = 0x00, ctrl4 = 0x00;

  // Configure MEMS: data rate, power mode, full scale and axes
  ctrl1 |= params->Power_Mode.underlying_value()
        |  params->Output_DataRate.underlying_value()
        |  params->Axes_Enable.underlying_value()
        |  params->Bandwidth.underlying_value();
  
  ctrl4 |= params->BlockData_Update.underlying_value()
        |  params->Endianness.underlying_value()
        |  params->Full_Scale.underlying_value();
                    
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
  tmpreg |= static_cast<uint8_t>(Boot_Mode::REBOOTMEMORY);
  write(RegAddr::CTRL_REG5, &tmpreg, 1);
}

// ----------------------------------------------------------------------------

// void L3GD20::

// ----------------------------------------------------------------------------

}