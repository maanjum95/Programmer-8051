/*******************************************************************************************************	
* 	File Name: Programmer_8051.cpp
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
#include "Programmer_8051.h"

Programmer_8051::Programmer_8051(byte RST, byte SCK, byte MOSI, byte MISO, int serialRate) {
	// setting the values of struct _pins
	this->_pins.RST = RST;
	this->_pins.SCK = SCK;
	this->_pins.MOSI = MOSI;
	this->_pins.MISO = MISO;

	// setting the value of the rate at which the serial port will work
	this->_serialRate = serialRate;

	// setting up the pins and also the serial
	this->setupPinModes();
	this->setupSerialRate();
}

void Programmer_8051::exec(void) {
	bool exec = true;
	byte input;
	byte addrH;
	byte addrL;
	byte data;
	int sigAddr;

	//	When starting the programming we must first pull up the RST pin to "1" and,
	//	wait 64 cycles (on a 16MHz crystal it is equivalent to 4ms)
	digitalWrite(this->_pins.SCK, 0);
	delay(DELAY_RESET_ms);
	digitalWrite(this->_pins.RST, 1);
	delay(DELAY_RESET_ms);

	while(exec) {
		// Executing an instruction depending upon the serial input
		while(!Serial.available());	//	Waiting for input

		switch(Serial.read()) {
			case TEST:
				Serial.write(SUCCESS);
				break;
			case ENABLE_PROGRAMMING:
				input = this->progEnable();
				Serial.write(input);
				break;
			case ERASE_CHIP:
				this->chipErase();
				Serial.write(SUCCESS);
				break;
			case READ_BYTE:
				while(Serial.available() < 2);
				
				addrH = Serial.read();
				addrL = Serial.read();
				
				data = this->readByte(addrH, addrL);
				Serial.write(data);
				break;
			case WRITE_BYTE:
				while(Serial.available() < 3);
				
				addrH = Serial.read();
				addrL = Serial.read();
				data = Serial.read();

				this->writeByte(addrH, addrL, data);
				break;
			case WRITE_LOCK_BITS:
				while(Serial.available() < 1);
				
				data = Serial.read();

				this->writeLockBits(data);
				break;
			case READ_LOCK_BITS:		
				data = this->readLockBits();

				Serial.write(data);
				break;
			case READ_SIGNATURE_BYTE:
				while(Serial.available() < 1);

				addrL = Serial.read();

				data = this->readSignatureByte(addrL);
				Serial.write(data);
				break;
			case EXIT:
				exec = false;
				Serial.write(SUCCESS);
				delay(1000);
				break;
			default:
				Serial.write(DONT_CARE);
			break;
		}
	}
	//	When the burning has finished we set RST to Low
	digitalWrite(this->_pins.RST, 0);
	Serial.end();

	//	Changing all the pinModes to Input so that they don't interfere with normal operation of 805
	pinMode(this->_pins.RST, INPUT);
	pinMode(this->_pins.SCK, INPUT);
	pinMode(this->_pins.MOSI, INPUT);
	pinMode(this->_pins.MISO, INPUT);
}

byte Programmer_8051::sendSPI(byte instruc) {
	byte toReturn;
	byte temp = instruc;
	byte bitOutput;

	//	Looping over the complete instruction and sending a bit at a time
	//	Also we are storing what is being sent by the 8051 in toReturn
	for (int i = 0; i < 8; i++) {
		//	Getting the MSB of the instruction and sending it via MOSI pin
		if (temp & 0x80)
			digitalWrite(this->_pins.MOSI, 1);
		else
			digitalWrite(this->_pins.MOSI, 0);

		//	Turning the clock on to indicate an insturction was just passed
		digitalWrite(this->_pins.SCK, 1);
		delayMicroseconds(DELAY_INSTRUC_us);

		//	Reading the input from the 8051 as it may be needed in some cases
		bitOutput = digitalRead(this->_pins.MISO);
		digitalWrite(this->_pins.SCK, 0);

		//	Adding the bit output of 8051 (bitOutput) to the toReturn
		if (bitOutput)
			toReturn |= 1;
		else
			toReturn &= 0xFE;

		//	We don't want to execute this instruction on the final iteration
		if (i != 7) {
			//	Shifting the bits right by one to prepare for the next iteration
			toReturn <<= 1;	
			temp <<= 1;
		}
		delayMicroseconds(DELAY_INSTRUC_us);  
	}
	return toReturn;
}

void Programmer_8051::setupPinModes(void) {
	pinMode(this->_pins.RST, OUTPUT);
	pinMode(this->_pins.SCK, OUTPUT);
	pinMode(this->_pins.MOSI, OUTPUT);
	pinMode(this->_pins.MISO, INPUT);
}

void Programmer_8051::setupSerialRate(void) {
	Serial.begin(this->_serialRate);
	while(!Serial) {}
}

byte Programmer_8051::progEnable(void) {
	this->sendSPI(PROG_EN_1);
	this->sendSPI(PROG_EN_2);
	this->sendSPI(PROG_EN_3);
	return this->sendSPI(PROG_EN_4);
}

void Programmer_8051::chipErase(void) {
	this->sendSPI(CHIP_ER_1);
	this->sendSPI(CHIP_ER_2);
	this->sendSPI(CHIP_ER_3);
	this->sendSPI(CHIP_ER_4);

	//	Delay so that the 8051 is completely erased and is ready for programming
	delay(DELAY_ERASE_ms);
}

byte Programmer_8051::readByte(byte addrH, byte addrL) {
	this->sendSPI(READ_BYTE_1);
	this->sendSPI(addrH);
	this->sendSPI(addrL);
	return this->sendSPI(DONT_CARE);
}

void Programmer_8051::writeByte(byte addrH, byte addrL, byte data) {
	this->sendSPI(WRITE_BYTE_1);
	this->sendSPI(addrH);
	this->sendSPI(addrL);
	this->sendSPI(data);
}

void Programmer_8051::writeLockBits(byte lockBits) {
	this->sendSPI(WRITE_LOCK_BITS_1);
	this->sendSPI(WRITE_LOCK_BITS_2 | (lockBits & 0x03));
	this->sendSPI(WRITE_LOCK_BITS_3);
	this->sendSPI(WRITE_LOCK_BITS_4);
}

//// **** THE datasheet is faulty as it mentions three lock bits
byte Programmer_8051::readLockBits(void) {
	//	The lock bits are returned in the last byte in the format
	// xxxLB3LB2LB1xx
	byte temp;

	this->sendSPI(READ_LOCK_BITS_1);
	this->sendSPI(READ_LOCK_BITS_2);
	this->sendSPI(READ_LOCK_BITS_3);
	temp = this->sendSPI(READ_LOCK_BITS_4);
	
	return (temp & 0x1C) >> 2;
}

byte Programmer_8051::readSignatureByte(byte addr) {
	this->sendSPI(READ_SIG_BITS_1);
	this->sendSPI(READ_SIG_BITS_2 | ((addr & 0x3E) >> 1));
	this->sendSPI(READ_SIG_BITS_3 | ((addr & 0x01) << 7));
	
	return this->sendSPI(READ_SIG_BITS_4);
}
