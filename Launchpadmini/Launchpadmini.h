//dans les.h, il est recommandé de ne jamais mettre la directiveusing namespace
#ifndef SERIAL
#define SERIAL

#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <thread>
#include "../ATEM.h"
 // POSIX terminal control definitions
//#include <asm/termbits.h>


class Launchpadmini
{
    //les membres de ma classs
    private :
    unsigned char  _dataLaunchPad[3];
    ATEM *_AtemSwitcher;
    std::thread _threadRefreshLaunchpad; 
    bool _threadOn = false;
    //les méthodes public de ma classs
    
    public:

    int handle;
    std::string  deviceName;
    int baud;

    Launchpadmini();
    Launchpadmini(std::string deviceName, int baud);
    ~Launchpadmini();

    bool Send( unsigned char  * data,int len);
    bool SendButtonPushRed( unsigned int NumButton);
    bool SendButtonPushLowRed( unsigned int NumButton);
    bool SendButtonPushLowGreen( unsigned int NumButton);
    bool SendButtonPushLowYellow( unsigned int NumButton);
    bool SendButtonOff( unsigned int NumButton);
    bool SendResetLaunchpad();
    
    int Receive( unsigned char  * data, int len);
    bool IsOpen(void);
    void Close(void);
    bool Open(std::string deviceName, int baud);
   
   void MappingWithAtemBlackMagic();

   void LaunchMappingThread(ATEM *AtemSwitcher);
   void RefreshMappingWithAtemBlackMagic();
   

};

unsigned int smallwordbutton(uint8_t value15_8,uint8_t value0_7);

#endif