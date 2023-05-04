# Insert into Messpunkte

# BME280:
INSERT INTO usr_web204_3.Messpunkte (mp_name, mp_maß, mp_einheit) VALUES('BME280', 'Temperature', '*C');
INSERT INTO usr_web204_3.Messpunkte (mp_name, mp_maß, mp_einheit) VALUES('BME280', 'Pressure', 'hPa');
INSERT INTO usr_web204_3.Messpunkte (mp_name, mp_maß, mp_einheit) VALUES('BME280', 'Humidity', '%');

# GY-302 BH1750:
INSERT INTO usr_web204_3.Messpunkte (mp_name, mp_maß, mp_einheit) VALUES('GY-302', 'Light', 'lx');

# Raindrop Sensor Module:
INSERT INTO usr_web204_3.Messpunkte (mp_name, mp_maß, mp_einheit) VALUES('Raindrop', 'Rain', '(true/false)');

