// Define which ESP pins are connected to the HC-SR04
#define echoPin D2
#define trigPin D4

long duration, distance;  // Variables used to store the echo pulse time and calculated distance (declared as long integer variable)
// Long instead of int because data from pulseIn can be large

long readings[3];  //Variable readings that stores the three measured values in a matrix to find the median

int count = 0;  // Tracker for how many median results have been recorded.

void setup() {
  Serial.begin(9600);  // Start serial communication with the computer at 9600 bits per second (baud rate)

  pinMode(trigPin, OUTPUT);   // TRIG sends the ultrasonic pulse
  pinMode(echoPin, INPUT);  // ECHO receives the returning pulse
}

void loop() {

  if (count >= 50) { 
    return;  // Stop taking measurements after 50 median results
  }

  for (int i = 0; i < 3; i++) {   // Take 3 measurements, 2 seconds apart (i starts at 0 and increases by 1 until it reaches 3)

    digitalWrite(trigPin, LOW);  // Set the TRIG pin to low
    delayMicroseconds(2);  //Ensures previous state is clear before new trigger

    digitalWrite(trigPin, HIGH);  // Send a 10 us high pulse to trigger the HC-SR04
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);   // Measure how long the ECHO pin stays high. Record time in duration
    
    distance = duration / 58.2;  // Conversion of echo travel time in us to distance in cm (Calculated from speed of sound)
  // Speed of Sound = 343 m/s -> 0.0343 cm/us. Distance = (Duration*0.0343)/2 -> Distance = Duration/58.2

    readings[i] = distance;  //Store each measured distance in the array (i=0,1,2)

    delay(2000);  // Wait 2 seconds before taking the next measurement
  }

  // Find median of the 3 measurements
  long median;  //variable used to store the median

  if ((readings[0] >= readings[1] && readings[0] <= readings[2]) ||  //Check if reading 0 is between readings 1 and 2
      (readings[0] <= readings[1] && readings[0] >= readings[2])) {
    median = readings[0];
  }
  else if ((readings[1] >= readings[0] && readings[1] <= readings[2]) ||  // Check if reading 1 is between readings 0 and 2
           (readings[1] <= readings[0] && readings[1] >= readings[2])) {
    median = readings[1];
  }
  else {
    median = readings[2];  // If else, then reading 2 must be the median
  }

  count++;  // Increase count variable by 1

  Serial.print(count);  // Display the result number
  Serial.print(": Median Distance: ");
  Serial.print(median);  // Display median distance
  Serial.println(" cm");
}
