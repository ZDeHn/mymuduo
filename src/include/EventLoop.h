#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include "TimerQueue.h"

namespace muduozdh{

class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }
    
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    void wakeup();

    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    bool isInLoopThread() const { return threadId_ ==  CurrentThread::tid(); }

    void runAt(Timestamp timestamp, Functor&& cb){
        timerQueue_->addTimer(std::move(cb), timestamp, 0.0);
    }

    void runAfter(double waitTime, Functor&&cb){
        Timestamp time(addTime(Timestamp::now(), waitTime));
        runAt(time, std::move(cb));
    }

    void runEvery(double interval, Functor&&cb){
        Timestamp timestamp(addTime(Timestamp::now(), interval));
        timerQueue_->addTimer(std::move(cb), timestamp, interval);
    }

private:

    using ChannelList = std::vector<Channel*>;

    void handleRead(); 
    void doPendingFunctors(); 

    std::atomic_bool looping_;  
    std::atomic_bool quit_; 
    
    const pid_t threadId_; 

    Timestamp pollReturnTime_; 
    std::unique_ptr<Poller> poller_;

    std::unique_ptr<TimerQueue> timerQueue_;

    int wakeupFd_; 
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctors_; 
    std::vector<Functor> pendingFunctors_; 
    std::mutex mutex_; 

};

}