/* 
 * File:   NCCCesc.h
 * Author: user
 *
 * Created on 2016年3月30日, 上午 10:25
 */

#define _NCCC_ESC_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _NCCC_ESC_NULL_TX_                  0
#define _NCCC_ESC_MAX_BIT_MAP_CNT_          40
#define _NCCC_ESC_MTI_SIZE_                 2
#define _NCCC_ESC_PCODE_SIZE_               3
#define _NCCC_ESC_TPDU_SIZE_                5
#define _NCCC_ESC_BIT_MAP_SIZE_             8
#define _NCCC_ESC_RRN_SIZE_                 12 /* RRN */
#define _NCCC_ESC_ISO_SEND_                 1536
#define _NCCC_ESC_ISO_RECV_                 1536
#define _NCCC_ESC_ISO_ASC_                  1 /* a */
#define _NCCC_ESC_ISO_BCD_                  2
#define _NCCC_ESC_ISO_NIBBLE_2_             3 /* ..nibble */
#define _NCCC_ESC_ISO_NIBBLE_3_             4 /* ...nibble */
#define _NCCC_ESC_ISO_BYTE_2_               5 /* ..ans */
#define _NCCC_ESC_ISO_BYTE_3_               6 /* ...ans */
#define _NCCC_ESC_ISO_BYTE_2_H_             7 /* ..ans */
#define _NCCC_ESC_ISO_BYTE_3_H_             8 /* ...ans */
#define _NCCC_ESC_ISO_ASC_1_                9 /* a */
#define _NCCC_ESC_ISO_BYTE_1_               10/* ..ans */

#define _NCCC_ESC_CANCELLED_		0
#define _NCCC_ESC_AUTHORIZED_		1
#define _NCCC_ESC_REFERRAL_		2
#define _NCCC_ESC_SETTLE_UPLOAD_BATCH_	3
#define _NCCC_ESC_DECLINED_		-1
#define _NCCC_ESC_COMM_ERROR_		-2
#define _NCCC_ESC_GEN2AC_ERR_		-3
#define _NCCC_ESC_PACK_ERR_		-4
#define _NCCC_ESC_UNPACK_ERR_		-5

/* ESC使用 */
#define	_ESC_FILE_RECEIPT_GZ_		"ESC_E1.txt.gz"
#define	_ESC_FILE_RECEIPT_GZ_ENCRYPTED_	"ESC_E1_E.txt.gz"
#define	_ESC_FILE_BMP_GZ_		"ESC_E2.bmp.gz"
#define	_ESC_FILE_BMP_GZ_ENCRYPTED_	"ESC_E2_E.bmp.gz"
#define _ESC_FILE_BMP_BACKUP_		"ESC_BMP_BACKUP.bmp"
#define _TABLE_E1_			31
#define _TABLE_E2_			32
#define _ESC_ISO_MAX_LEN_		720		/* ESC上傳時，單筆封包所含的簽單or簽名檔大小，避免整個封包超過單筆極限 */
#define	_ESC_LIMIT_			23

/* 標示支不支援ESC狀態 */
#define	_ESC_STATUS_NONE_			0		/* 未設定 */
#define	_ESC_STATUS_NOT_SUPPORTED_		1		/* 不支援ESC上傳 */
#define _ESC_STATUS_OVERLIMIT_			2		/* 超過水位而出紙本 */
#define _ESC_STATUS_SUPPORTED_			3		/* 支援ESC上傳 */

/* 這邊改為只用於結帳條統計 已上傳、出紙本、 重試中、 重試失敗不上傳 */
#define _ESC_ACCUM_STATUS_UPLOADED_	1		/* ESC已上傳 */
#define _ESC_ACCUM_STATUS_BYPASS_	2		/* 出紙本 */
#define	_ESC_ACCUM_STATUS_AGAIN_	3		/* 重試(過渡狀態，最後會變成已上傳或失敗) */
#define _ESC_ACCUM_STATUS_FAIL_		4		/* 失敗 */

/* 標示ESC上傳狀態*/
#define _ESC_UPLOAD_STATUS_UPLOAD_NOT_YET_	0	/* ESC未上傳 */
#define _ESC_UPLOAD_STATUS_UPLOADED_		1	/* ESC已上傳 */
#define _ESC_UPLOAD_STATUS_PAPER_		2	/* 已出紙本 */

/* IDLE下是否上傳 */
#define _ESC_IDLE_NO_NEED_UPLOAD_	0		/* 不需上傳 */
#define _ESC_IDLE_UPLOAD_		1		/* 需要上傳 */

#define _ESC_SEND_RETRY_LIMIT_		3

/* 和esc主機連線狀態 */
#define	_ESC_CONNECT_NOT_CONNECT_	0		/* 沒連 */
#define _ESC_CONNECT_ALREADY_		1		/* 有連 */

/* 在什麼狀態下上傳ESC */
#define _ESC_UPLOAD_TRADE_		1		/* 當筆 */
#define _ESC_UPLOAD_IDLE_		2		/* IDLE */
#define _ESC_UPLOAD_SETTLE_		3		/* 結帳 */

#define _ACQ_ID_		        "00493817"

#define	_ESC_REINFORCE_TXNCODE_SALE_	1
#define	_ESC_REINFORCE_TXNCODE_REFUND_	2

/* ESC電子簽單，上傳使用結構 Start */

typedef struct ESC_E1E2_DATA
{
	int	inTableIndex;				/* 用來表示E1 or E2 */
        int     inTotalPacketCnt;			/* 總封包數 */
        int     inPacketCnt;				/* 第幾個封包 */
        long  	lnTotalPacketSize;			/* 總封包大小 */
        int     inPacketSize;				/* 該封包大小 */
        char	szPackData[_ESC_ISO_MAX_LEN_ + 1];	/* 封包內容 */
        char    szLRC[1 + 1];				/* 檢查碼 = 內容長度到內容每個byteXOR */
} EscData;

/* ESC電子簽單，上傳使用結構 End */

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_NCCC_ESC_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_NCCC_ESC_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_NCCC_ESC_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_NCCC_ESC_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_NCCC_ESC_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_NCCC_ESC_TABLE;

typedef struct
{
        ISO_FIELD_NCCC_ESC_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_NCCC_ESC_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_NCCC_ESC_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_NCCC_ESC_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_NCCC_ESC_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inGetBitMapCode)(TRANSACTION_OBJECT *, int);/* 取得組 Bit Map 的交易類別 */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_NCCC_ESC_TABLE;

typedef struct
{
	int	inESCSaleUploadCnt;		/* Sale已上傳筆數 */
	int	inESCRefundUploadCnt;		/* Refund已上傳筆數 */
	int	inESCSalePaperCnt;		/* Sale出紙本筆數 */
	int	inESCRefundPaperCnt;		/* Refund出紙本筆數 */
	long	inESCSaleUploadAmt;		/* Sale已上傳金額 */
	long	inESCRefundUploadAmt;		/* Refund已上傳金額 */
	long	inESCSalePaperAmt;		/* Sale出紙本金額 */
	long	inESCRefundPaperAmt;		/* Refund出紙本金額 */
} ESC_REINFORCE;

int inNCCC_ESC_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_MerchantCopy_Check(TRANSACTION_OBJECT *pobTran);

int inNCCC_ESC_Func_Upload(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Func_Upload_Idle(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Func_Upload_Settle(TRANSACTION_OBJECT *pobTran);

int inNCCC_ESC_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack15(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack32(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack44(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack45(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack52(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_ESC_Pack64(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inNCCC_ESC_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_ESC_Check04(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_ESC_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

int inNCCC_ESC_UnPack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_ESC_UnPack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);


int inNCCC_ESC_BitMapSet(int *inBitMap, int inFeild);
int inNCCC_ESC_BitMapReset(int *inBitMap, int inFeild);
int inNCCC_ESC_BitMapCheck(unsigned char *inBitMap, int inFeild);
int inNCCC_ESC_CopyBitMap(int *inBitMap, int *inSearchBitMap);
int inNCCC_ESC_GetBitMapTableIndex(ISO_TYPE_NCCC_ESC_TABLE *srISOFunc, int inBitMapCode);
int inNCCC_ESC_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_NCCC_ESC_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap, unsigned char *uszSendBuf);
int inNCCC_ESC_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inNCCC_ESC_CheckUnPackField(int inField, ISO_FIELD_NCCC_ESC_TABLE *srCheckUnPackField);
int inNCCC_ESC_GetCheckField(int inField, ISO_CHECK_NCCC_ESC_TABLE *ISOFieldCheck);
int inNCCC_ESC_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_ESC_TABLE *srFieldType);
int inNCCC_ESC_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_ESC_TABLE *srFieldType);
int inNCCC_ESC_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);


int inNCCC_ESC_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType);
int inNCCC_ESC_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI);
int inNCCC_ESC_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap);
int inNCCC_ESC_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen);
int inNCCC_ESC_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inNCCC_ESC_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload);

int inNCCC_ESC_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inNCCC_ESC_ProcessAdvice(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_ProcessOnline(TRANSACTION_OBJECT *pobTran, int inType);
int inNCCC_ESC_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_AnalysePacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_ReversalSave(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
int inNCCC_ESC_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_ProcessReferral(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_GetReversalCnt(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode);
int inNCCC_ESC_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inNCCC_ESC_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inNCCC_ESC_CUP_PowerOnLogon(void);
int inNCCC_ESC_TMKKeyExchange(TRANSACTION_OBJECT *pobTran,unsigned char *uszUnPackBuf);void vdNCCC_ESC_ISO_FormatDebug(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);

int inNCCC_ESC_GetESC_Enable(int inOrgHDTIndex, char* szHostEnable);
int inNCCC_ESC_SwitchToESC_Host(int inOrgHDTIndex);
int inNCCC_ESC_Data_Compress_Encryption(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Data_Packet(char *szFileName, int inTableIndex);
int inNCCC_ESC_Insert_Again_Record(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Insert_Fail_Record(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Get_Again_Record_Most_TOP(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Get_Fail_Record_Most_TOP(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Delete_Again_Record_Most_TOP(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Delete_Fail_Record_Most_TOP(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Again_Table_Count(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Fail_Table_Count(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Process_PowerOff_When_Signing(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Func_Upload_Idle_Check_Time(void);
int inNCCC_ESC_Check_Again_Record_Inv_Exist(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Check_Fail_Record_Inv_Exist(TRANSACTION_OBJECT *pobTran);

void vdNCCC_ESC_ISO_FormatDebug_DISP(unsigned char *uszSendBuf, int inSendLen);
void vdNCCC_ESC_ISO_FormatDebug_DISP_59(unsigned char *uszSendBuf, int inFieldLen);
void vdNCCC_ESC_ISO_FormatDebug_PRINT(unsigned char *uszSendBuf, int inSendLen);
void vdNCCC_ESC_ISO_FormatDebug_PRINT_59(unsigned char *uszSendBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
int inNCCC_ESC_Data_Compress_Encryption_Test(void);
int inNCCC_ESC_Test_Redundant_ESCPacket_Switch(void);
