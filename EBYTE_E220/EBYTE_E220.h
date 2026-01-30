/*
  The MIT License (MIT)
  Copyright (c) 2019 Kris Kasrpzak
  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  On a personal note, if you develop an application or product using this library 
  and make millions of dollars, I'm happy for you!
*/

/* 
  Code by Kris Kasprzak kris.kasprzak@yahoo.com
  
  valid for E220-xxxTxxx series on LLCC68 chips
  
  This library is intended to be used with EBYTE_E220 transcievers, small wireless units for MCU's such as
  Teensy and Arduino. This library let's users program the operating parameters and both send and recieve data.
  This company makes several modules with different capabilities, but most #defines here should be compatible with them
  All constants were extracted from several data sheets and listed in binary as that's how the data sheet represented each setting
  Hopefully, any changes or additions to constants can be a matter of copying the data sheet constants directly into these #defines
  
  Usage of this library consumes around 970 bytes
  
  Revision		Data		Author			Description
  1.0			1/28/2026	Kasprzak		Initial creation

  
 
  Module connection
  Module	MCU						Description
  MO		Any digital pin*		pin to control working/program modes
  M1		Any digital pin*		pin to control working/program modes
  Rx		Any digital pin			pin to MCU TX pin (module transmits to MCU, hence MCU must recieve data from module
  Tx		Any digital pin			pin to MCU RX pin (module transmits to MCU, hence MCU must recieve data from module
  AUX		Any digital pin			pin to indicate when an operation is complete (low is busy, high is done)
  Vcc		+3v3 or 5V0				
  Vcc		Ground					Ground must be common to module and MCU		
  notes:
  * caution in connecting to Arduino pin 0 and 1 as those pins are for USB connection to PC
  you may need a 4K7 pullup to Rx and AUX pins (possibly Tx) if using and Arduino
  Module source
  http://www.ebyte.com/en/
  example module this library is intended to be used with
  http://www.ebyte.com/en/product-view-news.aspx?id=174
  Code usage
  1. Create a serial object
  2. Create EBYTE object that uses the serail object
  3. begin the serial object
  4. init the EBYTE object
  5. set parameters (optional but required if sender and reciever are different)
  6. send or listen to sent data
  
*/


// #define DEBUG

#ifndef EBYTE_E220_H_LIB
#define EBYTE_E220_H_LIB

#define EBYTE_E220_VER 1.0

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


// if you seem to get "corrupt settings add this line to your .ino
// #include <avr/io.h>

/* 
if modules don't seem to save or read parameters, it's probably due to slow pin changing times
in the module. I see this happen rarely. You will have to adjust this value
when settin M0 an M1 there is gererally a short time for the transceiver modules
to react. The data sheet says 2 ms, but more time is generally needed. I'm using
50 ms below and maybe too long, but it seems to work in most cases. Increase this value
if your unit will not return parameter settings.
*/

#define PIN_RECOVER 50 

// modes NORMAL send and recieve for example
#define EBYTE_MODE_NORMAL 0			// can send and recieve
#define MODE_WAKEUP 1			// sends a preamble to waken receiver
#define MODE_POWERDOWN 2		// can't transmit but receive works only in wake up mode
#define MODE_PROGRAM 3			// for programming

#define EBYTE_READ  0xC1
#define EBYTE_SUCCESS  0xC1
#define EBYTE_WRITE_PERMANENT  0xC0
#define EBYTE_WRITE_TEMPORARY  0xC2
	
//UART data rates
// (can be different for transmitter and reveiver)
#define UDR_1200 0b000		// 1200 baud
#define UDR_2400 0b001		// 2400 baud
#define UDR_4800 0b010		// 4800 baud
#define UDR_9600 0b011		// 9600 baud default
#define UDR_19200 0b100		// 19200 baud
#define UDR_38400 0b101		// 34800 baud
#define UDR_57600 0b110		// 57600 baud
#define UDR_115200 0b111	// 115200 baud

// parity bit options (must be the same for transmitter and reveiver)
#define PB_8N1 0b00			// default
#define PB_8O1 0b01
#define PB_8E1 0b11


// air data rates
// (must be the same for transmitter and reveiver)
#define ADR_2400_1 0b000		// 2400 baud
#define ADR_2400_2 0b001		// 2400 baud
#define ADR_2400 0b010		// 2400 baud
#define ADR_4800 0b011		// 4800 baud
#define ADR_9600 0b100		// 9600 baud
#define ADR_19200 0b101		// 19200 baud
#define ADR_38400 0b110		// 19200 baud
#define ADR_62500 0b111		// 19200 baud

// various options
#define SUB_200BYTES 0b00	//default
#define SUB_128BYTES 0b01	
#define SUB_64BYTES 0b10	
#define SUB_32BYTES 0b11	

// tranmit power (xxT22)
#define TRP_22DB 0b00	//default
#define TRP_17DB 0b01
#define TRP_13DB 0b10
#define TRP_10DB 0b11

// tranmit power (xxT30)
#define TRP_30DB 0b00	//default
#define TRP_27DB 0b01
#define TRP_24DB 0b10
#define TRP_21DB 0b11

// tranmission method
#define TRM_TRANSPARENT 0b0	//default
#define TRM_FIXEDPOINT 0b1

// wake up cycle
#define WOR_WAKEUP500  0b000
#define OPT_WAKEUP1000 0b001
#define OPT_WAKEUP1500 0b010
#define OPT_WAKEUP2000 0b011
#define OPT_WAKEUP2500 0b100
#define OPT_WAKEUP3000 0b101
#define OPT_WAKEUP3500 0b110
#define OPT_WAKEUP4000 0b111

class Stream;

class EBYTE_E220 {

public:

	EBYTE_E220(Stream *s, uint8_t PIN_M0 = 4, uint8_t PIN_M1 = 5, uint8_t PIN_AUX = 6);

	// code to initialize the library
	// this method reads all parameters from the module and stores them in memory
	// library modifications could be made to only read upon a change at a savings of 30 or so bytes
	// the issue with these modules are some parameters are a collection of several options AND
	// ALL parameters must be sent even if only one option is changed--hence get all parameters initially
	// so you know what the non changed parameters are know for resending back

	bool init();
	
	// methods to set modules working parameters NOTHING WILL BE SAVED UNLESS SaveParameters() is called
	void setMode(uint8_t mode = EBYTE_MODE_NORMAL);
	void setAddress(uint16_t val = 0);
	void setAddressH(uint8_t val = 0);
	void setAddressL(uint8_t val = 0);
	void setUARTBaudRate(uint8_t val);
	void setParityBit(uint8_t val);
	void setAirDataRate(uint8_t val);	
	void setPacketSize(uint8_t val);
	void setRSSIAmbientNoise(bool val);
	void setSoftwareModeSwitching(bool val);
	void setTransmitPower(uint8_t val);	
	void setChannel(uint8_t val);	
	void setRSSISignalStrength(bool val);
	void setTransmissionMethod(uint8_t val);
	void setLBTEnable(bool val);
	void setWORTIming(uint8_t val);	
	void setEncryptonH(uint8_t val);
	void setEncryptonL(uint8_t val);		
	bool getAux();
	
	// methods to get module data
	char *getModel();
	char *getVersion();
	uint8_t getProductInfo();

	// methods to get some operating parameters
	uint16_t getAddress();
	uint8_t getAddressH();
	uint8_t getAddressL();
	uint8_t getUARTBaudRate();
	uint8_t getParityBit();
	uint8_t getAirDataRate();	
	uint8_t getPacketSize();
	bool getRSSIAmbientNoise();
	bool getSoftwareModeSwitching();
	uint8_t getTransmitPower();	
	uint8_t getChannel();	
	bool getRSSISignalStrength();
	uint8_t getTransmissionMethod();
	bool getLBTEnable();
	uint8_t getWORTIming();	
	float getTransmitFrequency();	
	
	int16_t readRSSIAmbientNoise();	
	int16_t readRSSISignalStrength();
	
	
	
	// mehod to print parameters
	void printParameters();
	
	// parameters are set above but NOT saved, here's how you save parameters
	// notion here is you can set several but save once as opposed to saving on each parameter change
	// you can save permanently (retained at start up, or temp which is ideal for dynamically changing the address or frequency
	bool saveParameters(uint8_t val = EBYTE_WRITE_PERMANENT);
	
	// soft rebool
	bool reset();
	
	// restore EBYTE to factory defaults
	bool restoreDefaults();
	
	

private:

	bool ReadParameters();
	bool ReadModel();
	bool ReadVersion();
	void ClearBuffer();
	void BuildREG0();
	void BuildREG1();		
	void BuildREG3();
	// method to let method know of module is busy doing something (timeout provided to avoid lockups)
	void CompleteTask(unsigned long timeout = 0);
	
	// variable for the serial stream
	Stream*  _s;

	// pin variables
	int8_t _M0;
	int8_t _M1;
	int8_t _AUX;

	// variable for the 6 bytes that are sent to the module to program it
	// or bytes received to indicate modules programmed settings
	uint8_t Params[9];
	uint8_t Data[4];

	uint8_t Control;
	uint8_t ADDH;
	uint8_t ADDL;
	uint16_t Address;
	uint8_t REG0;
	uint8_t REG1;
	uint8_t REG2;
	
	uint8_t REG3;
	uint8_t CRYPT_H;
	uint8_t CRYPT_L;
	uint8_t PRODINFO;
	
	
	// individual variables for REG0
	uint8_t REG0_UARTDataRate;
	uint8_t REG0_ParityBit;	
	uint8_t REG0_AirDataRate;
	
	// individual variables for REG1
	uint8_t REG1_PacketSize;
	uint8_t REG1_RSSIEnableAmbientNoise;
	uint8_t REG1_Reserve;
	uint8_t REG1_SoftwareModeSwitching;
	uint8_t REG1_TransmitPower;
	
	// REG2
	uint8_t Channel;
	
	// REG3
	uint8_t REG3_RSSIEnableBytes;
	uint8_t REG3_TransmitMethod;
	uint8_t REG3_Reserve;
	uint8_t REG3_LBTEnable;
	uint8_t REG3_Reserve2;
	uint8_t REG3_WOR;
	
	

	char Model[30];
	char Version[30];
	uint8_t buf;
	uint8_t ProductInfo;

};

#endif