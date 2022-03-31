#include <Arduino.h>
#include <ESP32Servo.h>
#include <SPIFFS.h>
#include <ESP32QRCodeReader.h>
#include <BluetoothSerial.h>

using namespace std;

#define LOCK_PIN 12

ESP32QRCodeReader scanner(CAMERA_MODEL_AI_THINKER);
BluetoothSerial lockBT;
Servo lockServo;

int authCount = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println();

  scanner.setup();
  scanner.begin();


  lockServo.attach(LOCK_PIN);
}

void closeLock()
{
  lockServo.write(0);
}

void openLock()
{
  lockServo.write(180);
}

void QRScan()
{
  struct QRCodeData qrCode;

  File authList = SPIFFS.open("/access_list.txt", "r");
  if(!authList)
  {
    Serial.println("There was an error opening the access list\n");
    return;
  }
  vector<String> authVect;
  while(authList.available())
  {
    authVect.push_back(authList.readStringUntil('\n'));
  }
  authList.close();
  
  if(scanner.receiveQrCode(&qrCode, 100))
  {
    if(qrCode.valid)
    {
      if(strcmp((char*)qrCode.payload, authVect[0].c_str()) == 0)
      {
        Serial.println("Entered master key code, enabling bluetooth for editing\n");
        lockBT.begin("SmartLock");
        File adminAccessList = SPIFFS.open("/access_list.txt", FILE_WRITE);
        if(!adminAccessList)
        {
          Serial.println("There was an error opening the access list\n");
          return;
        }

        while(true)
        {
          char *btData;
          itoa(lockBT.read(), btData, 10);
          if(strcmp("add", btData) == 0)
          {
            while(true)
            {
              char *newAuthData; 
              itoa(lockBT.read(), newAuthData, 10);
              if(strcmp(btData, newAuthData) != 0)
              {
                adminAccessList.seek(SeekEnd);
                adminAccessList.println(newAuthData);
                authCount++;
              }
            }
          }
          else if(strcmp("delete", btData) == 0)
          {
            while(true)
            {
              char *delAuthData;
              itoa(lockBT.read(), delAuthData, 10);
              if(strcmp(btData, delAuthData) != 0)
              {
                int i = 0;
                if(i == authCount)
                {   
                  lockBT.println("No such entry found\n");
                  return;
                }
                else if(strcmp(delAuthData, authVect[i].c_str()) == 0)
                {
                  uint8_t keySize = authVect[i].length() + 2;
                  uint32_t S = (i-1)*keySize;
                  adminAccessList.seek(S);
                  char rem[keySize + 1];
                  for(uint8_t x = 0; i < (keySize - 2); i++)
                  {
                    rem[x] = ' ';
                  }
                  rem[keySize - 2] = '\r';
                  rem[keySize - 1] = '\n';
                  rem[keySize] = '\0';
                  adminAccessList.print(rem);
                  authCount--;
                  lockBT.println("Successfuly deleted from access list");
                }
                i++;
              }
            }
          }
          else if(strcmp("stop", btData) == 0)
          {
            lockBT.println("Exiting admin mode\n");
            adminAccessList.close();
            lockBT.end();
            return;
          }
        }
      }
      else 
      {
        int i = 0;
        while(true)
        {
          if(i > authCount)
          {
            Serial.println("Access denied, no such key\n");
            return;
          }
          if(strcmp((const char*)qrCode.payload, authVect[i].c_str()) == 0)
          {
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
      return;
    }
  }
}

void loop()
{
  QRScan();
  delay(100);
}