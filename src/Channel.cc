#include "Channel.h"
#include "Logging.h"
#include "EventLoop.h"

#include <sys/epoll.h>

using namespace muduozdh;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;


Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) { }

Channel::~Channel(){}

void Channel::tie(const std::shared_ptr<void> &obj){
    
    tie_ = obj;
    tied_ = true;

}

void Channel::update(){
    loop_->updateChannel(this);
}

void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime){

    if (tied_){

        std::shared_ptr<void> guard = tie_.lock();
        if (guard){

            handleEventWithGuard(receiveTime);
        }
    }
    else{    
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{

    LOG_INFO << "channel handleEvent revents" << revents_;

    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }

    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
        {
            errorCallback_();
        }
    }

    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}
