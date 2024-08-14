# uWeatherNG_Project

An ESP32-based weather monitoring system that collects environmental data from various sensors and publishes it to a MQTT broker topic. An agent subscribed to this topic receives the data and forwards it to an InfluxDB database. The data is then displayed on a website and visualized using Grafana dashboards.

## Background

This project was undertaken during the fourth year at TGM (Technologisches Gewerbemuseum Wien). We built upon the [project](https://github.com/lstranskyTGM/uWeather) developed in the third grade. Throughout the year, we documented our progress and presented the project's status to our teachers.

## Overview

### ESP32
- Main controller for the weather monitoring system
- Reads sensor data
- Sends data to database
- Enters DeepSleep for configured time
- Wakes up and repeats

### MQTT Broker
- Receives data from ESP32

### Telegraf
- Collects data from MQTT Broker
- Sends data to InfluxDB

### InfluxDB
- Stores data from ESP32

### Website
- Displays data from database
- Shows ESP32 location on Google Maps

### Grafana
- Visualizes data from database

### Cloudflared (Optional)

- Securely exposes local server to the internet

## Documentations

- [DockerCompose](https://github.com/lstranskyTGM/uWeatherNG/tree/main/DockerCompose)
- [ESP32](https://github.com/lstranskyTGM/uWeatherNG/tree/main/ESP32)
- [Hardware](https://github.com/lstranskyTGM/uWeatherNG/tree/main/Hardware)
- [Website](https://github.com/lstranskyTGM/uWeatherNG/tree/main/Website)

## Development

(In Progress)

## Implemented Functions

- [x] Swithed to LTE instead of WiFi
- [x] Added an Anemometer to measure wind speed
- [x] Added a Wind Vane to measure wind direction
- [x] Added an Air Quality Sensor to measure air quality
- [x] Implemented Grafana to display data from the database
- [x] Used the MQTT protocol for data transmission
- [x] Replaced the MySQL database with a InfluxDB database

## Unimplemented Functions

- [ ] Micro SD Card Reader Module (hardware implemented, code not written)

## Implemented Functions

## Possible Extensions

- [ ] Switch to LoRaWAN for extended range and cost-effective data transfer
- [ ] Add a Rain Gauge to measure rainfall
- [ ] Disable unused digital pins to conserve power when not needed
- [ ] Develop a custom PCB for the ESP32 and all sensors
- [ ] Replace the ESP32 Dev Module with a more compact and efficient ESP32 board
- [ ] Implement ESPHome / Home Assistant