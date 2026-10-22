#include <Wire.h>  
#include <stdio.h>
#include <string.h>
#include <Servo.h> 
#define servoPin 4
Servo myservo;
String message;
int order;
int distance;
float steeringAngle;
byte speedSetting = 0;
volatile bool move = false;  // set by receiveEvent when a new command arrives

#define enA 5 //EnableA command line - PWM pin
#define enB 6 //EnableB command line - PWM pin
#define INa A0 //Channel A direction
#define INb A1 //Channel A direction
#define INc A2 //Channel B direction
#define INd A3 //Channel B direction


#define MS_PER_100MM 400UL  // ms to drive 10 cm
#define MS_PER_90DEG 650UL  // ms to turn 90 degrees


#define USE_ENCODERS 0
#define encoderLeftPin 2
#define encoderRightPin 3
#define PULSES_PER_100MM 20UL   
#define PULSES_PER_90DEG 15UL   
volatile unsigned long leftPulses = 0;
volatile unsigned long rightPulses = 0;

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
#if USE_ENCODERS
  pinMode(encoderLeftPin, INPUT_PULLUP);
  pinMode(encoderRightPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderLeftPin), countLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderRightPin), countRight, RISING);
#endif
  speedSetting = 200;
  motors(speedSetting, speedSetting);
  stopMotors();
  myservo.write(90);  // wheels straight until the first command arrives
}


void loop() {
  if (move) {
    noInterrupts();   // copy the command so a new I2C write can't change it mid-move
    String command = message;
    move = false;
    interrupts();
    executeCommand(command);
  }
}


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
  distance = command.substring(1).toInt() * 100; 
  int angle = distance / 100;                     
  if(order == 1){
    goForwards();
    moveFor(driveTime(distance), (unsigned long)distance * PULSES_PER_100MM / 100);
    stopMotors();
  }else if(order == 2){
    goBackwards();
    moveFor(driveTime(distance), (unsigned long)distance * PULSES_PER_100MM / 100);
    stopMotors();
  }else if(order == 3){
    myservo.write(120);
    goClockwise();
    moveFor(turnTime(angle), (unsigned long)angle * PULSES_PER_90DEG / 90);
    stopMotors();
  }else if(order == 4){
    myservo.write(60);
    goAntiClockwise();
    moveFor(turnTime(angle), (unsigned long)angle * PULSES_PER_90DEG / 90);
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

// Keep the motors running until the move is done. 
void moveFor(unsigned long timeMs, unsigned long targetPulses) {
#if USE_ENCODERS
  noInterrupts();
  leftPulses = 0;
  rightPulses = 0;
  interrupts();
  unsigned long start = millis();
  while (millis() - start < 2 * timeMs) {
    noInterrupts();
    unsigned long average = (leftPulses + rightPulses) / 2;
    interrupts();
    if (average >= targetPulses) {
      return;
    }
  }
  Serial.println("Encoder timeout - check encoder wiring");
#else
  (void)targetPulses;
  delay(timeMs);
#endif
}

#if USE_ENCODERS
void countLeft() {
  leftPulses++;
}
void countRight() {
  rightPulses++;
}
#endif

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