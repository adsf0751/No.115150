/* 
 * File:   NCCCTicketIso.h
 * Author: user
 *
 * Created on 2017年12月14日, 上午 11:06
 */

#define _NCCC_TICKET_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _NCCC_TICKET_NULL_TX_			0
#define _NCCC_TICKET_MAX_BIT_MAP_CNT_		40
#define _NCCC_TICKET_MTI_SIZE_			2
#define _NCCC_TICKET_PCODE_SIZE_		3
#define _NCCC_TICKET_TPDU_SIZE_			5
#define _NCCC_TICKET_BIT_MAP_SIZE_		8
#define _NCCC_TICKET_RRN_SIZE_			12 /* RRN */
#define _NCCC_TICKET_ISO_SEND_			1536
#define _NCCC_TICKET_ISO_RECV_			1536
#define _NCCC_TICKET_ISO_ASC_			1 /* a */
#define _NCCC_TICKET_ISO_BCD_			2
#define _NCCC_TICKET_ISO_NIBBLE_2_		3 /* ..nibble */
#define _NCCC_TICKET_ISO_NIBBLE_3_		4 /* ...nibble */
#define _NCCC_TICKET_ISO_BYTE_2_		5 /* ..ans */
#define _NCCC_TICKET_ISO_BYTE_3_		6 /* ...ans */
#define _NCCC_TICKET_ISO_BYTE_2_H_		7 /* ..ans */
#define _NCCC_TICKET_ISO_BYTE_3_H_		8 /* ...ans */
#define _NCCC_TICKET_ISO_ASC_1_			9 /* a */
#define _NCCC_TICKET_ISO_BYTE_1_		10/* ..ans */
#define _NCCC_TICKET_ETICKET_CARD_		11/* for  票證UID使用 ..ans */

#define _NCCC_TICKET_COMM_TIMEOUT_		10

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_NCCC_TICKET_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_NCCC_TICKET_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_NCCC_TICKET_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_NCCC_TICKET_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_NCCC_TICKET_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_NCCC_TICKET_TABLE;

typedef struct
{
        ISO_FIELD_NCCC_TICKET_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_NCCC_TICKET_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_NCCC_TICKET_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_NCCC_TICKET_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_NCCC_TICKET_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_NCCC_TICKET_TABLE;


int inNCCC_TICKET_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);

/* Pack */
int inNCCC_TICKET_ATS_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_MFES_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack15(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack44(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack45(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack56_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_ATS_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_MFES_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_TICKET_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

/* Check */
int inNCCC_TICKET_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_TICKET_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_TICKET_Check32(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_TICKET_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

/* UnPack */
int inNCCC_TICKET_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_TICKET_UnPack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_TICKET_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_TICKET_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_TICKET_UnPack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_TICKET_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_TICKET_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);

int inNCCC_TICKET_BitMapSet(int *inBitMap, int inFeild);
int inNCCC_TICKET_BitMapReset(int *inBitMap, int inFeild);
int inNCCC_TICKET_BitMapCheck(unsigned char *inBitMap, int inFeild);
int inNCCC_TICKET_CopyBitMap(int *inBitMap, int *inSearchBitMap);
int inNCCC_TICKET_GetBitMapTableIndex(ISO_TYPE_NCCC_TICKET_TABLE *srISOFunc, int inBitMapCode);
int inNCCC_TICKET_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NCCC_TICKET_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap, unsigned char *uszSendBuf);
int inNCCC_TICKET_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inNCCC_TICKET_CheckUnPackField(int inField, ISO_FIELD_NCCC_TICKET_TABLE *srCheckUnPackField);
int inNCCC_TICKET_GetCheckField(int inField, ISO_CHECK_NCCC_TICKET_TABLE *ISOFieldCheck);
int inNCCC_TICKET_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_TICKET_TABLE *srFieldType, int inTicketType);
int inNCCC_TICKET_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_TICKET_TABLE *srFieldType);
int inNCCC_TICKET_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);


int inNCCC_TICKET_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, unsigned char *uszPackData, char *szMTI);
int inNCCC_TICKET_ATS_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, int *inBitMap);
int inNCCC_TICKET_MFES_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, int *inBitMap);
int inNCCC_TICKET_ATS_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen);
int inNCCC_TICKET_MFES_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen);
int inNCCC_TICKET_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inNCCC_TICKET_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload);

int inNCCC_TICKET_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inNCCC_TICKET_ProcessAdvice(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_ProcessOnline(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_AnalysePacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_ReversalSave(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
int inNCCC_TICKET_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_ProcessReferral(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_GetReversalCnt(TRANSACTION_OBJECT *pobTran);
int inNCCC_TICKET_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode);
int inNCCC_TICKET_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inNCCC_TICKET_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inNCCC_TICKET_Decide_Year(char* szYear, char* szMonth);

void vdNCCC_TICKET_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCC_TICKET_ISO_FormatDebug_DISP_56(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_TICKET_ISO_FormatDebug_DISP_59(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_TICKET_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCC_TICKET_ISO_FormatDebug_PRINT_56(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
void vdNCCC_TICKET_ISO_FormatDebug_PRINT_59(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
