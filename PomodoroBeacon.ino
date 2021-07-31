#include <Arduino.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>
#include "src/PomodoroService.h"
#include "src/QwiicTwistService.h"
#include "src/PomodoroBeaconSignals.h"
#include "src/SignalBus.h"

TWIST twist;

SignalBus sb;
QwiicTwistService twistSvc(&twist, POMOBCN_SIG_TWIST_TURNED);
PomodoroService pomoSvc(&twistSvc);

void setup()
{
  Serial.begin(115200);
  twistSvc.setup();
  pomoSvc.setup();
}

void loop()
{
  twistSvc.loop(sb);
  pomoSvc.loop(sb);
}
