#include "spi.h"

static SPI* SPI1ptr = nullptr;
static SPI* SPI2ptr = nullptr;
static SPI* SPI3ptr = nullptr;

static uint8_t dummy_buffer[256];

void SPI::init(SPI_TypeDef* SPIx/*, GPIO cs*/)
{
  SPIx_ = SPIx;
  // cs_ = cs;

  if (SPIx_ == SPI1)
  {
    // // Initialize GPIO pins for SPI1
    // sck_.init(GPIOB, GPIO_Pin_3, GPIO::PERIPH_OUT);
    // miso_.init(GPIOB, GPIO_Pin_4, GPIO::PERIPH_OUT);
    // mosi_.init(GPIOB, GPIO_Pin_5, GPIO::PERIPH_OUT);

    // // Set GPIO pins as alternate function
    // GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_5);
    // GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_5);
    // GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_5);


    // Set GPIO pins as alternate function
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_5);

    // Initialize GPIO pins for SPI1
    sck_.init(GPIOA, GPIO_Pin_5, GPIO::PERIPH_OUT);
    miso_.init(GPIOA, GPIO_Pin_6, GPIO::PERIPH_OUT);
    mosi_.init(GPIOA, GPIO_Pin_7, GPIO::PERIPH_OUT);

    // Enable clock to SPI1, an APB2 peripheral
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    SPI1ptr = this;
  }
  else if (SPIx_ == SPI2)
  {
    SPI2ptr = this;
  }
  else if (SPIx_ == SPI3)
  {
    SPI3ptr = this;
  }

  SPI_InitTypeDef SPI_InitStructure;


  // Set up the SPI peripheral
  SPI_I2S_DeInit(SPIx_);
  SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;  // 42/64 = 0.65625 MHz SPI Clock
  SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial     = 7;
  SPI_Init(SPIx_, &SPI_InitStructure);

  // SPI will generate RXNE event when Rx FIFO is a Quarter Full (QF)
  SPI_RxFIFOThresholdConfig(SPIx_, SPI_RxFIFOThreshold_QF);

  // SPI_CalculateCRC(SPIx_, DISABLE);
  
  SPI_Cmd(SPIx_, ENABLE);

  // // Wait for any transfers to clear (this should be really short if at all)
  // while (SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_TXE) == RESET);
  // SPI_I2S_ReceiveData(SPIx_); //dummy read if needed

  // // Configure the DMA
  // DMA_InitStructure_.DMA_FIFOMode = DMA_FIFOMode_Disable ;
  // DMA_InitStructure_.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
  // DMA_InitStructure_.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
  // DMA_InitStructure_.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  // DMA_InitStructure_.DMA_MemoryInc = DMA_MemoryInc_Enable;
  // DMA_InitStructure_.DMA_Mode = DMA_Mode_Normal;
  // DMA_InitStructure_.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  // DMA_InitStructure_.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  // DMA_InitStructure_.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  // DMA_InitStructure_.DMA_Channel = c_->DMA_Channel;
  // DMA_InitStructure_.DMA_PeripheralBaseAddr = (uint32_t)(&(SPIx_->DR));
  // DMA_InitStructure_.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  // DMA_InitStructure_.DMA_Priority = DMA_Priority_High;

  // // Configure the Appropriate Interrupt Routine
  // NVIC_InitTypeDef NVIC_InitStruct;
  // NVIC_InitStruct.NVIC_IRQChannel = c_->DMA_IRQn;
  // NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  // NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
  // NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x02;
  // NVIC_Init(&NVIC_InitStruct);

  transfer_cb_ = nullptr;
}

// ----------------------------------------------------------------------------

void SPI::set_divisor(uint16_t new_divisor) {
  SPI_Cmd(SPIx_, DISABLE);

  const uint16_t clearBRP = 0xFFC7;

  uint16_t temp = SPIx_->CR1;

  temp &= clearBRP;
  switch(new_divisor) {
  case 2:
    temp |= SPI_BaudRatePrescaler_2;
    break;
  case 4:
    temp |= SPI_BaudRatePrescaler_4;
    break;
  case 8:
    temp |= SPI_BaudRatePrescaler_8;
    break;
  case 16:
    temp |= SPI_BaudRatePrescaler_16;
    break;
  case 32:
    temp |= SPI_BaudRatePrescaler_32;
    break;
  case 64:
    temp |= SPI_BaudRatePrescaler_64;
    break;
  case 128:
    temp |= SPI_BaudRatePrescaler_128;
    break;
  case 256:
    temp |= SPI_BaudRatePrescaler_256;
    break;
  }
  SPIx_->CR1 = temp;
  SPI_Cmd(SPIx_, ENABLE);
}

// void SPI::enable(GPIO& cs) {
//   cs.write(GPIO::LOW);
// }

// void SPI::disable(GPIO& cs) {
//   cs.write(GPIO::HIGH);
// }

// uint8_t SPI::transfer_byte(uint8_t data, GPIO *cs)
// {
//   uint16_t spiTimeout;

//   spiTimeout = 0x1000;

//   if (cs != NULL)
//     enable(*cs);

//   while (SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_TXE) == RESET)
//   {
//     if ((--spiTimeout) == 0)
//       return false;
//   }

//   SPI_I2S_SendData(SPIx_, data);

//   spiTimeout = 0x1000;

//   while (SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_RXNE) == RESET)
//   {
//     if ((--spiTimeout) == 0)
//       return false;
//   }

//   if (cs)
//     disable(*cs);

//   return (uint8_t)SPI_I2S_ReceiveData(SPIx_);
// }

// bool SPI::transfer(uint8_t* out_data, uint32_t num_bytes, uint8_t* in_data, GPIO* cs, void (*cb)(void))
// {
//   busy_ = true;

//   // Point null arrays to dummy buffer so we have something to point to
//   if (out_data == NULL)
//     out_data = dummy_buffer;
//   if (in_data == NULL)
//     in_data = dummy_buffer;

//   // Connect with transfer callback
//   if (cb)
//     transfer_cb_ = cb;
//   else
//     transfer_cb_ = NULL;

//   // Configure the DMA
//   DMA_DeInit(c_->Tx_DMA_Stream); //SPI1_TX_DMA_STREAM
//   DMA_DeInit(c_->Rx_DMA_Stream); //SPI1_RX_DMA_STREAM

//   DMA_InitStructure_.DMA_BufferSize = num_bytes;

//   // Configure Tx DMA
//   DMA_InitStructure_.DMA_DIR = DMA_DIR_MemoryToPeripheral;
//   DMA_InitStructure_.DMA_Memory0BaseAddr = (uint32_t) out_data;
//   DMA_Init(c_->Tx_DMA_Stream, &DMA_InitStructure_);

//   // Configure Rx DMA
//   DMA_InitStructure_.DMA_DIR = DMA_DIR_PeripheralToMemory;
//   DMA_InitStructure_.DMA_Memory0BaseAddr = (uint32_t) in_data;
//   DMA_Init(c_->Rx_DMA_Stream, &DMA_InitStructure_);

//   //  Configure the Interrupt
//   DMA_ITConfig(c_->Tx_DMA_Stream, DMA_IT_TC, ENABLE);

//   if (cs != NULL)
//   {
//     enable(*cs);
//     cs_ = cs;
//   }
//   else
//     cs_ = NULL;

//   // Turn on the DMA streams
//   DMA_Cmd(c_->Tx_DMA_Stream, ENABLE);
//   DMA_Cmd(c_->Rx_DMA_Stream, ENABLE);

//   // Enable the SPI Rx/Tx DMA request
//   SPI_I2S_DMACmd(SPIx_, SPI_I2S_DMAReq_Rx, ENABLE);
//   SPI_I2S_DMACmd(SPIx_, SPI_I2S_DMAReq_Tx, ENABLE);

// }



// void SPI::transfer_complete_cb()
// {
// //  uint8_t rxne, txe;
// //  do
// //  {
// //    rxne = SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_RXNE);
// //    txe =  SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_TXE);
// //  }while (rxne == RESET || txe == RESET);

//   disable(*cs_);
//   DMA_ClearFlag(c_->Tx_DMA_Stream, c_->Tx_DMA_TCIF);
//   DMA_ClearFlag(c_->Rx_DMA_Stream, c_->Rx_DMA_TCIF);

//   DMA_Cmd(c_->Tx_DMA_Stream, DISABLE);
//   DMA_Cmd(c_->Rx_DMA_Stream, DISABLE);

//   SPI_I2S_DMACmd(SPIx_, SPI_I2S_DMAReq_Rx, DISABLE);
//   SPI_I2S_DMACmd(SPIx_, SPI_I2S_DMAReq_Tx, DISABLE);

// //  SPI_Cmd(SPIx_, DISABLE);

//   if (cs_ != NULL)
//   {
//     disable(*cs_);
//   }

//   busy_ = false;
//   if (transfer_cb_ != NULL)
//     transfer_cb_();
// }

// extern "C"
// {

// void DMA2_Stream3_IRQHandler()
// {
//   if (DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3))
//   {
//     DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3);
//     SPI1ptr->transfer_complete_cb();
//   }
// }

// void DMA1_Stream4_IRQHandler()
// {
//   if (DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4))
//   {
//     DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
//     SPI2ptr->transfer_complete_cb();
//   }
// }

// void DMA1_Stream5_IRQHandler()
// {
//   if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5))
//   {
//     DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
//     SPI3ptr->transfer_complete_cb();
//   }
// }

// }
