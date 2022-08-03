# Iot Lab Temperature Sensor

IOT sensor to monitor the cooling room temperature.
* High tempureature alert is sent via webhook to MS TEAMS
* Log the values to the PI historian

**Pinout Arduino MKR WIFI 1010**  
<p align="center">
  <img src="Pinout-MKRwifi1010_latest.png" >
</p>


## Parts List  

- [Arduino MKR WIFI 1010](https://www.computersalg.dk/i/4892362/arduino-mkr-wifi-1010-arm-cortex-m0-48-mhz-0-256-mb-32-kb-arduino-25-x-61-5-mm)  
- [LM335A - Precision Temperature Sensor](https://elektronik-lavpris.dk/p100429/lm335az-temp-sensor-40-100c-to92-05)  
- [OLED SPI Display SSD1306](https://elektronik-lavpris.dk/p143602/modu0052-ssd1306-128x64-pixel-uoled-display-module-blue/)

## Getting started  
- https://www.arduino.cc/en/Guide/MKRWiFi1010/connecting-sensors  
- https://randomnerdtutorials.com/arduino-lm35-lm335-lm34-temperature-sensor/
- https://elektronik-lavpris.dk/files/sup2/LM135.pdf
- https://www.engineersgarage.com/arduino-ssd1306-oled-display/
- https://circuitdigest.com/microcontroller-projects/arduino-ssd1306-oled-display
- https://www.sarathiblog.com/SPI-oled-arduino.html

## Further Reading
**MMQT**  
- https://docs.arduino.cc/tutorials/uno-wifi-rev2/uno-wifi-r2-mqtt-device-to-device  
- https://juanstechblog.blogspot.com/2022/02/diy-mqtt-temperature-humidity-sensor-arduino-esp8266-dht22.html
- http://www.whatimade.today/make-a-remote-temp-sensor-with-permanent-display-inside-your-house/

**Webhook**  
- https://create.arduino.cc/projecthub/usini/send-a-message-on-discord-f216e0
- https://stackoverflow.com/questions/55588751/how-to-post-to-a-ms-teams-webhook-from-esp8266
- https://forum.arduino.cc/t/wifi-end-disables-connection-but-not-the-wifi-module/604754/3
- https://docs.arduino.cc/tutorials/communication/wifi-nina-examples

**Send data to PI**  
- https://pisquare.osisoft.com/s/Blog-Detail/a8r1I000000H73GQAS/arduino-to-pi-with-pi-web-api
- https://pisquare.osisoft.com/s/Blog-Detail/a8r1I000000H73uQAC/pi-web-api-and-nodered
- https://pisquare.osisoft.com/s/Blog-Detail/a8r1I000000GvfiQAC/raspberry-pi-to-pi-with-pi-web-api-part-22
- https://pisquare.osisoft.com/s/Blog-Detail/a8r1I000000GvfYQAS/arduinoraspberry-pi-data-logger-part-12
- https://pisquare.osisoft.com/s/Blog-Detail/a8r1I000000GvrxQAC/internet-of-things-esp8266-writing-sensor-data-into-pi-via-the-webapi


**OLED Display**  
- https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/ 
- https://electropeak.com/learn/interfacing-0-96-inch-spi-oled-display-module-with-arduino/
- https://www.instructables.com/OLED-Tutorial-Convert-SPI-to-I2C/
- https://lastminuteengineers.com/oled-display-arduino-tutorial/

**Temperature Sensors**  
- https://create.arduino.cc/projecthub/sarful/serial-temperature-sensor-arduino-workshop-aab43a?ref=part&ref_id=8233&offset=149  
- https://forum.arduino.cc/t/lm135-precision-temperature-sensor/20085/4
- https://www.arnabkumardas.com/arduino-tutorial/lm335-temperature-sensor/
