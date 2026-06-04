#include "Globals.h"

double dt = 0.0;
unsigned long last_time;


double FINALPID_OUTPUT;
double PID_OUTPUT;
double SPEEDPID_OUTPUT;

void setup() {
  setupmotor();
  setupIMU();
  setupcontroller();
  setupEncoders();
}

void loop() {
  //--------DELTA TIME CALCULATION-----------------
  unsigned long now = micros();
  dt = (now - last_time) / 1000000.0;
  last_time = now;

  //-----------FUNCTIONS-----------------
  updateIMU();
  int pwm = controller();
  motor(pwm, FINALPID_OUTPUT);  
}
