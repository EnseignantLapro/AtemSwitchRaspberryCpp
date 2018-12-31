//Include Pour IHM entrée sortie cout cin en mode console
//un excelent tuto pour revoir tout les grands principe du c++
//THis test.cpp ise use for Test some thing 
//https://openclassrooms.com/fr/courses/1894236-programmez-avec-le-langage-c/1897113-decouvrez-la-notion-de-programmation-orientee-objet-poo

#include <iostream>
//Include pour les sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include <stdlib.h>
#include <errno.h>
#include <string>

//Include pour mes classes que je developpe
//on utile des " " car la ce sont les .h de notre dossier et non du dossier du le systeme pour les < >




//définition additionnel pour un systeme linux
//cela revient à déclarer une constante
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define PORT2 58886

//définition des typedef
//permet de réécrire une déclaration c'est une sorte d'alias.
// ainsi on utilise SOCKADDR_IN plutot que de déclarer une structure sockaddr_in c'est un racourcie
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

//on pourrait aussi comme les bibiothèque windows et linux sont différente,
//ajouter un #if dfeined (WIN32) les includ windows #elseif defined (LINUX) les includes linux #end if 

using namespace std;

void maFonction(int b){
    b = 3;
    cout << "Hello world by 223 remte\n";
}

//exemple de déclaration d'une class.
class user
{
    //les membres de ma classs
    private :
    int nomID;

    //les méthodes public de ma classs
    public :

    int getnomID(){
        return nomID;
    }

    //constructeur
    user(){ 
        nomID = 1;
    }

};

int main2() {
    int a = 1;
    
    //cas du poiteur sur mon objet
    user *UserID =  new user();

    //cas de copie de l'objet
    user UserID2 =   user();

    // utiliser les -> pour un pointeur d'objet
    a = UserID->getnomID();

    // utiliser le . pour un objet
    a = UserID2.getnomID();

    maFonction(a);

    //PROGRAMMATION ____ RESEAU_______
    //LE SERVER UDP

    //création de socket --------------------
    //prototype de la création de socket int socket(int domain, int type, int protocol);
    //la fonction retourn un int çà tombe bien on à typedef int SOCKET de déclarer en entete
    // on va donc l'utiliser c'est plus sympa d'avoir a ecrire SOCKET que int mais c'est le même chose c'est un alias
    SOCKET sock;
    
    //AF_INET c'est pour le protocole TCP/IP.
    //SOCK_DGRAM c'est pour utiliser UDP ( pour tcp c'est SOCK_STREAM
    //on utilise pas le champ protocol pour du TCP IP donc on met 0)
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    //Fin création de socket ---------------

    //paramétrage de socket -----------------
    //on va utiliser la structure déclaré en entete SOCKADDR_IN
    /*   voici la struture sockaddr_in
    struct sockaddr_in
    {
        short      sin_family; //pour de UDP linux c'est toujours AF_INET
        unsigned short   sin_port; //le port sur lequel sera connecté la socket
        struct   in_addr   sin_addr; //IP de la socket
        char   sin_zero[8]; // non utilisé
    }; 
    */
    //on appelera notre structure de paramétrage InfoServer
    SOCKADDR_IN InfoServer;
    //htonl est une fonction qui donne automatiquement IP de notre machine
    // met on peut aussi la forcer sin.sin_addr.s_addr = inet_addr("127.0.0.1");inet_addr("192.168.1.98");  
    InfoServer.sin_addr.s_addr = htonl(INADDR_ANY);  //inet_addr("192.168.1.98");//htonl(INADDR_ANY);//INADDR_LOOPBACK;//inet_addr("192.168.1.98"); //htonl(INADDR_LOOPBACK); 
    InfoServer.sin_family = AF_INET;
    //htonl est une fonction qui donne le port spécifié en paramètre
    InfoServer.sin_port = htons(9012);
    printf("Listage du port %d...\n", 9012);

    //pour appliquer ses paramétres (sinon la strucuture est dans le vide un peu comme en série getcomstat et setcomstat)
    /*on utilise pour cela le prototype :
    - int bind(int socket, const struct sockaddr* addr, socklen_t addrlen);
     la fonction retourne SOCKET_ERROR défini en entete -1 en cas d'erreur
    - int socket c'est notre socket ici sock déclaré plus haut
    - sockaddr* addr est un pointeur de la structure sockaddr du serveur. c'est IP du server qu'on veut contacter
    // c'est un pointeur qui est attendu or nous on a une struture donc on va le caster notre sin en mettant (SOCKADDR*)&
    - addrlen c'est la taille de notre structure qui peux varier selon les infos. */
    int error_message;
    error_message = ::bind(sock, (SOCKADDR*)&InfoServer, sizeof(InfoServer));

    //Fin paramétrage de socket -------------
    //UDP recupérer le message--------------

    char buffer[1500];
    memset (buffer,'\0',1500);
    sockaddr_in _from;
    socklen_t fromlen = sizeof(_from);
    error_message = recvfrom(sock, buffer, 1500, 0, reinterpret_cast<sockaddr*>(&_from), &fromlen);
    if(error_message<1){
        fprintf(stderr, "socket() message: %s\n", strerror(errno));
    }else{
        printf(" IP : %s ", inet_ntoa(_from.sin_addr));
        printf(" Port : %d ", ntohs(_from.sin_port));
        printf("Recu : %s ", buffer);
    }

    //FIN UDP recuperer le message-----------

    //Mise en écoute de notre socket que pour SOCK_STREAM---------
    // on utilise pour çà le protopype
    /*
    int listen(int socket, int backlog);
    -socket c'est notre socket (sock)
    -backlog représente le nombre maximal de connexions pouvant être mises en attente.
    */

    //error_message = listen(sock, 5);
    //fprintf(stderr, "socket() failed: %s\n", strerror(errno));
    //on utilise un deuxieme prototype pour accepter les connexions
    /*
    int accept(int socket, struct sockaddr* addr, socklen_t* addrlen);
    a fonction retourne la valeur INVALID_SOCKET en cas d'échec
    - addr est un pointeur sur le contexte d'adressage du client.
    - taille du contexte d'adressage du client
    */
    
    //création du socket que remplira un client
    SOCKADDR_IN destAddrUdp;
    SOCKET csock;

    //pour tcp socktream
    //socklen_t taille = sizeof(csin);
   // csock = accept(sock, (SOCKADDR*)&csin, &taille);
    //fprintf(stderr, "socket() failed: %s\n", strerror(errno));
    //Fin Mise en écoute de notre socket -----

    //fin de connexion 

    close(sock);
    string key;
    getline(cin, key); 
    //LE CLIENT UDP 
    //on va utiliser les mêmes  types de variable que le server UDP

    /* Création de la socket d'envoi */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
 
    /* Configuration de la connexion pour TCP AF_INET pour UDP AF_UNSPEC*/
    destAddrUdp.sin_addr.s_addr = inet_addr("192.168.1.3");
    destAddrUdp.sin_family = AF_INET;
    destAddrUdp.sin_port = htons(9910);
    
        /* Si le client arrive à se connecter cap UDP */
        
        /*  if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR)
            printf("Connexion à %s sur le port %d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));
        else
            printf("Impossible de se connecter\n"); 
        */

    buffer[0]= 0x08;
    buffer[1]= 0x18;
    buffer[2]= 0x80;
    buffer[3]= 0x14;

    buffer[4]= 0x00;
    buffer[5]= 0x00;
    buffer[6]= 0x00;
    buffer[7]= 0x00;
    buffer[8]= 0x00;
    buffer[9]= 0x00;
    buffer[10]= 0x00;

    buffer[11]= 0x63;
    buffer[12]= 0x00;
    buffer[13]= 0x0c;
    buffer[14]= 0x92;
    buffer[15]= 0x0a;
    buffer[16]= 0x43;
    buffer[17]= 0x50;
    buffer[18]= 0x67;
    buffer[19]= 0x49;
    buffer[20]= 0x00;
    buffer[21]= 0x0d;
    buffer[22]= 0x00;
    buffer[23]= 0x02;
    
    socklen_t tailleudp = sizeof(destAddrUdp);
    error_message = sendto(sock,buffer,sizeof(buffer),0, (SOCKADDR*)&destAddrUdp,sizeof(destAddrUdp));
        fprintf(stderr, "socket() message: %s\n", strerror(errno));
    
    


    /* On ferme la socket précédemment ouverte */
    close(sock);

    return a;
}

