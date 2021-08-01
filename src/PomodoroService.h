#pragma once

#include <cstdint>

class SignalBus;
class QwiicTwistService;
class CountdownService;

class PomodoroService
{
public:
    PomodoroService(QwiicTwistService* pTwistSvc, CountdownService* pCntdnSvc);

    void setup();
    void loop(SignalBus& sb);

private:
    QwiicTwistService* m_pTwistSvc;
    CountdownService* m_pCntdnSvc;

    enum class Phase
    {
        INIT,
        ARMING,
        MAIN,
        ADVISE,
        END
    };
    Phase m_phase;

    enum class Mode
    {
        INIT,
        WORK,
        BREAK,
        GAME,
        AWAY
    };
    Mode m_mode;

    struct ModeSpec
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        unsigned periodMinutes;
        bool repeating;
        Mode modeNext;
        Mode modePrev;
    };

    int16_t m_twistCount;

    void transition(const Phase phase, const Mode mode, const ModeSpec* pSpec);
    static const ModeSpec* getModeSpec(const Mode mode);
};
