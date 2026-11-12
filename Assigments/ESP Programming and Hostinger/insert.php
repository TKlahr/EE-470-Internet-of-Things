<?php
// EE-470 ESP8266 IoT Project
// Receives sensor readings and updates activity counts
// Version 1.0
// Inputs: nodeId, nodeTemp, nodeHumidity,
//         lightLevel, timeReceived
// Output: STORED, DUPLICATE, or ERROR

header('Content-Type: text/plain; charset=utf-8');

require 'connect.php';

// Check required parameters
$required = [
    'nodeId', 'nodeTemp', 'nodeHumidity',
    'lightLevel', 'timeReceived'
];

foreach ($required as $key) {
    if (!isset($_GET[$key]) || !is_string($_GET[$key])) {
        http_response_code(400);
        exit("ERROR: Missing or invalid parameter: $key");
    }
}

// Read measurements
$node = $_GET['nodeId'];
$temp = $_GET['nodeTemp'];
$humidity = $_GET['nodeHumidity'];
$light = $_GET['lightLevel'];
$time = $_GET['timeReceived'];

// Validate node and sensor values
if (
    !in_array($node, ['node_1', 'node_2'], true) ||
    !is_numeric($temp) ||
    !is_numeric($humidity) ||
    !ctype_digit($light) ||
    $temp < -40 || $temp > 80 ||
    $humidity < 0 || $humidity > 100 ||
    (int)$light > 1023
) {
    http_response_code(400);
    exit("ERROR: Invalid sensor data.");
}

// Validate timestamp
$date = DateTimeImmutable::createFromFormat(
    '!Y-m-d H:i:s', $time
);

if (!$date || $date->format('Y-m-d H:i:s') !== $time) {
    http_response_code(400);
    exit("ERROR: Invalid timestamp.");
}

try {
    // Begin transaction
    $conn->begin_transaction();

    // Insert measurement
    $stmt = $conn->prepare(
        "INSERT INTO sensor_data
        (node_name, time_received, temperature,
         humidity, light_level)
         VALUES (?, ?, ?, ?, ?)"
    );

    $lightValue = (int)$light;

    $stmt->bind_param(
        "ssddi",
        $node, $time, $temp, $humidity, $lightValue
    );

    $stmt->execute();
    $stmt->close();

    // Increment activity counter
    $stmt = $conn->prepare(
        "UPDATE sensor_activity
         SET message_count = message_count + 1
         WHERE node_name = ?"
    );

    $stmt->bind_param("s", $node);
    $stmt->execute();

    if ($stmt->affected_rows !== 1) {
        throw new RuntimeException(
            "Activity counter not initialized."
        );
    }

    $stmt->close();

    // Save both database changes
    $conn->commit();

    http_response_code(200);
    echo "STORED";

} catch (mysqli_sql_exception $e) {

    $conn->rollback();

    // Duplicate primary key: node + timestamp
    if ($e->getCode() == 1062) {
        http_response_code(200);
        echo "DUPLICATE";
    } else {
        error_log($e->getMessage());
        http_response_code(500);
        echo "ERROR: Database operation failed.";
    }

} catch (Throwable $e) {

    $conn->rollback();
    error_log($e->getMessage());

    http_response_code(500);
    echo "ERROR: Database operation failed.";
}

$conn->close();
?>
