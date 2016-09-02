/*
  Multple serial test
 
  Receives from the main serial port, sends to the others. 
  Receives from serial port 1, sends to the main serial (Serial 0).
 
  The circuit: 
  * Any serial device attached to Serial port 1
  * Serial monitor open on Serial port 0:
 
  created 30 Dec. 2008
  by Tom Igoe
 
  This example code is in the public domain.
 
*/

#define ACK 0x06
#define NACK 0x15
#define CSFAIL 0x51
#define debug 1
#define debugState 1
#define debugVerbose 0
#define debugStateThree 0
#define debugHEX 1
#define debugDEC 0

byte requestData[] = {0xA5, 0x08, 0x41, 0x00, 0x02, 0x4E, 0x1C, 0x5A};
int timeSlot = 300; //Lowest 200
unsigned long time0 = 0;
unsigned long time1 = 0;
int receiveCounter = 0;
int bytesToReceive = -1;

unsigned char reveivedData[50]; //Input array 
int outputCounter = 0;
int i = 0;
int state = 0;
unsigned char inChar;
unsigned char checksum;
unsigned int tempUInt_16;
signed int tempSInt_16;
unsigned long tempULong_32;
signed long tempSLong_32;

int availableBytes = 0;

void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  Serial1.begin(115200);
  
  // prints title with ending line break 
  Serial.println("\n\n\rRead Data from MCP39F511"); 
   
}

void loop() {
  //send read data command to MCP39F511
	time1 = millis();
	switch(state){
		case 0:
			if((time1 - time0) >= 1000){
				for(i = 0; i < sizeof(requestData); i++){
					Serial1.write(requestData[i]);
					delayMicroseconds(timeSlot);
				}
			time0 = millis();
			#if debugState
			Serial.println("0 Send Command");
			#endif
			state++;
			}
			break;
			
		case 1:
			availableBytes = Serial1.available();
			if(availableBytes > 0){
				inChar = (char)Serial1.read();
				if(inChar == ACK){
					reveivedData[receiveCounter] = inChar;
					checksum = inChar;
					receiveCounter++;
					state++;
					#if debug
					Serial.print("1 Receive first byte");
					Serial.print(" - inChar ");
					Serial.print(inChar, HEX);
					Serial.print(" - receiveCounter ");
					Serial.print(receiveCounter, HEX);
					Serial.print(" - state ");
					Serial.print(state, DEC);
					Serial.print(" - availableBytes ");
					Serial.println(availableBytes, DEC);
					#endif
				}
				else if(inChar == NACK){
					//Output error message
					Serial.println("Frame received with success command not understood or another error in command bytes");
					state--;
				}
				else if(inChar == CSFAIL){
					//Output error message
					Serial.println("Frame received with success, however, checksum did not match bytes in frame");
					state--;
				}
			}
			#if debugState
			Serial.println("State #1");
			#endif
			break;
			
		case 2:
			availableBytes = Serial1.available();
			if(availableBytes > 0){
				inChar = (char)Serial1.read();
				reveivedData[receiveCounter] = inChar;
				bytesToReceive = inChar;
				checksum += inChar;
				receiveCounter++;
				state++;
				#if debug
				Serial.print("2 Receive number of bytes to receive");
				Serial.print(" - inChar ");
				Serial.print(inChar, HEX);
				Serial.print(" - receiveCounter ");
				Serial.print(receiveCounter, HEX);
				Serial.print(" - bytesToReceive ");
				Serial.print(bytesToReceive, DEC);
				Serial.print(" - state ");
				Serial.print(state, DEC);
				Serial.print(" - availableBytes ");
				Serial.println(availableBytes, DEC);
				#endif
			}
			#if debugState
			Serial.println("State #2");
			#endif
			break;
			
		case 3:
			availableBytes = Serial1.available();
			if(availableBytes > 0){
				inChar = (char)Serial1.read();
				reveivedData[receiveCounter] = inChar;
				receiveCounter++;
				if(receiveCounter < bytesToReceive){
					checksum += inChar;
				}
				if(receiveCounter == bytesToReceive){
					state++;
					#if debugState
					Serial.println("State #3 -> #4");
					#endif
				}
				#if debug
				Serial.print("3 Receive remaining bytes");
				Serial.print(" - inChar ");
				Serial.print(inChar, HEX);
				Serial.print(" - receiveCounter ");
				Serial.print(receiveCounter, HEX);
				Serial.print(" - bytesToReceive ");
				Serial.print(bytesToReceive, DEC);
				Serial.print(" - state ");
				Serial.print(state, DEC);
				Serial.print(" - availableBytes ");
				Serial.println(availableBytes, DEC);
				#endif
			}
			#if debugStateThree
			Serial.println("State #3");
			#endif
			break;
			
		case 4:
			#if debugDEC
			// Serial.print("\n\rNumber of Bytes - 0x");
			// Serial.print(reveivedData[1], DEC);
			
			// Serial.print("\n\rSystem Status - 0x");
			// Serial.print(reveivedData[3], HEX);
			// Serial.print(reveivedData[2], HEX);
			
			// Serial.print("\n\rSystem Version - 0x");
			// Serial.print(reveivedData[5], HEX);
			// Serial.print(reveivedData[4], HEX);
			
			// Serial.print("\n\rVoltage RMS x 10 = 0x");
			// Serial.print(reveivedData[7], HEX);
			// Serial.print(reveivedData[6], HEX);
						
			// Serial.print("\n\rLine Frequency x 1000 = 0x");
			// Serial.print(reveivedData[9], HEX);
			// Serial.print(reveivedData[8], HEX);
						
			// Serial.print("\n\rAnalog Input Voltage = 0x");
			// Serial.print(reveivedData[11], HEX);
			// Serial.print(reveivedData[10], HEX);
						
			// Serial.print("\n\rPower Factor / 2^(-15) = 0x");
			// Serial.print(reveivedData[13], HEX);
			// Serial.print(reveivedData[12], HEX);
			
			// Serial.print("\n\rCurent RMS x 1000 = 0x");
			// Serial.print(reveivedData[17], HEX);
			// Serial.print(reveivedData[16], HEX);
			// Serial.print(reveivedData[15], HEX);
			// Serial.print(reveivedData[14], HEX);
						
			// Serial.print("\n\rActive Power x 100 = 0x");
			// Serial.print(reveivedData[21], HEX);
			// Serial.print(reveivedData[20], HEX);
			// Serial.print(reveivedData[19], HEX);
			// Serial.print(reveivedData[18], HEX);
									
			// Serial.print("\n\rReactive Power x 100 = 0x");
			// Serial.print(reveivedData[25], HEX);
			// Serial.print(reveivedData[24], HEX);
			// Serial.print(reveivedData[23], HEX);
			// Serial.print(reveivedData[22], HEX);
									
			// Serial.print("\n\rApparent Power x 100 = 0x");
			// Serial.print(reveivedData[29], HEX);
			// Serial.print(reveivedData[28], HEX);
			// Serial.print(reveivedData[27], HEX);
			// Serial.print(reveivedData[26], HEX);
			
			// Serial.print("\n\rChecksum Value Reveived = 0x");
			// Serial.print(reveivedData[30], HEX);
			// Serial.print("\n\rChecksum Value Calculated = 0x");
			// Serial.print(checksum, HEX);
			#endif
		
		
			#if debugHEX
			Serial.print("ACK - 0x");
			Serial.print(reveivedData[0], HEX);
			Serial.print("\n\rNumber of Bytes - 0x");
			Serial.print(reveivedData[1], HEX);
			
			Serial.print("\n\rSystem Status - 0x");
			Serial.print(reveivedData[3], HEX);
			Serial.print(reveivedData[2], HEX);
			
			Serial.print("\n\rSystem Version - 0x");
			Serial.print(reveivedData[5], HEX);
			Serial.print(reveivedData[4], HEX);
			
			Serial.print("\n\rVoltage RMS x 10 = 0x");
			Serial.print(reveivedData[7], HEX);
			Serial.print(reveivedData[6], HEX);
						
			Serial.print("\n\rLine Frequency x 1000 = 0x");
			Serial.print(reveivedData[9], HEX);
			Serial.print(reveivedData[8], HEX);
						
			Serial.print("\n\rAnalog Input Voltage = 0x");
			Serial.print(reveivedData[11], HEX);
			Serial.print(reveivedData[10], HEX);
						
			Serial.print("\n\rPower Factor / 2^(-15) = 0x");
			Serial.print(reveivedData[13], HEX);
			Serial.print(reveivedData[12], HEX);
			
			Serial.print("\n\rCurent RMS x 1000 = 0x");
			Serial.print(reveivedData[17], HEX);
			Serial.print(reveivedData[16], HEX);
			Serial.print(reveivedData[15], HEX);
			Serial.print(reveivedData[14], HEX);
						
			Serial.print("\n\rActive Power x 100 = 0x");
			Serial.print(reveivedData[21], HEX);
			Serial.print(reveivedData[20], HEX);
			Serial.print(reveivedData[19], HEX);
			Serial.print(reveivedData[18], HEX);
									
			Serial.print("\n\rReactive Power x 100 = 0x");
			Serial.print(reveivedData[25], HEX);
			Serial.print(reveivedData[24], HEX);
			Serial.print(reveivedData[23], HEX);
			Serial.print(reveivedData[22], HEX);
									
			Serial.print("\n\rApparent Power x 100 = 0x");
			Serial.print(reveivedData[29], HEX);
			Serial.print(reveivedData[28], HEX);
			Serial.print(reveivedData[27], HEX);
			Serial.print(reveivedData[26], HEX);
			
			Serial.print("\n\rChecksum Value Reveived  =  0x");
			Serial.print(reveivedData[30], HEX);
			Serial.print("\n\rChecksum Value Calculated = 0x");
			Serial.println(checksum, HEX);
			#endif
			
			#if debugVerbose
			for(outputCounter = 0; outputCounter < receiveCounter; outputCounter++){
				Serial.print(reveivedData[outputCounter], HEX);
				Serial.print(" - outputCounter ");
				Serial.print(outputCounter, DEC);
				Serial.print(" - receiveCounter ");
				Serial.println(receiveCounter, HEX);
			}
			#endif
			receiveCounter = 0;
			state = 0;
			#if debugState
			Serial.println("State #4");
			#endif
			break;

		// default :
			// break;
	}
 
}



