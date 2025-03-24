#include <SPI.h>
#include <RH_RF95.h>

RH_RF95 rf95;  // LoRa radio object

// My sender and receiver identifiers are full 9-digit student IDs
uint32_t senderKey = 101240771;       // Sender ID
uint32_t receiverKey = 101240771;     // Receiver's expected ID
uint16_t messageCount = 0;            // Counter for messages

int led = 13; // Onboard LED for activity indication

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
  messageCount++;

  // Total 10 bytes: 4 (ID) + 4 (receiverKey) + 2 (count)
  uint8_t payload[10];
  memcpy(&payload[0], &receiverKey, 4);        // 4 bytes for sender ID
  memcpy(&payload[4], &receiverKey, 4);      // 4 bytes for receiver key
  memcpy(&payload[8], &messageCount, 2);     // 2 bytes for message count

  // Debug output
  Serial.print("Senders Key: ");
  Serial.print(receiverKey);
  Serial.print(" | ReceiverKey: ");
  Serial.print(receiverKey);
  Serial.print(" | Msg #: ");
  Serial.println(messageCount);

  // Send the packet
  digitalWrite(led, HIGH);
  rf95.send(payload, sizeof(payload));  // Send 10-byte payload
  rf95.waitPacketSent();
  digitalWrite(led, LOW);

  delay(random(4000, 6000)); // Random delay between 4–6 seconds
}
