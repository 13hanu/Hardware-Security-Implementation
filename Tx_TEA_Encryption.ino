#include <SoftwareSerial.h>

const int SensorPin = A0; // Connect the Envelope output to Analog pin 0

// 128-bit key for TEA encryption
uint32_t key[4] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210};

SoftwareSerial softSerial(10, 11); // RX, TX

void encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
    unsigned int i;
    uint32_t v0=v[0], v1=v[1], delta=0x9e3779b9, sum=0;
    for (i=0; i < num_rounds; i++) {
        sum += delta;
        v0 += ((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1]);
        v1 += ((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3]);
    }
    v[0]=v0; v[1]=v1;
}

void setup() {
  Serial.begin(9600); // Start the Serial communication to send data to the computer
  softSerial.begin(4800); // Start software serial
}

void loop() {

  float sensorValue = analogRead(SensorPin); // Read the sensor output
  float voltage = sensorValue * (5.0/1023.0); // Convert the digital data to voltage
  float temperature = (voltage - 0.5) *100;

  uint32_t dataToSend = (uint32_t)(temperature * 100); // Converting float to int

  // Create the data block for encryption
  uint32_t data[2] = {dataToSend, 0};

  // Encrypt the data
  encipher(32, data, key);

  Serial.print("Original data: ");
  Serial.println(temperature);
  
  Serial.print("Encrypted data: ");
  Serial.print(data[0]);
  Serial.print(", ");
  Serial.println(data[1]);
  
  // Send the encrypted data
  softSerial.println(data[0]);
  softSerial.println(data[1]);

  delay(3000); // Wait for a bit to not spam the serial output
}