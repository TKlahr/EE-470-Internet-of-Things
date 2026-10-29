<?php
// EE-470 Assignment 4 - Part 2
// Insert sensor readings using HTTP GET

require 'connect.php';

header('Content-Type: text/plain; charset=utf-8');

// Check required GET parameters
if (!isset($_GET['nodeId'], $_GET['nodeTemp'],
           $_GET['nodeHumidity'])) {
    http_response_code(400);
    exit("Error: Missing required parameters.");
}

// Read values from URL
$node = $_GET['nodeId'];
$temp = $_GET['nodeTemp'];
$humidity = $_GET['nodeHumidity'];

// Validate input
if (!is_string($node) || $node === '' ||
    !is_scalar($temp) || !is_numeric($temp) ||
    !is_scalar($humidity) || !is_numeric($humidity)) {
    http_response_code(400);
    exit("Error: Invalid input.");
}

try {
    if (isset($_GET['timeReceived']) &&
        $_GET['timeReceived'] !== '') {

        // Use the timestamp provided in the URL
        $time = $_GET['timeReceived'];

        if (!is_string($time)) {
            throw new InvalidArgumentException("Invalid timestamp.");
        }

        $date = DateTimeImmutable::createFromFormat(
            '!Y-m-d H:i:s', $time
        );

        if (!$date || $date->format('Y-m-d H:i:s') !== $time) {
            throw new InvalidArgumentException("Invalid timestamp.");
        }

        $sql = "INSERT INTO sensor_data
                (node_name, time_received, temperature, humidity)
                VALUES (?, ?, ?, ?)";

        $stmt = $conn->prepare($sql);
        $stmt->bind_param("ssdd", $node, $time, $temp, $humidity);

    } else {

        // Database automatically generates the timestamp
        $sql = "INSERT INTO sensor_data
                (node_name, time_received, temperature, humidity)
                VALUES (?, CURRENT_TIMESTAMP, ?, ?)";

        $stmt = $conn->prepare($sql);
        $stmt->bind_param("sdd", $node, $temp, $humidity);
    }

    $stmt->execute();

    echo "New sensor reading inserted successfully!";

} catch (InvalidArgumentException $e) {
    http_response_code(400);
    echo "Error: Invalid timestamp. Use YYYY-MM-DD HH:MM:SS.";

} catch (mysqli_sql_exception $e) {
    http_response_code(400);
    error_log("Sensor insert failed: " . $e->getMessage());
    echo "Error: Reading was not inserted. Check the node, values, or timestamp.";
}

$conn->close();
?>
