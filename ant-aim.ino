#include <Servo.h>
#include "comms.h"
#include "vector.h"
#include <SoftwareSerial.h>

#define SENS 500
#define DISC (SENS/10000.0)
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
  yaw = atan((dest.y-base.y)/(dest.x-base.x))*180/PI;
  pitch = atan((dest.z-base.z)/sqrt(dest.x*dest.x+dest.y*dest.y))*180/PI;


  // Write constrained values to servos 
  yaw = constrain(yaw, 0, 180);
  pitch = constrain(pitch, 0, 180);
  realYaw += (yaw-realYaw) / 1000;
  realPitch += (pitch-realPitch) / 1000;
  servoX.write(realYaw);
  servoY.write(realPitch);
}
