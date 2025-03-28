#include <SPI.h>
#include <RH_RF95.h>

RH_RF95 rf95;  // LoRa radio object

// My sender and receiver identifiers are full 9-digit student IDs
uint32_t senderKey = 101240771;       // Sender ID
uint32_t receiverKey = 101240771;     // Receiver's expected ID
uint16_t messageCount = 0;            // Counter for messages

int led = 13; // Onboard LED for activity indication

const char key[] = "sumanrajsharma101240771"; // Hardcoded key
const int keyLength = sizeof(key) - 1; // Exclude null-terminator

// a simple random function that generate dummy C02 emissiond data
int16_t randomCO2EmissionData() {
  // simulating CO2 emissions in µmol/m²/s (based on Flux tower date found in internet)
  // Range: -200 (uptake) to 1000 (high emission)
  return random(-200, 1000);
}

// Encrypts or decrypts data in-place
void xorCipher(uint8_t* data, int length) {
  for (int i = 0; i < length; i++) {
    data[i] ^= key[i % keyLength]; // XOR each byte with repeating key
  }
}

// A simple hashing algorithm for integrity of the payload during trasmission
uint8_t simpleChecksum(uint8_t* data, int length) {
  uint8_t hash = 0;
  for (int i = 0; i < length; i++) {
    hash ^= data[i]; // XOR every byte
  }
  return hash;
}


void setup()
{
  Serial.begin(9600);
  pinMode(led, OUTPUT); // Set LED pin

  if (!rf95.init()) {
    Serial.println("LoRa initialization failed!");
    while (1);
  }

  rf95.setFrequency(915.0);               // Set to 915 MHz
  rf95.setTxPower(20, false);             // Max power
  rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128); // High-speed config
}

void loop()
{
  //Dummy data
  uint16_t co2 = randomCO2EmissionData();

  // Total 10 bytes: 4 (ID) + 4 (receiverKey) + 2 (count)
  uint8_t payload[10];
  memcpy(&payload[0], &receiverKey, 4);        // 4 bytes for sender ID
  memcpy(&payload[4], &receiverKey, 4);      // 4 bytes for receiver key
  memcpy(&payload[8], &co2, 2);     // 2 bytes for message count


  // Debug output
  Serial.print("Senders Key: ");
  Serial.print(receiverKey);
  Serial.print(" | ReceiverKey: ");
  Serial.print(receiverKey);
  Serial.print(" | CO2 Emission: ");
  Serial.print(co2);
  Serial.println(" µmol/m²/s");

  // Encrypt payload before sending
  xorCipher(payload, sizeof(payload));
  Serial.print("Encrypted payload: ");
  for (int i = 0; i < sizeof(payload); i++) {
    Serial.print(payload[i], HEX);
    Serial.print(" ");
  }
  Serial.println();


  // ✅ Compute hash of encrypted payload
  uint8_t hash = simpleChecksum(payload, 10);
  payload[10] = hash; // Store at the end

  Serial.print("Checksum (hash): 0x");
  Serial.println(hash, HEX); 


  // Send the packet
  digitalWrite(led, HIGH);
  rf95.send(payload, sizeof(payload)+1);  // Send 10-byte payload + 1 for the hash
  rf95.waitPacketSent();
  digitalWrite(led, LOW);

  delay(random(4000, 6000)); // Random delay between 4–6 seconds
}
