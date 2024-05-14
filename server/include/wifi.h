#pragma once
#include <esp8266wifi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>

// WIFI configuration
extern const uint16_t serverPort;
extern const char *ssid;
extern const char *password;
extern const char *broadcastMessage;

// WIFI variables
extern IPAddress myCurrentIPAddr;
extern WiFiUDP udp;
extern WiFiServer server;
extern WiFiClient client;
extern bool advertise;

// Other variables
extern char incomingPacket[1024];

void printConnected();

void printDisconnected();

void broadCastIP();