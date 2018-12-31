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
 // POSIX terminal control definitions
#include <asm/termbits.h>


class Launchpadmini
{
    //les membres de ma classs
    private :
    int _numDatagramme;

    //les méthodes public de ma classs
    
    public:

    int handle;
    std::string  deviceName;
    int baud;

    Launchpadmini();
    Launchpadmini(std::string deviceName, int baud);
    ~Launchpadmini();

    bool Send( unsigned char  * data,int len);
    int Receive( unsigned char  * data, int len);
    bool IsOpen(void);
    void Close(void);
    bool Open(std::string deviceName, int baud);
    bool NumberByteRcv(int &bytelen);

};

/* #define NCCS 19
#define CS8	0000060
#define CLOCAL	0004000
#define CREAD	0000200
#define VTIME 5
#define VMIN 6
#define CBAUD	0010017
#define BOTHER 0010000
#define	TCIOFLUSH	2
#define TCSETS		0x5402
#define TCFLSH		0x540B





#define TCGETS2		_IOR('T', 0x2A, struct termios2)
#define TCSETS2		_IOW('T', 0x2B, struct termios2) */



#endif