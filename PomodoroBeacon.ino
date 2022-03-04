#include <Arduino.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>

#define TICK_HZ 50
#define TICK_PERIOD_USEC 20000

enum Color {
    COLOR_R = (1U << 0),
    COLOR_G = (1U << 1),
    COLOR_B = (1U << 2),

    COLOR_RED = COLOR_R,
    COLOR_GREEN = COLOR_G,
    COLOR_BLUE = COLOR_B,

    COLOR_CYAN = COLOR_G | COLOR_B,
    COLOR_MAGENTA = COLOR_R | COLOR_B,
    COLOR_YELLOW = COLOR_R | COLOR_G,

    COLOR_BLACK = 0,
    COLOR_WHITE = COLOR_R | COLOR_G | COLOR_B
};

enum BeaconMode {
    BM_NONE = 0,
    BM_INIT,
    BM_SLEEP,
    BM_WORK,
    BM_BREAK,
    BM_GAME,
    BM_HIBERNATE
};

struct BeaconModeParam {
    int color;
    unsigned periodSecs;
    bool periodOnce;
};

enum LedPattern {
    LP_OFF = 0,
    LP_DIM,
    LP_ON,
    LP_BLINK_SLOW,
    LP_BLINK_STROBE
};

struct LedPatternParam {
    unsigned dimShift;
    unsigned paceShift;
    uint32_t pulses;
};

void colorToRgb(uint8_t *r, uint8_t *g, uint8_t *b, int color)
{
    *r = (color & COLOR_R) ? 0xFF : 0;
    *g = (color & COLOR_G) ? 0xFF : 0;
    *b = (color & COLOR_B) ? 0xFF : 0;
}

void setBeaconModeParam(BeaconModeParam *param, BeaconMode bm)
{
    memset(param, 0, sizeof(*param));
    unsigned pdMins = 0;
    switch (bm) {
    case BM_INIT:
        param->color = COLOR_WHITE;
        break;
    case BM_WORK:
        param->color = COLOR_CYAN;
        pdMins = 25;
        param->periodOnce = true;
        break;
    case BM_BREAK:
        param->color = COLOR_GREEN;
        pdMins = 5;
        param->periodOnce = true;
        break;
    case BM_GAME:
        param->color = COLOR_MAGENTA;
        pdMins = 15;
        break;
    case BM_HIBERNATE:
        param->color = COLOR_BLUE;
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
    case LP_DIM:
        param->dimShift = 5;
        // fall through
    case LP_ON:
        param->pulses = 0xFFFFFFFF;
        break;
    case LP_BLINK_SLOW:
        param->paceShift = 2;
        param->pulses = 0xFFFF0000;
        break;
    case LP_BLINK_STROBE:
        param->pulses = 0xE3800000;
        break;
    default:
        break;
    }
}

TWIST g_twist;
uint16_t g_count = 0x0100;
unsigned long g_tickTwistRotation = 0;

BeaconMode g_mode = BM_INIT;
BeaconModeParam g_modeParam;

void setup()
{
    g_twist.begin();
    g_twist.setCount(g_count);
    Serial.begin(115200);
}

bool eventTwistRotation(unsigned long tick)
{
    static BeaconMode mainModeRing[] = {
        BM_WORK, BM_BREAK, BM_GAME, BM_HIBERNATE };
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

    LedPattern lp = LP_DIM;
    if (secsTwistRotation < 5)
        lp = LP_ON;
    else if (g_modeParam.periodOnce && g_modeParam.periodSecs <= secsTwistRotation)
        lp = LP_BLINK_STROBE;
    else if ((secsTwistRotation + 30) % g_modeParam.periodSecs < 30)
        lp = LP_BLINK_SLOW;

    LedPatternParam param;
    setLedPatternParam(&param, lp);

    uint32_t sampleMask = 0x80000000 >> ((tick >> param.paceShift) & 0x1F);
    unsigned dimShift = (param.pulses & sampleMask) ? param.dimShift : 8;

    uint8_t r;
    uint8_t g;
    uint8_t b;
    colorToRgb(&r, &g, &b, g_modeParam.color);
    g_twist.setColor(r >> dimShift, g >> dimShift, b >> dimShift);
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
