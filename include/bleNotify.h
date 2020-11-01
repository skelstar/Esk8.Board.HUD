#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// prototypes
template <typename T>
void sendDataToClient(T data);

bool clientConnected = false;

class MyServerCallbacks : public BLECharacteristicCallbacks
{
  // receive

#define CLEAR_TRIP_ODO_COMMAND 99

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    memcpy(&hudData, pCharacteristic->getData(), sizeof(HUDData));
    packetReady = true;
    // Serial.printf("onWrite() got: %d\n", hudData.id);
    // hudData.id++;
    // sendDataToClient(hudData);
  }

  void onConnect(BLEServer *pServer)
  {
    Serial.printf("client connected\n");
    clientConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    Serial.printf("client disconnected\n");
    clientConnected = false;
  }
};

void setupBLE()
{
  BLEDevice::init("M5Atom Server");

  uint8_t mac5[6];
  esp_read_mac(mac5, ESP_MAC_BT);
  Serial.printf("[Bluetooth] Mac Address = %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac5[0], mac5[1], mac5[2], mac5[3], mac5[4], mac5[5]);

  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());

  pCharacteristic->setCallbacks(new MyServerCallbacks());
  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.printf("Characteristic defined! Now you can read it in your phone!\n");
}

template <typename T>
void sendDataToClient(T data)
{
  uint8_t bs[sizeof(data)];
  memcpy(bs, &data, sizeof(data));

  pCharacteristic->setValue(bs, sizeof(bs));
  pCharacteristic->notify();
}
