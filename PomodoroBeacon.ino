#include <SparkFun_Qwiic_Twist_Arduino_Library.h>
#include <SparkFun_RV8803.h>

void setup() {
  TWIST twist;
  twist.begin();
  twist.setColor(0, 255, 0);
}

void loop() {
  delay(10);
}
