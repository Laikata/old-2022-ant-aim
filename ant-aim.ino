#include <Servo.h>
#include "comms.h"
#include "vector.h"
#include <SoftwareSerial.h>

#define SPEED 1
#define MOVE_RATE 50 //ms
#define RECV_RATE 20 //ms
// Coordinates must be in radians for formulas to work
#define DEFAULT_COORDS {0.0 * PI/180, 0.0 * PI/180, 100.0}
#define DEFAULT_BASE_COORDS {2.0 * PI/180, 2.0 * PI/180, 3.0}

SoftwareSerial ss(2,3);
Servo servoX;
Servo servoY;

void loopSlowly(Servo *servo, int angle) {
  while(true){
    static unsigned long nextMove = millis();
    if(nextMove <= millis()){
      moveSlowly(servo, angle);
      if(servo->read() == angle) {
        break;
      }
      nextMove = millis() + MOVE_RATE;
    }
  }
}

void moveSlowly(Servo *servo, int angle) {
  int lastAngle = servo->read();
  if(angle > lastAngle) lastAngle++;
  else if(angle < lastAngle) lastAngle--;
  servo->write(lastAngle);
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  servoX.attach(7);
  loopSlowly(&servoX, 90);
  delay(2000);
  servoY.attach(8);
  loopSlowly(&servoY, 15);
  delay(2000);
  ss.begin(9600);
  //ss.setTimeout(10);
}

void loop() {
  // Auto-aim
  const static vec3 base = DEFAULT_BASE_COORDS;
  static vec3 dest = DEFAULT_COORDS;

  //comms_recv(&dest);  

  static unsigned long nextMove = millis();
  
  if(nextMove <= millis()) {
    float dlat = dest.x - base.x;
    float dlon = dest.y - base.y;
    float dhei = dest.z - base.z;
    
    static bool whichServo = false;
    static bool inverted = false;
    if(whichServo == false) {
      // We move servoX
      float yaw = atan2(dlon, dlat) * 180/PI;
      inverted = false;
      Serial.println(yaw);
      if(yaw < 0){
        yaw *= -1 /*180 + yaw*/;
        inverted = true;
      }
      
      Serial.println(yaw);
      //yaw = map(yaw, -180, 180, 0, 180);
  
      moveSlowly(&servoX, yaw);
      whichServo = true;
    }
    else {
      // We move servoY
      
      float forward = pow(sin(dlat / 2), 2) + cos(base.x) * cos(dest.x) * pow(sin(dlon / 2), 2);
      forward = 2 * asin(sqrt(forward)); 
  
      // Radius of Earth in Kilometers
      float R = 6371; 
        
      // Calculate the result 
      forward = forward * R;
      float pitch = atan2(dhei, forward) * 180/PI;

      if(inverted){
        pitch = 180 - pitch;
      }
  
      moveSlowly(&servoY, pitch);
      whichServo = false;
    }
    nextMove = millis() + MOVE_RATE;
  }
}
