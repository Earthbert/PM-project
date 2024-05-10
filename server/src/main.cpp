#include <Arduino.h>
#include <esp8266wifi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <Ticker.h>

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
Ticker broadCastTicker;
WiFiServer server(serverPort);

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
	server.begin(serverPort, 1u);
}

void loop() {
	if (!WiFi.isConnected()) {
		Serial.println("WiFi disconnected");
		connectToWiFi();
	}

	WiFiClient client = server.accept();
	if (client) {
		Serial.println("New client connected");
		client.println("Hello from ESP8266");
		client.stop();
	}
}
