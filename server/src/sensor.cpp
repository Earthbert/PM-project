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

static int fitInterval(int value, int in_min, int in_max, int out_min, int out_max) {
	if (value < in_min) {
		return out_min;
	}
	if (value > in_max) {
		return out_max;
	}
	float slope = 1.0 * (out_max - out_min) / (in_max - in_min);
	return out_min + slope * (value - in_min);
}

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

	int motorSpeed = fitInterval(temperature, motorInterval.lower, motorInterval.upper, 0, 1023);
	Serial.printf("Motor speed: %d\n", motorSpeed);
	turnOnMotor(motorSpeed);
}