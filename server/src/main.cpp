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
#define START_ADVERTISE 'A'
#define SET_TEMP_THRESHOLD 'T'
#define SET_HUMIDITY_THRESHOLD 'U'
#define SET_MOTOR_INTERVAL 'M'
#define SET_TEMP_TYPE 'P'
#define GET_CONFIG 'C'

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
bool disconnectedPrinted = true;
bool connectedPrinted = false;

// Timers
Ticker broadCastTicker;
Ticker sensorTicker;
Ticker wifiTicker;

// Other variables
char incomingPacket[255];

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

interval motorInterval = { 20, 25 };
char currentTemperatureType = CELSIUS;
int temperature_threshold = 20;
int humidity_threshold = 40;

// Temperature sensor
SimpleDHT11 dht11(D1);
byte temperature = 30;
byte humidity = 100;

void printConnected() {
	if (WiFi.status() != WL_CONNECTED)
		return;
	if (!connectedPrinted) {
		Serial.println("Connected to the WiFi network with IP address: ");
		myCurrentIPAddr = WiFi.localIP();
		Serial.println(myCurrentIPAddr);
		connectedPrinted = true;
	}
	disconnectedPrinted = false;
}

void printDisconnected() {
	if (WiFi.status() == WL_CONNECTED)
		return;
	if (!disconnectedPrinted) {
		Serial.println("Disconnected from the WiFi network");
		disconnectedPrinted = true;
	}
	connectedPrinted = false;
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

void turnOnMotor(int speed = 512) {
	digitalWrite(DIRA, HIGH);
	digitalWrite(DIRB, LOW);
	analogWrite(MOTORSPEED_PIN, speed);
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

	int motorSpeed = map(temperature, motorInterval.lower, motorInterval.upper, 0, 1023);
	Serial.printf("Motor speed: %d\n", motorSpeed);
	turnOnMotor(motorSpeed);
}

void setupMotor() {
	pinMode(MOTORSPEED_PIN, OUTPUT);
	pinMode(DIRA, OUTPUT);
	pinMode(DIRB, OUTPUT);
}

void setup() {
	Serial.begin(115200);
	setupLEDs();
	setupMotor();
	sensorTicker.attach(5, readSensors);
	broadCastTicker.attach(3, broadCastIP);
	WiFi.begin(ssid, password);
	server.begin(serverPort, 2u);
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
			parseIncomingPacket(incomingPacket);
		}
	}
}
