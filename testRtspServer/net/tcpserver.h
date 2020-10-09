#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "net/socket.h"
//#include "socketconnection.h"

class TcpServer
{
public:
    TcpServer(const char *ip, int port);
    ~TcpServer();

    int accept(char *ip, int *port);
private:
    int create();
    bool bind();
    void listen();

public:
    int mFd;
    char *mIp;
    int mPort;
};

#endif // TCPSERVER_H
