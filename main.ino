#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#define LED_PIN 12
#define TIMEOUT 30
 
/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

bool reversing = false;
int timeout = TIMEOUT;
float reg = 0;

void setup(void) 
{
  Serial.begin(9600);
 
  while(!accel.begin()) {
    Serial.println("Erorr: no ADXL345 detected\n");
    delay(1000);
  }
 
  /* Set the range */
  accel.setRange(ADXL345_RANGE_2_G);

  pinMode(LED_PIN, OUTPUT);
  int i;
  float sum = 0.0;
  for (i = 0; i < 10; i++)
    sum += getY();

  reg = sum / 12.0;
}

float getY() {
  sensors_event_t event; 
  accel.getEvent(&event);
  float y = event.acceleration.y - reg;
  Serial.print(y); Serial.print("\n");
  return y;
}


void loop(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  accel.getEvent(&event);
 
  /* Display the results (acceleration is measured in m/s^2) */
  //Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print(" ");
  float y = getY();
  
  if (reversing)
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);

  if (y > 2 && not reversing) {
    reversing = true;
    timeout = TIMEOUT;
  }
  else if (y > 2 && reversing) {
    timeout = TIMEOUT;
  }
  else if (y < 1 && reversing) {
    timeout -= 1;
  }

  if (timeout < 0)
    reversing = false;
   
  delay(100);
}
