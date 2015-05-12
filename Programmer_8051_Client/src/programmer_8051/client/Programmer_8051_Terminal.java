package programmer_8051.client;

import intelhex.IntelHexFile;
import intelhex.IntelHexRecord;
import intelhex.IntelHexRecord.CheckSumFailException;
import intelhex.IntelHexRecord.IncorrectRecordException;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Scanner;

import jssc.SerialPortException;
import jssc.SerialPortTimeoutException;

public class Programmer_8051_Terminal {
	SerialCOM serialCom = null;
	Scanner scanner = new Scanner(System.in);
	
	public void exec() {
		String temp;
		System.out.println(Terminal_Strings.WELCOME_STRING);
		System.out.println(Terminal_Strings.LICENSE_STATEMENT);
		
		
		while(true) {
			System.out.print(Terminal_Strings.PROMPT);
			temp = this.scanner.next();
			this.pasreCommand(temp);
		}
	}
	
	private void pasreCommand(String command) {
		switch(command) {
		case Terminal_Strings.COMMAND_HELP:
			this.commandHelp();
			break;
		case Terminal_Strings.COMMAND_CONNECT:
			this.commandConnect();
			break;
		case Terminal_Strings.COMMAND_ENABLE:
			this.commandEnable();
			break;
		case Terminal_Strings.COMMAND_DISCONNECT:
			this.commandDisconnect();
			break;
		case Terminal_Strings.COMMAND_STATUS:
			this.commandStatus();
			break;
		case Terminal_Strings.COMMAND_PROGRAM:
			this.commandProgram();
			break;
		case Terminal_Strings.COMMAND_ERASE:
			this.commandErase();
			break;
		case Terminal_Strings.COMMAND_DUMP:
			this.commandDump();
			break;
		default:
				System.out.println(String.format(Terminal_Strings.UNKNOWN_COMMAND, command));
		}
	}
	
	////// The method which will be called if a command is entered in the prompt
	private void commandHelp() {
		for (String temp : Terminal_Strings.COMMAND_LIST) {
			System.out.println(temp);
		}
	}
	
	private void commandConnect() {
		int port;
		int baudRate;	

		try {
			do {
				System.out.print("Enter the COM port: ");
				port = this.scanner.nextInt();
				
				System.out.println("Enter the baud rate: ");
				baudRate = this.scanner.nextInt();
			} while (port <= 0 || baudRate <= 0);
			this.serialCom = new SerialCOM(port, baudRate);			
			System.out.println("Success: Connection successfully made over at COM" + port + " with baud rate: " + baudRate);
			
		} catch (SerialPortException e) {
			System.out.println(e);
			e.printStackTrace();
		}
	}
	
	private void commandEnable() {
		// After the connection has been made we need to enable programming
		try {
			if (this.serialCom.enableProc())
				System.out.println("Success: Programming has successfully been enabled!");
			else
				System.out.println("Failure: Programming NOT enabled!!");
		} catch (SerialPortException e) {
			System.out.println(e);
			e.printStackTrace();
		}
	
	}
	
	private void commandDisconnect() {
		try {
			if (this.serialCom.disableProc()) // disabling programming
				System.out.println("Programming successfully disabled!");
			this.serialCom.end();
		} catch (SerialPortException e) {
			System.out.println(e);
			e.printStackTrace();
		} finally {
			this.serialCom = null; // destroy the object
		}
	}
	
	private boolean commandStatus() {
		try {
			if (this.serialCom == null) {
				System.out.println("Error: No connection to the Arduino detected! Use 'connect' command.");
				return false;
			} else if (!this.serialCom.testComm()) {
				System.out.println("Error: No response from the Arduino! Try resetting it and check if it is connected properly.");
				return false;
			} else {
				System.out.println("Success: Arduino board responding at COM" + this.serialCom.port + " with baud rate: " + this.serialCom.baudRate);
				return true;
			}
		} catch (SerialPortException e) {
			System.out.println(e);
			e.printStackTrace();
		} 
		return false;
	}
	
	private void commandProgram() {
		System.out.println("Enter file name with complete path: ");
		String fileName = this.scanner.next();
		System.out.println(fileName);
		IntelHexRecord[] records;
		try {
			records = IntelHexFile.readRecordsFromFile(fileName);
			for (IntelHexRecord temp: records) {
				if (temp.getRecordType() == IntelHexRecord.RECORD_DATA) {
					byte addrH = temp.getAddrH();
					byte addrL = temp.getAddrL();
					byte[] data = temp.getDataSequence();
					
					for (int i = 0; i < data.length; i++) {
						this.serialCom.writeByte(addrH, addrL, data[i]);
						addrL++;
						if (addrL == 0)
							addrH++;
					}
				} else if (temp.getRecordType() == IntelHexRecord.RECORD_END_OF_FILE) {
					System.out.println("Success: 8051 is successfully programmed!");
					break;
				}
			}
		} catch (IOException | CheckSumFailException | IncorrectRecordException | SerialPortException e) {
			System.out.println(e);
			e.printStackTrace();
		}
	}
	
	private void commandErase() {
		System.out.println("Erasing the Chip! Wait 5 secs max.");
		if (this.commandStatus()) {
			try {
				if (this.serialCom.eraseChip())
					System.out.println("Success: Chip successfully erased!");
				else 
					System.out.println("Failure: Chip erase un-successfull!!");
			} catch (SerialPortException | SerialPortTimeoutException e) {
				System.out.println(e);
				e.printStackTrace();
			}
		}
	}
	
	private void commandDump() {
		System.out.println("Enter the file name to dump ROM: ");
		String fileName = this.scanner.next();
		System.out.println("Dumping ROM contents to " + fileName);
		BufferedWriter bw;
		byte addrH = 0;
		byte addrL = 0;
		byte temp;
		try {
			bw = new BufferedWriter(new FileWriter(new File(fileName)));
			for (int i = 0; i < 4000; i++) {
				temp = this.serialCom.readByte(addrH, addrL);
				bw.write(String.format("%02X ", temp));
				
				addrL++;
				if (addrL == 0){ // if an overflow occurs we need to increment by higher byte of address
					addrH++;
					bw.write("\r\n");
				}
			}
			bw.close();
		} catch (IOException | SerialPortException e) {
			System.out.println(e);
			e.printStackTrace();
		}
	}
	
	private static class Terminal_Strings {
		public static final String WELCOME_STRING = "****************** WELCOME TO 8051 PROGRAMMER BY ma_anjum95 *******************";
		public static final String LICENSE_STATEMENT = "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, \n" 
		+ "INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE \n"
		+ "AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, \n"
		+ "DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF \n"
		+ "OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\n"
		+ "BY CONTINUING TO USE THIS SOFTWARE YOU ARE AGREEING TO THESE CONDITIONS.\n";
		public static final String PROMPT = ">>>>> ";
		public static final String UNKNOWN_COMMAND = "Error: %S is not a valid command! Type 'help' for the command list.";
		
		// The valid commands that can be used
		public static final String COMMAND_HELP = "help";
		public static final String COMMAND_CONNECT = "connect";
		public static final String COMMAND_ENABLE = "enable";
		public static final String COMMAND_DISCONNECT = "disconnect";
		public static final String COMMAND_STATUS = "status";
		public static final String COMMAND_PROGRAM = "program";
		public static final String COMMAND_ERASE = "erase";
		public static final String COMMAND_DUMP = "dump";
		
		public static final String[] COMMAND_LIST = {
			COMMAND_HELP + " : Opens up this menu with the command list.",
			COMMAND_CONNECT + " : Connects the client to the Arduino over serial COM.",
			COMMAND_ENABLE + " : Enables the Programming on 8051.",
			COMMAND_DISCONNECT + " : Disconnects the connection to the Arduino.",
			COMMAND_STATUS + " : Prints the current status of the connection.",
			COMMAND_PROGRAM + " : Programs the 8051 using prompt input from the user.",
			COMMAND_ERASE + " : Erases the 8051 chip, ready for programming.",
			COMMAND_DUMP + " : Dumps the contents of 8051 ROM in a file."
		};
	}
}
