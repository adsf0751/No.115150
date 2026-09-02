#ifndef IPASSMICROPAYMENT_INTTYPES_H
#  define IPASSMICROPAYMENT_INTTYPES_H

#  include "IPassMicroPayment_Config.h"

#  if defined(CONFIG_INT_TYPES_STDLIB)
#    include <inttypes.h>
#  elif defined(CONFIG_INT_TYPES_HOME_MADE)

// Note: on 64-bit machines, long may be 64-bit wide instead of 32.
typedef unsigned long  uint32_t;
typedef          long  int32_t ;
typedef unsigned short uint16_t;
typedef          short int16_t ;
typedef unsigned char  uint8_t ;
typedef          char  int8_t  ;

#  define PRIi32 "ld"
#  define PRIu32 "lu"

#  else
#    error Missing integer type definitions
#  endif

#endif
