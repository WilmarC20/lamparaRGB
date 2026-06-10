#ifndef LAMP_LOG_H
#define LAMP_LOG_H

#include "config.h"

#ifndef LAMP_SERIAL_LOG
#define LAMP_SERIAL_LOG 0
#endif

#if LAMP_SERIAL_LOG
#include <Arduino.h>
#define LAMP_LOG(...)    Serial.printf(__VA_ARGS__)
#define LAMP_LOG_LN(msg) Serial.println(msg)
#else
#define LAMP_LOG(...)    ((void)0)
#define LAMP_LOG_LN(msg) ((void)0)
#endif

#endif
