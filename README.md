# AtemSwitchRaspberryCpp
Portage of the https://github.com/kasperskaarhoj arduino Code To Raspberry C++ for ATEM Software Control 6.3

---COMMIT 4 : -----

Mapping with Novation Launchpad test ok for chanel Program / chanel Preview / cut and style Transition /
Add new thread for the mapping of launchpad . It's for update launchpad color when atem switcher have new info.

There is a bug when Atem not responding. there is no automatic reconnexion for this moment

---COMMIT 3 : -----
Tryin to take input from an USB midi controller. LaunchPad Mini Novation

the command pi@raspberrypi:~ $ amidi -l
Dir Device    Name
IO  hw:1,0,0  Launchpad Mini MIDI 1

Then pi@raspberrypi:~ $ amidi --port="hw:1,0,0" --dump
you can see the hexaCode send by the midi keyboard. Use it to map with your c++ program

Send color on launchpad 24 button 
$ amidi --port="hw:1,0,0" --send-hex 90 24 30
OO for of coolor on 24 button
$ amidi --port="hw:1,0,0" --send-hex 90 24 OO

launchpad use /dev/snd/midiC1D0 speed 115200b/s we have to read/write this files.

it's work

---COMMIT 2 : -----
EN : The code is more stable because there is a thread that is behind and ping constantly the atem mixer. perhaps like the thread sending data on "cout <<" it is impossible yet to recover the values of a keyboard.

FR : Le code est plus stable car il y a un thread qui est dérriere et qui ping sans cesse l'atem melangeur. Par contre comme le thread envoi des données sur "cout <<" il est impossible encore de récupérer les valeurs d'un clavier.
---COMMIT 1 : -----
This code works but it is unstable and depends on timings. If Atem does not receive a ping for a very short time, it disconnects. So it's very difficult to disguise because the time between each line is longer and Atem disconnects. But it's still possible by setting the breakpoints correctly.

We have to make a thread for ping UDP and reponse for a ping UDP ( its a protocol to atem 6.3)

The code works and it compile on raspberry pi 3b+

I dont kown how we can take some keybord input....

On the TestAtem Program 
I change the video input every 500ms. but if it lasts too long, we lose the connection


With this projet the idéa is to put input signal to GPIO for control an Atem switcher .

I'm a french developpeur 
Ce code fonctionne mais il est instable est dépend des timings. Si Atem ne reçois pas de ping durant un laps de temps trés court, il se déconnecte. Du coup c'est trés difficile de déguguer car les temps entre chaque ligne est plus long et Atem se déconnect. Mais c'est possible tout de même en positionnant les points d'arret correctement.