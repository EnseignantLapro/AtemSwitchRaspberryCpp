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
#include <string>
#include <sstream>

using namespace std;

ATEM AtemSwitcher;

//#include <MemoryFree.h>

// Configure the IP addresses and MAC address with the sketch "ConfigEthernetAddresses":
uint8_t ip[4];      // Will hold the Arduino IP address
uint8_t atem_ip[4]; // Will hold the ATEM IP address
uint8_t mac[6];     // Will hold the Arduino Ethernet shield/board MAC address (loaded from EEPROM memory, set with ConfigEthernetAddresses example sketch)

void setup()
{

  // Start the Ethernet, Serial (debugging) and UDP:

  SOCKADDR_IN sin;
  sin.sin_addr.s_addr = inet_addr("192.168.1.3"); //inet_addr("192.168.1.98");//htonl(INADDR_ANY);//INADDR_LOOPBACK;//inet_addr("192.168.1.98"); //htonl(INADDR_LOOPBACK);
  sin.sin_family = AF_INET;
  //htonl est une fonction qui donne le port spécifié en paramètre
  sin.sin_port = htons(9910);

  cout << " ATEM Switcher IP Address: " << inet_ntoa(sin.sin_addr) << "\n";

  //log consol True or false
  AtemSwitcher.serialOutput(false);

  // Initialize a connection to the switcher:
  AtemSwitcher.begin(sin);

  //AtemSwitcher.connect();

  // Shows free memory:
  //  Serial << F("freeMemory()=") << freeMemory() << "\n";
}

int main()
{
  //I don't anderstand why this variable must declare and not use => else not input keybord work
  
  setup();


	//Get input

  
  
  

  

 







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
        

        int chanel = 0;
        while (true)
        {

          cin.clear(); cin.sync();
          sleep(3);

          if (chanel > 5)
          {
            chanel = 0;
          }
          chanel++;
          cout << "Selection Program change! : ";
          AtemSwitcher.changeProgramInput(chanel);
          //in this test if the answer is To long => the switcher is disconnect :D haha
          cout << " \n Chanel choose is "<< chanel <<" "<< endl;

          //
          //getline(cin, InputChanelChoice);
          //stringstream(InputChanelChoice) >> my_InputChanelChoice_int;			


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