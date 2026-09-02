#define _ETHERNET_FLOW_CTOS_	-1
#define _ETHERNET_FLOW_IFES_	1
#define _ETHERNET_FLOW_NATIVE_	2


int inETHERNET_Open(void);
int inETHERNET_Close(void);
int inETHERNET_Cofig_Set(unsigned char uszTag, unsigned char *uszValue, unsigned char uszLen);
int inETHERNET_Cofig_Get(unsigned char uszTag, unsigned char *uszValue, unsigned char *uszLen);

int inETHERNET_Connect_Flow(char* szIP, char* szPort);
int inETHERNET_Connect(void);
int inETHERNET_Connect_By_Native(char *szIP, char *szPort);
int inETHERNET_Connect_TLS_Flow(char *szHostIP, char *szPort);

int inETHERNET_DisConnect_Flow(void);
int inETHERNET_DisConnect(void);
int inETHERNET_DisConnect_By_Native(void);
int inETHERNET_DisConnect_TLS_Flow(void);

int inETHERNET_Send_Ready_Flow(void);
int inETHERNET_Send_Ready(void);
int inETHERNET_Send_Ready_By_Native(int inFileHandle);
int inETHERNET_Send_Ready_TLS_CTOS(unsigned int uiSSL_ID);
int inETHERNET_Send_Ready_TLS_OPENSSL(SSL **ssl);

int inETHERNET_Send_Data_Flow(unsigned char* uszData, unsigned short usLen);
int inETHERNET_Send_Data(unsigned char* uszData, unsigned short usLen);
int inETHERNET_Send_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short usLen);
int inETHERNET_Send_Data_TLS_CTOS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short usLen);
int inETHERNET_Send_Data_TLS_OPENSSL(SSL **ssl, unsigned char *uszData, unsigned short usLen);

int inETHERNET_Receive_Ready_Flow(unsigned short *usLen);
int inETHERNET_Receive_Ready(unsigned short *usLen);
int inETHERNET_Receive_Ready_By_Native(int inFileHandle, unsigned short *usLen);
int inETHERNET_Receive_Ready_TLS_CTOS(unsigned int uiSSL_ID, unsigned short *usLen);
int inETHERNET_Receive_Ready_TLS_OPENSSL(SSL **ssl, unsigned short *usLen);

int inETHERNET_Receive_Data_Flow(unsigned char* uszData, unsigned short *usLen);
int inETHERNET_Receive_Data(unsigned char* uszData, unsigned short *usLen);
int inETHERNET_Receive_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short *usLen);
int inETHERNET_Receive_Data_TLS_CTOS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short *usLen);
int inETHERNET_Receive_Data_TLS_OPENSSL(SSL **ssl, unsigned char* uszData, unsigned short *usLen);

int inETHERNET_Flush_Rx(void);

int inETHERNET_Initial(void);
int inETHERNET_Begin(TRANSACTION_OBJECT *pobtran);
int inETHERNET_SetConfig(void);
int inETHERNET_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout);
int inETHERNET_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout);
int inETHERNET_END(void);
int inETHERNET_Flush(void);
int inETHERNET_NetDisconnect(void);
int inETHERNET_DeInitial(void);
int inETHERNET_Get_Status(unsigned int *uiStatus);
int inETHERNET_Watch_Status(void);
int inETHERNET_IsPhysicalOnine(void);
int inETHERNET_Ping_IP(char *szIP);
int inETHERNET_Check_Ethernet_Config_Correct(void);
int inETHERNET_DHCP_Flow(void);

/* NCCCTMS使用 */
int inETHERNET_NCCCTMS_Check(void);
