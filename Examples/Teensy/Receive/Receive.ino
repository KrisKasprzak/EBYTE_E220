/*

  This example shows how to connect to an EBYTE transceiver
  using a Teensy 3.2

  This code for for the receiver

  connections
  Module      Teensy
  M0          3
  M1          4
  Rx          1 (MCU Tx line)
  Tx          0 (MCU Rx line)
  Aux         2
  Vcc         3V3 (do NOT use the onboard regualtor if using the 30db unit as it draw too much power)
  Gnd         Gnd

*/

#include "EBYTE_E220.h"

// connect to any of the Teensy Serial ports
#define ESerial Serial1

#define PIN_M0 3
#define PIN_M1 4
#define PIN_AX 2

// i recommend putting this code in a .h file and including it
// from both the receiver and sender modules

// these are just dummy variables, replace with your own
struct DATA {
  unsigned long Count;
  int Bits;
  float Volts;
  float Amps;
};

bool havedata;
int Chan;
DATA MyData;
unsigned long Last, blink;
bool state = false;


// create the transceiver object, passing in the serial and pins
EBYTE_E220 Transceiver(&ESerial, PIN_M0, PIN_M1, PIN_AX);

void setup() {

  Serial.begin(9600);

  // start the transceiver serial port--i have yet to get a different
  // baud rate to work--data sheet says to keep on 9600

  ESerial.begin(9600);

  Serial.println("Starting Reader");

  // this init will set the pinModes for you
  Transceiver.init();

  // feel like you have messed up all your settings?
  // Transceiver.restoreDefaults();

  // wanna encrypt your data (must be same with sender)
  // Transceiver.setEncryptonH(100);
  // Transceiver.setEncryptonL(200);

  // need to see how much noise is around the receiver?(use read API call later)
  // Transceiver.setRSSIAmbientNoise(true);

  // want to see transmist signal strently (use read API call later)
  // Transceiver.setRSSISignalStrength(true);

  // Chan = 15;
  // Transceiver.setChannel(Chan);

  // feel like you have messed up all your settings?
  // Transceiver.restoreDefaults();

  // save the parameters to the unit,
  // Transceiver.saveParameters(EBYTE_WRITE_PERMANENT);

  // you can print all parameters and is good for debugging
  // if your units will not communicate, print the parameters
  // for both sender and receiver and make sure air rates, channel. address, encryption (maybe other data) is the same

  Transceiver.printParameters();
}

void loop() {
  if (((millis() - blink) > 100) && havedata) {
    blink = millis();
    digitalWrite(13, state);
    state = !state;
  }
  // if the transceiver serial is available, proces incoming data
  // you can also use ESerial.available()
  if (ESerial.available()) {
    havedata = true;
    // i highly suggest you send data using structures and not
    // a parsed data--i've always had a hard time getting reliable data using
    // a parsing method
    ESerial.readBytes((uint8_t*)&MyData, (uint8_t)sizeof(MyData));

    // dump out what was just received
    Serial.print("Count: ");
    Serial.println(MyData.Count);
    Serial.print("Bits: ");
    Serial.println(MyData.Bits);
    Serial.print("Volts: ");
    Serial.println(MyData.Volts);
    Serial.print("Signal strength: ");
    Serial.print(Transceiver.readRSSISignalStrength());
    // Serial.print(-(256-ESerial.read()));
    Serial.println(" db");
    ESerial.read();

    // if you got data, update the checker
    Last = millis();

  } else {
    // if the time checker is over some prescribed amount
    // let the user know there is no incoming data
    if ((millis() - Last) > 1000) {
      blink = millis();
      digitalWrite(13, HIGH);
      havedata = false;
      Serial.println("Searching: ");
      Last = millis();
    }
  }
}
