# ESP32

## Test Scripts

In the test scripts all functions for the ESP32 are tested seperately. This makes it easier to find errors and debug everything.   

## Main Script

<p align="center">
    <img src="https://github.com/lstranskyTGM/uWeatherNG/blob/main/ESP32/img/codeLogic.png?raw=true" alt="codeLogic" width="400" align="center">
</p>

The ESP32 script initiates by setting up various sensors and modules, including GPS, and collecting environmental data. It then attempts to transmit this information via MQTT over an LTE connection. If the connection is established, the collected data is published to a designated topic on an MQTT broker. After transmission, the ESP32 enters a deep sleep mode to conserve energy, periodically waking to update and resend data. Throughout this process, all status updates and messages are displayed both on the Serial Monitor and an OLED display. This configuration ensures continuous, efficient monitoring of environmental conditions with minimal human intervention.

## Sensor / Module List

### GY-302 BH1750 Light Sensor Module

The GY-302 BH1750 Photoresistor module is used to measure light intensity. The resistance will decrease in the presence of light and increase in the absence of it. 

**Light Intensity:** lux

[Buy here](https://www.az-delivery.com/products/gy-302-bh1750-lichtsensor-lichtstaerke-modul-fuer-arduino-und-raspberry-pi)

### GY-BME280 Environmental Sensor [I2C]

This sensor measures relative humidity, barometric pressure, and ambient temperature, communicating via the I2C bus.

**Relative Humidity:** %RH

**Barometric Pressure:** hPa

**Ambient Temperature:** °C

[Buy here](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/)

### FC-37 Rain Sensor Module

The FC-37 module detects rain, with lower resistance signaling rain presence and triggering a high output signal.

**Rain:** True/False

[Buy here](https://www.az-delivery.com/en/products/regen-sensor-modul)

### GY-SGP30 Air Quality Sensor [I2C]

THe GY-SGP30 is a digital air quality sensor that measures the amount of volatile organic compounds (VOCs) in the air. The sensor also measures the equivalent carbon-dioxide (eCO2) concentration.
It uses the I2C bus to communicate with the ESP32.

**Total Volatile Organic Compounds (TVOC):** ppb

**Equivalent Carbon-dioxide (eCO2):** ppm

[Buy here](https://funduinoshop.com/elektronische-module/sensoren/gase/gy-sgp30-tvoc-sensor-fuer-luftqualitaet-eco2)

### KY-003 Digital Hall Effect Sensor (Anemometer)

This sensor detects the magnetic field of a spinning magnet attached to an anemometer to calculate wind speed.

**Wind Speed:** km/h

[Buy here](https://www.az-delivery.com/en/products/hall-sensor-modul-digital)

### Rotary Hall Angle Sensor (Wind Vane)

The rotary hall angle sensor is used to measure the angle of the wind vane.

**Wind Direction:** Degrees (°)

[Buy here](https://www.aliexpress.com/item/4000143910873.html?gatewayAdapt=glo2deu)

### 1.3 inch OLED Display [I2C]

This OLED display shows the status and data collected by the ESP32, utilizing the I2C bus for communication.

[Buy here](https://www.az-delivery.de/en/products/1-3zoll-i2c-oled-display)

### Micro SD Card Reader Module

The micro SD card reader module is used to configure the Main Script.

[Buy here](https://www.az-delivery.de/en/products/copy-of-spi-reader-micro-speicherkartenmodul-fur-arduino)

### SIM7000E NB-IoT Module

The SIM7000E NB-IoT Module is used to connect the ESP32 to the internet, sending the data to a MQTT broker.
It also has a GPS module that can be used to get the current location of the ESP32.

**Latitude:** Degrees (°)

**Longitude:** Degrees (°)

**Altitude:** Meters (m)

[Buy here](https://www.waveshare.com/sim7000e-nb-iot-hat.htm)

## ToDo List

- [x] ~~Write all test scripts~~
- [x] ~~Write the main script~~









