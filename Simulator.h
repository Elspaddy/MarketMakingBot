#pragma once

#include <memory>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "DvFSimulator.h"

class Simulator
{
public:
    using OrderID = IDvfSimulator::OrderID;
    using OrderBook = IDvfSimulator::OrderBook;

    Simulator() : simulator_ptr{ DvfSimulator::Create() }, m_bb{ 200.0 }, m_ba{ 210.0 }
    {
	}

    void Refresh()
    {
        std::ofstream out("std_cout.txt");
        std::streambuf *coutbuf = std::cout.rdbuf();
        std::cout.rdbuf(out.rdbuf());

        const OrderBook order_book = simulator_ptr->GetOrderBook();

        std::cout.rdbuf(coutbuf);
        std::ifstream file("std_cout.txt");

        m_order_book_bids.clear();
        std::copy_if(order_book.begin(), order_book.end(), std::back_inserter(m_order_book_bids),
                [](const auto& element)
                    {
                        const auto& [price, volume] = element;
                        return volume > 0;
                    }
            );
        m_order_book_asks.clear();
        std::copy_if(order_book.begin(), order_book.end(), std::back_inserter(m_order_book_asks),
                [](const auto& element)
                    {
                        const auto& [price, volume] = element;
                        return volume < 0;
                    }
            );

        std::string textline;
        while (std::getline(file, textline))
        {
            std::istringstream textstream(textline);
            std::string text;
            textstream >> text;
            if (text == "BB")
            {
                textstream >> m_bb;
                textstream >> text >> m_ba;
            }
            else if (text == "Filled")
            {
                std::string fill_type;
                textstream >> fill_type;
                OrderID fill_order_id;
                textstream >> text >> text >> fill_order_id;
                double fill_price;
                textstream >> text >> fill_price;
                double fill_volume;
                textstream >> text >> fill_volume;
                if (fill_type == "BID")
                {
                    m_filled_bids.push_back(fill_order_id);
                }
                else
                {
                    m_filled_asks.push_back(fill_order_id);
                }
            }
            std::cout << textline << std::endl;
        }
    }

    void PrintOrderBook()
    {
        for (const auto&[price, volume] : m_order_book_bids)
        {
            std::cout << "Bid: price=" << price << ", volume=" << volume << "\n";
        }
        for (const auto&[price, volume] : m_order_book_asks)
        {
            std::cout << "Ask: price=" << price << ", volume=" << volume << "\n";
        }
        std::cout << "----------------------------------------\n";
    }

    double BB()
    {
        return m_bb;
    }

    double BA()
    {
        return m_ba;
    }

    double HighestBidOnBook()
    {
        double price{ 0 };
        if (m_order_book_bids.begin() != m_order_book_bids.end())
        {
            double volume;
            std::tie(price, volume) = *std::max_element(m_order_book_bids.begin(), m_order_book_bids.end(), 
                    [](const auto& lhs, const auto& rhs)
                        {
                            const auto& [lhs_price, lhs_volume] = lhs;
                            const auto& [rhs_price, rhs_volume] = rhs;
                            return lhs_price < rhs_price;
                        }
                );
        }
        return price;
    }

    double LowestBidOnBook()
    {
        double price{ 0 };
        if (m_order_book_bids.begin() != m_order_book_bids.end())
        {
            double volume;
            std::tie(price, volume) = *std::min_element(m_order_book_bids.begin(), m_order_book_bids.end(), 
                    [](const auto& lhs, const auto& rhs)
                        {
                            const auto& [lhs_price, lhs_volume] = lhs;
                            const auto& [rhs_price, rhs_volume] = rhs;
                            return lhs_price < rhs_price;
                        }
                );
        }
        return price;
    }

    double LowestAskOnBook()
    {
        double price{ 0 };
        if (m_order_book_asks.begin() != m_order_book_asks.end())
        {
            double volume;
            std::tie(price, volume) = *std::min_element(m_order_book_asks.begin(), m_order_book_asks.end(), 
                    [](const auto& lhs, const auto& rhs)
                        {
                            const auto& [lhs_price, lhs_volume] = lhs;
                            const auto& [rhs_price, rhs_volume] = rhs;
                            return lhs_price < rhs_price;
                        }
                );
        }
        return price;
    }

    double HighestAskOnBook()
    {
        double price{ 0 };
        if (m_order_book_asks.begin() != m_order_book_asks.end())
        {
            double volume;
            std::tie(price, volume) = *std::max_element(m_order_book_asks.begin(), m_order_book_asks.end(), 
                    [](const auto& lhs, const auto& rhs)
                        {
                            const auto& [lhs_price, lhs_volume] = lhs;
                            const auto& [rhs_price, rhs_volume] = rhs;
                            return lhs_price < rhs_price;
                        }
                );
        }
        return price;
    }

    OrderID PlaceOrder(double price, double volume)
    {
        OrderID order_id;
        if (auto order_id_opt = simulator_ptr->PlaceOrder(price, volume); order_id_opt)
        {
            order_id = order_id_opt.value();
        }
        else
        {
            order_id = 0;
        }
        return order_id;
    }

    bool CancelOrder(OrderID order_id)
    {
        return simulator_ptr->CancelOrder(order_id);
    }

    auto BidsOnBook(double bid_price, double bid_volume)
    {
        return std::count_if(m_order_book_bids.begin(), m_order_book_bids.end(),
                [bid_price, bid_volume](const auto& element)
                {
                    const auto& [price, volume] = element;
                    return price == bid_price && volume == bid_volume; 
                }
            );
    }

    auto AsksOnBook(double ask_price, double ask_volume)
    {
        return std::count_if(m_order_book_asks.begin(), m_order_book_asks.end(),
                [ask_price, ask_volume](const auto& element)
                {
                    const auto& [price, volume] = element;
                    return price == ask_price && volume == ask_volume; 
                }
            );
    }

    bool CheckBidFill(OrderID order_id)
    {
        auto it = std::find(m_filled_bids.begin(), m_filled_bids.end(), order_id);
        if (it != m_filled_bids.end())
        {
            m_filled_bids.erase(it);
            return true;
        }
        return false;
    }

    bool CheckAskFill(OrderID order_id)
    {
        auto it = std::find(m_filled_asks.begin(), m_filled_asks.end(), order_id);
        if (it != m_filled_asks.end())
        {
            m_filled_asks.erase(it);
            return true;
        }
        return false;
    }

private:
    struct DvfSimulatorDeleter
    {
        void operator()(IDvfSimulator* simulator_ptr)
        {
            DvfSimulator* derived_ptr = dynamic_cast<DvfSimulator*>(simulator_ptr);
            delete derived_ptr;
        }
    };
    std::unique_ptr<IDvfSimulator, DvfSimulatorDeleter> simulator_ptr;
    OrderBook m_order_book_bids;
    OrderBook m_order_book_asks;
    std::vector<OrderID> m_filled_bids;
    std::vector<OrderID> m_filled_asks;
    OrderBook filled_asks;
    double m_bb;
    double m_ba;
};

