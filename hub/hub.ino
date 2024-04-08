// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX

#include <SPI.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <WiFi101.h>
#include "arduino_secrets.h" 

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

/* Some other possible setups include:

// Feather 32u4:
#define RFM95_CS   8
#define RFM95_RST  4
#define RFM95_INT  7

// Feather M0:
#define RFM95_CS   8
#define RFM95_RST  4
#define RFM95_INT  3

// Arduino shield:
#define RFM95_CS  10
#define RFM95_RST  9
#define RFM95_INT  7

// Feather 32u4 w/wing:
#define RFM95_RST 11  // "A"
#define RFM95_CS  10  // "B"
#define RFM95_INT  2  // "SDA" (only SDA/SCL/RX/TX have IRQ!)

// Feather m0 w/wing:
#define RFM95_RST 11  // "A"
#define RFM95_CS  10  // "B"
#define RFM95_INT  6  // "D"
*/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

#define MAGNETOMETER 5
#define TRANSMISSION 10
#define RECEPTION 12

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "vcm-39821.vm.duke.edu";    // name address for Google (using DNS)
int server_port = 5000;
int space_number = 1;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

void setup() {
  pinMode(MAGNETOMETER, OUTPUT); // Magnetometer
  pinMode(TRANSMISSION, OUTPUT); // Transmission
  pinMode(RECEPTION, OUTPUT); // Reception

  pinMode(LED_BUILTIN, OUTPUT);
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

  Serial.println("Feather LoRa RX Test!");

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

  // Set up Wi-Fi radio
  WiFi.setPins(12,11,10);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();
}

void loop() {
  if (rf95.available()) {
    digitalWrite(TRANSMISSION, LOW);
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    //if (rf95.recv(buf, &len)) {
    while (!(rf95.recv(buf, &len))) {
    }
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(RECEPTION, HIGH);
    
    String receivedMessage = (char*)buf;

    if(receivedMessage.equals("Car Detected")){
      didDetectCar();
    }
    else{
      didNotDetectCar();
    }

    RH_RF95::printBuffer("Received: ", buf, len);
    Serial.print("Got: ");
    Serial.println((char*)buf);
      Serial.print("RSSI: ");
    Serial.println(rf95.lastRssi(), DEC);
    delay(3000);
    digitalWrite(RECEPTION, LOW);
    // Send a reply
    uint8_t data[] = "And hello back to you";
    digitalWrite(TRANSMISSION, HIGH);
    rf95.send(data, sizeof(data));
    rf95.waitPacketSent();
    Serial.println("Sent a reply");
    digitalWrite(LED_BUILTIN, LOW);
    } else {
      //Serial.println("Receive failed");
    }
  //}
}

void didDetectCar() {
    digitalWrite(MAGNETOMETER, HIGH);

    if (client.connect(server, server_port)) {
        Serial.println("connected to server");
        // Make a HTTP request:
        String s = "POST /update?taken=true&space="
        s += space_number;
        s += " HTTP/1.1";
        client.println("Host: vcm-39821.vm.duke.edu");
        client.println("Connection: close");
        client.println();
  }
}

void didNotDetectCar() {
    digitalWrite(MAGNETOMETER, LOW);

    if (client.connect(server, server_port)) {
        Serial.println("connected to server");
        // Make a HTTP request:
        String s = "POST /update?taken=false&space="
        s += space_number;
        s += " HTTP/1.1";
        client.println("Host: vcm-39821.vm.duke.edu");
        client.println("Connection: close");
        client.println();
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}