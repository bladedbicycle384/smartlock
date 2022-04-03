#include <Arduino.h>
#include <esp_camera.h>
#include <ESP32Servo.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESP32QRCodeReader.h>
#include <BluetoothSerial.h>

using namespace std;

#define LOCK_PIN 12

ESP32QRCodeReader scanner(CAMERA_MODEL_AI_THINKER);
BluetoothSerial lockBT;
Servo lockServo;

int authCount;
int testCount = 1;
String add = "add\r";
String del = "delete\r";
String stp = "stop\r";
struct QRCodeData qrCode;

void setup()
{
  Serial.begin(115200);

  SPIFFS.begin(true);

  scanner.setup();
  scanner.begin();

  delay(5000);

  // SPIFFS.remove("accesslist.txt");

  // File root = SPIFFS.open("/");
  // File file = root.openNextFile();
  // Serial.println("showcasing files.");

  // while(file)
  // {
  //   Serial.print("FILE: ");
  //   Serial.println(file.name());
  //   file = root.openNextFile();
  // }

  // File authList = SPIFFS.open("/accesslist.txt", FILE_WRITE);
  // if(!authList)
  // {
  //   Serial.println("error opening file");
  //   return;
  // }
  // if(authList.println("masterkey"))
  // {
  //   Serial.println("File was written");
  // }
  // else
  // {
  //   Serial.println("file write failed");
  // }
  // if(authList.println("testkey1"))
  // {
  //   Serial.println("File was written");
  // }
  // else
  // {
  //   Serial.println("file write failed");
  // }
  // if(authList.println("test key 2"))
  // {
  //   Serial.println("File was written");
  // }
  // else
  // {
  //   Serial.println("file write failed");
  // }
  // authList.close();


  // while(authList.available())
  // {
  //   Serial.write(authList.read());
  // }

  lockServo.attach(LOCK_PIN);
}

void closeLock()
{
  lockServo.write(0);
}

void openLock()
{
  lockServo.write(90);
}

void QRScan()
{
  Serial.println("before opening file");

  File authList = SPIFFS.open("/accesslist.txt");
  if(!authList)
  {
    Serial.println("There was an error opening the access list\n");
    return;
  }
  Serial.println("Scanning for qr");

  vector<String> authVect;
  while(authList.available())
  {
    authVect.push_back(authList.readStringUntil('\n'));
  }
  authList.close();

  // for(String s : authVect)
  // {
  //   Serial.println(s);
  // }

  Serial.println(testCount);
  Serial.println("");
  testCount++;
  delay(2000);

  if(scanner.receiveQrCode(&qrCode, 100))
  {
    Serial.print("QR scanned ");
    String qrData = (const char*)qrCode.payload;
    qrData += '\r';
    if(qrCode.valid)
    {
      Serial.print("Valid: ");
      Serial.println(qrData);

  	  // for(String s: authVect)
      // {
      //   Serial.println(s);
      //   Serial.println(qrData);
      //   if(s == qrData)
      //   {
      //     Serial.println("identical!?");
      //   }
      //   else
      //   {
      //     Serial.println("sadly not :pensive:");
      //   }
      // }

      if(qrData == authVect[0])
      {
        Serial.println("Entered master key code, enabling bluetooth for editing\n");
        lockBT.begin("SmartLock");
        File adminAccessList = SPIFFS.open("/accesslist.txt", FILE_WRITE);
        if(!adminAccessList)
        {
          Serial.println("There was an error opening the access list\n");
          return;
        }

        while(true)
        {
          String btData = lockBT.readString();
          btData += '\r';
          if(add == btData)
          {
            while(true)
            {
              String newAuthData = lockBT.readString();
              newAuthData += '\r';
              if(stp == newAuthData)
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
          else if(del == btData)
          {
            while(true)
            {
              bool stopped = false;
              bool found = true;

              String delAuthData = lockBT.readString();
              delAuthData += '\r';
              if(delAuthData.length() != 0)
              {
                for(int i = 0; i < authCount; i++)
                {
                  if(stp == delAuthData)
                  {
                    stopped = true;
                    break;
                  }
                  else if(delAuthData == authVect[i])
                  {
                    adminAccessList.seek(SeekSet);
                    uint8_t delSize = authVect[i].length() + 1;
                    for(int x= 0; x < authCount; x++)
                    {
                      String testDel = adminAccessList.readStringUntil('\n');
                      if(authVect[i] = testDel)
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
          else if(stp = btData)
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
          if(i == authCount)
          {
            Serial.println("Access denied, no such key\n");
            break;
          }
          if(qrData == authVect[i])
          {
            Serial.println("QR key accepted, opening lock\n");
            openLock();
            delay(10000);
            Serial.println("Closing lock");
            closeLock();
            break;
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
  authCount = 0;
  File authCountF = SPIFFS.open("/accesslist.txt");
  vector<String> authCountV;
  while(authCountF.available())
  {
    authCountV.push_back(authCountF.readStringUntil('\n'));
    authCount++;
    Serial.println(authCount);
  }

  QRScan();
  // String tk1 = "testkey1\r";
  
  // for(String s : authVect)
  // {
  //   Serial.println(s);
  //   Serial.println(tk1);
  //   if(s == tk1)
  //   {
  //     Serial.println("identical!?");
  //   }
  //   else
  //   {
  //     Serial.println("sadly not :pensive:");
  //   }
  // }
  authCountF.close();
  delay(2500);
}