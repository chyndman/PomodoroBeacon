#include "QwiicTwistService.h"
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>
#include "SignalBus.h"

QwiicTwistService::QwiicTwistService(TWIST* pTwist, const unsigned sigTurned)
    : m_pTwist(pTwist)
    , m_sigTurned( sigTurned )
{
}

void QwiicTwistService::setup()
{
    m_pTwist->begin();
    m_pTwist->setColor(255, 255, 255);
    m_pTwist->setCount(0);
}

void QwiicTwistService::loop(SignalBus& sb)
{
    sb.claim(m_sigTurned);
    if (sb.probe(0)) {
        int16_t count = getCount();
        if (count != m_count) {
            m_count = count;
            sb.latch(m_sigTurned);
        }
    }
}

void QwiicTwistService::setColor(const uint8_t red, const uint8_t green, const uint8_t blue)
{
    m_pTwist->setColor(red, green, blue);
}

void QwiicTwistService::setColorBlink(const uint8_t red, const uint8_t green, const uint8_t blue, const unsigned msOn, const unsigned msOff)
{
    // TODO
}

int16_t QwiicTwistService::getCount()
{
    return m_pTwist->getCount();
}
