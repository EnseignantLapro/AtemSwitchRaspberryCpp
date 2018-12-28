//dans les.h, il est recommandé de ne jamais mettre la directiveusing namespace

class ClassDatagrammeUDP
{
    //les membres de ma classs
    private :
    int _numDatagramme;

    //les méthodes public de ma classs
    public :

    //o On utilise le mot-clé const sur des méthodes qui ne  modifie pas les membres de l'objet
    //çà optimise votre programme
    int getNumDatagramme() const;

    //constructeur
    ClassDatagrammeUDP();

};