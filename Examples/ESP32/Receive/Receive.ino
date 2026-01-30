#include "EBYTE.h"
#include <SoftwareSerial.h>

/*
WARNING: IF USING AN ESP32
DO NOT USE THE PIN NUMBERS PRINTED ON THE BOARD
YOU MUST USE THE ACTUAL GPIO NUMBER
*/
#define PIN_RX 13  // Serial2 RX (connect this to the EBYTE Tx pin)
#define PIN_TX 15  // Serial2 TX pin (connect this to the EBYTE Rx pin)

#define PIN_M0 2   // D4 on the board (possibly pin 24)
#define PIN_M1 0   // D2 on the board (possibly called pin 22)
#define PIN_AX 12  // D15 on the board (possibly called pin 21)


// i recommend putting this code in a .h file and including it
// from both the receiver and sender modules
struct DATA {
  unsigned long Count;
  int Bits;
  float Volts;
  float Amps;
};

// these are just dummy variables, replace with your own
int Chan;
DATA MyData;
unsigned long Last;

SoftwareSerial Serial_1(PIN_RX, PIN_TX);  // MCU Rx Tx connect to EBYTE Tx Rx


// create the transceiver object, passing in the serial and pins
EBYTE Transceiver(&Serial_1, PIN_M0, PIN_M1, PIN_AX);

void setup() {


  Serial.begin(9600);

  Serial_1.begin(9600);

  Serial.println("Starting Reader");

  // this init will set the pinModes for you
  Serial.println(Transceiver.init());

  // all these calls are optional but shown to give examples of what you can do

  // Serial.println(Transceiver.GetAirDataRate());
  // Serial.println(Transceiver.GetChannel());
  // Transceiver.SetAddressH(0);
  //Transceiver.SetAddressL(0);
  //Chan = 16;
  //Transceiver.SetChannel(Chan);
  //Transceiver.SetAirDataRate(0);
  // save the parameters to the unit,
  //Transceiver.SaveParameters(PERMANENT);

  // you can print all parameters and is good for debugging
  // if your units will not communicate, print the parameters
  // for both sender and receiver and make sure air rates, channel
  // and address is the same
  Transceiver.PrintParameters();
}

void loop() {

  // if the transceiver serial is available, proces incoming data
  // you can also use ESerial.available()
  if (Transceiver.available()) {

    // i highly suggest you send data using structures and not
    // a parsed data--i've always had a hard time getting reliable data using
    // a parsing method
    Transceiver.GetStruct(&MyData, sizeof(MyData));

    // dump out what was just received
    Serial.print("Count: ");
    Serial.println(MyData.Count);
    Serial.print("Bits: ");
    Serial.println(MyData.Bits);
    Serial.print("Volts: ");
    Serial.println(MyData.Volts);
    // if you got data, update the checker
    Last = millis();
  } else {
    // if the time checker is over some prescribed amount
    // let the user know there is no incoming data
    if ((millis() - Last) > 1000) {
      Serial.println("Searching: ");
      Last = millis();
    }
  }
}