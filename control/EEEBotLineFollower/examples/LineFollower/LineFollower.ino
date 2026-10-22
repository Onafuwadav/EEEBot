// EEEBot IR line follower for the Arduino Nano.
//
// Reads the IR sensor array, works out where the line is with a weighted
// average, runs it through a PID controller and drives the steering servo
// and both motors. Motor and servo pins match firmware/nano_drive.
//
// Sensor pins are an assumption: the logbook doesn't record them. With the
// motors on A0-A3, the Nano's free analog inputs are A4-A7, so this uses
// four sensors there (this sketch doesn't use I2C, so A4/A5 are free).
// Change SENSOR_PINS for your board; any count from 2 up works.
//
// Calibrate: power up with the sensors over bare floor, wait for the LED,
// then hold them over the line until it goes off.

#include <Servo.h>
#include <EEEBotLineFollower.h>

#define servoPin 4
#define enA 5
#define enB 6
#define INa A0
#define INb A1
#define INc A2
#define INd A3

const uint8_t SENSOR_PINS[] = {A4, A5, A6, A7};  // left to right
const int NUM_SENSORS = sizeof(SENSOR_PINS) / sizeof(SENSOR_PINS[0]);

// Starting gains from the logbook (Kp = 1.0, Ki = 0.1, Kd = 0.01).
PID pid(1.0f, 0.1f, 0.01f);
SteeringConfig steering;
Servo myservo;

int offLine[NUM_SENSORS];
int onLine[NUM_SENSORS];
float lastPosition = 0.0f;
unsigned long lastMicros = 0;

void setup() {
  Serial.begin(9600);
  myservo.attach(servoPin);
  myservo.write(steering.centreAngle);
  pinMode(INa, OUTPUT);
  pinMode(INb, OUTPUT);
  pinMode(INc, OUTPUT);
  pinMode(INd, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  calibrate(offLine);              // sensors over the floor
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000);                     // move the sensors over the line
  calibrate(onLine);
  digitalWrite(LED_BUILTIN, LOW);

  goForwards();
  lastMicros = micros();
}

void loop() {
  int values[NUM_SENSORS];
  for (int i = 0; i < NUM_SENSORS; i++) {
    values[i] = normaliseReading(analogRead(SENSOR_PINS[i]), offLine[i], onLine[i]);
  }

  float position;
  if (!linePosition(values, NUM_SENSORS, position)) {
    // Line lost: keep steering hard towards the side it was last seen.
    position = lastPosition < 0 ? -1.0f : 1.0f;
  }
  lastPosition = position;

  unsigned long now = micros();
  float dt = (now - lastMicros) / 1e6f;
  lastMicros = now;

  float correction = pid.update(position, dt);
  DriveCommand cmd = mixSteering(correction, steering);
  myservo.write(cmd.servoAngle);
  analogWrite(enA, cmd.leftSpeed);
  analogWrite(enB, cmd.rightSpeed);

  delay(10);
}

// Average 20 readings per sensor.
void calibrate(int* out) {
  for (int i = 0; i < NUM_SENSORS; i++) {
    long sum = 0;
    for (int n = 0; n < 20; n++) {
      sum += analogRead(SENSOR_PINS[i]);
      delay(5);
    }
    out[i] = sum / 20;
  }
}

void goForwards() {
  digitalWrite(INa, HIGH);
  digitalWrite(INb, LOW);
  digitalWrite(INc, HIGH);
  digitalWrite(INd, LOW);
}
