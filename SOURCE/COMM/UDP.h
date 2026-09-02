/* 
 * File:   UDP.h
 * Author: RussellBai
 *
 * Created on 2021年12月1日, 下午 4:55
 */
#define _ECR_UDP_BUFF_SIZE_			_ECR_BUFF_SIZE_

int inUDP_Server_Open(void);
int inUDP_Server_Close(void);
int inUDP_FlushRxBuffer(void);
int inUDP_FlushTxBuffer(void);
int inUDP_Data_Send_Check(void);
int inUDP_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize);
int inUDP_Data_Receive_Check(unsigned short *usReceiveLen);
int inUDP_Data_Receive(unsigned char* uszData, unsigned short *usLen);
int inUDP_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran);
int inUDP_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inUDP_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inUDP_ECR_SendMirror(TRANSACTION_OBJECT * pobTran);
int inUDP_ECR_ReceiveEI(TRANSACTION_OBJECT *pobTran);
int inUDP_ECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inUDP_ECR_DeInitial(void);
int inUDP_ECR_Initial(void);
int inUDP_ECR_NCCC_144_To_400(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer);
int inUDP_CheckFromPOSIP(void);
int inUDP_Check_Not_Same_PosTxUniqueNO(void);
int inUDP_ECR_400_Standard_Initial(ECR_TABLE *srECROb);
int inUDP_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUDP_ECR_400_Standard_Close(ECR_TABLE* srECRob);
int inUDP_ECR_8N1_Standard_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
/* 7E1_Standard*/
int inUDP_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUDP_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 8N1_Customer_107_Bumper */
int inUDP_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inUDP_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 8N1_Customer_111_Kiosk_Standard_ */
int inUDP_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
/* 8N1_Customer_039_CARD_NO_HIDE_F8_B4_ */
int inUDP_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 8N1_Customer_098_Standard_ */
int inUDP_ECR_8N1_Customer_098_Mcdonalds_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_8N1_Customer_098_Mcdonalds_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_8N1_Customer_098_Mcdonalds_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_8N1_Customer_098_Mcdonalds_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 7E1_Customer_022_DEXIAN_ */
int inUDP_ECR_7E1_Customer_022_DEXIAN_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_7E1_Customer_022_DEXIAN_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 7E1_Customer_002_NICE_PLAZA_ */
int inUDP_ECR_7E1_Customer_002_NICE_PLAZA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_7E1_Customer_002_NICE_PLAZA_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inUDP_ECR_7E1_Customer_002_NICE_PLAZA_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 8N1_Customer_033_UNIAIR */
int inUDP_ECR_8N1_Customer_033_UNIAIR_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
/* 8N1_Customer_090_TAIPEI_101 */
int inUDP_ECR_8N1_Customer_090_TAIPEI_101_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);