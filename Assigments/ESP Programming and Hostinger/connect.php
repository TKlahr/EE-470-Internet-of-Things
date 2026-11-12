<?php
// Database connection settings
$servername = "localhost";
$username = "u457964232_db_TylerKlahr";
$password = "Winston2003?";
$dbname = "u457964232_TylerKlahr";

// Connect to the database
$conn = new mysqli($servername, $username, $password, $dbname);

// Check the connection
if ($conn->connect_error) {
    error_log("Database connection failed: " . $conn->connect_error);
    die("Database connection failed.");
}

?>
