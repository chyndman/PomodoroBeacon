#pragma once

#include <cstdint>

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
    TWIST* m_pTwist;
    const unsigned m_sigTurned;
    int16_t m_count;
};
