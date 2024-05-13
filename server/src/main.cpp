#include <Arduino.h>
#include <esp8266wifi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <Ticker.h>

// Commands
#define GET_TEMP 'G'
#define STOP_ADVERTISE 'S'


// LED configuration
const auto TEMPERATURE_LED = D6;
const auto HUMIDITY_LED = D2;
// WIFI configuration
const uint16_t serverPort = 6969u;
const char *ssid = "motorola-edge-40";
const char *password = "idontlikepm";
const uint16_t broadcastInterval = 2u;
const char *broadcastMessage = "I am alive!";

// WIFI variables
IPAddress myCurrentIPAddr;
WiFiUDP udp;
WiFiServer server(serverPort);
WiFiClient client;
bool advertise = true;

// Timers
Ticker broadCastTicker;
Ticker sensorTicker;

// Other variables
char incomingPacket[255];

// Current connfiguration
enum temperatureType {
	CELSIUS,
	FAHRENHEIT,
	KELVIN
};

struct interval {
	double lower;
	double upper;
};

temperatureType currentTemperatureType = CELSIUS;
double temperature = 10;
double humidity = 50;
interval temperatureInterval = { 20, 40 };
interval humidityInterval = { 0, 100 };

int humidityLEDIntensity = 500;
int temperatureLEDIntensity = 500;

void connectToWiFi() {
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.println("Connecting to WiFi..");
	}
	Serial.println("Connected to the WiFi network with IP address: ");
	myCurrentIPAddr = WiFi.localIP();
	Serial.println(myCurrentIPAddr);
}

void broadCastIP() {
	if (!advertise)
		return;
	Serial.println("Broadcasting Alive message..");
	udp.beginPacket(WiFi.broadcastIP(), serverPort);
	udp.print(broadcastMessage);
	udp.endPacket();
}

void setupLEDs() {
	pinMode(TEMPERATURE_LED, OUTPUT);
	pinMode(HUMIDITY_LED, OUTPUT);
}

void readSensors() {
	analogWrite(TEMPERATURE_LED, temperatureLEDIntensity);
	analogWrite(HUMIDITY_LED, humidityLEDIntensity);
}

void setup() {
	Serial.begin(115200);
	connectToWiFi();
	broadCastTicker.attach(broadcastInterval, broadCastIP);
	server.begin(serverPort, 2u);
	setupLEDs();
	sensorTicker.attach(1, readSensors);
}

void parseIncomingPacket(const char *incomingPacket) {
	switch (incomingPacket[0]) {
	case GET_TEMP:
		Serial.println("Getting temperature");
		client.print("25C");
		break;
	case STOP_ADVERTISE:
		Serial.println("Stopping advertisement");
		advertise = false;
		client.print("Stopped advertisement");
		break;
	default:
		Serial.println("Unknown command");
		break;
	}
}

void loop() {
	if (!WiFi.isConnected()) {
		Serial.println("WiFi disconnected");
		connectToWiFi();
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
		if (recvLen > 0) {
			Serial.println(incomingPacket);
			parseIncomingPacket(incomingPacket);
		}
	}
}
