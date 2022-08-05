#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128      // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
// declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int sensor_pin = A0;
float sensor_value;
float voltage_out;
float mv_per_kelvin = 10.0;    // LM335 proportional to temperature in Kelvin (ºK)  10mV/ºK
float input_voltage = 5000.0; // For better acc. measure the exact value

float temperatureC;
float temperatureK;

void setup() {
  pinMode(sensor_pin, INPUT);
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void loop() {
  sensor_value = analogRead(sensor_pin);
  voltage_out = (sensor_value * input_voltage) / 1024;

  // calculate temperature for LM335
  temperatureK = voltage_out / mv_per_kelvin;
  temperatureC = temperatureK - 273;

  print_value_to_console(temperatureK, temperatureC, voltage_out);
  print_value_to_screen(temperatureK, temperatureC);

  delay(1000);
}

void print_value_to_console(float _temperatureK, float _temperatureC, float _voltage_out) {

  Serial.print("  Temperature(K): ");
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
