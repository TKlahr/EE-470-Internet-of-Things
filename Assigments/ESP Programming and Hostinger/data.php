<?php
/*
 * ------------------------------------------------------------
 * Project/Program Name : EE-470 ESP8266 IoT Sensor Monitoring
 * File Name            : data.php
 * Author               : Tyler Klahr
 * Date                 : 11/11/2026
 * Version              : 1.0
 *
 * Purpose:
 *   Retrieve sensor measurements and statistics from MySQL
 *   for display on the IoT website dashboard.
 *
 * Inputs:
 *   Optional GET parameter: node (node_1 or node_2)
 *
 * Outputs:
 *   JSON containing sensor readings, summary statistics,
 *   and stored message activity counts.
 *
 * Example Application:
 *   Supply data to a Chart.js environmental monitoring
 *   dashboard.
 *
 * Dependencies:
 *   PHP, MySQL, connect.php
 *
 * Usage Notes:
 *   Defaults to node_1 if no node is specified.
 *   Uses only stored database measurements.
 * ------------------------------------------------------------
 */

header('Content-Type: application/json; charset=utf-8');

require 'connect.php';

// Select node (default: node_1)
$node = $_GET['node'] ?? 'node_1';

if (!is_string($node) ||
    !in_array($node, ['node_1', 'node_2'], true)) {
    http_response_code(400);
    echo json_encode(['error' => 'Invalid node']);
    exit;
}

try {
    // Retrieve measurements in chronological order
    $sql = "SELECT
                time_received,
                temperature,
                humidity,
                light_level
            FROM sensor_data
            WHERE node_name = ?
            ORDER BY time_received ASC";

    $stmt = $conn->prepare($sql);
    $stmt->bind_param("s", $node);
    $stmt->execute();

    $result = $stmt->get_result();

    $measurements = [];

    while ($row = $result->fetch_assoc()) {
        $measurements[] = [
            'timestamp' => $row['time_received'],
            'temperature' => $row['temperature'] === null
                ? null : (float)$row['temperature'],
            'humidity' => $row['humidity'] === null
                ? null : (float)$row['humidity'],
            'light' => $row['light_level'] === null
                ? null : (int)$row['light_level']
        ];
    }

    $stmt->close();

    // Calculate statistics
    $sql = "SELECT
                COUNT(*) AS measurement_count,
                AVG(temperature) AS avg_temperature,
                MIN(temperature) AS min_temperature,
                MAX(temperature) AS max_temperature,
                AVG(humidity) AS avg_humidity,
                MIN(humidity) AS min_humidity,
                MAX(humidity) AS max_humidity,
                AVG(light_level) AS avg_light,
                MIN(light_level) AS min_light,
                MAX(light_level) AS max_light
            FROM sensor_data
            WHERE node_name = ?";

    $stmt = $conn->prepare($sql);
    $stmt->bind_param("s", $node);
    $stmt->execute();

    $stats = $stmt->get_result()->fetch_assoc();
    $stmt->close();

    // Retrieve SensorActivity count
    $sql = "SELECT message_count
            FROM sensor_activity
            WHERE node_name = ?";

    $stmt = $conn->prepare($sql);
    $stmt->bind_param("s", $node);
    $stmt->execute();

    $activity = $stmt->get_result()->fetch_assoc();
    $stmt->close();

    // Return JSON
    echo json_encode([
        'node' => $node,
        'measurements' => $measurements,
        'statistics' => [
            'count' => (int)$stats['measurement_count'],
            'avg_temperature' => $stats['avg_temperature'] === null
                ? null : round((float)$stats['avg_temperature'], 2),
            'min_temperature' => $stats['min_temperature'] === null
                ? null : (float)$stats['min_temperature'],
            'max_temperature' => $stats['max_temperature'] === null
                ? null : (float)$stats['max_temperature'],
            'avg_humidity' => $stats['avg_humidity'] === null
                ? null : round((float)$stats['avg_humidity'], 2),
            'min_humidity' => $stats['min_humidity'] === null
                ? null : (float)$stats['min_humidity'],
            'max_humidity' => $stats['max_humidity'] === null
                ? null : (float)$stats['max_humidity'],
            'avg_light' => $stats['avg_light'] === null
                ? null : round((float)$stats['avg_light'], 2),
            'min_light' => $stats['min_light'] === null
                ? null : (int)$stats['min_light'],
            'max_light' => $stats['max_light'] === null
                ? null : (int)$stats['max_light']
        ],
        'activity_count' => $activity
            ? (int)$activity['message_count'] : 0
    ]);

} catch (mysqli_sql_exception $e) {
    http_response_code(500);
    error_log("Dashboard data error: " . $e->getMessage());
    echo json_encode(['error' => 'Database query failed']);
}

$conn->close();
?>
