// HardwareSerial RX:16   TX:17
#include "src/FTP_7600.h"

#define SERVER "ftp.vvtsmart.com"
#define USER_NAME "taivv@vvtsmart.com"
#define PASS_WORLD "votai1999"
#define PORT 21
#define TYPE_SERVER 0      //  0 – FTP server. 1 – Explicit FTPS server with AUTH SSL. 2 – Explicit FTPS server with AUTH TLS. 3 – Implicit FTPS server
#define TYPE_TRANSFER "A"  // "A": ASCII, "I": Binary
#define FILE_NAME "My Love - Westlife.mp3"

void setup() {
  Serial.begin(115200);
  ftp7600.StartSim();
  ftp7600.SelectDirectory("D:");                                                          // "D:", "C:", ...
  ftp7600.ConfigureFTP(SERVER, PORT, USER_NAME, PASS_WORLD, TYPE_SERVER, TYPE_TRANSFER);  // [ServerHost], [Port], [UserName], [PassWorld], [TypeServer], [TypeTransfer]
  ftp7600.GetDirectory("/");                                                              // list all files in directory
  ftp7600.DownloadFromFTP("octocat.jpg");                                                 // Download file
  ftp7600.PlayMusic("D:/My Love - Westlife.mp3", 0);                                      // Play music from sd card
  // ftp7600.StopMusic();                                                                    // Stop music
}
void loop() {
}