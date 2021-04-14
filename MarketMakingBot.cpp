#include <iostream>

#include "MarketMakingBot.h"
#include <chrono>

using namespace std::chrono_literals;
int main(int argc, char *argv[])
{
    MarketMakingBot market_making_bot;
    bool run_forever = true;
    const std::chrono::milliseconds delay = 5s;
    unsigned int report_every = 6;
    market_making_bot.Run(run_forever, delay, report_every);

    return 0;
}
