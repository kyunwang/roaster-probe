#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include "max6675.h"

using namespace websockets;

WebsocketsServer server;

MAX6675 thermocouple(PIN_CLK, PIN_CS, PIN_SO);

JsonDocument inputDoc;
JsonDocument outputDoc;
std::string outputMessage;

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

init_server:
  Serial.println("Initialising Server");

  server.listen(8080);

  if (!server.available()) {
    delay(1000);
    Serial.println("Retrying server init");
    goto init_server;
  }
}

void setup() {
  Serial.begin(9600);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  // Connect to wifi & WS host
  connect();
  Serial.println("connected!");
  Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP

  // Serial.println(PIN_CLK);
  // Serial.println(PIN_CS);
  // Serial.println(PIN_SO);

  // Giving some additional time to initialise the MAX6675
  delay(500);
}

void loop() {
  Serial.print("C = ");
  Serial.println(thermocouple.readCelsius());

  WebsocketsClient client = server.accept();

  if (!client.available()) {
    Serial.println("No client connected");
    delay(500);
    return;
  }

  auto message = client.readBlocking();

  Serial.print("Got Message: ");
  Serial.println(message.data());

  // Parse message
  deserializeJson(inputDoc, message.data());

  if (inputDoc["command"] == "getData") {
    int requestID = inputDoc["id"];

    // Prepare probe data
    outputDoc.clear();
    outputDoc["id"] = requestID;
    outputDoc["data"]["BT"] = thermocouple.readCelsius();
    // outputDoc["data"]["ET"] = random(40, 140);

    char buffer[256];
    serializeJson(outputDoc, buffer);
    client.send(buffer);
  }

  // client.close();

  delay(500);
}
