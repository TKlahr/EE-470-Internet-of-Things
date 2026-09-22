// Define which ESP pins are connected to the HC-SR04
#define echoPin D2
#define trigPin D4

long duration, distance;  // variables used to store the echo pulse time and calculated distance (declared as long integer variable)
// Long instead of int because data from pulseIn can be large

void setup() {
  Serial.begin(9600);  // Start serial communication with the computer at 9600 bits per second (baud rate)

  pinMode(trigPin, OUTPUT);  // TRIG sends the ultrasonic pulse
  pinMode(echoPin, INPUT);  // ECHO receives the returning pulse
}

void loop() {

  digitalWrite(trigPin, LOW);  // Set the TRIG pin to low
  delayMicroseconds(2);  //Ensures previous state is clear before new trigger

  digitalWrite(trigPin, HIGH);  // Send a 10 us high pulse to trigger the HC-SR04
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);  // Measure how long the ECHO pin stays high. Record time in duration
    
  distance = duration / 58.2;  // Conversion of echo travel time in us to distance in cm (Calculated from speed of sound)
  // Speed of Sound = 343 m/s -> 0.0343 cm/us. Distance = (Duration*0.0343)/2 -> Distance = Duration/58.2
    
  String disp = String(distance);  // Convert distance to string for display

  Serial.print("Distance: ");  // Print the measured distance in Serial Monitor
  Serial.print(disp);
  Serial.println(" cm");

  delay(1000);  // Wait 1 second before taking the next measurement
}
