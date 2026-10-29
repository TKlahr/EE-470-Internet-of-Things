// EE-470 Assignment 4 - Part 2
// Plot sensor temperature using Chart.js

async function loadSensorData() {

    // Retrieve readings from our PHP file
    const response = await fetch('data.php?node=node_1');
    const data = await response.json();

    // Extract timestamps and temperatures
    const times = data.map(row => row.time_received);
    const temperatures = data.map(row => Number(row.temperature));

    // Get the canvas element
    const ctx = document.getElementById('sensorChart');

    // Create the bar graph
    new Chart(ctx, {
        type: 'bar',

        data: {
            labels: times,
            datasets: [{
                label: 'Sensor Node 1',
                data: temperatures,
                backgroundColor: 'rgba(54, 162, 235, 0.6)',
                borderColor: 'rgba(54, 162, 235, 1)',
                borderWidth: 1
            }]
        },

        options: {
            responsive: true,
            plugins: {
                title: {
                    display: true,
                    text: 'Sensor Node 1'
                }
            },
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Time'
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: 'Temperature (°C)'
                    },
                    beginAtZero: true
                }
            }
        }
    });
}

// Load graph when page opens
loadSensorData();
