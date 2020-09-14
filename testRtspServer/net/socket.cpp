#include "socket.h"

Socket::Socket()
{

}

int Socket::createTcpSocket()
{
    int fd = socket(AF_INET, SOCK_STREAM| SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    return fd;
}

int Socket::cretaeUdpSocket()
{
    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    return fd;
}

bool Socket::bind(int sockfd, std::string ip, u_int16_t port)
{
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    if (::bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return false;
    }

    return true;
}

bool Socket::listen(int sockfd, int backlog)
{
    if (::listen(sockfd, backlog) < 0) {
        return false;
    }

    return true;
}

int Socket::accept(int sockfd)
{
    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof(struct sockaddr_in);

    int connfd = ::accept(sockfd, (struct sockaddr *)&addr, &addrlen);
    return connfd;
}

std::string Socket::getPerrIp(int sockfd)
{
    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0) {
        return inet_ntoa(addr.sin_addr);
    }

    return "0.0.0.0";
}

int16_t Socket::getPeerPort(int sockfd)
{
    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0) {
        return ntohs(addr.sin_port);
    }

    return 0;
}

int Socket::getPerrAddr(int sockfd, struct sockaddr_in *addr)
{
    socklen_t addrlen = sizeof(struct sockaddr_in);
    return getpeername(sockfd, (struct sockaddr *)addr, &addrlen);
}

void Socket::close(int sockfd)
{
    int ret = ::close(sockfd);
}

bool Socket::connect(int sockfd, std::string ip, u_int16_t port, int timeout)
{
    bool isConnected = true;
    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (::connect(sockfd, (struct sockaddr *)&addr, addrlen) < 0) {
        if (timeout > 0) {
            isConnected = false;
            fd_set fdWrite;
            FD_ZERO(&fdWrite);
            FD_SET(sockfd, &fdWrite);
            struct timeval tv = { timeout / 1000, timeout % 1000 * 1000};
            select(sockfd + 1, NULL, &fdWrite, NULL, &tv);
            if (FD_ISSET(sockfd, &fdWrite)) {
                isConnected = true;
            }
        } else {
            isConnected = false;
        }
    }

    return isConnected;
}

std::string Socket::getLocalIp()
{
    int sockfd = 0;
    char buf[512] = {0};
    struct ifconf ifconf;
    struct ifreq *ifreq;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        close(sockfd);
        return "0.0.0.0";
    }

    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;

    if (ioctl(sockfd, SIOCGIFCONF, &ifconf) < 0) {
        close(sockfd);
        return "0.0.0.0";
    }

    close(sockfd);

    ifreq = (struct ifreq *)ifconf.ifc_buf;
    for (int i = (ifconf.ifc_len / sizeof(struct ifreq)); i > 0; i--) {
        if (ifreq->ifr_flags == AF_INET) {
            if (strcmp(ifreq->ifr_name, "lo") != 0) {
                return inet_ntoa(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr);
            }
            ifreq++;
        }
    }

    return "0.0.0.0";
}
