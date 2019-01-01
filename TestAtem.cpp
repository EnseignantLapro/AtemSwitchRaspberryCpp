/*****************
 * Example: ATEM Monitor Preconfigured
 * Connects to the Atem Switcher and outputs changes to Preview and Program on the Serial monitor (at 9600 baud)
 * Arduino must be configured with ethernet addresses using the sketch "ConfigEthernetAddresses" first.
 *
 * - kasper
 */
/*****************
 * TO MAKE THIS EXAMPLE WORK:
 * - You must have an Arduino with Ethernet Shield (or compatible such as "Arduino Ethernet", http://arduino.cc/en/Main/ArduinoBoardEthernet)
 * - You must have an Atem Switcher connected to the same network as the Arduino - and you should have it working with the desktop software
 * - You must make specific set ups in the below lines where the comment "// SETUP" is found!
 */

// Include ATEM library and make an instance:
#include "ATEM.h"
#include "Launchpadmini/Launchpadmini.h"
#include <string>
#include <sstream>

using namespace std;

ATEM AtemSwitcher;
 Launchpadmini LaunchPad;

//#include <MemoryFree.h>

// Configure the IP addresses and MAC address with the sketch "ConfigEthernetAddresses":
uint8_t ip[4];      // Will hold the Arduino IP address
uint8_t atem_ip[4]; // Will hold the ATEM IP address
uint8_t mac[6];     // Will hold the Arduino Ethernet shield/board MAC address (loaded from EEPROM memory, set with ConfigEthernetAddresses example sketch)

void setup()
{
  //log consol True or false
  AtemSwitcher.serialOutput(false);

  SOCKADDR_IN sin;
  sin.sin_addr.s_addr = inet_addr("192.168.1.3"); //inet_addr("192.168.1.98");//htonl(INADDR_ANY);//INADDR_LOOPBACK;//inet_addr("192.168.1.98"); //htonl(INADDR_LOOPBACK);
  sin.sin_family = AF_INET;
  //htonl est une fonction qui donne le port spécifié en paramètre
  sin.sin_port = htons(9910);
  // Initialize a connection to the switcher:
  AtemSwitcher.begin(sin);
  cout << " ATEM Switcher IP Address: " << inet_ntoa(sin.sin_addr) << "\n";

}

int main()
{
  
  setup();
 
  unsigned char  dataLaunchPad[] = { 0x90, 0x00, 0x00};
    
  // Check for packets, respond to them etc. Keeping the connection alive!
  // VERY important that this function is called all the time - otherwise connection might be lost because packets from the switcher is
  // overlooked and not responded to.
 
  bool testAtem = true;
  
  if(testAtem){
  if (AtemSwitcher.InitAtemConnection())
  {
      cout << "\033[1;32m";
      cout << "Succes Connecting!\n";
      cout << "\033[0m\n";
      if (AtemSwitcher.AtemGetCommands())
      {
        cout << "\033[1;32m";
        cout << "Succes Command!\n";
        cout << "\033[0m\n";

        //test auto change input 3
        
        LaunchPad.LaunchMappingThread(&AtemSwitcher);
        int chanel = 0;
        while (true)
        {
          
          cout << "Selection Program change! : ";
          LaunchPad.Receive(dataLaunchPad,3);
          if(dataLaunchPad[1]==0x6F)break;
          //Programme Button
          if(dataLaunchPad[1]==0x00)AtemSwitcher.changeProgramInput(1);
          if(dataLaunchPad[1]==0x01)AtemSwitcher.changeProgramInput(2);
          if(dataLaunchPad[1]==0x02)AtemSwitcher.changeProgramInput(3);
          if(dataLaunchPad[1]==0x03)AtemSwitcher.changeProgramInput(4);
          if(dataLaunchPad[1]==0x04)AtemSwitcher.changeProgramInput(5);
          if(dataLaunchPad[1]==0x10)AtemSwitcher.changeProgramInput(6);
          if(dataLaunchPad[1]==0x05)AtemSwitcher.changeProgramInput(0);
          
          //Preview Button
          if(dataLaunchPad[1]==0x20)AtemSwitcher.changePreviewInput(1);
          if(dataLaunchPad[1]==0x21)AtemSwitcher.changePreviewInput(2);
          if(dataLaunchPad[1]==0x22)AtemSwitcher.changePreviewInput(3);
          if(dataLaunchPad[1]==0x23)AtemSwitcher.changePreviewInput(4);
          if(dataLaunchPad[1]==0x24)AtemSwitcher.changePreviewInput(5);
          if(dataLaunchPad[1]==0x30)AtemSwitcher.changePreviewInput(6);
          if(dataLaunchPad[1]==0x25)AtemSwitcher.changePreviewInput(0);

          //transition Button
          if(dataLaunchPad[1]==0x40)AtemSwitcher.changeTransitionType(0);
          if(dataLaunchPad[1]==0x41)AtemSwitcher.changeTransitionType(1);
          if(dataLaunchPad[1]==0x42)AtemSwitcher.changeTransitionType(2);
          //Preview Transition
          //when push inverse the TransitionPreview true => false , and false become true
          if(dataLaunchPad[1]==0x50)AtemSwitcher.changeTransitionPreview(!AtemSwitcher.getTransitionPreview());
          //CUT CUT AUTO
          if(dataLaunchPad[1]==0x52){
            AtemSwitcher.doCut();
            dataLaunchPad[1]=smallwordbutton(4,2);
            dataLaunchPad[2]=0x1E;
            LaunchPad.Send(dataLaunchPad,3);
          }
          if(dataLaunchPad[1]==0x53){
            AtemSwitcher.doAuto();
            dataLaunchPad[1]=smallwordbutton(4,3);
            dataLaunchPad[2]=0x1E;
            LaunchPad.Send(dataLaunchPad,3);
          }
          //NextTransition On Air ( put inverse of actual bool)
          if(dataLaunchPad[1]==0x45)AtemSwitcher.changeUpstreamKeyOn(1,!AtemSwitcher.getUpstreamKeyerStatus(1));

        }

        cout << "\033[1;31m";
        cout << "Fin du test Atem!\n";
        cout << "\033[0m\n";
      }
      else
      {
        cout << "\033[1;31m";
        cout << "No Commands Receiv!\n";
        cout << "\033[0m\n";
      }
    }
    else
    {
      cout << "\033[1;31m";
      cout << "Fail Connecting!\n";
      cout << "\033[0m\n";
    }
    AtemSwitcher.deconnect();
  }
   cout << "\033[1;31m";
        cout << "Fin du program !\n";
        cout << "\033[0m\n";

        return 1;
}