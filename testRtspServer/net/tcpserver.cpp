#include "tcpserver.h"

TcpServer::TcpServer(const char *ip, int port)
{
    mIp = (char *)malloc(256);
    memset(mIp, 0, sizeof(char) * 256);
    strcpy(mIp, ip);

    mPort = port;
    mFd = create();

    bind();
    listen();
}

int TcpServer::create()
{
    int sockfd;
    int on = 1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        return -1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

    return sockfd;
}

TcpServer::~TcpServer()
{
}

int TcpServer::accept(char *ip, int *port)
{
    int clientfd;
    socklen_t len = 0;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    len = sizeof(addr);

    clientfd = ::accept(mFd, (struct sockaddr *)&addr, &len);
    if(clientfd < 0)
        return -1;

    strcpy(ip, inet_ntoa(addr.sin_addr));
    *port = ntohs(addr.sin_port);

    return clientfd;
}

bool TcpServer::bind()
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(mPort);
    addr.sin_addr.s_addr = inet_addr(mIp);

    if(::bind(mFd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
        return false;

    return true;
}

void TcpServer::listen()
{
    ::listen(mFd, 10);
}
