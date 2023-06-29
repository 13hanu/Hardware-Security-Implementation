#include <arduinoFFT.h>
#include "Adafruit_ZeroFFT.h"
#include "signal.h"
#include <Wire.h>
#include <AESLib.h>
#include <SoftwareSerial.h>
#define TX_ID 789

#define ref_dB 94
const int AudioPin = A0; 
const int sampleRate = 20000;
const int frameSize = 64;
const int hopSize = 32;

// Separate arrays for raw data and FFT results
double rawData[frameSize];
double vReal[frameSize];
double vImag[frameSize];

arduinoFFT FFT = arduinoFFT(vReal, vImag, frameSize, sampleRate);


#define PIN_ANALOG_IN A0
#define SampleSize 16
#define TX_PIN 2
#define RX_PIN 3

#define ACK 'A'

SoftwareSerial mySerial(RX_PIN, TX_PIN);

byte aes_key[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(PIN_ANALOG_IN, INPUT);
  Wire.begin();
}

void loop() {

  // Shift rawData
  for (int i = 0; i < frameSize - hopSize; i++) {
    rawData[i] = rawData[i + hopSize];
  }
  // Read new samples into rawData
  for (int i = frameSize - hopSize; i < frameSize; i++) {
     //rawData[i] = analogRead(AudioPin);
    rawData[i] = analogRead(AudioPin) * 5.0 / 1023.0;
  }
  // Copy rawData into vReal and zero out vImag
  for (int i = 0; i < frameSize; i++) {
    vReal[i] = rawData[i];
    vImag[i] = 0;
  }
  // Apply windowing and FFT
  FFT.Windowing(vReal, frameSize, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, frameSize);

  double dBA;

  double sum = 0.0;
  for (int i = 0; i < frameSize / 2; i++) {

    double frequency = (i * 1.0 * sampleRate) / frameSize;
    double f2 = frequency * frequency;
    double f4 = f2 * f2;

    double numerator = 12194.0 * 12194.0 * f4;
    double denominator = (f2 + 20.6 * 20.6) * sqrt((f2 + 107.7 * 107.7) * (f2 + 737.9 * 737.9)) * (f2 + 12194.0 * 12194.0);

    double A_weighting = 2.0 + (20.0 * log10(numerator / denominator));
    
    double magnitude = sqrt(vReal[i]*vReal[i] + vImag[i]*vImag[i]);

    double dB = 20 * log10(magnitude / 1024) + ref_dB;
    double weighted_dB = dB + A_weighting;
    sum += pow(10, weighted_dB / 10);
    
    dBA = 10 * log10(sum);

  }

 
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

  
  Serial.print("\n---------------Transmitter data--------------------");
  Serial.print("\nInput Sound Level(dBA): ");
  Serial.println(dBA); 

   float bitReverseData = (dBA);
  //float bitReverseData = bitReverse(dBA);
  Serial.print("BitReverese of  Input data: ");
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
 //delay(random(1000, 2000));
}

void AESEncrypt(float data, byte* key, byte* result) {
  byte* dataBytes = (byte*)&data;
  AES aes;
  aes.set_key(key, 16);
  aes.encrypt(dataBytes, result);
}
byte bitReverse(byte x) {
  x = ((x & 0xF0) >> 4) | ((x & 0x0F) << 4);
  x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2);
  x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);
  return x;
}
