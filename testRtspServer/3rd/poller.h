#ifndef POLLER_H
#define POLLER_H

#include "ioevent.h"
#include <map>

class Poller
{
public:

    virtual ~Poller();
    virtual bool addIoEvent(IOEvent *event) = 0;
    virtual bool updateIoEvent(IOEvent *event) = 0;
    virtual bool removeIoEvent(IOEvent *event) = 0;
    virtual void handleEvent() = 0;

protected:
    Poller();

    typedef std::map<int, IOEvent *> IOEventMap;
    IOEventMap mEventMap;

};

#endif // POLLER_H
