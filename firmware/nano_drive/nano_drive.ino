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

// Wheel encoders. Set USE_ENCODERS to 1 once the encoder pins below are
// confirmed against the PCB's encoder header; D2/D3 are the Nano's
// external-interrupt pins. With 0 the timings above are used instead.
#define USE_ENCODERS 0
#define encoderLeftPin 2
#define encoderRightPin 3
#define PULSES_PER_100MM 20UL   // calibrate: pulses counted over 10 cm
#define PULSES_PER_90DEG 15UL   // calibrate: pulses counted over a 90 deg turn
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

// Keep the motors running until the move is done. With encoders, stop
// once the average wheel count reaches targetPulses (giving up after
// twice the calibrated time in case an encoder is disconnected).
// Without encoders, just wait the calibrated time.
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