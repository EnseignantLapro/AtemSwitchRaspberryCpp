#include "Launchpadmini.h"



using namespace std;


Launchpadmini::Launchpadmini()
{
  
   handle=-1;
   if(!Open("/dev/snd/midiC1D0",115200)) fprintf(stderr, "OpenDevice : %s\n", strerror(errno));  
}

Launchpadmini::Launchpadmini(string deviceName, int baud)
{
   handle=-1;
   Open(deviceName,baud);
}

Launchpadmini::~Launchpadmini()
{
  if(handle >=0)
      Close();
}

void Launchpadmini::Close(void)
{
   if(handle >=0)
      close(handle);
   handle = -1;
}


bool Launchpadmini::Open(string deviceName , int baud)
{
    struct termios tio;
    struct termios2 tio2;
    this->deviceName=deviceName;
    this->baud=baud;
    handle  = open(this->deviceName.c_str(),O_RDWR | O_NOCTTY /* | O_NONBLOCK */);

    if(handle <0){
       fprintf(stderr, "OpenDevice : %s\n", strerror(errno));
       return false;
    }
    
    return true;
}

bool Launchpadmini::IsOpen(void)
{
   return( handle >=0);
}

bool Launchpadmini::Send( unsigned char  * data,int len)
{
   if(!IsOpen()) return false;
   int rlen= write(handle,data,len); 
   //fprintf(stderr, "send : %s : %x %x %x \n", strerror(errno),data[0],data[1],data[2]);
  
   return(rlen == len);
}



int  Launchpadmini::Receive( unsigned char  * data, int len)
{
   if(!IsOpen()) return -1;

   // this is a blocking receives
   int lenRCV=0;
   while(lenRCV < len)
     {
       int rlen = read(handle,&data[lenRCV],len - lenRCV);
       lenRCV+=rlen;
     }
   return  lenRCV;
}

bool Launchpadmini::NumberByteRcv(int &bytelen)
{
   if(!IsOpen()) return false;
   ioctl(handle, FIONREAD, &bytelen);
   return true;
}