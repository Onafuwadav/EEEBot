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
bool move;

#define enA 5 //EnableA command line - should be a PWM pin
#define enB 6 //EnableB command line - should be a PWM pin
#define INa A0 //Channel A direction
#define INb A1 //Channel A direction
#define INc A2 //Channel B direction
#define INd A3 //Channel B direction

void setup() {
  Wire.begin(0x08);              // join i2c bus with address 8
  Wire.onReceive(receiveEvent);  // create a receive event
  Serial.begin(9600);            // start serial to visualise data
  Serial.print("lol");
  myservo.attach(servoPin); 
  pinMode(INa, OUTPUT);
  pinMode(INb, OUTPUT);
  pinMode(INc, OUTPUT);
  pinMode(INd, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);  
  speedSetting = 200;
  motors(speedSetting, speedSetting);
}


void loop() {
  myservo.write(90);
  delay(7500);
  goForwards();
  delay(2000);
  stopMotors();
  delay(1000);
  myservo.write(70);
  goForwards();
  delay(3000);
  myservo.write(120);
  goForwards();
  delay(1000);
  myservo.write(0);
}


int receiveEvent() {
  Serial.print("lolrrr");
  message = "";
  while (Wire.available()) {  // loop whilst bus is busy
    char c = Wire.read();     // receive data byte by byte
    message += c;             // form complete string
  }
  Serial.println(message);
  order = message.substring(0, 1).toInt();
  distance = message.substring(1).toInt() * 100;
  if(order == 1){
    goForwards();
    delay(300000);
    stopMotors();
  }else if(order == 2){
    goBackwards();
    delay(300000);
    stopMotors();
  }else if(order == 3){
    myservo.write(120);
    goClockwise();
    delay(300000);
    stopMotors();
  }else if(order == 4){
    myservo.write(60);
    goAntiClockwise();
    delay(300000);
    stopMotors();
  }
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