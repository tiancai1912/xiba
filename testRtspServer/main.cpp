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
#include "net/socketconnection.h"
#include "mp4file.h"
#include "base/mytimer.h"
#include "base/blockqueue.h"
#include "task.h"

#include <thread>
#include <unistd.h>

#include "base/threadpool.h"

#include "getopt.h"
#include <regex>
#include <string>

#include <poll.h>

#include "3rd/eventscheduler.h"

#include "net/tcpserver.h"
#include "server.h"

#define SERVER_PORT     8554
#define SERVER_RTP_PORT  55532
#define SERVER_RTCP_PORT 55533
#define BUF_MAX_SIZE    (1024*1024)

rtp g_rtp;
Mp4File g_file;

Task *g_task[16];

static struct option long_options[] = {
    {"directory", 1, 0, 'd'},
    {"listen", 2, 0, 'l'},
    {"port", 2, 0, 'p'},
    {"help", 0, 0, 'h'},
};

static char optstring[] = "d:l::p::h";


static int handleCmd_OPTIONS(char* result, int cseq)
{
    sprintf(result, "RTSP/1.0 200 ok\r\n"
                    "CSeq: %d\r\n"
                    "Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
                    "\r\n",
                    cseq);

    return 0;
}

//static int handleCmd_OPTIONS(char* result, int cseq)
//{
//    sprintf(result, "RTSP/1.0 400 Bad Request\r\n"
//                    "CSeq: %d\r\n"
//                    "\r\n",
//                    cseq);

//    return 0;
//}


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

struct CmdParams {
    char fileName[256];
    char ip[256];
    int port;

    CmdParams() {
        memset(fileName, 0, sizeof(char) * 256);
        memset(ip, 0, sizeof(char) * 256);
        port = -1;
    }
};

void handleTaskCallback(void *arg)
{
    TaskParams *params = (TaskParams *)arg;
    doClient(params->clientSockfd, params->clientIp, params->clientPor, params->serverRtpSockfd, params->serverRtpSockfd, params->task);
}

static bool checkRtspUrl(char *str)
{
    std::string pattern = "rtsp:\/\/\\d{1,3}.\\d{1,3}.\\d{1,3}.\\d{1,3}";
    std::regex express(pattern);

    bool ret = std::regex_match(str, express);
    return ret;
}

static int getPort(char *str)
{
    int port = std::stoi(str);
    return port;
}

static const char *getFileName(char *dir)
{
    std::string path(dir);
    int pos = path.find_last_of('/');

    std::string fileName = path.substr(pos + 1);
    return fileName.c_str();
}

struct OptResult
{
    int port;
    char ip[128];
    char fileName[128];

    OptResult() {
        port = -1;
        memset(ip, 0, 128);
        memset(fileName, 0, 128);
    }
};

static CmdParams parseOptions(int argc, char **argv)
{
    char *str;
    int long_index;
    int c;

    struct CmdParams params;

    while((c = getopt_long(argc, argv, optstring, long_options, &long_index)) != EOF) {
        switch (c) {
        case 'd':
            str = optarg;
            strcpy(params.fileName, getFileName(str));
            break;
        case 'l':
            str = optarg;
            if (checkRtspUrl(str) == true) {
                strcpy(params.ip, str);
            }
            break;
        case 'p':
            str = optarg;
            params.port = getPort(str);
            break;
        case 'h':
            printf("the help: %s\n", str);
            break;
        }
    }

    return params;
}

class TcpClient
{
public:
    TcpClient(char *ip, int port);
    ~TcpClient();

    int connect();
private:
    void bind();
};

//class TcpServer
//{
//public:
//    TcpServer(const char *ip, int port);
//    ~TcpServer();

//    int accept(char *ip, int *port);
//private:
//    int create();
//    bool bind();
//    void listen();

//public:
//    SocketConnection conn;
//    int mFd;
//    char *mIp;
//    int mPort;
//};

//TcpServer::TcpServer(const char *ip, int port)
//{
//    mIp = (char *)malloc(256);
//    memset(mIp, 0, sizeof(char) * 256);
//    strcpy(mIp, ip);

//    mPort = port;
//    mFd = create();

//    bind();
//    listen();
//}

//int TcpServer::create()
//{
//    int sockfd;
//    int on = 1;

//    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    if(sockfd < 0)
//        return -1;

//    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

//    return sockfd;
//}

//TcpServer::~TcpServer()
//{
//}

//int TcpServer::accept(char *ip, int *port)
//{
//    int clientfd;
//    socklen_t len = 0;
//    struct sockaddr_in addr;

//    memset(&addr, 0, sizeof(addr));
//    len = sizeof(addr);

//    clientfd = ::accept(mFd, (struct sockaddr *)&addr, &len);
//    if(clientfd < 0)
//        return -1;

//    strcpy(ip, inet_ntoa(addr.sin_addr));
//    *port = ntohs(addr.sin_port);

//    return clientfd;
//}

//bool TcpServer::bind()
//{
//    struct sockaddr_in addr;

//    addr.sin_family = AF_INET;
//    addr.sin_port = htons(mPort);
//    addr.sin_addr.s_addr = inet_addr(mIp);

//    if(::bind(mFd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
//        return false;

//    return true;
//}

//void TcpServer::listen()
//{
//    ::listen(mFd, 10);
//}

class UdpSocket
{
public:
  UdpSocket();
  ~UdpSocket();

  int fd();
  int bind(char *ip, int port);

private:
  int create();

private:
  int mFd;
  char *mIp;
  int mPort;
};

UdpSocket::UdpSocket()
{
    mFd = create();
}

UdpSocket::~UdpSocket()
{

}

int UdpSocket::fd()
{
    return mFd;
}

int UdpSocket::bind(char *ip, int port)
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if(::bind(mFd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
        return -1;

    return 0;
}

int UdpSocket::create()
{
    int sockfd;
    int on = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        return -1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
    return sockfd;
}

int main(int argc, char *argv[])
{
    struct CmdParams params = parseOptions(argc, argv);
    printf("the directory : %s\n", params.fileName);
    printf("matche result: %s\n", params.ip);
    printf("the port : %d\n", params.port);

    int serverRtpSockfd, serverRtcpSockfd;

    SocketConnection conn;
//    TcpServer server("0.0.0.0", SERVER_PORT);

    UdpSocket serverRtp;
    UdpSocket serverRtcp;
    serverRtpSockfd = serverRtp.fd();
    serverRtcpSockfd = serverRtcp.fd();
    if(serverRtpSockfd < 0 || serverRtcpSockfd < 0)
    {
        printf("failed to create udp socket\n");
        return -1;
    }

    g_serverRtpSockfd = serverRtpSockfd;

    if (serverRtp.bind("0,0,0,0", SERVER_RTP_PORT) < 0 ||
            serverRtcp.bind("0.0.0.0", SERVER_RTCP_PORT) < 0)
    {
        printf("failed to bind addr\n");
        return -1;
    }

    ThreadPool pool(3);

    Server server("0.0.0.0", 8554);
    server.start();

//    std::vector<struct pollfd> plist;

//    struct pollfd pp;
//    pp.fd = server.mFd;
//    pp.events = 0;
//    pp.events |= POLLIN;
//    pp.revents = 0;

//    plist.push_back(pp);




//    while(1)
//    {
//        int clientSockfd;
//        char clientIp[40];
//        int clientPort;

//        int nums = poll(&*plist.begin(), 1, 10000);
//        if (nums > 0) {
//            clientSockfd = server.accept(clientIp, &clientPort);
//            if(clientSockfd < 0)
//            {
//                printf("failed to accept client\n");
//                return -1;
//            }

//            printf("accept client;client ip:%s,client port:%d\n", clientIp, clientPort);

//            TaskParams *taskParams = new TaskParams{clientSockfd, clientIp, clientPort, serverRtpSockfd, serverRtcpSockfd, g_task[0]};
//            ThreadPool::Task task1;
//            task1.setTaskCallback(handleTaskCallback, (void *)taskParams);
//            pool.addTask(task1);
//        }
//    }

    getchar();
    return 0;
}

//{

//    EventScheduler *scheduler = EventScheduler::createNew(EventScheduler::PollerType::POLLER_POLL);
//    TimerEvent *event = TimerEvent::createNew(NULL);
//    event->setTimeoutCallback(callback1);
//    scheduler->addTimedEventRunEvery(event, 1000);

//    TimerEvent *event2 = TimerEvent::createNew(NULL);
//    event2->setTimeoutCallback(callback2);
//    scheduler->addTimedEventRunEvery(event2, 5000);

//    scheduler->loop();

//}
