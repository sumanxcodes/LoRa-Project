#include <SPI.h>
#include <RH_RF95.h>

RH_RF95 rf95;  // LoRa radio object

// both studentID and receiverKey to be full 9-digit uint32_t
uint32_t senderKey = 101240771;     
uint32_t receiverKey = 101240771;   

int led = 13;


const char key[] = "sumanrajsharma101240771"; // Hardcoded key
const int keyLength = sizeof(key) - 1; // Exclude null-terminator

// Encrypts or decrypts data in-place
void xorCipher(uint8_t* data, int length) {
  for (int i = 0; i < length; i++) {
    data[i] ^= key[i % keyLength]; // XOR each byte with repeating key
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);  // LED to show successful reception

  if (!rf95.init()) {
    Serial.println("LoRa receiver init failed");
    while (1); // Stop if LoRa init fails
  }

  rf95.setFrequency(915.0);
  rf95.setTxPower(20, false);
  rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);
}

void loop() {
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = RH_RF95_MAX_MESSAGE_LEN;

  if (rf95.recv(buf, &len)) {
    Serial.print("📦 Received packet length: ");
    Serial.println(len);

    Serial.print("Encrypted payload: ");
    for (int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Decrypt the received buffer befor parsing..
    xorCipher(buf, len);

    // Expecting 10 bytes: [senderKey (4)][receiverKey (4)][msgCount (2)]
    if (len >= 10) {
      uint32_t receivedSenderKey;
      uint32_t receivedReceiverKey;
      uint16_t co2;

      // Parse payload from buffer
      memcpy(&receivedSenderKey,    &buf[0], 4); // Sender's student ID
      memcpy(&receivedReceiverKey,  &buf[4], 4); // Intended receiver ID
      memcpy(&co2,        &buf[8], 2); // Message counter

      // Validate sender and intended receiver
      if (receivedSenderKey == senderKey && receivedReceiverKey == receiverKey) {
        Serial.print("✅ Valid Msg! From: ");
        Serial.print(receivedSenderKey);
        Serial.print(" | CO2 Emission #: ");
        Serial.print(co2);
        Serial.println(" µmol/m²/s");
        digitalWrite(led, HIGH);
      } else {
        Serial.print("⚠ Unauthorized. From: ");
        Serial.print(receivedSenderKey);
        Serial.print(" | Intended for: ");
        Serial.println(receivedReceiverKey);
        digitalWrite(led, LOW);
      }
    } else {
      Serial.println("⚠ Packet too short! Ignored.");
    }
  }
}
