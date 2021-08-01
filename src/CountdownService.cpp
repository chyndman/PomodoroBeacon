#include <Arduino.h>
#include "CountdownService.h"
#include "SignalBus.h"

CountdownService::CountdownService(const unsigned sigExpired)
    : m_sigExpired(sigExpired)
    , m_scheduledAtMs(0)
    , m_expireAtMs(0)
    , m_state(State::IDLE)
{
}

void CountdownService::loop(SignalBus& sb)
{
    bool doLatch = false;
    sb.claim(m_sigExpired);

    switch (m_state) {
    case State::SCHEDULED_CPU:
        if (cpuIsExpired()) {
            doLatch = true;
            m_state = State::EXPIRED;
        }
        break;
    default:
        break;
    }

    if (doLatch) {
        sb.latch(m_sigExpired);
    }
}

void CountdownService::schedule(const unsigned long ms)
{
    m_scheduledAtMs = millis();
    m_expireAtMs = m_scheduledAtMs + ms;
    m_state = State::SCHEDULED_CPU;
}

void CountdownService::cancel()
{
    m_scheduledAtMs = 0;
    m_expireAtMs = 0;
    m_state = State::IDLE;
}

bool CountdownService::cpuIsExpired() const
{
    const unsigned long now = millis();
    return (m_expireAtMs <= now) && ((m_scheduledAtMs <= m_expireAtMs) || (now < m_scheduledAtMs));
}
