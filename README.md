<b><h1><center>E220-xxxTxxx Transceivers</center></h1></b>
<br>
<h3>
<li>V1.0 1/31/2026, initial check in </li>
<li>V1.1 6/28/2026, improved reliability in readRSSIxxx</li>
</h3>
<br>


This library is intended to be used with UART type EBYTE transceivers (E220-xxxTxxx), which are small wireless units for MCU's such as
Teensy, ESP32, Arduino and others. This library lets users program the operating parameters and both send and receive data.
This library is specific to E220 series only but will support the 400 or 900 series, or the 22 db or 30db units.
All constants were extracted from several data sheets and listed in binary as that's how the data sheet represented each setting. This library is for the UART devices only.

<b> EBYTE Model numbers (full list)</b>
E220-400T22S
E220-400T22D
E220-400T30S
E220-400T30D
E220-900T22S
E220-900T22D
E220-900T30S
E220-900T30D
 
Usage of this library consumes around 600 bytes. 

In my many years of using these devices, here's what I find most appealing
1. Low cost
2. Very reliable
3. Power up send or receive order does not matter
4. great range, in one application I have a temp sensor on one end of my house, and a base at the other end--never lost a bit. I also have these mounted on race cars for live telemetry, again never lost a bit
5. simple to change settings
6. rich options, and these units have RSSI data

You only really need this library to program these EBYTE units. 

For reading data structures, you can call readBytes method directly on the EBYTE's Serial object:
<br>
<br>
<b>ESerial.readBytes((uint8_t*)& MyData, (uint8_t) sizeof(MyData));</b>
<br>
<br>
For writing data structures you can call write method directly on the EBYTE's Serial object
<br>
<br>
<b>ESerial.write((uint8_t*) &MyData, (uint8_t) sizeof(MyData) );</b>
<br>
<br>
<br>
 
<b><h3> Module connection </b></h3>
Module	MCU						Description
1. MO		Any digital pin*		pin to control working/program modes
2. M1		Any digital pin*		pin to control working/program modes
3. Rx		Any digital pin*			pin to MCU TX pin (module transmits to MCU, hence MCU must receive data from module
4. Tx		Any digital pin*			pin to MCU RX pin (module transmits to MCU, hence MCU must receive data from module
5. AUX		Any digital pin			pin to indicate when an operation is complete (low is busy, high is done) (you can omit with -1, but fixed recovery time used and may not be long enough to complete the operation)
6. Vcc		+3v3 or 5V0, note the units may run warmer with 5V0 and consume more power				
7. Vcc		Ground					Ground must be common to module and MCU		

notes

1. caution in connecting to Arduino pin 0 and 1 as those pins are for USB connection to PC so you can't have the EBYTE connected during programming. I recommend NOT using Arduino pins 0 and 1
2. The signal lines for these units are 3V3 but are 5 volt tolerant, however 5 volts may result in communication failures. If using a 5 volt MCU such as arduino, you may need to do the following.\
  a) You may need level shifters or possibly a simmple voltage divider for EBYTE Tx and AUX pins\
  b) You may be able to use a series 4K7 resistor between MCU Rx and EBYTE Tx and the EBYTE AUX\
4. In some of my applications, I did not have enough digital pins to connect the Aux pin. No worries (just pass -1 in the argument list in the object create code). The library has a built-in delay to provide an appropriate delay to let the transmission complete--you may have to experiment with the amount.
5. In some of my applications, I did not have enough digital pins to connect both M0 and M0 pin. No worries (I just connected both M0 and M1 to the same MCU pin--you can only communicate or program though).
6. Serial pins for connection is dependent on the MCU, Teensy 3.2 for example: Serial1 are Rx=0, Tx=1, Serial2 Rx=9, Tx=10, Serial3 Rx=7, Tx=8. Arduino can be most serial pins using SoftwareSerial(MCU_Rx_pin, MCU_Tx_pin), except pins 0 and 1 as those are for USB usage
7. Some MCU such as the Teensy, and ESP32 do NOT allow the use of SoftwareSerial to create a communications port. No worries, just hard wire the EBTYE to a dedicated UART port (pin 0 and pin 1 on a teensy 3.2 for Serial1.

<b><h3>Product website</b></h3> 
https://www.cdebyte.com/products/E220-900T22D

<b><h3>General code usage</b></h3> 
1. Create a serial object
2. Create EBYTE_220 object that uses the serial object
3. begin the serial object
4. init() the EBYTE object
5. set parameters (optional but required if sender and receiver are different)
6. send or listen to sent data (single byte) OR create and send a data structure

<b><h3>Tips on usage</b></h3> 

For best range:

<ul>
<li> Data sheet indicates best results are with antennas 2meters off of ground</li>
<li> Line of sight ideal, but my personal testing, transmission still successful with some obstructions</li>
<li> Slow air data rates can improve range, but due to longer transmission time, how often data can be sent will be sacrificed</li>
<li> Consider high gain antennas (can be purchased from the manufacturer) see their web site for details</li>
<li> The data sheet says for max range, power the units with 5.0 volts (keep 3V3 on the signal lines). I personaly found little range difference with higher supply voltage</li>
 <li> The data sheet says for max range, set the air data rate to 2.4 bps. One application sends 48 bytes ever second and best range was with lower air data rates </li>
 
</ul>

<b><h3>Data transmission packets</b></h3>

In my experience when sending several data, I don't recommend sending comma seperated fields with some special characters bookending the data. Getting data is just too unreliable. If you need to send multiple types of data, create and send a struct. Of the 100's of MB's I've transmitted over 10 years, I've never lost a bit.
<ul>
<li> This library does not have any methods for sending or receiving data. Use standard serial.print or serial.write methods to write bytes of data.
For writing data structures you can call write method directly on the EBYTE's Serial object. The example here is where MyData is a struct.</li>
<br>
<b>ESerial.write((uint8_t*) &MyData, (uint8_t) sizeof(MyData) );</b>
<br>
For reading data structures, you call readBytes method directly on the EBYTE's Serial object :
<br>
<b>ESerial.readBytes((uint8_t*)& MyData, (uint8_t) sizeof(MyData));</b>
<br>
<br>
<li> If you need to send data using a struct between different MCU's. processor compilers will pack data differently. If you get corrupted data on the recieving end, there are ways to force the compiler to not optimize struct packing--I've yet to get packing to work. What worked for me is EasyTransfer.h (google it to get the repo). In these libs you will use their method of sending and getting struct. Meaning you can use this library to program and manage settings but use EasyTransfer to handle sending data throught the serial lines the EBYTE is using. Sounds weird, but it's no differnet that say Serial1.sendBytes(...).
</ul>
<b><h3>Debugging</b></h3>
<ul>
 
<li> If your wireless module is returning all 0's for the PrintParameters() method or just the model AND you are using hardware serial AND you are using an ESP32, you may have to begin the serial definition will full details like this</li>
<br>
 
 - #include <HardwareSerial.h>
  
 - #define Serial_0 Serial2
   
 - Serial_0.begin(9600, SERIAL_8N1, 16, 17);

 
<li> If your wireless module is returning all 0's for the printParameters() method, make sure your wiring is correct and working, MCU Rx needs to connecte to the EBYTE Tx and vice versa. Also make sure M0, M0 (OUTPUT supported pin), and AUX (input supported pin) are connected to valid digital ports. Most issues are due to incorrect data line connections </li>
 
<li> If your wireless module is returning all 0's for the printParameters() method, AND you are sure your wiring is correct, your module may be slow to react to pinMode change performed during a mode change. The datasheet says delay of 2 ms is needed, but I've found 10 ms is more reliable. With some units, even more time is needed. The library default is 50 ms, but increase this in the .h file if parameters are not correctly read.</li>
  
<li> If your wireless module is returning all 0's for the printParameters() method, AND you are sure your wiring is correct and your MCU is 5v0, you may have to add voltage dividers on the MXU Tx and AUX line. These modules can be finicky if a 5v0 signal is being sent to the not power pins. I get very reliable results when powering the module with a separate 5v0 power supply. I generally use buck converters or linear regulators. </li>
    
<li> If using a 5v0 MCU you may need just series resistors on the MCU Tx line to the EBYTE Rx line and possibly the M0 and M1 lines. These EBYTE units are supposed to be 5 volt tolerant, but better safe than sorry. Also MFG claims 4K7 pullups can be needed on MCU Tx line and AUX. I have used these transceivers on UNO's, MEGA's, and NANO's w/o any resistors and all was well. I did have one case where a NANO did not work with these transceivers and required some odd powering.</li>
    
<li> If you are using their 1W units (30 db power output), power the unit separately from the MCU's onboard power supply. The current draw may exceed the onboard rating resulting in destroying the MCU. I have destroyed the onboard voltage regulator on a NANO when trying to power a 1W unit.</li>

<li> If transmitter and receiver are different MCU (Arduino <-> Teensy), sending data structures may pack differently, regardless of structure data types. This is due to how an 8-bit processor and 32-bit processor handle the packing process.   </li>    
 
Option 1) is to use EasTransfer lib. I use this lib and it works well.     

Option 2) try the __attribute__((packed)) variable attribute.     

Option 3) and don't laugh, but if sending a float considering clipping the precision by multiplying a float to 100 (and recasting to an int), then divide that value by 100 on the receiving end (recasting to a float)</li>    


<li> If you seem to get corrupt data from .printParameters, try addinng #include "avr/io.h" to your .INO program</li>

<li> If you are powering  your EBYTE modules from a separate power source, make sure all grounds are connected</li>

<li> If printParameters() returns valid data but you cannot send a struct, try these</li>
 1. start by sending a single byte--jsut to test if transmission is working\
 2. if send data in a timely manner, slow transmissions to say 2-3 sec between--just to test\
 3. consider slowing air data rate (default is 2.4 kbps)\

</ul>


