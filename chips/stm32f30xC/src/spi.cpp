#include "spi.h"

static airdamon::SPI* SPI1ptr = nullptr;
static airdamon::SPI* SPI2ptr = nullptr;
static airdamon::SPI* SPI3ptr = nullptr;

namespace airdamon {

static uint8_t dummy_buffer[256];

void SPI::init(const SPIConfig* config)
{
  cfg_ = config;

  // Initialize GPIO pins for the requested SPI peripheral
  sck_.init(cfg_->GPIOx, cfg_->sck_pin, GPIO::PERIPH_OUT);
  miso_.init(cfg_->GPIOx, cfg_->miso_pin, GPIO::PERIPH_OUT);
  mosi_.init(cfg_->GPIOx, cfg_->mosi_pin, GPIO::PERIPH_OUT);

  // Set GPIO pins as alternate function
  GPIO_PinAFConfig(cfg_->GPIOx, cfg_->sck_pin_source, cfg_->GPIO_AF);
  GPIO_PinAFConfig(cfg_->GPIOx, cfg_->miso_pin_source, cfg_->GPIO_AF);
  GPIO_PinAFConfig(cfg_->GPIOx, cfg_->mosi_pin_source, cfg_->GPIO_AF);

  if (cfg_->SPIx == SPI1)
    SPI1ptr = this;
  else if (cfg_->SPIx == SPI2)
    SPI2ptr = this;
  else if (cfg_->SPIx == SPI3)
    SPI3ptr = this;

  init_DMA();
  init_SPI();
  init_NVIC();
}

// ----------------------------------------------------------------------------

void SPI::set_divisor(uint16_t new_divisor) {
  SPI_Cmd(cfg_->SPIx, DISABLE);

  constexpr uint16_t clearBRP = 0xFFC7;

  uint16_t temp = cfg_->SPIx->CR1;

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
  cfg_->SPIx->CR1 = temp;
  SPI_Cmd(cfg_->SPIx, ENABLE);
}

// ----------------------------------------------------------------------------

uint8_t SPI::transfer_byte(uint8_t data)
{
  // prevent being interrupted
  busy_ = true;

  uint16_t spiTimeout = 0x1000;

  // Loop while DR register is not empty
  while (SPI_I2S_GetFlagStatus(cfg_->SPIx, SPI_I2S_FLAG_TXE) == RESET)
    if ((--spiTimeout) == 0) return 0x00;

  // send the byte of data
  SPI_SendData8(cfg_->SPIx, data);

  // wait to receive a byte
  spiTimeout = 0x1000;
  while (SPI_I2S_GetFlagStatus(cfg_->SPIx, SPI_I2S_FLAG_RXNE) == RESET)
    if ((--spiTimeout) == 0) return 0x00;

  busy_ = false;

  // We received a byte from the SPI bus
  return SPI_ReceiveData8(cfg_->SPIx);
}

// ----------------------------------------------------------------------------

void SPI::transfer(uint8_t *tx_data, uint32_t num_bytes, uint8_t *rx_data, std::function<void(void)> cb)
{
  // use DMA to transmit an array of bytes
  busy_ = true;

  // Point null arrays to dummy buffer so we have something to point to
  if (tx_data == nullptr)
    tx_data = dummy_buffer;
  if (rx_data == nullptr)
    rx_data = dummy_buffer;

  // Connect with transfer callback
  if (cb)
    cb_ = cb;
  else
    cb_ = nullptr;

  // Configure the DMA
  DMA_DeInit(cfg_->Tx_DMA_Channel);
  DMA_DeInit(cfg_->Rx_DMA_Channel);

  // let the DMA know how many bytes are to be transfered for Rx and Tx
  DMA_InitStructure_.DMA_BufferSize = num_bytes;

  // Configure Tx DMA
  DMA_InitStructure_.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure_.DMA_MemoryBaseAddr = reinterpret_cast<uint32_t>(tx_data);
  DMA_Init(cfg_->Tx_DMA_Channel, &DMA_InitStructure_);

  // Configure Rx DMA
  DMA_InitStructure_.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure_.DMA_MemoryBaseAddr = reinterpret_cast<uint32_t>(rx_data);
  DMA_Init(cfg_->Rx_DMA_Channel, &DMA_InitStructure_);

  //  Configure the "transfer complete" interrupt (for Rx)
  DMA_ITConfig(cfg_->Rx_DMA_Channel, DMA_IT_TC, ENABLE);

  // Turn on the DMA streams
  DMA_Cmd(cfg_->Tx_DMA_Channel, ENABLE);
  DMA_Cmd(cfg_->Rx_DMA_Channel, ENABLE);

  // Enable the SPI Rx/Tx DMA request
  SPI_I2S_DMACmd(cfg_->SPIx, SPI_I2S_DMAReq_Rx, ENABLE);
  SPI_I2S_DMACmd(cfg_->SPIx, SPI_I2S_DMAReq_Tx, ENABLE);
}

// ----------------------------------------------------------------------------

void SPI::transfer_complete_isr()
{
  // Disable the DMA transfer and wait for the next transfer to be requested.
  DMA_Cmd(cfg_->Tx_DMA_Channel, DISABLE);
  DMA_Cmd(cfg_->Rx_DMA_Channel, DISABLE);

  SPI_I2S_DMACmd(cfg_->SPIx, SPI_I2S_DMAReq_Rx, DISABLE);
  SPI_I2S_DMACmd(cfg_->SPIx, SPI_I2S_DMAReq_Tx, DISABLE);

  // there is no longer a DMA transfer happening
  busy_ = false;

  // if the caller asked for it, let them know that the DMA transfer is complete
  if (cb_) cb_();
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void SPI::init_DMA()
{
  // initialize the DMA struct, but don't initialize the peripheral.
  // The peripheral will be fully initialized and started when a transfer
  // is requested.

  DMA_StructInit(&DMA_InitStructure_);
  DMA_InitStructure_.DMA_PeripheralBaseAddr  = reinterpret_cast<uint32_t>(&(cfg_->SPIx->DR));
  // DMA_InitStructure_.DMA_MemoryBaseAddr      = < will be set for each SPI::transfer() >
  // DMA_InitStructure_.DMA_DIR                 = < will be set for each SPI::transfer() >
  // DMA_InitStructure_.DMA_BufferSize          = < will be set for each SPI::transfer() >
  DMA_InitStructure_.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
  DMA_InitStructure_.DMA_MemoryInc           = DMA_MemoryInc_Enable;
  DMA_InitStructure_.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure_.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;
  DMA_InitStructure_.DMA_Mode                = DMA_Mode_Normal;
  DMA_InitStructure_.DMA_Priority            = DMA_Priority_High;
  DMA_InitStructure_.DMA_M2M                 = DMA_M2M_Disable;
}

// ----------------------------------------------------------------------------

void SPI::init_SPI()
{
  SPI_InitTypeDef SPI_InitStructure;

  // Set up the SPI peripheral
  SPI_I2S_DeInit(cfg_->SPIx);
  SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;

  // inactive clock is high, data is transitioned on the failing (1st) clk edge, latched on rising (2nd) clk edge
  SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;

  SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;  // 72/8 = 9 MHz SPI Clock (SystemCoreClock == 72)
  SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial     = 7;
  SPI_Init(cfg_->SPIx, &SPI_InitStructure);

  // SPI will generate RXNE event when Rx FIFO is a Quarter Full (QF)
  SPI_RxFIFOThresholdConfig(cfg_->SPIx, SPI_RxFIFOThreshold_QF);

  // SPI_CalculateCRC(cfg_->SPIx, DISABLE);
  
  SPI_Cmd(cfg_->SPIx, ENABLE);
}

// ----------------------------------------------------------------------------

void SPI::init_NVIC()
{
  // Configure the appropriate interrupt routine
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = cfg_->Rx_DMA_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x02;
  NVIC_Init(&NVIC_InitStruct);
}

} // ns airdamon

// ----------------------------------------------------------------------------
// IRQ Handlers associated with SPI and DMA
// ----------------------------------------------------------------------------

// DMA SPI Rx: SPI1 to receive buffer complete
extern "C" void DMA1_Channel2_IRQHandler()
{
  if (DMA_GetITStatus(DMA1_IT_TC2))
  {
    DMA_ClearITPendingBit(DMA1_IT_TC2);
    SPI1ptr->transfer_complete_isr();
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------