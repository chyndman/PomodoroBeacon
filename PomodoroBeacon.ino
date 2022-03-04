#include <Arduino.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>

#define TICK_HZ 50
#define TICK_PERIOD_USEC 20000

enum BeaconMode {
    NONE = 0,
    INIT,
    SLEEP,
    WORK,
    BREAK,
    GAME,
    HIBERNATE
};

struct BeaconModeParam {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    unsigned periodSecs;
    bool periodOnce;
};

enum LedPattern {
    OFF = 0,
    DIM,
    ON,
    BLINK_SLOW,
    BLINK_STROBE
};

struct LedPatternParam {
    unsigned dimShift;
    unsigned paceShift;
    uint32_t pulses;
};

void setBeaconModeParam(BeaconModeParam *param, BeaconMode bm)
{
    memset(param, 0, sizeof(*param));
    unsigned pdMins = 0;
    switch (bm) {
    case BeaconMode::INIT:
        param->r = 0xFF;
        param->g = 0xFF;
        param->b = 0xFF;
        break;
    case BeaconMode::WORK:
        param->g = 0xFF;
        param->b = 0xFF;
        pdMins = 25;
        param->periodOnce = true;
        break;
    case BeaconMode::BREAK:
        param->g = 0xFF;
        pdMins = 5;
        param->periodOnce = true;
        break;
    case BeaconMode::GAME:
        param->r = 0xFF;
        param->b = 0xFF;
        pdMins = 15;
        break;
    case BeaconMode::HIBERNATE:
        param->r = 0xFF;
        pdMins = 12 * 60;
        break;
    default:
        break;
    }

    param->periodSecs = pdMins * 60;
}

void setLedPatternParam(LedPatternParam *param, LedPattern lp)
{
    memset(param, 0, sizeof(*param));
    switch (lp) {
    case LedPattern::DIM:
        param->dimShift = 5;
        // fall through
    case LedPattern::ON:
        param->pulses = 0xFFFFFFFF;
        break;
    case LedPattern::BLINK_SLOW:
        param->paceShift = 2;
        param->pulses = 0xFFFF0000;
        break;
    case LedPattern::BLINK_STROBE:
        param->pulses = 0xD8600000;
        break;
    default:
        break;
    }
}

TWIST g_twist;
uint16_t g_count = 0x0100;
unsigned long g_tickTwistRotation = 0;

BeaconMode g_mode = BeaconMode::INIT;
BeaconModeParam g_modeParam;

void setup()
{
    g_twist.begin();
    g_twist.setCount(g_count);
    Serial.begin(115200);
}

bool eventTwistRotation(unsigned long tick)
{
    static BeaconMode mainModeRing[] = { WORK, BREAK, GAME, HIBERNATE };
    bool result = false;
    uint16_t count = (uint16_t)g_twist.getCount();
    if (count != g_count) {
        g_count = count;
        result = true;
        g_tickTwistRotation = tick;

        g_mode = mainModeRing[
            count % (sizeof(mainModeRing) / sizeof(mainModeRing[0]))];
        setBeaconModeParam(&g_modeParam, g_mode);
    }
    return result;
}

void tickTwistLed(unsigned long tick)
{
    unsigned long secsTwistRotation = (tick - g_tickTwistRotation) / TICK_HZ;

    LedPattern lp = LedPattern::DIM;
    if (secsTwistRotation < 5)
        lp = LedPattern::ON;
    else if (g_modeParam.periodOnce && g_modeParam.periodSecs <= secsTwistRotation)
        lp = LedPattern::BLINK_STROBE;
    else if ((secsTwistRotation + 30) % g_modeParam.periodSecs < 30)
        lp = LedPattern::BLINK_SLOW;

    LedPatternParam param;
    setLedPatternParam(&param, lp);

    uint32_t sampleMask = 0x80000000 >> ((tick >> param.paceShift) & 0x1F);
    uint8_t channelMask = (param.pulses & sampleMask) ? 0xFF : 0;

    uint8_t r = channelMask & (g_modeParam.r >> param.dimShift);
    uint8_t g = channelMask & (g_modeParam.g >> param.dimShift);
    uint8_t b = channelMask & (g_modeParam.b >> param.dimShift);
    g_twist.setColor(r, g, b);
}

void loop()
{
    static unsigned long tick = 1;
    
    unsigned long usec = micros();
    unsigned long usec_end = usec + TICK_PERIOD_USEC;
    if (usec_end < usec)
        return;
    
    if (eventTwistRotation(tick))
        return;

    tick++;
    tickTwistLed(tick);

    while (usec_end > micros()) {}
}
