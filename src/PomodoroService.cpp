#include "PomodoroService.h"
#include "QwiicTwistService.h"
#include "CountdownService.h"
#include "SignalBus.h"
#include "PomodoroBeaconSignals.h"
#include <Arduino.h>

PomodoroService::PomodoroService(QwiicTwistService* pTwistSvc, CountdownService* pCntdnSvc)
    : m_pTwistSvc(pTwistSvc)
    , m_pCntdnSvc(pCntdnSvc)
    , m_phase(Phase::INIT)
    , m_mode(Mode::INIT)
    , m_twistCount(0)
{
}

void PomodoroService::setup()
{
    transition(m_phase, m_mode, getModeSpec(m_mode));
}

void PomodoroService::loop(SignalBus& sb)
{
    if (sb.probe(POMOBCN_SIG_TWIST_TURNED)) {
        const int16_t count = m_pTwistSvc->getCount();
        const ModeSpec* pSpec = getModeSpec(m_mode);
        const Mode modeNew = (count < m_twistCount) ? pSpec->modePrev : pSpec->modeNext;
        m_twistCount = count;
        transition(Phase::ARMING, modeNew, getModeSpec(modeNew));
    }
    else if (sb.probe(POMOBCN_SIG_COUNTDOWN_EXPIRED)) {
        const ModeSpec* pSpec = getModeSpec(m_mode);
        const Phase phaseNew =
            (Phase::ADVISE == m_phase && pSpec->repeating)
            ? Phase::MAIN
            : static_cast<Phase>(static_cast<unsigned>(m_phase) + 1);
        transition(phaseNew, m_mode, pSpec);
    }
    else if (Mode::INIT == m_mode) {
        static const unsigned long p = 8000;
        const unsigned long ms = millis();
        const float thetar = static_cast<float>((ms + (2 * p / 3)) % p) * (2 * M_PI) / static_cast<float>(p);
        const float thetag = static_cast<float>((ms + (p / 3)) % p) * (2 * M_PI) / static_cast<float>(p);
        const float thetab = static_cast<float>(ms % p) * (2 * M_PI) / static_cast<float>(p);
        const float cosr = cos(thetar);
        const float cosg = cos(thetag);
        const float cosb = cos(thetab);
        const int discosr = constrain(static_cast<int>((cosr + 0.5) * 255.0), 0, 255);
        const int discosg = constrain(static_cast<int>((cosg + 0.5) * 255.0), 0, 255);
        const int discosb = constrain(static_cast<int>((cosb + 0.5) * 255.0), 0, 255);
        const uint8_t r = static_cast<uint8_t>(discosr);
        const uint8_t g = static_cast<uint8_t>(discosg);
        const uint8_t b = static_cast<uint8_t>(discosb);
        m_pTwistSvc->setColor(r, g, b);
    }
}

void PomodoroService::transition(const Phase phase, const Mode mode, const ModeSpec* pSpec)
{
    m_phase = phase;
    m_mode = mode;
    Serial.print("transition ");
    Serial.print(static_cast<unsigned>(phase));
    Serial.print(", ");
    Serial.print(static_cast<unsigned>(mode));
    Serial.print(", ");
    Serial.print(m_twistCount);
    Serial.println("");

    switch (phase) {
    case Phase::ARMING:
        m_pTwistSvc->setColor(pSpec->r, pSpec->g, pSpec->b);
        m_pCntdnSvc->schedule(1000 * 5);
        break;
    case Phase::MAIN:
        if (0 < pSpec->periodMinutes) {
            m_pTwistSvc->setColor(pSpec->r / 32, pSpec->g / 32, pSpec->b / 32);
            m_pCntdnSvc->schedule(1000 * ((pSpec->periodMinutes * 60) - 30));
            break;
        }
        else {
            m_phase = Phase::ADVISE;
        }
        // fall through
    case Phase::ADVISE:
        m_pTwistSvc->setColorBlink(pSpec->r, pSpec->g, pSpec->b, 1200, 1200);
        m_pCntdnSvc->schedule(1000 * 30);
        break;
    case Phase::END:
        m_pTwistSvc->setColorBlink(pSpec->r, pSpec->g, pSpec->b, 40, 560);
        // fall through
    default:
        m_pCntdnSvc->cancel();
    }
}

const PomodoroService::ModeSpec* PomodoroService::getModeSpec(const Mode mode)
{
    const ModeSpec* pSpec = NULL;
    static const struct
    {
        Mode mode;
        ModeSpec spec;
    } map[] =
    {
        { Mode::INIT,   { 255, 255, 255, 0,   true,  Mode::WORK,  Mode::WORK  } },
        { Mode::WORK,   { 0,   0,   255, 25,  false, Mode::BREAK, Mode::AWAY  } },
        { Mode::BREAK,  { 0,   255, 0,   5,   false, Mode::GAME,  Mode::WORK  } },
        { Mode::GAME,   { 255, 255, 0,   60,  true,  Mode::AWAY,  Mode::BREAK } },
        { Mode::AWAY,   { 255, 0,   0,   720, false, Mode::WORK,  Mode::GAME  } }
    };

    for (unsigned i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
        if (map[i].mode == mode) {
            pSpec = &(map[i].spec);
            break;
        }
    }

    return pSpec;
}
