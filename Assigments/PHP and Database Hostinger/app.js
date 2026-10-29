
// EE-470 Assignment 4 - Part 2
// Display temperature data for the selected sensor

let sensorChart = null;

async function loadSensorData(node) {

    // Request data for the selected node
    const response = await fetch(
        'data.php?node=' + encodeURIComponent(node)
    );

    if (!response.ok) {
        throw new Error('Failed to retrieve sensor data');
    }

    const data = await response.json();

    // Extract timestamps and temperatures
    const times = data.map(row => row.time_received);
    const temperatures = data.map(row => Number(row.temperature));

    // Update the graph title
    const nodeNumber = node.split('_')[1];
    const chartTitle = 'Sensor Node ' + nodeNumber;

    const ctx = document.getElementById('sensorChart');

    // Remove the previous graph
    if (sensorChart !== null) {
        sensorChart.destroy();
    }

    // Create graph for the selected node
    sensorChart = new Chart(ctx, {
        type: 'bar',

        data: {
            labels: times,
            datasets: [{
                label: chartTitle,
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
                    text: chartTitle
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

// Get dropdown menu
const sensorSelect = document.getElementById('sensorSelect');

// Update graph when the user selects a node
sensorSelect.addEventListener('change', function() {
    loadSensorData(this.value).catch(console.error);
});

// Display Node 1 when the page first loads
loadSensorData(sensorSelect.value).catch(console.error);
