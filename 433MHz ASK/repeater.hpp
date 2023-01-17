#ifndef DEADLINE_TIMER_HPP
#define DEADLINE_TIMER_HPP

#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include <future>
#include <stdlib.h>
#include <stdint.h>

class Repeater
{
public:
    volatile bool running = false;
    uint32_t us;
    std::thread* async_thread;

    template <class callable, class... arguments>
    Repeater(uint32_t us, bool async, bool asynctask, callable&& f, arguments&&... args)
    {        
        this->us = us;
        std::function<
            typename std::result_of<callable(arguments...)>::type()
            > task(
                std::bind(std::forward<callable>(f), 
                std::forward<arguments>(args)...));

        if (async)
        {
            this->running = true;
            this->async_thread = new std::thread([us, task, asynctask, this]() {
                auto us_dura = std::chrono::microseconds(us);
                auto alarm = std::chrono::high_resolution_clock::now() + us_dura;

                while(this->running)
                {
                    std::chrono::microseconds sleep_dt = std::chrono::duration_cast<
                        std::chrono::microseconds>(
                            alarm - std::chrono::high_resolution_clock::now());

                    if (us < 100000)
                    {
                        Repeater::spin_sleep_for(sleep_dt);
                    }
                    else
                    {
                        std::this_thread::sleep_for(sleep_dt);
                    }

                    if (asynctask)
                    {
                        std::thread* t = new std::thread(task);
                        t->detach();
                    }
                    else
                    {
                        task();
                    }

                    alarm += us_dura;
                }
            });
            this->async_thread->detach();
        }
        else
        {
            this->running = true;
            auto us_dura = std::chrono::microseconds(us);
            auto alarm = std::chrono::high_resolution_clock::now() + us_dura;

            while(this->running)
            {
                std::chrono::microseconds sleep_dt = std::chrono::duration_cast<
                        std::chrono::microseconds>(
                            alarm - std::chrono::high_resolution_clock::now());

                // For sleep durations under 1ms, use a more precise spinwait
                if (us < 100000)
                {
                    Repeater::spin_sleep_for(sleep_dt);
                }
                else
                {
                    std::this_thread::sleep_for(sleep_dt);
                }

                if (asynctask)
                {
                    std::thread* t = new std::thread(task);
                    t->detach();
                }
                else
                {
                    task();
                }

                alarm += us_dura;
            }
        }
    }

    ~Repeater()
    {
        this->running = false;
        if (this->async_thread != NULL)
        {
            delete this->async_thread;
        }
    }

    // TODO This is subject to some wild variation in actual sleep time,
    // occasionally up and and including an extra order of magnitude.
    //
    // This needs to be more precise to be able to work with write time divs
    // smaller than 1ms. 
    static void spin_sleep_for(std::chrono::microseconds dura)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = start;
        auto elapsed = end - start;
        while (elapsed < dura)
        {
            end = std::chrono::high_resolution_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        }
    }
};

#endif
