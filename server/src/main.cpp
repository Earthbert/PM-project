#include <Arduino.h>
#include <Ticker.h>
#include "wifi.h"
#include "commands.h"
#include "outputs.h"
#include "sensor.h"

void setup() {
	Serial.begin(115200);
	setupLEDs();
	setupMotor();
	sensorTicker.attach(5, readSensors);
	broadCastTicker.attach(3, broadCastIP);
	WiFi.begin(ssid, password);
	server.begin(serverPort, 2u);
}

void loop() {
	if (WiFi.isConnected()) {
		printConnected();
	} else {
		printDisconnected();
	}

	if (!client) {
		if (advertise == false) {
			advertise = true;
			Serial.println("Starting advertisement");
		}
		client = server.accept();
		if (client) {
			Serial.println("Client connected");
			client.print("Welcome to the server");
			client.print(currentTemperatureType);
		}
	}

	if (client) {
		WiFiClient newClient = server.accept();
		if (newClient) {
			newClient.println("Only one client allowed at a time");
			newClient.stop();
		}
	}

	if (client) {
		if (!client.connected()) {
			Serial.println("Client disconnected");
			client.stop();
		}
		int recvLen = client.read((uint8_t *)incomingPacket, sizeof(incomingPacket));
		incomingPacket[recvLen] = 0;
		if (recvLen > 0) {
			Serial.println(incomingPacket);
			parseCommand(incomingPacket);
		}
	}
}
