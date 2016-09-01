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
#define debugStateThree 0

byte requestData[] = {0xA5, 0x08, 0x41, 0x00, 0x02, 0x4E, 0x20, 0x5E};
int timeSlot = 300; //Lowest 200
unsigned long time0 = 0;
unsigned long time1 = 0;
int receiveCounter = 0;
int bytesToReceive = -1;

unsigned char reveivedData[50]; //Input array 
boolean stringComplete = false; // whether the string is complete
int outputCounter = 0;
int i = 0;
int state = 0;
unsigned char inChar;

int availableBytes = 0;

void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  Serial1.begin(115200);
  // reserve 200 bytes for the inputString:
  // inputString.reserve(200);
  
  // prints title with ending line break 
  Serial.println("Read Data from MCP39F511"); 
   
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
				}
				else if(inChar == CSFAIL){
					//Output error message
					Serial.println("Frame received with success, however, checksum did not match bytes in frame");
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
			for(outputCounter = 0; outputCounter < receiveCounter; outputCounter++){
				Serial.print(reveivedData[outputCounter], HEX);
				Serial.print(" - outputCounter ");
				Serial.print(outputCounter, DEC);
				Serial.print(" - receiveCounter ");
				Serial.println(receiveCounter, HEX);
			}
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

 /*
   SerialEvent occurs whenever a new data comes in the
   hardware serial RX. This routine is run between each
   time loop() runs, so using delay inside loop can delay
   response. Multiple bytes of data may be available.
 */
 // void serialEvent() {
   // while (Serial1.available()) {
	   // Serial.println("9 Serial Data Avialable - SerialEvent");
     // // get the new byte:
     // char inChar = (char)Serial1.read(); 
     // if(receiveCounter == 1){
      // bytesToReceive = inChar;
	  // Serial.println("A Number of Bytes to Receive - ");
     // }
     // // add it to the inputString:
     // // inputString += inChar;	//TODO: Remove
	 // reveivedData[receiveCounter] = inChar;
	 // Serial.println("B Input Data Read - ");
     // // if the incoming character is a newline, set a flag
     // // so the main loop can do something about it:
     // if ((bytesToReceive - 1) == receiveCounter) {
     // stringComplete = true;
	 // Serial.println("C Completed Input Data Read - ");
     // } 
	 // receiveCounter++;
	 // Serial.println("D End of SerialEvent");
   // }
 // }



