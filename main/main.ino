#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Servo.h>

#define BW_LED 12
#define FW_LED 11
#define BLINK_LED 3
#define LF_SERVO 8
#define RT_SERVO 7
#define HC_TRIG_LF 10
#define HC_TRIG_RT 9
#define HC_ECHO_LF 5
#define HC_ECHO_RT 4
#define TIMEOUT 2000

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

Servo servo_l, servo_r;

bool reversing = false;
bool forwarding = false;
bool stopped = true;
float reg = 0;
float X = 0;
float Y = 0;
float lastX = 0;
float lastY = 0;
int timeout = TIMEOUT;
unsigned int blink_state = LOW;
unsigned long blink_interval = 300;
unsigned long previous = 0;

void setup(void) 
{
  Serial.begin(9600);
 
  while(!accel.begin()) {
    Serial.println("Erorr: no ADXL345 detected\n");
    delay(500);
  }
 
  /* Set the range */
  accel.setRange(ADXL345_RANGE_2_G);

  pinMode(BW_LED, OUTPUT);
  pinMode(FW_LED, OUTPUT);
  pinMode(BLINK_LED, OUTPUT);

  servo_l.attach(LF_SERVO);
  servo_l.write(0);
  servo_r.attach(RT_SERVO);
  servo_r.write(0);

  pinMode(HC_TRIG_LF, OUTPUT);
  pinMode(HC_TRIG_RT, OUTPUT);
  pinMode(HC_ECHO_LF, INPUT);
  pinMode(HC_ECHO_RT, INPUT);
}

void activate() {
  servo_l.write(90);
  servo_r.write(270);
}

void deactivate() {
  servo_l.write(0);
  servo_r.write(0);
}

float getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(1);
  digitalWrite(trig, HIGH);
  delayMicroseconds(5);
  digitalWrite(trig, LOW);

  float distance = pulseIn(echo, HIGH);

  //Serial.print("Distance = ");
  //Serial.print(distance);
  //Serial.print("\n");
  return distance;
}

void loop(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  accel.getEvent(&event);

  //X = lastX * 0.5 + event.acceleration.x * 0.5;
  Y = event.acceleration.y;
   
  //Serial.print("X: "); Serial.print(X); Serial.print(" ");
  //Serial.print("Y: "); Serial.print(Y); Serial.print("\n");
  
  if (reversing) {
    digitalWrite(BW_LED, HIGH);
    activate();
    float d1 = getDistance(HC_TRIG_LF, HC_ECHO_LF);
    float d2 = 10000;//getDistance(HC_TRIG_RT, HC_ECHO_RT);
    float m = min(d1, d2);

    if (m > 6000)
      blink_interval = 10000000;
    else if (m > 4000)
      blink_interval = 800;    
    else if (m > 3000)
      blink_interval = 500;
    else if (m > 2000)
      blink_interval = 300;
    else if (m > 1000)
      blink_interval = 200;
      
    Serial.println(blink_interval);

    unsigned long current = millis();
    if (current - previous >= blink_interval) {
      previous = current;

      if (blink_state == LOW) blink_state = HIGH;
      else blink_state = LOW;

      digitalWrite(BLINK_LED, blink_state);
    }
  }
  else {
    digitalWrite(BW_LED, LOW);
    deactivate();
  }
  
  if (forwarding) digitalWrite(FW_LED, HIGH);
  else digitalWrite(FW_LED, LOW);
  //Serial.print(timeout); Serial.print("\n");

  if (Y > 6) {
    forwarding = true;
    reversing = false;
    stopped = false;
    timeout = TIMEOUT;
  }
  else if (Y > 0.5) {
    // backwards acceleration
    if (stopped) {
      reversing = true;
      stopped = false;
      timeout = TIMEOUT;   
    }
    else if (reversing) {
      timeout = TIMEOUT;
    }
    else if (forwarding) {
      timeout -= 20;
      if (timeout < 0) {
        forwarding = false;
        stopped = true;
        timeout = TIMEOUT;
      }
    }
  }
  else if (Y > -0.5) {
    // no acceleration
    if (reversing) {
      timeout--;
      if (timeout < 0) {
        reversing = false;
        stopped = true;
        timeout = TIMEOUT;
      }
    }
    if (forwarding) {
      timeout = TIMEOUT;
    }
  }
  else {
    // forward acceleration
    if (stopped) {
      stopped = false;
      forwarding = true;
    }
    if (forwarding) {
      timeout = TIMEOUT;
    }
    if (reversing) {
      timeout -= 40;
      if (timeout < 0) {
        reversing = false;
        stopped = true;
        timeout = TIMEOUT;
      }
    }
  }
}
