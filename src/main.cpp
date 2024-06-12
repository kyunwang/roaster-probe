#include <Arduino.h>
#include "max6675.h"

int thermoSO = 12; // D6
int thermoCS = 15; // D8
int thermoCLK = 14; // D5

MAX6675 thermocouple(thermoCLK, thermoCS, thermoSO);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Serial.println("Max6675 testing");

  // Gicing some time to initialise teh MAX6675
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("C = ");
  Serial.println(thermocouple.readCelsius());
  // Serial.print("F = ");
  // Serial.println(thermocouple.readFahrenheit());

  // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
  delay(1000);
}
