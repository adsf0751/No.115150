#define _NCCCTMS_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _NCCCTMS_NULL_TX_			0
#define _NCCCTMS_SCHEDULE_			1
#define _NCCCTMS_LOGON_                         2
#define _NCCCTMS_RECORD_			3
#define _NCCCTMS_TASK_                          4
#define _NCCCTMS_TRACE_LOG_			5
#define _NCCCTMS_LOGOFF_			6
#define _NCCCTMS_FTP_AUTO_INQUIRY_REPORT_	7       /* 開機自動詢問FTP回報用 */
#define _NCCCTMS_FTP_DOWNLOAD_TMS_REPORT_	8	/* 下載 TMS回報用 */
#define _NCCCTMS_LOGON_FTPIDPW_			9	/* 取得ID、PW */
#define _NCCCTMS_DUTYFREE_			10	/* 昇恆昌上傳 */

#define _NCCCTMS_MAX_BIT_MAP_CNT_          40
#define _NCCCTMS_MTI_SIZE_                 2
#define _NCCCTMS_PCODE_SIZE_               3
#define _NCCCTMS_TPDU_SIZE_                5
#define _NCCCTMS_BIT_MAP_SIZE_             8
#define _NCCCTMS_RRN_SIZE_                 12 /* RRN */
#define _NCCCTMS_ISO_SEND_                 1536
#define _NCCCTMS_ISO_RECV_                 1536
#define _NCCCTMS_ISO_ASC_                  1 /* a */
#define _NCCCTMS_ISO_BCD_                  2
#define _NCCCTMS_ISO_NIBBLE_2_             3 /* ..nibble */
#define _NCCCTMS_ISO_NIBBLE_3_             4 /* ...nibble */
#define _NCCCTMS_ISO_BYTE_2_               5 /* ..ans */
#define _NCCCTMS_ISO_BYTE_3_               6 /* ...ans */
#define _NCCCTMS_ISO_BYTE_2_H_             7 /* ..ans */
#define _NCCCTMS_ISO_BYTE_3_H_             8 /* ...ans */
#define _NCCCTMS_ISO_ASC_1_                9 /* a */

#define _NCCCTMS_CANCELLED_                0
#define _NCCCTMS_AUTHORIZED_               1
#define _NCCCTMS_DECLINED_                -1
#define _NCCCTMS_COMM_ERROR_		  -2
#define _NCCCTMS_PACK_ERR_		  -3
#define _NCCCTMS_UNPACK_ERR_		  -4

typedef struct
{
	int inCode;
	int inListTotalCount;           /* File List總共有幾個檔案要下載 */
	int inListIndex; 		/* 要下載的索引 */
        int inTotalPacketCount;
	int inPackNo;
	int inTransactionResult;
        int inDownloadFlag;             /* 0 : 無需下載  1 : 立即下載  2 : 排程下載 */
        int inDownlaodScope;            /* 1 : AP下載 2 : Full參數下載 3 : Partial參數下載 */
        /* Trace Log使用 */
        int inTraceTotalSizes;
	int inTraceSearchIndex;
	int inTraceReadSizes;
        
        long    lnFileSize;
        unsigned long   ulFile_Handle;

	char szTPDU[10 + 1];
	char szNII[4 + 1];
	char szFileName[24 + 1];
	char szRespCode[2 + 1];
	char szProcessingCode[6 + 1];
        char szTotalPacketCount[4 + 1];              /* 總封包數 */
        char szPacketNo[4 + 1];                      /* 從第幾個封包開始下載 */
        char szErrFilePathName[60 + 1];              /* 記錄下載失敗的檔案路徑 */
        unsigned char uszField61[1536 + 1];          /* 下載的檔案 / Trace Log */
        unsigned char uszRequest;
        unsigned char uszDownlodType;                /* 1:Application Downlaod 2:Parameter Download */
} TMS_OBJECT;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TMS_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_NCCCTMS_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TMS_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_NCCCTMS_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_NCCCTMS_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_NCCCTMS_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_NCCCTMS_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_NCCCTMS_TABLE;

typedef struct
{
        ISO_FIELD_NCCCTMS_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_NCCCTMS_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_NCCCTMS_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_NCCCTMS_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_NCCCTMS_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inGetBitMapCode)(TMS_OBJECT *, int);/* 取得組 Bit Map 的交易類別 */
        int (*inPackMTI)(TMS_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TMS_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TMS_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TMS_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TMS_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TMS_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_NCCCTMS_TABLE;


int inNCCCTMS_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);


int inNCCCTMS_Pack03(TMS_OBJECT *srTMS, unsigned char *uszPackBuf);
int inNCCCTMS_Pack24(TMS_OBJECT *srTMS, unsigned char *uszPackBuf);
int inNCCCTMS_Pack39(TMS_OBJECT *srTMS, unsigned char *uszPackBuf);
int inNCCCTMS_Pack41(TMS_OBJECT *srTMS, unsigned char *uszPackBuf);
int inNCCCTMS_Pack42(TMS_OBJECT *srTMS, unsigned char *uszPackBuf);
int inNCCCTMS_Pack58(TMS_OBJECT *srTMS, unsigned char *uszPackBuf);
int inNCCCTMS_Pack61(TMS_OBJECT *srTMS, unsigned char *uszPackBuf);

int inNCCCTMS_Check03(TMS_OBJECT *srTMS, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

int inNCCCTMS_UnPack39(TMS_OBJECT *srTMS, unsigned char *uszUnPackBuf);
int inNCCCTMS_UnPack58(TMS_OBJECT *srTMS, unsigned char *uszUnPackBuf);
int inNCCCTMS_UnPack61(TMS_OBJECT *srTMS, unsigned char *uszUnPackBuf);

int inNCCCTMS_BitMapSet(int *inBitMap, int inFeild);
int inNCCCTMS_BitMapReset(int *inBitMap, int inFeild);
int inNCCCTMS_BitMapCheck(unsigned char *inBitMap, int inFeild);
int inNCCCTMS_CopyBitMap(int *inBitMap, int *inSearchBitMap);
int inNCCCTMS_GetBitMapTableIndex(ISO_TYPE_NCCCTMS_TABLE *srISOFunc, int inBitMapCode);
int inNCCCTMS_GetBitMapMessagegTypeField03(TMS_OBJECT *srTMS, ISO_TYPE_NCCCTMS_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap, unsigned char *uszSendBuf);
int inNCCCTMS_PackISO(TMS_OBJECT *srTMS, unsigned char *uszSendBuf, int inTxnCode);
int inNCCCTMS_CheckUnPackField(int inField, ISO_FIELD_NCCCTMS_TABLE *srCheckUnPackField);
int inNCCCTMS_GetCheckField(int inField, ISO_CHECK_NCCCTMS_TABLE *ISOFieldCheck);
int inNCCCTMS_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCCTMS_TABLE *srFieldType);
int inNCCCTMS_UnPackISO(TMS_OBJECT *srTMS, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);



int inNCCCTMS_ISOGetBitMapCode(TMS_OBJECT *srTMS, int inTxnType);
int inNCCCTMS_ISOPackMessageType(TMS_OBJECT *srTMS, int inTxnCode, unsigned char *uszPackData, char *szMTI);
int inNCCCTMS_ISOModifyBitMap(TMS_OBJECT *srTMS, int inTxnType, int *inBitMap);
int inNCCCTMS_ISOModifyPackData(TMS_OBJECT *srTMS, unsigned char *uszPackData, int *inPackLen);
int inNCCCTMS_ISOCheckHeader(TMS_OBJECT *srTMS, char *szSendISOHeader, char *szReceISOHeader);
int inNCCCTMS_ISOAnalyse(TMS_OBJECT *srTMS);

int inNCCCTMS_CommSendRecvToHost(TMS_OBJECT *srTMS, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
int inNCCCTMS_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran, TMS_OBJECT *srTMS);
int inNCCCTMS_CheckRespCode(TMS_OBJECT *srTMS);
int inNCCCTMS_DispHostResponseCode(TMS_OBJECT *srTMS);
int inNCCCTMS_SyncHostTerminalDateTime(TMS_OBJECT *srTMS);
int inNCCCTMS_ISO_CheckLRC(TMS_OBJECT *srTMS, unsigned char *szCheckData, int inSizes);
int inNCCCTMS_ISO_StoreTMSObject(TMS_OBJECT *srTMS, unsigned char *uszUpPack, int inUnPackSizes);
int inNCCCTMS_ISO_StoreFTPT(TMS_OBJECT *srTMS, unsigned char *uszUpPack, int inUnPackSizes);
void vdNCCCTMS_ISO_FormatDebug_DISP(TMS_OBJECT *srTMS, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCCTMS_ISO_FormatDebug_PRINT(TMS_OBJECT *srTMS, unsigned char *uszDebugBuf, int inSendLen);
int inNCCCTMS_ISO_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCCTMS_TABLE *srFieldType);
int inNCCCTMS_CheckFileSize(TMS_OBJECT *srTMS);
int inNCCCTMS_PrintErrorResult(TMS_OBJECT *srTMS);
int inNCCCTMS_CheckTraceLogFile(TMS_OBJECT *srTMS,unsigned char *uszFileName);

int inNCCCTMS_FTPS_CheckFileSize(TRANSACTION_OBJECT *pobTran);
int inNCCCTMS_DUTY_FREE_PackData(TMS_OBJECT *srTMS);




