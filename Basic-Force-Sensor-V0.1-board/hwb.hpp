#pragma once

#include <Arduino.h>

inline void setupHwbInput( bool pullup = false ) {
  const uint8_t pinNum = 2;
  DDRE &= ~( 1 << pinNum );
  if ( pullup )
    PORTE |= 1 << pinNum;
  else
    PORTE &= ~( 1 << pinNum );
}

inline bool readHwb() {
  const uint8_t pinNum = 2;
  return PINE & ( 1 << pinNum );
}
