#pragma once

#include <cstdint>
#include "CountdownService.h"

class SignalBus;
class TWIST;

class QwiicTwistService
{
public:
    QwiicTwistService(TWIST* pTwist, const unsigned sigTurned);

    void setup();
    void loop(SignalBus& sb);
    
    void setColor(const uint8_t red, const uint8_t green, const uint8_t blue);
    void setColorBlink(const uint8_t red, const uint8_t green, const uint8_t blue, const unsigned msOn, const unsigned msOff);
    int16_t getCount();

private:
    enum LedState
    {
        SOLID,
        BLINK_ON,
        BLINK_OFF
    };

    void applyColor(const uint8_t red, const uint8_t green, const uint8_t blue);
    void transition(const LedState state);

    TWIST* m_pTwist;
    const unsigned m_sigTurned;
    int16_t m_count;
    CountdownService m_blinkCntdnSvc;
    unsigned m_blinkMsOn;
    unsigned m_blinkMsOff;
    LedState m_ledState;
    uint8_t m_ledR;
    uint8_t m_ledG;
    uint8_t m_ledB;
};
