/*
  Testing connecting to WiFI and posting a msg to teams or discord
*/
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
const char server[] = "discord.com";
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const String discord_webhook = SECRET_WEBHOOK;
const String teams_webhook = SECRET_WEBHOOK_MS;
const String discord_tts = SECRET_TTS;

int port = 443;
int status = WL_IDLE_STATUS;     // the Wifi radio's status
WiFiSSLClient client;
HttpClient http_client = HttpClient(client, server, port);

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("[LOG]: Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);

  }
  
  Serial.println("[LOG]: You're connected to the network");  
  Serial.println("----------------------------------------");
  get_wifi_connection_info();
  Serial.println("----------------------------------------");
}

void loop() {
  
  String content;
  String content_type;
  String payload;
  
  Serial.print("[LOG]: Attempting to connect to: ");
  Serial.println(server);
 
  Serial.println("[LOG]: Making POST request");
  
  content_type  = "application/json";  
  content = "Test1";  
  payload = "{\"content\":\"" + content + "\", \"tts\":" + discord_tts + "}";
  Serial.print("[LOG]: Formatted payload: ");
  Serial.println(payload);
  
  post_webhook(payload, content_type, discord_webhook);

  Serial.println("[LOG]: Waiting 600 seconds");
  delay(600000);
  Serial.println("----------------------------------------");
}

void post_webhook(String content_, String content_type_, String webhook_) {

  http_client.post(webhook_, content_type_, content_);
  // read the status code and body of the response
  int statusCode = http_client.responseStatusCode();
  String response = http_client.responseBody();

  Serial.print("[HTTP]: Status code: ");
  Serial.println(statusCode);
  Serial.print("[HTTP]: Response: ");
  Serial.println(response);
}


void get_wifi_connection_info() {
  Serial.println("Board Information:");
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}
