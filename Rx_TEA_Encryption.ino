#include <SoftwareSerial.h>

// 128-bit key for TEA encryption
uint32_t key[4] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210};

SoftwareSerial softSerial(10, 11); // RX, TX

void decipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
    unsigned int i;
    uint32_t v0=v[0], v1=v[1], delta=0x9e3779b9, sum=delta*num_rounds;
    for (i=0; i < num_rounds; i++) {
        v1 -= ((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3]);
        v0 -= ((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1]);
        sum -= delta;
    }
    v[0]=v0; v[1]=v1;
}

void setup() {
  Serial.begin(9600); // Start the Serial communication to receive data from the computer
  softSerial.begin(4800); // Start software serial
}

void loop() {
  while(softSerial.available()) {
    // Read the two 32-bit parts of the encrypted data
    String receivedData1 = softSerial.readStringUntil('\n');
    String receivedData2 = softSerial.readStringUntil('\n');
    
    receivedData1.trim(); // Remove any trailing whitespace or newline character
    receivedData2.trim(); // Remove any trailing whitespace or newline character

    // Create the data block for decryption
    uint32_t data[2] = {receivedData1.toInt(), receivedData2.toInt()};

    // Decrypt the data
    decipher(32, data, key);

    Serial.print("Received encrypted data: ");
    Serial.print(receivedData1);
    Serial.print(", ");
    Serial.println(receivedData2);
    
    float dB = (float) data[0] / 100.0;
    
    Serial.print("Decrypted data: ");
    Serial.println(dB);

    delay(3000); // Wait for a bit to not spam the serial output
  }
}
