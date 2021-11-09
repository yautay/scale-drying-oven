#include <Esp.h>

#define NTC_PIN A0
#define HEATER_PIN D5
#define NTC 100000

const char* ssid = "minis-heater";
const char* password = "12345678";

unsigned long timerDelay = 2000;

unsigned int histeresis_bed = 2;