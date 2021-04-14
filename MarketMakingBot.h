#pragma once

#include <iostream>

#include "Simulator.h"
#include "Assets.h"
#include "Timer.h"

class MarketMakingBot
{
public:
    using OrderID = Simulator::OrderID;
    MarketMakingBot() : m_assets{ 10, 2000 }, m_state{ States::waiting_to_place_orders }
    {
    }

    void Run(bool run_forever, const std::chrono::milliseconds delay, unsigned int asset_display_period)
    {
        Timer timer;
        unsigned int count = 0;
        while (true)
        {
            timer.Start();
            m_simulator.Refresh();
            m_simulator.PrintOrderBook();
            if (run_forever)
            {
                switch (m_state)
                {
                    case States::waiting_to_place_orders:
                        std::cout << "waiting_to_place_orders\n";
                        PlaceOrders();
                        break;
                    case States::waiting_for_bid:
                        std::cout << "waiting_for_bid\n";
                        CheckBid();
                        break;
                    case States::waiting_for_ask:
                        std::cout << "waiting_for_ask\n";
                        CheckAsk();
                        break;
                    case States::orders_placed:
                        std::cout << "orders_placed\n";
                        CheckOrders();
                        break;
                }
                timer.Wait(delay);
                ++count;
                if (count % asset_display_period == 0)
                {
                    m_assets.ShowOverall();
                }
            }
            else
            {
                break;
            }
        }
    }

    double CompleteBidPrice()
    {
        return m_simulator.BB() * 0.99;
    }

    double CompleteAskPrice()
    {
        return m_simulator.BA() * 1.01;
    }

    double PlaceBidPrice()
    {
        return (m_simulator.HighestBidOnBook() + m_simulator.LowestBidOnBook()) / 2;
    }

    double PlaceAskPrice()
    {
        return (m_simulator.HighestAskOnBook() + m_simulator.LowestAskOnBook()) / 2;
    }

    double PlaceBidVolume()
    {
        return 5;
    }

    double PlaceAskVolume()
    {
        return -5;
    }

    Simulator::OrderID PlaceOrder(double price, double volume)
    {
        return m_simulator.PlaceOrder(price, volume);
    }

    void SettleBid()
    {
        m_assets.DeltaDollars(-m_bid_order_volume * m_bid_order_price);
        m_assets.DeltaEthereum(m_bid_order_volume);
    }

    void SettleAsk()
    {
        m_assets.DeltaDollars(-m_ask_order_volume * m_ask_order_price);
        m_assets.DeltaEthereum(m_ask_order_volume);
    }

    void PlaceOrders()
    {
        m_bid_order_price = PlaceBidPrice();
        m_bid_order_volume = PlaceBidVolume();
        m_bid_order_id = PlaceOrder(m_bid_order_price, m_bid_order_volume);
        m_ask_order_price = PlaceAskPrice();
        m_ask_order_volume = PlaceAskVolume();
        m_ask_order_id = PlaceOrder(m_ask_order_price, m_ask_order_volume);
        m_state = States::orders_placed;
    }

    void CheckOrders()
    {
        bool bid_filled = m_simulator.CheckBidFill(m_bid_order_id);
        bool ask_filled = m_simulator.CheckAskFill(m_ask_order_id);
        if (bid_filled && !ask_filled)
        {
            SettleBid();
            CancelAsk();
            m_ask_order_price = CompleteAskPrice();
            m_ask_order_volume = PlaceAskVolume();
            m_ask_order_id = PlaceOrder(m_ask_order_price, m_ask_order_volume);
            m_state = States::waiting_for_ask;
        }
        if (!bid_filled && ask_filled)
        {
            SettleAsk();
            CancelBid();
            m_bid_order_price = CompleteBidPrice();
            m_bid_order_volume = PlaceBidVolume();
            m_bid_order_id = PlaceOrder(m_bid_order_price, m_bid_order_volume);
            m_state = States::waiting_for_bid;
        }
        if (bid_filled && ask_filled)
        {
            SettleBid();
            SettleAsk();
            m_state = States::waiting_to_place_orders;
        }
    }

    bool CheckBid()
    {
        bool bid_filled = m_simulator.CheckBidFill(m_bid_order_id);
        if (bid_filled)
        {
            SettleBid();
            m_state = States::waiting_to_place_orders;
        }
        else
        {
            CancelBid();
            m_bid_order_price = CompleteBidPrice();
            m_bid_order_volume = PlaceBidVolume();
            m_bid_order_id = PlaceOrder(m_bid_order_price, m_bid_order_volume);
        }
        return bid_filled;
    }

    bool CheckAsk()
    {
        bool ask_filled = m_simulator.CheckAskFill(m_ask_order_id);
        if (ask_filled)
        {
            SettleAsk();
            m_state = States::waiting_to_place_orders;
        }
        else
        {
            CancelAsk();
            m_ask_order_price = CompleteAskPrice();
            m_ask_order_volume = PlaceAskVolume();
            m_ask_order_id = PlaceOrder(m_ask_order_price, m_ask_order_volume);
        }
        return ask_filled;
    }

    bool CancelBid()
    {
        bool canceled = m_simulator.CancelOrder(m_bid_order_id);
        if (!canceled)
        {
            std::cout << "Bid Cancel " << m_bid_order_id << " Failed!\n";
        }
        return canceled;
    }

    bool CancelAsk()
    {
        bool canceled = m_simulator.CancelOrder(m_ask_order_id);
        if (!canceled)
        {
            std::cout << "Ask Cancel " << m_ask_order_id << " Failed!\n";
        }
        return canceled;
    }

private:
    Simulator m_simulator;
    Assets m_assets;
    enum class States { waiting_to_place_orders, orders_placed, waiting_for_bid, waiting_for_ask };
    States m_state;
    OrderID m_bid_order_id;
    OrderID m_ask_order_id;
    double m_bid_order_price;
    double m_bid_order_volume;
    double m_ask_order_price;
    double m_ask_order_volume;
};
