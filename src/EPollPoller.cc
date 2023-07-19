#include "EpollPoller.h"
#include "Logging.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>
#include <strings.h>

using namespace muduozdh;

const int kNew = -1;  
const int kAdded = 1;
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize) {
    if (epollfd_ < 0){
        LOG << "epoll_create error ";
        exit(1);
    }
}

EPollPoller::~EPollPoller() {
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels){


    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;

    Timestamp now(Timestamp::now());

    if (numEvents > 0){

        LOG << numEvents << " events happend";

        fillActiveChannels(numEvents, activeChannels);

        if (numEvents == events_.size()){
            events_.resize(events_.size() * 2);
        }
    
    }
    else if (numEvents == 0){
;
        LOG << "Epoll Timeout!" ;
    }
    else{
        
        if (saveErrno != EINTR){
            errno = saveErrno;

            LOG << "EpollPoller::poll() error!" ;
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();

    LOG << "fd: "<< channel->fd() << " event: " << channel->events();

    if (index == kNew || index == kDeleted){

        if (index == kNew){

            int fd = channel->fd();
            channels_[fd] = channel;
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }

    else {

        int fd = channel->fd();
        if (channel->isNoneEvent()){

            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else{

            update(EPOLL_CTL_MOD, channel);
        }
    
    }
}

void EPollPoller::removeChannel(Channel *channel) 
{
    int fd = channel->fd();
    channels_.erase(fd);
    
    LOG << "fd:" << fd;

    int index = channel->index();

    if (index == kAdded){
        update(EPOLL_CTL_DEL, channel);
    }
    
    channel->set_index(kNew);
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const{

    for (int i=0; i < numEvents; ++i){

        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);

        activeChannels->push_back(channel); 
    }
}

void EPollPoller::update(int operation, Channel *channel){

    epoll_event event;
    bzero(&event, sizeof event);
    
    int fd = channel->fd();

    event.events = channel->events();
    event.data.fd = fd; 
    event.data.ptr = channel;
    
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL){
            
            LOG << "epoll_ctl del error ";
        }
        else{

            LOG << "epoll_ctl add/mod error";
            exit(1);
        }
    }
}