#include "sensor.h"
#include "outputs.h"
#include <SimpleDHT.h>

interval motorInterval = { 20, 25 };
char currentTemperatureType = CELSIUS;
int temperature_threshold = 20;
int humidity_threshold = 40;

// Temperature sensor
SimpleDHT11 dht11(D1);
byte temperature = 30;
byte humidity = 100;

void readSensors() {
	auto ret = dht11.read(&temperature, &humidity, NULL);
	if (ret != SimpleDHTErrSuccess) {
		Serial.print("Read DHT11 failed, err=");
		Serial.println(ret);
	}
	Serial.print("Temperature: ");
	Serial.print(temperature);
	Serial.print(" Humidity: ");
	Serial.println(humidity);

	if (temperature >= temperature_threshold) {
		digitalWrite(TEMPERATURE_LED, HIGH);
	} else {
		digitalWrite(TEMPERATURE_LED, LOW);
	}
	if (humidity >= humidity_threshold) {
		digitalWrite(HUMIDITY_LED, HIGH);
	} else {
		digitalWrite(HUMIDITY_LED, LOW);
	}

	int motorSpeed = map(temperature, motorInterval.lower, motorInterval.upper, 0, 1023);
	Serial.printf("Motor speed: %d\n", motorSpeed);
	turnOnMotor(motorSpeed);
}