/* 
 * File:   USB.h
 * Author: user
 *
 * Created on 2017年6月26日, 下午 3:39
 */

#define _ECR_USB_RETRYTIMES_			_ECR_RETRYTIMES_
#define _ECR_USB_RECEIVE_TIMER_			_ECR_RECEIVE_TIMER_
#define _ECR_USB_RECEIVE_REQUEST_TIMEOUT_	_ECR_RECEIVE_REQUEST_TIMEOUT_
#define _ECR_USB_GET_CARD_TIMEOUT_		_ECR_GET_CARD_TIMEOUT_
#define _ECR_USB_RECEIVE_ACK_TIMEOUT_		_ECR_RECEIVE_ACK_TIMEOUT_LONG_
#define _ECR_USB_BUFF_SIZE_			_ECR_BUFF_SIZE_

#define _USB_MODE_DEVICE_			d_USB_DEVICE_MODE			
#define _USB_MODE_HOST_				d_USB_HOST_MODE				

int inUSB_Open(void);
int inUSB_Close(void);
int inUSB_FlushTxBuffer(void);
int inUSB_FlushRxBuffer(void);
int inUSB_Data_Send_Check(void);
int inUSB_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize);
int inUSB_Data_Receive_Check(unsigned short *usReceiveLen);
int inUSB_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize);

int inUSB_ECR_Initial(void);
int inUSB_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran);
int inUSB_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inUSB_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inBaseUSB_ECR_SendMirror(TRANSACTION_OBJECT * pobTran);
int inBaseUSB_ECR_ReceiveEI(TRANSACTION_OBJECT * pobTran);
int inBaseUSB_ECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inUSB_ECR_SendMirror(TRANSACTION_OBJECT * pobTran);
int inUSB_ECR_ReceiveEI(TRANSACTION_OBJECT * pobTran);
int inUSB_ECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inUSB_ECR_DeInitial(void);

/* ECR介接 START */
int inUSB_ECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize);
int inUSB_ECR_Send_ACKorNAK(ECR_TABLE * srECROb, int inAckorNak);
int inUSB_ECR_Receive_ACKandNAK(ECR_TABLE * srECROb);
int inUSB_ECR_SelectTransType(ECR_TABLE * srECROb);
int inUSB_ECR_NCCC_144_To_400(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer);
/* ECR介接 END */

/* 8N1_Standard */
int inUSB_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUSB_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Standard_Close(ECR_TABLE* gsrECRob);
int inUSB_ECR_8N1_Standard_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
/* 7E1_Standard*/
int inUSB_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb);
int inUSB_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUSB_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_7E1_Standard_Close(ECR_TABLE* gsrECRob);
/* 8N1_TSB_KIOSK */
int inUSB_ECR_8N1_TSB_KIOSK_Initial(ECR_TABLE *srECROb);
int inUSB_ECR_8N1_TSB_KIOSK_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_8N1_TSB_KIOSK_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUSB_ECR_8N1_TSB_KIOSK_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_8N1_TSB_KIOSK_Close(ECR_TABLE* gsrECRob);
/* 8N1 Customer_107_Bumper*/
int inUSB_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUSB_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 8N1 Customer_111_Kiosk_Standard */
int inUSB_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
/* 8N1_Customer_039_CARD_NO_HIDE_F8_B4_ */
int inUSB_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUSB_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUSB_SetCDCMode(void);
int inUSB_SetSTDMode(void);
int inUSB_SetVidPid(unsigned int uiVidPid);
int inUSB_GetVidPid(unsigned int *uiVidPid);
int inUSB_HostOpen(unsigned short usVendorID, unsigned short usProductID);
int inUSB_HostClose(void);
int inUSB_HostSend(unsigned char *uszSendBuffer, unsigned long ulSendLen, unsigned long ulTransTimeout);
int inUSB_HostReceive(unsigned char *uszReceiveBuffer, unsigned long *ulReceiveLen, unsigned long ulTransTimeout);
int inUSB_SelectMode(int inMode);
int inUSB_GetStatus(unsigned int *uiStatus);
int inUSB_Get_Host_Device_Mode(int *inMode);
