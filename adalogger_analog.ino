#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Set the pins used
#define cardSelect 4

File logfile;
Adafruit_ADS1115 ads1115;

// blink out an error code
void error(uint8_t errno) {
  while (1) {
    uint8_t i;
    for (i = 0; i < errno; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (i = errno; i < 10; i++) {
      delay(200);
    }
  }
}

// This line is not needed if you have Adafruit SAMD board package 1.6.2+
//   #define Serial SerialUSB

void setup() {
  Serial.begin(115200);
  randomSeed(128);
  
  ads1115.begin();

  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(13, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2);
  }
  char filename[15];
  strcpy(filename, "/ANALOG00.CSV");
  for (uint8_t i = 0; i < 100; i++) {
    filename[7] = '0' + i/10;
    filename[8] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    Serial.print("Couldnt create ");
    Serial.println(filename);
    error(3);
  }
  Serial.print("Writing to ");
  Serial.println(filename);

  logfile.println("Time, ADC, Volts");
}

long LoggerTimestamp = 0L;  // ms
int LoggerInterval = 1;    //ms

long LoggerFlushTimestamp = 0L;  // ms
int LoggerFlushInterval = 1000;  // ms;

void loop() {
  long currentMillis = millis();
  if (LoggerTimestamp <= currentMillis) {
    digitalWrite(8, HIGH);
    int16_t adc0 = ads1115.readADC_SingleEnded(0);
    float volts0 = ads1115.computeVolts(adc0);
    logfile.print(currentMillis);
    logfile.print(",");
    logfile.print(adc0);
    logfile.print(",");
    logfile.println(volts0);
    if (LoggerFlushTimestamp <= currentMillis) {
      logfile.flush();
      LoggerFlushTimestamp = millis() + LoggerFlushInterval;
    }
    digitalWrite(8, LOW);
    LoggerTimestamp = millis() + LoggerInterval;
  }
}