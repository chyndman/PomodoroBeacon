#include "PomodoroService.h"
#include "QwiicTwistService.h"
#include "SignalBus.h"
#include "PomodoroBeaconSignals.h"
#include <Arduino.h>

PomodoroService::PomodoroService(QwiicTwistService* pTwistSvc)
    : m_pTwistSvc(pTwistSvc)
{
}

void PomodoroService::setup()
{
}

void PomodoroService::loop(SignalBus& sb)
{
    if (sb.probe(POMOBCN_SIG_TWIST_TURNED)) {
        Serial.print("probed ");
        const int16_t count = m_pTwistSvc->getCount();
        Serial.println(count);
        const uint8_t r = (0 == count % 3) ? 255 : 0;
        const uint8_t g = (1 == count % 3) ? 255 : 0;
        const uint8_t b = (2 == count % 3) ? 255 : 0;
        m_pTwistSvc->setColor(r, g, b);
    }
}
