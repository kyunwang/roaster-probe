#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include "max6675.h"

using namespace websockets;

WebsocketsClient client;

void onMessageCallback(WebsocketsMessage message) {
	Serial.print("Got Message: ");
	Serial.println(message.data());
}

void onEventsCallback(WebsocketsEvent event, String data) {
	if (event == WebsocketsEvent::ConnectionOpened) {
		Serial.println("Connection Opened");
	}
	else if (event == WebsocketsEvent::ConnectionClosed) {
		Serial.println("Connection Closed");
	}
	else if (event == WebsocketsEvent::GotPing) {
		Serial.println("Got a Ping!");
	}
	else if (event == WebsocketsEvent::GotPong) {
		Serial.println("Got a Pong!");
	}
}


// NodeMCU ESP32 pins
int thermoSO = 12; // D6
int thermoCS = 15; // D8
int thermoCLK = 14; // D5

MAX6675 thermocouple(thermoCLK, thermoCS, thermoSO);

JsonDocument inputDoc;
JsonDocument outputDoc;
std::string outputMessage;

// ====

#define USE_SERIAL Serial

void connect() {
connect_wifi:
	Serial.println("Connecting to WiFi");
	WiFi.begin(WLAN_SSID, WLAN_PASS);
	delay(500);

	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}

	Serial.println("WiFi connected");

connect_host:
	// run callback when messages are received
	client.onMessage(onMessageCallback);

	// run callback when events are occuring
	client.onEvent(onEventsCallback);

	Serial.println("Connecting to server");

	// Connect to WS server
	if (!client.connect(SOCKET_SERVER, SOCKET_PORT, "/artisan")) {
		delay(1000);

		if (WiFi.status() != WL_CONNECTED) {
			Serial.println("Wifi disconnected");
			goto connect_wifi;
		}

		goto connect_host;
	}

	Serial.println();
	Serial.println();

	// Send a message
	outputDoc.clear();
	outputDoc["command"] = "message";
	outputDoc["message"] = "Connected Ping";
	char buffer[256];
	serializeJson(outputDoc, buffer);
	client.send(buffer);

	// Send a ping
	client.ping();
}


// =====  Loop + Setup below
void setup() {
	Serial.begin(9600);

	for (uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	// Connect to wifi & WS host
	connect();
	Serial.println("connected!");

	// Giving some additional time to initialise the MAX6675
	delay(500);
}

void loop() {
	if (!client.available()) {
		Serial.println("Disconnected - Attempting to reconnect");
		connect();
		delay(1000);

		return;
	}

	client.poll();

	Serial.print("C = ");
	Serial.println(thermocouple.readCelsius());

	// Prepare probe data
	outputDoc.clear();
	outputDoc["command"] = "setData";
	outputDoc["data"]["BT"] = thermocouple.readCelsius();

	char buffer[256];
	serializeJson(outputDoc, buffer);
	client.send(buffer);

	// For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
	delay(500);
}
