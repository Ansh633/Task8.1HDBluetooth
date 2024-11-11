#include <ArduinoBLE.h>
#include <Wire.h>
#include <BH1750.h>

// Define BLE service and characteristic UUIDs for the LED control and light sensor data
const char* serviceUUID = "7546d197-156d-48a1-848b-7a27a4a5dc3a";  // Service UUID
const char* charUUID = "ad1ad0d9-a4a4-4879-9d02-ec57410dab48";      // Characteristic UUID

// Create BLE service and characteristic objects
BLEService ledService(serviceUUID);  // Define BLE service for LED control and light data
BLEIntCharacteristic lightCharacteristic(charUUID, BLERead | BLEWrite);  // Integer characteristic for light level

// Initialize BH1750 light sensor on I2C address 0x23 (default address)
BH1750 lightSensor(0x23);

// Variables for managing light sensor reading intervals
unsigned long lastMeasurementTime = 0;         // Tracks the time of the last measurement
const unsigned long measurementInterval = 1000; // Interval between measurements in milliseconds

void setup() {
    // Start serial communication for debugging purposes
    Serial.begin(9600);
    while (!Serial);  // Wait for the serial connection to initialize

    // Initialize BLE functionality
    Serial.println("Starting BLE...");
    if (!BLE.begin()) {  // If BLE initialization fails, print an error and halt the program
        Serial.println("Failed to start BLE module!");
        while (1);
    }
    Serial.println("BLE module started successfully.");

    // Set up BLE settings
    BLE.setLocalName("LED");                 // Name displayed during BLE scanning
    BLE.setAdvertisedService(ledService);    // Link the service to the BLE peripheral
    ledService.addCharacteristic(lightCharacteristic);  // Add characteristic to the service
    BLE.addService(ledService);              // Add the service to BLE peripheral
    BLE.advertise();                         // Start advertising the BLE service
    Serial.println("Advertising BLE Light Sensor Peripheral...");

    // Initialize I2C communication for the BH1750 light sensor
    Wire.begin();
    if (lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {  // High resolution mode for continuous readings
        Serial.println("BH1750 Sensor initialized.");
    } else {
        Serial.println("Error initializing BH1750 sensor.");
        while (1);  // If sensor initialization fails, halt the program
    }
}

void loop() {
    // Wait for a central device to connect to the BLE peripheral
    BLEDevice central = BLE.central();

    // Check if a central device has connected
    if (central) {
        Serial.print("Connected to central: ");
        Serial.println(central.address());  // Print the central's address

        // Loop while the central device remains connected
        while (central.connected()) {
            // Check if enough time has passed since the last measurement
            if (millis() - lastMeasurementTime >= measurementInterval) {
                lastMeasurementTime = millis();  // Update the time of the last measurement

                // Read the light level in lux (ambient light intensity) from the sensor
                float lux = lightSensor.readLightLevel();

                // Validate the reading (BH1750 returns -1 if reading fails)
                if (lux >= 0) {
                    Serial.print("Light Intensity: ");
                    Serial.print(lux);
                    Serial.println(" lx");

                    // Convert the light intensity to an integer value for BLE transmission
                    int luxInt = static_cast<int>(lux);

                    // Send the light intensity data over BLE to the connected device
                    lightCharacteristic.writeValue(luxInt);
                } else {
                    Serial.println("Failed to read light level.");
                }
            }

            // Call BLE.poll() to handle BLE events, including reads/writes to the characteristic
            BLE.poll();
        }

        // Print a message when the central device disconnects
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }

    // Keep BLE running to listen for new connections when no central is connected
    BLE.poll();
}
