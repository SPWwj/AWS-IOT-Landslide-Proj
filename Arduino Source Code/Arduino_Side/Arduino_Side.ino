/**************** Pin mapping ****************/

/*
 * GY-85 IMU
 * VCC_IN  ->   5V
 * GND     ->   GND
 * SCL     ->   A5
 * SDA     ->   A4
 *
 * Rainfall detector
 * VCC_IN  ->   5V
 * GND     ->   GND
 * A0      ->   A0
 * D0      ->   NC
 *
 * Soil Moisture Sensor
 * VCC_IN  ->   5V
 * GND     ->   GND
 * A0      ->   A1
 * D0      ->   NC (no connect)
 */

/**************** Library include directives ****************/

#include "GY_85.h"
#include <Wire.h>
#include <SoftwareSerial.h>

/**************** Variable declarations ****************/

SoftwareSerial esp8266(2,4);

//GY85 IMU
GY_85 gy85;
int ax, ay ,az;
float gx, gy, gz;

// Rain and soil moisture values
int rainValue, soilMoistureValue;

/**************** Helper functions ****************/

void readSensors() {
  ax = gy85.accelerometer_x(gy85.readFromAccelerometer());
  ay = gy85.accelerometer_y(gy85.readFromAccelerometer());
  az = gy85.accelerometer_z(gy85.readFromAccelerometer());
  gx = gy85.gyro_x(gy85.readGyro());
  gy = gy85.gyro_y(gy85.readGyro());
  gz = gy85.gyro_z(gy85.readGyro());
  rainValue = analogRead(A0);
  soilMoistureValue = analogRead(A1);
}

/**************** Main Program ****************/

void setup() {
  // Setup ESP8266 serial port
  Serial.begin(9600);
  esp8266.begin(9600);

  // Setup GY855 IMU
  Wire.begin();
  delay(10);
  gy85.init();
  delay(10);

  // Setup onboard LED for status indication
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Wait for '.' character
  while (esp8266.available()) {
    if (esp8266.read() == '.') {
      // Read sensors
      readSensors();
      // Construct data and send payload
      esp8266.print(soilMoistureValue);
      esp8266.print(',');
      esp8266.print(rainValue);
      esp8266.print(',');
      esp8266.print(ax);
      esp8266.print(',');
      esp8266.print(ay);
      esp8266.print(',');
      esp8266.print(az);
      esp8266.print(',');
      esp8266.print(gx);
      esp8266.print(',');
      esp8266.print(gy);
      esp8266.print(',');
      esp8266.println(gz);

      // For debug only
      Serial.print(soilMoistureValue);
      Serial.print(',');
      Serial.print(rainValue);
      Serial.print(',');
      Serial.print(ax);
      Serial.print(',');
      Serial.print(ay);
      Serial.print(',');
      Serial.print(az);
      Serial.print(',');
      Serial.print(gx);
      Serial.print(',');
      Serial.print(gy);
      Serial.print(',');
      Serial.println(gz);

      // Blink LED to indicate that ESP8266 has requested data
      digitalWrite(LED_BUILTIN, HIGH);
      delay(0.5);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}
