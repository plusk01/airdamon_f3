#include <led.h>

extern "C" {
#include <hw_config.h>
#include <usb_lib.h>
#include <usb_desc.h>
#include <usb_pwr.h>
}

extern __IO uint8_t Receive_Buffer[64];
extern __IO  uint32_t Receive_length ;
extern __IO  uint32_t length ;
uint8_t Send_Buffer[64];
uint32_t packet_sent=1;
uint32_t packet_receive=1;

int main()
{
  systemInit();

  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();

  LED info;
  info.init(GPIOB, GPIO_Pin_8);

  while(1)
  {
    // delay(1000);

    if (bDeviceState == CONFIGURED)
    {
      CDC_Receive_DATA();
      /*Check to see if we have data yet */
      if (Receive_length  != 0)
      {
        if (packet_sent == 1)
          CDC_Send_DATA ((unsigned char*)Receive_Buffer,Receive_length);
        info.toggle();
        Receive_length = 0;
      }
    }
  }
}
