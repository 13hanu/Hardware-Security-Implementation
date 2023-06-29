#include <Wire.h>
#include <AESLib.h>
#include <SoftwareSerial.h>

#define PIN_ANALOG_IN A0
#define SampleSize 16
#define TX_PIN 2
#define RX_PIN 3
#define TX_ID 789

SoftwareSerial mySerial(RX_PIN, TX_PIN);

byte aes_key[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

#define ACK 'A'

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(PIN_ANALOG_IN, INPUT);
  Wire.begin();
}

void loop() {


  float voltage = analogRead(A0) * (5.0 / 1023.0);
  float input = (voltage - 0.5) * 100.0;


  

  byte cipher[SampleSize]; 

  // Send the transmitter ID and wait for an acknowledgment
  mySerial.write(TX_ID);
  unsigned long startTime = millis();
  while (mySerial.available() == 0) {
    if (millis() - startTime > 2000) { // Timeout after 2 seconds
      Serial.println("No ID acknowledgment received. Connection might have failed.");
      return;
    }
  }

  char idAck = mySerial.read();
  if (idAck != ACK) {
    Serial.println("Received wrong ID acknowledgment. Connection might have failed.");
    return;
  }

  Serial.println("Received ID acknowledgment. Connection successful.");

  float bitReverseData = bitReverse(input);

  Serial.print("Input Temperature(`C): ");
  Serial.println(input);

  Serial.print("BitReverse of temperature: ");
  Serial.println(bitReverseData);

  AESEncrypt(bitReverseData, aes_key, cipher);

  Serial.print("Sent encrypted data: ");
  for(int i = 0; i < SampleSize; i++){
    Serial.print(cipher[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Send the encrypted data and wait for an acknowledgment
  mySerial.write(cipher, sizeof(cipher));
  startTime = millis();
  while (mySerial.available() == 0) {
    if (millis() - startTime > 2000) { // Timeout after 2 seconds
      Serial.println("No encryption acknowledgment received. Data transmission might have failed.");
      return;
    }
  }

  char encAck = mySerial.read();
  if (encAck != ACK) {
    Serial.println("Received wrong encryption acknowledgment. Data transmission might have failed.");
    return;
  }

  Serial.println("Received encryption acknowledgment. Data transmission successful.");

  delay(2000);
}

void AESEncrypt(float data, byte* key, byte* result) {
  byte* dataBytes = (byte*)&data;
  AES aes;
  aes.set_key(key, SampleSize);
  aes.encrypt(dataBytes, result);
}

byte bitReverse(byte x) {
  x = ((x & 0xF0) >> 4) | ((x & 0x0F) << 4);
  x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2);
  x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);
  return x;
}