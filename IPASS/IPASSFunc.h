/* 
 * File:   IPASSFunc.h
 * Author: user
 *
 * Created on 2017年12月22日, 上午 9:57
 */

/* 參考IPassMicroPayment_ErrCode */

#define _IPASS_DAVTI_SIZE_		275
#define _IPASS_CARD_DATA_SIZE_		91

#define _IPASS_SIGN_SIZE_56_BYTES_	56
#define _IPASS_SIGN_SIZE_64_BYTES_	64	/* 鎖卡時 */
#define _IPASS_SIGN_SIZE_112_BYTES_	112
#define _IPASS_SIGN_SIZE_136_BYTES_	136

#define _TICKET_ISO_MODE_IPASS_LOGON_		1
#define _TICKET_ISO_MODE_IPASS_REGISTER_	2

#define _TICKET_MSG_IPASS_PUT_CARD_IN_AREA_	1
#define _TICKET_MSG_IPASS_AUTO_TOP_UP_		2
#define _TICKET_MSG_IPASS_DO_NOT_MOVE_CARD_	3

#define _IPASS_COMAND_FIRST_	1
#define _IPASS_COMAND_SECOND_	2

int inIPASS_First_Tap(TRANSACTION_OBJECT *pobTran);
int inIPASS_Second_Tap(TRANSACTION_OBJECT *pobTran);
int inIPASS_Init_Command_Packet(unsigned char *uszSendBuffer);
int inIPASS_Init_Register_Packet(unsigned char *uszSendBuffer);
int inIPASS_Command_Packet(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuffer);
int inIPASS_Display_Error(TRANSACTION_OBJECT *pobTran, char *szError);
int inIPASS_Init_Flow(TRANSACTION_OBJECT *pobTran, unsigned char uszReInitBit);
void vdIPASS_Callback_FuncDebug(unsigned int data_len, unsigned short cmd, unsigned char sn, unsigned short sc, unsigned char *pdata);
int inIPASS_Logon_FuncBuildAndSendPacket(TRANSACTION_OBJECT *pobTran, int inMode);
int inIPASS_Fast_Tap(char* szUID, int inUIDLen);
int inIPASS_Query_Flow(TRANSACTION_OBJECT *pobTran);
int inIPASS_Top_Up_Amount_Check(TRANSACTION_OBJECT *pobTran);
int inIPASS_System_Log_Printf(void);
int inIPASS_Cus_096_Do_Cmd(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb);
USHORT IPASS_PollCard(void);
int iniPASS_PackResult_Cus096_Internal(char *szData, ECR_TABLE* srECROb);
int iniPASS_Init_Flow_Cus096_Internal(unsigned char *uszInData, int inInLen, unsigned char *uszOutData,unsigned int* uiOutLen, int inType);

void inIPASS_Init_Test(void);
