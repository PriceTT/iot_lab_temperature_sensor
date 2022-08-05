/*
Connect to WiFI, post temperature to console and discord via webhook
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

//////////////// Screen Setup ////////////////////////////////////////////////////////

#define SCREEN_WIDTH 128      // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
// declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

///////////// Webhook setup ///////////////////////////////////////////////////
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

/////////////Sensor set up///////////////////////////////////////////

const int sensor_pin = A0;
float sensor_value;
float voltage_out;
float mv_per_kelvin = 10.0;    // LM335 proportional to temperature in Kelvin (ºK)  10mV/ºK
float input_voltage = 5000.0; // For better acc. measure the exact value

float temperatureC;
float temperatureK;
float temp_offset = 157;     //Set to zero if calibrated
float ad_converter = 1023;

unsigned long loop_timer;
unsigned long  screen_refresh = 1000;            // Refresh rate oled screen ms
unsigned long  interval_sending_data = 60000;   // Interval for sending data ms
unsigned long  elapsed_time;                   // Elasped time intialized with random number


void setup() {
  // Initilaze pin
  pinMode(sensor_pin, INPUT);

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  // Connect to Wifi network
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

  loop_timer = millis();       // Start loop timer ms
  elapsed_time = millis();
  while (elapsed_time - loop_timer < interval_sending_data) {
    // Get sensor value
    sensor_value = analogRead(sensor_pin);
    voltage_out = (sensor_value * input_voltage) / ad_converter;

    // calculate temperature for LM335
    temperatureK = (voltage_out / mv_per_kelvin) - temp_offset;
    temperatureC = temperatureK - 273;

    print_value_to_console(temperatureK, temperatureC, voltage_out);
    print_value_to_screen(temperatureK, temperatureC);
    delay(screen_refresh);
    elapsed_time = millis();
  }

  // Send to discord

  Serial.print("[LOG]: Attempting to connect to: ");
  Serial.println(server);

  Serial.println("[LOG]: Making POST request");

  content_type  = "application/json";
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

void print_value_to_console(float _temperatureK, float _temperatureC, float _voltage_out) {

  Serial.print("[LOG]: Temperature(K): ");
  Serial.print(_temperatureK);
  Serial.print("  Temperature(ºC): ");
  Serial.print(_temperatureC);
  Serial.print("  Voltage(mV): ");
  Serial.println(_voltage_out);

}

void print_value_to_screen(float _temperatureK, float _temperatureC) {

  // clear display
  display.clearDisplay();

  // display temperature Celsius
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(temperatureC);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // display temperature Kelvin
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(temperatureK);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("K");

  display.display();

}
