#include "EventLoop.h"
#include "Logging.h"
#include "Poller.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

using namespace muduozdh;

__thread EventLoop *t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;


int createEventfd(){
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    
    if (evtfd < 0){

        LOG << "EventFd error " << errno;
        exit(1);
    }

    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false), quit_(false), callingPendingFunctors_(false), threadId_(CurrentThread::tid()), poller_(Poller::newDefaultPoller(this))
    , wakeupFd_(createEventfd()), wakeupChannel_(new Channel(this, wakeupFd_)), timerQueue_(new TimerQueue(this)){


    LOG << "EventLoop Created in thread " << threadId_;
    
    if (t_loopInThisThread){

        LOG << "Another EventLoop exists in this Thread "<< threadId_;
    }
    else{
        t_loopInThisThread = this;
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop(){

    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

// 开启事件循环
void EventLoop::loop(){

    looping_ = true;
    quit_ = false;

    LOG << "EventLoop start Looping"; 

    while(!quit_){

        activeChannels_.clear();

        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

        for (Channel *channel : activeChannels_){
            channel->handleEvent(pollReturnTime_);
        }
        
        doPendingFunctors();
    }

    LOG << "EventLoop stop Looping";

    looping_ = false;
}

void EventLoop::quit(){
    
    quit_ = true;

    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb){

    if (isInLoopThread()){
        cb();
    }

    else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb){

    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup(); 
    }
}

void EventLoop::handleRead(){

  uint64_t one = 1;
  ssize_t n = read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one){

    LOG << "EventLoop::hreadleRead() reads " << n << "bytes instead of 8";
  
  }

}

void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one){

        LOG << "EventLoop::wakeup() writes " << n << "bytes instead of 8";
    
    }
}

void EventLoop::updateChannel(Channel *channel){
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel){
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel){
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors(){

    std::vector<Functor> functors;
    
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor &functor : functors){
        functor();
    }

    callingPendingFunctors_ = false;
}