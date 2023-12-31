
/** NimBLE_Server Demo:
 *
 *  Demonstrates many of the available features of the NimBLE server library.
 *
 *  Created: on March 22 2020
 *      Author: H2zero
 *
 */

#include <NimBLEDevice.h>

static NimBLEServer *pServer;

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer)
  {
    NimBLEDevice::startAdvertising();
  };

  void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
  {
    pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
  };
  void onDisconnect(NimBLEServer *pServer)
  {
    NimBLEDevice::startAdvertising();
  };
  void onMTUChange(uint16_t MTU, ble_gap_conn_desc *desc){};

  uint32_t onPassKeyRequest()
  {
    Serial.println("Server Passkey Request");
    return 123456;
  };

  bool onConfirmPIN(uint32_t pass_key)
  {
    Serial.print("The passkey YES/NO number: ");
    Serial.println(pass_key);
    return true;
  };

  void onAuthenticationComplete(ble_gap_conn_desc *desc)
  {
    if (!desc->sec_state.encrypted)
    {
      NimBLEDevice::getServer()->disconnect(desc->conn_handle);
      // Serial.println("Encrypt connection failed - disconnecting client");
      return;
    }
    Serial.println("Starting BLE work!");
  };
};
void splitString(const String &input, const String &separator, String &part1, String &part2, String &part3, String &part4)
{
  int separatorPos1 = input.indexOf(separator);
  if (separatorPos1 != -1)
  {
    part1 = input.substring(0, separatorPos1);
    int separatorPos2 = input.indexOf(separator, separatorPos1 + separator.length());
    if (separatorPos2 != -1)
    {
      part2 = input.substring(separatorPos1 + separator.length(), separatorPos2);
      int separatorPos3 = input.indexOf(separator, separatorPos2 + separator.length());
      if (separatorPos3 != -1)
      {
        part3 = input.substring(separatorPos2 + separator.length(), separatorPos3);
        part4 = input.substring(separatorPos3 + separator.length());
      }
      else
      {
        // If there is no third separator, extract the remaining part as part3
        part3 = input.substring(separatorPos2 + separator.length());
      }
    }
  }
}
String twoDigits(int number)
{
  if (number < 10)
  {
    return "0" + String(number);
  }
  else
  {
    return String(number);
  }
}
String parseAndFormatDateTime(const String &dateTimeString)
{
  int year, month, day, hour, minute, second;
  sscanf(dateTimeString.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);
  String formattedDate = String(year) + "-" + twoDigits(month) + "-" + twoDigits(day) + " " + twoDigits(hour) + ":" + twoDigits(minute);
  return formattedDate;
}

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
  void onRead(NimBLECharacteristic *pCharacteristic){
      // Serial.print(pCharacteristic->getUUID().toString().c_str());
      // Serial.print(": onRead(), value: ");
      // Serial.println(pCharacteristic->getValue().c_str());
  };

  void onWrite(NimBLECharacteristic *pCharacteristic)
  {
    String input = pCharacteristic->getValue().c_str();
    // AMBIL DATA PARSINGAN BLE DISINI
    if (input.startsWith("full_data:"))
    {
      // INDIKASI START
      input = input.substring(10);
      String namaPetugas, namaTangki, noNIK, datePengukuran;
      splitString(input, "***", namaPetugas, namaTangki, noNIK, datePengukuran);
      Serial.print("Nama Petugas: ");
      Serial.println(namaPetugas);
      Serial.print("Nama Tangki: ");
      Serial.println(namaTangki);
      Serial.print("No. NIK: ");
      Serial.println(noNIK);
      Serial.print("datePengukuran: ");
      Serial.println(parseAndFormatDateTime(datePengukuran));
    }
    // INDIKASI KALIBRASI
    else if (input.startsWith("diameter:"))
    {
      input = input.substring(9);
      int diameter = input.toInt();
      Serial.print("Diameter: ");
      Serial.println(diameter);
    }
    else if (input.equals("stop:"))
    {
      // INDIKASI STOP
    }
    else
    {
      Serial.println("format unkonwn");
    }
  };

  void onNotify(NimBLECharacteristic *pCharacteristic){
      // Serial.println("Sending notification to clients");
  };

  void onStatus(NimBLECharacteristic *pCharacteristic, Status status, int code)
  {
    String str = ("Notification/Indication status code: ");
    str += status;
    str += ", return code: ";
    str += code;
    str += ", ";
    str += NimBLEUtils::returnCodeToString(code);
    Serial.println(str);
  };

  void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValues){};
};

/** Handler class for descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks
{
  void onWrite(NimBLEDescriptor *pDescriptor)
  {
    std::string dscVal = pDescriptor->getValue();
    Serial.print("Descriptor witten value:");
    Serial.println(dscVal.c_str());
  };

  void onRead(NimBLEDescriptor *pDescriptor)
  {
    Serial.print(pDescriptor->getUUID().toString().c_str());
    Serial.println(" Descriptor read");
  };
};
/** Define callback instances globally to use for multiple Charat eristics \ Descriptors */
static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting NimBLE Server");

  /** sets device name */
  NimBLEDevice::init("NimBLE-Arduino");

  /** Optional: set the transmit power, default is 3db */
#ifdef ESP_PLATFORM
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
#else
  NimBLEDevice::setPower(9); /** +9db */
#endif

  NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService *pDeadService = pServer->createService("DEAD");
  NimBLECharacteristic *pBeefCharacteristic = pDeadService->createCharacteristic(
      "BEEF",
      NIMBLE_PROPERTY::READ |
          NIMBLE_PROPERTY::WRITE |
          /** Require a secure connection for read and write access */
          NIMBLE_PROPERTY::READ_ENC | // only allow reading if paired / encrypted
          NIMBLE_PROPERTY::WRITE_ENC  // only allow writing if paired / encrypted
  );

  pBeefCharacteristic->setValue("Burger");
  pBeefCharacteristic->setCallbacks(&chrCallbacks);

  NimBLE2904 *pBeef2904 = (NimBLE2904 *)pBeefCharacteristic->createDescriptor("2904");
  pBeef2904->setFormat(NimBLE2904::FORMAT_UTF8);
  pBeef2904->setCallbacks(&dscCallbacks);

  NimBLEService *pBaadService = pServer->createService("BAAD");
  NimBLECharacteristic *pFoodCharacteristic = pBaadService->createCharacteristic(
      "F00D",
      NIMBLE_PROPERTY::READ |
          NIMBLE_PROPERTY::WRITE |
          NIMBLE_PROPERTY::NOTIFY);

  pFoodCharacteristic->setValue("Fries");
  pFoodCharacteristic->setCallbacks(&chrCallbacks);

  NimBLEDescriptor *pC01Ddsc = pFoodCharacteristic->createDescriptor(
      "C01D",
      NIMBLE_PROPERTY::READ |
          NIMBLE_PROPERTY::WRITE |
          NIMBLE_PROPERTY::WRITE_ENC, // only allow writing if paired / encrypted
      30);
  pC01Ddsc->setValue("Send it back!");
  pC01Ddsc->setCallbacks(&dscCallbacks);

  /** Start the services when finished creating all Characteristics and Descriptors */
  pDeadService->start();
  pBaadService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  /** Add the services to the advertisment data **/
  pAdvertising->addServiceUUID(pDeadService->getUUID());
  pAdvertising->addServiceUUID(pBaadService->getUUID());
  /** If your device is battery powered you may consider setting scan response
   *  to false as it will extend battery life at the expense of less data sent.
   */
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  Serial.println("Advertising Started");
}

float counter;
void loop()
{
  /** Do your thing here, this just spams notifications to all connected clients */
  if (pServer->getConnectedCount())
  {
    NimBLEService *pSvc = pServer->getServiceByUUID("BAAD");
    if (pSvc)
    {
      NimBLECharacteristic *pChr = pSvc->getCharacteristic("F00D");
      if (pChr)
      {
        counter += 1;
        pChr->setValue("height:" + String(counter, 2));
        pChr->notify(true);
      }
    }
  }
  delay(2000);
}