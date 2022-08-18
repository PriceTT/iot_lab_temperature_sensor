/*
  Testing connecting to WiFI and posting a msg to teams or discord
*/
#include <Wire.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <ArduinoMqttClient.h>
#include <NTPClient.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include "arduino_secrets.h"

//////////////////////////////// Screen Setup /////////////////////////////////////////////

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
// declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/////////////////////////////// Webhook and Secrets setup //////////////////////////////////
// Please enter your sensitive data in the Secret tab/arduino_secrets.h
const char server[] = "discord.com";
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const String discord_webhook = SECRET_WEBHOOK;
const String teams_webhook = SECRET_WEBHOOK_MS;
const String discord_tts = SECRET_TTS;
const char broker[] = SECRET_BROKER;
const char *certificate = SECRET_CERTIFICATE;
String content;
String content_type;
String payload;
byte mac[6];

int discord_port = 443;
int status = WL_IDLE_STATUS;  // the Wifi radio's status
//WiFiSSLClient client;
//HttpClient http_client = HttpClient(client, server, discord_port);

// https://www.ascii-codes.com/ connected 3 (♥), unconnected 21 (§)
int connected_internet = 3;
int connected_mqtt = 3;

//////////////////////////////// MQTT setup ///////////////////////////////////////////

WiFiClient wifiClient;                // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient);  // Used for SSL/TLS connection, integrates with ECC508
MqttClient mqttClient(sslClient);
WiFiSSLClient client;  //ssl for discord
HttpClient http_client = HttpClient(client, server, discord_port);
int broker_port = 8883;
const String publish_topic = "KA/LAB/DO/TEMP";
int counter = 0;
////////////////////////////// Sensor set up ///////////////////////////////////////////

unsigned long sum_sensor_value;
unsigned long average_sensor_value;
const int num_readings = 1000;  // average  number of readings

const int sensor_pin = A0;
float sensor_value;
float voltage_out;
float mv_per_kelvin = 10.0;    // LM335 proportional to temperature in Kelvin (ºK)  10mV/ºK
float input_voltage = 5000.0;  // For better acc. measure the exact value

float temperatureC;
float temperatureK;
float temp_offset = 157;  //Set to zero if calibrated
float ad_converter = 1023;
float temp_alert = 30;  // Temperature value to send alert

unsigned long loop_timer;
unsigned long screen_refresh = 1000;          // Refresh rate oled screen ms
unsigned long interval_sending_data = 10000;  // Interval for sending data ms
unsigned long elapsed_time;                   // Elasped time intialized with random number

time_t epochTime;
String formattedTime;
const int gmt_offset = 7200;

StaticJsonDocument<200> jsonBuffer;
JsonObject doc = jsonBuffer.to<JsonObject>();

/////////////////// Time setup /////////////////////

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "dk.pool.ntp.org");

void setup() {
  //Initialize serial
  Serial.begin(9600);

  // Initilaze pin
  delay(100);  //"Stabilization time".   Probably not necessary
  pinMode(sensor_pin, INPUT);
  delay(100);  //"Stabilization time".   Probably not necessary

  initialize_oled();
  delay(100);
  validate_server_certificate();


  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    initialize_ntpclient();
  }

  Serial.println("----------------------------------------");
  get_wifi_connection_info();
  Serial.println("----------------------------------------");
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    initialize_ntpclient();
  }

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();

  loop_timer = millis();  // Start loop timer ms
  elapsed_time = millis();
  // Screen refresh loop
  while (elapsed_time - loop_timer < interval_sending_data) {

    calculate_average_temperature(temperatureC, voltage_out);
    print_value_to_console(temperatureC, voltage_out);
    get_current_time(epochTime, formattedTime);
    print_value_to_screen(formattedTime.substring(0, 5), temperatureC, connected_internet, connected_mqtt);

    elapsed_time = millis();
  }

  // Publish json payload
  doc["Alias"] = "Temp_Room";
  doc["Timestamp"] = epochTime;
  doc["Value"] = temperatureC;
  publishMessage(doc);

  // TODO add discord refresh loop
  // Send alert to discord
  if (temperatureC >= temp_alert && WiFi.status() == WL_CONNECTED) {
    send_message_to_discord();
  }
}

void initialize_oled() {
  delay(100);  //"Stabilization time".   Probably not necessary
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  // Show initial display buffer contents on the screen with default splash screen --
  display.display();
  delay(1000);
  display.ssd1306_command(SSD1306_DISPLAYOFF);  // To switch display off
  delay(100);
  display.ssd1306_command(SSD1306_DISPLAYON);  // To switch display back on
  display.clearDisplay();
  display.setTextColor(WHITE);
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

void send_message_to_discord() {

  Serial.println("----------------------------------------");
  Serial.print("[LOG]: Temperature greater than ");
  Serial.print(String(temp_alert));
  Serial.println("ºC ... sending alert.");
  Serial.print("[LOG]: Attempting to connect to: ");
  Serial.println(server);
  Serial.println("[LOG]: Making POST request");

  content_type = "application/json";
  content = String(temperatureC, 2);
  payload = "{\"content\":\"" + content + "\", \"tts\":" + discord_tts + "}";
  Serial.print("[LOG]: Formatted payload: ");
  Serial.println(payload);

  post_webhook(payload, content_type, discord_webhook);

  Serial.print("[LOG]: Waiting ");
  Serial.print(String(interval_sending_data / 1000));
  Serial.println(" seconds before next transmission.");
  Serial.println("----------------------------------------");
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

  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
  Serial.println();
}

void print_value_to_screen(String _time, float _temperatureC, int _connected_internet, int _connected_mqtt) {

  // clear display
  display.clearDisplay();

  // display Time
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Time: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(_time);

  // display temperature Celsius
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(temperatureC);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // display connection status
  display.setTextSize(1);
  display.setCursor(115, 0);
  display.cp437(true);
  display.write(_connected_internet);
  display.cp437(true);
  display.write(_connected_mqtt);
  

  display.display();
}

void print_setup_to_screen(String _status, int _delay) {

  // clear display
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Status: ");
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print(_status);
  display.display();
  delay(_delay);
}

void send_mqtt_as_json(JsonObject &data) {
  String dataStr = "";
  serializeJson(data, dataStr);
  mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print(dataStr);
  mqttClient.endMessage();

  //httpClient.beginRequest();
  //httpClient.post(path);
  //httpClient.sendHeader("Content-Type", "application/json");
  //httpClient.sendHeader("Content-Length", dataStr.length());
  //httpClient.sendHeader("Authorization", "Bearer "  +    String(device_secret_key));
  //httpClient.beginBody();
  //httpClient.print(dataStr);
  //httpClient.endRequest();
  //client.flush();
}

unsigned long getTime() {
  // get the current time from the WiFi module
  return WiFi.getTime();
}

void validate_server_certificate() {

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1)
      ;
  }

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  sslClient.setEccSlot(0, certificate);
  mqttClient.onMessage(onMessageReceived);
}

void initialize_ntpclient() {
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(gmt_offset);
}


void connectWiFi() {
  String _status = "Connecting to Wifi " + String(ssid) + " .";
  Serial.print("[LOG]: Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  print_setup_to_screen(_status, 100);

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    _status = _status + ".";
    Serial.print(".");
    print_setup_to_screen(_status, 100);
    delay(5000);
  }
  

 
 
  Serial.println();

  Serial.println("[LOG]: You're connected to the network");
  Serial.println();
  print_setup_to_screen("Success: Connected to Wifi " + String(ssid), 2000);
}

void connectMQTT() {
  String _status = "Connecting to MQTT broker!  " + String(broker) + " .";
  Serial.print("[LOG]: Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.print(" ");
  print_setup_to_screen(_status, 2000);

  while (!mqttClient.connect(broker, broker_port)) {
    // failed, retry
    _status = _status + ".";
    Serial.print(".");
    print_setup_to_screen(_status, 100);
    delay(5000);
  }
  Serial.println();

  Serial.println("[LOG]: You're connected to the MQTT broker");
  Serial.println();
  print_setup_to_screen("Success connected to MQTT broker!  " + String(broker), 2000);

  // subscribe to a topic
  mqttClient.subscribe("arduino/incoming");
}

void publishMessage(JsonObject &data) {

  String dataStr = "";
  serializeJson(data, dataStr);
  Serial.println("[LOG]: Publishing message" + dataStr);
  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(publish_topic);
  mqttClient.print(dataStr);
  mqttClient.endMessage();
}

void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("[LOG]: Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();
}

void calculate_average_temperature(float &_temperatureC, float &_voltage_out) {
  //Initialize/reset
  sum_sensor_value = 0;
  //Smoothing sensor data. Take num_readings, find  average.
  for (int i = 0; i < num_readings; i++) {
    // Get sensor value
    sensor_value = analogRead(sensor_pin);
    sum_sensor_value = sum_sensor_value + sensor_value;
  }

  average_sensor_value = (sum_sensor_value / num_readings);
  _voltage_out = (average_sensor_value * input_voltage) / ad_converter;

  // calculate temperature for LM335
  temperatureK = (_voltage_out / mv_per_kelvin) - temp_offset;
  _temperatureC = temperatureK - 273;
}

void get_current_time(time_t &_epochTime, String &_formattedTime) {

  timeClient.update();
  _epochTime = timeClient.getEpochTime();
  _formattedTime = timeClient.getFormattedTime();
  //Serial.print("[LOG]: Epoch time: ");
  //Serial.print(_epochTime);
  //Serial.print("  Formatted time: ");
  //Serial.println(_formattedTime);
}

void print_value_to_console(float _temperatureC, float _voltage_out) {
  Serial.print("[LOG]: Temperature(ºC): ");
  Serial.print(_temperatureC);
  Serial.print("  Voltage(mV): ");
  Serial.println(_voltage_out);
}