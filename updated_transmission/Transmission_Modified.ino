// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_ICM20948 icm;
uint16_t measurement_delay_us = 65535; // Delay between measurements for testing

// First 3 here are boards w/radio BUILT-IN. Boards using FeatherWing follow.
#if defined (__AVR_ATmega32U4__)  // Feather 32u4 w/Radio
  #define RFM95_CS    8
  #define RFM95_INT   7
  #define RFM95_RST   4

#elif defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)  // Feather M0 w/Radio
  #define RFM95_CS    8
  #define RFM95_INT   3
  #define RFM95_RST   4

#elif defined(ARDUINO_ADAFRUIT_FEATHER_RP2040_RFM)  // Feather RP2040 w/Radio
  #define RFM95_CS   16
  #define RFM95_INT  21
  #define RFM95_RST  17

#elif defined (__AVR_ATmega328P__)  // Feather 328P w/wing
  #define RFM95_CS    4  //
  #define RFM95_INT   3  //
  #define RFM95_RST   2  // "A"

#elif defined(ESP8266)  // ESP8266 feather w/wing
  #define RFM95_CS    2  // "E"
  #define RFM95_INT  15  // "B"
  #define RFM95_RST  16  // "D"

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_NRF52840_FEATHER) || defined(ARDUINO_NRF52840_FEATHER_SENSE)
  #define RFM95_CS   10  // "B"
  #define RFM95_INT   9  // "A"
  #define RFM95_RST  11  // "C"

#elif defined(ESP32)  // ESP32 feather w/wing
  #define RFM95_CS   33  // "B"
  #define RFM95_INT  27  // "A"
  #define RFM95_RST  13

#elif defined(ARDUINO_NRF52832_FEATHER)  // nRF52832 feather w/wing
  #define RFM95_CS   11  // "B"
  #define RFM95_INT  31  // "C"
  #define RFM95_RST   7  // "A"

#endif


// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

#define MAGNETOMETER 5
#define TRANSMISSION 10
#define RECEPTION 12

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {

  // Try to initialize!
  if (!icm.begin_I2C()) {
    // if (!icm.begin_SPI(ICM_CS)) {
    // if (!icm.begin_SPI(ICM_CS, ICM_SCK, ICM_MISO, ICM_MOSI)) {

    Serial.println("Failed to find ICM20948 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("ICM20948 Found!");
  // icm.setMagDataRate(AK09916_MAG_DATARATE_10_HZ);
  Serial.print("Magnetometer data rate set to: ");
  switch (icm.getMagDataRate()) {
  case AK09916_MAG_DATARATE_SHUTDOWN:
    Serial.println("Shutdown");
    break;
  case AK09916_MAG_DATARATE_SINGLE:
    Serial.println("Single/One shot");
    break;
  case AK09916_MAG_DATARATE_10_HZ:
    Serial.println("10 Hz");
    break;
  case AK09916_MAG_DATARATE_20_HZ:
    Serial.println("20 Hz");
    break;
  case AK09916_MAG_DATARATE_50_HZ:
    Serial.println("50 Hz");
    break;
  case AK09916_MAG_DATARATE_100_HZ:
    Serial.println("100 Hz");
    break;
  }
  Serial.println();

  pinMode(MAGNETOMETER, OUTPUT); // Magnetometer
  pinMode(TRANSMISSION, OUTPUT); // Transmission
  pinMode(RECEPTION, OUTPUT); // Reception

  digitalWrite(RFM95_RST, HIGH);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  digitalWrite(MAGNETOMETER, HIGH);
  digitalWrite(TRANSMISSION, HIGH);
  digitalWrite(RECEPTION, HIGH);

  delay(100);

  digitalWrite(MAGNETOMETER, LOW);
  digitalWrite(TRANSMISSION, LOW);
  digitalWrite(RECEPTION, LOW);

  Serial.begin(115200);
  delay(100);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

float readMagnetometer() {
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t mag;
  sensors_event_t temp;
  icm.getEvent(&accel, &gyro, &temp, &mag);
  Serial.print("\t\tMag X: ");
  Serial.print(mag.magnetic.x);
  Serial.print(" \tY: ");
  Serial.print(mag.magnetic.y);
  Serial.print(" \tZ: ");
  Serial.print(mag.magnetic.z);
  Serial.println(" uT");
  return mag.magnetic.y;
}

bool isCar(){
  float magval = readMagnetometer();
  if(magval < 10.0){
    return true;
  }
  return false;
}



int16_t packetnum = 0;  // packet counter, we increment per xmission
char carpacket[20];

void loop() {
  delay(5000); // Wait 10 seconds between transmits, could also 'sleep' here!
  bool carval = isCar();
  if(carval){
    digitalWrite(MAGNETOMETER, HIGH);
    strcpy(carpacket, "Car Detected"); // Use strcpy to copy the string into carpacket
  }
  else{
    digitalWrite(MAGNETOMETER, LOW);
    strcpy(carpacket, "No Car Detected"); // Use strcpy to copy the string into carpacket
  }

  digitalWrite(RECEPTION, LOW);
  digitalWrite(TRANSMISSION, HIGH);
  Serial.println("Transmitting..."); // Send a message to rf95_server

  itoa(packetnum++, carpacket+15, 10);
  Serial.print("Sending "); 
  Serial.println(carpacket);
  //radiopacket[19] = 0;

  Serial.println("Sending...");
  delay(10);
  rf95.send((uint8_t *)carpacket, 20);

  Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  delay(3000);
  digitalWrite(TRANSMISSION, LOW);
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply...");
  // Wait until message enters the buffer
  while (!(rf95.recv(buf, &len))) {
    }
  // Have broken out of the while loop, so message must be in the buffer
  digitalWrite(RECEPTION, HIGH);

  Serial.print("Got reply: ");
  Serial.println((char*)buf);
  Serial.print("RSSI: ");
  Serial.println(rf95.lastRssi(), DEC);
// loop bracket below
}

