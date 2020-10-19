#ifndef SINK_H
#define SINK_H

#include <map>
#include "stream.h"

class Sink
{
public:
    Sink();
    ~Sink();

    bool addStream(Stream *stream);
    void removeStream(int id);

private:
    std::map<int, Stream> mStreams;

};

#endif // SINK_H
