package programmer_8051.client;

import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortTimeoutException;

public class SerialCOM { //implements SerialPortEventListener {
	
	private SerialPort serialPort;	 	  // The SerialPort on which communication will occur
	//private LinkedList<Byte> serialInput; // Queue containing the data from serial
	
	public final int port;
	public final int baudRate;
	
	// 	Serial_Commands as used in the Arduino to identify different Instruction
	//	We might want to send to the Arduino to program the 8051
	//	(*Reason for being sloppy is because JAVA does not allow enums to be dealt like bytes
	//		as in C++ and also does not allow to start them from a specific number*)
	private static final byte ENABLE_PROGRAMMING 	= 0x41;
	private static final byte ERASE_CHIP 			= 0x42;
	private static final byte READ_BYTE 			= 0x43;
	private static final byte WRITE_BYTE 			= 0x44;
	private static final byte WRITE_LOCK_BITS 		= 0x45;
	private static final byte READ_LOCK_BITS 		= 0x46;
	private static final byte READ_SIGNATURE_BYTE 	= 0x47;
	private static final byte EXIT 					= 0x48;
	private static final byte TEST 					= 0x49;
	private static final byte SUCCESS 				= 0x4A;
	
	private static final byte PROGRAMMING_ENABLED = 0x69; // The byte that is returned if 
														 // programming is successfully enabled
	/*
	 *	Opens the connection to passed in COM port with the specified baud rate.
	 *	Also applies a mask to the events and adds an event listener.
	 *	May throw a SerialPortException if:
	 *	1-	The COM port does not exist.
	 *	2-	The COM port is busy.
	 *	3- 	Others which I have not encountered yet!
	 * */
	public SerialCOM(int port, int baudRate) throws SerialPortException {
		this.serialPort = new SerialPort("COM" + port);
		//this.serialInput = new LinkedList<Byte>();
		this.port = port;
		this.baudRate = baudRate;
		
		this.serialPort.openPort();
		this.serialPort.setParams(baudRate, SerialPort.DATABITS_8,
				SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);	
	}
	
	/*
	 *	Closes the serial communication port to free the comm port to be
	 *	used by another software like Arduino IDE.
	 *	May throw a SerialPortException.
	 * */
	public void end() throws SerialPortException {
		this.serialPort.closePort();
	}
	
	/*
	 *	Writes the passed in byte to the Serial Port.
	 * 	Throws an exception if the serialPort is not open ,
	 * 	or is busy or the device is disconnected
	 * */
	private void write(byte data) throws SerialPortException {
		this.serialPort.writeByte(data);
	}
	
	private byte read() throws SerialPortException{
		byte[] temp = null;
		//temp = this.serialPort.readBytes(1, 100);
		temp = this.serialPort.readBytes(1);
		if (temp == null) {
			throw new SerialPortException(this.serialPort.getPortName(),
					"SerialCOM : read()", "Can't read from Serial Port");
		} else {
			return temp[0];
		}
	}
	
	/*
	 *	The methods which will be public and will be called by
	 *	other parts of code to send specific instructions to the 
	 *	Arduino to be executed on the 8051 micro-controller.
	 * */
	
	public boolean testComm() throws SerialPortException {
		if (!this.serialPort.isOpened()) // if port is not open return false
			return false;				 
		
		this.write(SerialCOM.TEST);
		
		if (this.read() == SerialCOM.SUCCESS)
			return true;
		return false;
	}
	
	public boolean enableProc() throws SerialPortException {
		this.write(SerialCOM.ENABLE_PROGRAMMING);

		if (this.read() == SerialCOM.PROGRAMMING_ENABLED)
			return true;
		return false;
	}
	
	public boolean eraseChip() throws SerialPortException, SerialPortTimeoutException {
		this.write(SerialCOM.ERASE_CHIP);
		
		if (this.read() == SerialCOM.SUCCESS)
			return true;
		return false;
	}
	
	public byte readByte(byte addrH, byte addrL) throws SerialPortException {
		this.write(SerialCOM.READ_BYTE);
		this.write(addrH);
		this.write(addrL);
		
		return this.read();
	}
	
	public void writeByte(byte addrH, byte addrL, byte data) throws SerialPortException {
		this.write(SerialCOM.WRITE_BYTE);
		this.write(addrH);
		this.write(addrL);
		this.write(data);
	}
	
	public void writeLockBits(byte lockBits) throws SerialPortException {
		this.write(SerialCOM.WRITE_LOCK_BITS);
		this.write(lockBits);
	}
	
	public byte readLockBits() throws SerialPortException {
		this.write(SerialCOM.READ_LOCK_BITS);
		
		return this.read();
	}
	
	public byte readSigByte(byte addr) throws SerialPortException {
		this.write(SerialCOM.READ_SIGNATURE_BYTE);
		this.write(addr);
		
		return this.read();
	}
	
	public boolean disableProc() throws SerialPortException {
		this.write(SerialCOM.EXIT);
		
		if (this.read() == SerialCOM.SUCCESS)
			return true;
		return false;
	}
}
////////////////////////////////// JUNK ///////////////////////////////////////////
//	/*
//	 *	Returns the first byte stored in the Linked List of data received from
//	 *	the Arduino. It will return -1 if the Linked List is empty
//	 * */
//	private byte read() {
//		if (this.serialInput.isEmpty())
//			return -1;
//		return this.serialInput.removeFirst();
//	}
	
//	/*
//	 * 	Returning the size of the Linked List storing the Serial Data recieved from
//	 * 	the Arduino.
//	 * 	Also to make program work we need to add in a delay
//	 * */
//	public int available() {
//		return this.serialInput.size();
//	}
//	

//	/*
//	 *	The SerialPortListener which listens for any Serial Data being sent by
//	 *	the Arduino and stores it at the end of a Linked List.
//	 * */
//	@Override
//	public synchronized void serialEvent(SerialPortEvent serialPortEvent) {
//		if (serialPortEvent.isRXCHAR()) { // if we receive data
//			if (serialPortEvent.getEventValue() > 0) { // if there is some existent data
//				try {
//					byte[] bytes = this.serialPort.readBytes(); // reading the bytes received on serial port
//					if (bytes != null) {
//						for (byte b : bytes) {
//							this.serialInput.add(b); // adding the bytes to the linked list
//							
//							// *** DEBUGGING *** //
//							System.out.print(String.format("%X ", b));
//						}
//					}			
//				} catch (SerialPortException e) {
//					System.out.println(e);
//					e.printStackTrace();
//				}
//			}
//		}
//		
//	}
//	