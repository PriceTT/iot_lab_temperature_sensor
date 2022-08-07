# Iot Lab Temperature Sensor

IOT sensor to monitor the cooling room temperature.
* High tempureature alert is sent via webhook to Discord/MS TEAMS
* Log the values to the PI historian

**Pinout Arduino MKR WIFI 1010**  
<p align="center">
  <img src="imgs/img.JPEG" >
</p>



## Parts List  

- [Arduino MKR WIFI 1010](https://www.computersalg.dk/i/4892362/arduino-mkr-wifi-1010-arm-cortex-m0-48-mhz-0-256-mb-32-kb-arduino-25-x-61-5-mm)  
- [LM335A - Precision Temperature Sensor](https://elektronik-lavpris.dk/p100429/lm335az-temp-sensor-40-100c-to92-05)  
- [OLED SPI Display SSD1306](https://elektronik-lavpris.dk/p143602/modu0052-ssd1306-128x64-pixel-uoled-display-module-blue/)

## Getting started with the hardware
- https://randomnerdtutorials.com/arduino-lm35-lm335-lm34-temperature-sensor/
- https://elektronik-lavpris.dk/files/sup2/LM135.pdf



**Convert SPI OLED to I2C**  
SPI should be faster than I2C but I could not get it to work. Hence, the display was [converted to I2C](https://www.instructables.com/OLED-Tutorial-Convert-SPI-to-I2C/).     

Note in the future purchace the [I2C version of the OLED](https://elektronik-lavpris.dk/p148473/sbc-oled01-oled-display-for-arduino-raspberry-pi-or-other-single-board/)  

## Getting started with the mqtt client
1. Clone this GIT repository:
```
git@github.com:PriceTT/iot_lab_temperature_sensor.git
```
___

2. Set up an virtual environment.  

Navigate to **mqtt_client** folder and create conda evironment which reads the `environment.yml` file.
```
conda env create
```

Activate the virtual environment.
```
conda activate mqtt-env

```
Insall python packages
```
poetry install  
```
___
3. Subscribe to the broker  
```
python mqtt_client.py 

```



## TODO  
- Test connecting to teams (blocked by getting the correct domain)
- ~Average temp over a period~
- ~Send data via MMQT~
- Test out PI integration


## Further Reading and Info
**MMQT**  
- https://docs.arduino.cc/tutorials/mkr-wifi-1010/mqtt-device-to-device
- https://juanstechblog.blogspot.com/2022/02/diy-mqtt-temperature-humidity-sensor-arduino-esp8266-dht22.html
- http://www.whatimade.today/make-a-remote-temp-sensor-with-permanent-display-inside-your-house/

**Rest API server**  
- https://www.survivingwithandroid.com/esp32-rest-api-esp32-api-server/
- https://www.arduino.cc/en/Guide/MKRWiFi1010/hosting-a-webserver

**Webhook**  
- https://create.arduino.cc/projecthub/usini/send-a-message-on-discord-f216e0
- https://stackoverflow.com/questions/55588751/how-to-post-to-a-ms-teams-webhook-from-esp8266
- https://forum.arduino.cc/t/wifi-end-disables-connection-but-not-the-wifi-module/604754/3
- https://docs.arduino.cc/tutorials/communication/wifi-nina-examples
- https://store-usa.arduino.cc/products/arduino-mkr-wifi-1010

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
- https://www.sarathiblog.com/SPI-oled-arduino.html
- https://www.engineersgarage.com/arduino-ssd1306-oled-display/
- https://circuitdigest.com/microcontroller-projects/arduino-ssd1306-oled-display

**Temperature Sensors**  
- https://create.arduino.cc/projecthub/sarful/serial-temperature-sensor-arduino-workshop-aab43a?ref=part&ref_id=8233&offset=149  
- https://forum.arduino.cc/t/lm135-precision-temperature-sensor/20085/4
- https://www.arnabkumardas.com/arduino-tutorial/lm335-temperature-sensor/
- https://diyodemag.com/education/the_classroom_the_lm335_temperature_sensor#tab-altronics-1167440571
- https://www.arduino.cc/en/Guide/MKRWiFi1010/connecting-sensors  
- https://forum.arduino.cc/t/averaging-of-data/545871/4

**Pinout Arduino MKR WIFI 1010**   
<p align="center">
  <img src="imgs/Pinout-MKRwifi1010_latest.png" >
</p>
