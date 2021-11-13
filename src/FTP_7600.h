#ifndef FTP_7600_h
#define FTP_7600_h
#include <Arduino.h>
#include <stdlib.h>
class FTP7600 {
 public:
  FTP7600();
  ~FTP7600();

  // FTP download file to Module EFS or uploading EFS file to FTP
  void StartSim();
  void ConfigureFTP(const char *FTPServer, unsigned int PortServer, const char *FTPUserName, const char *FTPPassWord, unsigned int ServerType, const char *TransferType);
  void DownloadFromFTP(const char *FileName);
  void GetDirectory(const char *Path);
  void SelectDirectory(const char *Path);
  void PlayMusic(const char *FileName, unsigned int Repeat);
  void StopMusic();
  uint8_t sendATcommand(const char *ATcommand, const char *expected_answer, unsigned int timeout);
};

extern FTP7600 ftp7600;
#endif