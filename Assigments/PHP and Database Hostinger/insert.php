<?php
// EE-470 Assignment 4 - Part 2
// Insert sensor readings using HTTP GET

require 'connect.php';

header('Content-Type: text/plain; charset=utf-8');

// Check required GET parameters
if (!isset($_GET['nodeId'], $_GET['nodeTemp'],
           $_GET['nodeHumidity'], $_GET['timeReceived'])) {
    http_response_code(400);
    exit("Error: Missing required parameters.");
}

// Read values from the URL
$node = $_GET['nodeId'];
$temp = $_GET['nodeTemp'];
$humidity = $_GET['nodeHumidity'];
$time = $_GET['timeReceived'];

// Validate the input
if (!is_string($node) || $node === '' ||
    !is_scalar($temp) || !is_numeric($temp) ||
    !is_scalar($humidity) || !is_numeric($humidity) ||
    !is_string($time)) {
    http_response_code(400);
    exit("Error: Invalid input.");
}

// Check timestamp format
$date = DateTimeImmutable::createFromFormat(
    '!Y-m-d H:i:s', $time
);

if (!$date || $date->format('Y-m-d H:i:s') !== $time) {
    http_response_code(400);
    exit("Error: Invalid timestamp. Use YYYY-MM-DD HH:MM:SS.");
}

// Insert data using a prepared statement
$sql = "INSERT INTO sensor_data
        (node_name, time_received, temperature, humidity)
        VALUES (?, ?, ?, ?)";

$stmt = $conn->prepare($sql);
$stmt->bind_param("ssdd", $node, $time, $temp, $humidity);

try {
    $stmt->execute();
    echo "New sensor reading inserted successfully!";
} catch (mysqli_sql_exception $e) {
    http_response_code(400);
    error_log("Sensor insert failed: " . $e->getMessage());
    echo "Error: Reading was not inserted. Check the node, values, or timestamp.";
}

$stmt->close();
$conn->close();
?>
