/* 
 * File:   RS232.h
 * Author: carolyn
 *
 * Created on 2016年1月6日, 上午 9:34
 */

#define _STX_					0x02
#define _ETX_					0x03
#define _ACK_					0x06
#define _NAK_					0x15
#define _ECR_RS232_RETRYTIMES_			_ECR_RETRYTIMES_
#define _ECR_RS232_RECEIVE_TIMER_		_ECR_RECEIVE_TIMER_
#define _ECR_RS232_RECEIVE_REQUEST_TIMEOUT_	_ECR_RECEIVE_REQUEST_TIMEOUT_
#define _ECR_RS232_GET_CARD_TIMEOUT_		_ECR_GET_CARD_TIMEOUT_
#define _ECR_RS232_RECEIVE_ACK_TIMEOUT_		_ECR_RECEIVE_ACK_TIMEOUT_LONG_
#define _ECR_RS232_BUFF_SIZE_			_ECR_BUFF_SIZE_

/* [20251219_BUG_MDF][UI] 修改ECR啟用時,電票感應時間為20秒 */
#define _ECR_RS232_TICKET_GET_CARD_TIMEOUT_  _ECR_SELECT_TIMEOUT_

/* RS232基本功能 */
int inRS232_Open(unsigned char uszComport, unsigned long ulBaudRate, unsigned char uszParity, unsigned char uszDataBits, unsigned char uszStopBits);
int inRS232_Close(unsigned char uszComport);
int inRS232_FlushRxBuffer(unsigned char uszComPort);
int inRS232_FlushTxBuffer(unsigned char uszComPort);
int inRS232_Data_Send_Check(unsigned char uszComPort);
int inRS232_Data_Send(unsigned char uszComPort, unsigned char *uszReceBuff, unsigned short usReceSize);
int inRS232_Data_Receive_Check(unsigned char uszComPort, unsigned short *usReceiveLen);
int inRS232_Data_Receive(unsigned char uszComPort, unsigned char *uszReceBuff, unsigned short *usReceSize);

/* 這裡用的是global變數 gsrECROb START */
int inRS232_ECR_Initial(void);
int inRS232_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran);
int inRS232_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran);
int inRS232_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inRS232_ECR_SendMirror(TRANSACTION_OBJECT * pobTran);
int inRS232_ECR_ReceiveEI(TRANSACTION_OBJECT *pobTran);
int inRS232_ECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType);
int inRS232_ECR_DeInitial(void);
int inRS232_ECR_Load_TMK_Initial(void);
int inRS232_ECR_Load_TMK_From_Master(TRANSACTION_OBJECT *pobTran);
int inRS232_ECR_Send_TMK(TRANSACTION_OBJECT *pobTran);
int inRS232_ECR_Load_TMK_DeInitial(void);
/* 這裡用的是global變數 gsrECROb END */

/* ECR介接 START */
int inRS232_ECR_NCCC_7E1_To_8N1(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer);
int inRS232_ECR_NCCC_7E1_To_8N1_Cheat(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer);
/* ECR介接 END */

/* EI START */
int inRS232_ECR_8N1_EInvoice_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_EInvoice_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* EI END */

/* 8N1_Standard */
int inRS232_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Standard_Close(ECR_TABLE* gsrECRob);
int inRS232_ECR_8N1_Standard_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
/* 7E1_Standard*/
int inRS232_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_7E1_Standard_Close(ECR_TABLE* gsrECRob);
/* LOAD_TMK_FROM_520_Standard */
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Close(ECR_TABLE* gsrECRob);
/* 8N1_TSB_KIOSK */
int inRS232_ECR_8N1_TSB_KIOSK_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_8N1_TSB_KIOSK_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_8N1_TSB_KIOSK_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_TSB_KIOSK_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_8N1_TSB_KIOSK_Close(ECR_TABLE* gsrECRob);
/* 8N1_Customer_107_Bumper */
int inRS232_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 8N1_Customer_111_Kiosk_Standard_ */
int inRS232_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
/* 8N1_Customer_039_CARD_NO_HIDE_F8_B4_ */
int inRS232_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 8N1_Customer_098_Standard_ */
int inRS232_ECR_8N1_Customer_098_Mcdonalds_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_098_Mcdonalds_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_098_Mcdonalds_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_098_Mcdonalds_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 8N1_Customer_034_TK3C_EINVOICE_ */
int inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 7E1_Customer_002_NICE_PLAZA_ */
int inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 7E1_Customer_003_WELLCOME_ */
int inRS232_ECR_7E1_Customer_003_WELLCOME_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_003_WELLCOME_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_003_WELLCOME_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 7E1_Customer_027_CHUNGHWA_TELECOM_ */
int inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 8N1_Customer_033_UNIAIR_ */
int inRS232_ECR_8N1_Customer_033_UNIAIR_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_033_UNIAIR_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_033_UNIAIR_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 7E1_Customer_053_TAICHUNG_ */
int inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* 7E1_Customer_035_MIRAMAR_CINEMAS */
int inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* Customer_096_097_EDA */
int inRS232_ECR_7E1_CUS_096_097_EDA_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_8N1_CUS_096_097_EDA_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_8N1_CUS_096_097_EDA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_CUS_096_097_EDA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_Cus_096_Ipass_Trasaction_Transaction(TRANSACTION_OBJECT *pobTran);
/* Customer_079_CINEMARK */
int inRS232_ECR_7E1_Customer_079_CINEMARK_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_079_CINEMARK_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_079_CINEMARK_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* Customer_082_IKEA */
int inRS232_ECR_7E1_Customer_082_IKEA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_082_IKEA_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_082_IKEA_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* Customer_038_Bellavita */
int inRS232_ECR_8N1_Customer_038_Bellavita_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
/* Customer_005_FPG */
int inRS232_ECR_8N1_Customer_005_FPG_Initial(ECR_TABLE *srECROb);
int inRS232_ECR_8N1_Customer_005_FPG_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_005_FPG_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_8N1_Customer_005_FPG_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* Customer_026_TAKA */
int inRS232_ECR_7E1_Customer_026_TAKA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_026_TAKA_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_026_TAKA_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* Customer_026_TAKA */
int inRS232_ECR_7E1_Customer_021_TAKAWEL_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_021_TAKAWEL_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);
int inRS232_ECR_7E1_Customer_021_TAKAWEL_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb);
/* Customer_090_TAIPEI_101 */
int inRS232_ECR_8N1_Customer_090_TAIPEI_101_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb);