#include "Launchpadmini.h"



using namespace std;

//JLA concatenation 2 * 4bit => to 8 bit value
unsigned int smallwordbutton(uint8_t value15_8,uint8_t value0_7)
{
	return value15_8 << 4 | value0_7;
   
}

Launchpadmini::Launchpadmini()
{
  
   handle=-1;
   //TODO search launchpad mini navation auto
   if(!Open("/dev/snd/midiC1D0",115200)) fprintf(stderr, "OpenDevice : %s\n", strerror(errno));  
}

void Launchpadmini::LaunchMappingThread(ATEM *AtemSwitcher){
   
   _AtemSwitcher = AtemSwitcher;
   _threadOn = true;
   _threadRefreshLaunchpad = std::thread(&Launchpadmini::RefreshMappingWithAtemBlackMagic, this);
   
}
void Launchpadmini::RefreshMappingWithAtemBlackMagic()
{
   
   
   while(_threadOn){
      usleep(100000);
      MappingWithAtemBlackMagic();
      //set Programm On Pad
      if(_AtemSwitcher->getProgramInput() < 6){
      _dataLaunchPad[1]=smallwordbutton(0,_AtemSwitcher->getProgramInput()-1);
      }
      if(_AtemSwitcher->getProgramInput() == 0 ){
         _dataLaunchPad[1]=smallwordbutton(0,6);
      }
      if(_AtemSwitcher->getProgramInput() == 6 ){
         _dataLaunchPad[1]=smallwordbutton(1,0);
      }

      _dataLaunchPad[2]=0x07;
      Send(_dataLaunchPad,3);

      //set Previeux on Pad
      if(_AtemSwitcher->getPreviewInput() < 6){
      _dataLaunchPad[1]=smallwordbutton(2,_AtemSwitcher->getPreviewInput()-1);
      }
      if(_AtemSwitcher->getPreviewInput() == 0 ){
         _dataLaunchPad[1]=smallwordbutton(2,6);
      }
      if(_AtemSwitcher->getPreviewInput() == 6 ){
         _dataLaunchPad[1]=smallwordbutton(3,0);
      }

      _dataLaunchPad[2]=0x30;
      Send(_dataLaunchPad,3);

      // Set Transition on Pad / Transition 0=MIX, 1=DIP, 2=WIPE, 3=DVE, 4=STING
      _dataLaunchPad[1]=smallwordbutton(4,_AtemSwitcher->getTransitionType());
      _dataLaunchPad[2]=0x1E;
      Send(_dataLaunchPad,3);

      //set transition Style and cut
      if(_AtemSwitcher->getTransitionPreview()){
      _dataLaunchPad[2]=0x01;
      }else{
      _dataLaunchPad[2]=0x1D;
      }
      _dataLaunchPad[1]=smallwordbutton(5,0);
      Send(_dataLaunchPad,3);

      //set Key Stream On
      if(_AtemSwitcher->getUpstreamKeyerStatus(1)){
      _dataLaunchPad[2]=0x01;
      }else{
         _dataLaunchPad[2]=0x1D;
      }
      _dataLaunchPad[1]=smallwordbutton(4,5);
      Send(_dataLaunchPad,3);
   }
}


Launchpadmini::Launchpadmini(string deviceName, int baud)
{
   handle=-1;
   Open(deviceName,baud);
}

Launchpadmini::~Launchpadmini()
{
   _threadOn = false;
   _threadRefreshLaunchpad.join();

   if(handle >=0) Close();
      
}

void Launchpadmini::Close(void)
{
   _threadOn = false;
   _threadRefreshLaunchpad.join();
   if(handle >=0)
      close(handle);
       
   handle = -1;
}


bool Launchpadmini::Open(string deviceName , int baud)
{
    
    this->deviceName=deviceName;
    this->baud=baud;
    handle  = open(this->deviceName.c_str(),O_RDWR | O_NOCTTY /* | O_NONBLOCK */);

    if(handle <0){
       fprintf(stderr, "OpenDevice : %s\n", strerror(errno));
       return false;
    }
    //off all button 
    SendResetLaunchpad();
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

//send red info to launchpad for one button
bool Launchpadmini::SendButtonPushRed( unsigned int NumButton)
{
   _dataLaunchPad[0]=0x90;
   _dataLaunchPad[1]=NumButton;
   _dataLaunchPad[2]=0x07;
   Send(_dataLaunchPad,3);
}

//send red info to launchpad for one button
bool Launchpadmini::SendButtonPushLowRed( unsigned int NumButton)
{
   _dataLaunchPad[0]=0x90;
   _dataLaunchPad[1]=NumButton;
   _dataLaunchPad[2]=0x01;
   Send(_dataLaunchPad,3);
}

//send red info to launchpad for one button
bool Launchpadmini::SendButtonPushLowGreen( unsigned int NumButton)
{
   _dataLaunchPad[0]=0x90;
   _dataLaunchPad[1]=NumButton;
   _dataLaunchPad[2]=0x50;
   Send(_dataLaunchPad,3);
}

//send red info to launchpad for one button
bool Launchpadmini::SendButtonPushLowYellow( unsigned int NumButton)
{
   _dataLaunchPad[0]=0x90;
   _dataLaunchPad[1]=NumButton;
   _dataLaunchPad[2]=0x1D;
   Send(_dataLaunchPad,3);
}

//send off led info the launchpad for one button
bool Launchpadmini::SendButtonOff( unsigned int NumButton)
{
   //B0 00  00
   _dataLaunchPad[0]=0x90;
   _dataLaunchPad[1]=NumButton;
   _dataLaunchPad[2]=0x00;
   Send(_dataLaunchPad,3);
}

bool Launchpadmini::SendResetLaunchpad()
{
   //B0 00  00
   _dataLaunchPad[0]=0xB0;
   _dataLaunchPad[1]=00;
   _dataLaunchPad[2]=0x00;
   Send(_dataLaunchPad,3);
}



int  Launchpadmini::Receive( unsigned char  * data, int len)
{
   if(!IsOpen()) return -1;

   // this is a blocking receives
   int lenRCV=0;
   int rlen=0;
   while(lenRCV < len)
     {
       rlen = read(handle,&data[lenRCV],len - lenRCV);
       lenRCV+=rlen;
     }
   
   
   
   ioctl(handle, FIONREAD, (int)&rlen);
   
   if(rlen>0){
      char flush[rlen];
      read(handle,flush,rlen);
   } 
   return  lenRCV;
}



void Launchpadmini::MappingWithAtemBlackMagic(){
      //Reset Button Color for new mappin
      
      

      //low red for inputprogram
      for(size_t i = 0; i < 7; i++){
         
         SendButtonPushLowRed(smallwordbutton(0,i)); 
      }
      SendButtonPushLowRed(smallwordbutton(1,0));
      //BAR
      SendButtonPushLowRed(smallwordbutton(1,5));
      //MEDIA1
      SendButtonPushLowRed(smallwordbutton(1,6));
      //MEDIA2
      SendButtonPushLowRed(smallwordbutton(1,7));    
      //low green for inputPreview
      for(size_t i = 0; i < 7; i++){
         
         SendButtonPushLowGreen(smallwordbutton(2,i));
      }
      SendButtonPushLowGreen(smallwordbutton(3,0));
      //BAR
      SendButtonPushLowGreen(smallwordbutton(3,5));
      //MEDIA1
      SendButtonPushLowGreen(smallwordbutton(3,6));
      //MEDIA2
      SendButtonPushLowGreen(smallwordbutton(3,7));    

      //low yellon Ambre for transition style
      //smallwordbutton(x,y) line , colone
      SendButtonPushLowYellow(smallwordbutton(4,0));
      SendButtonPushLowYellow(smallwordbutton(4,1));
      SendButtonPushLowYellow(smallwordbutton(4,2));
      SendButtonPushLowYellow(smallwordbutton(5,0));
      SendButtonPushLowYellow(smallwordbutton(5,2));
      SendButtonPushLowYellow(smallwordbutton(5,3));

      //low yellon Ambre for Next Transition
      SendButtonPushLowRed(smallwordbutton(4,5));
      SendButtonPushLowYellow(smallwordbutton(5,4));
      SendButtonPushLowYellow(smallwordbutton(5,5));
      

}