#pragma once

#include <chrono>
#include <thread>

using namespace std::chrono_literals;
class Timer
{
public:
    Timer()
    {
    }

    void Start()
    {
        m_start = std::chrono::steady_clock::now();
    }
    void Wait(const std::chrono::milliseconds delay)
    { 
        std::chrono::time_point<std::chrono::steady_clock> finish{ m_start + delay };
        while (std::chrono::steady_clock::now() < finish)
        {
            std::this_thread::sleep_for(10ms);
        }
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> m_start;
};
