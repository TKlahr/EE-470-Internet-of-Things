-- EE-470 Database Assignment
-- Tyler Klahr
-- Database Setup

CREATE TABLE Book (
    id INT NOT NULL AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE Price (
    id INT NOT NULL AUTO_INCREMENT,
    price FLOAT NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE sensor_register (
    node_name VARCHAR(10) PRIMARY KEY,
    manufacturer VARCHAR(10),
    longitude DECIMAL(15,10),
    latitude DECIMAL(15,10)
);

CREATE TABLE sensor_data (
    node_name VARCHAR(10),
    time_received DATETIME,
    temperature DECIMAL(6,2),
    humidity DECIMAL(6,2),
    PRIMARY KEY (node_name, time_received),
    FOREIGN KEY (node_name)
        REFERENCES sensor_register(node_name),
    CHECK (temperature BETWEEN -10 AND 100),
    CHECK (humidity BETWEEN 0 AND 100)
);

CREATE VIEW sensor_combined AS
SELECT
    sensor_data.node_name,
    sensor_data.time_received,
    sensor_data.temperature,
    sensor_data.humidity,
    sensor_register.manufacturer,
    sensor_register.longitude,
    sensor_register.latitude
FROM sensor_data
JOIN sensor_register
ON sensor_data.node_name = sensor_register.node_name;
