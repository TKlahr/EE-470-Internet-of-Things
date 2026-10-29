<?php
// Connect to the database
require 'connect.php';

// Retrieve registered sensors
$sql1 = "SELECT * FROM sensor_register ORDER BY node_name";
$result1 = $conn->query($sql1);

// Retrieve sensor readings
$sql2 = "SELECT * FROM sensor_data
         ORDER BY node_name, time_received";
$result2 = $conn->query($sql2);
?>

<!DOCTYPE html>
<html>
<head>
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
    </style>
</head>
<body>

<h1>IoT Sensor Database</h1>

<h2>Registered Sensors</h2>
<?php
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

displayTable($result1);
?>

<h2>Sensor Readings</h2>
<?php
displayTable($result2);
$conn->close();
?>

</body>
</html>
