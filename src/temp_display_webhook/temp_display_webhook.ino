/*
  Testing connecting to WiFI and posting a msg to teams or discord
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <ArduinoMqttClient.h>
#include <NTPClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

//////////////// Screen Setup ////////////////////////////////////////////////////////

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
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
String content;
String content_type;
String payload;

int port = 443;
int status = WL_IDLE_STATUS; // the Wifi radio's status
WiFiSSLClient client;
HttpClient http_client = HttpClient(client, server, port);

////////////// MQTT setup ///////////////////////////////////////////

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "test.mosquitto.org";
int broker_port = 1883;
const char topic[] = "KA/LAB/DO/TEMP";

int counter = 0;
int mqtt_con;
/////////////Sensor set up///////////////////////////////////////////

unsigned long sum_sensor_value;
unsigned long average_sensor_value;
const int num_readings = 1000; // average  number of readings

const int sensor_pin = A0;
float sensor_value;
float voltage_out;
float mv_per_kelvin = 10.0;   // LM335 proportional to temperature in Kelvin (ºK)  10mV/ºK
float input_voltage = 5000.0; // For better acc. measure the exact value

float temperatureC;
float temperatureK;
float temp_offset = 157; //Set to zero if calibrated
float ad_converter = 1023;
float temp_alert = -10; // Temperature value to send alert

unsigned long loop_timer;
unsigned long screen_refresh = 1000;         // Refresh rate oled screen ms
unsigned long interval_sending_data = 60000; // Interval for sending data ms
unsigned long elapsed_time;                  // Elasped time intialized with random number

StaticJsonDocument<200> jsonBuffer;
JsonObject doc = jsonBuffer.to<JsonObject>();

/////////////////// Time setup /////////////////////

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "dk.pool.ntp.org");

void setup()
{
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  // Initilaze pin
  delay(100); //"Stabilization time".   Probably not necessary
  pinMode(sensor_pin, INPUT);
  delay(100); //"Stabilization time".   Probably not necessary

  
  delay(100); //"Stabilization time".   Probably not necessary
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  delay(100);
  display.ssd1306_command(SSD1306_DISPLAYOFF); // To switch display off
  delay(100);
  display.ssd1306_command(SSD1306_DISPLAYON); // To switch display back on
  delay(100);
  Serial.print("[LOG]: Loading display");
  display.display();
  delay(100);
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  Serial.println("[LOG]: Display color set");
  
  // Connect to Wifi network
  while (!Serial)
    ;

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED)
  {
    Serial.print("[LOG]: Attempting to connect to network: ");
    Serial.println(ssid); 
    print_setup_to_screen("Attempting to connect to network " + String(ssid) + " ...", 2000);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 2 seconds for connection:
    delay(2000);

    // Initialize a NTPClient to get time
    timeClient.begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    timeClient.setTimeOffset(7200);
  }

  print_setup_to_screen("Success connected to network " + String(ssid) + " ...", 2000);

  Serial.println("[LOG]: You're connected to the network");
  Serial.println("----------------------------------------");
  get_wifi_connection_info();
  Serial.println("----------------------------------------");
  Serial.print("[LOG]: Attempting to connect to the MQTT broker: ");
  Serial.println(broker);
  
  
  mqtt_con = mqttClient.connect(broker, broker_port);

  while (!mqtt_con && counter < 3 )
  {
    Serial.print("[ERROR] MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    print_setup_to_screen("[ERROR] MQTT connection failed! ...", 5000);
  
    // wait 2 seconds for connection:
    counter = counter +1;
    delay(2000);
  }

  if (mqtt_con = 1) {
  print_setup_to_screen("Success connected to MQTT broker!  " + String(broker) + " ...", 5000);

  Serial.println("[LOG]: You're connected to the MQTT broker!");
  Serial.println();
  Serial.println("----------------------------------------");
  }
}

void loop()
{
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);

  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);

  // call poll() regularly to allow the library to send MQTT keep alive which
  // avoids being disconnected by the broker
  mqttClient.poll();
  loop_timer = millis(); // Start loop timer ms
  elapsed_time = millis();
  // Screen refresh loop
  while (elapsed_time - loop_timer < interval_sending_data)
  {
    sum_sensor_value = 0; //Initialize/reset
    //Smoothing sensor data. Take num_readings, find  average.
    for (int i = 0; i < num_readings; i++)
    {
      // Get sensor value
      sensor_value = analogRead(sensor_pin);
      sum_sensor_value = sum_sensor_value + sensor_value;
    }

    average_sensor_value = (sum_sensor_value / num_readings);
    voltage_out = (average_sensor_value * input_voltage) / ad_converter;

    // calculate temperature for LM335
    temperatureK = (voltage_out / mv_per_kelvin) - temp_offset;
    temperatureC = temperatureK - 273;

    print_value_to_console(temperatureK, temperatureC, voltage_out);
    print_value_to_screen(formattedTime.substring(0, 5), temperatureC);
    delay(screen_refresh);
    elapsed_time = millis();
  }
  if (mqttClient.connected()){
  Serial.println("----------------------------------------");
  Serial.print("[LOG]: Sending message to topic: ");
  Serial.println(topic);
   // TODO add exception handling
  // Publish the msg 
  
  doc["Alias"] = "Temp_Room";
  doc["Timestamp"] = epochTime;
  doc["Value"] = temperatureC;
  send_mqtt_as_json(doc);
  }
  // Send alert to discord
  if (temperatureC >= temp_alert)
  {
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
}

void post_webhook(String content_, String content_type_, String webhook_)
{

  http_client.post(webhook_, content_type_, content_);
  // read the status code and body of the response
  int statusCode = http_client.responseStatusCode();
  String response = http_client.responseBody();

  Serial.print("[HTTP]: Status code: ");
  Serial.println(statusCode);
  Serial.print("[HTTP]: Response: ");
  Serial.println(response);
}

void get_wifi_connection_info()
{
  byte mac[6];  // the MAC address of your Wifi Module
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
}

void print_value_to_console(float _temperatureK, float _temperatureC, float _voltage_out)
{

  Serial.print("[LOG]: Temperature(K): ");
  Serial.print(_temperatureK);
  Serial.print("  Temperature(ºC): ");
  Serial.print(_temperatureC);
  Serial.print("  Voltage(mV): ");
  Serial.println(_voltage_out);
}

void print_value_to_screen(String _time, float _temperatureC)
{

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

  display.display();
}

void print_setup_to_screen(String _status, int _delay)
{

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

void send_mqtt_as_json(JsonObject &data)
{
  String dataStr = "";
  serializeJson(data, dataStr);
  mqttClient.beginMessage(topic);
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
