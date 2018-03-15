#ifndef RC_H
#define RC_H

#include <stdint.h>

namespace airdamon { namespace sensors {

  class RC
  {
  public:
    virtual float read(uint8_t channel) = 0;
    virtual bool lost() = 0;
    
  };

}}

#endif