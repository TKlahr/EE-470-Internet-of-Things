<?php
// EE-470 Assignment 4 - Part 2
// Section 4 - Base64 Encoded Sensor Data

require 'connect.php';

header('Content-Type: text/plain; charset=utf-8');

// Accept Base64 data with or without "data="
if (isset($_GET['data']) && is_string($_GET['data'])) {

    // Format 1: ?data=ENCODED_MESSAGE
    $encoded = $_GET['data'];

} else {

    // Format 2: ?ENCODED_MESSAGE
    $encoded = $_SERVER['QUERY_STRING'] ?? '';
    $encoded = explode('&', $encoded, 2)[0];
}

if ($encoded === '') {
    http_response_code(400);
    exit("Error: Missing encoded data.");
}

// Decode Base64 message
$decoded = base64_decode($encoded, true);

if ($decoded === false) {
    http_response_code(400);
    exit("Error: Invalid Base64 message.");
}

// Parse decoded parameters
parse_str($decoded, $params);

// Check required parameters
if (!isset($params['nodeId'],
           $params['nodeTemp'],
           $params['timeReceived'])) {

    http_response_code(400);
    exit("Error: Missing required parameters.");
}

// Retrieve decoded values
$node = $params['nodeId'];
$temp = $params['nodeTemp'];
$time = $params['timeReceived'];
$humidity = $params['nodeHumidity'] ?? null;

// Validate sensor data
if (!is_string($node) || $node === '' ||
    !is_scalar($temp) || !is_numeric($temp) ||
    !is_string($time) ||
    ($humidity !== null &&
     (!is_scalar($humidity) || !is_numeric($humidity)))) {

    http_response_code(400);
    exit("Error: Invalid sensor data.");
}

// Validate timestamp
$date = DateTimeImmutable::createFromFormat(
    '!Y-m-d H:i:s', $time
);

if (!$date || $date->format('Y-m-d H:i:s') !== $time) {
    http_response_code(400);
    exit("Error: Invalid timestamp.");
}

// Display decoded sensor data
echo "Decoded Sensor Data:\n";
echo "nodeId = " . $node . "\n";
echo "nodeTemp = " . $temp . "\n";

if ($humidity !== null) {
    echo "nodeHumidity = " . $humidity . "\n";
} else {
    echo "nodeHumidity = Not provided\n";
}

echo "timeReceived = " . $time . "\n\n";

// Insert sensor reading into database
try {

    $sql = "INSERT INTO sensor_data
            (node_name, time_received, temperature, humidity)
            VALUES (?, ?, ?, ?)";

    $stmt = $conn->prepare($sql);

    $stmt->bind_param(
        "ssdd",
        $node,
        $time,
        $temp,
        $humidity
    );

    $stmt->execute();

    echo "New sensor reading inserted successfully!";

    $stmt->close();

} catch (mysqli_sql_exception $e) {

    http_response_code(400);

    error_log(
        "Base64 sensor insert failed: " . $e->getMessage()
    );

    echo "Error: Reading was not inserted. ";
    echo "Check the node, values, or timestamp.";
}

// Close database connection
$conn->close();
?>
