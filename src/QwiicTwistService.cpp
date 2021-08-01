#include "QwiicTwistService.h"
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>
#include "SignalBus.h"

QwiicTwistService::QwiicTwistService(TWIST* pTwist, const unsigned sigTurned)
    : m_pTwist(pTwist)
    , m_sigTurned(sigTurned)
    , m_count(0)
    , m_blinkCntdnSvc(0)
    , m_blinkMsOn(0)
    , m_blinkMsOff(0)
    , m_ledState(LedState::SOLID)
    , m_ledR(0)
    , m_ledG(0)
    , m_ledB(0)
{
}

void QwiicTwistService::setup()
{
    m_pTwist->begin();
    m_pTwist->setCount(0);
    applyColor(255, 255, 255);
    transition(LedState::SOLID);
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

    switch (m_ledState) {
    case LedState::BLINK_ON:
        if (m_blinkCntdnSvc.cpuIsExpired()) {
            transition(LedState::BLINK_OFF);
        }
        break;
    case LedState::BLINK_OFF:
        if (m_blinkCntdnSvc.cpuIsExpired()) {
            transition(LedState::BLINK_ON);
        }
        break;
    default:
        break;
    }
}

void QwiicTwistService::setColor(const uint8_t red, const uint8_t green, const uint8_t blue)
{
    applyColor(red, green, blue);
    transition(LedState::SOLID);
}

void QwiicTwistService::setColorBlink(const uint8_t red, const uint8_t green, const uint8_t blue, const unsigned msOn, const unsigned msOff)
{
    applyColor(red, green, blue);
    m_blinkMsOn = msOn;
    m_blinkMsOff = msOff;
    transition(LedState::BLINK_ON);
}

int16_t QwiicTwistService::getCount()
{
    return m_pTwist->getCount();
}

void QwiicTwistService::applyColor(const uint8_t red, const uint8_t green, const uint8_t blue)
{
    m_ledR = red;
    m_ledG = green;
    m_ledB = blue;
}

void QwiicTwistService::transition(const LedState state)
{
    m_ledState = state;
    bool on = false;
    unsigned blinkMs = 0;
    
    switch (state) {
    case LedState::SOLID:
        on = true;
        break;
    case LedState::BLINK_ON:
        blinkMs = m_blinkMsOn;
        on = true;
        break;
    case LedState::BLINK_OFF:
        blinkMs = m_blinkMsOff;
        break;
    default:
        break;
    }

    if (0 != blinkMs) {
        m_blinkCntdnSvc.schedule(blinkMs);
    }
    else {
        m_blinkCntdnSvc.cancel();
    }

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    if (on) {
        r = m_ledR;
        g = m_ledG;
        b = m_ledB;
    }
    m_pTwist->setColor(r, g, b);
}
