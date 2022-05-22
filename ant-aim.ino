#include <Servo.h>
#include "comms.h"
#include "vector.h"
#include <SoftwareSerial.h>

#define SPEED 5
#define DEFAULT_COORDS {0.0, 0.0, 0.0}
#define DEFAULT_BASE_COORDS {38.839414772331494, 0.0954006784762083, 3.0}

SoftwareSerial ss(2,3);
Servo servoX;
Servo servoY;

void setup() {
  Serial.begin(9600);
  ss.begin(9600);
  servoX.attach(7);
  servoY.attach(8);
  servoX.write(90);
  servoY.write(90);
  delay(5000);
}

void loop() {
  // Auto-aim
  const static vec3 base = DEFAULT_BASE_COORDS;
  static vec3 dest = DEFAULT_COORDS;
  comms_recv(&dest);

  yaw = atan2(dest.y - base.y, dest.x - base.x) * 180/PI;
  forward = sqrt(pow(dest.x - base.x, 2) + pow(dest.y - base.y, 2)); // Distance to cansat from a topdown view
  pitch = atan2(dest.z - base.z, forward) * 180/PI;

  // Constrain angles
  yaw = constrain(yaw, 0, 180);
  pitch = constrain(pitch, 0, 180);

  // Limit speed
  int lastYaw = servoX.read();
  if(lastYaw - yaw > 0 && lastYaw - yaw > SPEED) {
    yaw = SPEED;
  } else if(lastYaw - yaw < -SPEED) {
    yaw = -SPEED;
  }

  int lastPitch = servoY.read();
  if(lastPitch - pitch > 0 && lastPitch - pitch > SPEED) {
    pitch = SPEED;
  } else if(lastPitch - pitch < -SPEED) {
    pitch = -SPEED;
  }

  // Write to servos
  servoX.write(yaw);
  servoY.write(pitch);
}
