#include "server.h"

#define SERVER_PORT     8554

Server::Server(char *ip, int port) :
    mIp(ip),
    mPort(port)
{
    mScheduler = EventScheduler::createNew(EventScheduler::PollerType::POLLER_POLL);
}

Server::~Server()
{

}

bool Server::start()
{
    mTcpServer = new TcpServer(mIp, mPort);

    mTcpEvent = IOEvent::createNew(mTcpServer->mFd, this);
    mTcpEvent->setReadCallback(newConnectionCallback);
    mTcpEvent->enableReadHandling();
    mScheduler->addIOEvent(mTcpEvent);
    mScheduler->loop();
}

void Server::newConnectionCallback(void *arg)
{
    Server *server = (Server *)arg;
    server->newConnection();
}

void Server::newConnection()
{
    RtspConnection *connection = new RtspConnection();
    connection->mSocketId = mTcpServer->accept(connection->mPeerIp, &connection->mPerrPort);
    mConnections.push_back(connection);
    printf("the socketid: %d, the ip: %s, the port: %d\n", connection->mSocketId, connection->mPeerIp, connection->mPerrPort);
}
