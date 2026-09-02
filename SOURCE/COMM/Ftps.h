#define _FTP_DEFAULT_USER_			"edcuser"
#define _FTP_DEFAULT_PASSWORD_			"edc@123"
#define _FTP_DEFAULT_PORT_			"990"
#define _SFTP_DEFAULT_PORT_			"9990"
#define _SFTP_PRIVATE_KEY_FILE_NAME_		"client.key"

int inFTPS_Initial(TRANSACTION_OBJECT *pobTran);
int inFTPS_Download(FTPS_REC *srFtpsObj);
int inFTPS_Upload(FTPS_REC *srFtpsObj);
int inFTPS_Deinitial(TRANSACTION_OBJECT *pobTran);
int inFTPS_Test(void);
int inFTPS_Download_SFTP(FTPS_REC *srFtpsObj);

double Int64ToDouble(int64_t  in64);
int inFunc_Disp_ProgressBar(char *szDispBar, char *szDispPer, curl_off_t cuTotal, curl_off_t cuNow);

// 非必要function  libcurl版本為7.24
void vdFunc_Check_libCurl_Version(void);
int inFTPS_Check_Curl_Version(void);
