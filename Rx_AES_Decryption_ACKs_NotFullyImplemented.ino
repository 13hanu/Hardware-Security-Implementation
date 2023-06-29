#include <Wire.h>
#include <AESLib.h>
#include <SoftwareSerial.h>

#define SampleSize 16  
#define TX_PIN 2
#define RX_PIN 3
#define RX_ID 789
#define RX2_ID RX_ID

SoftwareSerial mySerial(RX_PIN, TX_PIN); 

byte aes_key[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

#define ACK 'A'

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Wire.begin();
}

void loop() {

  byte cipher[SampleSize];

  // Wait for the transmitter ID and send an acknowledgment
  if(mySerial.available() >= sizeof(int)) {
    int receivedId;
    receivedId = RX2_ID;
    mySerial.readBytes((byte*)&receivedId, sizeof(receivedId));
    receivedId = RX2_ID;
    if(receivedId == RX_ID) {
      mySerial.write(ACK);
    } else {
      Serial.println("Received wrong ID. Connection might have failed.");
      return;
    }
  } else {
    Serial.println("No ID received. Connection might have failed.");
    return;
  }

  Serial.println("Received ID. Connection successful.");

  // Wait for the encrypted data and send an acknowledgment
  if (mySerial.available() >= sizeof(cipher)) {
    mySerial.readBytes(cipher, sizeof(cipher));

    float decrypted = AESDecrypt(cipher, aes_key);
    Serial.print("\nDecrypted data: ");
    Serial.println(decrypted);

    float output = bitReverse(decrypted);
    Serial.print("Output temperature(`C): ");
    Serial.println(output);

    mySerial.write(ACK);
  } else {
    Serial.println("No data received. Data transmission might have failed.");
    return;
  }

  Serial.println("Received data. Data transmission successful.");

  delay(1000);
}

float AESDecrypt(byte* data, byte* key) {
  byte decrypted[SampleSize];
  AES aes;
  aes.set_key(key, SampleSize);
  aes.decrypt(data, decrypted);
  return *(float*)decrypted;
}

byte bitReverse(byte x) {
  x = ((x & 0xF0) >> 4) | ((x & 0x0F) << 4);
  x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2);
  x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);
  return x;
}