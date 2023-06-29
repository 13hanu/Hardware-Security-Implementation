#include <SoftwareSerial.h>

const int key = 0x65; // Encryption Key in HEX

SoftwareSerial softSerial(2, 3); // RX, TX

void setup() {
  Serial.begin(9600); // Start the Serial communication to receive data from the computer
  softSerial.begin(4800); // Start software serial
}

void loop() {
  while(softSerial.available()) {
    String receivedData = softSerial.readStringUntil('\n');
    
    receivedData.trim(); // Remove any trailing whitespace or newline character
    
    Serial.print("Received encrypted data: ");
    Serial.println(receivedData);
    
    int decryptedData = receivedData.toInt() ^ key;
    
    float dB = (float) decryptedData / 100.0;
    
    Serial.print("Decrypted data: ");
    Serial.println(dB);

    delay(3000); // Wait for a bit to not spam the serial output
  }
}
