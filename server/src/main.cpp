#include <Arduino.h>
#include <esp8266wifi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <Ticker.h>
#include <SimpleDHT.h>

// Commands
#define GET_TEMP 'G'
#define GET_HUMIDITY 'H'
#define STOP_ADVERTISE 'S'

// LED configuration
const auto TEMPERATURE_LED = D3;
const auto HUMIDITY_LED = D2;
// Motor configuration
const int MOTORSPEED_PIN = D5;
const int DIRA = D6;
const int DIRB = D8;
// WIFI configuration
const uint16_t serverPort = 6969u;
const char *ssid = "motorola-edge-40";
const char *password = "idontlikepm";
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
Ticker wifiTicker;

// Other variables
char incomingPacket[255];

// Current connfiguration
enum temperatureType {
	CELSIUS,
	FAHRENHEIT,
	KELVIN
};

struct interval {
	byte lower;
	byte upper;
};

temperatureType currentTemperatureType = CELSIUS;
interval temperatureInterval = { 20, 40 };
byte temperature_threshold = 30;
byte humidity_threshold = 50;

// Temperature sensor
SimpleDHT11 dht11(D1);
byte temperature = 30;
byte humidity = 100;

void connectToWiFi() {
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.println("Connecting to WiFi..");
		delay(1000);
	}
	Serial.println("Connected to the WiFi network with IP address: ");
	myCurrentIPAddr = WiFi.localIP();
	Serial.println(myCurrentIPAddr);
}

void broadCastIP() {
	if (!advertise || !WiFi.isConnected())
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

void turnOffMotor() {
	digitalWrite(DIRA, LOW);
	digitalWrite(DIRB, LOW);
	digitalWrite(MOTORSPEED_PIN, LOW);
}

void turnOnMotor() {
	digitalWrite(DIRA, HIGH);
	digitalWrite(DIRB, LOW);
	digitalWrite(MOTORSPEED_PIN, HIGH);
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

	turnOnMotor();
}

void setupMotor() {
	pinMode(MOTORSPEED_PIN, OUTPUT);
	pinMode(DIRA, OUTPUT);
	pinMode(DIRB, OUTPUT);
}

void setup() {
	Serial.begin(115200);
	connectToWiFi();
	setupLEDs();
	sensorTicker.attach(5, readSensors);
	broadCastTicker.attach(3, broadCastIP);
	server.begin(serverPort, 2u);
	setupMotor();
}

void parseIncomingPacket(const char *incomingPacket) {
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
