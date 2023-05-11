-- CREATE DATABASE

USE usr_web204_3;

-- CREATE TABLE

--
-- Datenbank: `usr_web204_3`
--

-- --------------------------------------------------------

--
-- Disabeling FOREIGN_KEY_CHECKS
--

SET FOREIGN_KEY_CHECKS = 0;

--
-- Tabelle Messorte
--
DROP TABLE IF EXISTS Messorte;
CREATE TABLE Messorte (
    mo_espID INT PRIMARY KEY,
    mo_name VARCHAR(255)
);

--
-- Tabelle Messpunkte
--
DROP TABLE IF EXISTS Messpunkte;
CREATE TABLE Messpunkte (
    mp_id INT PRIMARY KEY,
    mp_name VARCHAR(255),
    mp_maß VARCHAR(255),
    mp_einheit VARCHAR(255)
);

/*
--
-- Tabelle Messgroessen
--
DROP TABLE IF EXISTS Messgroessen;
CREATE TABLE Messgroessen (
    mg_id INT PRIMARY KEY,
    mg_name VARCHAR(255) UNIQUE,
    mg_einheit VARCHAR(255)
);
*/

--
-- Tabelle Messwerte
--
DROP TABLE IF EXISTS Messwerte;
CREATE TABLE Messwerte (
    mw_id INT PRIMARY KEY,
    mw_messpunkt INT,
    mw_messort INT,
    mw_wert FLOAT(5,2),
    mw_datumZeit datetime
);

--
-- Foreign Keys & Auto Increment
--

/*
ALTER TABLE Messorte
    MODIFY mo_id INT NOT NULL AUTO_INCREMENT;

ALTER TABLE Messpunkte
    ADD CONSTRAINT FK_MP2MG FOREIGN KEY (mp_messgroesse) REFERENCES Messgroessen(mg_id),
    MODIFY mp_id INT NOT NULL AUTO_INCREMENT;

ALTER TABLE Messgroessen
    MODIFY mg_id INT NOT NULL AUTO_INCREMENT;
*/

ALTER TABLE Messwerte
    ADD CONSTRAINT FK_MW2MP FOREIGN KEY (mw_messpunkt) REFERENCES Messpunkte(mp_id),
    ADD CONSTRAINT FK_MW2MO FOREIGN KEY (mw_messort) REFERENCES Messorte(mo_espID),
    MODIFY mw_id INT NOT NULL AUTO_INCREMENT;

