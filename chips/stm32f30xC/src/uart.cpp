#include "uart.h"

// pointers to UART objects for use in IRQ handlers
static airdamon::UART* UART1Ptr = nullptr;
static airdamon::UART* UART2Ptr = nullptr;

// printf function for putting a single character to screen
static void _putc(void* p, char c)
{
  airdamon::UART* pUART = static_cast<airdamon::UART*>(p);

  // if the tx buffer is full, let's wait until there is some
  // more space to prevent stomping on data in the buffer.
  while (pUART->would_stomp_dma_data());

  pUART->write(reinterpret_cast<uint8_t*>(&c), 1);
}

// ----------------------------------------------------------------------------

namespace airdamon {

void UART::init(const UARTConfig* config, uint32_t baudrate, Mode mode, bool inverted)
{
  cfg_ = config;

  // Initialize GPIO pins for USARTx
  rx_pin_.init(config->GPIOx, config->rx_pin, GPIO::PERIPH_IN);
  tx_pin_.init(config->GPIOx, config->tx_pin, GPIO::PERIPH_OUT);

  // Set GPIO pins as alternate function
  GPIO_PinAFConfig(config->GPIOx, config->rx_pin_source, config->GPIO_AF);
  GPIO_PinAFConfig(config->GPIOx, config->tx_pin_source, config->GPIO_AF);

  // Store the pointer to this object for use in IRQ handlers
  if (cfg_->USARTx == USART1)
    UART1Ptr = this;
  else if (cfg_->USARTx == USART2)
    UART2Ptr = this;

  init_DMA();
  init_UART(baudrate, mode, inverted);
  init_NVIC();
}

// ----------------------------------------------------------------------------

void UART::register_rx_callback(std::function<void(uint8_t)> cb)
{
  cb_rx_ = cb;
}

// ----------------------------------------------------------------------------

void UART::unregister_rx_callback()
{
  cb_rx_ = nullptr;
}

// ----------------------------------------------------------------------------

void UART::set_mode(uint32_t baudrate, Mode mode, bool inverted)
{
 init_UART(baudrate, mode, inverted);
}

// ----------------------------------------------------------------------------

void UART::connect_to_printf()
{
  init_printf(this, _putc);
}

// ----------------------------------------------------------------------------

void UART::write(uint8_t* ch, uint8_t len)
{
  // Put data into the tx_buffer
  for (uint8_t i=0; i<len; i++)
  {
    tx_buffer_[tx_buffer_head_] = ch[i];

    // Move the head of the buffer, wrapping around if necessary
    tx_buffer_head_ = (tx_buffer_head_ + 1) % TX_BUFFER_SIZE;
  }

  // Ensure that the DMA transfer is finished (by making sure the channel
  // is disabled) before trying to initiate a new transfer
  if (!(cfg_->Tx_DMA_Channel->CCR & DMA_CCR_EN))
    start_DMA_transfer();
}

// ----------------------------------------------------------------------------

void UART::start_DMA_transfer()
{
  // Set the start of the transmission to the oldest data
  cfg_->Tx_DMA_Channel->CMAR = reinterpret_cast<uint32_t>(&tx_buffer_[tx_buffer_tail_]);

  // save the last known DMA position (for checking data stomping)
  tx_old_DMA_pos_ = tx_buffer_tail_;

  // Check to see if the data in the buffer is contiguous or not
  // (i.e., has it wrapped around the end of the buffer?)
  if (tx_buffer_head_ > tx_buffer_tail_)
  {
    // Set the length of the transmission to the data on the buffer.
    // If contiguous, this is easy.
    DMA_SetCurrDataCounter(cfg_->Tx_DMA_Channel, tx_buffer_head_ - tx_buffer_tail_);

    // This signifies that there is no new/usable data in the buffer
    tx_buffer_tail_ = tx_buffer_head_;
  }
  else
  {
    // We will have to send the data in two groups, first the tail,
    // then the head we will do later
    DMA_SetCurrDataCounter(cfg_->Tx_DMA_Channel, TX_BUFFER_SIZE - tx_buffer_tail_);

    // Set the tail to the beginning so that next time the leftover
    // data (which is now contiguous) will be sent.
    tx_buffer_tail_ = 0;
  }

  // Start the transmission from the Tx buffer
  // to the Tx register in the USART peripheral
  DMA_Cmd(cfg_->Tx_DMA_Channel, ENABLE);
}

// ----------------------------------------------------------------------------

uint8_t UART::read_byte()
{
  uint8_t byte = 0;

  // pull the next byte off the Rx buffer
  // (the head counts down, because CNDTRx counts down)
  if (rx_buffer_head_ != rx_buffer_tail_)
  {
    // read a new byte and decrement the tail
    byte = rx_buffer_[RX_BUFFER_SIZE - rx_buffer_tail_];

    // if at the bottom of buffer, wrap to the top
    if (--rx_buffer_tail_ == 0)
      rx_buffer_tail_ = RX_BUFFER_SIZE;
  }

  return byte;
}

// ----------------------------------------------------------------------------

uint32_t UART::rx_bytes_waiting()
{
  uint32_t num_waiting_bytes = 0;

  // Get the current buffer head -- Remember, the DMA CNDTR counts down!
  rx_buffer_head_ = DMA_GetCurrDataCounter(cfg_->Rx_DMA_Channel);

  // if pointing at the same location, there are no new bytes
  if (rx_buffer_head_ == rx_buffer_tail_)
    num_waiting_bytes = 0;

  // There is contiguous data (i.e., has not wrapped to top yet)
  else if (rx_buffer_head_ < rx_buffer_tail_)
    num_waiting_bytes = rx_buffer_tail_ - rx_buffer_head_;

  // There was a wrap around, so not contiguous -- count parts on either end.
  else if (rx_buffer_head_ > rx_buffer_tail_)
    num_waiting_bytes = RX_BUFFER_SIZE - rx_buffer_head_ + rx_buffer_tail_ + 1;

  return num_waiting_bytes;
}

// ----------------------------------------------------------------------------

bool UART::tx_buffer_empty()
{
  return tx_buffer_head_ == tx_buffer_tail_;
}

// ----------------------------------------------------------------------------

bool UART::would_stomp_dma_data()
{
  bool is_DMA_processing = (cfg_->Tx_DMA_Channel->CCR & DMA_CCR_EN);
  bool would_stomp = (((tx_buffer_head_+1) % TX_BUFFER_SIZE) == tx_old_DMA_pos_);
  return is_DMA_processing && would_stomp;
}

// ----------------------------------------------------------------------------

void UART::handle_usart_irq()
{
  if (cb_rx_ != nullptr)
  {
    while (rx_bytes_waiting())
      cb_rx_(read_byte());
  }
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------


void UART::init_UART(uint32_t baudrate, Mode mode, bool inverted)
{
  // Configure a bi-directional, UART device
  USART_InitTypeDef USART_InitStruct;
  USART_InitStruct.USART_BaudRate             = baudrate;
  USART_InitStruct.USART_HardwareFlowControl  = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode                 = USART_Mode_Rx | USART_Mode_Tx;

  if (mode == Mode::m8N1)
  {
    USART_InitStruct.USART_WordLength           = USART_WordLength_8b;
    USART_InitStruct.USART_Parity               = USART_Parity_No;
    USART_InitStruct.USART_StopBits             = USART_StopBits_1;
  }
  else if (mode == Mode::m8E2)
  {
    // 9 bit UART word because of the parity bit
    USART_InitStruct.USART_WordLength           = USART_WordLength_9b;
    USART_InitStruct.USART_Parity               = USART_Parity_Even;
    USART_InitStruct.USART_StopBits             = USART_StopBits_2;
  }

  USART_Init(cfg_->USARTx, &USART_InitStruct);

  // Enable interrupts on "receive buffer not empty" (i.e., byte received)
  USART_ClearITPendingBit(cfg_->USARTx, USART_IT_RXNE);
  USART_ITConfig(cfg_->USARTx, USART_IT_RXNE, ENABLE);

  if (inverted)
    USART_InvPinCmd(USART2, USART_InvPin_Rx, ENABLE);

  // Enable the given USART perhipheral
  USART_Cmd(cfg_->USARTx, ENABLE);
}

// ----------------------------------------------------------------------------

void UART::init_DMA()
{
  DMA_InitTypeDef DMA_InitStructure;

  //
  // USART Tx DMA Config
  //

  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr  = reinterpret_cast<uint32_t>(&(cfg_->USARTx->TDR));
  DMA_InitStructure.DMA_MemoryBaseAddr      = reinterpret_cast<uint32_t>(tx_buffer_);
  DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize          = TX_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority            = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;


  // Configure the Tx DMA Channel Registers
  DMA_DeInit(cfg_->Tx_DMA_Channel);
  DMA_Init(cfg_->Tx_DMA_Channel, &DMA_InitStructure);


  //
  // USART Rx DMA Config
  //

  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr  = reinterpret_cast<uint32_t>(&(cfg_->USARTx->RDR));
  DMA_InitStructure.DMA_MemoryBaseAddr      = reinterpret_cast<uint32_t>(rx_buffer_);
  DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize          = RX_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode                = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority            = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;

  // Configure the Rx DMA Channel Registers
  DMA_DeInit(cfg_->Rx_DMA_Channel);
  DMA_Init(cfg_->Rx_DMA_Channel, &DMA_InitStructure);


  //
  // Enable perhipherals, manage interrupts, initialize buffers, etc
  //

  //  Hook up the DMA to the uart
  USART_DMACmd(cfg_->USARTx, USART_DMAReq_Tx, ENABLE);
  USART_DMACmd(cfg_->USARTx, USART_DMAReq_Rx, ENABLE);

  // Turn on the 'transfer complete' interrupt from the Tx DMA
  DMA_ITConfig(cfg_->Tx_DMA_Channel, DMA_IT_TC, ENABLE);

  // Initialize the Rx/Tx buffers
  rx_buffer_head_ = RX_BUFFER_SIZE; // DMA counts down on receive
  rx_buffer_tail_ = RX_BUFFER_SIZE;
  tx_buffer_head_ = 0;
  tx_buffer_tail_ = 0;
  tx_old_DMA_pos_ = 0;

  // Turn on the Rx DMA, since it is in circular mode
  DMA_Cmd(cfg_->Rx_DMA_Channel, ENABLE);
}

// ----------------------------------------------------------------------------

void UART::init_NVIC()
{
  // Configure the Nested Vector Interrupt Controller

  // DMA Tx Channel Interrupt
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = cfg_->Tx_DMA_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  // USART Global Interrupt
  NVIC_InitStruct.NVIC_IRQChannel = cfg_->USARTx_IRQn;
  NVIC_Init(&NVIC_InitStruct);
}

} // ns airdamon


// ----------------------------------------------------------------------------
// IRQ Handlers associated with USART and DMA
// ----------------------------------------------------------------------------

extern "C" void USART1_IRQHandler(void)
{
  // RDR has already been read (thus, RXNE cleared) because we are using DMA
  // if (USART_GetITStatus(USART1, USART_IT_RXNE))
  UART1Ptr->handle_usart_irq();
}

// ----------------------------------------------------------------------------

extern "C" void DMA1_Channel4_IRQHandler(void)
{
  // Signal that we are done with the latest DMA transfer
  DMA_Cmd(DMA1_Channel4, DISABLE);

  // If there is still data to process, start again.
  // This happens when data was added to the buffer, but we were in
  // the middle of a transfer. Now that the transfer is finished
  // (marked by disabling the DMA), we can process the buffer.
  if (!UART1Ptr->tx_buffer_empty())
    UART1Ptr->start_DMA_transfer();

  DMA_ClearITPendingBit(DMA1_IT_TC4);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

extern "C" void USART2_IRQHandler(void)
{
  // RDR has already been read (thus, RXNE cleared) because we are using DMA
  // if (USART_GetITStatus(USART2, USART_IT_RXNE))
  UART2Ptr->handle_usart_irq();
}

// ----------------------------------------------------------------------------

extern "C" void DMA1_Channel7_IRQHandler(void)
{
  // Signal that we are done with the latest DMA transfer
  DMA_Cmd(DMA1_Channel7, DISABLE);

  // If there is still data to process, start again.
  // This happens when data was added to the buffer, but we were in
  // the middle of a transfer. Now that the transfer is finished
  // (marked by disabling the DMA), we can process the buffer.
  if (!UART2Ptr->tx_buffer_empty())
    UART2Ptr->start_DMA_transfer();

  DMA_ClearITPendingBit(DMA1_IT_TC7);
}