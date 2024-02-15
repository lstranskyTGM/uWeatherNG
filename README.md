# uWeather_Project

An ESP32 Weatherstation Project with a MySQL Database and a Website.

## Task: 

This project was a task during the second half of the third year at TGM (Technologisches Gewerbemuseum Wien). We were required to form a team of four people and come up with an idea for a project, which we would then develop over the next six months. Throughout this period, we had to document our progress and give monthly presentations to our teachers.

## Function:

ESP32:
- Reads Data from Sensors
- Sends Data to Database
- Goes into DeepSleep for certain time
- Wakes up and repeats 

MySQL Database:
- Stores Data from ESP32

Website:
- Displays Data from Database
- Displays Location of ESP32 on Google Maps

## Development

We divided the team into two groups: FrontEnd and BackEnd. The FrontEnd Team was responsible for developing the website, while the BackEnd Team took charge of the ESP32 and the Database. 

This GitHub repository was created to faciliate version control during the development of the project.

The FrontEnd Team started off by creating a mockup design for the Website and the BackEnd Team wrote test scripts for the ESP32 and ordered all necessary parts.

Setting up the Database for the MySQL server and writing the SQLCreate file went fairly quick.

Creating the Website and all the Scripts for the ESP32 took the most time of all. Because the Website needed to be like the mockup design and the ESP32 testscripts needed to be adjusted and also tested with the real sensors set up and connected. 

We got send an broken gps module and wondered why the code for reading the module didn't work. After we got a new one we found out that the code was working all along and the gps module was just broken.

The ESP32 testscripts were then combined into one main script.

The Database got tweaked a few times during development to make it more efficient. And SQLInsert for all the sensor information and their measurment data was added.

Google Maps was also implemented into the Website so you can see the location of the ESP32.

An 3D model of the ESP32 case was created and printed. (Which took around 12h of printing time)

Every Sensor was then put into the case and connected to the ESP32. (The ESP32 us powered by a powerbank)

The nearly finished project besides some functions that still needed to be implemented was then presented at the hand in deadline to our teachers.

## Documentations:

- [ESP32 Markdown](https://github.com/lstranskyTGM/uWeather/tree/main/ESP32)
- [HardwareBox Markdown](https://github.com/lstranskyTGM/uWeather/tree/main/HardwareBox)
- [Database Markdown](https://github.com/lstranskyTGM/uWeather/tree/main/Database)
- [Website Markdown](https://github.com/lstranskyTGM/uWeather/tree/main/Website)

## Unimplemented Functions:

- NodeJS/VueJS Script that hosts the Website

## Possible Extensions:

- Add a gsm/lte Module instead of WiFi (with a SIM Card)
- Add a lora Module instead of WiFi that has a longer range (2-15 km)
- Add an Anemometer to measure wind speed
- Add a Wind Vane to measure wind direction
- Add a Rain Sensor to measure amount of rain
- Add a Air Quality Sensor to measure air quality
- Turn off digital pins that are not used to save power
- Develop a pcb for the ESP32 and all the sensors
- Replace the ESP32 Dev Module with a simpler ESP32 Board
- Implement Grafana to display the Data from the Database
- Implement ESPhome / homeassistant
- Use Networkprotocl MQTT
- Replace the MySQL Database with a influxDB Database
- ...










