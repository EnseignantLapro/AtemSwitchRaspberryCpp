# JLA Mon premier Makefile
# compilation de tous nos fichier .o
# si vous avez une dependant il faut la compiler plus bas.
# sur all: on a une dependance test.o du coup on doit compiler la cible test.o
# sur all: on a une dependance ClassDatagrammeUDP.o du coup on doit compiler la cible ClassDatagrammeUDP.o
all: EthernetUDP.o ATEM.o Launchpadmini.o TestAtem.o
	g++ EthernetUDP.o ATEM.o Launchpadmini.o TestAtem.o -o  TestAtem -g -pthread

#creation de nos fichiers .o
EthernetUDP.o: EthernetUDP.cpp EthernetUDP.h
	g++ -g -c EthernetUDP.cpp

#creation de nos fichiers .o
ATEM.o: ATEM.cpp ATEM.h
	g++ -g -c ATEM.cpp

#creation de nos fichiers .o
Launchpadmini.o: Launchpadmini/Launchpadmini.cpp Launchpadmini/Launchpadmini.h
	g++ -g -c Launchpadmini/Launchpadmini.cpp

#compile de la cible test.o
TestAtem.o: TestAtem.cpp
	g++ -g -c TestAtem.cpp

clean:
	rm -rf *.o
