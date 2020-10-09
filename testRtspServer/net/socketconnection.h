#ifndef SOCKETCONNECTION_H
#define SOCKETCONNECTION_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include <fcntl.h>
#include <assert.h>

class SocketConnection
{
public:
    SocketConnection();

    int connect(bool isTcp, char *ip, int port);
    void disConnect();
//    static char *readLine(char *buf, char *line);

//private:
    int createTcpSocket();
    int createUdpSocket();
    int bindSocketAddr(int sockfd, const char* ip, int port);
    int acceptClient(int sockfd, char* ip, int* port);

    static char* getLineFromBuf(char* buf, char* line);

    int m_socket_fd;
};

#endif // SOCKETCONNECTION_H
