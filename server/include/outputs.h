#pragma once
#include <Ticker.h>
#include <Arduino.h>

// LED configuration
extern const int TEMPERATURE_LED;
extern const int HUMIDITY_LED;

// Motor configuration
extern const int MOTORSPEED_PIN;
extern const int DIRA;
extern const int DIRB;

// Timers
extern Ticker broadCastTicker;
extern Ticker sensorTicker;
extern Ticker wifiTicker;

void setupLEDs();

void turnOffMotor();

void turnOnMotor(int speed);

void setupMotor();
