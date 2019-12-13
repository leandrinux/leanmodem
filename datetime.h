#ifndef datetime_h
#define datetime_h

#include "Arduino.h"
#include <NTPClient.h>

String getFullFormattedTime(NTPClient *timeClient);

#endif
