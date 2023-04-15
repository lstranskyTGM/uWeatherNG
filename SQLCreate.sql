-- CREATE DATABASE

DROP DATABASE IF EXISTS SensorDaten;
CREATE DATABASE SensorDaten;
USE SensorDaten;

-- CREATE TABLE

--
-- Datenbank: `SensorDaten`
--

-- --------------------------------------------------------

--
-- Tabelle Messorte
--

CREATE TABLE Messorte (
    mo_id INT PRIMARY KEY,
    mo_name VARCHAR(255),
);

--
-- Tabelle Messpunkte
--

CREATE TABLE Messpunkte (
    mp_id INT PRIMARY KEY,
    mp_messgröße INT,
    mp_name VARCHAR(255),
);

--
-- Tabelle Messgröße
--

CREATE TABLE Messgröße (
    mg_id INT PRIMARY KEY,
    mg_name VARCHAR(255),
    mg_einheit VARCHAR(255),
);

--
-- Tabelle Messwerte
--

CREATE TABLE Messwerte (
    mw_id INT PRIMARY KEY,
    mw_messpunkt INT,
    mw_messort INT,
    mw_wert FLOAT(5,2),
    mw_datum DATE,
);

--
-- Foreign Keys
--

ALTER TABLE Messpunkte
    ADD FOREIGN KEY (mp_messgröße) REFERENCES Messgröße(mp_messgröße);

ALTER TABLE Messwerte
    ADD FOREIGN KEY (mw_messpunkt) REFERENCES Messpunkte(mw_messpunkt);
    ADD Foreign KEY (mw_messort) REFERENCES Messorte(mw_messort);

--
-- Adding Triggers
--

-- ON Insert trigger
CREATE TRIGGER check_Messorte_insert
BEFORE INSERT ON Messorte
FOR EACH ROW
BEGIN

END;

CREATE TRIGGER check_Messpunkte_insert
BEFORE INSERT ON Messpunkte
FOR EACH ROW
BEGIN

END;

CREATE TRIGGER check_Messgröße_insert
BEFORE INSERT ON Messgröße
FOR EACH ROW
BEGIN

END;

-- ON update trigger
CREATE TRIGGER check_Messorte_update
BEFORE UPDATE ON Messorte
FOR EACH ROW
BEGIN

END;

CREATE TRIGGER check_Messpunkte_update
BEFORE UPDATE ON Messpunkte
FOR EACH ROW
BEGIN

END;

CREATE TRIGGER check_Messgröße_update
BEFORE UPDATE ON Messgröße
FOR EACH ROW
BEGIN

END;

-- ON delete trigger
CREATE TRIGGER check_Messorte_delete
BEFORE DELETE ON Messorte
FOR EACH ROW
BEGIN

END;

CREATE TRIGGER check_Messpunkte_delete
BEFORE DELETE ON Messpunkte
FOR EACH ROW
BEGIN

END;

CREATE TRIGGER check_Messgröße_delete
BEFORE DELETE ON Messpunkte
FOR EACH ROW
BEGIN

END;