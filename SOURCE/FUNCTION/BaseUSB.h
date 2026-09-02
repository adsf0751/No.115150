/* 
 * File:   BaseUSB.h
 * Author: user
 *
 * Created on 2017年11月13日, 下午 5:46
 */
#define _ECR_BaseUSB_RETRYTIMES_		_ECR_RETRYTIMES_
#define _ECR_BaseUSB_RECEIVE_TIMER_		_ECR_RECEIVE_TIMER_
#define _ECR_BaseUSB_RECEIVE_REQUEST_TIMEOUT_	_ECR_RECEIVE_REQUEST_TIMEOUT_
#define _ECR_BaseUSB_GET_CARD_TIMEOUT_		_ECR_GET_CARD_TIMEOUT_
#define _ECR_BaseUSB_RECEIVE_ACK_TIMEOUT_	_ECR_RECEIVE_ACK_TIMEOUT_LONG_
#define _ECR_BaseUSB_BUFF_SIZE_			_ECR_BUFF_SIZE_

int inBaseUSB_Open(void);
int inBaseUSB_Close(void);
int inBaseUSB_FlushTxBuffer(void);
int inBaseUSB_FlushRxBuffer(void);
int inBaseUSB_Data_Send_Check(void);
int inBaseUSB_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize);
int inBaseUSB_Data_Receive_Check(unsigned short *usReceiveLen);
int inBaseUSB_Data_Receive(unsigned char *uszReceBuff, unsigned short *usReceSize);

int inBaseUSB_ECR_Initial(void);
int inBaseUSB_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran);
int inBaseUSB_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inBaseUSB_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType);

/* ECR介接 START */
int inBaseUSB_ECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize);
int inBaseUSB_ECR_Send_ACKorNAK(ECR_TABLE * srECROb, int inAckorNak);
int inBaseUSB_ECR_Receive_ACKandNAK(ECR_TABLE * srECROb);
int inBaseUSB_ECR_SelectTransType(ECR_TABLE * srECROb);
/* ECR介接 END */

/* 8N1_Standard */
int inBaseUSB_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb);
int inBaseUSB_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inBaseUSB_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inBaseUSB_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inBaseUSB_ECR_8N1_Standard_Close(ECR_TABLE* gsrECRob);
/* 7E1_Standard*/
int inBaseUSB_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb);
int inBaseUSB_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inBaseUSB_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inBaseUSB_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inBaseUSB_ECR_7E1_Standard_Close(ECR_TABLE* gsrECRob);

int inBaseUSB_SetCDCMode(void);
int inBaseUSB_SetSTDMode(void);
int inBaseUSB_SetVidPid(unsigned int uiVidPid);
int inBaseUSB_GetVidPid(unsigned int *uiVidPid);


