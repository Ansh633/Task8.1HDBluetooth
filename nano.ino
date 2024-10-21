#include <ArduinoBLE.h>  // Include the Bluetooth library for Arduino Nano 33 IoT

int ledPin = 9;  // Pin where the LED is connected
BLEDevice connectedDevice;  // Declare a global variable to store the connected Bluetooth device

// Function to handle the LED brightness based on the proximity value
void handleLED(int proximityValue) {
  // Map the proximity value (expected to be 0 or 1) to brightness
  int brightness = proximityValue * 255;  // Full brightness when proximity is 1, off when it's 0
  analogWrite(ledPin, brightness);  // Adjust the LED brightness
}

// Function to establish the Bluetooth connection
bool connectToPeripheral() {
  BLE.scanForName("raspberrypi");  // Start scanning for a device with the name "raspberrypi"
  BLEDevice peripheral = BLE.available();  // Look for available Bluetooth devices

  // If a Bluetooth device is found, attempt to connect
  if (peripheral) {
    BLE.stopScan();  // Stop scanning when a device is found
    if (peripheral.connect()) {  // Try to connect to the Raspberry Pi
      Serial.println("Connected to Raspberry Pi");  // Indicate successful connection
      connectedDevice = peripheral;  // Store the connected device globally
      return true;  // Return true if connected
    }
  }
  return false;  // Return false if no device was found or connection failed
}

void setup() {
  Serial.begin(9600);  // Initialize serial communication for debugging
  pinMode(ledPin, OUTPUT);  // Set the LED pin as output
  
  // Initialize Bluetooth
  if (!BLE.begin()) {  // Check if Bluetooth starts correctly
    Serial.println("Failed to initialize Bluetooth!");
    while (1);  // Halt the system if Bluetooth fails
  }
}

void loop() {
  // Attempt to connect to a Bluetooth device if not already connected
  if (!connectedDevice || !connectedDevice.connected()) {
    Serial.println("Searching for Raspberry Pi...");
    if (!connectToPeripheral()) {
      delay(1000);  // Wait before trying to scan again
      return;  // Exit this loop iteration if no connection is made
    }
  }

  // If a Bluetooth device is connected, read incoming data
  if (connectedDevice.connected()) {
    while (connectedDevice.available()) {  // Check if there's data available
      String incomingData = connectedDevice.readString();  // Read the incoming data as a string
      int proximity = incomingData.toInt();  // Convert the string data to an integer (0 or 1)
      
      // Call the function to handle the LED brightness based on proximity
      handleLED(proximity);
    }
  }
}
