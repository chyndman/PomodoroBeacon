#include "SignalBus.h"

SignalBus::SignalBus()
    : m_sig(0)
{
}

void SignalBus::claim(const unsigned sig)
{
    if (m_sig == sig)
        m_sig = 0;
}

void SignalBus::latch(const unsigned sig)
{
    if (0 == m_sig)
        m_sig = sig;
}

bool SignalBus::probe(const unsigned sig) const
{
    return (m_sig == sig);
}
