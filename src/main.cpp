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
ESP32PWM pwm;
Servo lockServo;
ESP32Time espClock;

int authCount;

struct QRCodeData qrCode;

String add = "add\r";
String del = "delete\r";
String edt = "edit\r";
String list = "list\r";
String stp = "stop\r";
String readL = "log\r";
String setT = "time\r";

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
  authList.close();
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

  ESP32PWM::timerCount[0] = 4;
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
    String qrData = (const char*)qrCode.payload;
    qrData += '\r';
    Serial.println(qrData);
    if(qrCode.valid)
    {
      if(qrData == authVect[1])
      {
        openLock();
        lockBT.begin("SmartLock");
        Serial.println("Started bluetooth");
        while(true)
        {
          String btData = lockBT.readStringUntil('\n');
          if(btData.length() > 1)
          {
            Serial.println(btData);
          }
          if(setT == btData)
          {
            String btSec;
            String btMin;
            String btHour;
            String btDay;
            String btMonth;
            String btYear;
            lockBT.println("Enter seconds");
            while(true)
            {
              btSec = lockBT.readStringUntil('\n');
              if(btSec.length() > 1)
              {
                break;
              }
            }
            lockBT.println("Enter minutes");
            while(true)
            {
              btMin = lockBT.readStringUntil('\n');
              if(btMin.length() > 1)
              {
                break;
              }
            }
            lockBT.println("Enter hours");
            while(true)
            {
              btHour = lockBT.readStringUntil('\n');
              if(btHour.length() > 1)
              {
                break;
              }
            }
            lockBT.println("Enter day");
            while(true)
            {
              btDay = lockBT.readStringUntil('\n');
              if(btDay.length() > 1)
              {
                break;
              }
            }
            lockBT.println("Enter month");
            while(true)
            {
              btMonth = lockBT.readStringUntil('\n');
              if(btMonth.length() > 1)
              {
                break;
              }
            }
            lockBT.println("Enter year");
            while(true)
            {
              btYear = lockBT.readStringUntil('\n');
              if(btYear.length() > 1)
              {
                break;
              }
            }

            espClock.setTime(btSec.toInt(), btMin.toInt(), btHour.toInt(), btDay.toInt(), btMonth.toInt(), btYear.toInt());
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

            for(int i = 0; i < (logVect.size() - 1); i++)
            {
              lockBT.println(logVect[i]);
            }
          }
          else if(add == btData)
          {
            lockBT.println("Entered add mode");
            while(true)
            {
              String newAuthData = lockBT.readStringUntil('\n');
              if(stp == newAuthData)
              {
                lockBT.println("Exited addition mode");
                break;
              }
              else if(newAuthData.length() > 1)
              {
                authVect.push_back(newAuthData);
                lockBT.println("Successfuly added to access list");
                SPIFFS.remove("/accesslist.txt");
                File newAuthList = SPIFFS.open("/accesslist.txt", FILE_WRITE);
                if(!newAuthList)
                {
                  Serial.println("error creating file");
                }
                for(int y = 0; y < (authCount + 1); y++)
                {
                  String newAuthEntry = authVect[y];
                  newAuthEntry.remove(newAuthEntry.length()-1, 1);
                  newAuthList.println(newAuthEntry);
                }
                newAuthList.close();
                
                lockBT.println(newAuthData);

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
            lockBT.println("Entered del mode");
            while(true)
            {
              bool stopped = false;
              bool found = true;

              String delAuthData = lockBT.readStringUntil('\n');
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
                    File newAuthList = SPIFFS.open("/accesslist.txt", FILE_WRITE);
                    if(!newAuthList)
                    {
                      Serial.println("error creating file");
                    }
                    for(int y = 0; y < (authCount - 1); y++)
                    {
                      String newAuthEntry = authVect[y];
                      newAuthEntry.remove(newAuthEntry.length()-1, 1);
                      newAuthList.println(newAuthEntry);
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
            lockBT.println("Entered edit mode");
            while(true)
            {
              bool stopped = false;
              bool found = true;

              String edtAuthData = lockBT.readStringUntil('\n');
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
                      String newEdtData = lockBT.readStringUntil('\n');
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
                          String newAuthEntry = authVect[y];
                          newAuthEntry.remove(newAuthEntry.length()-1, 1);
                          newAuthList.println(newAuthEntry);
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
          else if(list == btData)
          {
            lockBT.println("Listing all access keys:");
            for(int i = 1; i < authCount; i++)
            {
              lockBT.println(authVect[i]);
            }
          }
          else if(stp == btData)
          {
            lockBT.println("Exiting admin mode\n");
            lockBT.disconnect();
            lockBT.end();
            closeLock();
            return;
          }
        }
      }
      else 
      {
        int i = 1;
        while(true)
        {
          if(i == authCount + 1)
          {
            Serial.println("No such entry found");
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
  
  authCountF.close();

  QRScan();
  
  delay(200);
}