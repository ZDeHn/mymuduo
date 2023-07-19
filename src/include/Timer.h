#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>

namespace muduozdh{

class Timer : noncopyable{

public:

    using TimerCallback = std::function<void()>;

    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb)), expiration_(when), interval_(interval), repeat_(interval > 0.0){}

    void run() const{
        callback_();
    }

    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }

    void restart(Timestamp now);

private:

    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;

};


}