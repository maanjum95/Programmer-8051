/*******************************************************************************************************	
* 	File Name: Programmer_8051.h
*	Project Name: Programmer-8051
* 	
* 	The MIT License (MIT)
*
*	Copyright (c) 2015 M.A. Anjum
*	Email : ma.anjum95@gmail.com
*	
*	Permission is hereby granted, free of charge, to any person obtaining a copy of this software
*	and associated documentation files (the "Software"), to deal in the Software without restriction,
*	including without limitation the rights to use, copy, modify, merge, publish, distribute,
*	sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
*	is furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in all copies 
*	or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
*	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
*	AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
*	DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
*	OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************/
/*************************************** SERIAL PROGRAMMING **********************************************
*
*	Working of serial programming is based on the datasheet specifications of
*	Atmel AT89S51 Datasheet (http://www.keil.com/dd/docs/datashts/atmel/at89s51_ds.pdf)
*	
*	Serial Programming Algorithm follows the following steps:
*		1- Power Up Sequence (Vcc & GND), setting RST to 'H' and connection crystal across XTAL1&2
*		2- Enabling Serial Programming by sending Programming Enable Serial Instruction to MOSI &
*				the frequency used for the SCK clock must be 1/16 times that of crystal connected across XTAL1&2 
*		3- Programming Code array one byte at a time in Byte or Page format (Byte format implemented here)
*		4- Any memory location can be verified using Read Instructions that return contents of the selected address at MISO
*		5- At the end of programming RST can be set low to commenct normal device operation.
*	
*	Serial Programming Instruction Set used in this library are mentioned below and can be read at the link mentioned above
*	INSTRUCTION 			|	BYTE 1 	|	BYTE 2 		|	BYTE 3 		| 	BYTE 4				|	Operation
*	Programming Enable 		|	0x54	|	0x53		|	0xXX		|	0xXX (0x69 output)	|	Enable Serial Programming while RST is high
*	Chip Erase				|	0xAC	|	0x8X		|	0xXX		|	0xXX				|	Chipe Erase Flash memory array
*	Read Program Memory(BM)	|	0x20	|	0xX(A11-8)	|	0x(A7-0)	|	0x(D7-0)			|	Read data from Program memory in the byte mode
* 	Write Program Memory(BM)|	0x40	|	0xX(A11-8)	| 	0x(A7-0)	|	0x(D7-0)			|	Write data to Program memory in the byte mode	
*	Write Lock Bits			|	0xAC	|	0xE(00B1B2)	|	0xXX		|	0xXX				|	Write Lock bits
*	Read Lock Bits			|	0x24	|	0xXX		|	0xXX		|	(xxxLB3LB2LB1xx)		|	Read back current status of lock bits (programmed read as "1")
*	Read Signature Bytes	|	0x28	|	(xxxA5-A1)	|	0x(A0xxx)X 	|	<Signature Byte>	|	Read Signature Byte	
*
*	IMP
*		1- Singature bytes are not readable in Lock Bit Modes 3 & 4
*/

#ifndef PROGRAMMER_8051_H
#define PROGRAMMER_8051_H

#include "Arduino.h"

/**	
*	Defining the indiviudalbytes used for different programming instruction as mentioned above
*/

//	Dont cares or X will be treated as 0's
#define DONT_CARE 0x00

#define PROG_EN_1 0xAC
#define PROG_EN_2 0x53
#define PROG_EN_3 DONT_CARE
#define PROG_EN_4 DONT_CARE

#define CHIP_ER_1 0xAC
#define CHIP_ER_2 0x80
#define CHIP_ER_3 DONT_CARE
#define CHIP_ER_4 DONT_CARE

#define READ_BYTE_1 0x20

#define WRITE_BYTE_1 0x40

#define WRITE_LOCK_BITS_1 0xAC
#define WRITE_LOCK_BITS_2 0xE0
#define WRITE_LOCK_BITS_3 DONT_CARE
#define WRITE_LOCK_BITS_4 DONT_CARE

#define READ_LOCK_BITS_1 0x24
#define READ_LOCK_BITS_2 DONT_CARE
#define READ_LOCK_BITS_3 DONT_CARE
#define READ_LOCK_BITS_4 DONT_CARE

#define READ_SIG_BITS_1 0x28
#define READ_SIG_BITS_2 DONT_CARE
#define READ_SIG_BITS_3 DONT_CARE
#define READ_SIG_BITS_4 DONT_CARE

//	The delay in each SCK flip-flop which should be atleast 1/16th times that of the crystal attached
//	*Imp* In my case I used a 12 MHz crystal and the Arduino has 16MHz crystal.
#define DELAY_INSTRUC_us 2

//	After the Chip Erase Instruction has been issued we should atleast wait 500ms
//	The actual value used was 640 just to be on the safe side
#define DELAY_ERASE_ms 640 

//	When starting the programming we must first pull up the RST pin to "1" and,
//	wait 64 cycles (on a 12MHz crystal it is equivalent to 6ms)
//	The actual value used was 8 just to be on the safe side
#define DELAY_RESET_ms 8

/**
*	Pins_8051 struct refers to the four digital pin numbers which need
*	to be attached to 8051 in order to successfully program it.
*	The pins and their respective connections to 8051 ports are as follows:
*	RST : connected to pin number  : 9
*	SCK : connected to pin number  : 8	port : P1.7
*	MOSI : connected to pin number : 6	port : P1.5
*	MISO : connected to pin number : 7	port : P1.6
*	
*	Each of the above pins has following 
*	RST reset pulled to 1 to stop the exection of program and also enable programming
*	SCK the shift clock which acts as a clock during programming
*	MOSI for the input of 8051 ie output from Arduino
* 	MISO for the output of 8501 ie input to Arduino
*/
struct Pins_8051{
	byte RST;
	byte SCK;
	byte MOSI;
	byte MISO;
};

/**
*	enum Serial_Commands contains all the commands which will be sent by
*	the client software using COM to execute specific commands.
*	The nature of commands can be easily interpreted by reading the individual constrant name.
*	The same enum needs to be copied over to the client software and used to send commands.
*/
enum Serial_Commands {
	ENABLE_PROGRAMMING = 0x41,
	ERASE_CHIP,
	READ_BYTE,
	WRITE_BYTE,
	WRITE_LOCK_BITS,
	READ_LOCK_BITS,
	READ_SIGNATURE_BYTE,
	EXIT,
	TEST,
	SUCCESS
};

class Programmer_8051 {
	Pins_8051 _pins;	//	This Digital pins which need to be connected to 8051
	int _serialRate;	//	The COM baud rate on which the Arduino and client software will communicate

	void setupPinModes(void);	//	Sets up the pin modes of the digital pins passed in to the constructor
	void setupSerialRate(void);	//	Sets up the serial rate for serial communication

	//	The following methods just call the sendSPI function with the correct instruction's byte pattern as mentioned in the comments
	byte progEnable(void);
	void chipErase(void);
	byte readByte(byte addrH, byte addrL);
	void writeByte(byte addrH, byte addrL, byte data);
	void writeLockBits(byte lockBits);
	byte readLockBits(void);
	byte readSignatureByte(byte addr);

	/**
	*	sendSPI send a byte of instruction to the 8051 micro-controller and returns the reponse from the 8501 
	*	The algorithm follows the following steps:
	*		1- Loop over the size of the byte (8 times) 
	*			a- Extract MSB and set MOSI to the MSB.
	*			b- Set SCK (CLK) SCK = 1.
	*			c- Delay for DELAY_INSTRUC_us (us).
	*			d- Read the MISO from the 8051.
	*			e- Clear SCK (CLK) SCK = 0.
	*			f- Store the read MISO to a variable
	*			g- Shift left by 1 the given byte and vaeiable storing MISO bits.
	*		2- Return stored MISO bits in the form of a byte.
	*/
	byte sendSPI(byte instruc);
public:
	/**
	*	Parameter of the Constructors are as follows:
	*	1- RST 			: The digital pin number to which the pin number 9 of 8051 is connected 
	*	2- SCK 			: The digital pin number to which the pin number 8 of 8051 is connected
	*	3- MOSI 		: The digital pin number to which the pin number 6 of 8051 is connected
	*	4- MISO 		: The digital pin number to which the pin number 7 of 8051 is connected
	*	5- serialRate 	: The COM baud rate on which the communication between client and Arduino will take place.
	*/
	Programmer_8051(byte RST, byte SCK, byte MOSI, byte MISO, int serialRate);

	/**
	*	exec will execute respective instruction based on what is passed over the serail COM.
	*	It will keep on looping unless EXIT is given by the client software.
	*	Once the EXIT command has been issued by the client software,
	*	to program again or another 8051 micro-controller the Arduino needs to be reset.
	*/
	void exec(void);
};

#endif