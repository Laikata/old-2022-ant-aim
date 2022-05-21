#include <Servo.h>
#include "comms.h"
#include "vector.h"

#define SENS 500
#define DISC (SENS/10000.0)
#define DEFAULT_COORDS {0.0, 0.0, 0.0}

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

vec3 dest_pos(){
  vec3 vec;
  vec.x = 0;
  vec.y = 0;
  vec.z = 1;
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
  vec3 base = base_pos();
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
  realYaw += (yaw-realYaw) / 100;
  realPitch += (pitch-realPitch) / 100;
  servoX.write(realYaw);
  servoY.write(realPitch);
  Serial.print(yaw);
  Serial.print(", ");
  Serial.print(pitch);
  Serial.print(", ");
  Serial.print(realYaw);
  Serial.print(", ");
  Serial.print(realPitch);
  Serial.print("\n");
}
