#pragma once
#ifndef __TIMER__
#define __TIMER__
#include<chrono>
using std::chrono::duration;
using std::chrono::time_point;
using std::chrono::steady_clock;

class IntervalTimer {
private:
    long long hit_count;
    duration<double> interval;
    time_point<steady_clock> last;
public:
    IntervalTimer(double _interval):
        hit_count(0LL), last(steady_clock::now())
    {
        interval = duration<double>(_interval);
    }

    template<typename Function>
    inline void Loop(const Function& callback) {
        for (;;) {
            if ((steady_clock::now() - last) >= interval) {
                last = steady_clock::now();
                callback(++hit_count);
            }
        }
    }
};

#endif
