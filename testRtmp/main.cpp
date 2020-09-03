#include <QCoreApplication>

#include <librtmp/rtmp.h>
#include <librtmp/amf.h>
#include <QDebug>

int main(int argc, char *argv[])
{
//    nalhead_pos=0;
//    m_nFileBufSize=BUFFER_SIZE;
//    m_pFileBuf=(unsigned char*)malloc(BUFFER_SIZE);
//    m_pFileBuf_tmp=(unsigned char*)malloc(BUFFER_SIZE);
//    InitSockets();

    RTMP* m_pRtmp = RTMP_Alloc();
    RTMP_Init(m_pRtmp);
    if (RTMP_SetupURL(m_pRtmp,(char*)("rtmp://10.1.198.100/live/test")) == FALSE)
    {
        RTMP_Free(m_pRtmp);
        return false;
    }

//    RTMP_EnableWrite(m_pRtmp);

    qDebug() << "start connect" <<endl;
    if (RTMP_Connect(m_pRtmp, NULL) == FALSE)
    {
        RTMP_Free(m_pRtmp);
        return false;
    }

    qDebug() << "start connect stream" <<endl;

    if (RTMP_ConnectStream(m_pRtmp,0) == FALSE)
    {
        RTMP_Close(m_pRtmp);
        RTMP_Free(m_pRtmp);
        return false;
    }


    getchar();
    return 0;
}
