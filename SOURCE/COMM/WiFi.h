/* 
 * File:   WiFi.h
 * Author: user
 *
 * Created on 2017年5月3日, 上午 9:33
 */
#define _WiFi_Info_MaxNum_			50
#define _Socket_Connections_			1
#define _ECR_WIFI_BUFF_SIZE_			_ECR_BUFF_SIZE_
#define _Pepper_Demo_Data_Size_			12
#define _Pepper_Nestle_Sale_Data_Size_		8 + 220 + 15 + 5	/* 額外加入自定義的MID byte */

#define _ECR_WIFI_RECEIVE_TIMER_		_ECR_RECEIVE_TIMER_
#define _ECR_WIFI_RECEIVE_REQUEST_TIMEOUT_	_ECR_RECEIVE_REQUEST_TIMEOUT_
#define _ECR_WIFI_RETRYTIMES_			_ECR_RETRYTIMES_

#define _ECR_WIFI_01_TRANSTYPE_SALE_		"01"

#define _ECR_WIFI_01_TRANSTYPE_SALE_NO_		1

#define _ECR_WIFI_01_HOSTID_DINERS_		"00"
#define _ECR_WIFI_01_HOSTID_AMEX_		"01"
#define _ECR_WIFI_01_HOSTID_TSB_		"02"

/* ECR_NCCC_CARDTYPE */
#define	_ECR_WIFI_01_CARDTYPE_VISA_		"01"	/* VISA */
#define	_ECR_WIFI_01_CARDTYPE_MASTERCARD_	"02"	/* MASTERCARD */
#define	_ECR_WIFI_01_CARDTYPE_JCB_		"03"	/* JCB */
#define	_ECR_WIFI_01_CARDTYPE_UCARD_		"04"	/* U CARD*/
#define	_ECR_WIFI_01_CARDTYPE_DINERS_		"05"	/* DINERS */
#define	_ECR_WIFI_01_CARDTYPE_AMEX_		"06"	/* AMEX */
#define	_ECR_WIFI_01_CARDTYPE_SMARTPAY_		"07"	/* SMART PAY */
#define	_ECR_WIFI_01_CARDTYPE_CUP_		"08"	/* CUP */

/* 定義ECR版本，避免亂掉 */
#define _ECR_WIFI_VERSION_00_Pepper_Demo_	0
#define _ECR_WIFI_VERSION_01_Pepper_Nestle_	1

#define _ECR_WIFI_DEMO_DATA_			"S201023502                           000000013500                                                                                                                                                                                        6601000081    E"

int inWiFi_Open(void);
int inWiFi_Close(void);
int inWiFi_Connect_Flow(char *szHostIP, char *szPort);
int inWiFi_DisConnect_Flow(void);
int inWiFi_Initial(void);
int inWiFi_Begin(TRANSACTION_OBJECT *pobtran);
int inWiFi_SetConfig(void);
int inWiFi_END(void);
int inWiFi_Flush(void);
int inWiFi_DeInitial(void);

int inWiFi_Switch(void);
int inWiFi_Status(void);
int inWiFi_GetStatus(unsigned int *uiStatus);
int inWiFi_IsConected(void);
int inWiFi_IsScanning(void);
int inWiFi_Scan(void);
int inWiFi_ScanProcess(void);
int inWiFi_WifiDisconnectAP(void);
int inWiFi_ConfigGet(unsigned char uszTag, unsigned char* uszValue, unsigned char *uszLen);
int inWiFi_ConfigSet(unsigned char uszTag, unsigned char* uszValue, unsigned char uszLen);
int inWiFi_SetConfig_Flow(void);
int inWiFi_AutoSetCofig(void);
int inWiFi_GetConfig_Flow(void);
int inWiFi_Server_Open(void);
int inWiFi_Create_Accept_Thread(void);
void vdWiFi_Accept_Thread(void);
int inWiFi_IsAccept(void);
int inWiFi_Close_Client(void);
int inWiFi_ConnectedAP_InfoGet(unsigned char uszTag, unsigned char* uszValue, unsigned char *uszLen);
int inWiFi_ConnectedAP_InfoGet_Flow(void);
int inWiFi_Get_Quality(unsigned char* uszQuality);
int inWiFi_Test_Menu(void);

/* ECR Function Start */
int inWiFi_ECR_Initial(void);
int inWiFi_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran);
int inWiFi_ECR_Send_Result(TRANSACTION_OBJECT *pobTran);
int inWiFi_ECR_Send_Error(TRANSACTION_OBJECT *pobTran, int inErrorType);
int inWiFi_ECR_Send_Mirror(TRANSACTION_OBJECT *pobTran);
int inWiFi_ECR_Receive_EI(TRANSACTION_OBJECT *pobTran);
int inWiFi_ECR_Send_EI(TRANSACTION_OBJECT *pobTran, int inErrorType);
int inWiFi_ECR_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize);
int inWiFi_ECR_Receive_01_TSB(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize);

/* ECR Function End */

/* Test */
int inWiFi_TLS_Test(void);
