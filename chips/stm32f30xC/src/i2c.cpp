#include "i2c.h"

#define while_check(cond) \
  { \
    uint32_t timeout_var = 50000; \
    while ((cond) && --timeout_var); \
    if (!timeout_var) \
    { \
      handle_hardware_failure(); \
      return false; \
    } \
  }

// I2C ptrs used by IRQ handlers
static I2C* I2C1_Ptr;
static I2C* I2C2_Ptr;
static I2C* I2C3_Ptr;

// ----------------------------------------------------------------------------

void I2C::init(I2C_TypeDef* I2Cx)
{
  // store the desired I2C peripheral
  I2Cx_ = I2Cx;

  if (I2Cx_ == I2C1)
  {
    // Initialize GPIO pins for I2C1
    scl_.init(GPIOB, GPIO_Pin_6, GPIO::PERIPH_IN_OUT);
    sda_.init(GPIOB, GPIO_Pin_7, GPIO::PERIPH_IN_OUT);

    // Set GPIO pins as alternate function
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_4);

    // properly initialize GPIO I2C pins
    // unstick();

    // Enable clock to I2C1, an APB1 peripheral
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  }

  // Configure the I2Cx perhipheral
  I2C_InitTypeDef I2C_InitStructure;
  I2C_StructInit(&I2C_InitStructure);
  I2C_InitStructure.I2C_Timing              = 0x00902025; //I2C_HIGHSPEED_TIMING; 
  I2C_InitStructure.I2C_AnalogFilter        = I2C_AnalogFilter_Enable;
  I2C_InitStructure.I2C_DigitalFilter       = 0x00;
  I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;
  I2C_InitStructure.I2C_OwnAddress1         = 0x00;
  I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2Cx_, &I2C_InitStructure);

  // I2C_StretchClockCmd(I2Cx, ENABLE);
  
  // Enable the I2Cx perhipheral
  I2C_Cmd(I2Cx_, ENABLE);
}

// ----------------------------------------------------------------------------

// blocking, single register write (for configuring devices)
bool I2C::write(uint8_t addr, uint8_t reg, uint8_t data)
{
  // if (current_status_ != IDLE) return BUSY;

  // Wait for I2C BUSY flag to clear
  while_check(I2C_GetFlagStatus(I2Cx_, I2C_ISR_BUSY));
  
  I2C_TransferHandling(I2Cx_, addr, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);

  // Wait for hardware to signal that the I2C_TXDR reg is empty and needs to be filled
  while_check(I2C_GetFlagStatus(I2Cx_, I2C_ISR_TXIS) == RESET);

  I2C_SendData(I2Cx_, reg);
  while_check(I2C_GetFlagStatus(I2Cx_, I2C_ISR_TCR) == RESET);

  I2C_TransferHandling(I2Cx_, addr, 1, I2C_AutoEnd_Mode, I2C_No_StartStop);

  // Wait for hardware to signal that the I2C_TXDR reg is empty and needs to be filled
  while_check(I2C_GetFlagStatus(I2Cx_, I2C_ISR_TXIS) == RESET);

  // Write data to TXDR
  I2C_SendData(I2Cx_, data);

  // Wait until STOPF flag is set
  while_check(I2C_GetFlagStatus(I2Cx_, I2C_ISR_STOPF) == RESET);

  // clear the STOPF flag
  I2C_ClearFlag(I2Cx_, I2C_ICR_STOPCF);

  return true;
}

bool I2C::handle_error()
{

}

bool I2C::handle_event()
{

}

void I2C::hardware_failure()
{

}

void I2C::handle_hardware_failure()
{

}

void I2C::unstick()
{
  scl_.set_mode(GPIO::OUTPUT);
  sda_.set_mode(GPIO::OUTPUT);

  scl_.write(GPIO::HIGH);
  sda_.write(GPIO::HIGH);

  for (int i = 0; i < 8; ++i)
  {
    delayMicroseconds(1);
    scl_.toggle();
  }

  sda_.write(GPIO::LOW);
  delayMicroseconds(1);
  scl_.write(GPIO::LOW);
  delayMicroseconds(1);

  scl_.write(GPIO::HIGH);
  delayMicroseconds(1);
  sda_.write(GPIO::HIGH);
  delayMicroseconds(1);

  scl_.set_mode(GPIO::PERIPH_IN_OUT);
  sda_.set_mode(GPIO::PERIPH_IN_OUT);
  current_status_ = IDLE;
}

extern "C"
{

// C-based IRQ functions (defined in the STD lib somewhere)
void DMA1_Stream2_IRQHandler(void)
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

  //   I2C2_Ptr->transfer_complete_cb(); // TODO make this configurable
  // }
}

void DMA1_Stream0_IRQHandler(void)
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

  //   I2C1_Ptr->transfer_complete_cb(); // TODO make this configurable
  // }
}

void I2C1_ER_IRQHandler(void) {
  I2C1_Ptr->handle_error();
}

void I2C1_EV_IRQHandler(void) {
  I2C1_Ptr->handle_event();
}

void I2C2_ER_IRQHandler(void) {
  I2C2_Ptr->handle_error();
}

void I2C2_EV_IRQHandler(void) {
  I2C2_Ptr->handle_event();
}

void I2C3_ER_IRQHandler(void) {
  I2C3_Ptr->handle_error();
}

void I2C3_EV_IRQHandler(void) {
  I2C3_Ptr->handle_event();
}

}
