#include "EthernetUDP.h"

using namespace std;
//dans ClassDatagramme j'implemente les méthodes
//pour savoir à quelle class les méthodes appartiennent il faut mettre
//devant chaque méthode Nomdelaclass::

int recvAvailable(SOCKET sock){

    return 0;
}

int EthernetUDP::getNumDatagramme() const {
        
        
        return _numDatagramme;
}

//constructeur
EthernetUDP::EthernetUDP(){ 
    
   
}

//destructeur
EthernetUDP::~EthernetUDP(){ 
    close(_sock);
    
}

void EthernetUDP::begin(SOCKADDR_IN switcherIP){
    //silence is golden
    _switcherIP = switcherIP;
    _IPProgramLicen.sin_addr.s_addr = htonl(INADDR_ANY);  
    _IPProgramLicen.sin_family = AF_INET;
    _IPProgramLicen.sin_port = htons(PORT2);
    printf("Listage du port %d...\n", PORT2);

    _numDatagramme = _sock = socket(AF_INET, SOCK_DGRAM, 0);

    fprintf(stderr, "sock() message error : %s\n", strerror(errno));  
    _error_message = ::bind(_sock, (SOCKADDR*)&_IPProgramLicen, sizeof(_IPProgramLicen));
     fprintf(stderr, "Bind() message error : %s\n", strerror(errno));  
       
}
void EthernetUDP::endsock(){
     //silence is golden
     close(_sock);
}

    
void EthernetUDP::beginPacket(SOCKADDR_IN _switcherIP,  int size){
     //silence is golden
     int a = 0;
}
void EthernetUDP::write(uint8_t * buffer,int size){
     //silence is golden
     
    size_t j=0;
    for(size_t i = _Size_SavepacketSend; i < (_Size_SavepacketSend + size ); i++)
    {
        _SavepacketSend[i]=buffer[j];
        j++;
    }
    
        
    _Size_SavepacketSend +=size;



}
void EthernetUDP::endPacket(){
     //silence is golden
     //usleep(10000);
    _error_message = sendto(_sock,_SavepacketSend,_Size_SavepacketSend,0, (SOCKADDR*)&_switcherIP,sizeof(_switcherIP));
    
    cout << "\033[1;32m";
            for(size_t i = 0; i < _error_message; i++)
            {
                printf("|%02x", _SavepacketSend[i]);
                
            }
            cout << "\033[0m\n\n"; 
    if (_error_message == 0){
        fprintf(stderr, "socket() Envoi message error : %s\n", strerror(errno));  
    }

    _Size_SavepacketSend = 0;
        
}

//Permet de compter le nombre d'octet du message
uint16_t EthernetUDP::WaitePacket(){
    clear_remaining();
    _error_message = 0;
    if(_sock>0){
        socklen_t fromlen = sizeof(_Udpfrom);
        _error_message = recvfrom(_sock, _Savepacket, _remaining, 0, reinterpret_cast<sockaddr*>(&_Udpfrom), &fromlen);
        if (_error_message == 0)
        {
            fprintf(stderr, "socket() Envoi message error : %s\n", strerror(errno));  
      
        }
    }
    return _error_message;

}


uint16_t EthernetUDP::parsePacket(){
    
    
    clear_remaining();
    
    
    
   
    _remaining = _SizepacketBuff;
    if(_remaining > 0){
        socklen_t fromlen = sizeof(_Udpfrom);
         //usleep(20000);
        _error_message = recvfrom(_sock, _Savepacket, _remaining, 0, reinterpret_cast<sockaddr*>(&_Udpfrom), &fromlen);
        if (_error_message > 0)
        {
            _remaining = _error_message;
      
        }else{
            fprintf(stderr, "socket() Envoi message error : %s\n", strerror(errno));  
        }
    }
    
    return _remaining;
  
  // There aren't any packets available

}
bool EthernetUDP::available(){
     //silence is golden
   
    
    for(size_t i = 0; i < 10; i++)
    {
        
        ioctl (_sock,FIONREAD,&_SizepacketBuff);
        if  (_SizepacketBuff > 0)
        {
            return true;
        }else{
            usleep(40000);
        }
    }
    cout << "No Packet in buffer\n";
    return false;
    
    
}
void EthernetUDP::read(uint8_t *_packetBuffer  ,int zise){
    
   

    if (_remaining > 0){

      
       size_t i=0;
        size_t k=_KeyRead_Savepacket;
       size_t imax= _KeyRead_Savepacket + _remaining;
       size_t imax2= _KeyRead_Savepacket + zise;
       size_t j=0;
        if (_remaining <= zise) {
            // data should fit in the buffer
            

            //read is put the savepacket in the packetBuffer between _KeyRead_Savepacket and _KeyRead_Savepacket + zise
            
            for( i = _KeyRead_Savepacket; i < (imax); i++)
            {
                _packetBuffer[j]=_Savepacket[i];
                _remaining --;
                j++;
            }
             _KeyRead_Savepacket = i;
         
        }
        else{
           //read is put the savepacket in the packetBuffer between _KeyRead_Savepacket and _KeyRead_Savepacket + zise
            for( i = _KeyRead_Savepacket; i < (imax2); i++)
            {
                _packetBuffer[j]=_Savepacket[i];
                _remaining --;
                j++;
            }
            _KeyRead_Savepacket = i;
           
        }

        //ON AFFICHE QUE LES ENTETES BLACKMAGIC ou les init
        if(k==0){
       /*  if (zise>20){
                //cyan
                cout << "\033[1;36m";
            }
            //coucou
            if (zise<12){
                //red
                cout << "\033[1;31m";
            }
            if (zise==12){
                //green
                cout << "\033[1;32m";
            }
            if (zise==20){
                //yellow
                cout << "\033[1;33m";
            }
            if (zise==8){
                //blue
                cout << "\033[1;34m";
            }
            if (zise==32){
                //blue
                cout << "\033[1;35m";
            }
            if (zise==32){
                //blue
                cout << "\033[1;37m";
            } */
            
            
            for(size_t i = 0; i < 12; i++)
            {
                printf("|%02x", _packetBuffer[i]);
                
            }
            
            //cout << "\033[0m\n";  
             //cout << "\n";  
        }  
       
        

        
    }
    else
    {
        _KeyRead_Savepacket = 0;
    }

    // If we get here, there's no data available or recv failed
   
   
    
}


void EthernetUDP::clear_remaining()
{
  // could this fail (loop endlessly) if _remaining > 0 and recv in read fails?
  // should only occur if recv fails after telling us the data is there, lets
  // hope the w5100 always behaves :)
  
      //cout << "clear_remaining purge " << _remaining << " packet\n";
      _KeyRead_Savepacket = 0;
    _remaining = 0;
    
  
}
