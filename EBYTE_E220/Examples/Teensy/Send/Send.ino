#include "EBYTE_E220.h"

#define ESerial Serial1

#define PIN_M0 A7
#define PIN_M1 A7
#define PIN_AX A0

// i recommend putting this code in a .h file and including it
// from both the receiver and sender modules

// these are just dummy variables, replace with your own

struct DATA {
  unsigned long Count;
  int Bits;
  float Volts;
  float Amps;
};

int i = 0;
int Chan;
DATA MyData;

// create the transceiver object, passing in the serial and pins
EBYTE_E220 Transceiver(&ESerial, PIN_M0, PIN_M1, PIN_AX);

void setup() {

  Serial.begin(9600);

  while (!Serial) {}

  // start the transceiver serial port--i have yet to get a different
  // baud rate to work--data sheet says to keep on 9600

  ESerial.begin(9600);

  Serial.println("Starting Sender");

  // this init will set the pinModes for you
  Serial.print("init success:");
  Serial.println(Transceiver.init());




  // all these calls are optional but shown to give examples of what you can do
  //Transceiver.setRSSIAmbientNoise(true);
  //Transceiver.setRSSISignalStrength(true);

  /// Transceiver.setAddressH(0);
   //Transceiver.setAddressL(7);

  //Chan = 2;
  // Transceiver.setChannel(Chan);

  // save the parameters to the unit,

 // Transceiver.saveParameters(EBYTE_WRITE_PERMANENT);

  // you can print all parameters and is good for debugging
  // if your units will not communicate, print the parameters
  // for both sender and receiver and make sure air rates, channel
  // and address is the same
  Transceiver.printParameters();

    //Serial.print("retry :");
  //Serial.println(Transceiver.restore());

  
  //Transceiver.printParameters();
/*
  Serial.print("Ambient Noise: ");
  Serial.print(Transceiver.readRSSIAmbientNoise());
  Serial.println("db");
  Serial.print("Frequency       : ");
  Serial.print(Transceiver.getTransmitFrequency());
  Serial.println("MHz");
*/
  while (1) {}
}

void loop() {

  // measure some data and save to the structure
  //MyData.Count++;
  // MyData.Bits = i++; analogRead(A0);
  // MyData.Volts = MyData.Bits * ( 5.0 / 1024.0 );

  // i highly suggest you send data using structures and not
  // a parsed data--i've always had a hard time getting reliable data using
  // a parsing method
  //ET.SendStruct(&MyData, sizeof(MyData));
  //ESerial.flush();
  // let the use know something was sent
  //Serial.print("Sending: "); Serial.println(MyData.Count);

  //delay(1000);
}
