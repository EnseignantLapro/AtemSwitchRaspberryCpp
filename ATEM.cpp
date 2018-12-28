/*
Copyright 2012 Kasper Skårhøj, SKAARHOJ, kasperskaarhoj@gmail.com

This file is part of the ATEM library for Arduino

The ATEM library is free software: you can redistribute it and/or modify 
it under the terms of the GNU General Public License as published by the 
Free Software Foundation, either version 3 of the License, or (at your 
option) any later version.

The ATEM library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE. 
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with the ATEM library. If not, see http://www.gnu.org/licenses/.

*/

#include "ATEM.h"


using namespace std;
//JLA Fonction rajouté conversion Arduino->c++
unsigned long millis()
{
	unsigned long sysTime = time(0);
	return sysTime*1000;
	
}

//JLA concatenation 2 * 8bit => to 16 bit value
unsigned int word(uint8_t value15_8,uint8_t value0_7)
{
	return value15_8 << 8 | value0_7;
   
}

//JLA portage arduino to c++
uint8_t highByte(uint16_t value15_8){

	return (uint8_t)value15_8 & 0xFF;
	
	
}
//JLA portage arduino to c++
uint8_t lowByte(uint16_t value0_7){

	return (uint8_t)value0_7 >> 8;;
}




//#include <MemoryFree.h>

/**
 * Constructor (using arguments is deprecated! Use begin() instead)
 */
ATEM::ATEM(){}
ATEM::ATEM(SOCKADDR_IN ip, const uint16_t localPort){
	_ATEM_FtbS_state = false;

	//begin(ip);
}

/**
 * Setting up IP address for the switcher (and local port to send packets from)
 */
void ATEM::begin(SOCKADDR_IN ip){
		// Set up Udp communication object:
	EthernetUDP Udp;
	_Udp = Udp;
	
	_switcherIP = ip;	// Set switcher IP address
	//_localPort = localPort;	// Set local port (just a random number I picked)

	
	_isConnectingTime = 0;
	
	_ATEM_AMLv_channel=0;
}


/**
 * Get ATEM session ID
 */
uint16_t ATEM::getSessionID() {
	return _sessionID;
}

/**
 * Initiating connection handshake to the ATEM switcher
 * Modified by JLA for work on raspberry
 */
bool ATEM::connect() {
	_isConnectingTime = millis();
	// Init localPacketIDCounter to 1;
	_hasInitialized = false;
	_isConnected = false;
	_lastContact = 0;
	_Udp.begin(_switcherIP);

		// Setting this, because even though we haven't had contact, it constitutes an attempt that should be responded to at least:
	_lastContact = millis();

	// Send connectString to ATEM:
	// TODO: Describe packet contents according to rev.eng. API
	if (_serialOutput) 	{
  		cout << "Sending packet connect to ATEM switcher.";
	}
	//Session : 0x53, 0xAB,
	//Client ID 0xC1
	uint8_t connectHello[] = {  
		0x10, 0x14, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint16_t packetLength = 0;
	while(true) {
		if (_serialOutput) cout << "Send init ";
		_Udp.write(connectHello,20);
		_Udp.endPacket();   

		//Read Init Reponse you have to wait à little the response
		if(_Udp.available() &&_Udp.parsePacket()==20 ){
			if (_serialOutput) cout << "Read 20   ";
			_Udp.read(_packetBuffer,20);
			uint16_t packetLength = word(_packetBuffer[0] & 0x07, _packetBuffer[1]);
			if(packetLength==20){
				//Envoi ACK 
				_localPacketPingIdCounter++ ;
				cout << "-rep-";
				//0x80, 0x0c, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3A, 0x00, 0x00 };
				uint8_t connectHelloAnswerString[] = {  
				0x80, 0x0c, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00 };
				_Udp.write(connectHelloAnswerString,12);
				_Udp.endPacket();
				_isConnected = true;

				
				return  1;
			}

		}else{
			if ( 2000 < (unsigned long)millis())	{
				if (_serialOutput) 	{
		      		cout <<"Timeout waiting for ATEM switcher response";	
				}
				break;
			}
		}
	}
	return 0;
}
void ATEM::deconnect() {
	_isConnected = false;
	_threadAtemContinueConnet.join();
	_Udp.endsock();
}

/**
 * Keeps connection to the switcher alive - basically, this means answering back to ping packages.
 * Therefore: Call this in the Arduino loop() function and make sure it gets call at least 2 times a second
 * Other recommendations might come up in the future.
 */

bool ATEM::InitAtemConnection() {
	cout << "Init Atem Television studio ---------------------- \n";
	bool connect = ATEM::connect();
	return connect;
}

//Dev by JAL for work on rapsberry c++
bool ATEM::AtemGetCommands() {
	cout << "Get Commands Atem Television ---------------------- \n";

	
	while(_Udp.available()){
		
		uint16_t packetSize = 0;
		packetSize = _Udp.parsePacket();
		
		//PacketInStock and size!0
		if (packetSize !=0){
			_lastContact = millis();
			//READ HEAD ATEM PROTOCOLE
			cout << "\033[1;33m"; //yellow
			_Udp.read(_packetBuffer, 12);
			cout << "\033[0m\n";  
			
			uint16_t packetLength = word(_packetBuffer[0] & 0x07, _packetBuffer[1]); //B00000111

			//New session by Atem Switch TV
			_sessionID = word(_packetBuffer[2], _packetBuffer[3]);

			//N° of switch packet ID
			_lastRemotePacketID = word(_packetBuffer[10],_packetBuffer[11]); 

			uint8_t command = _packetBuffer[0] & 0xF8;//B11111000;

			bool command_flag_fACK = command & 0x08 ? true : false;	//B00001000 If true, ATEM expects an acknowledgement answer back!
			bool command_flag_INIT = command & 0x10 ? true : false; //B00010000
			bool command_flag_RETR = command & 0x20 ? true : false; //B00100000
			bool command_flag_HELO = command & 0x40 ? true : false; //B01000000
			bool command_flag_ANSW = command & 0x80 ? true : false; //B10000000
			if(command_flag_fACK) cout << "-ACK-";
			if(command_flag_INIT) cout << "-INI-";
			if(command_flag_RETR) cout << "-RET-";
			if(command_flag_HELO) cout << "-HEL-";
			if(command_flag_ANSW) cout << "-REP-";
			
			if(packetLength > 12 && ( !command_flag_RETR && !command_flag_INIT  && !command_flag_HELO )){

				//decode Packet BMD UDP Protocol
				_parsePacket(packetLength);
				
				_hasInitialized = true;
				

			}else if (packetLength == 12 && command_flag_fACK && !command_flag_RETR){
				//it's a ping request
				_sendAnswerPacket(4);
				
			}else if (packetLength == 20 && command_flag_INIT){
				//Recend ACK for Retransmission init
				uint8_t connectHelloAnswerString[] = {  
				//0x80, 0x0c, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3A, 0x00, 0x00 };
				0x80, 0x0c, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00 };
				cout << "-REP-";
				 connectHelloAnswerString[2] = _sessionID >> 8;  // Session ID
  				connectHelloAnswerString[3] = _sessionID & 0xFF;  // Session ID
				_Udp.write(connectHelloAnswerString,12);
				_Udp.endPacket();
			}else if (command_flag_ANSW && !command_flag_RETR) {
				_parsePacket(packetLength);
			}else if(command_flag_fACK && command_flag_RETR){
				
					_sendAnswerPacket(4);
				
			}
			//else if : what à do with retransmission ?? because it's comming before à finish traitement of one message

			//envoi accusé de reception ACK
			//put ACK 1 and Switcher pkt id = _lastRemotePacketID

		}
	}

	//send ACK for the last command _lastRemotePacketID

	cout << "END Commands Atem Television ---------------------- \n";
	
	if(!_isContinuConnected){
		//JLA launch thread to keep the connection alive
		cout << "-ThreadGo-\n";
		_isContinuConnected = true;
		_threadAtemContinueConnet = std::thread(&ATEM::AtemContinueConnet,this);
	}

	if(_hasInitialized && _isContinuConnected){
		
		return 1;
	}

	return 0;

}



void ATEM::AtemContinueConnet() {
	cout << "Listen Continue Connet - \n";
	//send ping
	bool activeconnection =false;
	while (true)
    {
		while (!_Udp.writeavailable())
    	{
			activeconnection =true;
			usleep(100000);
		}

		if(_Udp.writeavailable()){
			_sendAnswerPacket(0);
			usleep(100000);
			activeconnection = AtemGetCommands();
		}

		if(!activeconnection){
			break;
		}
		
	}
	cout << "END Listen Atem Television ---------------------- \n";
	

}


// This function works only on arduino it dev by Skårhøj, SKAARHOJ, kasperskaarhoj@gmail.com
// a rewrite another function for rapsberry
//bool ATEM::InitAtemConnection() {
// And bool ATEM::AtemGetCommands() {
void ATEM::runLoop() {

  // WARNING:
  // It can cause severe timing problems using "slow" functions such as cout << *() 
  // in the runloop, in particular during "boot" where the ATEM delivers some 10-20 kbytes of system status info which
  // must exit the RX-buffer quite fast. Therefore, using cout <<  for debugging in this 
  // critical phase will in it self affect program execution!

  // Limit of the RX buffer of the Ethernet interface is another general issue.
  // When ATEM sends the initial system status packets (10-20 kbytes), they are sent with a few microseconds in between
  // The RX buffer of the Ethernet interface on Arduino simply does not have the kapacity to take more than 2k at a time.
  // This means, that we only receive the first packet, the others seems to be discarded. Luckily most information we like to 
  // know about is in the first packet (and some in the second, the rest is probably thumbnails for the media player).
  // It may be possible to bump up this buffer to 4 or 8 k by simply re-configuring the amount of allowed sockets on the interface.
  // For some more information from a guy seemingly having a similar issue, look here:
  // http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1282170842

	uint16_t packetSize = 0;

	
	if (_isConnectingTime > 0)	{
		cout <<"Init Atem Television studio\n";
			// Waiting for the ATEM to answer back with a packet 20 bytes long.
			// According to packet analysis with WireShark, this feedback from ATEM
			// comes within a few microseconds!
		
		packetSize = _Udp.parsePacket();
		//_Udp.parsePacket(); sert pour un datagramme complet
		//packetSize = 20;
		
		if (_Udp.available() && packetSize==20)   {  
				_lastContact = millis();	
			    cout <<"Size Packet : " << packetSize <<",";
				// Read the response packet. We will only subtract the session ID
				// According to packet analysis with WireShark, this feedback from ATEM
				// comes a few microseconds after our connect invitation above. Two packets immediately follow each other.
				// After approx. 200 milliseconds a third packet is sent from ATEM - a sort of re-sent because it gets impatient.
				// And it seems that THIS third packet is the one we actually read and respond to. In other words, I believe that 
				// the ethernet interface on Arduino actually misses the first two for some reason!

			_Udp.read(_packetBuffer,20);
			
			
			//_sessionID = _packetBuffer[15];

			// Send connectAnswerString to ATEM:
			_Udp.beginPacket(_switcherIP,  9910);
	
			// TODO: Describe packet contents according to rev.eng. API
			// Session 53ab
			// Client 3A
			//0x80, 0x0c, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3A, 0x00, 0x00 };
			uint8_t connectHelloAnswerString[] = {  
			  0x80, 0x0c, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			_Udp.write(connectHelloAnswerString,12);
			_Udp.endPacket();

			_isConnectingTime = 0;	// End connecting
		} else {
			if (_isConnectingTime+2000 < (unsigned long)millis())	{
				if (_serialOutput) 	{
		      		cout <<"Timeout waiting for ATEM switcher response";
				}
				_isConnectingTime = 0;
			}
		}
	} else {
		cout <<"Connected to Atem\n";



		// If there's data available, read a packet, empty up:
		// cout <<("ATEM runLoop():");
		int input = 2;
		while(true) {	// Iterate until buffer is empty:
			
			packetSize = _Udp.parsePacket();
			
			

			if (_Udp.available() && packetSize !=0)   {  
				_lastContact = millis();
				//	cout << ("New Packet");
				//	cout << (("PACKET: ";
					//  cout <<(packetSize, DEC);

				// Read packet header of 12 bytes:
				_Udp.read(_packetBuffer, 12);

				_sessionID = word(_packetBuffer[2], _packetBuffer[3]);

				// Read out packet length (first word), remote packet ID number and "command":
					//JLA word(_packetBuffer[0] & B00000111, _packetBuffer[1]); je pass en hexa
				uint16_t packetLength = word(_packetBuffer[0] & 0x07, _packetBuffer[1]); //B00000111
				_lastRemotePacketID = word(_packetBuffer[10],_packetBuffer[11]); 
				cout <<"\nRead " << packetSize <<" packet----";
				cout <<" switch Pk ID " << _lastRemotePacketID <<" --------------------------\n";
				
				uint8_t command = _packetBuffer[0] & 0xF8;//B11111000;
				bool command_ACK = command & 0x08 ? true : false;	//B00001000 If true, ATEM expects an acknowledgement answer back!
				bool command_INIT = command & 0x10 ? true : false;	//B000010000 If true, ATEM expects an acknowledgement answer back!
					// The five bits in "command" (from LSB to MSB):
				// 1 = ACK, "Please respond to this packet" (using the _lastRemotePacketID). Exception: The initial 10-20 kbytes of Switcher status
				// 2 = ?. Set during initialization? (first hand-shake packets contains that)
				// 3 = "This is a retransmission". You will see this bit set if the ATEM switcher did not get a timely response to a packet.
				// 4 = ? ("hello packet" according to "ratte", forum at atemuser.com)
				// 5 = "This is a response on your request". So set this when answering...
				

				if (packetSize==packetLength) {  // Just to make sure these are equal, they should be!
					_lastContact = millis();
					

					// If a packet is 12 bytes long it indicates that all the initial information 
					// has been delivered from the ATEM and we can begin to answer back on every request
					// Currently we don't know any other way to decide if an answer should be sent back...
					if(!_hasInitialized && packetSize == 12) {
						_hasInitialized = true;
						cout <<"\nInit Ok ";
						cout << "Session ID: ";
						printf("0x%02x", _sessionID);
						_sendAnswerPacket(_lastRemotePacketID);
						
					} else if(packetSize == 12){
						//ping
						cout <<"\nping ";
						_sendAnswerPacket(_lastRemotePacketID);
					}else if(packetSize == 20){
						cout <<"\nInit ";
						_sendAnswerPacket(_lastRemotePacketID);
						
					}else {
						if (packetLength > 12 && !command_INIT)	{	// !command_INIT is because there seems to be no commands in these packets and that will generate an error.
						_parsePacket(packetLength);
						_isConnected = true;
						}
						else
						{cout << "HEUUUU---------------HEUUUU-------------------HEUUUUUU: ";}
					}

					

					

					// If we are initialized, lets answer back no matter what:
					// TODO: "_hasInitialized && " should be inserted back before "command_ACK" but 
					// with Arduino 1.0 UDP library it has proven MORE likely that the initial
					// connection is made if we ALWAYS answer the switcher back.
					// Apparently the initial "chaos" of keeping up with the incoming data confuses 
					// the UDP library so that we might never get initialized - and thus never get connected
					// So... for now this is how we do it:
					// CHANGED with arduino 1.0.1..... put back in.
					if (_hasInitialized && command_ACK) {
						if (_serialOutput) {
							cout << "ACK, rpID: ";
							printf("0x%02x", _lastRemotePacketID);
							
						}
						_sendAnswerPacket(_lastRemotePacketID);
						
					}
					
				} else {
					if (_serialOutput) 	{
				 		cout << "ERROR: Packet size mismatch: ";
						cout << packetSize;
						cout << " != ";
						cout << packetLength;
					}
					// Flushing the buffer:
					// TODO: Other way? _Udp.flush() ??
					while(_Udp.available()) {
						_Udp.read(_packetBuffer, 96);
					}
				}
			} else {
				_sendAnswerPacket(_lastRemotePacketID);
				//Send Ping
				//0x80, 0x0c, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3A, 0x00, 0x00 };
				uint8_t Ping[] = {  
				
				0x08, 0x0c, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
				Ping[2] = _sessionID >> 8;  // Session ID
  				Ping[3] = _sessionID & 0xFF;  // Session ID
				Ping[11] = _ClientPakID++;
				_Udp.write(Ping,12);
				_Udp.endPacket();
				break;	// Exit while(true) loop because there is no more packets in buffer.
			}

			
			
		}
	}
}

bool ATEM::isConnectionTimedOut()	{
	unsigned long currentTime = millis();
	unsigned long TimeContactOut = _lastContact; // Timeout of 30 sec.

	
	
	TimeContactOut += 30000;
	
	if (_lastContact>0 && (TimeContactOut < currentTime))	{	
		_lastContact = 0;
		_isConnected = false;
		return true;
	}
	return false;
}

bool ATEM::isConnected()	{
	return _isConnected;
}

void ATEM::delay(const unsigned int delayTimeMillis)	{	// Responsible delay function which keeps the ATEM run loop up! DO NOT USE INSIDE THIS CLASS! Recursion could happen...
	unsigned long timeout = millis();
	timeout+=delayTimeMillis;

	while(timeout > millis())	{
		runLoop();
	}
}

/**
 * Reads from UDP channel to buffer. Will fill the buffer to the max or to the size of the current segment being parsed
 * Returns false if there are no more bytes, otherwise true 
 */
bool ATEM::_readToPacketBuffer() {
	return _readToPacketBuffer(96);
}
bool ATEM::_readToPacketBuffer(uint8_t maxBytes) {
	maxBytes = maxBytes<=96 ? maxBytes : 96;
	int remainingBytes = _cmdLength-8-_cmdPointer;

	if (remainingBytes>0)	{
		if (remainingBytes <= maxBytes)	{
			_Udp.read(_packetBuffer, remainingBytes);
			_cmdPointer+= remainingBytes;
			return false;	// Returns false if finished.
		} else {
			_Udp.read(_packetBuffer, maxBytes);
			_cmdPointer+= maxBytes;
			return true;	// Returns true if there are still bytes to be read.
		}
	} else {
		return false;
	}
}

/**
 * If a package longer than a normal acknowledgement is received from the ATEM Switcher we must read through the contents.
 * Usually such a package contains updated state information about the mixer
 * Selected information is extracted in this function and transferred to internal variables in this library.
 */
void ATEM::_parsePacket(uint16_t packetLength)	{
	 uint8_t idx;	// General reusable index usable for keyers, mediaplayer etc below.
	
 		// If packet is more than an ACK packet (= if its longer than 12 bytes header), lets parse it:
      uint16_t indexPointer = 12;	// 12 bytes has already been read from the packet...
      
    while (indexPointer < packetLength)  {

        // Read the length of segment (first word):
        _Udp.read(_packetBuffer, 8);
        _cmdLength = word(_packetBuffer[0], _packetBuffer[1]);
		_cmdPointer = 0;
        
			// Get the "command string", basically this is the 4 char variable name in the ATEM memory holding the various state values of the system:
        char cmdStr[] = { 
          _packetBuffer[4], _packetBuffer[5], _packetBuffer[6], _packetBuffer[7], '\0'};

			// If length of segment larger than 8 (should always be...!)
        if (_cmdLength>8)  {
			if(strcmp(cmdStr, "AMLv"))	{
				
			  _readToPacketBuffer();	// Fill packet buffer unless it's AMLv (AudioMonitorLevels)
			}
			if (_serialOutput) cout << "Command: ";
			// Extract the specific state information we like to know about:
			if(strcmp(cmdStr, "PrgI") == 0) { 
				if (_serialOutput) cout << " PrgI "; // Program Bus status
				if (!ver42())	{
					_ATEM_PrgI = _packetBuffer[1];
				} else {
					_ATEM_PrgI = (uint16_t)(_packetBuffer[2]<<8) | _packetBuffer[3];
				}
				if (_serialOutput) cout << "Program Bus: ";
				if (_serialOutput) cout << _ATEM_PrgI;
				cout << "\033[1;36m"; //cyan
				cout << "New Input ! " << _ATEM_PrgI;
				cout << "\033[0m\n"; 
			} else
			if(strcmp(cmdStr, "PrvI") == 0) { 
				 // Preview Bus status
				if (_serialOutput) cout << " PrvI ";
				if (!ver42())	{
					_ATEM_PrvI = _packetBuffer[1];
				} else {
					_ATEM_PrvI = (uint16_t)(_packetBuffer[2]<<8) | _packetBuffer[3];
				}
				if (_serialOutput) cout << "Preview Bus: ";
				if (_serialOutput) cout << _ATEM_PrvI;
			} else
			if(strcmp(cmdStr, "TlIn") == 0) {
				  // Tally status for inputs 1-8
				 if (_serialOutput)cout << " TlIn ";
				uint8_t count = _packetBuffer[1]; // Number of inputs
				// 16 inputs supported so make sure to read max 16.
				if(count > 16) {
				count = 16;
				}
					// Inputs 1-16, bit 0 = Prg tally, bit 1 = Prv tally. Both can be set simultaneously.
				if (_serialOutput) cout <<"Tally updated: ";
				for(uint8_t i = 0; i < count; ++i) {
				_ATEM_TlIn[i] = _packetBuffer[2+i];
				}

			} else 
			if(strcmp(cmdStr, "Time") == 0) { 
				if (_serialOutput) cout << " Time "; // Time. What is this anyway?
			/*	cout << (_packetBuffer[0]);
				cout << (':');
				cout << (_packetBuffer[1]);
				cout << (':');
				cout << (_packetBuffer[2]);
				cout << (':');
				cout << (_packetBuffer[3]);
				cout <<();
			*/} else 
			if(strcmp(cmdStr, "TrPr") == 0) { 
				 // Transition Preview
				 if (_serialOutput) cout << " TrPr ";
				_ATEM_TrPr = _packetBuffer[1] > 0 ? true : false;
				if (_serialOutput) cout << "Transition Preview: a convertire en binaire :";
				if (_serialOutput) cout << _ATEM_TrPr;
			} else
			if(strcmp(cmdStr, "TrPs") == 0) { 
				if (_serialOutput) cout << " TrPs "; // Transition Position
				_ATEM_TrPs_frameCount = _packetBuffer[2];	// Frames count down
				_ATEM_TrPs_position = _packetBuffer[4]*256 + _packetBuffer[5];	// Position 0-1000 - maybe more in later firmwares?
			} else
			if(strcmp(cmdStr, "TrSS") == 0) {  // Transition Style and Keyer on next transition
				_ATEM_TrSS_KeyersOnNextTransition = _packetBuffer[2] & 0x1F; //B11111;	// Bit 0: Background; Bit 1-4: Key 1-4
				if (_serialOutput) cout << "Keyers on Next Transition: ";
				if (_serialOutput) cout << _ATEM_TrSS_KeyersOnNextTransition;

				_ATEM_TrSS_TransitionStyle = _packetBuffer[1];
				if (_serialOutput) cout << "Transition Style: ";	// 0=MIX, 1=DIP, 2=WIPE, 3=DVE, 4=STING
				if (_serialOutput) cout << _ATEM_TrSS_TransitionStyle;
			} else
			if(strcmp(cmdStr, "FtbS") == 0) {  // Fade To Black State
				_ATEM_FtbS_state = _packetBuffer[2]| _packetBuffer[1]; // State of Fade To Black, 0 = off and 1 = activated
				_ATEM_FtbS_frameCount = _packetBuffer[3];	// Frames count down
				if (_serialOutput) cout << "FTB:";
				if (_serialOutput) cout << (_ATEM_FtbS_state);
				if (_serialOutput) cout << "/";
				if (_serialOutput) cout <<(_ATEM_FtbS_frameCount);
			} else
			if(strcmp(cmdStr, "FtbP") == 0) { 
				if (_serialOutput) cout << " FtbP "; // Fade To Black - Positions(?) (Transition Time in frames for FTB): 0x01-0xFA
				_ATEM_FtbP_time = _packetBuffer[1];
			} else
			if(strcmp(cmdStr, "TMxP") == 0) {
				if (_serialOutput) cout << " TMxP ";
				  // Mix Transition Position(?) (Transition Time in frames for Mix transitions.): 0x01-0xFA
				_ATEM_TMxP_time = _packetBuffer[1];
			} else
			if(strcmp(cmdStr, "DskS") == 0) { 
				if (_serialOutput) cout << " DskS ";
				 // Downstream Keyer state. Also contains information about the frame count in case of "Auto"
				idx = _packetBuffer[0];
				if (idx >=0 && idx <=1)	{
					_ATEM_DskOn[idx] = _packetBuffer[1] > 0 ? true : false;
					if (_serialOutput) cout << "Dsk Keyer ";
					if (_serialOutput) cout << (idx+1);
					if (_serialOutput) cout << ": ";
					if (_serialOutput) cout << _ATEM_DskOn[idx];
				}
			} else
			if(strcmp(cmdStr, "DskP") == 0) {  // Downstream Keyer Tie
				if (_serialOutput) cout << " DskP ";
				idx = _packetBuffer[0];
				if (idx >=0 && idx <=1)	{
					_ATEM_DskTie[idx] = _packetBuffer[1] > 0 ? true : false;
					if (_serialOutput) cout << "Dsk Keyer";
					if (_serialOutput) cout << (idx+1);
					if (_serialOutput) cout << " Tie: ";
					if (_serialOutput) cout << _ATEM_DskTie[idx];
				}
			} else
			if(strcmp(cmdStr, "KeOn") == 0) {  // Upstream Keyer on
				if (_serialOutput) cout << " KeOn ";
				idx = _packetBuffer[1];
				if (idx >=0 && idx <=3)	{
					_ATEM_KeOn[idx] = _packetBuffer[2] > 0 ? true : false;
					if (_serialOutput) cout << "Upstream Keyer ";
					if (_serialOutput) cout << (idx+1);
					if (_serialOutput) cout << ": ";
					if (_serialOutput) cout << _ATEM_KeOn[idx];
				}
			} else 
			if(strcmp(cmdStr, "ColV") == 0) { 
				if (_serialOutput) cout << " ColV ";
				 // Color Generator Change
					// Todo: Relatively easy: 8 bytes, first is the color generator, the last 6 is hsl words
			} else 
			if(strcmp(cmdStr, "MPCE") == 0) {  // Media Player Clip Enable
				if (_serialOutput) cout << " MPCE ";
					idx = _packetBuffer[0];
					if (idx >=0 && idx <=1)	{
						_ATEM_MPType[idx] = _packetBuffer[1];
						_ATEM_MPStill[idx] = _packetBuffer[2];
						_ATEM_MPClip[idx] = _packetBuffer[3];
					}
			} else 
			if(strcmp(cmdStr, "AuxS") == 0) {  // Aux Output Source
					if (_serialOutput) cout << " AuxS ";
					uint8_t auxInput = _packetBuffer[0];
					if (auxInput >=0 && auxInput <=2)	{
						if (!ver42())	{
							_ATEM_AuxS[auxInput] = _packetBuffer[1];
						} else {
							_ATEM_AuxS[auxInput] = (uint16_t)(_packetBuffer[2]<<8) | _packetBuffer[3];
						}
						if (_serialOutput) cout << "Aux ";
						if (_serialOutput) cout << (auxInput+1);
						if (_serialOutput) cout << " Output: ";
						if (_serialOutput) cout << _ATEM_AuxS[auxInput];
					}

			} else 
			if(strcmp(cmdStr, "_ver") == 0) {  // Firmware version
				if (_serialOutput) cout << "_ver ";
				_ATEM_ver_m = _packetBuffer[1];	// Firmware version, "left of decimal point" (what is that called anyway?)
				_ATEM_ver_l = _packetBuffer[3];	// Firmware version, decimals ("right of decimal point")
			} else 
			if(strcmp(cmdStr, "_pin") == 0) {  // Name
				if (_serialOutput) cout << "_pin ";
				for(uint8_t i=0;i<16;i++)	{
					_ATEM_pin[i] = _packetBuffer[i];
				}
				_ATEM_pin[16] = 0;	// Termination
			} else 
			if(strcmp(cmdStr, "AMTl") == 0) {  // Audio Monitor Tally (on/off settings)
				if (_serialOutput) cout << "AMTl ";
				// Same system as for video: "TlIn"... just implement when time.
			} else 
			// Note for future reveng: For master control, volume at least comes back in "AMMO" (CAMM is the command code.)
			if(strcmp(cmdStr, "AMIP") == 0) {  // Audio Monitor Input P... (state) (On, Off, AFV)
				if (_serialOutput) cout << "AMIP ";
						// 0+1 = Channel (high+low byte):
					uint16_t channelNumber = (uint16_t)(_packetBuffer[0]<<8) | _packetBuffer[1];
			/*	cout << ("CHANNEL: ");
				cout << (channelNumber);
				cout << (" = ");
				cout <<(_packetBuffer[8]);
			*/	
				// 8 = On/Off/AFV
			if (channelNumber>=1 && channelNumber<=16)	{
				_ATEM_AudioChannelMode[channelNumber-1] = _packetBuffer[8];
			} else if (channelNumber==2001) {
				_ATEM_AudioChannelModeSpc[0] = _packetBuffer[8];	// MP1
			} else if (channelNumber==2002) {
				_ATEM_AudioChannelModeSpc[1] = _packetBuffer[8];	// MP2
			} else if (channelNumber==1001) {
				_ATEM_AudioChannelModeSpc[2] = _packetBuffer[8];	// XLR
			} else if (channelNumber==1201) {
				_ATEM_AudioChannelModeSpc[3] = _packetBuffer[8];	// RCA
			}

					// 10+11 = Balance (0xD8F0 - 0x0000 - 0x2710)
					// 8+9 = Volume (0x0020 - 0xFF65)
				

/*				for(uint8_t a=0;a<_cmdLength-8;a++)	{
	            	cout << ((uint8_t)_packetBuffer[a], HEX);
	            	cout << (" ");
				}
				cout <<("");				
*/				
/*				1M/E:
					0: MASTER
					1: (Monitor?)
					2-9: HDMI1 - SDI8
					10: MP1
					11: MP2
					12: EXT
				
				TVS:
					0: MASTER
					1: (Monitor?)
					2-7: INPUT1-6 (HDMI - HDMI - HDMI/SDI - HDMI/SDI - SDI - SDI)
					8: EXT
				*/
		/*		cout << ("Audio Channel: ");
				cout <<(_packetBuffer[0]);	// _packetBuffer[2] seems to be input number (one higher...)
				cout << (" - State: ");
				cout <<(_packetBuffer[3] == 0x01 ? "ON" : (_packetBuffer[3] == 0x02 ? "AFV" : (_packetBuffer[3] > 0 ? "???" : "OFF");
				cout << (" - Volume: ");
				cout << ((uint16_t)_packetBuffer[4]*256+_packetBuffer[5]);
				cout << ("/");
				cout <<((uint16_t)_packetBuffer[6]*256+_packetBuffer[7]);
		   */
		  	} else 
			if(strcmp(cmdStr, "AMLv") == 0) {  // Audio Monitor Levels
				if (_serialOutput) cout << "AMLv ";
				// Get number of channels:
			  	_readToPacketBuffer(4);	// AMLv (AudioMonitorLevels)

				uint8_t numberOfChannels = _packetBuffer[1];
				uint8_t readingOffset=0;
				
			  	_readToPacketBuffer(32);	// AMLv (AudioMonitorLevels)
				if (_ATEM_AMLv_channel<=1)	{	// Master or Monitor vol
					readingOffset= _ATEM_AMLv_channel<<4;
					_ATEM_AMLv[0] = ((uint16_t)(_packetBuffer[readingOffset+1]<<8) | _packetBuffer[readingOffset+2]);	//drops the 8 least sign. bits! -> 15 bit resolution for VU purposes. fine enough.
					readingOffset+=4;
					_ATEM_AMLv[1] = ((uint16_t)(_packetBuffer[readingOffset+1]<<8) | _packetBuffer[readingOffset+2]);	//drops the 8 least sign. bits! -> 15 bit resolution for VU purposes. fine enough.
				} else {
						// Match indexes to input src numbers:
				  	_readToPacketBuffer(numberOfChannels & 1 ? (numberOfChannels+1)<<1 : numberOfChannels<<1);	// The block of input source numbers is always divisible by 4 bytes, so we must read a multiplum of 4 at all times
					for(uint8_t j=0; j<numberOfChannels; j++)	{
//						uint16_t inputNum = ((uint16_t)(_packetBuffer[j<<1]<<8) | _packetBuffer[(j<<1)+1]);
//						cout <<(inputNum);
						/*
							0x07D1 = 2001 = MP1
							0x07D2 = 2002 = MP2
							0x03E9 = 1001 = XLR
							0x04b1 = 1201 = RCA
							*/
					}
						// Get level data for each input:
					for(uint8_t j=0; j<numberOfChannels; j++)	{
						_readToPacketBuffer(16);
						if (_ATEM_AMLv_channel == j+3)	{
							readingOffset = 0;
							_ATEM_AMLv[0] = ((uint16_t)(_packetBuffer[readingOffset+1]<<8) | _packetBuffer[readingOffset+2]);	//drops the 8 least sign. bits! -> 15 bit resolution for VU purposes. fine enough.
							readingOffset+=4;
							_ATEM_AMLv[1] = ((uint16_t)(_packetBuffer[readingOffset+1]<<8) | _packetBuffer[readingOffset+2]);	//drops the 8 least sign. bits! -> 15 bit resolution for VU purposes. fine enough.
						}
					}
					
				}
			} else 
			if(strcmp(cmdStr, "VidM") == 0) {  // Video format (SD, HD, framerate etc.)
				if (_serialOutput) cout << "VidM ";
				_ATEM_VidM = _packetBuffer[0];	
		    } else {
			
		
		
			// SHOULD ONLY THE UNCOMMENTED for development and with a high Baud rate on serial - 115200 for instance. Otherwise it will not connect due to serial writing speeds.
			/*
	            if (_serialOutput) {
					cout << (("???? Unknown token: ";
					cout << (cmdStr);
					cout << (" : ");
				}
				for(uint8_t a=(-2+8);a<_cmdLength-2;a++)	{
	            	if (_serialOutput && (uint8_t)_packetBuffer[a]<16) cout << (0);
	            	if (_serialOutput) cout << ((uint8_t)_packetBuffer[a], HEX);
	            	if (_serialOutput) cout << (" ");
				}
				if (_serialOutput) cout <<("");
	        */
			}
			
			// Empty, if long packet and not read yet:
			while (_readToPacketBuffer())	{}
		
			indexPointer+=_cmdLength;
    	} else { 
      		indexPointer = 2000;
          
			// Flushing the buffer:
			// TODO: Other way? _Udp.flush() ??
	         // while(_Udp.available()) {
			//	  cout << "ParsePaquet 96";
	         //     _Udp.read(_packetBuffer, 96);
	         // }
        }
    }
}

/**
 * Sending a regular answer packet back (tell the switcher that "we heard you, thanks.")
 */
// send remotePacket 0 to ping
//packetType;  //0 PingACK //1 Init //2Retry //3Hello //4 AWSer

void ATEM::sendPing()  {
	 uint8_t packetBufferPing[12];  
	 uint16_t returnPacketLength = 10+2;
	 packetBufferPing[0] = returnPacketLength/256;
 	 packetBufferPing[1] = returnPacketLength%256;
  
 	 packetBufferPing[2] = _sessionID >> 8;  // Session ID
 	 packetBufferPing[3] = _sessionID & 0xFF;  // Session ID
	 packetBufferPing[0] |= 0x08;//B00001000;
	 cout << "-Send Ping-\n";
	

	packetBufferPing[4] = 0;  // Remote Packet ID, MSB
	packetBufferPing[5] = 0;  // Remote Packet ID, L
	packetBufferPing[8] = 0x00;
	packetBufferPing[9] = 0x00;

	packetBufferPing[10] = _localPacketPingIdCounter/256;  // Remote Packet ID, MSB
	packetBufferPing[11] = _localPacketPingIdCounter%256;  // Remote Packet ID, LSB
	_localPacketPingIdCounter++;

	_Udp.sendping(packetBufferPing);  

}

void ATEM::_sendAnswerPacket(uint16_t typePacket)  {

  //Answer packet:
  memset(_packetBuffer, 0, 12);			// Using 12 bytes of answer buffer, setting to zeros.
  
  uint16_t returnPacketLength = 10+2;
  _packetBuffer[0] = returnPacketLength/256;
  _packetBuffer[1] = returnPacketLength%256;
  
  _packetBuffer[2] = _sessionID >> 8;  // Session ID
  _packetBuffer[3] = _sessionID & 0xFF;  // Session ID
  
    // ??? API
  // The rest is zeros.

  // Create header:
  //JLA j'ai rajouté 8 packet à la fin



  
	if(typePacket != 0){
		//ping Client ID = 0
		_packetBuffer[0] |= 0x80;//B10000000;
		cout << "-REP-";
		_packetBuffer[4] = _lastRemotePacketID/256;  // Remote Packet ID, MSB
		_packetBuffer[5] = _lastRemotePacketID%256;  // Remote Packet ID, LSB

		
		
		_packetBuffer[8] = 0x00;  // Remote Packet ID, MSB
		_packetBuffer[9] = 0x00;  // Remote Packet ID, LSB
		_packetBuffer[10] = 0;  // Remote Packet ID, MSB
		_packetBuffer[11] = 0;  // Remote Packet ID, LSB
	}
	else{
		
	   _packetBuffer[0] |= 0x08;//B00001000;
	   cout << "-ACK-";
	   

		_packetBuffer[4] = 0;  // Remote Packet ID, MSB
		_packetBuffer[5] = 0;  // Remote Packet ID, L
		_packetBuffer[8] = 0x00;
		_packetBuffer[9] = 0x00;

		_packetBuffer[10] = _localPacketPingIdCounter/256;  // Remote Packet ID, MSB
		_packetBuffer[11] = _localPacketPingIdCounter%256;  // Remote Packet ID, LSB
		_localPacketPingIdCounter++;

	}

	// Send connectAnswerString to ATEM:
	// Send Reponse AWS
	_Udp.write(_packetBuffer,returnPacketLength);
	_Udp.endPacket();  

	if(typePacket != 0){
		_sendAnswerPacket(0);
	}
	


}

/**
 * Sending a command packet back (ask the switcher to do something)
 */
void ATEM::_sendCommandPacket(const char cmd[4], uint8_t commandBytes[64], uint8_t cmdBytes)  {	// TEMP: 16->64

  if (cmdBytes <= 64)	{	// Currently, only a lenght up to 16 - can be extended, but then the _packetBuffer buffer must be prolonged as well (to more than 36)	<- TEMP 16->64
	  //Answer packet preparations:
	  memset(_packetBuffer, 0, 84);	// <- TEMP 36->84
	  _packetBuffer[2] = _sessionID >> 8;  // Session ID
	  _packetBuffer[3] = _sessionID & 0xFF;  // Session ID
	  _packetBuffer[10] = _lastRemotePacketID/256;  // Remote Packet ID, MSB
	  _packetBuffer[11] = _lastRemotePacketID%256;  // Remote Packet ID, LSB
	  ///if(_ClientPakID > 254) _ClientPakID = 0;
	  _packetBuffer[8]=0;
	  _packetBuffer[9]=0;
      //_packetBuffer[9] = _ClientPakID++;
	  // The rest is zeros.

	  // Command identifier (4 bytes, after header (12 bytes) and local segment length (4 bytes)):
	  int i;
	  for (i=0; i<4; i++)  {
	    _packetBuffer[12+4+i] = cmd[i];
	  }

  		// Command value (after command):
	  for (i=0; i<cmdBytes; i++)  {
	    _packetBuffer[12+4+4+i] = commandBytes[i];
	  }

	  // Command length:
	  _packetBuffer[12] = (4+4+cmdBytes)/256;
	  _packetBuffer[12+1] = (4+4+cmdBytes)%256;

	  // Create header:
	  uint16_t returnPacketLength = 10+2+(4+4+cmdBytes);
	  _packetBuffer[0] = returnPacketLength/256;
	  _packetBuffer[1] = returnPacketLength%256;
	  _packetBuffer[0] |= 0x08;//B00001000;

	  // Send connectAnswerString to ATEM:
	  
	  _Udp.write(_packetBuffer,returnPacketLength);
	  _Udp.endPacket();  

	  
	}
}

/**
 * Sets all zeros in packet buffer:
 */
void ATEM::_wipeCleanPacketBuffer() {
	memset(_packetBuffer, 0, 96);
}

/**
 * Sets all zeros in packet buffer:
 */
void ATEM::_sendPacketBufferCmdData(const char cmd[4], uint8_t cmdBytes)  {
  
  if (cmdBytes <= 96-20)	{
	  //Answer packet preparations:
	  uint8_t _headerBuffer[20];
	  memset(_headerBuffer, 0, 20);
	  _headerBuffer[2] = _sessionID >> 8;  // Session ID
	  _headerBuffer[3] = _sessionID & 0xFF;  // Session ID
	  _headerBuffer[10] = _localPacketPingIdCounter/256;  // Remote Packet ID, MSB
	  _headerBuffer[11] = _localPacketPingIdCounter%256;  // Remote Packet ID, LSB

	  // The rest is zeros.

	  // Command identifier (4 bytes, after header (12 bytes) and local segment length (4 bytes)):
	  int i;
	  for (i=0; i<4; i++)  {
	    _headerBuffer[12+4+i] = cmd[i];
	  }

	  // Command length:
	  _headerBuffer[12] = (4+4+cmdBytes)/256;
	  _headerBuffer[12+1] = (4+4+cmdBytes)%256;

	  // Create header:
	  uint16_t returnPacketLength = 20+cmdBytes;
	  _headerBuffer[0] = returnPacketLength/256;
	  _headerBuffer[1] = returnPacketLength%256;
	  _headerBuffer[0] |= 0x08;//B00001000;

	  //JLA 
	  _headerBuffer[14] = 0x00;
	  _headerBuffer[15] = 0x00;
	  // Send connectAnswerString to ATEM:

	  _Udp.write(_headerBuffer,20);
	  _Udp.write(_packetBuffer,cmdBytes);
	  _Udp.endPacket();  
		_localPacketPingIdCounter++;
	  _localPacketIdCounter++;
	}
}





/********************************
 *
 * General Getter/Setter methods
 *
 ********************************/


/**
 * Setter method: If _serialOutput is set, the library may use cout << () to give away information about its operation - mostly for debugging.
 */
void ATEM::serialOutput(bool serialOutput) {
	_serialOutput = serialOutput;
}

/**
 * Getter method: If true, the initial handshake and "stressful" information exchange has occured and now the switcher connection should be ready for operation. 
 */
bool ATEM::hasInitialized()	{
	return _hasInitialized;
}

/**
 * Returns last Remote Packet ID
 */
uint16_t ATEM::getATEM_lastRemotePacketId()	{
	return _lastRemotePacketID;
}

uint8_t ATEM::getATEMmodel()	{
/*	cout <<(_ATEM_pin);
	cout <<(strcmp(_ATEM_pin, "ATEM Television ") == 0);
	cout <<(strcmp(_ATEM_pin, "ATEM 1 M/E Produ") == 0);
	cout <<(strcmp(_ATEM_pin, "ATEM 2 M/E Produ") == 0);	// Didn't test this yet!
*/
	if (_ATEM_pin[5]=='T')	{
		if (_serialOutput) cout <<"ATEM TeleVision Studio Detected";
		return 0;
	}
	if (_ATEM_pin[5]=='1')	{
		if (_serialOutput) cout <<"ATEM 1 M/E Detected";
		return 1;
	}
	if (_ATEM_pin[5]=='2')	{
		if (_serialOutput) cout <<"ATEM 2 M/E Detected";
		return 2;
	}
	if (_ATEM_pin[5]=='P')	{
		if (_serialOutput) cout <<"ATEM Production Studio 4K";
		return 3;
	}
	return 255;
}







/********************************
 *
 * ATEM Switcher state methods
 * Returns the most recent information we've 
 * got about the switchers state
 *
 ********************************/

uint16_t ATEM::getProgramInput() {
	return _ATEM_PrgI;
}
uint16_t ATEM::getPreviewInput() {
	return _ATEM_PrvI;
}
bool ATEM::getProgramTally(uint8_t inputNumber) {
  	// TODO: Validate that input number exists on current model! <8 at the moment.
	return (_ATEM_TlIn[inputNumber-1] & 1) >0 ? true : false;
}
bool ATEM::getPreviewTally(uint8_t inputNumber) {
  	// TODO: Validate that input number exists on current model! 1-8 at the moment.
	return (_ATEM_TlIn[inputNumber-1] & 2) >0 ? true : false;
}
bool ATEM::getUpstreamKeyerStatus(uint8_t inputNumber) {
	if (inputNumber>=1 && inputNumber<=4)	{
		return _ATEM_KeOn[inputNumber-1];
	}
	return false;
}
bool ATEM::getUpstreamKeyerOnNextTransitionStatus(uint8_t inputNumber) {	// input 0 = background
	if (inputNumber>=0 && inputNumber<=4)	{
			// Notice: the first bit is set for the "background", not valid.
		return (_ATEM_TrSS_KeyersOnNextTransition & (0x01 << inputNumber)) ? true : false;
	}
	return false;
}

bool ATEM::getDownstreamKeyerStatus(uint8_t inputNumber) {
	if (inputNumber>=1 && inputNumber<=2)	{
		return _ATEM_DskOn[inputNumber-1];
	}
	return false;
}
uint16_t ATEM::getTransitionPosition() {
	return _ATEM_TrPs_position;
}
bool ATEM::getTransitionPreview()	{
	return _ATEM_TrPr;
}
uint8_t ATEM::getTransitionType()	{
	return _ATEM_TrSS_TransitionStyle;
}
uint8_t ATEM::getTransitionMixTime() {
	return _ATEM_TMxP_time;		// Transition time for Mix Transitions
}
bool ATEM::getFadeToBlackState() {
	return _ATEM_FtbS_state;    // Active state of Fade-to-black
}
uint8_t ATEM::getFadeToBlackFrameCount() {
	return _ATEM_FtbS_frameCount;    // Returns current frame in the FTB
}
uint8_t ATEM::getFadeToBlackTime() {
	return _ATEM_FtbP_time;		// Transition time for Fade-to-black
}
bool ATEM::getDownstreamKeyTie(uint8_t keyer)	{
	if (keyer>=1 && keyer<=2)	{	// Todo: Should match available keyers depending on model?
		return _ATEM_DskTie[keyer-1];
	}
	return false;
}
uint16_t ATEM::getAuxState(uint8_t auxOutput)  {
  // TODO: Validate that input number exists on current model!
	// On ATEM 1M/E: Black (0), 1 (1), 2 (2), 3 (3), 4 (4), 5 (5), 6 (6), 7 (7), 8 (8), Bars (9), Color1 (10), Color 2 (11), Media 1 (12), Media 1 Key (13), Media 2 (14), Media 2 Key (15), Program (16), Preview (17), Clean1 (18), Clean 2 (19)

	if (auxOutput>=1 && auxOutput<=3)	{	// Todo: Should match available aux outputs
		return _ATEM_AuxS[auxOutput-1];
    }
	return 0;
}	
uint8_t ATEM::getMediaPlayerType(uint8_t mediaPlayer)  {
	if (mediaPlayer>=1 && mediaPlayer<=2)	{	// TODO: Adjust to particular ATEM model... (here 1M/E)
		return _ATEM_MPType[mediaPlayer-1];	// Media Player 1/2: Type (1=Clip, 2=Still)
	}
	return 0;
}
uint8_t ATEM::getMediaPlayerStill(uint8_t mediaPlayer)  {
	if (mediaPlayer>=1 && mediaPlayer<=2)	{	// TODO: Adjust to particular ATEM model... (here 1M/E)
		return _ATEM_MPStill[mediaPlayer-1]+1;	// Still number (if MPType==2)
	}
	return 0;
}
uint8_t ATEM::getMediaPlayerClip(uint8_t mediaPlayer)  {
	if (mediaPlayer>=1 && mediaPlayer<=2)	{	// TODO: Adjust to particular ATEM model... (here 1M/E)
		return _ATEM_MPClip[mediaPlayer-1]+1;	// Clip number (if MPType==1)
	}
	return 0;
}
uint16_t ATEM::getAudioLevels(uint8_t channel)	{
		// channel can be 0 (L) or 1 (R)
	return _ATEM_AMLv[channel];
}
uint8_t ATEM::getAudioChannelMode(uint16_t channelNumber)	{
	/*
	1-16 = inputs
	0x07D1 = 2001 = MP1
	0x07D2 = 2002 = MP2
	0x03E9 = 1001 = XLR
	0x04b1 = 1201 = RCA
	*/

	if (channelNumber>=1 && channelNumber<=16)	{
		return _ATEM_AudioChannelMode[channelNumber-1];
	} else if (channelNumber==2001) {
		return _ATEM_AudioChannelModeSpc[0];	// MP1
	} else if (channelNumber==2002) {
		return _ATEM_AudioChannelModeSpc[1];	// MP2
	} else if (channelNumber==1001) {
		return _ATEM_AudioChannelModeSpc[2];	// XLR
	} else if (channelNumber==1201) {
		return _ATEM_AudioChannelModeSpc[3];	// RCA
	}
	
	return 0;
}


/********************************
 *
 * ATEM Switcher Change methods
 * Asks the switcher to changes something
 *
 ********************************/



void ATEM::changeProgramInput(uint16_t inputNumber)  {
  // TODO: Validate that input number exists on current model!
	// On ATEM 1M/E: Black (0), 1 (1), 2 (2), 3 (3), 4 (4), 5 (5), 6 (6), 7 (7), 8 (8), Bars (9), Color1 (10), Color 2 (11), Media 1 (12), Media 2 (14)

  _wipeCleanPacketBuffer();
  if (!ver42())	{
	  _packetBuffer[1] = inputNumber;
  } else {
	 // Firmaware JLA
	  _packetBuffer[0] = 0;
	  _packetBuffer[1] = 0;
	  _packetBuffer[2] = (inputNumber >> 8);
	  _packetBuffer[3] = (inputNumber & 0xFF);
  }
  _sendPacketBufferCmdData("CPgI", 4);
}
void ATEM::changePreviewInput(uint16_t inputNumber)  {
  // TODO: Validate that input number exists on current model!

  _wipeCleanPacketBuffer();
  if (!ver42())	{
	  _packetBuffer[1] = inputNumber;
  } else {
	  _packetBuffer[2] = (inputNumber >> 8);
	  _packetBuffer[3] = (inputNumber & 0xFF);
  }
  _sendPacketBufferCmdData("CPvI", 4);
}
void ATEM::doCut()	{
  _wipeCleanPacketBuffer();
  _packetBuffer[1] = 0xef;
  _packetBuffer[2] = 0xbf;
  _packetBuffer[3] = 0x5f;
  _sendPacketBufferCmdData("DCut", 4);
}
void ATEM::doAuto()	{
	doAuto(0);
}
void ATEM::doAuto(uint8_t me)	{
  _wipeCleanPacketBuffer();
  _packetBuffer[0] = me;
  _sendPacketBufferCmdData("DAut", 4);
}
void ATEM::fadeToBlackActivate()	{
  _wipeCleanPacketBuffer();
  _packetBuffer[1] = 0x02;
  _packetBuffer[2] = 0x58;
  _packetBuffer[3] = 0x99;
  _sendPacketBufferCmdData("FtbA", 4);	// Reflected back from ATEM in "FtbS"
}
void ATEM::changeTransitionPosition(uint16_t value)	{
	if (value>0 && value<=1000)	{
		uint8_t commandBytes[4] = {0, 0xe4, (value*10)/256, (value*10)%256};
		_sendCommandPacket("CTPs", commandBytes, 4);  // Change Transition Position (CTPs)
	}
}
void ATEM::changeTransitionPositionDone()	{	// When the last value of the transition is sent (1000), send this one too (we are done, change tally lights and preview bus!)
	uint8_t commandBytes[4] = {0, 0xf6, 0, 0};  	// Done
	_sendCommandPacket("CTPs", commandBytes, 4);  // Change Transition Position (CTPs)
}
void ATEM::changeTransitionPreview(bool state)	{
	uint8_t commandBytes[4] = {0x00, state ? 0x01 : 0x00, 0x00, 0x00};
	_sendCommandPacket("CTPr", commandBytes, 4);	// Reflected back from ATEM in "TrPr"
}
void ATEM::changeTransitionType(uint8_t type)	{
	if (type>=0 && type<=4)	{	// 0=MIX, 1=DIP, 2=WIPE, 3=DVE, 4=STING
		uint8_t commandBytes[4] = {0x01, 0x00, type, 0x02};
		_sendCommandPacket("CTTp", commandBytes, 4);	// Reflected back from ATEM in "TrSS"
	}
}
void ATEM::changeTransitionMixTime(uint8_t frames)	{
	if (frames>=1 && frames<=0xFA)	{
		uint8_t commandBytes[4] = {0x00, frames, 0x00, 0x00};
		_sendCommandPacket("CTMx", commandBytes, 4);	// Reflected back from ATEM in "TMxP"
	}
}
void ATEM::changeFadeToBlackTime(uint8_t frames)	{
	if (frames>=1 && frames<=0xFA)	{
		uint8_t commandBytes[4] = {0x01, 0x00, frames, 0x02};
		_sendCommandPacket("FtbC", commandBytes, 4);	// Reflected back from ATEM in "FtbP"
	}
}
void ATEM::changeUpstreamKeyOn(uint8_t keyer, bool state)	{
	if (keyer>=1 && keyer<=4)	{	// Todo: Should match available keyers depending on model?
	  _wipeCleanPacketBuffer();
	  _packetBuffer[1] = keyer-1;
	  _packetBuffer[2] = state ? 0x01 : 0x00;
	  _packetBuffer[3] = 0x90;
	  _sendPacketBufferCmdData("CKOn", 4);	// Reflected back from ATEM in "KeOn"
	}
}
void ATEM::changeUpstreamKeyNextTransition(uint8_t keyer, bool state)	{	// Supporting "Background" by "0"
	if (keyer>=0 && keyer<=4)	{	// Todo: Should match available keyers depending on model?
		uint8_t stateValue = _ATEM_TrSS_KeyersOnNextTransition;
		if (state)	{
			stateValue = stateValue | (0x1 << keyer);
		} else {
			stateValue = stateValue & (~(0x1 << keyer));
		}
				// TODO: Requires internal storage of state here so we can preserve all other states when changing the one we want to change.
					// Below: Byte 2 is which ME (1 or 2):
		uint8_t commandBytes[4] = {0x02, 0x00, 0x6a, stateValue & 0x1F };//B11111
		_sendCommandPacket("CTTp", commandBytes, 4);	// Reflected back from ATEM in "TrSS"
	}
}
void ATEM::changeDownstreamKeyOn(uint8_t keyer, bool state)	{
	if (keyer>=1 && keyer<=2)	{	// Todo: Should match available keyers depending on model?
		
		uint8_t commandBytes[4] = {keyer-1, state ? 0x01 : 0x00, 0xff, 0xff};
		_sendCommandPacket("CDsL", commandBytes, 4);	// Reflected back from ATEM in "DskP" and "DskS"
	}
}
void ATEM::changeDownstreamKeyTie(uint8_t keyer, bool state)	{
	if (keyer>=1 && keyer<=2)	{	// Todo: Should match available keyers depending on model?
		uint8_t commandBytes[4] = {keyer-1, state ? 0x01 : 0x00, 0xff, 0xff};
		_sendCommandPacket("CDsT", commandBytes, 4);
	}
}
void ATEM::doAutoDownstreamKeyer(uint8_t keyer)	{
	if (keyer>=1 && keyer<=2)	{	// Todo: Should match available keyers depending on model?
  		uint8_t commandBytes[4] = {keyer-1, 0x32, 0x16, 0x02};	// I don't know what that actually means...
  		_sendCommandPacket("DDsA", commandBytes, 4);
	}
}
void ATEM::changeAuxState(uint8_t auxOutput, uint16_t inputNumber)  {
  // TODO: Validate that input number exists on current model!
	// On ATEM 1M/E: Black (0), 1 (1), 2 (2), 3 (3), 4 (4), 5 (5), 6 (6), 7 (7), 8 (8), Bars (9), Color1 (10), Color 2 (11), Media 1 (12), Media 1 Key (13), Media 2 (14), Media 2 Key (15), Program (16), Preview (17), Clean1 (18), Clean 2 (19)

	if (auxOutput>=1 && auxOutput<=6)	{	// Todo: Should match available aux outputs
		if (!ver42())	{
	  		uint8_t commandBytes[4] = {auxOutput-1, inputNumber, 0, 0};
	  		_sendCommandPacket("CAuS", commandBytes, 4);
		} else {
	  		uint8_t commandBytes[8] = {0x01, auxOutput-1, inputNumber >> 8, inputNumber & 0xFF, 0,0,0,0};
	  		_sendCommandPacket("CAuS", commandBytes, 8);
		}
		//cout << ("freeMemory()=");
		//cout <<(freeMemory());
    }
}
void ATEM::settingsMemorySave()	{
	uint8_t commandBytes[4] = {0, 0, 0, 0};
	_sendCommandPacket("SRsv", commandBytes, 4);
}
void ATEM::settingsMemoryClear()	{
	uint8_t commandBytes[4] = {0, 0, 0, 0};
	_sendCommandPacket("SRcl", commandBytes, 4);
}
void ATEM::changeColorValue(uint8_t colorGenerator, uint16_t hue, uint16_t saturation, uint16_t lightness)  {
	if (colorGenerator>=1 && colorGenerator<=2
			&& hue>=0 && hue<=3600 
			&& saturation >=0 && saturation <=1000 
			&& lightness >=0 && lightness <= 1000
		)	{	// Todo: Should match available aux outputs
  		uint8_t commandBytes[8] = {0x07, colorGenerator-1, 
			highByte(hue), lowByte(hue),
			highByte(saturation), lowByte(saturation),
			highByte(lightness), lowByte(lightness)
							};
  		_sendCommandPacket("CClV", commandBytes, 8);
    }
}
void ATEM::mediaPlayerSelectSource(uint8_t mediaPlayer, bool movieclip, uint8_t sourceIndex)  {
	if (mediaPlayer>=1 && mediaPlayer<=2)	{	// TODO: Adjust to particular ATEM model... (here 1M/E)
		uint8_t commandBytes[8];
		memset(commandBytes, 0, 8);
  		commandBytes[1] = mediaPlayer-1;
		if (movieclip)	{
			commandBytes[0] = 4;
			if (sourceIndex>=1 && sourceIndex<=2)	{
				commandBytes[4] = sourceIndex-1;
			}
		} else {
			commandBytes[0] = 0x03;
			if (sourceIndex>=1 && sourceIndex<=32)	{
				commandBytes[3] = sourceIndex-1;
			}
		}
		commandBytes[9] = 0x10;
		_sendCommandPacket("MPSS", commandBytes, 8);
			
			// For some reason you have to send this command immediate after (or in fact it could be in the same packet)
			// If not done, the clip will not change if there is a shift from stills to clips or vice versa.
		uint8_t commandBytes2[8] = {0x01, mediaPlayer-1, movieclip?2:1, 0xbf, movieclip?0x96:0xd5, 0xb6, 0x04, 0};
		_sendCommandPacket("MPSS", commandBytes2, 8);
	}
}

void ATEM::mediaPlayerClipStart(uint8_t mediaPlayer)  {
	if (mediaPlayer>=1 && mediaPlayer<=2)	{
		uint8_t commandBytes2[8] = {0x01, mediaPlayer-1, 0x01, 0xbf, 0x21, 0xa9, 0x94, 0xfa}; // 3rd byte is "start", remaining 5 bytes seems random...
		_sendCommandPacket("SCPS", commandBytes2, 8);
	}
}


void ATEM::changeSwitcherVideoFormat(uint8_t format)	{
	// Changing the video format it uses: 525i59.94 NTSC (0), 625i50 PAL (1), 720p50 (2), 720p59.94 (3), 1080i50 (4), 1080i59.94 (5)
	if (format>=0 && format<=5)	{	// Todo: Should match available aux outputs
  		uint8_t commandBytes[4] = {format, 0xeb, 0xff, 0xbf};
  		_sendCommandPacket("CVdM", commandBytes, 4);
    }	
}



void ATEM::changeDVESettingsTemp(unsigned long Xpos,unsigned long Ypos,unsigned long Xsize,unsigned long Ysize)	{	// TEMP
														//B1111
		  uint8_t commandBytes[64] = {0x00, 0x00, 0x00, 0xF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, highByte(Xsize), lowByte(Xsize), 0x00, 0x00, highByte(Ysize), lowByte(Ysize), (Xpos >>24) & 0xFF, (Xpos >>16) & 0xFF, (Xpos >>8) & 0xFF, (Xpos >>0) & 0xFF, (Ypos >>24) & 0xFF, (Ypos >>16) & 0xFF, (Ypos >>8) & 0xFF, (Ypos >>0) & 0xFF, 0xbf, 0xff, 0xdb, 0x7f, 0xc2, 0xa2, 0x09, 0x90, 0xdb, 0x7e, 0xbf, 0xff, 0x82, 0x34, 0x2e, 0x0b, 0x05, 0x00, 0x00, 0x00, 0x34, 0xc1, 0x00, 0x2c, 0xe2, 0x00, 0x4e, 0x02, 0xa3, 0x98, 0xac, 0x02, 0xdb, 0xd9, 0xbf, 0xff, 0x74, 0x34, 0xe9, 0x01};
  		_sendCommandPacket("CKDV", commandBytes, 64);
}
void ATEM::changeDVEMaskTemp(unsigned long top,unsigned long bottom,unsigned long left,unsigned long right)	{	// TEMP
  		uint8_t commandBytes[64] = {0x03, 0xc0 | 0x20, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01, highByte(top), lowByte(top), highByte(bottom), lowByte(bottom), highByte(left), lowByte(left), highByte(right), lowByte(right),0,0,0,0};
  		_sendCommandPacket("CKDV", commandBytes, 64);
}
void ATEM::changeDVEBorder(bool enableBorder)	{	// TEMP
  		uint8_t commandBytes[64] = {0x00, 0x00, 0x00, 0x20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, enableBorder?1:0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0};
  		_sendCommandPacket("CKDV", commandBytes, 64);
}

void ATEM::changeDVESettingsTemp_Rate(uint8_t rateFrames)	{	// TEMP
  		uint8_t commandBytes[64] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xdb, 0x7f, 0xc2, 0xa2, 0x09, 0x90, 0xdb, 0x7e, 0xbf, 0xff, 0x82, 0x34, 0x2e, 0x0b, 0x05, 0x00, 0x00, 0x00, 0x34, 0xc1, 0x00, 0x2c, 0xe2, 0x00, 0x4e, 0x02, 0xa3, 0x98, 0xac, 0x02, 0xdb, 0xd9, 0xbf, 0xff, rateFrames, 0x34, 0xe9, 0x01};
  		_sendCommandPacket("CKDV", commandBytes, 64);
}
void ATEM::changeDVESettingsTemp_RunKeyFrame(uint8_t runType)	{	// runType: 1=A, 2=B, 3=Full, 4=on of the others (with an extra paramter:)
  		uint8_t commandBytes[8] = {0x02, 0x00, 0x00, 0x02, 0x00, runType, 0xff, 0xff};
  		_sendCommandPacket("RFlK", commandBytes, 8);
}
void ATEM::changeKeyerMask(uint16_t topMask, uint16_t bottomMask, uint16_t leftMask, uint16_t rightMask) {
	changeKeyerMask(0, topMask, bottomMask, leftMask, rightMask);
}
void ATEM::changeKeyerMask(uint8_t keyer, uint16_t topMask, uint16_t bottomMask, uint16_t leftMask, uint16_t rightMask)	{
		// In "B11110", bits are (from right to left): 0=?, 1=topMask, 2=bottomMask, 3=leftMask, 4=rightMask
  		uint8_t commandBytes[12] = {0x1E, 0x00, keyer-1, 0x00, highByte(topMask), lowByte(topMask), highByte(bottomMask), lowByte(bottomMask), highByte(leftMask), lowByte(leftMask), highByte(rightMask), lowByte(rightMask)};
  		_sendCommandPacket("CKMs", commandBytes, 12);
}

void ATEM::changeDownstreamKeyMask(uint8_t keyer, uint16_t topMask, uint16_t bottomMask, uint16_t leftMask, uint16_t rightMask)	{
		// In "B11110", bits are (from right to left): 0=?, 1=topMask, 2=bottomMask, 3=leftMask, 4=rightMask
		if (keyer>=1 && keyer<=2)	{
  			uint8_t commandBytes[12] = {0x1E, keyer-1, 0x00, 0x00, highByte(topMask), lowByte(topMask), highByte(bottomMask), lowByte(bottomMask), highByte(leftMask), lowByte(leftMask), highByte(rightMask), lowByte(rightMask)};
  			_sendCommandPacket("CDsM", commandBytes, 12);
		}
}



void ATEM::changeUpstreamKeyFillSource(uint8_t keyer, uint16_t inputNumber)	{
	if (keyer>=1 && keyer<=4)	{	// Todo: Should match available keyers depending on model?
	  	// TODO: Validate that input number exists on current model!
		// 0-15 on 1M/E
		if (!ver42())	{
			uint8_t commandBytes[4] = {0, keyer-1, inputNumber, 0};
			_sendCommandPacket("CKeF", commandBytes, 4);
		} else {
			uint8_t commandBytes[4] = {0, keyer-1, highByte(inputNumber), lowByte(inputNumber)};
			_sendCommandPacket("CKeF", commandBytes, 4);
		}
	}
}

// TODO: ONLY clip works right now! there is a bug...
void ATEM::changeUpstreamKeyBlending(uint8_t keyer, bool preMultipliedAlpha, uint16_t clip, uint16_t gain, bool invKey)	{
	if (keyer>=1 && keyer<=4)	{	// Todo: Should match available keyers depending on model?
		uint8_t commandBytes[12] = {0x02, keyer-1, 0, preMultipliedAlpha?1:0, highByte(clip), lowByte(clip), highByte(gain), lowByte(gain), invKey?1:0, 0, 0, 0};
		_sendCommandPacket("CKLm", commandBytes, 12);
	}
}

// TODO: ONLY clip works right now! there is a bug...
void ATEM::changeDownstreamKeyBlending(uint8_t keyer, bool preMultipliedAlpha, uint16_t clip, uint16_t gain, bool invKey)	{
	if (keyer>=1 && keyer<=4)	{	// Todo: Should match available keyers depending on model?
		uint8_t commandBytes[12] = {0x02, keyer-1, preMultipliedAlpha?1:0, 0, highByte(clip), lowByte(clip), highByte(gain), lowByte(gain), invKey?1:0, 0, 0, 0};
		_sendCommandPacket("CDsG", commandBytes, 12);
	}
}

// Statuskode retur: DskB, data byte 2 derefter er fill source, data byte 3 er key source, data byte 1 er keyer 1-2 (0-1)
// Key source command er : CDsC - og ellers ens med...
void ATEM::changeDownstreamKeyFillSource(uint8_t keyer, uint16_t inputNumber)	{
	if (keyer>=1 && keyer<=2)	{	// Todo: Should match available keyers depending on model?
	  	// TODO: Validate that input number exists on current model!
		// 0-15 on 1M/E
		if (!ver42())	{
			uint8_t commandBytes[4] = {keyer-1, inputNumber, 0, 0};
			_sendCommandPacket("CDsF", commandBytes, 4);
		} else {
			uint8_t commandBytes[4] = {keyer-1, 0, highByte(inputNumber), lowByte(inputNumber)};
			_sendCommandPacket("CDsF", commandBytes, 4);
		}
	}
}

void ATEM::changeDownstreamKeyKeySource(uint8_t keyer, uint16_t inputNumber)	{
	if (keyer>=1 && keyer<=2)	{	// Todo: Should match available keyers depending on model?
	  	// TODO: Validate that input number exists on current model!
		// 0-15 on 1M/E
		if (!ver42())	{
			uint8_t commandBytes[4] = {keyer-1, inputNumber, 0, 0};
			_sendCommandPacket("CDsC", commandBytes, 4);
		} else {
			uint8_t commandBytes[4] = {keyer-1, 0, highByte(inputNumber), lowByte(inputNumber)};
			_sendCommandPacket("CDsC", commandBytes, 4);
		}
	}
}

void ATEM::changeAudioChannelMode(uint16_t channelNumber, uint8_t mode)	{	// Mode: 0=Off, 1=On, 2=AFV
  if (mode<=2)	{
	  _wipeCleanPacketBuffer();
		if (!ver42())	{
		  _packetBuffer[0] = 0x01;	// Setting ON/OFF/AFV
		  _packetBuffer[1] = channelNumber;	// Input 1-8 = channel 0-7(!), Media Player 1+2 = channel 8-9, Ext = channel 10 (For 1M/E!)
		  _packetBuffer[2] = mode;	// 0=Off, 1=On, 2=AFV
		  _packetBuffer[3] = 0x03;	
		  _sendPacketBufferCmdData("CAMI", 12);	// Reflected back from ATEM as "AMIP"
		} else {
		  _packetBuffer[0] = 0x01;	// Setting ON/OFF/AFV
		  _packetBuffer[2] = highByte(channelNumber);
		  _packetBuffer[3] = lowByte(channelNumber);
		  _packetBuffer[4] = mode;	// 0=Off, 1=On, 2=AFV
		  _sendPacketBufferCmdData("CAMI", 12);	// Reflected back from ATEM as "AMIP"
		}
  }
}
void ATEM::changeAudioChannelVolume(uint16_t channelNumber, uint16_t volume)	{

	/*
	Based on data from the ATEM switcher, this is an approximation to the integer value vs. the dB value:
	dB	+60 added	Number from protocol		Interpolated
	6	66	65381		65381
	3	63	46286		46301,04
	0	60	32768		32789,13
	-3	57	23198		23220,37
	-6	54	16423		16444,03
	-9	51	11627		11645,22
	-20	40	3377		3285,93
	-30	30	1036		1040,21
	-40	20	328		329,3
	-50	10	104		104,24
	-60	0	33		33

	for (int i=-60; i<=6; i=i+3)  {
	   cout << (i);
	   cout << (" dB = ");
	   cout << (33*pow(1.121898585, i+60));
	   cout <<();
	}


	*/

// CAMI command structure:  	CAMI    [01=buttons, 02=vol, 04=pan (toggle bits)] - [input number, 0-…] - [buttons] - [buttons] - [vol] - [vol] - [pan] - [pan]
// CAMM: 01:de:80:00:e4:10:ff:bf (master) [volume is 80:00]

  _wipeCleanPacketBuffer();

	if (!ver42())	{
	  _packetBuffer[0] = 0x02;	// Setting Volume Level
	  _packetBuffer[1] = channelNumber;	// Input 1-8 = channel 0-7(!), Media Player 1+2 = channel 8-9, Ext = channel 10 (For 1M/E!)		///		Input 1-6 = channel 0-5(!), Ext = channel 6 (For TVS!)
		if (volume > 0xff65)	{
			volume = 0xff65;
		}
	  _packetBuffer[4] = volume/256;	
	  _packetBuffer[5] = volume%256;	

	  _sendPacketBufferCmdData("CAMI", 8);
	} else {
	  _packetBuffer[0] = 0x02;	// Setting Volume Level

	  _packetBuffer[2] = highByte(channelNumber);
	  _packetBuffer[3] = lowByte(channelNumber);

		if (volume > 0xff65)	{
			volume = 0xff65;
		}
	  _packetBuffer[6] = highByte(volume);	
	  _packetBuffer[7] = lowByte(volume);	

	  _sendPacketBufferCmdData("CAMI", 12);
		
	}
}

void ATEM::changeAudioMasterVolume(uint16_t volume)	{

// CAMI command structure:  	CAMI    [01=but, 02=vol, 04=pan (toggle bits)] - [input number, 0-…] - [buttons] - [buttons] - [vol] - [vol] - [pan] - [pan]
// CAMM: 01:de:80:00:e4:10:ff:bf (master) [volume is 80:00]

  _wipeCleanPacketBuffer();

  _packetBuffer[0] = 0x01;

	if (volume > 0xff65)	{
		volume = 0xff65;
	}

  _packetBuffer[2] = volume/256;	
  _packetBuffer[3] = volume%256;	

  _sendPacketBufferCmdData("CAMM", 8);
}
void ATEM::sendAudioLevelNumbers(bool enable)	{
  _wipeCleanPacketBuffer();
  _packetBuffer[0] = enable ? 1 : 0;
  _sendPacketBufferCmdData("SALN", 4);
}
void ATEM::setAudioLevelReadoutChannel(uint8_t AMLv)	{
				/*
				Channels on an 1M/E: (16 byte segments:)
				0: MASTER
				1: (Monitor?)
				2-9: HDMI1 - SDI8
				10: MP1
				11: MP2
				12: EXT
				
Values:
FCP			HyperDeck	ATEM			Value
Output		Studio 		Input			in
Level:		Playback:	Colors:			Protocol:

0 			red			red				32767
-3 			red			red				23228
-6 			red/yellow	red				16444
-9 			yellow		red/yellow		11640
-12 		yellow		yellow			8240
-18 		green		yellow/green	4130
-24			green		green			2070
-42			green		green			260				


(Values = 32809,85 * 1,12^dB (trendline based on numbers above))
(HyperDeck Studio: red=>yellow @ -6db, yellow=>green @ -15db (assumed))
(ATEM: red=>yellow @ -9db, yellow=>green @ -20db)
(ATEM Input registered the exact same level values, FCP had been writing to the ProRes 422 file.)				
				
				*/
	_ATEM_AMLv_channel = AMLv;	// Should check that it's in range 0-12
}

bool ATEM::ver42()	{
	//cout <<(_ATEM_ver_m);
	//cout <<(_ATEM_ver_l);
	
	// ATEM Control Panel software v. 4.2 = firmware version 2.12
	
	//cout <<((_ATEM_ver_m>2) || (_ATEM_ver_m>=2 && _ATEM_ver_l>=12));
	
	//return (_ATEM_ver_m>2) || (_ATEM_ver_m>=2 && _ATEM_ver_l>=12);
	//JLA
	return true;
}





void ATEM::setWipeReverseDirection(bool reverse) {

  _wipeCleanPacketBuffer();

  _packetBuffer[0] = 0x01;	
  _packetBuffer[18] = reverse;	

  _sendPacketBufferCmdData("CTWp", 20);
}

