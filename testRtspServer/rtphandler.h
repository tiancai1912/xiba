#ifndef RTPHANDLER_H
#define RTPHANDLER_H

#include <stdint.h>


/*
 *
 *    0                   1                   2                   3
 *    7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                           timestamp                           |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |           synchronization source (SSRC) identifier            |
 *   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *   |            contributing source (CSRC) identifiers             |
 *   :                             ....                              :
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

struct RtpHeader
{
    /* byte 0 */
    uint8_t csrcLen:4;
    uint8_t extension:1;
    uint8_t padding:1;
    uint8_t version:2;

    /* byte 1 */
    uint8_t payloadType:7;
    uint8_t marker:1;

    /* bytes 2,3 */
    uint16_t seq;

    /* bytes 4-7 */
    uint32_t timestamp;

    /* bytes 8-11 */
    uint32_t ssrc;
};


struct AdtsHeader
{
    unsigned int syncword;  //12 bit 同步字 '1111 1111 1111'，说明一个ADTS帧的开始
    unsigned int id;        //1 bit MPEG 标示符， 0 for MPEG-4，1 for MPEG-2
    unsigned int layer;     //2 bit 总是'00'
    unsigned int protectionAbsent;  //1 bit 1表示没有crc，0表示有crc
    unsigned int profile;           //1 bit 表示使用哪个级别的AAC
    unsigned int samplingFreqIndex; //4 bit 表示使用的采样频率
    unsigned int privateBit;        //1 bit
    unsigned int channelCfg; //3 bit 表示声道数
    unsigned int originalCopy;         //1 bit
    unsigned int home;                  //1 bit

    /*下面的为改变的参数即每一帧都不同*/
    unsigned int copyrightIdentificationBit;   //1 bit
    unsigned int copyrightIdentificationStart; //1 bit
    unsigned int aacFrameLength;               //13 bit 一个ADTS帧的长度包括ADTS头和AAC原始流
    unsigned int adtsBufferFullness;           //11 bit 0x7FF 说明是码率可变的码流

    /* number_of_raw_data_blocks_in_frame
     * 表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧
     * 所以说number_of_raw_data_blocks_in_frame == 0
     * 表示说ADTS帧中有一个AAC数据块并不是说没有。(一个AAC原始帧包含一段时间内1024个采样及相关数据)
     */
    unsigned int numberOfRawDataBlockInFrame; //2 bit
};

class Packet
{
public:
    Packet()
    {

    }

    ~Packet()
    {

    }

    struct RtpHeader mRtpHeader;
    unsigned char *mPayload;
};

class RtpHandler
{
public:
    RtpHandler();
    ~RtpHandler();

    virtual bool sendPacket(char *buffer, int len) = 0;

protected:
    void initRtpHeader(Packet *rtpPacket,
                       uint8_t csrcLen,
                       uint8_t extension,
                       uint8_t padding,
                       uint8_t version,
                       uint8_t payloadType,
                       uint8_t marker,
                       uint16_t seq,
                       uint32_t timestamp,
                       uint32_t ssrc);

    char * genPacket(char *in);
    int sendRtpPacket(Packet *packet, int len);

private:
    int mSocketFd;
    int mRtpPort;
    char *mIp;
    struct Packet *mRtpPacket;
};

class RtpH264Handler : public RtpHandler
{
public:
    RtpH264Handler () {}
    ~RtpH264Handler() {}

    virtual bool sendPacket(char *buffer, int len);
};

class RtpAACHandler : public RtpHandler
{
public:
    RtpAACHandler () {}
    ~RtpAACHandler() {}

    virtual bool sendPacket(char *buffer, int len);
};

#endif // RTPHANDLER_H
