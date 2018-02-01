#include "uart.h"

// pointers to UART objects for use in IRQ handlers
static UART* UART1Ptr = nullptr;

// ----------------------------------------------------------------------------
// IRQ Handlers associated with USART and DMA
// ----------------------------------------------------------------------------

extern "C" void USART1_IRQHandler(void)
{

  UART1Ptr->toggleLED();

  (char)USART_ReceiveData(USART1);

  // USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  // USART_ClearITPendingBit(USART1, USART_IT_ORE);
}

extern "C" void DMA1_Channel4_IRQHandler(void)
{
  // Signal that we are done with the latest DMA transfer
  DMA_Cmd(DMA1_Channel4, DISABLE);

  // If there is still data to process, start again.
  // This happens when data was added to the buffer, but we were in
  // the middle of a transfer. Now that the transfer is finished
  // (marked by disabling the DMA), we can process the buffer.
  if (!UART1Ptr->tx_buffer_empty())
    UART1Ptr->startDMA();

  DMA_ClearITPendingBit(DMA1_IT_TC4);
}

// ============================================================================

static void _putc(void* p, char c)
{
  UART* pUART = static_cast<UART*>(p);
  pUART->write(reinterpret_cast<uint8_t*>(&c), 1);
  // pUART->put_byte_polling(c);

  // USART_SendData(USART1, c);
}

// ----------------------------------------------------------------------------

UART::UART(USART_TypeDef *_uart)
{
  dev_ = _uart;

  if (dev_ == USART1)
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


    USARTx_IRQn_ = USART1_IRQn;
    TxDMAIRQ_ = DMA1_Channel4_IRQn;
    RxDMAIRQ_ = DMA1_Channel5_IRQn;
    UART1Ptr = this;

    // Assign the appropriate DMA channels that
    // are connected to the USART1 peripheral
    Tx_DMA_Channel_ = DMA1_Channel4;
    Rx_DMA_Channel_ = DMA1_Channel5;

  }

  led_.init(GPIOE, GPIO_Pin_14);

  init_DMA();
  init_UART(115200);
  init_NVIC();
  init_printf(this, _putc);

  receive_CB_ = nullptr;
}

void UART::toggleLED() {
  led_.toggle();
}

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
  USART_Init(dev_, &USART_InitStruct);

  // Enable interrupts on "receive buffer not empty" (i.e., byte received)
  USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  // Enable the given USART perhipheral
  USART_Cmd(dev_, ENABLE);
}

// ----------------------------------------------------------------------------

void UART::init_DMA()
{
  DMA_InitTypeDef DMA_InitStructure;

  //
  // USART Tx DMA Config
  //

  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t)(&(dev_->TDR));
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
  DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t)(&(dev_->RDR));
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
  USART_DMACmd(dev_, USART_DMAReq_Tx, ENABLE);
  USART_DMACmd(dev_, USART_DMAReq_Rx, ENABLE);

  // Turn on the transfer complete interrupt source from the DMA
  DMA_ITConfig(Tx_DMA_Channel_, DMA_IT_TC, ENABLE);
  // DMA_ITConfig(Rx_DMA_Channel_, DMA_IT_TC, ENABLE);

  // Turn on the DMA
  // DMA_Cmd(Rx_DMA_Channel_, ENABLE);
  // DMA_Cmd(Tx_DMA_Channel_, ENABLE);

  // set the buffer pointers to where the DMA is starting (starts at 256 and counts down)
  // rx_buffer_tail_ = DMA_GetCurrDataCounter(Rx_DMA_Channel_);
  rx_buffer_head_ = DMA_GetCurrDataCounter(Rx_DMA_Channel_);

  // Initialize the Rx/Tx buffers
  rx_buffer_head_ = RX_BUFFER_SIZE; // DMA counts down on receive
  rx_buffer_tail_ = RX_BUFFER_SIZE;
  tx_buffer_head_ = 0;
  tx_buffer_tail_ = 0;
}

// ----------------------------------------------------------------------------

void UART::init_NVIC()
{
  //
  // Configure the Nested Vector Interrupt Controller
  //

  // USART Interrupts
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = USARTx_IRQn_;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  // NVIC_Init(&NVIC_InitStruct);

  // DMA Tx Channel Interrupt
  NVIC_InitStruct.NVIC_IRQChannel = TxDMAIRQ_;
  NVIC_Init(&NVIC_InitStruct);

  // DMA Rx Channel Interrupt
  // NVIC_InitStruct.NVIC_IRQChannel = RxDMAIRQ_;
  // NVIC_Init(&NVIC_InitStruct);
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

  // if (DMA_GetCmdStatus(Tx_DMA_Channel_) != ENABLE)
  // Ensure that the DMA transfer is finished (by making sure the channel
  // is disabled) before trying to initiate a new transfer
  if (!(Tx_DMA_Channel_->CCR & DMA_CCR_EN))
    startDMA();
}

// ----------------------------------------------------------------------------

void UART::startDMA()
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
  // pull the next byte off the array
  // (the head counts down, because CNTR counts down)
  if(rx_buffer_head_ != rx_buffer_tail_)
  {
    // read a new byte and decrement the tail
    byte = rx_buffer_[RX_BUFFER_SIZE - rx_buffer_tail_];
    if(--rx_buffer_tail_ == 0)
    {
      // wrap to the top if at the bottom
      rx_buffer_tail_ = RX_BUFFER_SIZE;
    }
  }
  return byte;
}


void UART::put_byte_polling(uint8_t c)
{
  // Naive (and slow) polling implementation of sending a byte

  // wait for the TX Register to be empty...
  while (!USART_GetFlagStatus(dev_, USART_FLAG_TXE));

  // Add the next data word to the TX register
  USART_SendData(USART1, c);
}

uint32_t UART::rx_bytes_waiting()
{
  // Remember, the DMA CNDTR counts down
  rx_buffer_head_ = DMA_GetCurrDataCounter(Rx_DMA_Channel_);
  if (rx_buffer_head_ < rx_buffer_tail_)
  {
    // Easy, becasue it's contiguous
    return rx_buffer_tail_ - rx_buffer_head_;
  }
  else if (rx_buffer_head_ > rx_buffer_tail_)
  {
    // Add the parts on either end of the buffer
    // I'm pretty sure this is wrong
    return rx_buffer_tail_ + RX_BUFFER_SIZE - rx_buffer_head_;
  }
  else
  {
    return 0;
  }
}

uint32_t UART::tx_bytes_free()
{
  tx_buffer_head_ = DMA_GetCurrDataCounter(Tx_DMA_Channel_);
  if (tx_buffer_head_ > tx_buffer_tail_)
  {
    return rx_buffer_head_ - rx_buffer_tail_;
  }
  else if (tx_buffer_head_ < tx_buffer_tail_)
  {
    // Add the parts on either end of the buffer
    // I'm pretty sure this is wrong
    return tx_buffer_head_ + RX_BUFFER_SIZE - rx_buffer_tail_;
  }
  else
  {
    return 0;
  }
}

bool UART::set_baud_rate(uint32_t baud)
{
  init_UART(baud);
}

bool UART::tx_buffer_empty()
{
  return tx_buffer_head_ == tx_buffer_tail_;
}

bool UART::set_mode(uint8_t mode){}

bool UART::flush()
{
  // uint32_t timeout = 10000;
  // while (!tx_buffer_empty() && --timeout);
  // if (timeout)
  //   return true;
  // else
  //   return false;
}

void UART::register_rx_callback(std::function<void(uint8_t)> cb)
{
  // receive_CB_ = cb;
}

void UART::unregister_rx_callback()
{
  // receive_CB_ = nullptr;
}