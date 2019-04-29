#include <betafpv.h>

struct param_t {
  uint8_t a;
  uint8_t _padding[2048]; // to test crossing page boundary
  uint32_t b;
  float c;
  char d[10];
};

void printParam(param_t P)
{

  uint32_t psum = 0;
  for (int i=0; i<sizeof(P._padding); ++i) psum += P._padding[i];

  printf("Param:\n");
  printf("  .a: %d\n", P.a);
  printf("  sum of padding: %d\n", psum);
  printf("  .b: %d\n", P.b);
  printf("  .c: %d\n", static_cast<int>(1000*P.c));
  printf("  .d: %s\n", P.d);
  printf("\n");
}

int main()
{
  board_init();

  LED info;
  info.init(GPIOB, GPIO_Pin_8);

  VCP vcp;
  vcp.init();
  vcp.connect_to_printf();

  airdamon::Flash eeprom;
  eeprom.init(sizeof(param_t));


  //
  // Flash eeprom stats
  //
  //
  

  delay(3000);

  printf("Size of params: %d\n", sizeof(param_t));
  printf("Flash num page: %d\n", eeprom.getNumPages());
  printf("Flash start page: %d\n", eeprom.getStartPage());
  printf("Flash start addr: 0x%X\n", eeprom.getStartAddr());
  printf("\n");

  //
  // Read what is currently in flash eeprom
  //

  param_t P;

  void * ptrP = reinterpret_cast<void *>(&P);

  bool r1 = eeprom.read(ptrP, sizeof(param_t));
  printf("eeprom read status: %d\n\n", (r1)?1:0);
  printParam(P);

  //
  // Write something new in flash
  //

  P.a = 6;
  memset(P._padding, 0, sizeof(P._padding));
  P.b = 19910829;
  P.c = 8.625f;
  strcpy(P.d, "Parker");

  bool r2 = eeprom.write(ptrP, sizeof(param_t));

  printf("Writing ");
  printParam(P);

  printf("eeprom write status: %d\n", (r2)?1:0);

  printf("\n");
  printf("Power cycle and see if it worked!\n");
  printf("\n");

  while(1)
  {
    delay(1000);
    info.toggle();
  }
}
