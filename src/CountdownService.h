#pragma once

#include <cstdint>

class SignalBus;

class CountdownService
{
public:
    CountdownService(const unsigned sigExpired);

    void loop(SignalBus& sb);
    void schedule(const unsigned long ms);
    void cancel();
    bool cpuIsExpired() const;

private:
    enum class State
    {
        IDLE,
        EXPIRED,
        SCHEDULED_CPU
    };

    const unsigned m_sigExpired;
    unsigned long m_scheduledAtMs;
    unsigned long m_expireAtMs;
    State m_state;
};
