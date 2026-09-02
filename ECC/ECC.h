/* 
 * File:   ECC.h
 * Author: user
 *
 * Created on 2018年3月12日, 下午 5:29
 */

#define _ECC_PARA_FILE_		"ICERINI.xml"
#define _ECC_API_REQ_FILE_	"ICERAPI.REQ"
#define _ECC_API_REQ_FILE2_	"ICERAPI2.REQ"  
#define _ECC_API_REQ_TEMP_	"ICERTEMP.REQ"
#define _ECC_API_REQ_TEMP_BAK_	"ICERTEMP.REQ.BAK"	/* 處理Temp時使用 */

#define _ECC_API_RES_FILE_		"ICERAPI.RES"
#define _ECC_API_RES_FILE2_		"ICERAPI2.RES"
#define _ECC_API_REQ_CHECK_		"ICERAPI.REQ.OK"
#define _ECC_API_REQ_RETRY_FILE_	"ICERRETRY.REQ"

#if (READER_MANUFACTURERS == LINUX_API)
	#define _ECC_FOLDER_NAME_		"ICERData"
	#define	_ECC_ICER_LOG_FOLDER_NAME_	"ICERLog"
	#define _ECC_ROOT_UPPER_PATH_		_AP_ROOT_PATH_
	#define _ECC_FOLDER_PATH_		"./ICERData/"
	#define _ECC_ICER_LOG_FOLDER_PATH_	"./ICERData/ICERLog/"
#else
	#if defined _NEW_ECC_
		#define _ECC_SUB_FOLDER_NAME_		"eticket"
		#define _ECC_FOLDER_NAME_		"ecc"
		#define	_ECC_ICER_LOG_FOLDER_NAME_	""
		#define _ECC_ROOT_UPPER_PATH_		_AP_PUB_PATH_
		#define _ECC_FOLDER_PATH_		"/home/ap/pub/ecc/"
		#define _ECC_ICER_LOG_FOLDER_PATH_	"/home/ap/pub/ecc/"
	#elif defined _NEW_ECC_NEXSYS_
		#define _ECC_FOLDER_NAME_		""
		#define	_ECC_ICER_LOG_FOLDER_NAME_	""
		#define _ECC_ROOT_UPPER_PATH_		_AP_ROOT_PATH_
		#define _ECC_FOLDER_PATH_		"./"
		#define _ECC_ICER_LOG_FOLDER_PATH_	"./"
		#define _ECC_DEBUG_SWITCH_FILE_NAME_	"ECC_DEBUG_FLAG"
	#else
		#define _ECC_FOLDER_NAME_		""
		#define	_ECC_ICER_LOG_FOLDER_NAME_	""
		#define _ECC_ROOT_UPPER_PATH_		_AP_ROOT_PATH_
		#define _ECC_FOLDER_PATH_		"./fs_data/"
		#define _ECC_ICER_LOG_FOLDER_PATH_	"./fs_data/"
	#endif
#endif

#define _ECC_HEADER_LENTH_	17
#define _ECC_F03_LENTH_		3
#define _ECC_F04_LENTH_		6
#define _ECC_F11_LENTH_		3
#define _ECC_F12_LENTH_		3
#define _ECC_F13_LENTH_		2
#define _ECC_F22_LENTH_		2
#define _ECC_F24_LENTH_		2
#define _ECC_F25_LENTH_		1
#define _ECC_F32_LENTH_		3
#define _ECC_F35_LENTH_		35
#define _ECC_F37_LENTH_		12
#define _ECC_F38_LENTH_		6
#define _ECC_F39_LENTH_		2
#define _ECC_F41_LENTH_		8
#define _ECC_F42_LENTH_		15
#define _ECC_F48_LENTH_		21
#define _ECC_F56_LENTH_		6
//#define _ECC_F59_ET_LENTH_	34      /* 這裡指Table "ET" Data位置，前面有Length(2)+N1(6)+N2(6)+N3(8)+NF(8)+ET(4) = 34 */ 
#define _ECC_F62_LENTH_		72      /* F_59Length(2)+F_59(60)+F_60Length(2)+F_60(6)+F_62Length(2) = 72 */

#define _XML_TAG_TYPE_NONE_	0
#define _XML_TAG_TYPE_START_	1
#define _XML_TAG_TYPE_END_	2

#define _ECC_DEFAULT_BATCH_	"01"

typedef struct
{
	char	szT1100[6 + 1];
	char	szT1101[6 + 1];
	char	szT1102[6 + 1];
	char	szT1103[6 + 1];
}ECC_RETRY_DATA;

int inECC_Init_Flow(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_ICERINI(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Init_ICERAPI(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Init_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran);
int inECC_UNPACK_ICERAPI(TRANSACTION_OBJECT *pobTran, char *szFileName);
int inECC_UNPACK_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran, char *szFileName, char* szSource);
int inECC_ResponeCode(TRANSACTION_OBJECT *pobTran);
int inECC_ResponeCode_MCDONALDS(TRANSACTION_OBJECT *pobTran);
int inECC_APIflow(TRANSACTION_OBJECT *pobTran);
int inECC_AutoAdd_Flow(TRANSACTION_OBJECT *pobTran);
int inECC_Deduct_Flow(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Deduct_ICERAPI(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Deduct_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran);
int inECC_Refund_Flow(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Refund_ICERAPI(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Refund_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran);
int inECC_TOP_UP_Flow(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_TOP_UP_ICERAPI(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_TOP_UP_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran);
int inECC_Void_TOP_UP_Flow(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Void_TOP_UP_ICERAPI(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Void_TOP_UP_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran);
int inECC_Inquiry_Flow(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Inquiry_ICERAPI(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Inquiry_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran);
int inECC_Settle_Flow(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Settle_ICERAPI(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_Settle_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran);
int inECC_POLL_Flow(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_POLL_ICERAPI(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_POLL_ICERAPI_NCCC(TRANSACTION_OBJECT *pobTran);
int inECC_Batch_Check(char* szECCBatch);
int inECC_Batch_Update(void);
int inECC_ParseXML_In_fs_data(char* szFileName, xmlDocPtr* srDoc);
int inECC_ParseXML_In_ICERData(char* szFileName, xmlDocPtr *srDoc);
int inECC_Delete_Log(int inRemainSize, unsigned char uszCanOverBit);
int inECC_Process_Temp(void);
int inECC_Insert_Temp(void);
int inECC_Settle_Other_REV_ADV(TRANSACTION_OBJECT *pobTran);
int inECC_PACK_RETRY_ICERAPI(TRANSACTION_OBJECT *pobTran);
int inECC_Set_Update_Batch_Flag(TRANSACTION_OBJECT *pobTran);
int inECC_Check_ICER_LOG_SIZE(void);
int inECC_HAL_SetDebug(void);
