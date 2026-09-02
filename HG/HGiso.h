
#define _HG_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _HG_NULL_TX_                  0
#define _HG_MAX_BIT_MAP_CNT_          40
#define _HG_MTI_SIZE_                 2
#define _HG_PCODE_SIZE_               3
#define _HG_TPDU_SIZE_                5
#define _HG_BIT_MAP_SIZE_             8
#define _HG_RRN_SIZE_                 12 /* RRN */
#define _HG_ISO_SEND_                 1536
#define _HG_ISO_RECV_                 1536
#define _HG_ISO_ASC_                  1 /* a */
#define _HG_ISO_BCD_                  2
#define _HG_ISO_NIBBLE_2_             3 /* ..nibble */
#define _HG_ISO_NIBBLE_3_             4 /* ...nibble */
#define _HG_ISO_BYTE_2_               5 /* ..ans */
#define _HG_ISO_BYTE_3_               6 /* ...ans */
#define _HG_ISO_BYTE_2_H_             7 /* ..ans */
#define _HG_ISO_BYTE_3_H_             8 /* ...ans */
#define _HG_ISO_ASC_1_                9 /* a */

#define _F58_40_ServerBalPoints_		40
#define	_F58_41_LoyaltyTransPoints_             41
#define _F58_51_PaymentTerm_                    51
#define _F58_54_LackPointForRefund_             54
#define _F58_55_PayAmount_                      55

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_HG_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_HG_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_HG_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_HG_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_HG_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_HG_TABLE;

typedef struct
{
        ISO_FIELD_HG_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_HG_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_HG_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_HG_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_HG_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inGetBitMapCode)(TRANSACTION_OBJECT *, int);/* 取得組 Bit Map 的交易類別 */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_HG_TABLE;

int inHG_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inHG_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inHG_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inHG_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

int inHG_BitMapSet(int *inBitMap, int inFeild);
int inHG_BitMapReset(int *inBitMap, int inFeild);
int inHG_BitMapCheck(unsigned char *inBitMap, int inFeild);
int inHG_CopyBitMap(int *inBitMap, int *inSearchBitMap);
int inHG_GetBitMapTableIndex(ISO_TYPE_HG_TABLE *srISOFunc, int inBitMapCode);
int inHG_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_HG_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap, unsigned char *uszSendBuf);
int inHG_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inHG_CheckUnPackField(int inField, ISO_FIELD_HG_TABLE *srCheckUnPackField);
int inHG_GetCheckField(int inField, ISO_CHECK_HG_TABLE *ISOFieldCheck);
int inHG_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_HG_TABLE *srFieldType);
int inHG_GetFieldIndex(int inField, ISO_FIELD_TYPE_HG_TABLE *srFieldType);
int inHG_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);

int inHG_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inHG_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inHG_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inHG_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inHG_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inHG_UnPack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);

int inHG_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType);
int inHG_ISOPackMTI(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *bPackData, char *szMTI);
int inHG_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI);
int inHG_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap);
int inHG_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen);
int inHG_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inHG_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inHG_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload);

int inHG_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inHG_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inHG_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inHG_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inHG_ProcessOnline(TRANSACTION_OBJECT *pobTran);
int inHG_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inHG_AnalysePacket(TRANSACTION_OBJECT *pobTran);
int inHG_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran);
int inHG_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran);
int inHG_ReversalSave(TRANSACTION_OBJECT *pobTran);
int inHG_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran);
int inHG_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
int inHG_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran);
int inHG_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inHG_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inHG_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran);
int inHG_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inHG_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
int inHG_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inHG_ProcessReferral(TRANSACTION_OBJECT *pobTran);
int inHG_GetReversalCnt(TRANSACTION_OBJECT *pobTran);
int inHG_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode);
int inHG_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inHG_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);

int inHG_Func_SetHost_CREDIT(TRANSACTION_OBJECT *pobTran);
int inHG_Func_SetHost_HG(TRANSACTION_OBJECT *pobTran);
int inHG_Func_SetTxnOnlineOffline_CREDIT(TRANSACTION_OBJECT *pobTran);
int inHG_Func_SetTxnOnlineOffline_HG(TRANSACTION_OBJECT *pobTran);
int inHG_Func_BuildAndSendPacket_CREDIT(TRANSACTION_OBJECT *pobTran);		/* 交易使用 */
int inHG_Func_BuildAndSendPacket_HG(TRANSACTION_OBJECT *pobTran);		/* 交易使用 */
int inHG_Func_BuildAndSendPacket_HG_Flow(TRANSACTION_OBJECT *pobTran);
int inHG_SendAdvice_VOID(TRANSACTION_OBJECT *pobTran, long lnAdvInvoice);
int inHG_GetHG_Enable(int inOrgHDTIndex, char *szHostEnable);
int inHG_SwitchToHG_Host(int inOrgHDTIndex);

void vdHG_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdHG_ISO_FormatDebug_DISP_58(unsigned char *uszDebugBuf, int inFieldLen);
void vdHG_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdHG_ISO_FormatDebug_PRINT_58(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
