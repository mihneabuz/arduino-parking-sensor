#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Servo.h>

#define BW_LED 12
#define FW_LED 11
#define LF_SERVO 8
#define RT_SERVO 9
#define HC_TRIG 10
#define HC_LF 5
#define HC_RT 6
#define TIMEOUT 500

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

Servo servo_l, servo_r;

bool reversing = false;
bool forwarding = false;
bool coasting = true;
bool stopped = true;
int timeout = TIMEOUT;
float reg = 0;
float X = 0;
float Y = 0;
float lastX = 0;
float lastY = 0;

void setup(void) 
{
  Serial.begin(9600);
 
  while(!accel.begin()) {
    Serial.println("Erorr: no ADXL345 detected\n");
    delay(1000);
  }
 
  /* Set the range */
  accel.setRange(ADXL345_RANGE_2_G);

  pinMode(BW_LED, OUTPUT);
  pinMode(FW_LED, OUTPUT);

  servo_l.attach(LF_SERVO);
  servo_l.write(0);
  servo_r.attach(RT_SERVO);
  servo_r.write(0);

  pinMode(HC_TRIG, OUTPUT);
  pinMode(HC_LF, INPUT);
  pinMode(HC_RT, INPUT);
}

void activate() {
  servo_l.write(90);
  servo_r.write(270);
}

void deactivate() {
  servo_l.write(0);
  servo_r.write(0);
}

void getDistance() {
  digitalWrite(HC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(HC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(HC_TRIG, LOW);

  //float distance = min(pulseIn(HC_LF, HIGH), pulseIn(HC_RT, HIGH));
  float distance = pulseIn(HC_LF, HIGH);

  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.print("\n");
 
}

void loop(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  accel.getEvent(&event);

  X = lastX * 0.5 + event.acceleration.x * 0.5;
  Y = lastY * 0.5 + event.acceleration.y * 0.5;
  
  /* Display the results (acceleration is measured in m/s^2) */
  //Serial.print("X: "); Serial.print(X); Serial.print(" ");
  //Serial.print("Y: "); Serial.print(Y); Serial.print("\n");
  
  if (reversing) {
    digitalWrite(BW_LED, HIGH);
    activate();
    getDistance();
  }
  else {
    digitalWrite(BW_LED, LOW);
    deactivate();
  }
  
  if (forwarding)
    digitalWrite(FW_LED, HIGH);
  else
    digitalWrite(FW_LED, LOW);
 
  if (Y > 1) {
    // backwards acceleration
    if (stopped) {
      Serial.print("Start reversing\n");
      reversing = true;
      stopped = false;
      timeout = TIMEOUT;   
    }
    else if (reversing) {
      timeout = TIMEOUT;
    }
    else if (forwarding) {
      timeout -= 40;
      if (timeout < 0) {
        forwarding = false;
        stopped = true;
      }
    }
  }
  else if (Y > -1) {
    // no acceleration
    if (reversing) {
      timeout--;
      if (timeout < 0) {
        reversing = false;
        stopped = true;
      }
    }
    if (forwarding) {
      timeout = TIMEOUT;
    }
  }
  else {
    // forward acceleration
    if (stopped) {
      Serial.print("Start forwarding\n");
      stopped = false;
      forwarding = true;
    }
    if (forwarding) {
      timeout = TIMEOUT;
    }
    if (reversing) {
      timeout -= 20;
      if (timeout < 0) {
        reversing = false;
        stopped = true;
      }
    }
  }

  if (timeout < 0)
    reversing = false;
   
  delay(10);
}
