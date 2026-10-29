<?php
// EE-470 Assignment 4 - Part 2
// Display sensor data and calculate averages

// Connect to the database
require 'connect.php';

// Retrieve registered sensors
$sql1 = "SELECT * FROM sensor_register ORDER BY node_name";
$result1 = $conn->query($sql1);

// Retrieve sensor readings
$sql2 = "SELECT * FROM sensor_data
         ORDER BY node_name, time_received";
$result2 = $conn->query($sql2);

// Calculate averages for node_1
$sql3 = "SELECT AVG(temperature) AS avg_temp,
                AVG(humidity) AS avg_humidity
         FROM sensor_data
         WHERE node_name = 'node_1'";

$result3 = $conn->query($sql3);
$averages = $result3->fetch_assoc();

// Function to display database results as HTML tables
function displayTable($result) {
    if (!$result) {
        echo "<p>Unable to retrieve data.</p>";
        return;
    }

    if ($result->num_rows == 0) {
        echo "<p>No records found.</p>";
        return;
    }

    echo "<table><tr>";

    // Display column names
    foreach ($result->fetch_fields() as $field) {
        echo "<th>" . htmlspecialchars($field->name) . "</th>";
    }
    echo "</tr>";

    // Display database records
    while ($row = $result->fetch_assoc()) {
        echo "<tr>";
        foreach ($row as $value) {
            echo "<td>" . htmlspecialchars((string)$value) . "</td>";
        }
        echo "</tr>";
    }

    echo "</table>";
}
?>

<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>IoT Sensor Database</title>

    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 30px;
        }

        table {
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 30px;
        }

        th, td {
            border: 1px solid #ccc;
            padding: 10px;
            text-align: left;
        }

        th {
            background-color: #e6e6e6;
        }

        h1 {
            margin-bottom: 30px;
        }

        h2 {
            margin-top: 35px;
        }

        .averages {
            background-color: #f2f2f2;
            padding: 20px;
            border: 1px solid #ccc;
            margin-top: 15px;
            margin-bottom: 30px;
        }
    </style>
</head>

<body>

<h1>IoT Sensor Database</h1>

<h2>Registered Sensors</h2>
<?php
displayTable($result1);
?>

<h2>Sensor Readings</h2>
<?php
displayTable($result2);
?>

<h2>Node 1 Sensor Averages</h2>

<div class="averages">
<?php
if ($averages['avg_temp'] !== null) {

    echo "<p>Average Temperature for Node 1: "
         . number_format($averages['avg_temp'], 2)
         . " &deg;C</p>";

    echo "<p>Average Humidity for Node 1: "
         . number_format($averages['avg_humidity'], 2)
         . " %</p>";

} else {
    echo "<p>No readings available for Node 1.</p>";
}
?>
</div>

</body>
</html>

<?php
// Close database connection
$conn->close();
?>
