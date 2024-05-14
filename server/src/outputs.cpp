#include "outputs.h"
#include <Arduino.h>
#include <Ticker.h>

// LED configuration
const int TEMPERATURE_LED = D3;
const int HUMIDITY_LED = D2;

// Motor configuration
const int MOTORSPEED_PIN = D5;
const int DIRA = D6;
const int DIRB = D8;

// Timers
Ticker broadCastTicker;
Ticker sensorTicker;
Ticker wifiTicker;

void setupLEDs() {
	pinMode(TEMPERATURE_LED, OUTPUT);
	pinMode(HUMIDITY_LED, OUTPUT);
}

void turnOffMotor() {
	digitalWrite(DIRA, LOW);
	digitalWrite(DIRB, LOW);
	digitalWrite(MOTORSPEED_PIN, LOW);
}

void turnOnMotor(int speed) {
	digitalWrite(DIRA, HIGH);
	digitalWrite(DIRB, LOW);
	analogWrite(MOTORSPEED_PIN, speed);
}

void setupMotor() {
	pinMode(MOTORSPEED_PIN, OUTPUT);
	pinMode(DIRA, OUTPUT);
	pinMode(DIRB, OUTPUT);
}
