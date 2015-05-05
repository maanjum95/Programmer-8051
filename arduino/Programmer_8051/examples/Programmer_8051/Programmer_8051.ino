/*******************************************************************************************************	
* 	File Name: Programmer_8051.ino
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
#include <Programmer_8051.h>

byte reset = 2;    //  The digital pin for RST
byte clk = 3;      //  The digital pin for SCK
byte mosi = 4;     //  The digital pin for MOSI
byte miso = 5;     //  The digital pin for MISO
int serial = 9600; //  The serial rate of communication (Don't change this at higher rates the data gets buggy)
Programmer_8051 *programmer;

void setup() {
  programmer = new Programmer_8051(reset, clk, mosi, miso, serial);
  programmer->exec(); // starts the programmer
}

void loop() {
  // There is not really any loop to the program
  // To burn another or the same Chip again just reset the Arduino and it will restart
}
