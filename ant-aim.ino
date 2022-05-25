#include <Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "comms.h"
#include "vector.h"
#include <SoftwareSerial.h>

#define SPEED 1
#define MOVE_RATE 50 //ms
#define RECV_RATE 20 //ms
// Coordinates must be in radians for formulas to work
#define DEFAULT_COORDS {37.174242 * PI/180, -3.600380 * PI/180, 3.0}
#define DEFAULT_BASE_COORDS {37.165646 * PI/180, -3.597890 * PI/180, 3.0}

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  100 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  460 // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

SoftwareSerial ss(2,3);

int lastAngle[] = {90, 90};

void moveServo(int servo, int angle){
  if(angle > lastAngle[servo]) lastAngle[servo]++;
  else if(angle < lastAngle[servo]) lastAngle[servo]--;
  angle = map(lastAngle[servo], 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(servo, 0, angle);
}

void loopSlowly(int servo, int angle) {
  while(true){
    static unsigned long nextMove = millis();
    if(nextMove <= millis()){
      moveServo(servo, angle);
      if(lastAngle[servo] == angle) {
        break;
      }
      nextMove = millis() + MOVE_RATE;
    }
  }
}

void setup() {
  Serial.begin(9600);
  ss.begin(9600);
  ss.setTimeout(500);
  pwm.begin();

  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(10);
  
  moveServo(0, 90);
  delay(10);
  moveServo(1, 90);
  delay(2000);
}

void loop() {
  // Auto-aim
  const static vec3 base = DEFAULT_BASE_COORDS;
  static vec3 dest = DEFAULT_COORDS;

  comms_recv(&dest);
  

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
      if(yaw < 0){
        yaw *= -1 /*180 + yaw*/;
        inverted = true;
      }
      
      //yaw = map(yaw, -180, 180, 0, 180);
  
      moveServo(0, yaw);
      whichServo = true;
    }
    else {
      // We move servoY
      
      float forward = pow(sin(dlat / 2), 2) + cos(base.x) * cos(dest.x) * pow(sin(dlon / 2), 2);
      forward = 2 * asin(sqrt(forward)); 
  
      // Radius of Earth in Kilometers
      float R = 6371; 
        
      // Calculate the result 
      forward = forward * R * 1000;
      float pitch = atan2(dhei, forward) * 180/PI;

      if(inverted){
        pitch = 180 - pitch;
      }
  
      moveServo(1, pitch);
      whichServo = false;
    }
    nextMove = millis() + MOVE_RATE;
  }
}
