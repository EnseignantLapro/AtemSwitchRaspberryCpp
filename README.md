# AtemSwitchRaspberryCpp
Portage of the https://github.com/kasperskaarhoj arduino Code To Raspberry C++ for ATEM Software Control 6.3


This code work but is instable. It's dependant on timming.
When atem switcher have no ping , he stop the connection and we stop the program.
We have to make a thread for ping UDP and reponse for a ping UDP ( its a protocol to atem 6.3)

The code work an compile on raspberry pi 3b+

I dont kown how we can take some keybord input....

On the TestAtem Program 
I change the video input every second. but if it lasts too long, we lose the connection


With this projet the idéa is to take somme input signal to GPIO for control an Atem switcher .