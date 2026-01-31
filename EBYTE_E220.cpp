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


#include <EBYTE_E220.h>
#include <Stream.h>

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/*
create the transciever object
*/

EBYTE_E220::EBYTE_E220(Stream *s, uint8_t PIN_M0, uint8_t PIN_M1, uint8_t PIN_AUX)

{
	_s = s;
	_M0 = PIN_M0;
	_M1 = PIN_M1;
	_AUX = PIN_AUX;		
}

/*
Initialize the unit--basicall this reads the modules parameters and stores the parameters
for potential future module programming
*/

bool EBYTE_E220::init() {

	bool ok = true;

	pinMode(_AUX, INPUT);
	pinMode(_M0, OUTPUT);
	pinMode(_M1, OUTPUT);

	setMode(EBYTE_MODE_NORMAL);

	// get the EBYTE parameters
	ok = ReadParameters();

	if (!ok) {
		#ifdef DEBUG
			Serial.println("ReadParameters failed");
		#endif
		return false;
	}

	return true;
}

/*
Utility method to wait until module is doen tranmitting
a timeout is provided to avoid an infinite loop
*/

void EBYTE_E220::CompleteTask(unsigned long timeout) {

	unsigned long t = millis();

	// make darn sure millis() is not about to reach max data type limit and start over
	if (((unsigned long) (t + timeout)) == 0){
		t = 0;
	}

	// if AUX pin was supplied and look for HIGH state
	// note you can omit using AUX if no pins are available, but you will have to use delay() to let module finish
	
	// per data sheet control after aux goes high is 2ms so delay for at least that long
	// some MCU are slow so give 50 ms
	
	if (_AUX != -1) {
		
		while (digitalRead(_AUX) == LOW) {
			//Serial.println("EBYTE_E220 line 218 waiting for aux");
			delay(2);
			if ((millis() - t) > timeout){
				//Serial.println("aux timeout");
				break;
			}
		}
	}
	else {
		// if you can't use aux pin, use 4K7 pullup with Arduino
		// you may need to adjust this value if transmissions fail
		delay(1000);

	}

	// delay(PIN_RECOVER);
}

/*
method to set the mode (program, normal, etc.)
*/

void EBYTE_E220::setMode(uint8_t mode) {
	
	// data sheet claims module needs some extra time after mode setting (2ms)
	// most of my projects uses 10 ms, but 40ms is safer

	delay(PIN_RECOVER);
	
	if (mode == EBYTE_MODE_NORMAL) {
		digitalWrite(_M0, LOW);
		digitalWrite(_M1, LOW);

	}
	else if (mode == MODE_WAKEUP) {
		digitalWrite(_M0, HIGH);
		digitalWrite(_M1, LOW);

	}
	else if (mode == MODE_POWERDOWN) {
		digitalWrite(_M0, LOW);
		digitalWrite(_M1, HIGH);

	}
	else if (mode == MODE_PROGRAM) {
		digitalWrite(_M0, HIGH);
		digitalWrite(_M1, HIGH);

	}

	// data sheet says 2ms later control is returned, let's give just a bit more time
	// these modules can take time to activate pins
	delay(PIN_RECOVER);

	// clear out any junk
	ClearBuffer();

	// wait until aux pin goes back low
	CompleteTask(4000);
	
}

// perform a sofft reboot
// no "Cx" commands so we need to use AT commands
bool EBYTE_E220::reset() {

	int i = 0;
	char Response[10];
	const char *prefix = "=";
	size_t len = strlen(prefix);
	setMode(MODE_PROGRAM);
	_s->print("AT+RESET");
	delay(100); // data sheet says 30	
	while (_s->available()) {		
		char c = _s->read();
		Response[i] = c;
		i++;
		if (i > 9){
			break;
		}
	}
	
	if (strncmp(Response, prefix, len) == 0) {
        memmove(Response, Response + len, strlen(Response + len) + 1);
    }

	delay(100); // data sheet says 30
	setMode(EBYTE_MODE_NORMAL);
	if (strncmp(Response, "OK", 2) == 0){
		return true;
	}
	return false;
}

// retore the E220 back to defaults
// no "Cx" commands so we need to use AT commands
bool EBYTE_E220::restoreDefaults() {

	int i = 0;
	char Response[10];
	const char *prefix = "=";
	size_t len = strlen(prefix);
	setMode(MODE_PROGRAM);
	_s->print("AT+DEFAULT");
	delay(100); // data sheet says 30	
	while (_s->available()) {		
		char c = _s->read();
		Response[i] = c;
		i++;
		if (i > 9){
			break;
		}
	}
	
	if (strncmp(Response, prefix, len) == 0) {
        memmove(Response, Response + len, strlen(Response + len) + 1);
    }

	delay(100); // data sheet says 30
	setMode(EBYTE_MODE_NORMAL);
	if (strncmp(Response, "OK", 2) == 0){
		if (ReadParameters()){
			return true;
		}
		else {
			return false;
		}
	}		

	return false;

}


/*
method to set the addresses
*/


void EBYTE_E220::setAddressH(uint8_t val) {
	ADDH = val;
	getAddress();
}

void EBYTE_E220::setAddressL(uint8_t val) {
	ADDL = val;
	getAddress();
}

void EBYTE_E220::setAddress(uint16_t Val) {
	ADDH = ((Val & 0xFFFF) >> 8);
	ADDL = (Val & 0xFF);
}


/*
methods to set REG0
*/
void EBYTE_E220::setUARTBaudRate(uint8_t val) {
	REG0_UARTDataRate = val;
	BuildREG0();
}
void EBYTE_E220::setParityBit(uint8_t val) {
	REG0_ParityBit = val;
	BuildREG0();
}
void EBYTE_E220::setAirDataRate(uint8_t val) {
	REG0_AirDataRate = val;
	BuildREG0();	
}


/*
methods to set REG1
*/
void EBYTE_E220::setPacketSize(uint8_t val) {
	REG1_PacketSize = val;
	BuildREG1();	
}
void EBYTE_E220::setRSSIAmbientNoise(bool val) {
	REG1_RSSIEnableAmbientNoise = val;
	BuildREG1();	
}
void EBYTE_E220::setSoftwareModeSwitching(bool val) {
	REG1_SoftwareModeSwitching = val;
	BuildREG1();	
}		
void EBYTE_E220::setTransmitPower(uint8_t val) {
	REG1_TransmitPower = val;
	BuildREG1();	
}

/*
methods to set REG2
*/

void EBYTE_E220::setChannel(uint8_t val) {
	Channel = val;
	REG2 = val;

}

/*
methods to set REG3
*/

void EBYTE_E220::setRSSISignalStrength(bool val) {
	REG3_RSSIEnableBytes = val;
	BuildREG3();
}

void EBYTE_E220::setTransmissionMethod(uint8_t val) {
	REG3_TransmitMethod = val;
	BuildREG3();
}
void EBYTE_E220::setLBTEnable(bool val) {
	REG3_LBTEnable = val;
	BuildREG3();
}


void EBYTE_E220::setWORTIming(uint8_t val) {
	REG3_WOR = val;
	BuildREG3();
}

void EBYTE_E220::setEncryptonH(uint8_t val) {
	CRYPT_H = val;
}

void EBYTE_E220::setEncryptonL(uint8_t val) {
	CRYPT_L = val;
}


/*
method to get module model
*/

char *EBYTE_E220::getModel() {

	return Model;
	
}

/*
method to get module version
*/

char *EBYTE_E220::getVersion() {

	return Version;
	
}


// methods to get address values

uint16_t EBYTE_E220::getAddress(){
	
	Address = (ADDH << 8) | (ADDL);
	return Address;
}

uint8_t EBYTE_E220::getAddressH(){
	return ADDH;
}

uint8_t EBYTE_E220::getAddressL(){
	return ADDL;
}

// methods to get REG0
uint8_t EBYTE_E220::getUARTBaudRate(){
	return REG0 >> 5;
}

uint8_t EBYTE_E220::getParityBit(){
	return (REG0 & 0b00011000) >> 3;
}

uint8_t EBYTE_E220::getAirDataRate(){
	return REG0 &  0b00000111;
}

// methods to get REG1
		
uint8_t EBYTE_E220::getPacketSize(){
	return REG1 >> 6;
}
bool EBYTE_E220::getRSSIAmbientNoise(){
	return (REG1 & 0b00100000) >> 5;
}
		
bool EBYTE_E220::getSoftwareModeSwitching(){
	return (REG1 & 0b00000100) >> 3;
}
uint8_t EBYTE_E220::getTransmitPower(){
	return REG1 & 0b00000011;	
}

// methods to get REG2
uint8_t EBYTE_E220::getChannel(){
	return REG2;	
}

// methods to get REG3	

bool EBYTE_E220::getRSSISignalStrength(){
	return REG3 >> 7;
}	

uint8_t EBYTE_E220::getTransmissionMethod(){
	return (REG3 & 0b01000000) >> 6;
}
bool EBYTE_E220::getLBTEnable(){
	return (REG3 & 0b00010000) >> 4;
}
uint8_t EBYTE_E220::getWORTIming(){
	return REG3 & 0b00000111;	
}
	
uint8_t EBYTE_E220::getProductInfo(){
	return PRODINFO;	
}	

float EBYTE_E220::getTransmitFrequency(){
	return 850.125f + Channel ;	
}	
	
// methods to read RSSI data 


int16_t EBYTE_E220::readRSSIAmbientNoise(){
	
	int16_t RSSIValue = -999.0f;

	if (!REG1_RSSIEnableAmbientNoise){		
		return RSSIValue;		
	}

	//setMode(MODE_PROGRAM); // not needed as this method is intended to be run in normal mode

	_s->write(0xC0);
	_s->write(0xC1);
	_s->write(0xC2);
	_s->write(0xC3);	
	_s->write((uint8_t) 0x00); 
	_s->write(0x02); 
	
	_s->readBytes((uint8_t*)& Data, (uint8_t) sizeof(Data));
	
	//setMode(EBYTE_MODE_NORMAL); // not needed as this method is intended to be run in normal mode
	
	if (EBYTE_SUCCESS == Data[0]){
		RSSIValue = Data[3];
		RSSIValue = -(256 - RSSIValue);
	}

	return RSSIValue;	
}	


int16_t EBYTE_E220::readRSSISignalStrength(){
	
	int16_t RSSIValue = 0;
	
	if (!REG3_RSSIEnableBytes){		
		return -999.0f;		
	}
	
	// setMode(MODE_PROGRAM); // not needed as this method is intended to be run in normal mode	
	// sender tacks on a byte after transmission (single byte or a struct), so... just read a single byte
	// a slight delay seems to be needed
	delay(10); // this may need adjustment
	RSSIValue = _s->read();
	RSSIValue = -(256 - RSSIValue);
	return RSSIValue;	
}	
	

/*
method to build the REG bytes for programming
*/
void EBYTE_E220::BuildREG0() {
	REG0 = 0;
	REG0 = ((REG0_UARTDataRate & 0xFF) << 5) | ((REG0_ParityBit & 0xFF) << 3) | (REG0_AirDataRate & 0xFF);
}

void EBYTE_E220::BuildREG1() {
	REG1 = 0;
	REG1 = ((REG1_PacketSize & 0xFF) << 6) | ((REG1_RSSIEnableAmbientNoise & 0xFF) << 5) | ((REG1_SoftwareModeSwitching & 0xFF) << 2)| (REG1_TransmitPower&0b11);
}

void EBYTE_E220::BuildREG3() {
	REG3 = 0;
	REG3 = ((REG3_RSSIEnableBytes & 0xFF) << 7) | ((REG3_TransmitMethod & 0xFF) << 6) | ((REG3_LBTEnable & 0xFF) << 4)| (REG3_WOR&0b111);
}

bool EBYTE_E220::getAux() {
	return digitalRead(_AUX);
}

/*
method to save parameters to the module
*/

bool EBYTE_E220::saveParameters(uint8_t val) {
	
	bool success = false;
	
#ifdef DEBUG
	ClearBuffer();

	Serial.print("val: ");
	Serial.println(val);

	Serial.print("AddressHigh: ");
	Serial.println(ADDH);

	Serial.print("AddressLow: ");
	Serial.println(ADDL);

	Serial.print("REG0: ");
	Serial.println(REG0);

	Serial.print("REG1: ");
	Serial.println(REG1);

	Serial.print("REG1: ");
	Serial.println(REG1);
	
	Serial.print("REG2: ");
	Serial.println(REG2);

	Serial.print("REG3: ");
	Serial.println(REG3);

	Serial.print("CRYPT_H: ");
	Serial.println(CRYPT_H);

	Serial.print("CRYPT_L: ");
	Serial.println(CRYPT_L);
	
	Serial.print("VERSION: ");
	Serial.println(Version);

#endif

	setMode(MODE_PROGRAM);
	
	Params[0] = ADDH;
	Params[1] = ADDL;
	Params[2] = REG0;	
	Params[3] = REG1;
	Params[4] = REG2;  
	Params[5] = REG3;
	Params[6] = CRYPT_H; 
	Params[7] = CRYPT_L;
	// Params[8] = PRODINFO; // read only

	_s->write(val);
	_s->write((uint8_t)0x00);
	_s->write(0x08);
	for ( uint8_t i = 0; i < 8; i++){			
		_s->write(Params[i]);
	}

	_s->readBytes((uint8_t*)& Data, (uint8_t) sizeof(Data));	
	// check for return of C1
	if (Data[0] == EBYTE_SUCCESS){
		success = true;
	}
	
	setMode(EBYTE_MODE_NORMAL);

	return success;
	
}

/*
method to print parameters, this can be called anytime after init(), because init gets parameters
and any method updates the variables
*/

void EBYTE_E220::printParameters() {

	Serial.println("----------------------------------------");
	Serial.print(F("Model no.              : "));  Serial.println(Model);
	Serial.print(F("Version                : "));  Serial.println(Version);
	Serial.print(F("PRODINFO  (HEX/DEC/BIN): "));  Serial.print(PRODINFO, HEX); Serial.print(F("/"));  Serial.print(PRODINFO, DEC); Serial.print(F("/"));  Serial.println(PRODINFO, BIN);	
	Serial.print(F("RSSI Ambient Noise     : ")); Serial.print(readRSSIAmbientNoise()); Serial.println(F(" db"));
	Serial.print(F("Transmit frequency     : ")); Serial.print(getTransmitFrequency()); Serial.println(F(" MHz"));
	
	
	Serial.println(F(" "));
	Serial.print(F("HEAD (HEX/DEC/BIN)   : "));  Serial.print(Control, HEX); Serial.print(F("/"));  Serial.print(Control, DEC); Serial.print(F("/"));  Serial.println(Control, BIN);
	Serial.print(F("AddH (HEX/DEC/BIN)   : "));  Serial.print(ADDH , HEX); Serial.print(F("/")); Serial.print(ADDH , DEC); Serial.print(F("/"));  Serial.println(ADDH , BIN);
	Serial.print(F("AddL (HEX/DEC/BIN)   : "));  Serial.print(ADDL , HEX); Serial.print(F("/")); Serial.print(ADDL , DEC); Serial.print(F("/"));  Serial.println(ADDL , BIN);
	Serial.print(F("Address (HEX/DEC/BIN): "));  Serial.print(Address, HEX); Serial.print(F("/")); Serial.print(Address, DEC); Serial.print(F("/")); Serial.println(Address, BIN);
	Serial.print(F("REG0 (HEX/DEC/BIN)   : "));  Serial.print(REG0, HEX); Serial.print(F("/")); Serial.print(REG0, DEC); Serial.print(F("/"));  Serial.println(REG0, BIN);
	Serial.print(F("REG1 (HEX/DEC/BIN)   : "));  Serial.print(REG1, HEX); Serial.print(F("/")); Serial.print(REG1, DEC); Serial.print(F("/"));  Serial.println(REG1, BIN);
	Serial.print(F("REG2 (HEX/DEC/BIN)   : "));  Serial.print(REG2, HEX); Serial.print(F("/")); Serial.print(REG2, DEC); Serial.print(F("/"));  Serial.println(REG2, BIN);
	Serial.print(F("REG3 (HEX/DEC/BIN)   : "));  Serial.print(REG3, HEX); Serial.print(F("/")); Serial.print(REG3, DEC); Serial.print(F("/"));  Serial.println(REG3, BIN);	
	Serial.print(F("CRYPT_H (HEX/DEC/BIN):  "));  Serial.print(CRYPT_H, HEX); Serial.print(F("/")); Serial.print(CRYPT_H, DEC); Serial.print(F("/")); Serial.println(CRYPT_H, BIN);
	Serial.print(F("CRYPT_L (HEX/DEC/BIN):  "));  Serial.print(CRYPT_L, HEX); Serial.print(F("/")); Serial.print(CRYPT_L, DEC); Serial.print(F("/")); Serial.println(CRYPT_L, BIN);
	Serial.println(F(" "));	
	Serial.print(F("UARTDataRate (HEX/DEC/BIN)          : "));  Serial.print(REG0_UARTDataRate, HEX); Serial.print(F("/"));  Serial.print(REG0_UARTDataRate, DEC); Serial.print(F("/"));  Serial.println(REG0_UARTDataRate, BIN);
	Serial.print(F("ParityBit (HEX/DEC/BIN)             : "));  Serial.print(REG0_ParityBit, HEX); Serial.print(F("/"));  Serial.print(REG0_ParityBit, DEC); Serial.print(F("/"));  Serial.println(REG0_ParityBit, BIN);
	Serial.print(F("AirDataRate (HEX/DEC/BIN)           : "));  Serial.print(REG0_AirDataRate, HEX); Serial.print(F("/"));  Serial.print(REG0_AirDataRate, DEC); Serial.print(F("/"));  Serial.println(REG0_AirDataRate, BIN);	
	Serial.print(F("PacketSize (HEX/DEC/BIN)            : "));  Serial.print(REG1_PacketSize, HEX); Serial.print(F("/"));  Serial.print(REG1_PacketSize, DEC); Serial.print(F("/"));  Serial.println(REG1_PacketSize, BIN);
	Serial.print(F("RSSIEnableAmbientNoise (HEX/DEC/BIN): "));  Serial.print(REG1_RSSIEnableAmbientNoise, HEX); Serial.print(F("/"));  Serial.print(REG1_RSSIEnableAmbientNoise, DEC); Serial.print(F("/"));  Serial.println(REG1_RSSIEnableAmbientNoise, BIN);
	Serial.print(F("SoftwareModeSwitching (HEX/DEC/BIN) : "));  Serial.print(REG1_SoftwareModeSwitching, HEX); Serial.print(F("/"));  Serial.print(REG1_SoftwareModeSwitching, DEC); Serial.print(F("/"));  Serial.println(REG1_SoftwareModeSwitching, BIN);
	Serial.print(F("TransmitPower (HEX/DEC/BIN)         : "));  Serial.print(REG1_TransmitPower, HEX); Serial.print(F("/"));  Serial.print(REG1_TransmitPower, DEC); Serial.print(F("/"));  Serial.println(REG1_TransmitPower, BIN);		
	Serial.print(F("Channel (HEX/DEC/BIN)               : "));  Serial.print(Channel, HEX); Serial.print(F("/"));  Serial.print(Channel, DEC); Serial.print(F("/"));  Serial.println(Channel, BIN);
	Serial.print(F("RSSIEnableBytes (HEX/DEC/BIN)       : "));  Serial.print(REG3_RSSIEnableBytes, HEX); Serial.print(F("/"));  Serial.print(REG3_RSSIEnableBytes, DEC); Serial.print(F("/"));  Serial.println(REG3_RSSIEnableBytes, BIN);
	Serial.print(F("TransmitMethod (HEX/DEC/BIN)        : "));  Serial.print(REG3_TransmitMethod, HEX); Serial.print(F("/"));  Serial.print(REG3_TransmitMethod, DEC); Serial.print(F("/"));  Serial.println(REG3_TransmitMethod, BIN);
	Serial.print(F("LBTEnable (HEX/DEC/BIN)             : "));  Serial.print(REG3_LBTEnable, HEX); Serial.print(F("/"));  Serial.print(REG3_LBTEnable, DEC); Serial.print(F("/"));  Serial.println(REG3_LBTEnable, BIN);
	Serial.print(F("WOR (HEX/DEC/BIN)                   : "));  Serial.print(REG3_WOR, HEX); Serial.print(F("/"));  Serial.print(REG3_WOR, DEC); Serial.print(F("/"));  Serial.println(REG3_WOR, BIN);
	Serial.println("----------------------------------------");

}

/*
method to read parameters, 
*/

bool EBYTE_E220::ReadParameters() {
	
	if (!ReadModel()){
		Serial.println("ReadModel() fail");
		return false;
	}
	ReadVersion();	
	
	setMode(MODE_PROGRAM);
	
	for ( uint8_t i = 0; i < 9; i++){
		
		_s->write(EBYTE_READ);
		_s->write((uint8_t) i); //  rip through all registgers (even write only)
		_s->write(0x01); // get 1 byte at a time	
		_s->readBytes((uint8_t*)& Data, (uint8_t) sizeof(Data));	

		Params[i] = 0; // clear out previous
		Params[i] = Data[3];
		
		#ifdef DEBUG
			Serial.println("Reading parameters");
			Serial.print("Command: ");
			Serial.print(Data[0]);
			Serial.print(", Start: ");
			Serial.print(Data[1]);
			Serial.print("Len: ");
			Serial.print(Data[2]);
			Serial.print(", Data: ");		
			Serial.print("Params[");
			Serial.print(i);		
			Serial.print("]: ");		
			Serial.print(Params[i], DEC);
			Serial.print(" / ");
			Serial.print(Params[i], BIN);
			Serial.print(" / ");
			Serial.println(Params[i], HEX);
		#endif
		
	}
		
	setMode(EBYTE_MODE_NORMAL);

	ADDH =  Params[0];
	ADDL =  Params[1];
	REG0 = Params[2];	
	REG1 = Params[3];
	REG2 = Params[4];
	REG3 = Params[5];
	CRYPT_H = Params[6]; // write only always read as 0
	CRYPT_L = Params[7]; // write only always read as 0
	PRODINFO = Params[8];
	
	ADDH  = getAddressH();
	ADDL =  getAddressL();
	Address =  getAddress();

	REG0_UARTDataRate =  getUARTBaudRate();
	REG0_ParityBit =  getParityBit();
	REG0_AirDataRate =  getAirDataRate();
	
	REG1_PacketSize =  getPacketSize();
	REG1_RSSIEnableAmbientNoise =  getRSSIAmbientNoise();
	REG1_SoftwareModeSwitching = getSoftwareModeSwitching();
	REG1_TransmitPower =  getTransmitPower();
	
	Channel =  getChannel();
	
	REG3_RSSIEnableBytes = getRSSISignalStrength();
	REG3_TransmitMethod = getTransmissionMethod();
	REG3_LBTEnable = getLBTEnable();
	REG3_WOR = getWORTIming();

	if (EBYTE_SUCCESS == Data[0]){
		return true;
	}
	Serial.println("707 readParameters() fail");
	return false;
	
}

// the "C1" method for getting model and version are not supported
// we must use good ol' AT commands

bool EBYTE_E220::ReadModel() {

	int i = 0;
	const char *prefix = "DEVTYPE=";
    size_t len = strlen(prefix);	
	setMode(MODE_PROGRAM);
	_s->print("AT+DEVTYPE=?\r\n");
	delay(100); // data sheet says 30	
	while (_s->available()) {		
		char c = _s->read();
		if (c == '\n'){			
			break;
		}
		Model[i] = c;
		i++;
		if (i > 29){
			break;
		}
	}
	
	delay(100); // data sheet says 30
	setMode(EBYTE_MODE_NORMAL);

    if (strncmp(Model, prefix, len) == 0) {
        memmove(Model, Model + len, strlen(Model + len) + 1);
    }	
	// simple check to see if this is an E220
	if (strncmp(Model, "E220", 4) == 0){
		return true;
	}	
	return false;
	
}

// the "C1" method for getting model and version are not supported
// we must use good ol' AT commands

bool EBYTE_E220::ReadVersion() {

	int i = 0;
	const char *prefix = "FWCODE=";
    size_t len = strlen(prefix);	
	setMode(MODE_PROGRAM);
	_s->print("AT+FWCODE=?\r\n");	
	delay(100); // data sheet says 30	
	while (_s->available()) {		
		char c = _s->read();
		if (c == '\n'){			
			break;
		}
		Version[i] = c;
		i++;
		if (i > 29){
			break;
		}
	}
	delay(100); // data sheet says 30
	setMode(EBYTE_MODE_NORMAL);

	if (strncmp(Version, prefix, len) == 0) {
        memmove(Version, Version + len, strlen(Version + len) + 1);
    }	
	return true; // maybe someday I'll add some checker but version really doesn't matter
	
}

/*
method to clear the serial buffer

without clearing the buffer, i find getting the parameters very unreliable after programming.
i suspect stuff in the buffer affects rogramming 
hence, let's clean it out
this is called as part of the setmode

*/
void EBYTE_E220::ClearBuffer(){

	unsigned long amt = millis();

	while(_s->available()) {
		_s->read();
		if ((millis() - amt) > 5000) {
          Serial.println("runaway");
          break;
        }
	}

}
