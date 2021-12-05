#include "FTP_7600.h"

#include <HardwareSerial.h>

#include "config.h"
#define mySerial Serial2
char response[1000];
FTP7600::FTP7600() {
}
FTP7600::~FTP7600() {
}

void FTP7600::StartSim() {
  uint8_t answer = 0;
  mySerial.begin(115200);
  answer = sendATcommand("AT", "OK", 2000);
  if (answer == 1) {
    sendATcommand("AT+CRESET", "OK", 2000);
    answer = 0;
  } else {
    Serial.print("Starting up...\n");
    while (answer == 0) {
      answer = sendATcommand("AT", "OK", 2000);
      Serial.print(".");
      delay(1000);
    }
    Serial.println("Successful start");
  }
  delay(5000);
  while ((sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0)
    delay(500);
  sendATcommand("ATE0", "OK", 2000);
  sendATcommand("AT+CATR=1", "OK", 2000);
  sendATcommand("AT+CNMP=2", "OK", 2000);
  sendATcommand("AT+CSQ", "OK", 2000);
  sendATcommand("AT+CPSI?", "OK", 2000);
  mySerial.print("AT+CGSOCKCONT=1,\"IP\",\"CMNET\"\r");
  delay(500);
}

/**************************FTP download file to Module EFS , uploading EFS file to FTP**************************/
void FTP7600::ConfigureFTP(const char *FTPServer, unsigned int PortServer, const char *FTPUserName,
                           const char *FTPPassWord, unsigned int ServerType, const char *TransferType) {
  char aux_str[100];
  sendATcommand("AT+CFTPSSTART", "+CFTPSSTART: 0", 5000);
  sprintf(aux_str, "AT+CFTPSLOGIN=\"%s\",%u,\"%s\",\"%s\",%u", FTPServer, PortServer, FTPUserName, FTPPassWord, ServerType);
  sendATcommand(aux_str, "+CFTPSLOGIN: 0", 10000);
  sprintf(aux_str, "AT+CFTPSTYPE=%s", TransferType);
  sendATcommand(aux_str, "+CFTPSTYPE: 0", 2000);
}

void FTP7600::DownloadFromFTP(const char *FileName) {
  char aux_str[100];
  sprintf(aux_str, "AT+CFTPSGETFILE=\"%s\",2", FileName);
  sendATcommand(aux_str, "+CFTPSGETFILE: 0", 500000);
}

void FTP7600::GetDirectory(const char *Path) {
  char aux_str[50];
  sprintf(aux_str, "AT+CFTPSLIST=\"%s\"", Path);
  sendATcommand(aux_str, "+CFTPSLIST: DATA,1480", 10000);
}

void FTP7600::LogOut() {
  sendATcommand("AT+CFTPSLOGOUT", "+CFTPSLOGOUT: 0", 5000);
  sendATcommand("AT+CFTPSSTOP", "+CFTPSSTOP: 0", 5000);
}

void FTP7600::SelectDirectory(const char *Path) {
  char aux_str[50];
  sprintf(aux_str, "AT+FSCD=%s", Path);
  sendATcommand(aux_str, "OK", 2000);
  sendATcommand("AT+FSLS=0", "OK", 10000);
}

void FTP7600::PlayMusic(const char *FileName, unsigned int Repeat) {
  char aux_str[100];
  delay(1000);
  sprintf(aux_str, "AT+CCMXPLAY=\"%s\",0,%d", FileName, Repeat);
  sendATcommand(aux_str, "+AUDIOSTATE: audio play stop", 10000);
}

void FTP7600::StopMusic() {
  sendATcommand("AT+CCMXSTOP", "OK", 5000);
}

void FTP7600::RequestHttp(String url) {
  Serial.print("Send request");
  sendATcommand("AT+HTTPINIT", "OK", 5000);
  sendATcommand("AT+NETOPEN", "OK", 5000);
  mySerial.print("AT+HTTPPARA=\"URL\",\"" + url + "\"\r");
}

String FTP7600::ResponseHttp() {
  String res;
  if (sendATcommand("AT+HTTPACTION=0", "+HTTPACTION: 0,200", 5000) == 1) {
    sendATcommand("AT+HTTPREAD=1000", "+HTTPREAD: 1000", 2000);
    res = (String)response;
    res = res.substring(res.indexOf('*') + 1, res.indexOf('#'));
  } else
    res = "";
  sendATcommand("AT+HTTPTERM", "OK", 2000);
  sendATcommand("AT+NETCLOSE", "OK", 2000);
  return res;
}

float FTP7600::getQuality() {
  sendATcommand("AT+CSQ", "OK", 2000);
  String quality = (String)response;
  quality = (String)response;
  quality.remove(0, quality.indexOf(":", 0) + 1);
  quality = quality.substring(1, quality.length() - 2);
  quality.replace(",", ".");
  return (quality.toFloat() > 31) ? 31 : quality.toFloat();
}

void FTP7600::GetTime() {
  sendATcommand("AT+CNTPCID=1", "OK", 2000);
  sendATcommand("AT+CNTP=\"pool.ntp.org\",28", "OK", 2000);
  sendATcommand("AT+CNTP", "+CNTP:0", 2000);
  sendATcommand("AT+CCLK?", "OK", 2000);
  year = (response[10] - 48) * 10 + (response[11] - 48);
  month = (response[13] - 48) * 10 + (response[14] - 48);
  date = (response[16] - 48) * 10 + (response[17] - 48);
  hour = (response[19] - 48) * 10 + (response[20] - 48);
  minute = (response[22] - 48) * 10 + (response[23] - 48);
  second = (response[25] - 48) * 10 + (response[26] - 48);
}

int FTP7600::StatePlayer() {
  sendATcommand("AT+CCMXPLAY?", "OK", 2000);
  return String(response).substring(String(response).indexOf(":") + 1).toInt();
}

uint8_t FTP7600::sendATcommand(const char *ATcommand, const char *expected_answer, unsigned int timeout) {
  uint8_t x = 0, answer = 0;
  unsigned long previous;
  memset(response, '\0', 1000);
  delay(100);
  while (mySerial.available() > 0)
    mySerial.read();
  mySerial.println(ATcommand);
  x = 0;
  previous = millis();
  do {
    if (mySerial.available() != 0) {
      response[x] = mySerial.read();
      Serial.print(response[x]);
      x++;
      if (strstr(response, expected_answer) != NULL)
        answer = 1;
    }
  } while ((answer == 0) && ((millis() - previous) < timeout));
  return answer;
}
FTP7600 ftp7600 = FTP7600();