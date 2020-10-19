#include "rtphandler.h"

RtpHandler::RtpHandler()
{

}

RtpHandler::~RtpHandler()
{

}

void RtpHandler::initRtpHeader(Packet *rtpPacket,
                   uint8_t csrcLen,
                   uint8_t extension,
                   uint8_t padding,
                   uint8_t version,
                   uint8_t payloadType,
                   uint8_t marker,
                   uint16_t seq,
                   uint32_t timestamp,
                   uint32_t ssrc)
{
    rtpPacket->mRtpHeader.csrcLen = csrcLen;
    rtpPacket->mRtpHeader.extension = extension;
    rtpPacket->mRtpHeader.padding = padding;
    rtpPacket->mRtpHeader.version = version;
    rtpPacket->mRtpHeader.payloadType =  payloadType;
    rtpPacket->mRtpHeader.marker = marker;
    rtpPacket->mRtpHeader.seq = seq;
    rtpPacket->mRtpHeader.timestamp = timestamp;
    rtpPacket->mRtpHeader.ssrc = ssrc;
}

char * RtpHandler::genPacket(char *in)
{

}

int RtpHandler::sendRtpPacket(Packet *packet, int len)
{

}

bool RtpH264Handler::sendPacket(char *buffer, int len)
{

}

bool RtpAACHandler::sendPacket(char *buffer, int len)
{

}
