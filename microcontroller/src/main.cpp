#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Arduino.h>
#include "max6675.h"
#include <ArduinoJson.h>

#include <ArduinoWebsockets.h>

// Internal
using namespace websockets;

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

WebsocketsClient client;

int thermoSO = 12; // D6
int thermoCS = 15; // D8
int thermoCLK = 14; // D5

MAX6675 thermocouple(thermoCLK, thermoCS, thermoSO);

// Wifi init
WiFiClient wifiClient;


JsonDocument inputDoc;
JsonDocument outputDoc;
std::string outputMessage;

// ====

const char* subscriptionTopics[] = {
  "+/+/dummyData"
};

// enum class PublishTopics : char {
//   BeanTemp = 'probe/output/bean_temperature'
// };

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

	// Connect to server
	// if (!client.connect(MQTT_SERVER, MQTT_PORT, "/probe")) {
	// 	delay(1000);

	// 	if (WiFi.status() != WL_CONNECTED) {
	// 		Serial.println("Wifi disconnected");
	// 		goto connect_wifi;
	// 	}

	// 	goto connect_host;
	// }

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

	// Connect to wifi & host
	connect();

	Serial.println("Max6675 testing");

	// Serial.println(MQTT_SERVER);
	// Serial.println(MQTT_PORT);

	Serial.println("connected!");

	// Giving some additional time to initialise the MAX6675
	delay(500);
}

void loop() {
	if (!client.available()) {
		Serial.println("Disconnected - Attempting to reconnect");
		connect();
		delay(1000);
	}

	client.poll();

	Serial.print("C = ");
	Serial.println(thermocouple.readCelsius());

	// JsonDocument outDoc
	outputDoc.clear();
	outputDoc["command"] = "setData";
	outputDoc["BT"] = thermocouple.readCelsius();
	// outputDoc["ET"] = thermocouple.readCelsius();
	char buffer[256];
	serializeJson(outputDoc, buffer);
	client.send(buffer);

	// For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
	delay(500);
}
