
#define _NCCC_ATS_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _NCCC_ATS_NULL_TX_                  0
#define _NCCC_ATS_MAX_BIT_MAP_CNT_          40
#define _NCCC_ATS_MTI_SIZE_                 2
#define _NCCC_ATS_PCODE_SIZE_               3
#define _NCCC_ATS_TPDU_SIZE_                5
#define _NCCC_ATS_BIT_MAP_SIZE_             8
#define _NCCC_ATS_RRN_SIZE_                 12 /* RRN */
#define _NCCC_ATS_ISO_SEND_                 1536
#define _NCCC_ATS_ISO_RECV_                 1536
#define _NCCC_ATS_ISO_ASC_                  1 /* a */
#define _NCCC_ATS_ISO_BCD_                  2
#define _NCCC_ATS_ISO_NIBBLE_2_             3 /* ..nibble */
#define _NCCC_ATS_ISO_NIBBLE_3_             4 /* ...nibble */
#define _NCCC_ATS_ISO_BYTE_2_               5 /* ..ans */
#define _NCCC_ATS_ISO_BYTE_3_               6 /* ...ans */
#define _NCCC_ATS_ISO_BYTE_2_H_             7 /* ..ans */
#define _NCCC_ATS_ISO_BYTE_3_H_             8 /* ...ans */
#define _NCCC_ATS_ISO_ASC_1_                9 /* a */
#define _NCCC_ATS_ISO_BYTE_1_               10/* ..ans */

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_NCCC_ATS_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_NCCC_ATS_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_NCCC_ATS_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_NCCC_ATS_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_NCCC_ATS_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_NCCC_ATS_TABLE;

typedef struct
{
        ISO_FIELD_NCCC_ATS_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_NCCC_ATS_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_NCCC_ATS_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_NCCC_ATS_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_NCCC_ATS_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_NCCC_ATS_TABLE;


int inNCCC_ATS_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);

/* Pack */
int inNCCC_ATS_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack15(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack44(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack45(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack52(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack59_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ATS_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

/* Check */
int inNCCC_ATS_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_ATS_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_ATS_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

/* UnPack */
int inNCCC_ATS_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ATS_UnPack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);


int inNCCC_ATS_BitMapSet(int *inBitMap, int inFeild);
int inNCCC_ATS_BitMapReset(int *inBitMap, int inFeild);
int inNCCC_ATS_BitMapCheck(unsigned char *inBitMap, int inFeild);
int inNCCC_ATS_CopyBitMap(int *inBitMap, int *inSearchBitMap);
int inNCCC_ATS_GetBitMapTableIndex(ISO_TYPE_NCCC_ATS_TABLE *srISOFunc, int inBitMapCode);
int inNCCC_ATS_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NCCC_ATS_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap, unsigned char *uszSendBuf);
int inNCCC_ATS_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inNCCC_ATS_CheckUnPackField(int inField, ISO_FIELD_NCCC_ATS_TABLE *srCheckUnPackField);
int inNCCC_ATS_GetCheckField(int inField, ISO_CHECK_NCCC_ATS_TABLE *ISOFieldCheck);
int inNCCC_ATS_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_ATS_TABLE *srFieldType);
int inNCCC_ATS_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_ATS_TABLE *srFieldType);
int inNCCC_ATS_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);


int inNCCC_ATS_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, unsigned char *uszPackData, char *szMTI);
int inNCCC_ATS_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inISOTxnCode, int *inBitMap);
int inNCCC_ATS_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen);
int inNCCC_ATS_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inNCCC_ATS_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload);

int inNCCC_ATS_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inNCCC_ATS_Advice_ESC_SendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inNCCC_ATS_ProcessAdvice(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ProcessAdvice_ESC(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ProcessOnline(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_AnalysePacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ReversalSave(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
int inNCCC_ATS_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_OnlineAnalyseEMV(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_OnlineEMV_Complete(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_ProcessReferral(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_GetReversalCnt(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode);
int inNCCC_ATS_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inNCCC_ATS_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);

int inNCCC_ATS_CUP_LogOn(TRANSACTION_OBJECT* pobTran);
int inNCCC_ATS_TMKKeyExchange(TRANSACTION_OBJECT *pobTran,unsigned char *uszUnPackBuf);
void vdNCCC_ATS_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCC_ATS_ISO_FormatDebug_DISP_EMV(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_ATS_ISO_FormatDebug_DISP_58(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_ATS_ISO_FormatDebug_DISP_59(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_ATS_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCC_ATS_ISO_FormatDebug_PRINT_EMV(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
void vdNCCC_ATS_ISO_FormatDebug_PRINT_58(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
void vdNCCC_ATS_ISO_FormatDebug_PRINT_59(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
void vdNCCC_ATS_ISO_FormatDebug_POS_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCC_ATS_ISO_FormatDebug_POS_PRINT_EMV(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_ATS_ISO_FormatDebug_POS_PRINT_58(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_ATS_ISO_FormatDebug_POS_PRINT_59(unsigned char *uszDebugBuf, int inFieldLen);

int inNCCC_ATS_OnlineAnalyseFISC(TRANSACTION_OBJECT* pobTran);
int inNCCC_ATS_FuncInquiryIssueBank(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_OnlineAnalyseTrust(TRANSACTION_OBJECT *pobTran);
int inNCCC_ATS_DispHostResponseCode_Trust(TRANSACTION_OBJECT *pobTran);
