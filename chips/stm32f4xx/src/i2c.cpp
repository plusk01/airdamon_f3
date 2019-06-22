#include "i2c.h"

#define timed_while_check(cond, timedout) \
  { \
    uint32_t timeout_var = 500; \
    while ((cond) && --timeout_var); \
    timedout = (timeout_var == 0); \
  }

// I2C ptrs used by IRQ handlers
static airdamon::I2C* I2C1Ptr = nullptr;
static airdamon::I2C* I2C2Ptr = nullptr;
static airdamon::I2C* I2C3Ptr = nullptr;

// ----------------------------------------------------------------------------

namespace airdamon {

void I2C::init(const I2CConfig * config, const ClockSpeed clock_speed)
{
  cfg_ = config;

  scl_pin_.init(cfg_->GPIOx, cfg_->scl_pin, GPIO::PERIPH_IN_OUT);
  sda_pin_.init(cfg_->GPIOx, cfg_->sda_pin, GPIO::PERIPH_IN_OUT);

  // Set GPIO pins as alternate function
  GPIO_PinAFConfig(cfg_->GPIOx, cfg_->scl_pin_source, cfg_->GPIO_AF);
  GPIO_PinAFConfig(cfg_->GPIOx, cfg_->sda_pin_source, cfg_->GPIO_AF);

  // Store the pointer to this object for use in IRQ handlers
  if (cfg_->I2Cx == I2C1)
    I2C1Ptr = this;
  else if (cfg_->I2Cx == I2C2)
    I2C2Ptr = this;
  else if (cfg_->I2Cx == I2C3)
    I2C3Ptr = this;

  unstick();
  
  init_I2C(clock_speed);
  // init_DMA();
  // init_NVIC();
  
  // TODO: Remove this hack
  write(0, 0, 0);
}

// ----------------------------------------------------------------------------

// blocking write, 1 byte
bool I2C::write(uint8_t addr, uint8_t data)
{
  uint8_t buf[1] = { data };
  return write(addr, buf, 1);
}

// ----------------------------------------------------------------------------

// blocking write
bool I2C::write(uint8_t addr, const uint8_t * data, uint8_t len)
{
  bool err = false;
  // if (current_status_ != IDLE) return BUSY;

  // Wait for I2C BUSY flag to clear
  timed_while_check(I2C_GetFlagStatus(cfg_->I2Cx, I2C_FLAG_BUSY) == SET, err);
  if (err) return false;

  //
  // Turn off interrupts for blocking write
  //

  I2C_ITConfig(cfg_->I2Cx, I2C_IT_EVT | I2C_IT_ERR, DISABLE);
  I2C_Cmd(cfg_->I2Cx, ENABLE);

  //
  // Generate START condition (which selects Master Mode; refman 27.3.3)
  //
  
  I2C_GenerateSTART(cfg_->I2Cx, ENABLE);

  // Wait for the start condition to be released on the bus and for the bus to be quiet
  timed_while_check(I2C_CheckEvent(cfg_->I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR, err);
  if (err) goto END_TRANSMISSION;

  //
  // Select the slave (by address) that we want to speak to
  //

  // As a Master Transmitter, put onto the bus the slave address we want to speak to
  I2C_Send7bitAddress(cfg_->I2Cx, addr << 1, I2C_Direction_Transmitter);

  // Wait for the slave to ACK (or for hardware to set ack failure (AF))
  timed_while_check(I2C_CheckEvent(cfg_->I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR
          && (I2C_GetLastEvent(cfg_->I2Cx) & I2C_SR1_AF) == RESET, err);
  err = (I2C_GetLastEvent(cfg_->I2Cx) & I2C_SR1_AF) == SET;
  if (err) goto END_TRANSMISSION;

  //
  // Now that we have the slave's attention, send it data
  //

  for (uint8_t i=0; i<len; ++i) {
    I2C_SendData(cfg_->I2Cx, data[i]);

    // wait for the byte to have been written to the data register to be shifted out
    timed_while_check(I2C_CheckEvent(cfg_->I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR, err);
    if (err) goto END_TRANSMISSION;
  }

  //
  // Send STOP condition; return bus to IDLE
  //

END_TRANSMISSION:
  I2C_GenerateSTOP(cfg_->I2Cx, ENABLE);
  I2C_Cmd(cfg_->I2Cx, DISABLE);

  return !err;
}

// ----------------------------------------------------------------------------

uint8_t I2C::read(uint8_t addr, uint8_t * data, uint8_t len)
{
  uint8_t recvd = 0;
  bool err = false;
  // if (current_status_ != IDLE) return BUSY;

  // Wait for I2C BUSY flag to clear
  timed_while_check(I2C_GetFlagStatus(cfg_->I2Cx, I2C_FLAG_BUSY) == SET, err);
  if (err) return false;

  //
  // Turn off interrupts for blocking read
  //

  I2C_ITConfig(cfg_->I2Cx, I2C_IT_EVT | I2C_IT_ERR, DISABLE);
  I2C_Cmd(cfg_->I2Cx, ENABLE);

  // Be a good listener and acknowledge what the slave says
  I2C_AcknowledgeConfig(cfg_->I2Cx, ENABLE);

  //
  // Generate START condition (which selects Master Mode; refman 27.3.3)
  //
  
  I2C_GenerateSTART(cfg_->I2Cx, ENABLE);

  // Wait for the start condition to be released on the bus and for the bus to be quiet
  timed_while_check(I2C_CheckEvent(cfg_->I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR, err);
  if (err) goto END_TRANSMISSION;

  //
  // Select the slave (by address) that we want to hear from
  //

  // As a Master Receiver, put onto the bus the slave address we want to hear from
  I2C_Send7bitAddress(cfg_->I2Cx, addr << 1, I2C_Direction_Receiver);

  // Wait for the slave to ACK (or for hardware to set ack failure (AF))
  timed_while_check(I2C_CheckEvent(cfg_->I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR
          && (I2C_GetLastEvent(cfg_->I2Cx) & I2C_SR1_AF) == RESET, err);
  err = (I2C_GetLastEvent(cfg_->I2Cx) & I2C_SR1_AF) == SET;
  if (err) goto END_TRANSMISSION;

  //
  // Now that we have the slave's attention, receive its data
  //

  for (uint8_t i=0; i<len; ++i) {
    // wait for the byte to have been written to the data register to be shifted out
    timed_while_check(I2C_CheckEvent(cfg_->I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR, err);
    if (err) goto END_TRANSMISSION;

    data[i] = I2C_ReceiveData(cfg_->I2Cx);
    recvd++;
  }

  //
  // Send STOP condition; return bus to IDLE
  //

END_TRANSMISSION:
  I2C_GenerateSTOP(cfg_->I2Cx, ENABLE);
  I2C_Cmd(cfg_->I2Cx, DISABLE);

  return recvd;
}

// ----------------------------------------------------------------------------

bool I2C::handle_error()
{
  return false;
}

// ----------------------------------------------------------------------------

bool I2C::handle_event()
{
  return false;
}

// ----------------------------------------------------------------------------

void I2C::hardware_failure()
{

}

// ----------------------------------------------------------------------------

void I2C::handle_hardware_failure()
{

}

// ----------------------------------------------------------------------------

void I2C::unstick()
{
  scl_pin_.set_mode(GPIO::OUTPUT);
  sda_pin_.set_mode(GPIO::OUTPUT);

  scl_pin_.write(GPIO::HIGH);
  sda_pin_.write(GPIO::HIGH);

  delayMicroseconds(100);

  for (uint8_t i=0; i<16; ++i) {
    delayMicroseconds(1);
    scl_pin_.toggle();
  }

  // Send a START condition
  sda_pin_.write(GPIO::LOW);
  delayMicroseconds(1);
  scl_pin_.write(GPIO::LOW);
  delayMicroseconds(1);

  // Send a STOP condition
  scl_pin_.write(GPIO::HIGH);
  delayMicroseconds(1);
  sda_pin_.write(GPIO::HIGH);
  delayMicroseconds(1);

  scl_pin_.set_mode(GPIO::PERIPH_IN_OUT);
  sda_pin_.set_mode(GPIO::PERIPH_IN_OUT);
  current_status_ = IDLE;
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void I2C::init_I2C(const ClockSpeed clock_speed)
{
  I2C_DeInit(cfg_->I2Cx);

  I2C_InitTypeDef I2C_InitStructure;
  I2C_StructInit(&I2C_InitStructure);
  I2C_InitStructure.I2C_ClockSpeed          = uv(clock_speed);
  I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1         = 0x00; // The first device address
  I2C_InitStructure.I2C_Ack                 = I2C_Ack_Disable; // RESET every I2C DISABLE
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(cfg_->I2Cx, &I2C_InitStructure);

  // Enable the I2Cx perhipheral
  I2C_Cmd(cfg_->I2Cx, ENABLE);
}

// ----------------------------------------------------------------------------

void I2C::init_DMA()
{
  DMA_Cmd(cfg_->Rx_DMA_Stream, DISABLE);

  DMA_DeInit(cfg_->Rx_DMA_Stream);
  DMA_InitStructure_.DMA_FIFOMode           = DMA_FIFOMode_Enable;
  DMA_InitStructure_.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full ;
  DMA_InitStructure_.DMA_MemoryBurst        = DMA_MemoryBurst_Single ;
  DMA_InitStructure_.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
  DMA_InitStructure_.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure_.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
  DMA_InitStructure_.DMA_MemoryInc          = DMA_MemoryInc_Enable;
  DMA_InitStructure_.DMA_Mode               = DMA_Mode_Normal;
  DMA_InitStructure_.DMA_Channel            = cfg_->DMA_Channel;
  
  DMA_InitStructure_.DMA_PeripheralBaseAddr = reinterpret_cast<uint32_t>(&(cfg_->I2Cx->DR));
  DMA_InitStructure_.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
  DMA_InitStructure_.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
  DMA_InitStructure_.DMA_Priority           = DMA_Priority_High;
  DMA_InitStructure_.DMA_DIR                = DMA_DIR_PeripheralToMemory;
}

// ----------------------------------------------------------------------------

void I2C::init_NVIC()
{
  // I2C Event Interrupt
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel                    = cfg_->I2Cx_EV_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  // I2C Error Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = cfg_->I2Cx_ER_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  
  // DMA Rx Stream Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = cfg_->Rx_DMA_IRQn;
  NVIC_Init(&NVIC_InitStructure);
}

} // ns airdamon

// ----------------------------------------------------------------------------
// IRQ Handlers associated with I2C and DMA
// ----------------------------------------------------------------------------

extern "C" void I2C1_ER_IRQHandler(void) {
  I2C1Ptr->handle_error();
}

// ----------------------------------------------------------------------------

extern "C" void I2C1_EV_IRQHandler(void) {
  I2C1Ptr->handle_event();
}

// ----------------------------------------------------------------------------

extern "C" void DMA1_Stream0_IRQHandler(void)
{
  // if (DMA_GetFlagStatus(DMA1_Stream0, DMA_FLAG_TCIF0))
  // {
  //   /* Clear transmission complete flag */
  //   DMA_ClearFlag(DMA1_Stream0, DMA_FLAG_TCIF0);

  //   I2C_DMACmd(I2C1, DISABLE);
     /*Send I2C1 STOP Condition */
  //   I2C_GenerateSTOP(I2C1, ENABLE);
  //   /* Disable DMA channel*/
  //   DMA_Cmd(DMA1_Stream0, DISABLE);

  //   I2C1Ptr->transfer_complete_cb(); // TODO make this configurable
  // }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

extern "C" void I2C2_ER_IRQHandler(void) {
  I2C2Ptr->handle_error();
}

// ----------------------------------------------------------------------------

extern "C" void I2C2_EV_IRQHandler(void) {
  I2C2Ptr->handle_event();
}

// ----------------------------------------------------------------------------

extern "C" void DMA1_Stream2_IRQHandler(void)
{

  // if (DMA_GetFlagStatus(DMA1_Stream2, DMA_FLAG_TCIF2))
  // {
  //   /* Clear transmission complete flag */
  //   DMA_ClearFlag(DMA1_Stream2, DMA_FLAG_TCIF2);

  //   I2C_DMACmd(I2C2, DISABLE);
  //   /*Send I2C1 STOP Condition */
  //   I2C_GenerateSTOP(I2C2, ENABLE);
  //   /* Disable DMA channel*/
  //   DMA_Cmd(DMA1_Stream2, DISABLE);

  //   I2C2Ptr->transfer_complete_cb(); // TODO make this configurable
  // }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

extern "C" void I2C3_ER_IRQHandler(void) {
  I2C3Ptr->handle_error();
}

// ----------------------------------------------------------------------------

extern "C" void I2C3_EV_IRQHandler(void) {
  I2C3Ptr->handle_event();
}

// ----------------------------------------------------------------------------

// I2C3 is also DMA1_Stream2? (refman table 42)
