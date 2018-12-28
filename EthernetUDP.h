//dans les.h, il est recommandé de ne jamais mettre la directiveusing namespace
//Include pour les sockets
#ifndef EthernetUDP_h
#define EthernetUDP_h

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <iostream>

typedef struct sockaddr_in SOCKADDR_IN;
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define PORT2 9910

class EthernetUDP
{
    //les membres de ma classs
    private :
    int _numDatagramme,_error_message;
    SOCKET _sock;
    SOCKADDR_IN _switcherIP;
    sockaddr_in _Udpfrom;
    sockaddr_in _IPProgramLicen;
    uint16_t _remaining=0; // remaining bytes of incoming packet yet to be processed
    uint16_t _offset=0; // offset into the packet being sent

    //les méthodes public de ma classs
    public :

    //On utilise le mot-clé const sur des méthodes qui ne  modifie pas les membres de l'objet
    //çà optimise votre programme
    int getNumDatagramme() const;

    //constructeur
    EthernetUDP();
    ~EthernetUDP();

    void begin(SOCKADDR_IN switcherIP );
    void endsock();
    void beginPacket(SOCKADDR_IN _switcherIP,  int size);
	void write(uint8_t * buffer,int zise);
	void endPacket();   
    uint16_t parsePacket();
    uint16_t WaitePacket();
    uint8_t _Savepacket[1500]; 
    uint8_t _SavepacketSend[1500]; 
    uint8_t _SizepacketBuff=0; 
    int _KeyRead_Savepacket = 0;
    int _Size_SavepacketSend = 0;
    bool available();
    bool writeavailable();
    

    void read(uint8_t *_packetBuffer  ,int zise);
    //send atem ping protocol
    void sendping(uint8_t * packetSend);
   
    void clear_remaining();

};

#endif