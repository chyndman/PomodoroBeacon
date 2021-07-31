#pragma once

#include <cstdint>

class SignalBus
{
public:
    SignalBus();

    void claim(const unsigned sig);
    void latch(const unsigned sig);
    bool probe(const unsigned sig) const;

private:
    unsigned m_sig;
};
