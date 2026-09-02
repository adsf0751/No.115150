
#define _AMEX_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _AMEX_NULL_TX_                  0
#define _AMEX_MAX_BIT_MAP_CNT_          40
#define _AMEX_MTI_SIZE_                 2
#define _AMEX_PCODE_SIZE_               3
#define _AMEX_TPDU_SIZE_                5
#define _AMEX_BIT_MAP_SIZE_             8
#define _AMEX_RRN_SIZE_                 12 /* RRN */
#define _AMEX_ISO_SEND_                 1536
#define _AMEX_ISO_RECV_                 1536
#define _AMEX_ISO_ASC_                  1 /* a */
#define _AMEX_ISO_BCD_                  2
#define _AMEX_ISO_NIBBLE_2_             3 /* ..nibble */
#define _AMEX_ISO_NIBBLE_3_             4 /* ...nibble */
#define _AMEX_ISO_BYTE_2_               5 /* ..ans */
#define _AMEX_ISO_BYTE_3_               6 /* ...ans */
#define _AMEX_ISO_BYTE_2_H_             7 /* ..ans */
#define _AMEX_ISO_BYTE_3_H_             8 /* ...ans */
#define _AMEX_ISO_ASC_1_                9 /* a */

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_AMEX_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_AMEX_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_AMEX_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_AMEX_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_AMEX_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_AMEX_TABLE;

typedef struct
{
        ISO_FIELD_AMEX_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_AMEX_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_AMEX_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_AMEX_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_AMEX_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inGetBitMapCode)(TRANSACTION_OBJECT *, int);/* 取得組 Bit Map 的交易類別 */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_AMEX_TABLE;


int inAMEX_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran);
int inAMEX_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);


int inAMEX_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack15(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack44(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack45(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inAMEX_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);


int inAMEX_BitMapSet(int *inBitMap, int inFeild);
int inAMEX_BitMapReset(int *inBitMap, int inFeild);
int inAMEX_BitMapCheck(unsigned char *inBitMap, int inFeild);
int inAMEX_CopyBitMap(int *inBitMap, int *inSearchBitMap);
int inAMEX_GetBitMapTableIndex(ISO_TYPE_AMEX_TABLE *srISOFunc, int inBitMapCode);
int inAMEX_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_AMEX_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap, unsigned char *uszSendBuf);
int inAMEX_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inAMEX_CheckUnPackField(int inField, ISO_FIELD_AMEX_TABLE *srCheckUnPackField);
int inAMEX_GetCheckField(int inField, ISO_CHECK_AMEX_TABLE *ISOFieldCheck);
int inAMEX_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_AMEX_TABLE *srFieldType);
int inAMEX_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);


int inAMEX_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inAMEX_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inAMEX_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inAMEX_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inAMEX_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);


int inAMEX_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType);
int inAMEX_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI);
int inAMEX_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap);
int inAMEX_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen);
int inAMEX_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inAMEX_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inAMEX_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload);

int inAMEX_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inAMEX_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inAMEX_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inAMEX_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inAMEX_ProcessAdvice(TRANSACTION_OBJECT *pobTran);
int inAMEX_ProcessOnline(TRANSACTION_OBJECT *pobTran);
int inAMEX_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inAMEX_AnalysePacket(TRANSACTION_OBJECT *pobTran);
int inAMEX_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran);
int inAMEX_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran);
int inAMEX_ReversalSave(TRANSACTION_OBJECT *pobTran);
int inAMEX_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran);
int inAMEX_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
int inAMEX_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran);
int inAMEX_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inAMEX_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inAMEX_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran);
int inAMEX_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inAMEX_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
int inAMEX_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inAMEX_ProcessReferral(TRANSACTION_OBJECT *pobTran);
int inAMEX_GetReversalCnt(TRANSACTION_OBJECT *pobTran);
int inAMEX_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode);
int inAMEX_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inAMEX_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);


