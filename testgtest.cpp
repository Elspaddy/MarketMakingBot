#include "MarketMakingBot.h"

#include <gtest/gtest.h>

TEST(MarketMaking, bid_order)
{
    MarketMakingBot market_making_bot;
    bool run_forever = false;
    const std::chrono::milliseconds delay = 0s;
    unsigned int report_every = 1;
    market_making_bot.Run(run_forever, delay, report_every);
    double price = market_making_bot.PlaceBidPrice();
    double volume = market_making_bot.PlaceBidVolume();
    MarketMakingBot::OrderID order_id = market_making_bot.PlaceOrder(price, volume);
    bool filled = market_making_bot.CheckBid();

    EXPECT_EQ(filled, false);
}

TEST(MarketMaking, ask_order)
{
    MarketMakingBot market_making_bot;
    bool run_forever = false;
    const std::chrono::milliseconds delay = 0s;
    unsigned int report_every = 1;
    market_making_bot.Run(run_forever, delay, report_every);
    double price = market_making_bot.PlaceAskPrice();
    double volume = market_making_bot.PlaceAskVolume();
    MarketMakingBot::OrderID order_id = market_making_bot.PlaceOrder(price, volume);
    bool filled = market_making_bot.CheckAsk();

    EXPECT_EQ(filled, false);
}
