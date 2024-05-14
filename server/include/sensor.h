#pragma once
#include <SimpleDHT.h>
#include <Arduino.h>

// Current connfiguration
enum temperatureType : char {
	CELSIUS = 'C',
	FAHRENHEIT = 'F',
	KELVIN = 'K'
};

struct interval {
	int lower;
	int upper;
};

extern interval motorInterval;
extern char currentTemperatureType;
extern int temperature_threshold;
extern int humidity_threshold;

// Temperature sensor
extern SimpleDHT11 dht11;
extern byte temperature;
extern byte humidity;

void readSensors();
