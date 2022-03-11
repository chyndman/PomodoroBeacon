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
    BM_INIT,
    BM_POMO,
    BM_SLEEP,
    BM_COUNT
};

enum PomodoroState {
    POMOST_WORK,
    POMOST_BREAK,
    POMOST_GAME,
    POMOST_COUNT
};

struct PomodoroStateParam {
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

void setPomodoroStateParam(PomodoroStateParam *param, PomodoroState st)
{
    memset(param, 0, sizeof(*param));
    unsigned pdMins = 0;
    switch (st) {
    case POMOST_WORK:
        param->color = COLOR_CYAN;
        pdMins = 25;
        param->periodOnce = true;
        break;
    case POMOST_BREAK:
        param->color = COLOR_GREEN;
        pdMins = 5;
        param->periodOnce = true;
        break;
    case POMOST_GAME:
        param->color = COLOR_YELLOW;
        pdMins = 15;
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

unsigned long g_tick;

uint16_t g_count;
unsigned long g_tickTwistRotation;

BeaconMode g_mode;

PomodoroState g_pomoState;
PomodoroStateParam g_pomoParam;

void softReset(void)
{
    g_tick = 0;
    g_count = 0x0100;
    g_twist.setCount(g_count);
    g_tickTwistRotation = 0;
    g_mode = BM_INIT;
    g_pomoState = POMOST_WORK;
    setPomodoroStateParam(&g_pomoParam, g_pomoState);
}

void setup()
{
    g_twist.begin();
    Serial.begin(115200);
    softReset();
}

bool eventTwistButton()
{
    bool result = false;
    if (g_twist.isClicked()) {
        result = true;
        BeaconMode nextMode = static_cast<BeaconMode>((g_mode + 1) % BM_COUNT);
        g_mode = nextMode;
    }
    return result;
}

bool eventTwistRotation()
{
    bool result = false;
    uint16_t count = (uint16_t)g_twist.getCount();
    if (count != g_count) {
        result = true;
        g_tickTwistRotation = g_tick;

        g_pomoState =
            static_cast<PomodoroState>(
                (g_pomoState
                 + POMOST_COUNT
                 + (count - g_count))
                % POMOST_COUNT);
        setPomodoroStateParam(&g_pomoParam, g_pomoState);

        g_count = count;
    }
    return result;
}

void getRgbPomo(uint8_t *r, uint8_t *g, uint8_t *b)
{
    unsigned long secsTwistRotation = (g_tick - g_tickTwistRotation) / TICK_HZ;

    LedPattern lp = LP_DIM;
    if (secsTwistRotation < 5)
        lp = LP_ON;
    else if (g_pomoParam.periodOnce && g_pomoParam.periodSecs <= secsTwistRotation)
        lp = LP_BLINK_STROBE;
    else if ((secsTwistRotation + 30) % g_pomoParam.periodSecs < 30)
        lp = LP_BLINK_SLOW;

    LedPatternParam param;
    setLedPatternParam(&param, lp);

    uint32_t sampleMask = 0x80000000 >> ((g_tick >> param.paceShift) & 0x1F);
    unsigned dimShift = (param.pulses & sampleMask) ? param.dimShift : 8;

    colorToRgb(r, g, b, g_pomoParam.color);
    *r >>= dimShift;
    *g >>= dimShift;
    *b >>= dimShift;
}

void tickTwistLed()
{
    uint8_t r = 0xff;
    uint8_t g = 0xff;
    uint8_t b = 0xff;

    switch (g_mode) {
    case BM_INIT:
        break;
    case BM_POMO:
        getRgbPomo(&r, &g, &b);
        break;
    case BM_SLEEP:
        r = 0x1f;
        g = 0;
        b = 0;
        break;
    default:
        break;
    }

    g_twist.setColor(r, g, b);
}

void loop()
{
    unsigned long usec = micros();
    unsigned long usec_end = usec + TICK_PERIOD_USEC;
    if (usec_end < usec)
        return;

    if (eventTwistButton()
        || eventTwistRotation())
        return;

    g_tick++;
    tickTwistLed();

    while (usec_end > micros()) {}
}
