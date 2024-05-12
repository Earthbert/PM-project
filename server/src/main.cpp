#include <Arduino.h>
#include <esp8266wifi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <Ticker.h>

// Commands
#define GET_TEMP "GET_TEMPERATURE"

// LED configuration
const auto RED_LED = D6;
const uint16_t BLINK_INTERVAL = 500u;
// WIFI configuration
const uint16_t serverPort = 6969u;
const uint16_t broadcastPort = 6970u;
const char *ssid = "motorola-edge-40";
const char *password = "idontlikepm";
const uint16_t broadcastInterval = 2u;
const char *broadcastMessage = "I am alive!";

// WIFI variables
IPAddress myCurrentIPAddr;
WiFiUDP udp;
WiFiServer server(serverPort);
WiFiClient client;

// Timers
Ticker broadCastTicker;

// Other variables
char incomingPacket[255];

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
	Serial.println("Broadcasting Alive message..");
	udp.beginPacket(WiFi.broadcastIP(), broadcastPort);
	udp.print(broadcastMessage);
	udp.endPacket();
}

void setup() {
	Serial.begin(115200);
	connectToWiFi();
	broadCastTicker.attach(broadcastInterval, broadCastIP);
	server.begin(serverPort, 2u);
}

void parseIncomingPacket(const char *incomingPacket) {
	if (strncmp(incomingPacket, GET_TEMP, strlen(GET_TEMP)) == 0) {
		Serial.println("GET_TEMP command received");
		client.print("25");
	}
}

void loop() {
	if (!WiFi.isConnected()) {
		Serial.println("WiFi disconnected");
		connectToWiFi();
	}

	if (!client) {
		client = server.accept();
		if (client) {
			Serial.println("Client connected");
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
