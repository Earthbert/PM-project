#pragma once

// Commands
#define GET_TEMP 'G'
#define GET_HUMIDITY 'H'
#define STOP_ADVERTISE 'S'
#define START_ADVERTISE 'A'
#define SET_TEMP_THRESHOLD 'T'
#define SET_HUMIDITY_THRESHOLD 'U'
#define SET_MOTOR_INTERVAL 'M'
#define SET_TEMP_TYPE 'P'
#define GET_CONFIG 'C'

void parseCommand(const char *incomingPacket);