# AtemSwitchRaspberryCpp
Portage of the https://github.com/kasperskaarhoj arduino Code To Raspberry C++ for ATEM Software Control 6.3


This code works but it is unstable and depends on timings. If Atem does not receive a ping for a very short time, it disconnects. So it's very difficult to disguise because the time between each line is longer and Atem disconnects. But it's still possible by setting the breakpoints correctly.

We have to make a thread for ping UDP and reponse for a ping UDP ( its a protocol to atem 6.3)

The code works and it compile on raspberry pi 3b+

I dont kown how we can take some keybord input....

On the TestAtem Program 
I change the video input every 500ms. but if it lasts too long, we lose the connection


With this projet the idéa is to put input signal to GPIO for control an Atem switcher .

I'm a french developpeur 
Ce code fonctionne mais il est instable est dépend des timings. Si Atem ne reçois pas de ping durant un laps de temps trés court, il se déconnecte. Du coup c'est trés difficile de déguguer car les temps entre chaque ligne est plus long et Atem se déconnect. Mais c'est possible tout de même en positionnant les points d'arret correctement.