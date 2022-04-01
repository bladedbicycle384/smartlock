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
          String btData = lockBT.readString();
          if(strcmp("add", btData.c_str()) == 0)
          {
            while(true)
            {
              String newAuthData = lockBT.readString();
              if(strcmp("stop", newAuthData.c_str()) == 0)
              {
                lockBT.println("Exited addition mode");
                break;
              }
              else if(newAuthData.length() > 0)
              {
                adminAccessList.seek(SeekEnd);
                adminAccessList.println(newAuthData);
                authCount++;
              }
            }
          }
          else if(strcmp("delete", btData.c_str()) == 0)
          {
            while(true)
            {
              bool stopped = false;
              bool found = true;

              String delAuthData = lockBT.readString();
              if(delAuthData.length() != 0)
              {
                for(int i = 0; i < authCount; i++)
                {
                  if(strcmp("stop", delAuthData.c_str()) == 0)
                  {
                    stopped = true;
                    break;
                  }
                  else if(strcmp(delAuthData.c_str(), authVect[i].c_str()) == 0)
                  {
                    adminAccessList.seek(SeekSet);
                    uint8_t delSize = authVect[i].length() + 1;
                    for(int x= 0; x < authCount; x++)
                    {
                      String testDel = adminAccessList.readStringUntil('\n');
                      if(strcmp(authVect[i].c_str(), testDel.c_str()) == 0)
                      {
                        adminAccessList.seek(-testDel.length(), SeekCur);
                        break;
                      }
                    }
                    char rem[delSize + 1];
                    for(uint8_t x = 0; i < (delSize - 1); i++)
                    {
                      rem[x] = ' ';
                    }
                    rem[delSize - 1] = '\r';
                    rem[delSize] = '\n';
                    adminAccessList.print(rem);
                    lockBT.println("Successfuly deleted from access list");
                    break;
                  }
                  if(i == authCount - 1)
                  {
                    found = false;
                  }
                }

                if(found == false)
                {
                  lockBT.println("No such entry found");
                }
                if(stopped == true)
                {
                  lockBT.println("Exited deletion mode");
                  break;
                }
              }
            }
          }
          else if(strcmp("stop", btData.c_str()) == 0)
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
          if(i = authCount)
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
  delay(1000);
}