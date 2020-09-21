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
#include "net/rtp.h"
#include "socketconnection.h"
#include "mp4file.h"
#include "base/mytimer.h"
#include "base/blockqueue.h"
#include "task.h"

#include <thread>
#include <unistd.h>

#include "base/threadpool.h"

#define SERVER_PORT     8554
#define SERVER_RTP_PORT  55532
#define SERVER_RTCP_PORT 55533
#define BUF_MAX_SIZE    (1024*1024)

rtp g_rtp;
Mp4File g_file;

Task *g_task[16];

static int handleCmd_OPTIONS(char* result, int cseq)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
                    "\r\n",
                    cseq);

    return 0;
}

BlockQueue g_block_queue;

int g_serverRtpSockfd = -1;
//char g_clientIp[40] = {0};
int g_clientPort = -1;
int g_audio_clientPort = -1;

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
                 "m=video 0 RTP/AVP 96\r\n"
                 "c=IN IP4 127.0.0.1\r\n"
                 "a=rtpmap:96 H264/90000\r\n"
//                 "a=framerate:25"
                 "a=control:track0\r\n"
                 "m=audio 0 RTP/AVP 97\r\n"
                 "c=IN IP4 127.0.0.1\r\n"
                 "a=rtpmap:97 mpeg4-generic/44100/2\r\n"
                 "a=fmtp:97 SizeLength=13;\r\n"
                 "a=control:track1\r\n",
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
                        int serverRtpSockfd, int serverRtcpSockfd, Task *task)
{
    char method[40];
    char url[100];
    char version[40];
    int cseq;
    int clientRtpPort, clientRtcpPort;
    int videoClientRtpPort, videoClientRtcpPort;
    int audioClientRtpPort, audioClientRtcpPort;
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
            if (strstr(url, "track1")) {
                printf("the track: %s", url);
                audioClientRtpPort = clientRtpPort;
                audioClientRtcpPort = clientRtcpPort;
            } else {
                videoClientRtpPort = clientRtpPort;
                videoClientRtcpPort = clientRtcpPort;
            }

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

        if (!strcmp(method, "PLAY")) {
            task = new Task();
            task->start(g_serverRtpSockfd, (char *)clientIP, videoClientRtpPort, audioClientRtpPort);
        }
    }
out:
    task->stop();
    close(clientSockfd);
    free(rBuf);
    free(sBuf);
}

struct TaskParams
{
  int   clientSockfd;
  char *clientIp;
  int clientPor;
  int serverRtpSockfd;
  int serverRtcpSockfd;
  Task *task;
};

void handleTaskCallback(void *arg)
{
    TaskParams *params = (TaskParams *)arg;
    doClient(params->clientSockfd, params->clientIp, params->clientPor, params->serverRtpSockfd, params->serverRtpSockfd, params->task);

}

int main(int argc, char *argv[])
{
    int serverSockfd;
    int serverRtpSockfd, serverRtcpSockfd;
    int ret;

    SocketConnection conn;

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

    g_serverRtpSockfd = serverRtpSockfd;

    if(conn.bindSocketAddr(serverRtpSockfd, "0.0.0.0", SERVER_RTP_PORT) < 0 ||
        conn.bindSocketAddr(serverRtcpSockfd, "0.0.0.0", SERVER_RTCP_PORT) < 0)
    {
        printf("failed to bind addr\n");
        return -1;
    }

    printf("rtsp://127.0.0.1:%d\n", SERVER_PORT);

    ThreadPool pool(3);

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

//        std::thread *tt = new std::thread(doClient, clientSockfd, clientIp, clientPort, serverRtpSockfd, serverRtcpSockfd, g_task[0]);
//        tt->join();

        TaskParams *taskParams = new TaskParams{clientSockfd, clientIp, clientPort, serverRtpSockfd, serverRtcpSockfd, g_task[0]};
        ThreadPool::Task task1;
        task1.setTaskCallback(handleTaskCallback, (void *)taskParams);
        pool.addTask(task1);
//        doClient(clientSockfd, clientIp, clientPort, serverRtpSockfd, serverRtcpSockfd, g_task[0]);
    }

    getchar();
    return 0;
}
