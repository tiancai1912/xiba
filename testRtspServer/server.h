#ifndef SERVER_H
#define SERVER_H

#include "3rd/eventscheduler.h"
#include "net/tcpserver.h"


#define MAX_PEER_IP 128

class Server
{
public:
    Server(char *ip, int port);
    ~Server();

    bool start();

    struct RtspConnection
    {
      int mSocketId;
      int mPerrPort;
      char mPeerIp[MAX_PEER_IP];

      RtspConnection()
      {
          mSocketId = -1;
          mPerrPort = -1;
          memset(mPeerIp, 0, MAX_PEER_IP);
      }
    };

    static void handleReadCallback(void *arg);
    void handleRead();


private:
    char *mIp;
    int mPort;

    EventScheduler *mScheduler;
    TcpServer *mTcpServer;
    IOEvent *mTcpEvent;
    std::vector<RtspConnection> mConnections;
};

#endif // SERVER_H
