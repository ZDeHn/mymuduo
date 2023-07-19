#include "Poller.h"
#include "EpollPoller.h"

#include <stdlib.h>

using namespace muduozdh;

Poller* Poller::newDefaultPoller(EventLoop *loop){
    
    return new EPollPoller(loop);
}
