#include <Arduino.h>
#include <string.h>
#include <esp_camera.h>
#include <ESP32Servo.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESP32QRCodeReader.h>
#include <BluetoothSerial.h>
#include <ESP32Time.h>

using namespace std;

#define LOCK_PIN 12

ESP32QRCodeReader scanner(CAMERA_MODEL_AI_THINKER);
BluetoothSerial lockBT;
Servo lockServo;
ESP32Time espClock;

int authCount;
int testCount = 1;

struct QRCodeData qrCode;

String add = "A\r";
String del = "D\r";
String edt = "E\n";
String stp = "S\r";
String setT = "set time\r";
String readL = "read log\r";

void file_setup()
{
  File authList = SPIFFS.open("/accesslist.txt", FILE_WRITE);
  if(!authList)
  {
    Serial.println("error opening file");
    return;
  }
  authList.println("true");
  authList.println("masterkey");
  authList.println("devicename");
  authList.println("deviceplacement");
  authList.println("wifi name");
  authList.println("wifipassword");
  authList.close();

  File logList = SPIFFS.open("/auditlog.txt", FILE_WRITE);
  if(!logList)
  {
    Serial.println("error opening file");
  }
  logList.println("true");
  logList.close();
}

void list_files()
{
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  Serial.println("showcasing files.");

  while(file)
  {
    Serial.print("FILE: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }

  file.close();
  root.close();
}

void setup()
{
  Serial.begin(115200);

  SPIFFS.begin(true);

  File authList = SPIFFS.open("/accesslist.txt");
  if(!authList)
  {
    Serial.println("error opening file");
  }
  String setupCheckImp = authList.readStringUntil('\n');
  authList.close();

  String setupCheck = "true\r";
  if(setupCheck != setupCheckImp)
  {
    Serial.println("File either corrupted or not found, setting up an empty one");
    SPIFFS.remove("/accesslist.txt");
    file_setup();
  }
  
  scanner.setup();
  scanner.begin();

  lockServo.attach(LOCK_PIN);
}

void closeLock()
{
  lockServo.write(0);
  delay(2000);
}

void openLock()
{
  lockServo.write(180);
  delay(200);
}

void QRScan()
{
  File authList = SPIFFS.open("/accesslist.txt");
  if(!authList)
  {
    Serial.println("error opening file");
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
      String qrData = (const char*)qrCode.payload;
      qrData += '\r';
      if(qrData == authVect[1])
      {
        lockBT.begin("SmartLock");

        while(true)
        {
          String btData = lockBT.readString();
          btData += '\r';
          if(setT == btData)
          {
            int btSec = lockBT.read();
            int btMin = lockBT.read();
            int btHour = lockBT.read();
            int btDay = lockBT.read();
            int btMonth = lockBT.read();
            int btYear = lockBT.read();

            espClock.setTime(btSec, btMin, btHour, btDay, btMonth, btYear);
          }
          else if(readL == btData)
          {
            File auditLog = SPIFFS.open("/auditlog.txt");
            vector<String> logVect;
            while(auditLog.available())
            {
              logVect.push_back(auditLog.readStringUntil('\n'));
            }
            auditLog.close();

            for(int i = 1; i < logVect.size(); i++)
            {
              lockBT.println(logVect[i]);
            }
          }
          else if(add == btData)
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
              else if(newAuthData.length() > 1)
              {
                File adminAccessList = SPIFFS.open("/accesslist.txt", FILE_WRITE);
                if(!adminAccessList)
                {
                  Serial.println("error opening file");
                  return;
                }
                adminAccessList.seek(SeekEnd);
                adminAccessList.println(newAuthData);
                authCount++;
                adminAccessList.close();
                
                File auditLog = SPIFFS.open("/auditlog.txt", FILE_WRITE);
                if(!auditLog)
                {
                  Serial.println("error opening file");
                  return;
                }
                auditLog.seek(SeekEnd);
                auditLog.println("added code at " + espClock.getDateTime());
                auditLog.close();
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
                for(int i = 1; i < authCount; i++)
                {
                  if(stp == delAuthData)
                  {
                    stopped = true;
                    break;
                  }
                  else if(delAuthData == authVect[i])
                  { 
                    authVect.erase(authVect.begin() + i);
                    lockBT.println("Successfuly deleted from access list");
                    SPIFFS.remove("/accesslist.txt");
                    authCount--;
                    File newAuthList = SPIFFS.open("/accesslist.txt", FILE_WRITE);
                    if(!newAuthList)
                    {
                      Serial.println("error creating file");
                    }
                    for(int y = 0; y < authCount; y++)
                    {
                      newAuthList.println(authVect[y]);
                    }
                    newAuthList.close();
                    File auditLog = SPIFFS.open("/auditlog.txt", FILE_WRITE);
                    if(!auditLog)
                    {
                      Serial.println("error opening file");
                      return;
                    }
                    auditLog.seek(SeekEnd);
                    auditLog.println("deleted code at " + espClock.getDateTime());
                    auditLog.close();
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
          else if(edt == btData)
          {
            while(true)
            {
              bool stopped = false;
              bool found = true;

              String edtAuthData = lockBT.readString();
              edtAuthData += '\r';
              if(edtAuthData.length() > 1)
              {
                for(int i = 1; i < authCount; i++)
                {
                  if(stp == edtAuthData)
                  {
                    stopped = true;
                    break;
                  }
                  else if(edtAuthData == authVect[i])
                  {
                    while(true)
                    {
                      String newEdtData = lockBT.readString();
                      newEdtData += '\r';
                      if(newEdtData.length() > 1)
                      {
                        authVect[i] = newEdtData;
                        lockBT.println("Successfuly edited key");
                        SPIFFS.remove("/accesslist.txt");
                        File newAuthList = SPIFFS.open("/accesslist.txt", FILE_WRITE);
                        if(!newAuthList)
                        {
                          Serial.println("error creating file");
                        }
                        for(int y = 0; y < authCount; y++)
                        {
                          newAuthList.println(authVect[y]);
                        }
                        newAuthList.close();
                        File auditLog = SPIFFS.open("/auditlog.txt", FILE_WRITE);
                        if(!auditLog)
                        {
                          Serial.println("error opening file");
                          return;
                        }
                        auditLog.seek(SeekEnd);
                        auditLog.println("edited code at " + espClock.getDateTime());
                        auditLog.close();
                        break;
                      }
                    }
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
                  lockBT.println("Exited edit mode");
                  break;
                }
              }
            }
          }
          else if(stp == btData)
          {
            lockBT.println("Exiting admin mode\n");
            lockBT.end();
            return;
          }
        }
      }
      else 
      {
        int i = 1;
        while(true)
        {
          if(i == authCount)
          {
            delay(5000);
            break;
          }
          if(qrData == authVect[i])
          {
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
  }
  Serial.println(authCount);

  authCountF.close();

  QRScan();
  
  delay(200);
}