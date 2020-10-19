#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stdlib.h>
#include <algorithm>
#include <assert.h>
#include "net/socketsops.h"

class Buffer
{
public:
    static const int initialSize;
    explicit Buffer() :
        mBufferSize(initialSize),
        mReadIndex(0),
        mWriteIndex(0)

    {
        mBuffer = (char *)malloc(mBufferSize);
    }

    ~Buffer()
    {
        free(mBuffer);
    }

    int readableBytes() const
    {
        return mWriteIndex - mReadIndex;
    }

    int writableBytes() const
    {
        return mBufferSize - mWriteIndex;
    }

    int prependableBytes() const
    {
        return mReadIndex;
    }

    char *peek()
    {
        return begin() + mReadIndex;
    }

    const char *peek() const
    {
        return begin() + mReadIndex;
    }

    const char *findCRLF() const
    {
        const char *crlf = std::search(peek(), beginWrite(), KCRLF, KCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char *findCRLF(const char *start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const char *crlf = std::search(start, beginWrite(), KCRLF, KCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char *findLastCrlf() const
    {
        const char *crlf = std::find_end(peek(), beginWrite(), KCRLF, KCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    void retrieve(int len)
    {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            mReadIndex += len;
        } else {
            retrieveAll();
        }
    }

    void retrieveUntil(const char *end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveAll()
    {
        mReadIndex = 0;
        mWriteIndex = 0;
    }

    char *beginWrite()
    {
        return begin() + mWriteIndex;
    }

    const char *beginWrite() const
    {
        return begin() + mWriteIndex;
    }

    void hasWritten(int len)
    {
        assert(len <= writableBytes());
        mWriteIndex += len;
    }

    void unWrite(int len)
    {
        assert(len <= readableBytes());
        mWriteIndex -= len;
    }

    void ensureWriteableBytes(int len)
    {
        if (writableBytes() < len) {
            makeSpace(len);
        }

        assert(writableBytes() >= len);
    }

    void makeSpace(int len)
    {
        if (writableBytes() + prependableBytes() < len) {
            mBufferSize = mWriteIndex + len;
            mBuffer = (char *)realloc(mBuffer, mBufferSize);
        } else {
            int readable = readableBytes();
            std::copy(begin() + mReadIndex,
                      begin() + mWriteIndex,
                      begin());
            mReadIndex = 0;
            mWriteIndex = mReadIndex + readable;
            assert(readable == readableBytes());
        }
    }

    void append(const char *data, int len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void append(const void *data, int len)
    {
        append((const char *)(data), len);
    }

    int read(int fd);
    int wirte(int fd);


private:
    char *begin()
    {
        return mBuffer;
    }

    const char *begin() const
    {
        return mBuffer;
    }

private:
    char *mBuffer;
    int mBufferSize;
    int mReadIndex;
    int mWriteIndex;

    static const char *KCRLF;


};

#endif // BUFFER_H
