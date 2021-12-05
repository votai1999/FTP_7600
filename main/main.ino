// HardwareSerial RX:16   TX:17
#include <ArduinoJson.h>
#include <DS3231.h>
#include <Wire.h>

#include "Preferences.h"
#include "src/FTP_7600.h"
#include "src/config.h"

Preferences devicePreferences;
RTClib myRTC;
DS3231 Clock;

byte year, month, date, hour, minute, second;
const char *Mode, *NumRepeat, *TimeStart, *TimeStop, *Type, *TimeUpdate, *FileName, *UpdateFileWarn;
int hourStart = 0, minStart = 0, hourStop = 0, minStop = 0;

bool scheduled = false;
char nameFile[100];
String stateNode;
int timeDelay = 1;

void scheduledPlayer(void *parameter) {
  while (true) {
    if (scheduled) {
      timeDelay = 1;
      DateTime now = myRTC.now();
      // Scheduled play
      if ((int)now.hour() == hourStart && (int)now.minute() == minStart) {
        ftp7600.PlayMusic((const char *)nameFile, atoi(NumRepeat) - 1);
        Serial.println("\nScheduled player");
        timeDelay = 60;
      }
      // Scheduled stop
      if ((int)now.hour() == hourStop && (int)now.minute() == minStop) {
        ftp7600.StopMusic();
        Serial.println("\nScheduled stop");
        timeDelay = 60;
      }
      // Daily update time
      if ((int)now.hour() == 0 && (int)now.minute() == 0 && (int)now.second() < 10) {
        ftp7600.GetTime();
        if (month < 13 && date < 32 && hour < 24 && minute < 60 && second < 60)
          setTime();
        timeDelay = 15;
      }
    }
    for (int i = 0; i < timeDelay; i++) {
      vTaskDelay(1000);
    }
  }
  vTaskDelete(NULL);
}

// Update time
void setTime() {
  Serial.println("Set time");
  Clock.setClockMode(false);  //mode 24h
  Clock.setYear((int)year);
  Clock.setMonth((int)month);
  Clock.setDate((int)date);
  Clock.setHour((int)hour);
  Clock.setMinute((int)minute);
  Clock.setSecond((int)second);
}

void setup() {
  Serial.begin(115200);
  ftp7600.StartSim();
  ftp7600.SelectDirectory("D:");  // "D:", "C:", ...
  ftp7600.GetTime();              // Get time from internet
  Serial.printf("Get Time: %d:%d:%d %d/%d/%d\n", year, month, date, hour, minute, second);
  Wire.begin();
  if (month < 13 && date < 32 && hour < 24 && minute < 60 && second < 60)
    setTime();
  DateTime now = myRTC.now();
  Serial.printf("Read Time: %d:%d:%d %d/%d/%d\n", (int)now.year(), (int)now.month(), (int)now.day(), (int)now.hour(), (int)now.minute(), (int)now.second());
  //  Clear NVS
  devicePreferences.begin("DeviceTime", false);
  devicePreferences.putString("timeUpdate", "");
  devicePreferences.end();
  // Task scheduled player
  xTaskCreate(
      scheduledPlayer,
      "scheduledPlayer",
      4096,
      NULL,
      1,
      NULL);
  disableCore0WDT();
}
void loop() {
  // Send request
  if (ftp7600.StatePlayer() != 0)
    stateNode = "playing";
  else
    stateNode = "pending";
  String url = host + "id=" + ID + "&quality=" + ftp7600.getQuality() + "&state=" + stateNode + "&updateFileWarn=0";
  ftp7600.RequestHttp(url);
  delay(1000);  //Time out
  // Get response
  String res = ftp7600.ResponseHttp();
  Serial.println(res);
  // String to Json
  StaticJsonDocument<500> doc;
  DeserializationError err = deserializeJson(doc, res);
  if (err) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(err.c_str());
  } else {
    Mode = doc["mode"];
    NumRepeat = doc["numRepeat"];
    Type = doc["type"];
    TimeUpdate = doc["timeUpdate"];
    FileName = doc["fileName"];
    UpdateFileWarn = doc["updateFileWarn"];
    devicePreferences.begin("DeviceTime", true);
    String time_update = devicePreferences.getString("timeUpdate", "");
    devicePreferences.end();
    // Check new update data
    if (time_update != String(TimeUpdate)) {
      devicePreferences.begin("DeviceTime", false);
      devicePreferences.putString("timeUpdate", TimeUpdate);
      devicePreferences.end();
      // Warning mode
      if (String(Type) == "2") {
        scheduled = false;
        Serial.println("Canh Bao");
        if (atoi(UpdateFileWarn) == 1) {
          ftp7600.ConfigureFTP(SERVER, PORT, USER_NAME, PASS_WORLD, TYPE_SERVER, TYPE_TRANSFER);
          // ftp7600.GetDirectory("/");
          ftp7600.DownloadFromFTP(FileName);
          ftp7600.LogOut();
        }
        sprintf(nameFile, "D:/%s", FileName);
        ftp7600.PlayMusic((const char *)nameFile, atoi(NumRepeat) - 1);
      } else {
        // Normal mode
        Serial.println("Tin Thuong");
        ftp7600.ConfigureFTP(SERVER, PORT, USER_NAME, PASS_WORLD, TYPE_SERVER, TYPE_TRANSFER);
        // ftp7600.GetDirectory("/");
        ftp7600.DownloadFromFTP(FileName);
        ftp7600.LogOut();
        // Scheduled mode
        if (String(Mode) == "TheoLich") {
          TimeStart = doc["timeStart"];
          hourStart = String(TimeStart).substring(0, String(TimeStart).indexOf(":")).toInt();
          minStart = String(TimeStart).substring(String(TimeStart).indexOf(":") + 1).toInt();
          TimeStop = doc["timeStop"];
          hourStop = String(TimeStop).substring(0, String(TimeStop).indexOf(":")).toInt();
          minStop = String(TimeStop).substring(String(TimeStop).indexOf(":") + 1).toInt();
          Serial.printf("TimeStart: %s\n", TimeStart);
          Serial.printf("TimeStop: %s\n", TimeStop);
          sprintf(nameFile, "D:/%s", FileName);
          scheduled = true;
        } else {
          scheduled = false;
          sprintf(nameFile, "D:/%s", FileName);
          ftp7600.PlayMusic((const char *)nameFile, atoi(NumRepeat) - 1);
        }
      }
    }
    // Serial.printf("Update File Warn: %s\n", UpdateFileWarn);
    // Serial.printf("Mode: %s\n", Mode);
    // Serial.printf("Num: %s\n", NumRepeat);
    // Serial.printf("State: %s\n", Type);
    // Serial.printf("TimeUpdate: %s\n", TimeUpdate);
    // Serial.printf("FileName: %s\n", FileName);
  }
  delay(DELAY_UPDATE);
}