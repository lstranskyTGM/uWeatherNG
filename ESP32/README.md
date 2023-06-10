# ESP32:

## Test Scripts:

In the test scripts all functions for the ESP32 are tested seperately. This makes it easier to find errors and to test and debug everything.   

## Main Script:

<p align="center">
    <img src="https://github.com/lstranskyTGM/uWeather/blob/main/ESP32/img/codeLogic.png?raw=true" alt="codeLogic" width="300" align="center">
</p>

The ESP32 starts off setting all pins, variables, ... Then the availability off all devices is checked and the Bootcount updates (that gets saved during DeepSleep). The wifi connection gets established. After that the ESP32 requests an NTP (Network-Time-Protocol) Server and writes the time into the RTC (Real-Time-Clock). Next the MySQL Server is connected to and the sensor data is send to the server. The ESP32 closes all Connections and goes into DeepSleep for a before set Intervall (3-4 minutes). The ESP32 wakes up and starts again.

All the status information is also printed on the Serial Monitor and the OLED Display.

## Other:

- MySQLconnection: https://github.com/ChuckBell/MySQL_Connector_Arduino
- RequestNTP: https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/
- RTC: https://www.theelectronics.co.in/2022/04/how-to-use-internal-rtc-of-esp32.html
- DeepSleep: https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
- RainSensor: https://diyi0t.com/rain-sensor-tutorial-for-arduino-and-esp8266/
- GPS Module: https://how2electronics.com/esp32-gps-tracker-using-l86-gps-module-oled-display/
- OLED Display: https://lastminuteengineers.com/oled-display-esp32-tutorial/

# Sensor List:

## GY-302 BH1750:

The GY-302 BH1750 Photoresistor module is used to measure light intensity. The resistance will decrease in the presence of light and increase in the absence of it. 

Light intensity: lx

[Buy here](https://www.az-delivery.de/products/gy-302-bh1750-lichtsensor-lichtstaerke-modul-fuer-arduino-und-raspberry-pi)

## BME280:

Humidity sensor measuring relative humidity, barometric pressure and ambient temperature

Relative humidity: %

Barometric pressure: hPa

Ambient temperature: m

[Buy here](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/)

## FC-37 Rain Sensor Module:

Raindrop Rain Sensor Module is a module used to detect and measure rain. It usually consists of a sensor that can detect the presence of water droplets, and an electronic circuit that can process the signal from the sensor and output an indication of whether it is raining or not. 

Rain: (true/false)

[Buy here](https://www.az-delivery.de/en/products/regen-sensor-modul)

## ublox NEO-6M GPS Module:

The NEO-6 module series is a family of stand-alone GPS receivers featuring the high performance u-blox 6 positioning engine.

Longitute: °
Latitude: °

[Buy here](https://www.az-delivery.de/en/products/regen-sensor-modul)

# ToDo List:

- [x] ~~Write all test scripts~~
- [x] ~~Write the main script using testscripts~~
- [x] ~~Find a power source for the esp32 (akku, solar panel, ...)~~
- [x] ~~3D print a case for the esp32~~
- [x] ~~Make the sensors protected against rain and direct daylight~~









