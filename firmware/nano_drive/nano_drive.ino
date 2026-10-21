#include <Wire.h>  
#include <stdio.h>
#include <string.h>
#include <Servo.h>    //include the servo library
#define servoPin 4
Servo myservo;
String message;
int order;
int distance;
float steeringAngle;
byte speedSetting = 0;
volatile bool move = false;  // set by receiveEvent when a new command arrives

#define enA 5 //EnableA command line - should be a PWM pin
#define enB 6 //EnableB command line - should be a PWM pin
#define INa A0 //Channel A direction
#define INb A1 //Channel A direction
#define INc A2 //Channel B direction
#define INd A3 //Channel B direction

// Timing calibration at speedSetting = 200. These are starting values:
// send "11" (forward 10 cm) and "390" (90 deg clockwise) from the keypad
// and adjust until the robot matches the maze grid.
#define MS_PER_100MM 400UL  // ms to drive 10 cm
#define MS_PER_90DEG 650UL  // ms to turn 90 degrees

void setup() {
  Wire.begin(0x08);              // join i2c bus with address 8
  Wire.onReceive(receiveEvent);  // create a receive event
  Serial.begin(9600);            // start serial to visualise data
  Serial.println("Nano drive ready - waiting for I2C commands on 0x08");
  myservo.attach(servoPin); 
  pinMode(INa, OUTPUT);
  pinMode(INb, OUTPUT);
  pinMode(INc, OUTPUT);
  pinMode(INd, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);  
  speedSetting = 200;
  motors(speedSetting, speedSetting);
  stopMotors();
  myservo.write(90);  // wheels straight until the first command arrives
}


void loop() {
  if (move) {
    noInterrupts();           // copy the command so a new I2C write can't change it mid-move
    String command = message;
    move = false;
    interrupts();
    executeCommand(command);
  }
}


// Runs inside the I2C interrupt: only store the command and flag it.
// The movement (and its delays) happens in loop().
void receiveEvent(int howMany) {
  message = "";
  while (Wire.available()) {  // loop whilst bus is busy
    char c = Wire.read();     // receive data byte by byte
    message += c;             // form complete string
  }
  move = true;
}

void executeCommand(String command) {
  Serial.print("Received command: ");
  Serial.println(command);
  order = command.substring(0, 1).toInt();
  distance = command.substring(1).toInt() * 100;  // forward/back: quantity is in 10 cm steps -> mm
  int angle = distance / 100;                       // turns: quantity is the angle in degrees
  if(order == 1){
    goForwards();
    delay(driveTime(distance));
    stopMotors();
  }else if(order == 2){
    goBackwards();
    delay(driveTime(distance));
    stopMotors();
  }else if(order == 3){
    myservo.write(120);
    goClockwise();
    delay(turnTime(angle));
    stopMotors();
  }else if(order == 4){
    myservo.write(60);
    goAntiClockwise();
    delay(turnTime(angle));
    stopMotors();
  }else{
    Serial.println("Unknown order - expected 1 to 4");
  }
  myservo.write(90);  // straighten the wheels ready for the next command
}

// Time to drive distance mm at speedSetting
unsigned long driveTime(int distanceMm) {
  return (unsigned long)distanceMm * MS_PER_100MM / 100;
}

// Time to turn angle degrees at speedSetting
unsigned long turnTime(int angle) {
  return (unsigned long)angle * MS_PER_90DEG / 90;
}

void motors(int leftSpeed, int rightSpeed) {
  analogWrite(enA, leftSpeed);
  analogWrite(enB, rightSpeed);
}
void goForwards() {
  digitalWrite(INa, HIGH);
  digitalWrite(INb, LOW);
  digitalWrite(INc, HIGH);
  digitalWrite(INd, LOW);
}
void goBackwards() {
  digitalWrite(INa, LOW);
  digitalWrite(INb, HIGH);
  digitalWrite(INc, LOW);
  digitalWrite(INd, HIGH);
}
void goClockwise() {
  digitalWrite(INa, HIGH);
  digitalWrite(INb, LOW);
  digitalWrite(INc, LOW);
  digitalWrite(INd, HIGH);
}
void goAntiClockwise() {
  digitalWrite(INa, LOW);
  digitalWrite(INb, HIGH);
  digitalWrite(INc, HIGH);
  digitalWrite(INd, LOW);
}
void stopMotors() { 
  digitalWrite(INa, LOW);
  digitalWrite(INb, LOW);
  digitalWrite(INc, LOW);
  digitalWrite(INd, LOW);
}