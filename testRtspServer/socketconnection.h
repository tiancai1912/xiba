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

    int createTcpSocket();
    int createUdpSocket();
    int bindSocketAddr(int sockfd, const char* ip, int port);
    int acceptClient(int sockfd, char* ip, int* port);

    static char* getLineFromBuf(char* buf, char* line);

    void doClient(int clientSockfd, const char* clientIP, int clientPort,
                            int serverRtpSockfd, int serverRtcpSockfd);

    int handleCmd_OPTIONS(char* result, int cseq);
    int handleCmd_DESCRIBE(char* result, int cseq, char* url);
    int handleCmd_SETUP(char* result, int cseq, int clientRtpPort);
    int handleCmd_PLAY(char* result, int cseq);
};

#endif // SOCKETCONNECTION_H
