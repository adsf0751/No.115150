
#define _DINERS_CHECK_BIT_MAP_(x, b)      ((x) & (1U<<(b)))

#define _DINERS_NULL_TX_             0
#define _DINERS_MAX_BIT_MAP_CNT_     40
#define _DINERS_MTI_SIZE_            2
#define _DINERS_PCODE_SIZE_          3
#define _DINERS_TPDU_SIZE_           5
#define _DINERS_BIT_MAP_SIZE_        8
#define _DINERS_RRN_SIZE_            12         /* RRN */
#define _DINERS_ISO_SEND_            1536
#define _DINERS_ISO_RECV_                 1536
#define _DINERS_ISO_ASC_             1          /* a */
#define _DINERS_ISO_BCD_             2
#define _DINERS_ISO_NIBBLE_2_        3          /* ..nibble */
#define _DINERS_ISO_NIBBLE_3_        4          /* ...nibble */
#define _DINERS_ISO_BYTE_2_          5          /* ..ans */
#define _DINERS_ISO_BYTE_3_          6          /* ...ans */
#define _DINERS_ISO_BYTE_2_H_        7          /* ..ans */
#define _DINERS_ISO_BYTE_3_H_        8          /* ...ans */
#define _DINERS_ISO_ASC_1_           9          /* a */

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_DINERS_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_DINERS_TABLE;

typedef struct
{
        int inFieldNum;         /* Field Number */
        int inFieldType;        /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen;         /* Field Len */
} ISO_FIELD_TYPE_DINERS_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_DINERS_MTI_SIZE_ * 2 + 1];          /* Message Type */
        char szPCode[_DINERS_PCODE_SIZE_ * 2 + 1];      /* Processing Code */
} BIT_MAP_DINERS_TABLE;

typedef struct
{
        ISO_FIELD_DINERS_TABLE *srPackISO;      /* 組封包的功能結構 */
        ISO_FIELD_DINERS_TABLE *srUnPackISO;    /* 解封包的功能結構 */
        ISO_CHECK_DINERS_TABLE *srCheckISO;     /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_DINERS_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_DINERS_TABLE *srBitMap;         /* Bit Map的陣列 */
        int (*inGetBitMapCode)(TRANSACTION_OBJECT *, int); /* 取得組 Bit Map 的交易類別 */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *); /* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_DINERS_TABLE;

int inDINERS_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran);
int inDINERS_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);

int inDINERS_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inDINERS_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inDINERS_BitMapSet(int *inBitMap, int inFeild);
int inDINERS_BitMapReset(int *inBitMap, int inFeild);
int inDINERS_BitMapCheck(unsigned char *inBitMap, int inFeild);
int inDINERS_CopyBitMap(int *inBitMap, int *inSearchBitMap);
int inDINERS_GetBitMapTableIndex(ISO_TYPE_DINERS_TABLE *srISOFunc, int inBitMapCode);
int inDINERS_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_DINERS_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap, unsigned char *uszSendBuf);
int inDINERS_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inDINERS_CheckUnPackField(int inField, ISO_FIELD_DINERS_TABLE *srCheckUnPackField);
int inDINERS_GetCheckField(int inField, ISO_CHECK_DINERS_TABLE *ISOFieldCheck);
int inDINERS_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_DINERS_TABLE *srFieldType);
int inDINERS_GetFieldIndex(int inField, ISO_FIELD_TYPE_DINERS_TABLE *srFieldType);
int inDINERS_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);

int inDINERS_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inDINERS_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inDINERS_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inDINERS_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inDINERS_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inDINERS_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);

int inDINERS_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType);
int inDINERS_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI);
int inDINERS_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap);
int inDINERS_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen);
int inDINERS_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inDINERS_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inDINERS_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char * uszTcUpload);

int inDINERS_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inDINERS_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inDINERS_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inDINERS_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inDINERS_ProcessAdvice(TRANSACTION_OBJECT *pobTran);
int inDINERS_ProcessOnline(TRANSACTION_OBJECT *pobTran);
int inDINERS_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inDINERS_AnalysePacket(TRANSACTION_OBJECT *pobTran);
int inDINERS_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran);
int inDINERS_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran);
int inDINERS_ReversalSave(TRANSACTION_OBJECT *pobTran);
int inDINERS_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran);
int inDINERS_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
int inDINERS_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran);
int inDINERS_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inDINERS_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inDINERS_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran);
int inDINERS_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inDINERS_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
int inDINERS_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inDINERS_ProcessReferral(TRANSACTION_OBJECT *pobTran);
int inDINERS_GetReversalCnt(TRANSACTION_OBJECT *pobTran);
int inDINERS_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode);
int inDINERS_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inDINERS_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);

void vdDINERS_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdDINERS_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);