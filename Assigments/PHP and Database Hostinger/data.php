<?php
// EE-470 Assignment 4 - Part 2
// Retrieve sensor data for Chart.js

require 'connect.php';

header('Content-Type: application/json; charset=utf-8');

// Select one sensor node
$node = $_GET['node'] ?? 'node_1';

// Retrieve temperature readings ordered by time
$sql = "SELECT node_name, time_received,
               temperature, humidity
        FROM sensor_data
        WHERE node_name = ?
        ORDER BY time_received";

$stmt = $conn->prepare($sql);
$stmt->bind_param("s", $node);
$stmt->execute();

$result = $stmt->get_result();

$data = [];

while ($row = $result->fetch_assoc()) {
    $data[] = $row;
}

// Return the readings as JSON
echo json_encode($data, JSON_PRETTY_PRINT);

$stmt->close();
$conn->close();
?>
