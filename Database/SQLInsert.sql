# Insert into Messpunkte

# BME280:
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(1, 'BME280', 'Temperature', '*C');
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(2, 'BME280', 'Pressure', 'hPa');
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(3, 'BME280', 'Humidity', '%');

# GY-302 BH1750:
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(4, 'GY-302', 'Light', 'lx');

# Raindrop Sensor Module:
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(5, 'RaindropSensor', 'Rain', '(true/false)');

# NEO-6M GPS Module:
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(6, 'NEO-6M', 'Latitude', '°');
INSERT INTO usr_web204_3.Messpunkte (mp_id, mp_name, mp_maß, mp_einheit) VALUES(7, 'NEO-6M', 'Longitude', '°');

