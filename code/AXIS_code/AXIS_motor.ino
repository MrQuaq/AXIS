#define PWM_FREQ 20000
#define PWM_RES 8

#define CH1 0
#define CH2 1

#define PWMA 25
#define AIN1 27
#define AIN2 26

#define PWMB 18
#define BIN1 16
#define BIN2 17

#define STBY 23

void setupmotor() {

  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  ledcSetup(CH1, PWM_FREQ, PWM_RES);
  ledcAttachPin(PWMA, CH1);

  ledcSetup(CH2, PWM_FREQ, PWM_RES);
  ledcAttachPin(PWMB, CH2);
}

void motor(int pwm, double PID_OUTPUT){

  pwm = constrain(abs(pwm), 0, 255);

  if (FINALPID_OUTPUT < -20) {

    digitalWrite(AIN1, HIGH);
    digitalWrite(BIN1, HIGH);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN2, LOW);

    ledcWrite(CH1, pwm);
    ledcWrite(CH2, pwm);
  }

  else if (FINALPID_OUTPUT > 20) {

    digitalWrite(AIN1, LOW);
    digitalWrite(BIN1, LOW);
    digitalWrite(AIN2, HIGH);
    digitalWrite(BIN2, HIGH);

    ledcWrite(CH1, pwm);
    ledcWrite(CH2, pwm);
  }

  else {
    ledcWrite(CH1, 0);
    ledcWrite(CH2, 0);
  }
}

  