#include "commands.h"
#include "wifi.h"
#include "sensor.h"

void parseCommand(const char *incomingPacket) {
	switch (incomingPacket[0]) {
	case GET_TEMP:
		Serial.println("Getting temperature");
		client.print(temperature);
		break;
	case GET_HUMIDITY:
		Serial.println("Getting humidity");
		client.print(humidity);
		break;
	case STOP_ADVERTISE:
		Serial.println("Stopping advertisement");
		advertise = false;
		client.print("Stopped advertisement");
		break;
	case START_ADVERTISE:
		Serial.println("Starting advertisement");
		advertise = true;
		client.print("Started advertisement");
		break;
	case SET_TEMP_THRESHOLD:
		Serial.println("Setting temperature threshold");
		sscanf(incomingPacket + 1, "%d", &temperature_threshold);
		client.print(temperature_threshold);
		break;
	case SET_HUMIDITY_THRESHOLD:
		Serial.println("Setting humidity threshold");
		sscanf(incomingPacket + 1, "%d", &humidity_threshold);
		client.print(humidity_threshold);
		break;
	case SET_MOTOR_INTERVAL:
		Serial.println("Setting motor interval");
		sscanf(incomingPacket + 1, "%d_%d", &motorInterval.lower, &motorInterval.upper);
		client.printf("%d_%d", motorInterval.lower, motorInterval.upper);
		break;
	case SET_TEMP_TYPE:
		Serial.println("Setting temperature type");
		sscanf(incomingPacket + 1, "%c", &currentTemperatureType);
		client.printf("%c", currentTemperatureType);
		break;
	case GET_CONFIG:
		Serial.println("Getting configuration");
		char temperatureType[20];
		switch (currentTemperatureType)
		{
			case CELSIUS:
				strcpy(temperatureType, "Celsius");
				break;
			case FAHRENHEIT:
				strcpy(temperatureType, "Fahrenheit");
				break;
			case KELVIN:
				strcpy(temperatureType, "Kelvin");
				break;
		}
		client.printf("Temperature threshold: %d\n"
			"Humidity threshold: %d\n"
			"Motor interval: %d - %d\n"
			"Curren temperature type: %s\n",
			temperature_threshold, humidity_threshold,
			motorInterval.lower, motorInterval.upper, temperatureType);
		break;
	default:
		Serial.println("Unknown command");
		break;
	}
}