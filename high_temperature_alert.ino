/*
  Testing connecting to WiFI and posting a msg to teams or discord
*/
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
const char server[] = "outlook.office.com";
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const String discord_webhook = SECRET_WEBHOOK;
const String teams_webhook = SECRET_WEBHOOK_MS;
const String discord_tts = SECRET_TTS;

int port = 443;
WiFiSSLClient client;
HttpClient http_client = HttpClient(client, server, port);

int status = WL_IDLE_STATUS;     // the Wifi radio's status

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  
  Serial.println("----------------------------------------");
  printData();
  Serial.println("----------------------------------------");
}

void loop() {
  
  // check the network connection once every 10 seconds:
 //delay(10000);
 //printData();
  Serial.println("making POST request");
  msg_send("Test1");

  Serial.println("Wait 60 seconds");
  delay(60000);
  Serial.println("----------------------------------------");
}

void msg_send(String content) {
  Serial.println("[HTTP] Connecting to: ");
  Serial.println(server);
  Serial.println("[HTTP] Message: " + content);
  //Serial.println("[HTTP] TTS: " + discord_tts);
  //http_client.post(discord_webhook, "application/json", "{\"content\":\"" + content + "\", \"tts\":" + discord_tts + "}");
  String content_json = "{\"text\":\"" + content + "\"}";
  Serial.println(content_json);
  http_client.post(discord_webhook, "application/json", content_json);
  // read the status code and body of the response
  int statusCode = http_client.responseStatusCode();
  String response = http_client.responseBody();

  Serial.print("[HTTP] Status code: ");
  Serial.println(statusCode);
  Serial.print("[HTTP] Response: ");
  Serial.println(response);
}

void printData() {
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
