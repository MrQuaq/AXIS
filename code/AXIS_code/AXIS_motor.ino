#define PWM_FREQ 20000
#define PWM_RES 8

#define CH1 0
#define CH2 1

#define PWMA 16
#define PWMB 17
#define A01 18
#define A02 19
#define B01 20
#define B02 22

void setupmotor() {

  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);

  pinMode(A01, OUTPUT);
  pinMode(A02, OUTPUT);
  pinMode(B01, OUTPUT);
  pinMode(B02, OUTPUT);

  ledcSetup(CH1, PWM_FREQ, PWM_RES);
  ledcAttachPin(PWMA, CH1);

  ledcSetup(CH2, PWM_FREQ, PWM_RES);
  ledcAttachPin(PWMB, CH2);
}

void motor(int pwm, double PID_OUTPUT){

  pwm = constrain(abs(pwm), 0, 255);

  if (FINALPID_OUTPUT < -20) {

    digitalWrite(A01, HIGH);
    digitalWrite(B01, HIGH);
    digitalWrite(A02, LOW);
    digitalWrite(B02, LOW);

    ledcWrite(CH1, pwm);
    ledcWrite(CH2, pwm);
  }

  else if (FINALPID_OUTPUT > 20) {

    digitalWrite(A01, LOW);
    digitalWrite(B01, LOW);
    digitalWrite(A02, HIGH);
    digitalWrite(B02, HIGH);

    ledcWrite(CH1, pwm);
    ledcWrite(CH2, pwm);
  }

  else {
    ledcWrite(CH1, 0);
    ledcWrite(CH2, 0);
  }
}

  