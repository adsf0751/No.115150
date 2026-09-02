#define _COMM_MODEM_MODE_               "0"	/* 撥接模式 */
#define _COMM_ETHERNET_MODE_            "1"	/* TCP/IP 模式 */
#define _COMM_GPRS_MODE_                "5"	/* GPRS 模式 */
#define _COMM_3G_MODE_			"6"	/* 3G 模式 */
#define _COMM_4G_MODE_			"7"	/* 4G 模式 */

#define _COMM_WIFI_MODE_		"9"	/* 這不是TMS原有的參數，只是借這個欄位用 */
/* 0=撥接，1=TCP/IP，2=GSM，3=RS485，4=IP_BOX，5=GPRS，6=3G，7=4G */
#define _COMM_RECEIVE_MAX_LENGTH_	1536

#define _PING_WAY_ETHERNET_	1
#define _PING_WAY_GPRS_		2

typedef enum
{
	_COMM_SELECT_MENU_NONE_		= 0x00,
	_COMM_SELECT_MENU_MODEM_,
	_COMM_SELECT_MENU_ETHERNET_UCL_,
	_COMM_SELECT_MENU_ETHERNET_SSL_,
	_COMM_SELECT_MENU_ETHERNET_PING_,
	_COMM_SELECT_MENU_ETHERNET_DHCP_UCL_,
	_COMM_SELECT_MENU_ETHERNET_DHCP_SSL_,
	_COMM_SELECT_MENU_GPRS_UCL_,
	_COMM_SELECT_MENU_GPRS_SSL_,
	_COMM_SELECT_MENU_WIFI_UCL_,
	_COMM_SELECT_MENU_WIFI_SSL_,
	_COMM_SELECT_MENU_WIFI_DHCP_UCL_,
	_COMM_SELECT_MENU_WIFI_DHCP_SSL_,
} _COMM_SELECT_MENU_;

typedef struct comm_struct
{
        int (*inInitialize)(void);
        int (*inBegin)(TRANSACTION_OBJECT *pobtran);
        int (*inCheck)(void);
        int (*inSend)(unsigned char *, int, int);
        int (*inReceive)(unsigned char *, int, int);
        int (*inFlush)(void);
        int (*inEnd)(void);
	int (*inDeinitialize)(void);
} COMM_OBJECT;

#define COMM_OBJECT_SIZE sizeof(COMM_OBJECT)

/* Functions */
int inCOMM_InitCommDevice(void);
int inCOMM_ConnectStart(TRANSACTION_OBJECT *pobTran);
int inCOMM_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout , unsigned char uszDispMsgBit);
int inCOMM_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout, unsigned char uszDispMsgBit);
int inCOMM_End(TRANSACTION_OBJECT *pobTran);
int inCOMM_MODEM_Mode_Change(TRANSACTION_OBJECT *pobTran);
int inCOMM_MODEM_DialBackUpOn(TRANSACTION_OBJECT *pobTran);
int inCOMM_MODEM_DialBackUpOff(TRANSACTION_OBJECT *pobTran);
int inCOMM_CreditPredialDisconnect(TRANSACTION_OBJECT *pobTran);
int inCOMM_DoInitial(void);
int inCOMM_ETHERNET_SetFuncIndex(void);
int inCOMM_MODEM_SetFuncIndex(void);
int inCOMM_WiFi_SetFuncIndex(void);
int inCOMM_GPRS_SetFuncIndex(void);
int inCOMM_Fun3_SetCommWay(void);
int inCOMM_Fun3_SetCommWay_Page1(void);
int inCOMM_Fun3_SetCommWay_Page2(void);
int inCOMM_Fun3_SetCommWay_Page3(void);
int inCOMM_Fun3_SetPhone(void);
int inCOMM_Fun3_Ethernet_SetIPAddress(void);
int inCOMM_Fun3_GPRS_SetIPAddress(void);
int inCOMM_DeInitCommDevice(void);
int inCOMM_TCP_SetConnectTO(unsigned long ulTime);
int inCOMM_TCP_SetRetryCounter(unsigned short usTime);
int inCOMM_Ping_Test_Menu(int inConnectWay);
int inCOMM_Ping_Host_Menu(int inConnectWay);
int inCOMM_Ping_IP_Menu(int inConnectWay);
int inCOMM_Set_UDP(void);

int inCOMM_Test(void);

