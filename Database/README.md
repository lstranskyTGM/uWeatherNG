# Database:

<p align="center">
    <img src="https://github.com/lstranskyTGM/uWeather/blob/main/Database/img/database.png?raw=true" alt="database" width="500" align="center">
</p>

## Messorte:

- mo_espID (PK)
- mo_name

## Messpunkte:

- mp_id (PK)
- mp_name
- mp_maß
- mp_einheit

## Messwerte:

- mw_id (PK)
- mw_messpunkt (FK)
- mw_messort (FK)
- mw_wert
- mw_datumZeit

The Database consists of these 3 tables. The table `Messorte` and `Messpunkte` are set once by an SQLInsert File. The table `Messwerte` is filled by the ESP32 script.

The Database `Messwerte` has two foreign keys. All the ids in the Database are set by an auto increment except the espID, which is a constant variable in the ESP32 script.

The `mw_datumZeit` is set by the ESP32 script from the RTC (Real Time Clock) of the ESP32.

## Messpunkte Content:

|mp_id| mp_name| mp_maß| mp_einheit|
|:----|:----|:----|:----|
|1|BH1750|Light|lx|
|2|BME280|Temperature|*C|
|3|BME280|Pressure|hPa|
|4|BME280|Humidity|%|
|5|FC-37|Rain|t/f|
|6|NEO-6M|Latitude|°|
|7|NEO-6M|Longitude|°|

```sql
# Insert into Messpunkte


# GY-302 BH1750:
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(1, 'BH1750', 'Light', 'lx');

# BME280:
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(2, 'BME280', 'Temperature', '*C');
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(3, 'BME280', 'Pressure', 'hPa');
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(4, 'BME280', 'Humidity', '%');

# Raindrop Sensor Module:
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(5, 'FC-37', 'Rain', 't/f');

# NEO-6M GPS Module:
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(6, 'NEO-6M', 'Latitude', '°');
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(7, 'NEO-6M', 'Longitude', '°');

# Add Messort:
INSERT INTO usr_web204_3.Messorte (mo_espID, mo_name) VALUES(101, 'Prototype');
```

# ToDo List:

- [x] Create Database (SQLCreate)
- [x] Create SQLInsert for fixed values