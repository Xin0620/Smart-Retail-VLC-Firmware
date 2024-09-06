#include "BluetoothUtils.h"

bool BluetoothUtils::previousEnabledState = true;
NimBLEServer *BluetoothUtils::bleServer;
NimBLECharacteristic *BluetoothUtils::coordCharacteristic;

class ServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer)
  {
    Serial.println("BLE: Device connected");
    GlobalVariables::bleClients++;
    pServer->getAdvertising()->start();
  };
  void onDisconnect(NimBLEServer *pServer)
  {
    Serial.println("BLE: Device disconnected");
    GlobalVariables::bleClients--;
    pServer->getAdvertising()->start();
  }
};

void BluetoothUtils::setup()
{
  Serial.println("Initializing Bluetooth Low-Energy (BLE)");
  // Server
  NimBLEDevice::init(GlobalVariables::trolleyId);
  bleServer = NimBLEDevice::createServer();
  bleServer->advertiseOnDisconnect(true);
  bleServer->setCallbacks(new ServerCallbacks());

  // Service
  NimBLEService *pService = bleServer->createService(SERVICE_UUID);

  // Location characteristic
  coordCharacteristic = pService->createCharacteristic(
      COORDINATES_CHAR_UUID,
      NIMBLE_PROPERTY::READ); // BLECharacteristic::PROPERTY_WRITE
  updateCoordinatesCharacteristics(String("0, 0"));
  pService->start();

  // Advertising
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();
}

void BluetoothUtils::run() {
  if(GlobalVariables::trolleyEnabled && !previousEnabledState) {
    Serial.println("Restart BLE server");
    previousEnabledState = true;
    setup();
  }
  else if(!GlobalVariables::trolleyEnabled && previousEnabledState) {
    Serial.println("Stop BLE server");
    previousEnabledState = false;
    std::vector<uint16_t> connectedDevices = bleServer->getPeerDevices();
    for(uint16_t deviceId : connectedDevices) {
      Serial.println("Disconnect this device");
      bleServer->disconnect(deviceId);
    }
    NimBLEDevice::deinit(true);
  }
}

void BluetoothUtils::updateCoordinatesCharacteristics(String coordinates)
{
    coordCharacteristic->setValue(coordinates);
}