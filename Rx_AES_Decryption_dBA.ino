#include <Wire.h>
#include <AESLib.h>
#include <SoftwareSerial.h>

#define SampleSize 16  
#define TX_PIN 2
#define RX_PIN 3

SoftwareSerial mySerial(RX_PIN, TX_PIN); 

byte aes_key[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Wire.begin();
}
void loop() {
  byte cipher[SampleSize];
  if (mySerial.available() >= sizeof(cipher)) {
    mySerial.readBytes(cipher, sizeof(cipher));
    Serial.print("\n");
    Serial.print("\n---------------Receiver data-----------------------");
     Serial.print("\nReceived encrypted data: ");
      for (int i = 0; i < sizeof(cipher); i++) {
     Serial.print(cipher[i], HEX);
     Serial.print(" ");
  }
    float decrypted = AESDecrypt(cipher, aes_key);
    Serial.print("\nDecrypted data (bitreverse of input): ");
    Serial.println(decrypted);

    float output = bitReverse(decrypted);
    //float output = decrypted;
    Serial.print("Bitreversed Decryped data, Output Sound Level (dBA): ");
    Serial.println(output);
    Serial.print("\n---------------------------------------------------");
    // Send acknowledgement back to the transmitter
    mySerial.write('A');
  }
 delay(2000);
 // delay(random(1000, 2000));
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