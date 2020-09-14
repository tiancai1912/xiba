#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/uio.h>

#include <string.h>
#include <unistd.h>
//#include <sys/types.h>          /* See NOTES */
//#include <fcntl.h>
//#include <stdio.h>
#include <sys/ioctl.h>
//#include <netinet/in.h>
#include <net/if.h>

class Socket
{
public:
    Socket();

    int createTcpSocket();
    int cretaeUdpSocket();
    bool bind(int sockfd, std::string ip, u_int16_t port);
    bool listen(int sockfd, int backlog);
    int accept(int sockfd);
    std::string getPerrIp(int sockfd);
    int16_t getPeerPort(int sockfd);
    int getPerrAddr(int sockfd, struct sockaddr_in *addr);
    void close(int sockfd);
    bool connect(int sockfd, std::string ip, u_int16_t port, int timeout);
    std::string getLocalIp();

};

#endif // SOCKET_H
