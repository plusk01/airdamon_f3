#include "uart.h"

// pointers to UART objects for use in IRQ handlers
static UART* UART1Ptr = nullptr;

// printf function for putting a single character to screen
static void _putc(void* p, char c)
{
  UART* pUART = static_cast<UART*>(p);
  pUART->write_byte(reinterpret_cast<uint8_t*>(&c), 1);
}

// ----------------------------------------------------------------------------

UART::UART(USART_TypeDef* uart, uint32_t baudrate)
{
  USARTx_ = uart;

  if (USARTx_ == USART1)
  {
    // Initialize GPIO pins for USART1
    tx_pin_.init(GPIOA, GPIO_Pin_9, GPIO::PERIPH_OUT);
    rx_pin_.init(GPIOA, GPIO_Pin_10, GPIO::PERIPH_IN);

    // Set GPIO pins as alternate function
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

    // Enable clock to USART1, an APB2 peripheral
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // Enable clock to DMA1, an AHB peripheral
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    Tx_DMA_IRQn_ = DMA1_Channel4_IRQn;

    // Assign the appropriate DMA channels that
    // are connected to the USART1 peripheral
    Tx_DMA_Channel_ = DMA1_Channel4;
    Rx_DMA_Channel_ = DMA1_Channel5;

    // Store the pointer to this object for use in IRQ handlers
    UART1Ptr = this;
  }

  init_DMA();
  init_UART(baudrate);
  init_NVIC();
  init_printf(this, _putc);
}

// ----------------------------------------------------------------------------

void UART::write_byte(uint8_t* ch, uint8_t len)
{
  // Put data into the tx_buffer
  for (uint8_t i=0; i<len; i++)
  {
    tx_buffer_[tx_buffer_head_] = ch[i];

    // Move the head of the buffer, wrapping around if necessary
    tx_buffer_head_ = (tx_buffer_head_ + 1) % TX_BUFFER_SIZE;
  }

  // if (DMA_GetCmdStatus(Tx_DMA_Channel_) != ENABLE)
  // Ensure that the DMA transfer is finished (by making sure the channel
  // is disabled) before trying to initiate a new transfer
  if (!(Tx_DMA_Channel_->CCR & DMA_CCR_EN))
    start_DMA_transfer();
}

// ----------------------------------------------------------------------------

void UART::start_DMA_transfer()
{
  // Set the start of the transmission to the oldest data
  Tx_DMA_Channel_->CMAR = (uint32_t)&tx_buffer_[tx_buffer_tail_];

  // Check to see if the data in the buffer is contiguous or not
  // (i.e., has it wrapped around the end of the buffer?)
  if (tx_buffer_head_ > tx_buffer_tail_)
  {
    // Set the length of the transmission to the data on the buffer.
    // If contiguous, this is easy.
    DMA_SetCurrDataCounter(Tx_DMA_Channel_, tx_buffer_head_ - tx_buffer_tail_);

    // This signifies that there is no new/usable data in the buffer
    tx_buffer_tail_ = tx_buffer_head_;
  }
  else
  {
    // We will have to send the data in two groups, first the tail,
    // then the head we will do later
    DMA_SetCurrDataCounter(Tx_DMA_Channel_, TX_BUFFER_SIZE - tx_buffer_tail_);

    // Set the tail to the beginning so that next time the leftover
    // data (which is now contiguous) will be sent.
    tx_buffer_tail_ = 0;
  }

  // Start the transmission from the Tx buffer
  // to the Tx register in the USART peripheral
  DMA_Cmd(Tx_DMA_Channel_, ENABLE);
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
  rx_buffer_head_ = DMA_GetCurrDataCounter(Rx_DMA_Channel_);

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
// Private Methods
// ----------------------------------------------------------------------------


void UART::init_UART(uint32_t baudrate)
{
  // Configure a bi-directional, 8-N-1 UART device
  USART_InitTypeDef USART_InitStruct;
  USART_InitStruct.USART_BaudRate             = baudrate;
  USART_InitStruct.USART_WordLength           = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits             = USART_StopBits_1;
  USART_InitStruct.USART_Parity               = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl  = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode                 = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTx_, &USART_InitStruct);

  // Enable interrupts on "receive buffer not empty" (i.e., byte received)
  USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  // Enable the given USART perhipheral
  USART_Cmd(USARTx_, ENABLE);
}

// ----------------------------------------------------------------------------

void UART::init_DMA()
{
  DMA_InitTypeDef DMA_InitStructure;

  //
  // USART Tx DMA Config
  //

  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t)(&(USARTx_->TDR));
  DMA_InitStructure.DMA_MemoryBaseAddr      = (uint32_t)tx_buffer_;
  DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize          = TX_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority            = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;


  // Configure the Tx DMA Channel Registers
  DMA_DeInit(Tx_DMA_Channel_);
  DMA_Init(Tx_DMA_Channel_, &DMA_InitStructure);


  //
  // USART Rx DMA Config
  //

  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t)(&(USARTx_->RDR));
  DMA_InitStructure.DMA_MemoryBaseAddr      = (uint32_t)rx_buffer_;
  DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize          = RX_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode                = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority            = DMA_Priority_Medium;
  DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;

  // Configure the Rx DMA Channel Registers
  DMA_DeInit(Rx_DMA_Channel_);
  DMA_Init(Rx_DMA_Channel_, &DMA_InitStructure);


  //
  // Enable perhipherals, manage interrupts, initialize buffers, etc
  //

  //  Hook up the DMA to the uart
  USART_DMACmd(USARTx_, USART_DMAReq_Tx, ENABLE);
  USART_DMACmd(USARTx_, USART_DMAReq_Rx, ENABLE);

  // Turn on the 'transfer complete' interrupt from the Tx DMA
  DMA_ITConfig(Tx_DMA_Channel_, DMA_IT_TC, ENABLE);

  // Initialize the Rx/Tx buffers
  rx_buffer_head_ = RX_BUFFER_SIZE; // DMA counts down on receive
  rx_buffer_tail_ = RX_BUFFER_SIZE;
  tx_buffer_head_ = 0;
  tx_buffer_tail_ = 0;

  // Turn on the Rx DMA, since it is in circular mode
  DMA_Cmd(Rx_DMA_Channel_, ENABLE);
}

// ----------------------------------------------------------------------------

void UART::init_NVIC()
{
  // Configure the Nested Vector Interrupt Controller

  // DMA Tx Channel Interrupt
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = Tx_DMA_IRQn_;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}


// ----------------------------------------------------------------------------
// IRQ Handlers associated with USART and DMA
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