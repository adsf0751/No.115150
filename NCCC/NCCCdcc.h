/* 
 * File:   NCCCdcc.h
 * Author: user
 *
 * Created on 2016年3月30日, 上午 10:24
 */

#define _NCCC_DCC_CHECK_BIT_MAP_(x, b)	((x) & (1U<<(b)))

#define _NCCC_DCC_NULL_TX_              0
#define _NCCC_DCC_MAX_BIT_MAP_CNT_	40
#define _NCCC_DCC_MTI_SIZE_		2
#define _NCCC_DCC_PCODE_SIZE_		3
#define _NCCC_DCC_TPDU_SIZE_		5
#define _NCCC_DCC_BIT_MAP_SIZE_		8

#define _NCCC_DCC_RRN_SIZE_		12		/* RRN */
#define _NCCC_DCC_ISO_SEND_		1536
#define _NCCC_DCC_ISO_RECV_		1536

#define _NCCC_DCC_ISO_ASC_		1		/* a */
#define _NCCC_DCC_ISO_BCD_		2
#define _NCCC_DCC_ISO_NIBBLE_2_		3		/* ..nibble */
#define _NCCC_DCC_ISO_NIBBLE_3_		4		/* ...nibble */
#define _NCCC_DCC_ISO_BYTE_2_		5		/* ..ans */
#define _NCCC_DCC_ISO_BYTE_3_		6		/* ...ans */
#define _NCCC_DCC_ISO_BYTE_2_H_		7		/* ..ans */
#define _NCCC_DCC_ISO_BYTE_3_H_		8		/* ...ans */
#define _NCCC_DCC_ISO_ASC_1_		9		/* a */
#define _NCCC_DCC_ISO_BYTE_1_           10		/* ..ans */

/* DCC參數檔案 START */
#define _NCCC_DCC_FILE_CARD_BIN_				"DCCBin.dat"
#define _NCCC_DCC_FILE_CARD_BIN_TEMP_				"DCCBin_T.dat"
#define _NCCC_DCC_FILE_CARD_BIN_TEMP_2_				"DCCBin_T_2.dat"
#define _NCCC_DCC_FILE_PARA_					"DCCPara.dat"
#define _NCCC_DCC_FILE_PARA_TEMP_				"DCCPara_T.dat"

#define _NCCC_DCC_FILE_EXCHANGE_RATE_SOURCE_			"ERS.dat"
#define _NCCC_DCC_FILE_VISA_DISCLAIMER_				"VISADis.dat"
#define _NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_			"MASTERDis.dat"
#define _NCCC_DCC_FILE_VISA_SUPPORTED_CURRENCY_LIST_		_CCI_FILE_NAME_
#define _NCCC_DCC_FILE_MASTERCARD_SUPPORTED_CURRENCY_LIST_	"CCI_M.dat"
					
/* DCC參數檔案 END */

#define _NCCC_DCC_BIN_RECORD_SIZE_	8		/* bin長度加0D0A */

#define _NCCC_DCC_FLOW_CANCEL_		1		/* 取消回idle */
#define _NCCC_DCC_FLOW_TWD_TRASACTION_	2		/* 台幣交易流程 */
#define _NCCC_DCC_FLOW_RATE_REQUEST_	3		/* DCC詢價流程 */
#define _NCCC_DCC_FLOW_MANUAL_SELECT_	4		/* 幣別選單流程 */

#define _NCCC_DCC_DOWNLOAD_MODE_NOARMAL_	"0"	/* 0:除非手動否則不下載 */
#define _NCCC_DCC_DOWNLOAD_MODE_HOUR_		"1"	/* 1:整點提示(如果沒有舊參數又下載失敗) */
#define _NCCC_DCC_DOWNLOAD_MODE_NOW_		"2"	/* 2:立即下載(用於TMS連動DCC重開機立即下載，在TMS更新完後從0設為2) */

#define _CURRENCY_JPY_ENG_NAME_			"JPY"
#define _CURRENCY_JPY_ENG_NAME_LEN_		3
#define _CURRENCY_JPY_CHI_NAME_			"日幣"
#define _CURRENCY_JPY_CHI_NAME_LEN_		6
#define _CURRENCY_JPY_CALLING_CODES_		"81"
#define _CURRENCY_JPY_CALLING_CODES_LEN_	2

#define _CURRENCY_USD_ENG_NAME_			"USD"
#define _CURRENCY_USD_ENG_NAME_LEN_		3
#define _CURRENCY_USD_CHI_NAME_			"美元"
#define _CURRENCY_USD_CHI_NAME_LEN_		6
#define _CURRENCY_USD_CALLING_CODES_		"1"
#define _CURRENCY_USD_CALLING_CODES_LEN_	1

#define _CURRENCY_HKD_ENG_NAME_			"HKD"
#define _CURRENCY_HKD_ENG_NAME_LEN_		3
#define _CURRENCY_HKD_CHI_NAME_			"港幣"
#define _CURRENCY_HKD_CHI_NAME_LEN_		6
#define _CURRENCY_HKD_CALLING_CODES_		"852"
#define _CURRENCY_HKD_CALLING_CODES_LEN_	3

#define _CURRENCY_EUR_ENG_NAME_			"EUR"
#define _CURRENCY_EUR_ENG_NAME_LEN_		3
#define _CURRENCY_EUR_CHI_NAME_			"歐元"
#define _CURRENCY_EUR_CHI_NAME_LEN_		6
#define _CURRENCY_EUR_CALLING_CODES_		"30~44,49"
#define _CURRENCY_EUR_CALLING_CODES_LEN_	8

#define _CURRENCY_KRW_ENG_NAME_			"KRW"
#define _CURRENCY_KRW_ENG_NAME_LEN_		3
#define _CURRENCY_KRW_CHI_NAME_			"韓元"
#define _CURRENCY_KRW_CHI_NAME_LEN_		6
#define _CURRENCY_KRW_CALLING_CODES_		"82"
#define _CURRENCY_KRW_CALLING_CODES_LEN_	2

#define _CURRENCY_MYR_ENG_NAME_			"MYR"
#define _CURRENCY_MYR_ENG_NAME_LEN_		3
#define _CURRENCY_MYR_CHI_NAME_			"馬幣"
#define _CURRENCY_MYR_CHI_NAME_LEN_		6
#define _CURRENCY_MYR_CALLING_CODES_		"60"
#define _CURRENCY_MYR_CALLING_CODES_LEN_	2

#define _CURRENCY_SGD_ENG_NAME_			"SGD"
#define _CURRENCY_SGD_ENG_NAME_LEN_		3
#define _CURRENCY_SGD_CHI_NAME_			"新幣"
#define _CURRENCY_SGD_CHI_NAME_LEN_		6
#define _CURRENCY_SGD_CALLING_CODES_		"65"
#define _CURRENCY_SGD_CALLING_CODES_LEN_	2

#define _CURRENCY_THB_ENG_NAME_			"THB"
#define _CURRENCY_THB_ENG_NAME_LEN_		3
#define _CURRENCY_THB_CHI_NAME_			"泰幣"
#define _CURRENCY_THB_CHI_NAME_LEN_		6
#define _CURRENCY_THB_CALLING_CODES_		"66"
#define _CURRENCY_THB_CALLING_CODES_LEN_	2

#define _CURRENCY_AUD_ENG_NAME_			"AUD"
#define _CURRENCY_AUD_ENG_NAME_LEN_		3
#define _CURRENCY_AUD_CHI_NAME_			"澳幣"
#define _CURRENCY_AUD_CHI_NAME_LEN_		6
#define _CURRENCY_AUD_CALLING_CODES_		"61"
#define _CURRENCY_AUD_CALLING_CODES_LEN_	2

#define _CURRENCY_CAD_ENG_NAME_			"CAD"
#define _CURRENCY_CAD_ENG_NAME_LEN_		3
#define _CURRENCY_CAD_CHI_NAME_			"加幣"
#define _CURRENCY_CAD_CHI_NAME_LEN_		6
#define _CURRENCY_CAD_CALLING_CODES_		"1"
#define _CURRENCY_CAD_CALLING_CODES_LEN_	1

#define _CURRENCY_TWD_ENG_NAME_			"TWD"
#define _CURRENCY_TWD_ENG_NAME_LEN_		3
#define _CURRENCY_TWD_CHI_NAME_			"台幣"
#define _CURRENCY_TWD_CHI_NAME_LEN_		6

#define _CURRENCY_UNKNOWN_CHI_NAME_		"未知"
#define _CURRENCY_UNKNOWN_CHI_NAME_LEN_		6

#define _DCC_INSTALL_INDICATOR_NOT_NEW_INSTALL_		'0'
#define _DCC_INSTALL_INDICATOR_NEW_INSTALL_		'1'
#define _DCC_INSTALL_INDICATOR_NOT_NEW_INSTALL_11BIN_	'2'
#define _DCC_INSTALL_INDICATOR_NEW_INSTALL_11BIN_	'3'

#define _DCC_FINAL_OPTION_LINE_WIDTH_	4
#define _DCC_FINAL_OPTION_LINE_MARGIN_	4
#define _DCC_FINAL_OPTION_INNER_WIDTH_	(_LCD_XSIZE_ - (_DCC_FINAL_OPTION_LINE_WIDTH_ * 2) - (_DCC_FINAL_OPTION_LINE_MARGIN_ * 2))
#define _DCC_FINAL_OPTION_INNER_HEIGHT_	((_LCD_YSIZE_ / 8) - (_DCC_FINAL_OPTION_LINE_WIDTH_ * 2))

#define _DCC_FINAL_OPTION_OUTER_WIDTH_		(_DCC_FINAL_OPTION_INNER_WIDTH_	+ (_DCC_FINAL_OPTION_LINE_WIDTH_ * 2))
#define _DCC_FINAL_OPTION_OUTER_HEIGHT_		(_DCC_FINAL_OPTION_INNER_HEIGHT_+ (_DCC_FINAL_OPTION_LINE_WIDTH_ * 2))

/* 如果要置中，則x起始點為(_LCD_XSIZE_ - _SIGNEDPAD_OUT_WIDTH_) / 2 */
#define	_DCC_FINAL_OPTION_FOREIGN_CURRENCY_INNER_X1_		((_LCD_XSIZE_ - _DCC_FINAL_OPTION_INNER_WIDTH_) / 2)
#define	_DCC_FINAL_OPTION_FOREIGN_CURRENCY_INNER_Y1_		(_COORDINATE_Y_LINE_8_2_ + _DCC_FINAL_OPTION_LINE_WIDTH_)
#define	_DCC_FINAL_OPTION_FOREIGN_CURRENCY_OUTER_X1_		((_LCD_XSIZE_ - _DCC_FINAL_OPTION_OUTER_WIDTH_) / 2)
#define	_DCC_FINAL_OPTION_FOREIGN_CURRENCY_OUTER_Y1_		_COORDINATE_Y_LINE_8_2_

#define	_DCC_FINAL_OPTION_LOCAL_CURRENCY_INNER_X1_		((_LCD_XSIZE_ - _DCC_FINAL_OPTION_INNER_WIDTH_) / 2)
#define	_DCC_FINAL_OPTION_LOCAL_CURRENCY_INNER_Y1_		(_COORDINATE_Y_LINE_8_7_ + _DCC_FINAL_OPTION_LINE_WIDTH_)
#define	_DCC_FINAL_OPTION_LOCAL_CURRENCY_OUTER_X1_		((_LCD_XSIZE_ - _DCC_FINAL_OPTION_OUTER_WIDTH_) / 2)
#define	_DCC_FINAL_OPTION_LOCAL_CURRENCY_OUTER_Y1_		_COORDINATE_Y_LINE_8_7_

typedef struct
{
        unsigned long   ulDCC_BINLen;			/* BinTable累積的長度 */
	char		szDCC_BTMC[8 + 1];		/* Batch/File transfer message count (8 Byte) */
	char		szDCC_BTFI[32 + 1];		/* Batch/File transfer file identfication (32 Byte) */
	char		szDCC_FTFS[6 + 1];		/* File Transfer file size (6 Byte) */
	char		szDCC_FTEDRC[6 + 1];		/* File transfer elementary data record count (6 Byte) */
	char		szDCC_FTREDRC[6 + 1];		/* File transfer remaining elementary data record count (6 Byte) */
	char		szDCC_FC[4 + 1];		/* Function Code (4 Byte) */
	char		szDCC_AP[2 + 1];		/* Available Parameters (1 Byte) */
	char		szDCC_AC[4 + 1];		/* Action Code (4 Byte) */
	char		szDCC_PURC[2 + 1];		/* Parameter Update Result Code (1 Byte) */
	char		szDCC_BTURC[2 + 1];		/* BIN Table Update Ressult Code (1 Byte) */
	char		szDCC_Install[2 + 1];		/* Installation Indicator (1 Byte) */
	char		szDCC_BINVersion[4 + 1];	/* BIN Table Version (4 Byte) */
	char		szDCC_FileList[8 + 1];		/* 目前總共有7個檔案，若陣列中填入1代表需要下載該檔案 */
	char		szDCC_FileDownloadStaus[8 + 1];	/* 目前總共有7個檔案，若陣列中填入1代表該檔案下載成功 */
        unsigned char   uszDCC_Record[254 + 1];		/* Data Record (max to 250 Byte) */
} DCC_DATA;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOLoad)(TRANSACTION_OBJECT *, unsigned char *); /* 組 Field 的功能 */
} ISO_FIELD_NCCC_DCC_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int (*inISOCheck)(TRANSACTION_OBJECT *, unsigned char *, unsigned char *); /* 檢查 Field 的功能 */
} ISO_CHECK_NCCC_DCC_TABLE;

typedef struct
{
        int inFieldNum; /* Field Number */
        int inFieldType; /* Field Type */
        unsigned char uszDispAscii; /* 是否顯示其 ASCII 字元 */
        int inFieldLen; /* Field Len */
} ISO_FIELD_TYPE_NCCC_DCC_TABLE;

typedef struct
{
        int inTxnID; /* 交易類別 */
        int *inBitMap; /* Bit Map */
        char szMTI[_NCCC_DCC_MTI_SIZE_ * 2 + 1]; /* Message Type */
        char szPCode[_NCCC_DCC_PCODE_SIZE_ * 2 + 1]; /* Processing Code */
} BIT_MAP_NCCC_DCC_TABLE;

typedef struct
{
        ISO_FIELD_NCCC_DCC_TABLE *srPackISO; /* 組封包的功能結構 */
        ISO_FIELD_NCCC_DCC_TABLE *srUnPackISO; /* 解封包的功能結構 */
        ISO_CHECK_NCCC_DCC_TABLE *srCheckISO; /* 檢查封包的功能結構 */
        ISO_FIELD_TYPE_NCCC_DCC_TABLE *srISOFieldType; /* ISO Field 型態結構 */
        BIT_MAP_NCCC_DCC_TABLE *srBitMap; /* Bit Map的陣列 */
        int (*inPackMTI)(TRANSACTION_OBJECT *, int , unsigned char *, char *); /* 組 Message Type 的功能 */
        int (*inModifyBitMap)(TRANSACTION_OBJECT *, int , int *);/* 組封包前修改 Bit Map 的功能 */
        int (*inModifyPackData)(TRANSACTION_OBJECT *, unsigned char *, int *); /* 組封包後修改整個封包的功能 */
        int (*inCheckISOHeader)(TRANSACTION_OBJECT *, char *, char *); /* 檢查 Message Type 的功能 */
        int (*inOnAnalyse)(TRANSACTION_OBJECT *); /* Online交易分析 */
        int (*inAdviceAnalyse)(TRANSACTION_OBJECT *, unsigned char *); /* 收到 Advice 後的分析 */
} ISO_TYPE_NCCC_DCC_TABLE;

int inNCCC_DCC_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack25(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack25_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack35(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack35_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack35_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack54(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack55_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack55_Software(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack56(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack57_tSAM(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack60(TRANSACTION_OBJECT* pobTran, unsigned char* uszPackBuf);
int inNCCC_DCC_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);
int inNCCC_DCC_Pack63(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf);

int inNCCC_DCC_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_DCC_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_DCC_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_DCC_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_DCC_UnPack55(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_DCC_UnPack59(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);
int inNCCC_DCC_UnPack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf);

int inNCCC_DCC_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);
int inNCCC_DCC_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket);

int inNCCC_DCC_Func_SetTxnOnlineOffline(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Func_BuildAndSendPacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI);
int inNCCC_DCC_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap);
int inNCCC_DCC_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *bPackData, int *inPackLen);
int inNCCC_DCC_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader);
int inNCCC_DCC_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload);
int inNCCC_DCC_BitMapSet(int *inBitMap, int inFeild);
int inNCCC_DCC_BitMapReset(int *inBitMap, int inFeild);
int inNCCC_DCC_GetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_SetSTAN(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ProcessReversal(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inNCCC_DCC_Advice_ESC_SendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt);
int inNCCC_DCC_ProcessAdvice(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ProcessAdvice_ESC(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ProcessOnline(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ProcessOffline(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_AnalysePacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ReversalSave(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket);
int inNCCC_DCC_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_CheckRespCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_CheckAuthCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_OnlineAnalyseEMV(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_OnlineEMV_Complete(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_DispHostResponseCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ProcessReferral(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode);
int inNCCC_DCC_CheckUnPackField(int inField, ISO_FIELD_NCCC_DCC_TABLE *srCheckUnPackField);
int inNCCC_DCC_GetCheckField(int inField, ISO_CHECK_NCCC_DCC_TABLE *ISOFieldCheck);
int inNCCC_DCC_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_NCCC_DCC_TABLE *srFieldType);
int inNCCC_DCC_GetFieldIndex(int inField, ISO_FIELD_TYPE_NCCC_DCC_TABLE *srFieldType);
int inNCCC_DCC_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf);
int inNCCC_DCC_GetReversalCnt(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode);
int inNCCC_DCC_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);
int inNCCC_DCC_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit);

int inNCCC_DCC_CHECK(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_GetDCC_Enable(int inOrgHDTIndex, char *szHostEnable);
int inNCCC_DCC_SwitchToDCC_Host(int inOrgHDTIndex);
int inNCCC_DCC_CurrencyOption(TRANSACTION_OBJECT * pobTran);
int inNCCC_DCC_Display_Currency_Option(int inPage);
int inNCCC_DCC_Get_Currency_Name_CHI_ByCurrencyNameENG(char *szCurrencyNameENG, char *szCurrencyNameCHI);
int inNCCC_DCC_Get_Country_Calling_Codes(char *szCurrencyNameENG, char *szCallingCodes);
int inNCCC_DCC_Get_Currency_Name_ENG_ByCurrencyCode(char *szCurrencyCode, char *szCurrencyNameENG);
int inNCCC_DCC_ChipCard_Check_CurrencyCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_CTLS_Check_CurrencyCode(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_OnlineRate(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Final_DCC_Option(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_OnlineRate_CheckF59_TableR(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_LocalBin_Check(char* szPAN);
int inNCCC_DCC_Sync_BatchNumber(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ChangeToTWD_For_SALE(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_ChangeToTWD_For_PRE_COMP(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Transacton_Origin_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Transacton_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Already_Have_Parameter(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Fun6_ExchangeRateDownload(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_PRINT_ExchangeRate(TRANSACTION_OBJECT *pobTran);

/* DCC 參數下載 */
int inNCCC_DCC_TMSDownload(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_TMSDownload_CheckStatus(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_SaveParameter(TRANSACTION_OBJECT *pobTran, unsigned long* ulHandle);
int inNCCC_DCC_CheckSumData(unsigned char *uszDataRecord);
int inNCCC_DCC_CutAndFormat_TempFile(void);
int inNCCC_DCC_Format_DCCBin_TempFile(void);
int inNCCC_DCC_Cut_ParameterFile(void);
int inNCCC_DCC_Make_Parameter_File(unsigned char *uszFileName, unsigned char *uszData, int* inCnt);
int inNCCC_DCC_Make_CCI(unsigned char *uszFileName, unsigned char *uszData, int *inCnt);
int inNCCC_DCC_AutoDownload_Check(void);
int inNCCC_DCC_AutoDownload_Check_Date(void);
int inNCCC_DCC_AutoDownload_Check_Time(void);
int inNCCC_DCC_AutoDownload(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Disclaimer_Display(char *szFilename, int inFontSize, int inLine);
int inNCCC_DCC_TMS_Schedule_Hour_Check(void);
int inNCCC_DCC_TMS_Schedule_Check(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_TMS_Schedule(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Hour_Notify(TRANSACTION_OBJECT *pobTran);
int inNCCC_DCC_Clean_Parameter(void);

void vdNCCC_DCC_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCC_DCC_ISO_FormatDebug_DISP_EMV(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_DCC_ISO_FormatDebug_DISP_58(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_DCC_ISO_FormatDebug_DISP_59(unsigned char *uszDebugBuf, int inFieldLen);
void vdNCCC_DCC_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen);
void vdNCCC_DCC_ISO_FormatDebug_PRINT_EMV(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
void vdNCCC_DCC_ISO_FormatDebug_PRINT_58(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
void vdNCCC_DCC_ISO_FormatDebug_PRINT_59(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle);
