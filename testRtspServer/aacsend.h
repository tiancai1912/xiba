#ifndef AACSEND_H
#define AACSEND_H

#include "rtp.h"

class AACSend
{
public:
    AACSend();

    int createTcpSocket();
    int createUdpSocket();
    int bindSocketAddr(int sockfd, const char* ip, int port);
    int parseAdtsHeader(uint8_t* in, struct AdtsHeader* res);

    int rtpSendAACFrame(int socket, const char* ip, int16_t port,
                                struct RtpPacket* rtpPacket, uint8_t* frame, uint32_t frameSize);
    int acceptClient(int sockfd, char* ip, int* port);
    char* getLineFromBuf(char* buf, char* line);
    int handleCmd_OPTIONS(char* result, int cseq);
    int handleCmd_DESCRIBE(char* result, int cseq, char* url);
    int handleCmd_SETUP(char* result, int cseq, int clientRtpPort);
    int handleCmd_PLAY(char* result, int cseq);
    void doClient(int clientSockfd, const char* clientIP, int clientPort,
                            int serverRtpSockfd, int serverRtcppSockfd);
};

#endif // AACSEND_H
