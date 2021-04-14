// Starting point for Deversifi C++ coding challenge
// Please do not modify or distribute this file

// Requires C++ 17 features

#pragma once

#include <optional>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include <set>

class IDvfSimulator
{
public:
    using OrderID = uint32_t;

    // Simulates a call to https://api.deversifi.com/bfx/v2/book/tETHUSD/
    // returning a structure representative of the JSON structure in the form
    // [[price, volume], ...] with +ve volumes representing bids and -ve asks
    using OrderBook = std::vector<std::pair<double, double>>;
    virtual OrderBook GetOrderBook() noexcept = 0;

    // Simulates a POST-only type call to https://api.deversifi.com/v1/trading/w/submitOrder 
    // returning an order ID if place was successful or a std::nullopt if not
    // Note: +ve volumes represent a bid type and -ve ask type order
    virtual std::optional<OrderID> PlaceOrder(double price, double amount) noexcept = 0;

    // Simulates a POST-only type call to https://api.deversifi.com/v1/trading/w/cancelOrder
    // indicating sucess / failure of cancel operation via bool return
    virtual bool CancelOrder(OrderID oid) noexcept = 0;
};

class DvfSimulator : public IDvfSimulator
{
public:
    virtual ~DvfSimulator() noexcept = default;

    static IDvfSimulator* Create() noexcept
    {
        return static_cast<IDvfSimulator*>(new DvfSimulator());
    }

    virtual OrderBook GetOrderBook() noexcept override final
    {
        // Simulate a possible market movement since the last call
        const bool noChange = GetRandom(1, 3) == 1;
        if (!noChange)
        {
            static const auto maxShift = 20;
            const double priceMovement = GetRandom(1, GetRandom(2, maxShift));
            const bool moveLeft = GetRandom(1, 2) == 1 && m_bb > maxShift && m_ba > maxShift;

            if (moveLeft)
            {
                m_bb -= priceMovement;
                m_ba -= priceMovement;
            }
            else
            {
                m_bb += priceMovement;
                m_ba += priceMovement;
            }

            std::cout << "BB " << m_bb << " BA " << m_ba << std::endl;

            FillOrders();
        }

        OrderBook ob;

        std::multiset<Order> placedBids(m_bids);
        std::multiset<Order> placedAsks(m_asks);

        static const auto ordersOnEachSide{ 10 };

        for (uint32_t bids{ ordersOnEachSide }; bids > 0; --bids)
        {
            double price = m_bb - (bids * 5.0) + GetRandom(1, 4);

            while (!placedBids.empty() && price > placedBids.begin()->price)
            {
                auto it = placedBids.begin();
                ob.push_back({ it->price, it->amount });
                placedBids.erase(placedBids.begin());
            }

            ob.push_back({ price, GetRandom(1, 10) * 0.25 });
        }

        for (uint32_t asks{ 0 }; asks < ordersOnEachSide; ++asks)
        {
            double price = m_ba + (asks * 5.0) + GetRandom(1, 4);

            while (!placedAsks.empty() && price < placedAsks.begin()->price)
            {
                auto it = placedAsks.begin();
                ob.push_back({ it->price, it->amount });
                placedAsks.erase(placedAsks.begin());
            }

            ob.push_back({ price, GetRandom(1, 10) * -0.25 });
        }

        return ob;
    }

    virtual std::optional<OrderID> PlaceOrder(double price, double amount) noexcept override final
    {
        if (   (amount > 0.0 && price >= m_ba)
            || (amount < 0.0 && price <= m_bb))
        {
            // Assume order is place-only, since it would be filled it fails to place
            std::cout
                << "Failed to place order id " << std::to_string(m_nextClientOid)
                << " @ " << price
                << " : " << amount
                << " " << (amount > 0.0 ? "BID" : "ASK")
                << " as this would result in an immediate fill!"
                << std::endl;
            return {};
        }

        const bool isBid = amount > 0.0;

        std::cout 
            << "Placed order id " << std::to_string(m_nextClientOid) 
            << " @ " << price 
            << " : " << amount 
            << " " << (isBid ? "BID" : "ASK") << std::endl;

        if (isBid)
        {
            m_bids.insert(Order{ m_nextClientOid, price, amount });
        }
        else
        {
            m_asks.insert(Order{ m_nextClientOid, price, amount });
        }

        return m_nextClientOid++;
    }

    virtual bool CancelOrder(OrderID oid) noexcept override final
    {
        auto checkAndRemoveFn = [&oid](auto& col) -> bool
        {
            for (auto it = col.begin(); it != col.end(); ++it)
            {
                if (it->oid == oid)
                {
                    std::cout
                        << "Cancelled order id " << std::to_string(it->oid)
                        << " @ " << it->price
                        << " : " << it->amount
                        << " " << (it->amount > 0 ? "BID" : "ASK") << std::endl;
                    col.erase(it);
                    return true;
                }
            }

            return false;
        };

        if (   checkAndRemoveFn(m_bids)
            || checkAndRemoveFn(m_asks))
        {
            return true;
        }

        return false;
    }

private:

    DvfSimulator() noexcept
    {
        srand(static_cast<uint32_t>(time(nullptr)));
    }

    uint32_t GetRandom(uint32_t start, uint32_t end)
    {
        return rand() % (end - start + 1) + start;
    }

    void FillOrders()
    {
        while(!m_bids.empty() && m_bids.begin()->price > m_bb)
        {
            auto it = m_bids.begin();
            std::cout
                << "Filled BID order id " << std::to_string(it->oid)
                << " @ " << it->price
                << " : " << it->amount 
                << std::endl;

            m_bids.erase(it);
        }

        while (!m_asks.empty() && m_asks.begin()->price < m_ba)
        {
            auto it = m_asks.begin();
            std::cout
                << "Filled ASK order id " << std::to_string(it->oid)
                << " @ " << it->price
                << " : " << it->amount
                << std::endl;

            m_asks.erase(it);
        }
    }

    OrderID m_nextClientOid{ static_cast<OrderID>(time(nullptr)) };

    double m_bb{ 200.0 };
    double m_ba{ 210.0 };

    struct Order
    {
        const OrderID oid;
        const double price;
        const double amount;

        bool operator < (const Order& rhs) const
        {
            return price < rhs.price;
        }
    };

    std::multiset<Order> m_bids;
    std::multiset<Order> m_asks;
};
