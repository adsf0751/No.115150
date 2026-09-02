
#define _INIT_PACKET_           1
#define _READER_PACKET_         2
#define _TERM_PACKET_           3

#define _ICASH_PACKET_MODE_UNLOCK_	1
#define _ICASH_PACKET_MODE_GET_PIN_	2
#define _ICASH_PACKET_MODE_OTHER_	3

#define _TICKET_MSG_ICASH_PUT_CARD_IN_AREA_	1
#define _TICKET_MSG_ICASH_AUTO_TOP_UP_		2
#define _TICKET_MSG_ICASH_DO_NOT_MOVE_CARD_	3

int inICASH_DEMOflow(TRANSACTION_OBJECT *pobTran);
int inICASH_Init_Flow(TRANSACTION_OBJECT *pobTran);
int inICASH_SetupReader(TRANSACTION_OBJECT *pobTran);
int inICASH_SAM_Unlock(TRANSACTION_OBJECT *pobTran);
int inICASH_Display_Error(TRANSACTION_OBJECT *pobTran, char *szError);
int inICASH_Init_Packet(TRANSACTION_OBJECT *pobTran, int inType, char *szData, char *szDateTime);
int inICASH_Logon_FuncBuildAndSendPacket(TRANSACTION_OBJECT *pobTran, int inMode);
int inICASH_Get_PIN(TRANSACTION_OBJECT *pobTran);
int inICASH_Logon(TRANSACTION_OBJECT *pobTran);
int inICASH_Fast_Tap(TRANSACTION_OBJECT *pobTran);
int inICASH_Get_CardAmt(TRANSACTION_OBJECT *pobTran);
int inICASH_AutoAdd_Flow(TRANSACTION_OBJECT *pobTran);
int inICASH_Top_Up_Amount_Check(TRANSACTION_OBJECT *pobTran);
int inICASH_First_Tap(TRANSACTION_OBJECT *pobTran);
int inICASH_Second_Tap(TRANSACTION_OBJECT *pobTran);
int inICASH_Command_Packet(TRANSACTION_OBJECT *pobTran, unsigned char *szSendBuffer);
int inICASH_Command_Packet2(TRANSACTION_OBJECT *pobTran, int inLen, unsigned char *szInputBuffer, unsigned char *szSendBuffer);
int inICASH_Command_Packet3(TRANSACTION_OBJECT *pobTran, int inLen, unsigned char *szInputBuffer, unsigned char *szSendBuffer);
int inICASH_Func_BuildAndSendPacket_Comfirm(TRANSACTION_OBJECT *pobTran);
int inICASH_Command_Packet_Comfirm(TRANSACTION_OBJECT *pobTran);
