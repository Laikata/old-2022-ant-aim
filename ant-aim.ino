#include <Servo.h>
#include "comms.h"
#include "vector.h"
#include <SoftwareSerial.h>

#define SENS 500
#define DISC (SENS/10000.0)
#define DEFAULT_COORDS {0.0, 0.0, 0.0}
#define DEFAULT_BASE_COORDS {38.83450, 0.10326, 3.0}

SoftwareSerial ss(2,3);
Servo servoX;
Servo servoY;

float pitch = 45; // 0 to 180
float yaw = 90; // 0 to 180
float realPitch = pitch;
float realYaw = yaw;

vec3 base_pos(){
  vec3 vec;
  vec.x = 0;
  vec.y = 0;
  vec.z = 0;
  return vec;
}

void setup() {
  Serial.begin(9600);
  servoX.attach(7);
  servoY.attach(8);
  delay(5000);
  for(int i = 0; i < 100; i++){
    servoX.write(((90*(100-i))+(yaw*i))/100);
    servoY.write(((90*(100-i))+(pitch*i))/100);
    delay(10);
  }
  delay(1000);
}

void loop() {
  // Auto-aim
  const static vec3 base = DEFAULT_BASE_COORDS;
  static vec3 dest = DEFAULT_COORDS;
  comms_recv(&dest);
  yaw = atan((dest.y-base.y)/(dest.x-base.x))*180/PI;
  pitch = atan((dest.z-base.z)/sqrt(dest.x*dest.x+dest.y*dest.y))*180/PI;

  // Manual joystick adjustment
  float yawI = 0;
  float pitchI = 0;
  if(analogRead(A0) > 5) yawI = map(analogRead(A0), 0, 1023, -SENS, SENS) / 1000.0;
  if(analogRead(A1) > 5) pitchI = map(analogRead(A1), 0, 1023, -SENS, SENS) / 1000.0;
  if (yawI < -DISC || yawI > DISC) yaw += yawI;
  if (pitchI < -DISC || pitchI > DISC) pitch += pitchI;

  // Write constrained values to servos 
  yaw = constrain(yaw, 0, 180);
  pitch = constrain(pitch, 0, 180);
  realYaw += (yaw-realYaw) / 1000;
  realPitch += (pitch-realPitch) / 1000;
  servoX.write(realYaw);
  servoY.write(realPitch);
}
