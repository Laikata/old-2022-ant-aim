#include <Servo.h>
#include "comms.h"

#define SENS 25
#define DISC (SENS/10000.0)

Servo servoX;
Servo servoY;

float pitch = 0; // 0 to 180
float yaw = 0; // 0 to 180

bool manual = true;

bool btnPressedLast = true;

void setup() {
  Serial.begin(9600);
  servoX.attach(7);
  servoY.attach(8);
  delay(3000);
  servoX.write(90);
  servoY.write(90);
}

void loop(){
  {
    bool btn = digitalRead(8);
    if(!btnPressedLast && btn) manual = !manual;
    btnPressedLast = btn;
  }

  digitalWrite(10, manual);
  if (manual) {
    float yawI = map(analogRead(A0), 0, 1023, -SENS, SENS) / 1000.0;
    float pitchI = map(analogRead(A1), 0, 1023, -SENS, SENS) / 1000.0;
    if (yawI < -DISC || yawI > DISC) yaw += yawI;
    if (pitchI < -DISC || pitchI > DISC) pitch += pitchI;
  }
  else{
    float destYaw = 90;
    float destPitch = millis() / 100 % 180;
    yaw += (destYaw - yaw) / 5000.0;
    pitch += (destPitch - pitch) / 5000.0;
  }
  yaw = constrain(yaw, 0, 180);
  pitch = constrain(pitch, 0, 180);
  servoX.write((int)yaw);
  servoY.write(map((int)pitch, 0, 180, 0, 180));
}
