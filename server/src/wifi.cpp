#include "wifi.h"

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

// Other variables
char incomingPacket[1024];

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