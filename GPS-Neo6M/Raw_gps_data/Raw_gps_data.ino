#include <SoftwareSerial.h>

// Set up the extra serial port
// RX is pin 3 (reads data from your device)
// TX is pin 4 (sends data to your device, if needed)
SoftwareSerial mySerial(3, 4); 

void setup() {
  // Start the hardware serial (TX0/RX0) so you can see it on your screen
  Serial.begin(9600);
  
  // Start the software serial to listen to pins 3 and 4
  mySerial.begin(9600);
  
  Serial.println("Listening to pins 3 and 4...");
}

void loop() {
  // Check if any data has arrived on pin 3
  if (mySerial.available()) {
    
    // Read the data from pin 3
    char incomingData = mySerial.read();
    
    // Print that exact data out to TX0 so it shows up on your Serial Monitor
    Serial.print(incomingData); 
  }
}