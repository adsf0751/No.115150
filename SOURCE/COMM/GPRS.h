/* 
 * File:   GPRS.h
 * Author: user
 *
 * Created on 2017年6月7日, 上午 9:45
 */
#define _GPRS_FLOW_CTOS_	-1
#define _GPRS_FLOW_IFES_	1
#define _GPRS_FLOW_NATIVE_	2

int inGPRS_Open(unsigned char *uszIP, unsigned char *uszAPNName, unsigned char *uszID, unsigned char *uszPW);
int inGPRS_Close(void);
int inGPRS_Close_Asynchronous(void);
int inGPRS_Connect_Flow(char *szHostIP, char *szPort);
int inGPRS_Connect_Ex(unsigned char *uszSocket, unsigned char *uszHostIP, unsigned short usPort);
int inGPRS_Connect_By_Native(char *szHostIP, char *szPort);
int inGPRS_Connect_TLS_Flow(char *szHostIP, char *szPort);

int inGPRS_DisConnect_Flow(void);
int inGPRS_Disconnect(unsigned char uszSocket);
int inGPRS_DisConnect_By_Native(void);
int inGPRS_DisConnect_TLS_Flow(void);

int inGPRS_Send_Ready_Flow(void);
int inGPRS_Send_Ready(void);
int inGPRS_Send_Ready_By_Native(int inFileHandle);
int inGPRS_Send_Ready_TLS_CTOS(unsigned int uiSSL_ID);
int inGPRS_Send_Ready_TLS_OPENSSL(SSL **ssl);

int inGPRS_Send_Data_Flow(unsigned char* uszData, unsigned short usLen);
int inGPRS_Send_Data(unsigned char uszSocket, unsigned char* uszData, unsigned short usLen);
int inGPRS_Send_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short usLen);
int inGPRS_Send_Data_TLS_CTOS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short usLen);
int inGPRS_Send_Data_TLS_OPENSSL(SSL **ssl, unsigned char *uszData, unsigned short usLen);

int inGPRS_Receive_Ready_Flow(unsigned short *usLen);
int inGPRS_Receive_Ready(unsigned short *usLen);
int inGPRS_Receive_Ready_By_Native(int inFileHandle, unsigned short *usLen);
int inGPRS_Receive_Ready_TLS_CTOS(unsigned int uiSSL_ID, unsigned short *usLen);
int inGPRS_Receive_Ready_TLS_OPENSSL(SSL **ssl, unsigned short *usLen);

int inGPRS_Receive_Data_Flow(unsigned char* uszData, unsigned short *usLen);
int inGPRS_Receive_Data(unsigned char uszSocket, unsigned char* uszData, unsigned short *usLen);
int inGPRS_Receive_Data_By_Native(int inFileHandle, unsigned char* uszData, unsigned short *usLen);
int inGPRS_Receive_Data_TLS_CTOS(unsigned int uiSSL_ID, unsigned char* uszData, unsigned short *usLen);
int inGPRS_Receive_Data_TLS_OPENSSL(SSL **ssl, unsigned char* uszData, unsigned short *usLen);

int inGPRS_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout);
int inGPRS_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout);

int inGPRS_Initial(void);
int inGPRS_Begin(TRANSACTION_OBJECT *pobtran);
int inGPRS_SetConfig(void);
int inGPRS_END(void);
int inGPRS_Flush(void);
int inGPRS_DeInitial(void);
int inGPRS_GetStatus(unsigned int *uiStatus);
int inGPRS_CheckStatus(void);
int inGPRS_GetSocketStatus(unsigned int *uiStatus, unsigned char uszSocket);
int inGPRS_CheckSocketStatus(unsigned char uszSocket);
int inGPRS_IsConnected(void);
int inGPRS_Detect_Network_Status(unsigned char* pbStatus);
int inGPRS_Check_Network_Status(void);
int inGPRS_Get_IP(unsigned char* uszIP);
int inGPRS_Ping_IP(char *szIP);

/* NCCCTMS使用 */
int inGPRS_NCCCTMS_Check(void);
int inGPRS_NCCCTMS_Send(unsigned char *uszSendBuff, int inSendSize, int inSendTimeout);
int inGPRS_NCCCTMS_Receive(unsigned char *uszReceiveBuff, int inReceiveSize, int inReceiveTimeout);

