#include "FTP_7600.h"

#include <HardwareSerial.h>
#define mySerial Serial2
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
  char aux_str[50];
  sprintf(aux_str, "AT+CFTPSGETFILE=\"%s\",2", FileName);
  sendATcommand(aux_str, "+CFTPSGETFILE: 0", 500000);
}

void FTP7600::GetDirectory(const char *Path) {
  char aux_str[50];
  sprintf(aux_str, "AT+CFTPSLIST=\"%s\"", Path);
  sendATcommand(aux_str, "+CFTPSLIST: DATA,1480", 10000);
}

void FTP7600::SelectDirectory(const char *Path) {
  char aux_str[50];
  sprintf(aux_str, "AT+FSCD=%s", Path);
  sendATcommand(aux_str, "OK", 2000);
  sendATcommand("AT+FSLS=0", "OK", 10000);
}

void FTP7600::PlayMusic(const char *FileName, unsigned int Repeat) {
  char aux_str[50];
  delay(1000);
  sprintf(aux_str, "AT+CCMXPLAY=\"%s\",0,%d", FileName, Repeat);
  sendATcommand(aux_str, "OK", 10000);
}

void FTP7600::StopMusic() {
  sendATcommand("AT+CCMXSTOP", "OK", 5000);
}

uint8_t FTP7600::sendATcommand(const char *ATcommand, const char *expected_answer, unsigned int timeout) {
  char response[1000];
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