#include "ClassDatagrammeUDP.h"

//dans ClassDatagramme j'implemente les méthodes
//pour savoir à quelle class les méthodes appartiennent il faut mettre
//devant chaque méthode Nomdelaclass::

int ClassDatagrammeUDP::getNumDatagramme() const {
        return _numDatagramme;
    }

//constructeur
ClassDatagrammeUDP::ClassDatagrammeUDP(){ 
    _numDatagramme = 1;
    
}