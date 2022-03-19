#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP32QRCodeReader.h>
#include <BluetoothSerial.h>

#define LOCK_PIN 12

ESP32QRCodeReader scanner(CAMERA_MODEL_AI_THINKER);
BluetoothSerial SerialBT;
int authCount = 0;
DynamicJsonDocument accessList(10240);
accessList["masterKey"] = "sample_master_key";

void closeLock()
{
  digitalWrite(LOCK_PIN, LOW);
}

void openLock()
{
  digitalWrite(LOCK_PIN, HIGH);
}

void QRScan()
{
  struct QRCodeData qrCode;

  if(scanner.receiveQrCode(&qrCode, 100))
  {
    if(qrCode.valid)
    {
      if(strcmp((const char*)qrCode.payload, accessList["masterKey"]) == 0)
      {
        Serial.println("Entered master key code, enabling bluetooth for editing\n");
        SerialBT.begin("SmartLock");
        
        while(true)
        {
          char *btData;
          itoa(SerialBT.read(), btData, 10);
          if(strcmp("add", btData) == 0)
          {
            while(true)
            {
              char *newAuthData; 
              itoa(SerialBT.read(), newAuthData, 10);
              if(strcmp(btData, newAuthData) != 0)
              {
                accessList["authUsers"][authCount] = SerialBT.read();
                authCount++;
                return;
              }
            }
          }
          else if(strcmp("delete", btData) == 0)
          {
            while(true)
            {
              char *delAuthData;
              itoa(SerialBT.read(), delAuthData, 10);
              if(strcmp(btData, delAuthData) != 0)
              {
                int i = 0;
                if(i == authCount)
                {
                  Serial.println("No such entry found\n");
                  return;
                }
                else if(strcmp(delAuthData, accessList["authUsers"][i]) == 0)
                {
                  accessList["authUsers"][i].remove;
                  authCount--;
                  Serial.println("Successfuly deleted from access list");
                  return;
                }
                i++;
              }
            }
          } 
        }
      }
      else 
      {
        while(true)
        {
          int i = 0;
          if(i == authCount)
          {
            Serial.println("Access denied\n");
            return;
          }
          if(strcmp((const char*)qrCode.payload, accessList["authUsers"][i]) == 0)
          {
            // open lock
            Serial.println("QR key accepted, opening lock\n");
            openLock();
            delay(10000);
            closeLock();
            return;
          }
          i++;
        }
      }
    }
    else
    {
      Serial.println("Invalid data\n");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  scanner.setup();
  scanner.begin();

  pinMode(LOCK_PIN, OUTPUT);
}

void loop()
{
  QRScan();
  delay(100);
}