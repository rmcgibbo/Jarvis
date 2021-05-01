#pragma once
// #include "ESPTelnet.h"
#include <utility>

struct IO {    
    void setup(bool useWifi = true);
    void run();

    // ESPTelnet telnet_;
};

extern IO io;


size_t LOG(const char *format, ...);

template <typename S>
void LOG(const S s) noexcept
{
  Serial.println(s);
  //io.telnet_.println(s);
}