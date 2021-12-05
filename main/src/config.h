#define SERVER "ftp.vvtsmart.com"
#define USER_NAME "taivv@vvtsmart.com"
#define PASS_WORLD "votai1999"
#define PORT 21
#define TYPE_SERVER 0       //  0 – FTP server. 1 – Explicit FTPS server with AUTH SSL. 2 – Explicit FTPS server with AUTH TLS. 3 – Implicit FTPS server
#define TYPE_TRANSFER "I"   // "A": ASCII, "I": Binary
#define ID 5                // Id node
#define DELAY_UPDATE 20000  //20s

extern byte year, month, date, hour, minute, second;
static String host = "https://vvtsmart.com/4GRadio/PHP/updateData.php?";