#include <QCoreApplication>
#include <QDebug>


/*
 * 作者：zhangyu
 */

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

#include "rtp.h"

#include "socketconnection.h"

#include "mp4file.h"

#define H264_FILE_NAME  "test.h264"
#define AAC_FILE_NAME "test.aac"

#define SERVER_PORT     8554
#define SERVER_RTP_PORT  55532
#define SERVER_RTCP_PORT 55533
#define BUF_MAX_SIZE    (1024*1024)

rtp g_rtp;
Mp4File g_file;

static int handleCmd_OPTIONS(char* result, int cseq)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
                    "\r\n",
                    cseq);

    return 0;
}

//static int handleCmd_DESCRIBE(char* result, int cseq, char* url)
//{
//    char sdp[500];
//    char localIp[100];

//    sscanf(url, "rtsp://%[^:]:", localIp);

//    sprintf(sdp, "v=0\r\n"
//                 "o=- 9%ld 1 IN IP4 %s\r\n"
//                 "t=0 0\r\n"
//                 "a=control:*\r\n"
//                 "m=video 0 RTP/AVP 96\r\n"
//                 "a=rtpmap:96 H264/90000\r\n"
//                 "a=control:track0\r\n"
//                 "m=audio 0 RTP/AVP 97\r\n"
//                 "a=rtpmap:97 mpeg4-generic/44100/2\r\n"
//                 "a=fmtp:97 sizeLength=13;\r\n"
//                 "a=control:track1\r\n",
//            time(NULL), localIp);

//    sprintf(result, "RTSP/1.0 200 OK\r\nCSeq: %d\r\n"
//                    "Content-Base: %s\r\n"
//                    "Content-type: application/sdp\r\n"
//                    "Content-length: %d\r\n\r\n"
//                    "%s",
//                    cseq,
//                    url,
//                    strlen(sdp),
//                    sdp);

//    return 0;
//}

static int handleCmd_DESCRIBE(char* result, int cseq, char* url)
{
    char sdp_video[500];
    char sdp_audio[500];
    char localIp[100];

    sscanf(url, "rtsp://%[^:]:", localIp);

    sprintf(sdp_video, "v=0\r\n"
                 "o=- 9%ld 1 IN IP4 %s\r\n"
                 "t=0 0\r\n"
                 "a=control:*\r\n"
                 "a=type:broadcast\r\n"
                       "m=audio 0 RTP/AVP 97\r\n"
                       "c=IN IP4 127.0.0.1\r\n"
                      "a=rtpmap:97 mpeg4-generic/44100/2\r\n"
                      "a=fmtp:97 SizeLength=13;\r\n"
                      "a=control:track1\r\n",
//                 "m=video 0 RTP/AVP 96\r\n"
//                 "c=IN IP4 127.0.0.1\r\n"
//                 "a=rtpmap:96 H264/90000\r\n"
//                 "a=framerate:25"
//                 "a=control:track0\r\n",
                 time(NULL), localIp);



    sprintf(result, "RTSP/1.0 200 OK\r\nCSeq: %d\r\n"
                    "Content-Base: %s\r\n"
                    "Content-type: application/sdp\r\n"
                    "Content-length: %d\r\n\r\n"
                    "%s",
                    cseq,
                    url,
                    strlen(sdp_video),
                    sdp_video);

    return 0;
}

static int handleCmd_SETUP(char* result, int cseq, int clientRtpPort)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n"
                    "Session: 66334873\r\n"
                    "\r\n",
                    cseq,
                    clientRtpPort,
                    clientRtpPort+1,
                    SERVER_RTP_PORT,
                    SERVER_RTCP_PORT);

    return 0;
}

static int handleCmd_PLAY(char* result, int cseq)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Range: npt=0.000-\r\n"
                    "Session: 66334873; timeout=60\r\n\r\n",
                    cseq);

    return 0;
}

static void doClient(int clientSockfd, const char* clientIP, int clientPort,
                        int serverRtpSockfd, int serverRtcpSockfd)
{
    char method[40];
    char url[100];
    char version[40];
    int cseq;
    int clientRtpPort, clientRtcpPort;
    char *bufPtr;
    char* rBuf = (char *)malloc(BUF_MAX_SIZE);
    char* sBuf = (char *)malloc(BUF_MAX_SIZE);
    char line[400];

    while(1)
    {
        int recvLen;

        recvLen = recv(clientSockfd, rBuf, BUF_MAX_SIZE, 0);
        if(recvLen <= 0)
            goto out;

        rBuf[recvLen] = '\0';
        printf("---------------C->S--------------\n");
        printf("%s", rBuf);

        /* 解析方法 */
        bufPtr = SocketConnection::getLineFromBuf(rBuf, line);
        if(sscanf(line, "%s %s %s\r\n", method, url, version) != 3)
        {
            printf("parse err\n");
            goto out;
        }

        /* 解析序列号 */
        bufPtr = SocketConnection::getLineFromBuf(bufPtr, line);
        if(sscanf(line, "CSeq: %d\r\n", &cseq) != 1)
        {
            printf("parse err\n");
            goto out;
        }

        /* 如果是SETUP，那么就再解析client_port */
        if(!strcmp(method, "SETUP"))
        {
            while(1)
            {
                bufPtr = SocketConnection::getLineFromBuf(bufPtr, line);
                if(!strncmp(line, "Transport:", strlen("Transport:")))
                {
                    sscanf(line, "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n",
                                    &clientRtpPort, &clientRtcpPort);
                    break;
                }
            }
        }

        if(!strcmp(method, "OPTIONS"))
        {
            if(handleCmd_OPTIONS(sBuf, cseq))
            {
                printf("failed to handle options\n");
                goto out;
            }
        }
        else if(!strcmp(method, "DESCRIBE"))
        {
            if(handleCmd_DESCRIBE(sBuf, cseq, url))
            {
                printf("failed to handle describe\n");
                goto out;
            }
        }
        else if(!strcmp(method, "SETUP"))
        {
            if(handleCmd_SETUP(sBuf, cseq, clientRtpPort))
            {
                printf("failed to handle setup\n");
                goto out;
            }
        }
        else if(!strcmp(method, "PLAY"))
        {
            if(handleCmd_PLAY(sBuf, cseq))
            {
                printf("failed to handle play\n");
                goto out;
            }
        }
        else
        {
            goto out;
        }

        printf("---------------S->C--------------\n");
        printf("%s", sBuf);
        send(clientSockfd, sBuf, strlen(sBuf), 0);

        /* 开始播放，发送RTP包 */
        if(!strcmp(method, "PLAY"))
        {
            int frameSize, startCode;
            struct RtpPacket* videoRtpPacket = (struct RtpPacket*)malloc(500000);
            g_rtp.rtpHeaderInit(videoRtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_H264, 0,
                          0, 0, 0x88923423);

            struct AdtsHeader adtsHeader;
            struct RtpPacket* audioRtpPacket = (struct RtpPacket *)malloc(5000);
            rtp::rtpHeaderInit(audioRtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_AAC, 1, 0, 0, 0x32411);


            printf("start play\n");
            printf("client ip:%s\n", clientIP);
            printf("client port:%d\n", clientRtpPort);

            while (1)
            {
                printf("get one frame\n");

                Mp4File::PacketItem *item = g_file.getOneFrame();
                printf("get one ffmpeg frame\n");
                if (item->type == Mp4File::PacketType::PACKET_VIDEO) {
//                    printf("get one video frame\n");

//                    if(!g_rtp.startCode3(item->data) && !g_rtp.startCode4(item->data))
//                        continue;

//                    if(g_rtp.startCode3(item->data))
//                        startCode = 3;
//                    else
//                        startCode = 4;

//                    item->length -= startCode;
//                    g_rtp.rtpSendH264Frame(serverRtpSockfd, (char *)clientIP, clientRtpPort,
//                                     videoRtpPacket, (unsigned char *)(item->data + startCode), item->length);
////                    g_rtp.rtpSendH264Frame(serverRtpSockfd, (char *)clientIP, clientRtpPort,
////                                     videoRtpPacket, (unsigned char *)(item->data), item->length);
//                    videoRtpPacket->rtpHeader.timestamp += 90000/25;

//                    printf("send video frame\n");

//                    free(item);
//                    item = NULL;

//                    usleep(1000*1000/25);
                } else if (item->type == Mp4File::PacketType::PACKET_AUDIO) {
                    printf("get one audio frame\n");
                    g_rtp.rtpSendAACFrame(serverRtpSockfd, clientIP, clientRtpPort,
                                    audioRtpPacket, (unsigned char *)(item->data), item->length);

                    usleep(23000);
                }
            }

            free(videoRtpPacket);
            free(audioRtpPacket);
            goto out;
        }
//        /* 开始播放，发送RTP包 */
//        if(!strcmp(method, "PLAY"))
//        {
//            struct AdtsHeader adtsHeader;
//            struct RtpPacket* rtpPacket;
//            uint8_t* frame;
//            int ret;

//            int fd = open(AAC_FILE_NAME, O_RDONLY);
//            if(fd < 0)
//            {
//                printf("failed to open %s\n", AAC_FILE_NAME);
//                goto out;
//            }

//            frame = (uint8_t*)malloc(5000);
//            rtpPacket = (struct RtpPacket *)malloc(5000);

//            g_rtp.rtpHeaderInit(rtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_AAC, 1, 0, 0, 0x32411);

//            while(1)
//            {
//                printf("read one audio frame\n");

//                ret = read(fd, frame, 7);
//                if(ret <= 0)
//                {
//                    printf("break\n");
//                    break;
//                }

//                if(g_rtp.parseAdtsHeader(frame, &adtsHeader) < 0)
//                {
//                    printf("parse err\n");
//                    break;
//                }

//                ret = read(fd, frame, adtsHeader.aacFrameLength-7);
//                if(ret < 0)
//                {
//                    printf("read err\n");
//                    break;
//                }

//                g_rtp.rtpSendAACFrame(serverRtpSockfd, clientIP, clientRtpPort,
//                                rtpPacket, frame, adtsHeader.aacFrameLength-7);

//                usleep(23000);
//            }

//            free(frame);
//            free(rtpPacket);
//            goto out;
//        }



    }
out:
    close(clientSockfd);
    free(rBuf);
    free(sBuf);
}

int main(int argc, char *argv[])
{

    int serverSockfd;
    int serverRtpSockfd, serverRtcpSockfd;
    int ret;

    SocketConnection conn;
    g_file.openFile("test.mp4");

    serverSockfd = conn.createTcpSocket();
    if(serverSockfd < 0)
    {
        printf("failed to create tcp socket\n");
        return -1;
    }

    ret = conn.bindSocketAddr(serverSockfd, "0.0.0.0", SERVER_PORT);
    if(ret < 0)
    {
        printf("failed to bind addr\n");
        return -1;
    }

    ret = listen(serverSockfd, 10);
    if(ret < 0)
    {
        printf("failed to listen\n");
        return -1;
    }

    serverRtpSockfd = conn.createUdpSocket();
    serverRtcpSockfd = conn.createUdpSocket();
    if(serverRtpSockfd < 0 || serverRtcpSockfd < 0)
    {
        printf("failed to create udp socket\n");
        return -1;
    }

    if(conn.bindSocketAddr(serverRtpSockfd, "0.0.0.0", SERVER_RTP_PORT) < 0 ||
        conn.bindSocketAddr(serverRtcpSockfd, "0.0.0.0", SERVER_RTCP_PORT) < 0)
    {
        printf("failed to bind addr\n");
        return -1;
    }

    printf("rtsp://127.0.0.1:%d\n", SERVER_PORT);

    while(1)
    {
        int clientSockfd;
        char clientIp[40];
        int clientPort;

        clientSockfd = conn.acceptClient(serverSockfd, clientIp, &clientPort);
        if(clientSockfd < 0)
        {
            printf("failed to accept client\n");
            return -1;
        }

        printf("accept client;client ip:%s,client port:%d\n", clientIp, clientPort);

        doClient(clientSockfd, clientIp, clientPort, serverRtpSockfd, serverRtcpSockfd);
    }

    qDebug() << "hellp rtsp\n";
    g_file.closeFile();
    getchar();
    return 0;
}
