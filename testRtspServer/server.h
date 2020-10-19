#ifndef SERVER_H
#define SERVER_H

#include "3rd/eventscheduler.h"
#include "net/tcpserver.h"


#define MAX_PEER_IP 128

typedef void (*callback) (void *arg);

class RtspConnection
{
public:

    RtspConnection()
    {
        mSocketId = -1;
        mPerrPort = -1;
        memset(mPeerIp, 0, MAX_PEER_IP);
    }

    void handleMsg(char *msg);

    void handleOptionCmd();
    void handleDescribeCmd();
    void handleSetupCmd();
    void handlePlayCmd();
    void handlePauseCmd();
    void handleTeardownCmd();

    int getPeerPort();
    char *getPeerIp();

    int mSocketId;
    int mPerrPort;
    char mPeerIp[MAX_PEER_IP];
};

class Server
{
public:
    Server(char *ip, int port);
    ~Server();

    bool start();

    static void newConnectionCallback(void *arg);
    void newConnection();

    bool addSink();
    void removeSink();

private:
    char *mIp;
    int mPort;

    EventScheduler *mScheduler;
    TcpServer *mTcpServer;
    IOEvent *mTcpEvent;
    std::vector<RtspConnection *> mConnections;
};

#endif // SERVER_H
