#pragma once

class Assets
{
public:
    Assets(double starting_ethereum, double starting_dollar) : m_ethereum{ starting_ethereum }, m_dollar{ starting_dollar }
    {
    }

    void DeltaEthereum(double delta)
    {
        m_ethereum += delta;
    }

    void DeltaDollars(double delta)
    {
        m_dollar += delta;
    }

    void ShowOverall()
    {
        std::cout << "Ethereum: " << m_ethereum << " Dollar: " << m_dollar << "\n";
    }
private:
    double m_ethereum;
    double m_dollar;
};
