-- EE-470 Database Assignment
-- Tyler Klahr
-- SQL Queries and Testing

-- Display all books
SELECT * FROM Book;

-- Display all prices
SELECT * FROM Price;

-- Join Book and Price tables
SELECT Book.name, Price.price
FROM Book
JOIN Price ON Book.id = Price.id;

-- Display registered sensors
SELECT * FROM sensor_register;

-- Display sensor measurements
SELECT * FROM sensor_data;

-- Display combined sensor information
SELECT * FROM sensor_combined;

-- Test foreign key constraint
-- Expected result: Error #1452
-- node_6 does not exist in sensor_register

-- INSERT INTO sensor_data
-- (node_name, time_received, temperature, humidity)
-- VALUES
-- ('node_6', '2022-10-01 13:00:00', 25.0, 50);
