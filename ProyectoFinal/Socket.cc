#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char * address, const char * port):sd(-1)
{
    //Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
    //Con el resultado inicializar los miembros sd, sa y sa_len de la clase

    struct addrinfo *res;
    struct addrinfo hints;

    memset((void*) &hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int rc = getaddrinfo(address,port,&hints,&res);
    if(rc != 0){
        std::cout << gai_strerror(rc) << std::endl;
    }

    sd = socket(res->ai_family, res->ai_socktype,0);
    if(sd < 0){

        std::cout << gai_strerror(sd) << std::endl;
    }

    sa = *res->ai_addr;
    sa_len = res->ai_addrlen;
    freeaddrinfo(res);
}

int Socket::recv(Serializable &obj, Socket * &sock)
{

    struct sockaddr sa;
    socklen_t sa_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);
    if ( bytes <= 0 )
        return -1;

    if ( sock != 0 )
        sock = new Socket(&sa, sa_len);

    obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable& obj, const Socket& sock)
{
    //Serializar el objeto
    //Enviar el objeto binario a sock usando el socket sd
    obj.to_bin();
    
    ssize_t bytes = sendto(sd,obj.data(),obj.size(),0,&sock.sa, sock.sa_len);

    if(bytes <=0) 
        return -1;
        
    return 0;
}

bool operator== (const Socket &s1, const Socket &s2)
{
    //Comparar los campos sin_family, sin_addr.s_addr y sin_port
    //de la estructura sockaddr_in de los Sockets s1 y s2
    //Retornar false si alguno difiere

    struct sockaddr_in* addrIn1 = (struct sockaddr_in*) &(s1.sa);
    struct sockaddr_in* addrIn2 = (struct sockaddr_in*) &(s2.sa);

    bool sameAdress = addrIn1->sin_addr.s_addr == addrIn2->sin_addr.s_addr;
    bool samePort = addrIn1->sin_port == addrIn2->sin_port;
    bool sameFamily = addrIn1->sin_family == addrIn2->sin_family;

    return sameAdress && samePort && sameFamily;
};

std::ostream& operator<<(std::ostream& os, const Socket& s)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    getnameinfo((struct sockaddr *) &(s.sa), s.sa_len, host, NI_MAXHOST, serv,
                NI_MAXSERV, NI_NUMERICHOST);

    os << host << ":" << serv;

    return os;
};

