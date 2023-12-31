#include <SoftwareSerial.h>

const int SensorPin = A0; // Connect the Envelope output to Analog pin 0
const int key = 0x65; // Encryption Key in HEX

SoftwareSerial softSerial(2, 3); // RX, TX

void setup() {
  Serial.begin(9600); // Start the Serial communication to send data to the computer
  softSerial.begin(4800); // Start software serial
}

void loop() {
  
  float sensorValue = analogRead(SensorPin); // Read the sensor output
  float voltage = sensorValue * (5.0/1023.0); // Convert the digital data to voltage
  float temperature = (voltage - 0.5) *100;

  int dataToSend = (int)(temperature * 100); // Converting float to int
  
  Serial.print("Original data: ");
  Serial.println(dataToSend);

  int encryptedData = dataToSend ^ key;
  
  Serial.print("Encrypted data: ");
  Serial.println(encryptedData);
  
  softSerial.println(encryptedData); // Transmit the encrypted data

  delay(3000); // Wait for a bit to not spam the serial output
}