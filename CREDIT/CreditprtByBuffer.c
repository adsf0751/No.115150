#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_qrcode.h>
#include <libxml/xpath.h>
#include <fcntl.h>		//2019.02.15 added by Hachi
#include <math.h>		//2019/2/26 下午 3:38
#include <unistd.h> 		//2019/3/4 下午 6:23
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/PRINT/PrtMsg.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/CCI.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CDTX.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/EST.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/SCDT.h"
#include "../SOURCE/FUNCTION/Signpad.h"
#include "../SOURCE/FUNCTION/PWD.h"
#include "../SOURCE/FUNCTION/PCD.h"
#include "../SOURCE/FUNCTION/IPASSDT.h"
#include "../SOURCE/FUNCTION/ECCDT.h"
#include "../SOURCE/FUNCTION/ICASHDT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/TMSIPDT.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../NCCC/NCCCsrc.h"
#include "../NCCC/NCCCdcc.h"
#include "../NCCC/NCCCloyalty.h"
#include "../NCCC/NCCCtms.h"
#include "../NCCC/NCCCtmsCPT.h"
#include "../NCCC/NCCCtmsSCT.h"
#include "../NCCC/NCCCTicketSrc.h"
#include "../NCCC/NCCCtmsFTP.h"
#include "../FISC/NCCCfisc.h"
#include "../NCCC/NCCCesc.h"
#include "../NCCC/TAKAsrc.h"
#include "../CTLS/CTLS.h"
#include "../HG/HGsrc.h"
#include "../HG/HGiso.h"
#include "../ECC/ICER/stdAfx.h"
#include "../ECC/ECC.h"
#include "Creditfunc.h"
#include "CreditprtByBuffer.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern  int		ginDebug;			/* Debug使用 extern */
extern	int		ginMachineType;
extern	char		gszTermVersionID[16 + 1];
extern	char		gszTermVersionDate[16 + 1];
extern	BMPHeight	gsrBMPHeight;			/* 圖片高度 */
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */
extern	unsigned short	gusPrintFontStyleRegular;

int     inPrinttype_ByBuffer = 0;        /* 0 = 橫式，1 = 直式 */

/* 列印帳單使用(START) */
PRINT_RECEIPT_TYPE_TABLE_BYBUFFER srReceiptType_ByBuffer[] =
{
	/* 信用卡 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer,
		inCREDIT_PRINT_Cup_Amount_ByBuffer,
		inCREDIT_PRINT_Amount_ByBuffer,
		inCREDIT_PRINT_Inst_ByBuffer,
		inCREDIT_PRINT_Redeem_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer
	},
       
	/* DCC 簽單 For Sale */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_DCC_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer
	},
	
	/* DCC 簽單 Not For Sale */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_DCC_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer
	},
	
	/* DCC 簽單 轉台幣 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_DCC_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer
	},
	
	/* 優惠兌換卷 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_LOYALTY_REDEEM_Data,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_
	},
	
	/* 優惠兌換取消 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_VOID_LOYALTY_REDEEM_Data,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_
	},
       
	/* SmartPay簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_FISC_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_FISC_Amount_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer
	},
	
	/* HappyGo交易簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_HG_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_HG_Amount_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_HG_ReceiptEND_ByBuffer
	},
       
	/* HappyGo混合交易簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer,
		inCREDIT_PRINT_Cup_Amount_ByBuffer,
		inCREDIT_PRINT_Amount_ByBuffer,
		inCREDIT_PRINT_Inst_ByBuffer,
		inCREDIT_PRINT_Redeem_ByBuffer,
		inCREDIT_PRINT_HG_Multi_Data_ByBuffer,
		inCREDIT_PRINT_ReceiptEND_ByBuffer
	},
	
	/* 一卡通簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer_ESVC,
		_NULL_CH_,
		inCREDIT_PRINT_Amount_ByBuffer_IPASS,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC
	},
	
	/* 悠遊卡簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer_ESVC,
		_NULL_CH_,
		inCREDIT_PRINT_Amount_ByBuffer_ECC,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC
	},
	
	/* 愛金卡簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer_ESVC,
		_NULL_CH_,
		inCREDIT_PRINT_Amount_ByBuffer_ICASH,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC
	},

	/* 縮小版簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_Data_ByBuffer_Small,
		inCREDIT_PRINT_Cup_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Inst_ByBuffer_Small,
		inCREDIT_PRINT_Redeem_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_Small
	},
       
	/* 縮小版DCC簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_DCC_Data_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small
	},
	
	/* 縮小版DCC 簽單 Not For Sale */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_DCC_Data_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small
	},
	
	/* 縮小版DCC 簽單 轉台幣 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_DCC_Data_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small
	},
       
	/* 縮小版SmartPay簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_FISC_Data_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_FISC_Amount_ByBuffer_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_Small
	},
       
	/* 縮小版HappyGo混合交易簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_Data_ByBuffer_Small,
		inCREDIT_PRINT_Cup_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Inst_ByBuffer_Small,
		inCREDIT_PRINT_Redeem_ByBuffer_Small,
		inCREDIT_PRINT_HG_Multi_Data_ByBuffer,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_Small
	},
	
	/* 信用卡(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_Data_ByBuffer_046,
		inCREDIT_PRINT_Cup_Amount_ByBuffer_046,
		inCREDIT_PRINT_Amount_ByBuffer_046,
		inCREDIT_PRINT_Inst_ByBuffer_046,
		inCREDIT_PRINT_Redeem_ByBuffer_046,
		inCREDIT_PRINT_LEGAL_046,
		inCREDIT_PRINT_Dutyfree_ReceiptEND_ByBuffer
	},
       
	/* DCC 簽單 For Sale(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_DCC_Data_ByBuffer_046,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_046
	},
	
	/* DCC 簽單 Not For Sale(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_DCC_Data_ByBuffer_046,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_046
	},
	
	/* DCC 簽單 轉台幣(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_DCC_Data_ByBuffer_046,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_046
	},
       
	/* SmartPay簽單(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_FISC_Data_ByBuffer_046,
		_NULL_CH_,
		inCREDIT_PRINT_FISC_Amount_ByBuffer_046,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_Dutyfree_ReceiptEND_ByBuffer
	},
	
	/* HappyGo交易簽單(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_HG_Data_ByBuffer_046,
		_NULL_CH_,
		inCREDIT_PRINT_HG_Amount_ByBuffer_046,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_HG_ReceiptEND_ByBuffer
	},
       
	/* HappyGo混合交易簽單(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_Data_ByBuffer_046,
		inCREDIT_PRINT_Cup_Amount_ByBuffer_046,
		inCREDIT_PRINT_Amount_ByBuffer_046,
		inCREDIT_PRINT_Inst_ByBuffer_046,
		inCREDIT_PRINT_Redeem_ByBuffer_046,
		inCREDIT_PRINT_HG_Multi_Data_ByBuffer_046,
		inCREDIT_PRINT_Dutyfree_ReceiptEND_ByBuffer
	},
	
	/* 一卡通簽單(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_Data_ByBuffer_ESVC_046,
		_NULL_CH_,
		inCREDIT_PRINT_Amount_ByBuffer_IPASS_046,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC_046
	},
	
	/* 悠遊卡簽單(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_Data_ByBuffer_ESVC_046,
		_NULL_CH_,
		inCREDIT_PRINT_Amount_ByBuffer_ECC_046,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC_046
	},
	
	/* 愛金卡簽單(046) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_046,
		inCREDIT_PRINT_Data_ByBuffer_ESVC_046,
		_NULL_CH_,
		inCREDIT_PRINT_Amount_ByBuffer_ICASH_046,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC_046
	},
	
	/* 信用卡(027) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer,
		inCREDIT_PRINT_Cup_Amount_ByBuffer,
		inCREDIT_PRINT_Amount_ByBuffer,
		inCREDIT_PRINT_Inst_ByBuffer,
		inCREDIT_PRINT_Redeem_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_027
	},
	
	/* SmartPay簽單(027) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_FISC_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_FISC_Amount_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_027
	},
	
	/* HappyGo混合交易簽單(027) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer,
		inCREDIT_PRINT_Cup_Amount_ByBuffer,
		inCREDIT_PRINT_Amount_ByBuffer,
		inCREDIT_PRINT_Inst_ByBuffer,
		inCREDIT_PRINT_Redeem_ByBuffer,
		inCREDIT_PRINT_HG_Multi_Data_ByBuffer,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_027
	},
	
	/* 客製化【061】 SALE 交易 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_DCC_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_GRAND_HOTEL_ReceiptEND_ByBuffer
	},
	
	/* 客製化【061】 NOT FOR SALE 交易 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_DCC_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_GRAND_HOTEL_ReceiptEND_ByBuffer
	},
	
	/* 客製化【061】 DCC 簽單 轉台幣 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_DCC_Data_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_GRAND_HOTEL_ReceiptEND_ByBuffer
	},
	
	/* 041 預借現金 043 建設公司(065) */
	{
		inCREDIT_PRINT_Logo_ByBuffer_041_043,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer_041_043,
		inCREDIT_PRINT_Cup_Amount_ByBuffer,
		inCREDIT_PRINT_Amount_ByBuffer_041_043,
		inCREDIT_PRINT_Inst_ByBuffer,
		inCREDIT_PRINT_Redeem_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043
	},
	
	/* 信用卡(客製化005-FPG) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer_005,
		inCREDIT_PRINT_Cup_Amount_ByBuffer,
		inCREDIT_PRINT_Amount_ByBuffer,
		inCREDIT_PRINT_Inst_ByBuffer,
		inCREDIT_PRINT_Redeem_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer
	},
	
	/* HappyGo交易簽單(客製化005-FPG) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_HG_Data_ByBuffer_005,
		_NULL_CH_,
		inCREDIT_PRINT_HG_Amount_ByBuffer,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_HG_ReceiptEND_ByBuffer
	},
       
	/* HappyGo混合交易簽單(客製化005-FPG) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer_005,
		inCREDIT_PRINT_Cup_Amount_ByBuffer,
		inCREDIT_PRINT_Amount_ByBuffer,
		inCREDIT_PRINT_Inst_ByBuffer,
		inCREDIT_PRINT_Redeem_ByBuffer,
		inCREDIT_PRINT_HG_Multi_Data_ByBuffer,
		inCREDIT_PRINT_ReceiptEND_ByBuffer
	},
	
	/* 縮小版簽單(S) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_Data_ByBuffer_Small_S,
		inCREDIT_PRINT_Cup_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Amount_ByBuffer_Small_S,
		inCREDIT_PRINT_Inst_ByBuffer_Small,
		inCREDIT_PRINT_Redeem_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_Small_S
	},
       
	/* 縮小版DCC簽單(S) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_DCC_Data_ByBuffer_Small_S,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small_S
	},
	
	/* 縮小版DCC 簽單 Not For Sale(S) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_DCC_Data_ByBuffer_Small_S,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small_S
	},
	
	/* 縮小版DCC 簽單 轉台幣(S) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_DCC_Data_ByBuffer_Small_S,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small_S
	},
       
	/* 縮小版SmartPay簽單(S) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_FISC_Data_ByBuffer_Small_S,
		_NULL_CH_,
		inCREDIT_PRINT_FISC_Amount_ByBuffer_Small,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_Small_S
	},
       
	/* 縮小版HappyGo混合交易簽單(S) */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_Data_ByBuffer_Small_S,
		inCREDIT_PRINT_Cup_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Amount_ByBuffer_Small_S,
		inCREDIT_PRINT_Inst_ByBuffer_Small,
		inCREDIT_PRINT_Redeem_ByBuffer_Small,
		inCREDIT_PRINT_HG_Multi_Data_ByBuffer,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_Small_S
	},
	
	/* 041 預借現金 043 建設公司縮小簽單 */
	{
		inCREDIT_PRINT_Logo_ByBuffer_041_043,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_Data_ByBuffer_041_043_Small,
		inCREDIT_PRINT_Cup_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Amount_ByBuffer_041_043_Small,
		inCREDIT_PRINT_Inst_ByBuffer_Small,
		inCREDIT_PRINT_Redeem_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043_Small
	},
	
	/* 041 預借現金 043 建設公司縮小簽單(S) */
	{
		inCREDIT_PRINT_Logo_ByBuffer_041_043,
		inCREDIT_PRINT_Tidmid_ByBuffer_Small,
		inCREDIT_PRINT_Data_ByBuffer_041_043_Small_S,
		inCREDIT_PRINT_Cup_Amount_ByBuffer_Small,
		inCREDIT_PRINT_Amount_ByBuffer_041_043_Small_S,
		inCREDIT_PRINT_Inst_ByBuffer_Small,
		inCREDIT_PRINT_Redeem_ByBuffer_Small,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_041_043_Small_S
	},
        
        /* 信託專用 */
	{
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Tidmid_ByBuffer,
		inCREDIT_PRINT_Data_ByBuffer_Trust,
		_NULL_CH_,
		inCREDIT_PRINT_Amount_ByBuffer_Trust,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_ReceiptEND_ByBuffer_Trust
	},
};

/* 列印總額報表使用 (START) */
TOTAL_REPORT_TABLE_BYBUFFER srTotalReport_ByBuffer[] =
{
	/* 不印ESC結帳條(Ex:Diners) */
	{
		inCREDIT_PRINT_Check_ByBuffer,
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Top_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer,
		inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer,
		inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer,
		_NULL_CH_,
		inCREDIT_PRINT_Total_HG_ByBuffer,
		inCREDIT_PRINT_End_ByBuffer
	},
       
	/* 印ESC結帳條(只有NCCC) */
	{
		inCREDIT_PRINT_Check_ByBuffer,
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Top_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer,
		inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer,
		inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer,
		inCREDIT_PRINT_TotalAmountByOther,
		inCREDIT_PRINT_Total_HG_ByBuffer,
		inCREDIT_PRINT_End_ByBuffer
	},
	
	/* 印ESC結帳條(只有NCCC)(046)總額明細報表金額字變大 */
	{
		inCREDIT_PRINT_Check_ByBuffer,
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Top_ByBuffer_046,
		inCREDIT_PRINT_TotalAmount_ByBuffer_046,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046,
		inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer_046,
		inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer_046,
		inCREDIT_PRINT_TotalAmountByOther_046,
		inCREDIT_PRINT_Total_HG_ByBuffer_046,
		inCREDIT_PRINT_End_ByBuffer_046
	},
	
	/* TAKA */
	{
		inCREDIT_PRINT_Check_ByBuffer,
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Top_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer_TAKA,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_TAKA,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_End_ByBuffer
	},
	
	/* 041 預借現金 */
	{
		inCREDIT_PRINT_Check_ByBuffer,
		inCREDIT_PRINT_Logo_ByBuffer_041_043,
		inCREDIT_PRINT_Top_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer,
		inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer,
		inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer,
		inCREDIT_PRINT_TotalAmountByOther,
		inCREDIT_PRINT_Total_HG_ByBuffer,
		inCREDIT_PRINT_End_ByBuffer
	},
};

/* 列印明細報表使用 (START) */
DETAIL_REPORT_TABLE_BYBUFFER srDetailReport_ByBuffer[] =
{
	/* 一般(ex:Diners) */
	{
		inCREDIT_PRINT_Check_ByBuffer,			/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,			/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer,			/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer,		/* inTotalAmount */
		inCREDIT_PRINT_DetailReportMiddle_ByBuffer,	/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer			/* inReportEnd */
	},
	
	/* NCCC */
	{
		inCREDIT_PRINT_Check_ByBuffer,			/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,			/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer,			/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer,		/* inTotalAmount */
		inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer,/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer			/* inReportEnd */
	},
	
	/* DCC */
	{
		inCREDIT_PRINT_Check_ByBuffer,			/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,			/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer,			/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer,		/* inTotalAmount */
		inCREDIT_PRINT_DCC_DetailReportMiddle_ByBuffer,	/* inMiddle */
		inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer			/* inReportEnd */
	},
       
	/* HG */
	{
		inCREDIT_PRINT_Check_ByBuffer,			/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,			/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer,			/* inReportTop */
		_NULL_CH_,					/* inTotalAmount */
		inCREDIT_PRINT_HG_DetailReportMiddle_ByBuffer,	/* inMiddle */
		inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer			/* inReportEnd */
	},
	
	/* 現行AE、diners已併機，只做NCCC版 */
	/* NCCC(046)昇恆昌明細報表字體加大 */
	{
		inCREDIT_PRINT_Check_ByBuffer,				/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,				/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer_046,			/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer_046,		/* inTotalAmount */
		inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer_046,	/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer_046,		/* inBottom */
		inCREDIT_PRINT_End_ByBuffer_046				/* inReportEnd */
	},
	
	/* DCC(046)昇恆昌明細報表字體加大 */
	{
		inCREDIT_PRINT_Check_ByBuffer,				/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,				/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer_046,			/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer_046,		/* inTotalAmount */
		inCREDIT_PRINT_DCC_DetailReportMiddle_ByBuffer_046,	/* inMiddle */
		inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer_046,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer_046				/* inReportEnd */
	},
       
	/* HG(046)昇恆昌明細報表字體加大 */
	{
		inCREDIT_PRINT_Check_ByBuffer,				/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,				/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer_046,			/* inReportTop */
		_NULL_CH_,						/* inTotalAmount */
		inCREDIT_PRINT_HG_DetailReportMiddle_ByBuffer_046,	/* inMiddle */
		inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer_046,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer_046				/* inReportEnd */
	},
	
	/* TAKA */
	{
		inCREDIT_PRINT_Check_ByBuffer,			/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer,			/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer,			/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer,		/* inTotalAmount */
		inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer,/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer			/* inReportEnd */
	},
	
	/* 041 預借現金 */
	{
		inCREDIT_PRINT_Check_ByBuffer,			/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer_041_043,		/* inReportLogo */
		inCREDIT_PRINT_Top_ByBuffer,			/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer,		/* inTotalAmount */
		inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer,/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer			/* inReportEnd */
	},
};

/* 列印電票總額報表使用 (START) */
TOTAL_REPORT_TABLE_BYBUFFER_ESVC srTotalReport_ByBuffer_ESVC[] =
{
	{
		_NULL_CH_,
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,
		inCREDIT_PRINT_Top_ESVC_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_End_ByBuffer_ESVC
	},
	
	{
		_NULL_CH_,
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,
		inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC_Settle,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_ESVC_Settle,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_End_ByBuffer_ESVC
	},
        
        /* 客製化098 */
        {
		_NULL_CH_,
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,
		inCREDIT_PRINT_Top_ESVC_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer_098_ESVC,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_098_ESVC,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_End_ByBuffer_ESVC
	},
	
	{
		_NULL_CH_,
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,
		inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer,
		inCREDIT_PRINT_TotalAmount_ByBuffer_098_ESVC_Settle,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_098_ESVC_Settle,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_End_ByBuffer_ESVC
	},
	
	/* 客製化046 */
	{
		_NULL_CH_,
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,
		inCREDIT_PRINT_Top_ESVC_ByBuffer_046,
		inCREDIT_PRINT_TotalAmount_ByBuffer_046_ESVC,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046_ESVC,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_End_ByBuffer_ESVC_046
	},
	
	{
		_NULL_CH_,
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,
		inCREDIT_PRINT_Top_ESVC_SETTLE_ByBuffer_046,
		inCREDIT_PRINT_TotalAmount_ByBuffer_046_ESVC_Settle,
		inCREDIT_PRINT_TotalAmountByCard_ByBuffer_046_ESVC_Settle,
		_NULL_CH_,
		_NULL_CH_,
		_NULL_CH_,
		inCREDIT_PRINT_End_ByBuffer_ESVC_046
	},
};
/* 列印電票總額報表使用 (END) */

/* 列印電票明細報表使用 (START) */
DETAIL_REPORT_TABLE_BYBUFFER_ESVC srDetailReport_ByBuffer_ESVC[] =
{
	{
		inCREDIT_PRINT_Check_ByBuffer_ESVC,		/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,		/* inReportLogo */
		inCREDIT_PRINT_Top_ESVC_ByBuffer,		/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer_ESVC,	/* inTotalAmount */
		inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC,/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC,/* inBottom */
		inCREDIT_PRINT_End_ByBuffer_ESVC		/* inReportEnd */
	},
        
	/* 098 */
        {
		inCREDIT_PRINT_Check_ByBuffer_ESVC,		/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,		/* inReportLogo */
		inCREDIT_PRINT_Top_ESVC_ByBuffer,		/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer_098_ESVC,	/* inTotalAmount */
		inCREDIT_PRINT_DetailReportMiddle_ByBuffer_ESVC,/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer_ESVC,/* inBottom */
		inCREDIT_PRINT_End_ByBuffer_ESVC		/* inReportEnd */
	},
	
	/* 046 */
	{
		inCREDIT_PRINT_Check_ByBuffer_ESVC,			/* inReportCheck */
		inCREDIT_PRINT_Logo_ByBuffer_ESVC,			/* inReportLogo */
		inCREDIT_PRINT_Top_ESVC_ByBuffer_046,			/* inReportTop */
		inCREDIT_PRINT_TotalAmount_ByBuffer_046_ESVC,		/* inTotalAmount */
		inCREDIT_PRINT_DetailReportMiddle_ByBuffer_046_ESVC,	/* inMiddle */
		inCREDIT_PRINT_DetailReportBottom_ByBuffer_046_ESVC,	/* inBottom */
		inCREDIT_PRINT_End_ByBuffer_ESVC_046			/* inReportEnd */
	},
};
/* 列印電票明細報表使用 (END) */

TOTAL_REPORT_TABLE_BYBUFFER_TRUST srTotalReport_ByBuffer_TRUST[] =
{
	{
		_NULL_CH_,
		inCREDIT_PRINT_Logo_ByBuffer,
		inCREDIT_PRINT_Top_ByBuffer_TRUST,
		inCREDIT_PRINT_TotalAmount_ByBuffer_TRUST,
		inCREDIT_PRINT_TotalAmountByInstitutionCode_ByBuffer,
		inCREDIT_PRINT_End_ByBuffer_TRUST
	},
};

/*
Function        :inCREDIT_PRINT_Receipt_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印信用卡
*/
int inCREDIT_PRINT_Receipt_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
        int			inPrintIndex = 0, inRetVal;
	char			szTRTFileName[12 + 1];
	char			szShort_Receipt_Mode[2 + 1];
	char			szDebugMsg[100 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszDefaultBit = VS_FALSE;
	unsigned char		uszNotUseShortReceiptBit = VS_FALSE;
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 EDC不印簽單 2018/11/28 下午 3:03 by Russell */
	/* (需求單 - 107276)自助交易標準400做法 EDC不印簽單 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)   ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return (VS_SUCCESS);
	}
	else
	{
		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);

		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
		/* 客製化沒有縮小簽單 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_046_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_046_;
				}

			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_046_;
			}
			/* 昇恆昌無優惠兌換，刪除 */
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_FISC_046_;
			}
			/* Happy GO交易 */
			else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_046_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_046_;
			}
			/* 電票交易 */
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inLoadTDTRec(pobTran->srTRec.inTDTIndex);

				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
					return (VS_SUCCESS);

				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_046_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_046_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_046_;
				}
				else
				{
					return (VS_SUCCESS);
				}
			}
                        else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)/* 信託專用 */
                        {
                                inPrintIndex = _REPORT_INDEX_TRUST_SALE_;
                        }
			/* 信用卡一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_046_;
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_, _CUSTOMER_INDICATOR_SIZE_))
		{
			uszNotUseShortReceiptBit = VS_TRUE;
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				uszDefaultBit = VS_TRUE;
			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				uszDefaultBit = VS_TRUE;
			}
			/* 優惠兌換 */
			else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				uszDefaultBit = VS_TRUE;
			}
			/* 優惠兌換取消 */
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				uszDefaultBit = VS_TRUE;
			}
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_FISC_027_;
			}
			/* Happy GO交易 */
			else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
			{
				uszDefaultBit = VS_TRUE;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_027_;
			}
			/* 電票交易 */
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				uszDefaultBit = VS_TRUE;
			}
                        else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)/* 信託專用 */
                        {
                                inPrintIndex = _REPORT_INDEX_TRUST_SALE_;
                        }
			/* 信用卡一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_027_;
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_061_GRAND_HOTEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			uszNotUseShortReceiptBit = VS_TRUE;
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_061_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_061_;
				}
			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_061_;
			}
                        else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)/* 信託專用 */
                        {
                                inPrintIndex = _REPORT_INDEX_TRUST_SALE_;
                        }
			else
			{
				uszDefaultBit = VS_TRUE;
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
			inGetShort_Receipt_Mode(szShort_Receipt_Mode);
			if (memcmp(szShort_Receipt_Mode, "Y", strlen("Y")) == 0)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_041_043_SMALL_;
			}
			else if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_S_, strlen(_SHORT_RECEIPT_S_)) ||
				 !memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_041_043_SMALL_S_;
			}
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_041_043_;
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
		{
			uszNotUseShortReceiptBit = VS_TRUE;
			if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_005_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_005_;
			}
                        else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)/* 信託專用 */
                        {
                                inPrintIndex = _REPORT_INDEX_TRUST_SALE_;
                        }
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_005_;
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
			inGetShort_Receipt_Mode(szShort_Receipt_Mode);
			if (memcmp(szShort_Receipt_Mode, "Y", strlen("Y")) == 0)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_041_043_SMALL_;
			}
			else if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_S_, strlen(_SHORT_RECEIPT_S_)) ||
				 !memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_041_043_SMALL_S_;
			}
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_041_043_;
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			uszNotUseShortReceiptBit = VS_TRUE;
			uszDefaultBit = VS_TRUE;
		}
		else
		{
			uszDefaultBit = VS_TRUE;
		}
		
		if (uszDefaultBit == VS_TRUE)
		{
			/* 縮小版帳單 */
			/* 客製化不用縮小簽單 */
			memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
			inGetShort_Receipt_Mode(szShort_Receipt_Mode);
			if (memcmp(szShort_Receipt_Mode, "Y", strlen("Y")) == 0	&&
			    uszNotUseShortReceiptBit != VS_TRUE)
			{
				/* 信用卡DCC */
				if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
				{
					/* For Sale */
					if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_SMALL_;
					}
					/* 信用卡DCC Not For Sale */
					else
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_SMALL_;
					}

				}
				/* DCC 當筆轉台幣 */
				else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_SMALL_;
				}
				/* 優惠兌換 */
				else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_LOYALTY_REDEEM_;
				}
				/* 優惠兌換取消 */
				else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_;
				}
				/* 信用卡SmartPay */
				else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_FISC_SMALL_;
				}
				/* Happy GO交易 */
				else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_;
				}
				/* Happy GO混合交易 */
				else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_SMALL_;
				}
				/* 電票交易目前沒有縮小簽單，導回同一個 */
				else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
				{
					inLoadTDTRec(pobTran->srTRec.inTDTIndex);

					if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
					    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
					    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
						return (VS_SUCCESS);

					if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_;
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_;
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_;
					}
					else
					{
						return (VS_SUCCESS);
					}
				}
                                else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)/* 信託專用 */
                                {
                                        inPrintIndex = _REPORT_INDEX_TRUST_SALE_;
                                }
				/* 信用卡一般 */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_SMALL_;;
				}
			}
			else if ((!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_S_, strlen(_SHORT_RECEIPT_S_)) ||
				  !memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))	&&
				 uszNotUseShortReceiptBit != VS_TRUE)
			{
				/* 信用卡DCC */
				if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
				{
					/* For Sale */
					if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_SMALL_S_;
					}
					/* 信用卡DCC Not For Sale */
					else
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_SMALL_S_;
					}

				}
				/* DCC 當筆轉台幣 */
				else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_SMALL_S_;
				}
				/* 優惠兌換 */
				else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_LOYALTY_REDEEM_;
				}
				/* 優惠兌換取消 */
				else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_;
				}
				/* 信用卡SmartPay */
				else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_FISC_SMALL_S_;
				}
				/* Happy GO交易 */
				else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_;
				}
				/* Happy GO混合交易 */
				else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_SMALL_S_;
				}
				/* 電票交易目前沒有縮小簽單，導回同一個 */
				else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
				{
					inLoadTDTRec(pobTran->srTRec.inTDTIndex);

					if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
					    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
					    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
						return (VS_SUCCESS);

					if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_;
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_;
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_;
					}
					else
					{
						return (VS_SUCCESS);
					}
				}
                                else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)/* 信託專用 */
                                {
                                        inPrintIndex = _REPORT_INDEX_TRUST_SALE_;
                                }
				/* 信用卡一般 */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_SMALL_S_;
				}
			}
			else
			{
				/* 信用卡DCC */
				if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
				{
					/* For Sale */
					if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_;
					}
					/* 信用卡DCC Not For Sale */
					else
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_;
					}

				}
				/* DCC 當筆轉台幣 */
				else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_;
				}
				/* 優惠兌換 */
				else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_LOYALTY_REDEEM_;
				}
				/* 優惠兌換取消 */
				else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_;
				}
				/* 信用卡SmartPay */
				else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_FISC_;
				}
				/* Happy GO交易 */
				else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_;
				}
				/* Happy GO混合交易 */
				else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_;
				}
				/* 電票交易 */
				else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
				{
					inLoadTDTRec(pobTran->srTRec.inTDTIndex);

					if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
					    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
					    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
						return (VS_SUCCESS);

					if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_;
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_;
					}
					else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
					{
						inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_;
					}
					else
					{
						return (VS_SUCCESS);
					}
				}
                                else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)/* 信託專用 */
                                {
                                        inPrintIndex = _REPORT_INDEX_TRUST_SALE_;
                                }
				/* 信用卡一般 */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_;
				}
			}
		}/* 客製化判別 End */

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/30 下午 1:30
		 * 昇恆昌客製化，分期紅利要在金額上方 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			do
			{
				inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
				/* 列印LOGO */
				if (srReceiptType_ByBuffer[inPrintIndex].inLogo != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* 列印TID MID */
				if (srReceiptType_ByBuffer[inPrintIndex].inTop != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* 列印DATA */
				if (srReceiptType_ByBuffer[inPrintIndex].inData != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inData(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);


				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					/* 列印CUP金額 */
					if (srReceiptType_ByBuffer[inPrintIndex].inCUPAmount != NULL)
						if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inCUPAmount(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}
				else
				{
					/* 分期資料 */
					if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
					{
						if (srReceiptType_ByBuffer[inPrintIndex].inInstallment != NULL)
							if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inInstallment(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
								return (inRetVal);
					}
					/* 紅利資料 */
					else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
					{
						if (srReceiptType_ByBuffer[inPrintIndex].inRedemption != NULL)
							if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inRedemption(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
								return (inRetVal);
					}
					
					/* 列印金額 */
					if (srReceiptType_ByBuffer[inPrintIndex].inAmount != NULL)
						if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inAmount(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

				/* OTHER資料 */
				if (srReceiptType_ByBuffer[inPrintIndex].inOther != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inOther(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* HappyGo */

				/* LoyaltyRedeem */


				/* 列印簽名欄  & 警語 */
				/* 因為有電子簽名圖檔所以個別處理 */
				if (srReceiptType_ByBuffer[inPrintIndex].inEnd != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
				break;
			}while(1);
		}
		else
		{
			do
			{
				inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
				/* 列印LOGO */
				if (srReceiptType_ByBuffer[inPrintIndex].inLogo != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* 列印TID MID */
				if (srReceiptType_ByBuffer[inPrintIndex].inTop != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* 列印DATA */
				if (srReceiptType_ByBuffer[inPrintIndex].inData != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inData(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);


				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					/* 列印CUP金額 */
					if (srReceiptType_ByBuffer[inPrintIndex].inCUPAmount != NULL)
						if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inCUPAmount(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}
				else
				{
					/* 列印金額 */
					if (srReceiptType_ByBuffer[inPrintIndex].inAmount != NULL)
						if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inAmount(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);

					/* 分期資料 */
					if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
					{
						if (srReceiptType_ByBuffer[inPrintIndex].inInstallment != NULL)
							if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inInstallment(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
								return (inRetVal);
					}
					/* 紅利資料 */
					else if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
					{
						if (srReceiptType_ByBuffer[inPrintIndex].inRedemption != NULL)
							if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inRedemption(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
								return (inRetVal);
					}

				}

				/* OTHER資料 */
				if (srReceiptType_ByBuffer[inPrintIndex].inOther != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inOther(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				/* HappyGo */

				/* LoyaltyRedeem */


				/* 列印簽名欄  & 警語 */
				/* 因為有電子簽名圖檔所以個別處理 */
				if (srReceiptType_ByBuffer[inPrintIndex].inEnd != NULL)
					if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);

				if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
				break;
			}while(1);
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer
Date&Time       :2016/3/7 下午 3:57
Describe        :列印總額帳單
*/
int inCREDIT_PRINT_TotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0;
	char			szDebugMsg[100 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        ACCUM_TOTAL_REC		srAccumRec;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 不列印結帳條 2018/11/28 下午 3:03 by Russell */
	/* (需求單 - 107276)自助交易標準400做法 不列印結帳條 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)   ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_046_NCCC;
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_041_NCCC;
		}
		else
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_NCCC_;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
		{
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		}
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Get record 失敗.");	
			}

			return (VS_ERROR);
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmount != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 有金額才印 */
			if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
			{
				/* 卡別金額總計 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCard != NULL)
					if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);
				/* 有分期記錄才印 */
				if (srAccumRec.lnInstTotalCount > 0)
				{
					/* 分期金額總計 */
					if (srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment != NULL)
						if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

				/* 有紅利記錄才印 */
				if (srAccumRec.lnRedeemTotalCount > 0)
				{
					/* 紅利金額總計 */
					if (srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption != NULL)
						if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

			}

			/* 結束 */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			break;
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer_NCCC
Date&Time       :2017/3/7 下午 5:00
Describe        :列印總額帳單
*/
int inCREDIT_PRINT_TotalReport_ByBuffer_NCCC(TRANSACTION_OBJECT *pobTran)
{
	int			inOrgIndex = -1;
	int			inHGIndex = -1;
	int			inRetVal = 0, inPrintIndex = 0;
	int			inYLen = 0;
	int			inFileLen = 0;
	char			szDebugMsg[100 + 1] = {0};
	char			szFuncEnable[2 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	unsigned char		uszFileName[30 + 1] = {0};
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        ACCUM_TOTAL_REC		srAccumRec;
	HG_ACCUM_TOTAL_REC	srHGAccumRec;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 不列印結帳條 2018/11/28 下午 3:03 by Russell */
	/* (需求單 - 107276)自助交易標準400做法 不列印結帳條 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)   ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return (VS_SUCCESS);
	}
	else
	{

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_046_NCCC;
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_041_NCCC;
		}
		else
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_NCCC_;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Get record 失敗.");	
			}

			return (VS_ERROR);
		}

		/* 開啟HappyGo交易總額檔案 */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHGIndex);
		if (inHGIndex == -1)
		{

		}
		else
		{
			/* 轉成HG */
			inOrgIndex = pobTran->srBRec.inHDTIndex;
			pobTran->srBRec.inHDTIndex = inHGIndex;
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			memset(&srHGAccumRec, 0x00, sizeof(srHGAccumRec));
			inRetVal = inHG_GetRecord(pobTran, &srHGAccumRec);

			/* 轉回來 */
			pobTran->srBRec.inHDTIndex = inOrgIndex;
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			if (inRetVal == VS_NO_RECORD)
				memset(&srHGAccumRec, 0x00, sizeof(srHGAccumRec));
			else if (inRetVal == VS_ERROR)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Get HG record 失敗.");	
				}

				return (VS_ERROR);
			}
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmount != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 有金額才印 */
			if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
			{
				/* 卡別金額總計 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCard != NULL)
					if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);
				/* 有分期記錄才印 */
				if (srAccumRec.lnInstTotalCount > 0)
				{
					/* 分期金額總計 */
					if (srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment != NULL)
						if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByInstallment(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

				/* 有紅利記錄才印 */
				if (srAccumRec.lnRedeemTotalCount > 0)
				{
					/* 紅利金額總計 */
					if (srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption != NULL)
						if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByRedemption(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
							return (inRetVal);
				}

			}

			/* 上傳電子簽單至ESC系統 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmountByOther != NULL)
				srTotalReport_ByBuffer[inPrintIndex].inAmountByOther(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

			/* HappyGo */
			inHG_GetHG_Enable(pobTran->srBRec.inHDTIndex, szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* 印HG總額 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountHG != NULL)
					if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountHG(pobTran, &srHGAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);
			}

			/* 是否印優惠兌換 */
			/* (需求單 - 107367)優惠查核改由ATS判斷 修改為含日期判斷 by Russell 2019/7/2 下午 4:27 */
			/* (需求單-109327)-vx520客製化需求 by Russell 2022/4/22 下午 2:04
			 * 昇恆昌不印優惠兌換，但是電文要照送 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
                         !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				
			}
			else
			{
				inRetVal = inNCCC_Loyalty_ASM_Flag();
				if (inRetVal == VS_SUCCESS)
				{
					inRetVal = inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);
					if (inRetVal != VS_SUCCESS)
					{
						return (inRetVal);
					}
				}
			}

			/* 結束 */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			break;
		}
		
		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
		* 儲存結帳報表的buffer */
		if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->uszEverRich_Settle_RepeintBit = VS_TRUE;
				
				memset(uszBuffer1, 0x00, sizeof(uszBuffer1));
				inCREDIT_PRINT_Dutyfree_Reprint_Report_NCCC_First(pobTran, uszBuffer1, &inYLen);
				inFileLen = PB_CANVAS_X_SIZE * (inYLen + 8);
				memset(uszFileName, 0x00, sizeof(uszFileName));
				inFunc_ComposeFileName(pobTran, (char*)uszFileName, _DUTY_FREE_REPRINT_TOTAL_REPORT_FILE_EXTENSION_, 0);
				inNCCC_Func_Duty_Free_Save_Total_Report_Buffer(uszFileName, uszBuffer1, inFileLen);
				
				pobTran->uszEverRich_Settle_RepeintBit = VS_FALSE;
			}
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer_DCC
Date&Time       :2018/5/20 下午 4:50
Describe        :列印總額帳單
*/
int inCREDIT_PRINT_TotalReport_ByBuffer_DCC(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0;
	int			inYLen = 0;
	int			inFileLen = 0;
	char			szDebugMsg[100 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	unsigned char		uszFileName[30 + 1] = {0};
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        ACCUM_TOTAL_REC		srAccumRec;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 不列印結帳條 2018/11/28 下午 3:03 by Russell */
	/* (需求單 - 107276)自助交易標準400做法 不列印結帳條 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)   ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return (VS_SUCCESS);
	}
	else
	{

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_046_NCCC;
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_041_NCCC;
		}
		else
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_NCCC_;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Get record 失敗.");	
			}

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmount != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 有金額才印 */
			if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
			{
				/* 卡別金額總計 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCard != NULL)
					if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);
			}

			/* 上傳電子簽單至ESC系統 */
			if (srTotalReport_ByBuffer[inPrintIndex].inAmountByOther != NULL)
				srTotalReport_ByBuffer[inPrintIndex].inAmountByOther(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

			/* 結束 */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			
			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			break;
		}
		
		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
		* 儲存結帳報表的buffer */
		if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->uszEverRich_Settle_RepeintBit = VS_TRUE;
				
				memset(uszBuffer1, 0x00, sizeof(uszBuffer1));
				inCREDIT_PRINT_Dutyfree_Reprint_Report_DCC_First(pobTran, uszBuffer1, &inYLen);
				inFileLen = PB_CANVAS_X_SIZE * (inYLen + 8);
				memset(uszFileName, 0x00, sizeof(uszFileName));
				inFunc_ComposeFileName(pobTran, (char*)uszFileName, _DUTY_FREE_REPRINT_TOTAL_REPORT_FILE_EXTENSION_, 0);
				inNCCC_Func_Duty_Free_Save_Total_Report_Buffer(uszFileName, uszBuffer1, inFileLen);
				
				pobTran->uszEverRich_Settle_RepeintBit = VS_FALSE;
			}
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer_HG
Date&Time       :2017/3/7 下午 5:00
Describe        :列印HG總額帳單
*/
int inCREDIT_PRINT_TotalReport_ByBuffer_HG(TRANSACTION_OBJECT *pobTran)
{
	int			inOrgIndex = -1;
	int			inNCCCIndex = -1;
	int			inRetVal = 0, inPrintIndex = 0;
	char			szDebugMsg[100 + 1] = {0};
	char			szFuncEnable[2 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	unsigned char		uszFileName[30 + 1] = {0};
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        ACCUM_TOTAL_REC		srAccumRec;
	HG_ACCUM_TOTAL_REC	srHGAccumRec;

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 不列印結帳條 2018/11/28 下午 3:03 by Russell */
	/* (需求單 - 107276)自助交易標準400做法 不列印結帳條 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)   ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_046_NCCC;
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_041_NCCC;
		}
		else
		{
			inPrintIndex = _TOTAL_REPORT_INDEX_NCCC_;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 開啟交易總的檔案 */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
		if (inNCCCIndex == -1)
		{
			return (VS_ERROR);
		}
		/* 轉成NCCC */
		inOrgIndex = pobTran->srBRec.inHDTIndex;
		pobTran->srBRec.inHDTIndex = inNCCCIndex;
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		/* 轉回來 */
		pobTran->srBRec.inHDTIndex = inOrgIndex;
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Get record 失敗.");	
			}

			return (VS_ERROR);
		}

		/* 開啟HappyGo交易總額檔案 */
		memset(&srHGAccumRec, 0x00, sizeof(srHGAccumRec));
		inRetVal = inHG_GetRecord(pobTran, &srHGAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srHGAccumRec, 0x00, sizeof(srHGAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Get HG record 失敗.");	
			}

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			/* 列印LOGO */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* HappyGo */
			inHG_GetHG_Enable(pobTran->srBRec.inHDTIndex, szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* 印HG總額 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountHG != NULL)
					if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inAmountHG(pobTran, &srHGAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
						return (inRetVal);
			}

			/* 是否印優惠兌換 */
			/* (需求單 - 107367)優惠查核改由ATS判斷 修改為含日期判斷 by Russell 2019/7/2 下午 4:27 */
			/* (需求單-109327)-vx520客製化需求 by Russell 2022/4/22 下午 2:04
			 * 昇恆昌不印優惠兌換，但是電文要照送 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
                         !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{

			}
			else
			{
				inRetVal = inNCCC_Loyalty_ASM_Flag();
				if (inRetVal == VS_SUCCESS)
				{
					inRetVal = inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);
					if (inRetVal != VS_SUCCESS)
					{
						return (inRetVal);
					}
				}
			}

			/* 結束 */
			if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			
			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
			break;
		}
		
		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
		* 儲存結帳報表的buffer */
		if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->uszEverRich_Settle_RepeintBit = VS_TRUE;
				
				inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
				/* 列印LOGO */
				if (srTotalReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
					srTotalReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				/* 列印TID MID */
				if (srTotalReport_ByBuffer[inPrintIndex].inReportTop != NULL)
					srTotalReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				/* 全部金額總計 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmount != NULL)
					srTotalReport_ByBuffer[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

				/* 有金額才印 */
				if (srAccumRec.llTotalSaleAmount != 0L || srAccumRec.llTotalRefundAmount != 0L)
				{
					/* 卡別金額總計 */
					if (srTotalReport_ByBuffer[inPrintIndex].inAmountByCard != NULL)
						srTotalReport_ByBuffer[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				}

				/* 上傳電子簽單至ESC系統 */
				if (srTotalReport_ByBuffer[inPrintIndex].inAmountByOther != NULL)
					srTotalReport_ByBuffer[inPrintIndex].inAmountByOther(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);

				/* 結束 */
				if (srTotalReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
					srTotalReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				
				memset(uszFileName, 0x00, sizeof(uszFileName));
				inFunc_ComposeFileName(pobTran, (char*)uszFileName, _DUTY_FREE_REPRINT_TOTAL_REPORT_FILE_EXTENSION_, 0);
				if (srBhandle1.inYcurrent >= srBhandle1.inYcover)
				{
					inNCCC_Func_Duty_Free_Save_Total_Report_Buffer(uszFileName, uszBuffer1, PB_CANVAS_X_SIZE * (srBhandle1.inYcurrent + 8));
				}
				else
				{
					inNCCC_Func_Duty_Free_Save_Total_Report_Buffer(uszFileName, uszBuffer1, PB_CANVAS_X_SIZE * (srBhandle1.inYcover + 8));
				}
				
				pobTran->uszEverRich_Settle_RepeintBit = VS_FALSE;
			}
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_DetailReport_ByBuffer
Date&Time       :2016/3/7 下午 4:08
Describe        :列印明細帳單
*/
int inCREDIT_PRINT_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0, inRecordCnt = 0;
	char			szTRTFileName[16 + 1];
	char			szDebugMsg[100 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        ACCUM_TOTAL_REC		srAccumRec;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */

		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);
		/* (需求單-109327)-vx520客製化需求 by Russell 2022/4/18 上午 10:44
		    昇恆昌客製化明細加粗 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
                         !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
			{
				inPrintIndex = _DETAIL_REPORT_INDEX_046_DCC_;
			}
			else if (memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
			{
				inPrintIndex = _DETAIL_REPORT_INDEX_046_HG_;
			}
			else
			{
				inPrintIndex = _DETAIL_REPORT_INDEX_046_NCCC_;
			}
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 041不支援HG和DCC */
			inPrintIndex = _DETAIL_REPORT_INDEX_041_NCCC_;
		}
		else
		{
			if (memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
			{
				inPrintIndex = _DETAIL_REPORT_INDEX_DCC_;
			}
			else if (memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
			{
				inPrintIndex = _DETAIL_REPORT_INDEX_HG_;
			}
			else
			{
				inPrintIndex = _DETAIL_REPORT_INDEX_NORMAL_;
			}
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 檢查是否有帳 */
		if (srDetailReport_ByBuffer[inPrintIndex].inReportCheck != NULL)
		{
			if ((inRecordCnt = srDetailReport_ByBuffer[inPrintIndex].inReportCheck(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) == VS_ERROR)
				return (VS_ERROR); /* 表示檔案開啟失敗 */
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			inLogPrintf(AT, "Get record 失敗.");

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			srBhandle1.uszDetailPrint = VS_TRUE;
			
			/* 列印LOGO */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srDetailReport_ByBuffer[inPrintIndex].inTotalAmount != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inTotalAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 明細規格 */
			if (srDetailReport_ByBuffer[inPrintIndex].inMiddle != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inMiddle(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 明細資料 */
			if (srDetailReport_ByBuffer[inPrintIndex].inBottom != NULL)
			{
				inRetVal = srDetailReport_ByBuffer[inPrintIndex].inBottom(pobTran, inRecordCnt, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				if (inRetVal != VS_SUCCESS	&&
				    inRetVal != VS_NO_RECORD)
				{
					return (inRetVal);
				}
			}

			/* 結束 */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
			
			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

			break;
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_DetailReport_ByBuffer
Date&Time       :2016/3/7 下午 4:08
Describe        :列印明細帳單
*/
int inCREDIT_PRINT_NCCC_DetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = 0, inPrintIndex = 0, inRecordCnt = 0;
	char			szDebugMsg[100 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
        ACCUM_TOTAL_REC		srAccumRec;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */

		/* (需求單-109327)-vx520客製化需求 by Russell 2022/4/18 上午 10:44
		    昇恆昌客製化明細加粗 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)		||
                         !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPrintIndex = _DETAIL_REPORT_INDEX_046_NCCC_;
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 041不支援HG和DCC */
			inPrintIndex = _DETAIL_REPORT_INDEX_041_NCCC_;
		}
		else
		{
			inPrintIndex = _DETAIL_REPORT_INDEX_NCCC_;
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 檢查是否有帳 */
		if (srDetailReport_ByBuffer[inPrintIndex].inReportCheck != NULL)
		{
			if ((inRecordCnt = srDetailReport_ByBuffer[inPrintIndex].inReportCheck(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) == VS_ERROR)
				return (VS_ERROR); /* 表示檔案開啟失敗 */
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord(pobTran, &srAccumRec);

		if (inRetVal == VS_NO_RECORD)
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		else if (inRetVal == VS_ERROR)
		{
			inLogPrintf(AT, "Get record 失敗.");

			return (VS_ERROR);
		}

		while (1)
		{
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
			srBhandle1.uszDetailPrint = VS_TRUE;
			
			/* 列印LOGO */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportLogo != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 列印TID MID */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportTop != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 全部金額總計 */
			if (srDetailReport_ByBuffer[inPrintIndex].inTotalAmount != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inTotalAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			/* 明細規格 */
			if (srDetailReport_ByBuffer[inPrintIndex].inMiddle != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inMiddle(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
			/* 明細資料 */
			if (srDetailReport_ByBuffer[inPrintIndex].inBottom != NULL)
			{
				inRetVal = srDetailReport_ByBuffer[inPrintIndex].inBottom(pobTran, inRecordCnt, uszBuffer1, &srFont_Attrib1, &srBhandle1);
				if (inRetVal != VS_SUCCESS &&
				    inRetVal != VS_NO_RECORD)
				{
				       return (inRetVal);
				}
			}

			/* 是否印優惠兌換 */
			/* (需求單 - 107367)優惠查核改由ATS判斷 修改為含日期判斷 by Russell 2019/7/2 下午 4:27 */
			/* (需求單-109327)-vx520客製化需求 by Russell 2022/4/22 下午 2:04
			 * 昇恆昌不印優惠兌換，但是電文要照送 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
                         !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)	||
			 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
			{
				
			}
			else
			{
				inRetVal = inNCCC_Loyalty_ASM_Flag();
				if (inRetVal == VS_SUCCESS)
				{
					inRetVal = inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1);
					if (inRetVal != VS_SUCCESS)
					{
						return (inRetVal);
					}
				}
			}

			/* 結束 */
			if (srDetailReport_ByBuffer[inPrintIndex].inReportEnd != NULL)
				if ((inRetVal = srDetailReport_ByBuffer[inPrintIndex].inReportEnd(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);

			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
			
			if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

			break;
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Schedule_ByBuffer
Date&Time       :2017/2/23 下午 5:16
Describe        :列印重要訊息通知
*/
int inCREDIT_PRINT_Schedule_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inBMPHeight = 0;
	
	if (inPRINT_Buffer_GetHeight((unsigned char*)"IMPORTANT.bmp", &inBMPHeight) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
        /* 列印重要訊息通知 */
        if (inPRINT_Buffer_PutGraphic((unsigned char*)_TMS_SCHEDULE_IMPORTANT_, uszBuffer, srBhandle, inBMPHeight, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_LOGO_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印LOGO
*/
int inCREDIT_PRINT_Logo_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szShort_Receipt_Mode[1 + 1] = {0};
	
        memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
        inGetShort_Receipt_Mode(szShort_Receipt_Mode);
        
	/* 印Slogan 384*180 */
	if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
	{
		if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_UP_, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
	}
	
        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
        {
                /* 印NCC的LOGO */
                if (inPRINT_Buffer_PutIn(_NCCC_TEXT_LOGO_, _FONT_SIZE_SHORT_RECEIPT_U_NCCC_LOGO_TEXT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }
        else
        {
                /* 印NCC的LOGO */
                if (inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }

        /* 印商店的LOGO */
        if (inCREDIT_PRINT_MerchantLogo(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
	{
                return (VS_ERROR);
        }
	
	/* 印商店名稱 */
        if (inCREDIT_PRINT_MerchantName(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_MarchantSlogan
Date&Time       :2016/9/7 下午 5:17
Describe        :用來決定要不要印slogan的
*/
int inCREDIT_PRINT_MarchantSlogan(TRANSACTION_OBJECT *pobTran, int inPrintPosition, unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	int	inDateNow, inDateStart, inDateEnd;
	char	szSloganEnable[2 + 1];
	char	szSloganPrtPosition[1 + 1];
	char	szDateNow[8 + 1], szDateStart[8 + 1], szDateEnd[8 + 1];
	
	/* SloganEnable沒On代表不用印Slogan */
	memset(szSloganEnable, 0x00, sizeof(szSloganEnable));
	inGetPrtSlogan(szSloganEnable);
	if(memcmp(szSloganEnable, "N", 1) == 0)
	{
		return (VS_SUCCESS);
	}
	
	/* 列印位置不對就跳過*/
	memset(szSloganPrtPosition, 0x00, sizeof(szSloganPrtPosition));
	inGetSloganPrtPosition(szSloganPrtPosition);
	if (inPrintPosition != atoi(szSloganPrtPosition))
		return (VS_SUCCESS);
	
	/* 算出目前的日期 */
	memset(szDateNow, 0x00, sizeof(szDateNow));
	inDateNow = atoi(pobTran->srBRec.szDate);
	
	/* 算出起始日期(起始日含此日) */
	memset(szDateStart, 0x00, sizeof(szDateStart));
	inGetSloganStartDate(szDateStart);
	inDateStart = atoi(szDateStart);
	
	/* 商店活動日期未到不用印 */
	if (inDateNow < inDateStart)
		return (VS_SUCCESS);
		
	/* 算出結束日期(結束日不含此日) */
	memset(szDateEnd, 0x00, sizeof(szDateEnd));
	inGetSloganEndDate(szDateEnd);
	inDateEnd = atoi(szDateEnd);

	/* 商店活動日期過期不用印 */
	if (inDateNow >= inDateEnd)
		return (VS_SUCCESS);
	
	if (inPRINT_Buffer_PutGraphic((unsigned char*)_SLOGAN_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inSloganHeight, _APPEND_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_MerchantLogo
Date&Time       :2016/9/7 下午 5:55
Describe        :用來決定要不要印商店LOGO
*/
int inCREDIT_PRINT_MerchantLogo(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	char	szPrtMerchantLogo[1 + 1];
	
	/* PrtMerchantLogo沒On代表不用印商店Logo */
	memset(szPrtMerchantLogo, 0x00, sizeof(szPrtMerchantLogo));
	inGetPrtMerchantLogo(szPrtMerchantLogo);
	if(memcmp(szPrtMerchantLogo, "N", 1) == 0)
	{
		return (VS_SUCCESS);
	}
	
	if (inPRINT_Buffer_PutGraphic((unsigned char*)_MERCHANT_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inMerchantLogoHeight, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_MerchantName
Date&Time       :2016/9/7 下午 5:55
Describe        :用來決定要不要印商店表頭
*/
int inCREDIT_PRINT_MerchantName(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	char	szPrtMerchantName[1 + 1];
	
	/* PrtMerchantName沒On代表不用印商店表頭 */
	memset(szPrtMerchantName, 0x00, sizeof(szPrtMerchantName));
	inGetPrtMerchantName(szPrtMerchantName);
	if(memcmp(szPrtMerchantName, "N", 1) == 0)
	{
		return (VS_SUCCESS);
	}
	
	if (inPRINT_Buffer_PutGraphic((unsigned char*)_NAME_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inTitleNameHeight, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Notice
Date&Time       :2016/9/13 上午 11:40
Describe        :用來決定要不要印商店提示
*/
int inCREDIT_PRINT_Notice(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	char	szPrtNotice[1 + 1];
	
	/* szPrtNotice沒On代表不用印商店提示 */
	memset(szPrtNotice, 0x00, sizeof(szPrtNotice));
	inGetPrtNotice(szPrtNotice);
	if(memcmp(szPrtNotice, "N", 1) == 0)
	{
		return (VS_SUCCESS);
	}
	
	if (inPRINT_Buffer_PutGraphic((unsigned char*)_NOTICE_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inNoticeHeight, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TIDMID_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印TID & MID
*/
int inCREDIT_PRINT_Tidmid_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* Get商店代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);

                /* 列印商店代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "商店代號：%s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* Get端末機代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);

                /* 列印端末機代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "端末機代號：%s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }
        else
        {
                /* 橫式 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);

                /* 列印商店代號 */
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
                sprintf(szPrintBuf, "商店代號 %s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* Get端末機代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);

                /* 列印端末機代號 */
                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
                sprintf(szPrintBuf, "端末機代號 %s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        int     inRetVal;
        char 	szPrintBuf[100 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /*卡別、卡號*/
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szPrintBuf1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */

                /* 城市別(City) */
                inRetVal = inPRINT_Buffer_PutIn("城市別(City)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
                inGetCityName(szPrintBuf1);
                sprintf(szPrintBuf, "%s", szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 卡別 檢查碼 */
		inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 大高卡只能顯示有效期 */
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("有效期(Exp. Date)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼(Check No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                /* 卡別 */
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		/* 檢查碼 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			sprintf(szTemplate1, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
		}
		else
		{
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
		}
                memcpy(szPrintBuf1, szTemplate1, strlen(szTemplate1));
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 卡號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
		}
		else
		{
			if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
				{
					szPrintBuf[i] = 0x2A;
				}

			}
			else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
				 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					{
						szPrintBuf[i] = 0x2A;
					}
				}

			}
		}
		
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
				strcat(szPrintBuf, "(C)");
		}
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			strcat(szPrintBuf, "(Q)");
		}
		else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			strcat(szPrintBuf, "(Q)");
		}
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf, "(W)");
					else
						strcat(szPrintBuf,"(M)");
				}
				else
					strcat(szPrintBuf,"(S)");
			}
		}
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 主機別 & 交易別 */
		inRetVal = inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		inGetHostLabel(szTemplate1);
		sprintf(szPrintBuf1, "%s", szTemplate1);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

		sprintf(szPrintBuf2, "%s", szTemplate1);
		sprintf(szPrintBuf, "%s %s", szPrintBuf1 , szPrintBuf2);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
                
		/* 批次號碼、授權碼 */
		inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("授權碼(Auth Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* Batch Num */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* Auth Code */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memcpy(szPrintBuf1, pobTran->srBRec.szAuthCode, _AUTH_CODE_SIZE_);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 回覆碼 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inRetVal = inPRINT_Buffer_PutIn("回覆碼(Resp. Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, pobTran->srBRec.szRespCode, 2);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

                /* 日期時間 */
                inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* 序號 調閱編號 */
		inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inPRINT_Buffer_PutIn("品群碼(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 原品群碼 */
				inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				char	szPOS_ID[6 + 1] = {0};
				memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
				inGetPOS_ID(szPOS_ID);

				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
				memcpy(&szTemplate1[12], szPOS_ID, 6);
				/* hardcode */
				inRetVal = inPRINT_Buffer_PutIn(szTemplate1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
		/* 取消維持原交易不列印機票資訊 */
                if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
                {

                        inRetVal = inPRINT_Buffer_PutIn("出發地機場(departure)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szFlightTicketPDS0523, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn("目的地機場(arrival)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szFlightTicketPDS0524, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        
                        inRetVal = inPRINT_Buffer_PutIn("航班號碼(Flight No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                        {
                                sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                        }
                        else
                        {
                                sprintf(szTemplate2, "%s", " ");
                        }
                        sprintf(szPrintBuf, "%s", szTemplate2);

                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        		
                        memset(szTemplate1, 0x00, sizeof(szTemplate1));
                        inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);
                        
			if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))	||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))	||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
			{
				inRetVal = inPRINT_Buffer_PutIn("交易類型(transaction category)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn("交易類型(transaction category)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
                        
                }
        
		/* TC */
		/* 驗測開關 */
		memset(szExamBit, 0x00, sizeof(szExamBit));
		inGetExamBit(szExamBit);
		/* Uny交易 */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* 取消或退貨，只印數字交易碼 */
			/* 最小字體 */
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
			    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			/* 正向交易，要印條碼和數字 */
			else
			{
				
				if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
		/* 實體卡交易和掃碼交易互斥 */
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				/* 感應磁條 */
				if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
				    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE		|| 
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
				{
					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				else
				{
					if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
					    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
					{
						if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "TC:%02X%02X%02X%02X%02X%02X%02X%02X",
											pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						/* 商店聯卡號遮掩 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						uszChangeLineBit = VS_FALSE;
						/* AID */
						if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
						{
							if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
							{
								/* CUP晶片要印 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 【需求單-109121】Mastercard交易簽單列印AID需求 by Russell 2020/7/23 下午 4:23 */
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
							else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
							{
								/* 銀聯閃付 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* AE晶片要印出AID */
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
						}

						/* M/C交易列印AP Lable (START) */
						if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* M/C交易列印AP Lable (END) */
						else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						/* 一定要換行 */
						else
						{
							if (uszChangeLineBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, " ");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}

					}
					else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
					{
						/* 商店聯卡號遮掩 */
						/* 持卡人存根也要印 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}
				}
			}
			else
			{
				/* 商店聯卡號遮掩 */
				/* 磁條卡列印交易編號 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
		
		/* 斷行 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_CUP_AMOUNT_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印銀聯AMOUNT
*/
int inCREDIT_PRINT_Cup_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
        char    szPrintBuf[84 + 1] = {0}, szTemplate[42 + 1] = {0};
	long    lnTempTxnAmt = 0;

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	if (inPrinttype_ByBuffer)
        {
		/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
                if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                {
                        lnTempTxnAmt = pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount;

                }
                else
                {
                        lnTempTxnAmt = pobTran->srBRec.lnTxnAmount;
                }
		
                /* 直式 */
                if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
                {
			if (pobTran->srBRec.inCode == _CUP_REFUND_	||
			    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "NT$ %ld", lnTempTxnAmt);
				inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			}
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "NT$ %ld", 0 - lnTempTxnAmt);
				inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			}
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
			if (pobTran->srBRec.inCode == _CUP_REFUND_	||
			    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
				inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			}
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
				inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			}
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
        /* 橫式 */
        /* 負向交易 */
	else if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
                if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                {
                        lnTempTxnAmt = pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount;
                }
                else
                {
                        lnTempTxnAmt = pobTran->srBRec.lnTxnAmount;
                }
		
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
		    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  lnTempTxnAmt);
			inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - lnTempTxnAmt));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
		/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
		if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
		{
			inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠後金額   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠金額　   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("==============================================================================================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("備註欄(Reference)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_RemarksInformation, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("優惠券號(Coupon ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_Coupon, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		if (pobTran->srBRec.inCode == _CUP_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}

		inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "總計(Total) :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
		if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
		{
			inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", 0 - (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
			}
			else
			{
				sprintf(szPrintBuf, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠前金額   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠金額　   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠後金額   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("==============================================================================================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("備註欄(Reference)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_RemarksInformation, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("優惠券號(Coupon ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_Coupon, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 列印銀聯交易提示文字 */
	inPRINT_Buffer_PutGraphic((unsigned char*)_CUP_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inCupLegalHeight, _APPEND_);
	
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_AMOUNT_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* 金額 */
                if(pobTran->srBRec.inCode == _TIP_)
                {
                        /* 金額 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 總計 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf,"總計(Total) :%s", szTemplate);
                }
                else if ((pobTran->srBRec.uszVOIDBit == VS_TRUE	&& 
			 (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_)) ||
			  pobTran->srBRec.inCode == _REFUND_		|| 
			  pobTran->srBRec.inCode == _INST_REFUND_	|| 
			  pobTran->srBRec.inCode == _REDEEM_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
        /* 橫式 */
        /* 負向交易 */
	else if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
		if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				 /* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 金額 */
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "金額(Amount):");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				/* lnTipAmount為0表示非小費 */
				if (pobTran->srBRec.lnTipTxnAmount == 0L)
				{
					/* 小費 */
					inPRINT_Buffer_PutIn("小費(Tips)  :__________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 總計 */
					inPRINT_Buffer_PutIn("總計(Total) :__________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 小費 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "小費(Tips)  :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


					/* 總計 */
					/* 初始化 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				
			}
			
		}
		/* 小費沒開時 */
		else
		{
			 /* 初始化 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			}
			else
			{
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			}
			
			inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
	}
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_INST_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印分期
*/
int inCREDIT_PRINT_Inst_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		
		/* 分期期數 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		inFunc_Amount_Comma(szTemplate1, "" , ' ', _SIGNED_NONE_, 18, _PADDING_LEFT_);
		sprintf(szTemplate, "%s  ", szTemplate1);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "期");
		inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期期數   :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 首期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnTipTxnAmount > 0L)
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount));
		else
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment));
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 19, _PADDING_RIGHT_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "首期金額   :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 每期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentPayment));
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 19, _PADDING_RIGHT_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "每期金額   :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 分期手續費 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentFormalityFee));
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 19, _PADDING_RIGHT_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期手續費 :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* 斷行 */
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 分期警語*/
		inPRINT_Buffer_PutGraphic((unsigned char*)_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inInstHeight, _APPEND_);

		/* 斷行 */
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	}
		
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_REDEEM_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印紅利
*/
int inCREDIT_PRINT_Redeem_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
	
	if (pobTran->srBRec.uszRedeemBit == VS_TRUE)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
		{
			if (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_MINUS_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_MINUS_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", (pobTran->srBRec.lnRedemptionPoints));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if(pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else
		{
			if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_MINUS_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_MINUS_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", (pobTran->srBRec.lnRedemptionPoints));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_)
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 支付金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "支付金額　　 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵金額 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利扣抵點數 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利扣抵點數 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				/* 紅利剩餘點數 */
				if (pobTran->inTransactionCode == _REDEEM_SALE_ || pobTran->srBRec.inCode == _REDEEM_SALE_)
				{
					/* 紅利扣抵才印 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnRedemptionPointsBalance));
					inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
					sprintf(szTemplate, "%s  ",szTemplate1);
					inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "點");
					inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "紅利剩餘點數 :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

			}

		}

		/* 斷行 */
		inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
		
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印結尾
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szTemplate[42 + 1]= {0};
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
                        /* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			/* 要簽名 */
			else
			{
				/* 藉由TRT_FileName比對來組出bmp的檔名 */
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				
				memset(szSignature, 0x00, sizeof(szSignature));
				/* 因為用invoice所以不用inFunc_ComposeFileName */
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
				memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
				sprintf(szSignaturePath, "./fs_data/%s", szSignature);
				/* 圖檔存在、有在signpad簽名、且非重印（重印不出簽名）（目前簽名狀態存不了Batch，先把&& pobTran->srBRec.inSignStatus == _SIGN_SIGNED_此條件拿掉） */
				if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS && pobTran->inRunOperationID != _OPERATION_REPRINT_)
				{
					/* 電子簽名 */
					inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
				}
				/* 手簽 */
				else
				{
					/* a space 2 line */
					for (i = 0; i < 2; i++)
					{
						inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					
				}	
			}
                        
                        inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
                }
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
                else
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("　　　　　　　　　 持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　  Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
                /* 列印警示語 */
                inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
		
		/* Print Slogan */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);
			
			if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
			    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
			    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
			{
				if (inCREDIT_PRINT_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}
			
		}
		
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_RewardAdvertisement_Check
Date&Time       :2019/10/7 下午 12:01
Describe        :是否要印出優惠資訊
*/
int inCREDIT_PRINT_RewardAdvertisement_Check(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inLastInv = 0;
	char	szTemplate[16 + 1] = {0};
	
	/* 如果重印時已過優惠期間就不印 */
	if (inNCCC_Loyalty_ASM_Flag() != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "重印時已過優惠期間不印優惠資訊");
		}
		return (VS_ERROR);
	}
	
	/* 重印帳單只有前一筆(該Host最後一筆簽單)才印優惠資訊 */
	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
        	inRetVal = inBATCH_Find_Last_Txn_Inv_SQLite(pobTran, szTemplate);
		
		if (inRetVal == VS_SUCCESS)
		{
			inLastInv = atoi(szTemplate);
		}
		else
		{
			inLastInv = _BATCH_LAST_RECORD_;
		}
		
		/* invoice Num減一即最後一筆 */
		if ((pobTran->srBRec.lnOrgInvNum == inLastInv)	||
		    (pobTran->srBRec.lnOrgInvNum == _BATCH_LAST_RECORD_))
		{
			inLogPrintf(AT, "是最後一筆要印優惠資訊");
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "不是最後一筆不重印優惠資訊");
			}
			return (VS_ERROR);
		}
	}
	
	/* 國隆說小費不印原交易優惠 */
	if (pobTran->srBRec.inCode == _TIP_	||
	    pobTran->srBRec.inOrgCode == _TIP_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "小費不印優惠資訊");
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_RewardAdvertisement
Date&Time       :2017/2/14 上午 10:31
Describe        :列印優惠資訊
*/
int inCREDIT_PRINT_RewardAdvertisement(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{	
	int			i = 0;
	char			szRewardL1L2L3[1650 + 1];	/* L5最常可到1618Bytes 取1650 */
	char			szPrintBuf[300 + 1];
	char			szDemoMode[2 + 1] = {0};
	unsigned long		ulBufferSize;
	LOYALTY_L1L2L3_OBJECT	srLoyaltyData;
		
	/* 確認是否要印出優惠資訊，不印就跳過 */
	if (inCREDIT_PRINT_RewardAdvertisement_Check(pobTran) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "不印優惠資訊");
		}
		
		return (VS_SUCCESS);
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE)
		{
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE && 
			    pobTran->srBRec.uszDCCTransBit != VS_TRUE && 
			    pobTran->srBRec.uszNCCCDCCRateBit != VS_TRUE &&
			    (pobTran->srBRec.inOrgCode == _SALE_ ||
			     pobTran->srBRec.inOrgCode == _PRE_COMP_ ||
			     pobTran->srBRec.inOrgCode == _INST_SALE_ ||
			     pobTran->srBRec.inOrgCode == _REDEEM_SALE_ ||
			     pobTran->srBRec.lnHGTransactionType == _HG_REWARD_ ||
			     pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ ||
			     pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_))
			{
				/* 優惠活動標題 */
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "原優惠已取消");
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				return (VS_SUCCESS);
			}
			else if (pobTran->srBRec.uszDCCTransBit != VS_TRUE && 
			         pobTran->srBRec.uszNCCCDCCRateBit != VS_TRUE &&
				 (pobTran->srBRec.inCode == _SALE_ ||
				  pobTran->srBRec.inOrgCode == _SALE_ ||
			          pobTran->srBRec.inCode == _PRE_COMP_ ||
			          pobTran->srBRec.inCode == _INST_SALE_ ||
			          pobTran->srBRec.inCode == _REDEEM_SALE_ ||
			          pobTran->srBRec.lnHGTransactionType == _HG_REWARD_ ||
			          pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_ ||
			          pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_))
			{
				/* 優惠活動標題 */
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);				
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "聯合百貨兌換券");
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);			
			
			
				/* 一維條碼 */
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "11345879239875934754");
				inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			
				/* 優惠活動內容 */
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "恭喜您符合XX銀行回饋門檻！請於2099/12/31前至XXX實體門市出示此兌換券，即可兌換精美餐具組一份。");
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				return (VS_SUCCESS);
			}
			else
				return (VS_SUCCESS);
			
		}
	
		return (VS_SUCCESS);
	}
	
	ulBufferSize = sizeof(szRewardL1L2L3);
	memset(szRewardL1L2L3, 0x00, sizeof(szRewardL1L2L3));
	if (inNCCC_Loyalty_Read_Reward_Data(&ulBufferSize, szRewardL1L2L3, _REWARD_FILE_NAME_) != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("inNCCC_Loyalty_Read_Reward_Data Error");
		
		return (VS_ERROR);
	}
	
	/* 放到結構中 */
	if (inNCCC_Loyalty_Data_Format(&srLoyaltyData, (int)ulBufferSize, szRewardL1L2L3) != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("inNCCC_Loyalty_Data_Format Error");
		
		return (VS_ERROR);
	}
	
	if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE)
	{
		/* 列印優惠或廣告資訊之個數 */
		if (atoi(srLoyaltyData.srL1DATA.szPrintRewardNum) > 0)
		{
			/* 取消交易不印 */
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				for (i = 0; i < atoi(srLoyaltyData.srL1DATA.szPrintRewardNum); i++)
				{
					
					/* 優惠活動之標題 */
					if (memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleFlag, "1", 1) == 0 && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleLen) > 0))
					{
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleContent, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleLen));
						inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					}

					/* 一維條碼(一) */
					if ((memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Flag, "1", 1) == 0) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len) > 0))
					{
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Content, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len));
						inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
						inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					}

					/* 一維條碼(二) */
					if (!memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Flag, "1", 1) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len) > 0))
					{
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Content, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len));
						inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
						inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					}
	
					/* 優惠活動內容 */
					if (!memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentFlag, "1", 1) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentLen) > 0))
					{
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContent, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentLen));
						inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
	
					/* 是否印補充內容 */
					if (memcmp(srLoyaltyData.srL1DATA.szSupInfFlag, "1", strlen("1")) == 0)
					{
						/* 此為L1判別法，L5另外判斷，若不須送的狀況下面會continue掉 */
						if (memcmp(srLoyaltyData.srL1DATA.szSupInfLocation, "1", strlen("1")) == 0)
						{
							if (i == 0)
							{
								/* 只送第一個 */
							}
							else
							{
								continue;
							}
						}
						else if (memcmp(srLoyaltyData.srL1DATA.szSupInfLocation, "2", strlen("2")) == 0)
						{
							if (i == 1)
							{
								/* 只送第二個 */
							}
							else
							{
								continue;
							}
						}
						else if (memcmp(srLoyaltyData.srL1DATA.szSupInfLocation, "3", strlen("3")) == 0)
						{
							if (i == 0 || i == 0)
							{
								/* 一二都要送 */
							}
							else
							{
								continue;
							}
						}
						else
						{
							/* 4表示為其他狀況，仍須請端末機列印補充資訊內容 */
							continue;
						}
					
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.szSupInfContent, atoi(srLoyaltyData.srL1DATA.szSupInfLen));
						inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					else
					{
						/* 不需印補充內容，跳過 */
					}
					
				}
				
			}
		}
		else
		{
			
		}
		
		/* 是否印補充內容 */
		if (memcmp(srLoyaltyData.srL1DATA.szSupInfFlag, "1", strlen("1")) == 0)
		{
			/* szSupInfLocation為4在這邊印 */
			if (memcmp(srLoyaltyData.srL1DATA.szSupInfLocation, "4", strlen("4")) == 0)
			{
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memcpy(szPrintBuf, srLoyaltyData.srL1DATA.szSupInfContent, atoi(srLoyaltyData.srL1DATA.szSupInfLen));
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
		}
		else
		{
			/* 不需印補充內容，跳過 */
		}
		
	}
	else if (pobTran->srBRec.uszRewardL2Bit == VS_TRUE)
	{
		/*
			列印(優惠活動||累計訊息)及廣告資訊之個數。
			‘0’=表示無(優惠活動||累計訊息)及廣告資訊
			‘1’=表示僅有一個(累計訊息)或廣告資訊
			‘2’=表示有(優惠活動||累計訊息)及廣告資訊
			註1. 本欄位值=’0’，則僅回傳至優惠序號。
			註2. 取消(含沖銷取消)須上傳原交易。
			註3. 本欄位值=’1’是指僅有一個，但仍需判斷第一段與第二段電文。
		*/
		
		
		/* 是否列印優惠活動之標題。‘0’=不列印，’1’=列印
			註：若無(優惠活動||累計訊息)，則本欄位預設為”0”，
			以下 (優惠活動||累計訊息)相關欄位補零或空白，不需計算檢查碼。
		*/
		
		/* 列印優惠或廣告資訊之個數 */
		if (atoi(srLoyaltyData.srL2DATA.szPrintRewardNum) > 0)
		{
			/* 取消交易不印 */
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				/* (優惠活動||累計訊息)之標題 */
				if (memcmp(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szTitleFlag, "1", 1) == 0 && (atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szTitleLen) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szTitleContent, atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szTitleLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}

				/* 一維條碼(一) */
				if ((memcmp(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code1Flag, "1", 1) == 0) && (atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code1Len) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code1Content, atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code1Len));
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}

				/* 一維條碼(二) */
				if (!memcmp(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code2Flag, "1", 1) && (atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code2Len) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code2Content, atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code2Len));
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}

				/* (優惠活動||累計訊息)內容 */
				if (!memcmp(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szContentFlag, "1", 1) && (atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szContentLen) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szContent, atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szContentLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

				/* 是否印補充內容 */
				if (memcmp(srLoyaltyData.srL2DATA.szSupInfFlag, "1", strlen("1")) == 0)
				{
					/*
					 * 補充資訊內容列印位置。(各優惠活動、優惠累計訊息或廣告資訊最後一行左靠)(若不列印則補零)
					‘1’=僅優惠活動或優惠累計訊息須列印
					‘2’=僅廣告資訊須列印
					‘3’=兩個都須列印
					‘4’=表示為其他狀況，仍須請端末機列印補充資訊內容
					*/
					if (memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "1", strlen("1")) == 0 || 
					    memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "3", strlen("3")) == 0)
					{

						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL2DATA.szSupInfContent, atoi(srLoyaltyData.srL2DATA.szSupInfLen));
						inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}

				}

				/* 廣告資訊 */
				/*
					an 1	是否列印廣告資訊之標題。‘0’=不列印，’1’=列印
					註：若無廣告資訊，則本欄位預設為”0”，以下廣告資訊相關欄位補零或空白，不需計算檢查碼。
				*/

				/* (優惠活動||累計訊息)之標題 */
				if (memcmp(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADTitleFlag, "1", 1) == 0 && (atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADTitleLen) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AD_DATA.szADTitleContent, atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADTitleLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}


				/* 廣告資訊之內容 */
				if (!memcmp(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADFlag, "1", 1) && (atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADLen) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AD_DATA.szADContent, atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

				/* 是否列印URL資訊之QR Code */
				if (!memcmp(srLoyaltyData.srL2DATA.srL2_AD_DATA.szQRCodeFlag, "1", 1) && (atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szQRCodeLen) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AD_DATA.szQRCodeContent, atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szQRCodeLen));
					/* 列印QR CODE */
					inPRINT_Buffer_QRcode(szPrintBuf, uszBuffer, srBhandle, _PRINT_DEFINE_X_03_);
				}
				
				/* 是否印補充內容 */
				if (memcmp(srLoyaltyData.srL2DATA.szSupInfFlag, "1", strlen("1")) == 0)
				{
					/*
					 * 補充資訊內容列印位置。(各優惠活動、優惠累計訊息或廣告資訊最後一行左靠)(若不列印則補零)
					‘1’=僅優惠活動或優惠累計訊息須列印
					‘2’=僅廣告資訊須列印
					‘3’=兩個都須列印
					‘4’=表示為其他狀況，仍須請端末機列印補充資訊內容
					*/
					if (memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "2", strlen("2")) == 0 || 
					    memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "3", strlen("3")) == 0)
					{

						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL2DATA.szSupInfContent, atoi(srLoyaltyData.srL2DATA.szSupInfLen));
						inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}

				}
			}
		}
		else
		{
			
		}
		
		/* 如果是4其他狀況，則要印置中 */
		/* 是否印補充內容 */
		if (memcmp(srLoyaltyData.srL2DATA.szSupInfFlag, "1", strlen("1")) == 0)
		{
			/*
			 * 補充資訊內容列印位置。(各優惠活動、優惠累計訊息或廣告資訊最後一行左靠)(若不列印則補零)
			‘1’=僅優惠活動或優惠累計訊息須列印
			‘2’=僅廣告資訊須列印
			‘3’=兩個都須列印
			‘4’=表示為其他狀況，仍須請端末機列印補充資訊內容
			*/
			if (memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "4", strlen("4")) == 0)
			{
			
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memcpy(szPrintBuf, srLoyaltyData.srL2DATA.szSupInfContent, atoi(srLoyaltyData.srL2DATA.szSupInfLen));
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			
		}
		
	}
	/* 目前L5與L1相同，直接套用L1的 僅差在補充資訊列印位置 */
	else if (pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
	{
		/* 列印優惠或廣告資訊之個數 */
		if (atoi(srLoyaltyData.srL1DATA.szPrintRewardNum) > 0)
		{
			/* 取消交易不印 */
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				for (i = 0; i < atoi(srLoyaltyData.srL1DATA.szPrintRewardNum); i++)
				{
					
					/* 優惠活動之標題 */
					if (memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleFlag, "1", 1) == 0 && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleLen) > 0))
					{
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleContent, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleLen));
						inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					}

					/* 一維條碼(一) */
					if ((memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Flag, "1", 1) == 0) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len) > 0))
					{
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Content, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len));
						inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
						inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					}

					/* 一維條碼(二) */
					if (!memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Flag, "1", 1) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len) > 0))
					{
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Content, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len));
						inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
						inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
						inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					}
	
					/* 優惠活動內容 */
					if (!memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentFlag, "1", 1) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentLen) > 0))
					{
						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContent, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentLen));
						inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
	
					/* 是否印補充內容 */
					if (memcmp(srLoyaltyData.srL1DATA.szSupInfFlag, "1", strlen("1")) == 0)
					{
						/* 256向右位移7位等於1，以此類推 */
						if ((srLoyaltyData.srL1DATA.szSupInfLocation[0] >> ( 7 - i ) & 1) == 1)
						{
							
						}
						else
						{
							/* 不印跳過 */
							continue;
						}

						inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						memcpy(szPrintBuf, srLoyaltyData.srL1DATA.szSupInfContent, atoi(srLoyaltyData.srL1DATA.szSupInfLen));
						inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					else
					{
						/* 不需印補充內容，跳過 */
					}
					
				}
				
			}
			
		}
		else
		{
			
		}
		
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_MPAS_RewardAdvertisement
Date&Time       :2019/10/7 上午 11:46
Describe        :【需求單-107226】小額收銀機連線版-支援優惠平台 :
			小額特店刷卡機開啟刷卡優惠平台功能，收銀機交易時，依ATS(MPAS)主機回覆優惠資訊內容 :
			有優惠資訊 : 列印簽單與優惠資訊。
			無優惠資訊 : 比照現行做法。
*/
int inCREDIT_PRINT_MPAS_RewardAdvertisement(TRANSACTION_OBJECT *pobTran)
{
	int			i = 0;
	int			inPrintIndex = 0, inRetVal;
	char			szShort_Receipt_Mode[2 + 1];
	char			szDebugMsg[100 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	
	/* 確認是否要印出優惠資訊，不印就跳過 */
	if (inCREDIT_PRINT_RewardAdvertisement_Check(pobTran) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "不印優惠資訊");
		}
		
		return (VS_SUCCESS);
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 EDC不印簽單 2018/11/28 下午 3:03 by Russell */
	/* (需求單 - 107276)自助交易標準400做法 EDC不印簽單 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)   ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return (VS_SUCCESS);
	}
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/4/22 下午 2:27 */
		/* 客製化沒有縮小簽單 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 信用卡DCC */
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		{
			/* For Sale */
			if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_046_;
			}
			/* 信用卡DCC Not For Sale */
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_046_;
			}

		}
		/* DCC 當筆轉台幣 */
		else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_046_;
		}
		/* 昇恆昌無優惠兌換，刪除 */
		/* 信用卡SmartPay */
		else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_FISC_046_;
		}
		/* Happy GO交易 */
		else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_046_;
		}
		/* Happy GO混合交易 */
		else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_046_;
		}
		/* 電票交易 */
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			inLoadTDTRec(pobTran->srTRec.inTDTIndex);

			if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
			    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
			    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
				return (VS_SUCCESS);

			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_046_;
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_046_;
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_046_;
			}
			else
			{
				return (VS_SUCCESS);
			}
		}
		/* 信用卡一般 */
		else
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_046_;
		}
	}
	/* 客製化沒有縮小簽單 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_, _CUSTOMER_INDICATOR_SIZE_))
	{
		/* 信用卡DCC */
		if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
		{
			/* For Sale */
			if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_;
			}
			/* 信用卡DCC Not For Sale */
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_;
			}

		}
		/* DCC 當筆轉台幣 */
		else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_;
		}
		/* 優惠兌換 */
		else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_LOYALTY_REDEEM_;
		}
		/* 優惠兌換取消 */
		else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_;
		}
		/* 信用卡SmartPay */
		else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_FISC_027_;
		}
		/* Happy GO交易 */
		else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_;
		}
		/* Happy GO混合交易 */
		else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_027_;
		}
		/* 電票交易 */
		else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
		{
			inLoadTDTRec(pobTran->srTRec.inTDTIndex);

			if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
			    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
			    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
				return (VS_SUCCESS);

			if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_;
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_;
			}
			else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_;
			}
			else
			{
				return (VS_SUCCESS);
			}
		}
		/* 信用卡一般 */
		else
		{
			inPrintIndex = _REPORT_INDEX_NORMAL_027_;
		}
	}
	else
	{
		/* 縮小版帳單 */
		memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
		inGetShort_Receipt_Mode(szShort_Receipt_Mode);
		if (memcmp(szShort_Receipt_Mode, "Y", strlen("Y")) == 0)
		{
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_SMALL_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_SMALL_;
				}

			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_SMALL_;
			}
			/* 優惠兌換 */
			else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_LOYALTY_REDEEM_;
			}
			/* 優惠兌換取消 */
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_;
			}
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_FISC_SMALL_;
			}
			/* Happy GO交易 */
			else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_SMALL_;
			}
			/* 電票交易目前沒有縮小簽單，導回同一個 */
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inLoadTDTRec(pobTran->srTRec.inTDTIndex);
				
				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
					return (VS_SUCCESS);

				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_;
				}
				else
				{
					return (VS_SUCCESS);
				}
			}
			/* 信用卡一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_SMALL_;;
			}
		}
		else if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_S_, strlen(_SHORT_RECEIPT_S_)) ||
			 !memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
		{
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_SMALL_S_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_SMALL_S_;
				}

			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_SMALL_S_;
			}
			/* 優惠兌換 */
			else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_LOYALTY_REDEEM_;
			}
			/* 優惠兌換取消 */
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_;
			}
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_FISC_SMALL_S_;
			}
			/* Happy GO交易 */
			else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_SMALL_S_;
			}
			/* 電票交易目前沒有縮小簽單，導回同一個 */
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inLoadTDTRec(pobTran->srTRec.inTDTIndex);

				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
					return (VS_SUCCESS);

				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_;
				}
				else
				{
					return (VS_SUCCESS);
				}
			}
			/* 信用卡一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_SMALL_S_;
			}
		}
		else
		{
			/* 信用卡DCC */
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				/* For Sale */
				if ((pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_FOR_SALE_;
				}
				/* 信用卡DCC Not For Sale */
				else
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_DCC_NOT_FOR_SALE_;
				}

			}
			/* DCC 當筆轉台幣 */
			else if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE && (pobTran->srBRec.inCode == _SALE_ || pobTran->srBRec.inCode == _PRE_COMP_))
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_DCC_CHANGE_TWD_;
			}
			/* 優惠兌換 */
			else if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_LOYALTY_REDEEM_;
			}
			/* 優惠兌換取消 */
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_VOID_LOYALTY_REDEEM_;
			}
			/* 信用卡SmartPay */
			else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_FISC_;
			}
			/* Happy GO交易 */
			else if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_SINGLE_;
			}
			/* Happy GO混合交易 */
			else if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_HG_MULTIPLE_;
			}
			/* 電票交易 */
			else if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				inLoadTDTRec(pobTran->srTRec.inTDTIndex);
				
				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
					return (VS_SUCCESS);

				if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_IPASS_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ECC_;
				}
				else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
				{
					inPrintIndex = _REPORT_INDEX_NORMAL_ESVC_ICASH_;
				}
				else
				{
					return (VS_SUCCESS);
				}
			}
			/* 信用卡一般 */
			else
			{
				inPrintIndex = _REPORT_INDEX_NORMAL_;
			}
		}
	}

	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
		inLogPrintf(AT, szDebugMsg);
	}

	while (1)
	{
		inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
		/* 列印LOGO */
		if (srReceiptType_ByBuffer[inPrintIndex].inLogo != NULL)
			if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 列印TID MID */
		if (srReceiptType_ByBuffer[inPrintIndex].inTop != NULL)
			if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 要讓優惠兌換列印優惠資訊，並出現"兌換成功"字樣 */
		if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_	||
		    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* 列印DATA */
			if (srReceiptType_ByBuffer[inPrintIndex].inData != NULL)
				if ((inRetVal = srReceiptType_ByBuffer[inPrintIndex].inData(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
					return (inRetVal);
		}
		else
		{
			if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
			{
				if (inCREDIT_PRINT_ESVC_RewardAdvertisement(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				if (inCREDIT_PRINT_RewardAdvertisement(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
					return (VS_ERROR);
			}
		}

		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);
		break;
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ESVC_RewardAdvertisement
Date&Time       :2020/5/5 下午 5:53
Describe        :列印優惠資訊
*/
int inCREDIT_PRINT_ESVC_RewardAdvertisement(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{	
	int			i = 0;
	char			szRewardL1L2L3[1650 + 1];	/* L5最常可到1618Bytes 取1650 */
	char			szPrintBuf[300 + 1];
	char			szDemoMode[2 + 1] = {0};
	unsigned long		ulBufferSize;
	LOYALTY_L1L2L3_OBJECT	srLoyaltyData;
		
	/* 確認是否要印出優惠資訊，不印就跳過 */
	if (inCREDIT_PRINT_RewardAdvertisement_Check(pobTran) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "不印優惠資訊");
		}
		
		return (VS_SUCCESS);
	}

	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
		    pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
		    pobTran->srTRec.uszRewardL5Bit == VS_TRUE)
		{
			if (pobTran->srTRec.lnTxnAmount >= 100 && pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
			{
				/* 優惠活動標題 */
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);				
				
				/* 優惠活動之標題內容。(端末機列印時需自動置中，左靠右補空白) */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "聯合餐飲小菜兌換券");
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				
				/* 一維條碼內容。(右靠左補零) */
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "11345879239875934754");
				inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

				/* 優惠活動內容。(右靠左補零) */
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "恭喜您符合一卡通優惠門檻！請於29991231前至聯合餐飲門市出示兌換券，即可兌換小菜一份。");
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				return (VS_SUCCESS);
			}
			else
				return (VS_SUCCESS);
			
		}
	
		return (VS_SUCCESS);
	}
	
	ulBufferSize = sizeof(szRewardL1L2L3);
	memset(szRewardL1L2L3, 0x00, sizeof(szRewardL1L2L3));
	if (inNCCC_Loyalty_Read_Reward_Data(&ulBufferSize, szRewardL1L2L3, _REWARD_ESVC_FILE_NAME_) != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("inNCCC_Loyalty_Read_Reward_Data Error");
		
		return (VS_ERROR);
	}
	
	/* 放到結構中 */
	if (inNCCC_Loyalty_Data_Format(&srLoyaltyData, (int)ulBufferSize, szRewardL1L2L3) != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("inNCCC_Loyalty_Data_Format Error");
		
		return (VS_ERROR);
	}
	
	if (pobTran->srTRec.uszRewardL1Bit == VS_TRUE)
	{
		/* 列印優惠或廣告資訊之個數 */
		if (atoi(srLoyaltyData.srL1DATA.szPrintRewardNum) > 0)
		{
			for (i = 0; i < atoi(srLoyaltyData.srL1DATA.szPrintRewardNum); i++)
			{

				/* 優惠活動之標題 */
				if (memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleFlag, "1", 1) == 0 && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleLen) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleContent, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}

				/* 一維條碼(一) */
				if ((memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Flag, "1", 1) == 0) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Content, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len));
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}

				/* 一維條碼(二) */
				if (!memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Flag, "1", 1) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Content, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len));
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}

				/* 優惠活動內容 */
				if (!memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentFlag, "1", 1) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentLen) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContent, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

				/* 是否印補充內容 */
				if (memcmp(srLoyaltyData.srL1DATA.szSupInfFlag, "1", strlen("1")) == 0)
				{
					/* 此為L1判別法，L5另外判斷，若不須送的狀況下面會continue掉 */
					if (memcmp(srLoyaltyData.srL1DATA.szSupInfLocation, "1", strlen("1")) == 0)
					{
						if (i == 0)
						{
							/* 只送第一個 */
						}
						else
						{
							continue;
						}
					}
					else if (memcmp(srLoyaltyData.srL1DATA.szSupInfLocation, "2", strlen("2")) == 0)
					{
						if (i == 1)
						{
							/* 只送第二個 */
						}
						else
						{
							continue;
						}
					}
					else if (memcmp(srLoyaltyData.srL1DATA.szSupInfLocation, "3", strlen("3")) == 0)
					{
						if (i == 0 || i == 0)
						{
							/* 一二都要送 */
						}
						else
						{
							continue;
						}
					}
					else
					{
						/* 4表示為其他狀況，仍須請端末機列印補充資訊內容 */
						continue;
					}

					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.szSupInfContent, atoi(srLoyaltyData.srL1DATA.szSupInfLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 不需印補充內容，跳過 */
				}
			}
		}
		else
		{
			
		}
		
		/* 是否印補充內容 */
		if (memcmp(srLoyaltyData.srL1DATA.szSupInfFlag, "1", strlen("1")) == 0)
		{
			/* szSupInfLocation為4在這邊印 */
			if (memcmp(srLoyaltyData.srL1DATA.szSupInfLocation, "4", strlen("4")) == 0)
			{
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memcpy(szPrintBuf, srLoyaltyData.srL1DATA.szSupInfContent, atoi(srLoyaltyData.srL1DATA.szSupInfLen));
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
		}
		else
		{
			/* 不需印補充內容，跳過 */
		}
		
	}
	else if (pobTran->srTRec.uszRewardL2Bit == VS_TRUE)
	{
		/*
			列印(優惠活動||累計訊息)及廣告資訊之個數。
			‘0’=表示無(優惠活動||累計訊息)及廣告資訊
			‘1’=表示僅有一個(累計訊息)或廣告資訊
			‘2’=表示有(優惠活動||累計訊息)及廣告資訊
			註1. 本欄位值=’0’，則僅回傳至優惠序號。
			註2. 取消(含沖銷取消)須上傳原交易。
			註3. 本欄位值=’1’是指僅有一個，但仍需判斷第一段與第二段電文。
		*/
		
		
		/* 是否列印優惠活動之標題。‘0’=不列印，’1’=列印
			註：若無(優惠活動||累計訊息)，則本欄位預設為”0”，
			以下 (優惠活動||累計訊息)相關欄位補零或空白，不需計算檢查碼。
		*/
		
		/* (優惠活動||累計訊息)之標題 */
		if (memcmp(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szTitleFlag, "1", 1) == 0 && (atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szTitleLen) > 0))
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szTitleContent, atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szTitleLen));
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
		/* 一維條碼(一) */
		if ((memcmp(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code1Flag, "1", 1) == 0) && (atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code1Len) > 0))
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code1Content, atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code1Len));
			inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}

		/* 一維條碼(二) */
		if (!memcmp(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code2Flag, "1", 1) && (atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code2Len) > 0))
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code2Content, atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.sz39Code2Len));
			inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}

		/* (優惠活動||累計訊息)內容 */
		if (!memcmp(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szContentFlag, "1", 1) && (atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szContentLen) > 0))
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szContent, atoi(srLoyaltyData.srL2DATA.srL2_AWARD_DATA.szContentLen));
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 是否印補充內容 */
		if (memcmp(srLoyaltyData.srL2DATA.szSupInfFlag, "1", strlen("1")) == 0)
		{
			/*
			 * 補充資訊內容列印位置。(各優惠活動、優惠累計訊息或廣告資訊最後一行左靠)(若不列印則補零)
			‘1’=僅優惠活動或優惠累計訊息須列印
			‘2’=僅廣告資訊須列印
			‘3’=兩個都須列印
			‘4’=表示為其他狀況，仍須請端末機列印補充資訊內容
			*/
			if (memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "1", strlen("1")) == 0 || 
			    memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "3", strlen("3")) == 0)
			{
			
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memcpy(szPrintBuf, srLoyaltyData.srL2DATA.szSupInfContent, atoi(srLoyaltyData.srL2DATA.szSupInfLen));
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
		}
		
		/* 廣告資訊 */
		/*
			an 1	是否列印廣告資訊之標題。‘0’=不列印，’1’=列印
			註：若無廣告資訊，則本欄位預設為”0”，以下廣告資訊相關欄位補零或空白，不需計算檢查碼。
		*/

		/* (優惠活動||累計訊息)之標題 */
		if (memcmp(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADTitleFlag, "1", 1) == 0 && (atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADTitleLen) > 0))
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AD_DATA.szADTitleContent, atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADTitleLen));
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}

		
		/* 廣告資訊之內容 */
		if (!memcmp(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADFlag, "1", 1) && (atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADLen) > 0))
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AD_DATA.szADContent, atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szADLen));
			inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
		/* 是否列印URL資訊之QR Code */
		if (!memcmp(srLoyaltyData.srL2DATA.srL2_AD_DATA.szQRCodeFlag, "1", 1) && (atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szQRCodeLen) > 0))
		{
			inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memcpy(szPrintBuf, srLoyaltyData.srL2DATA.srL2_AD_DATA.szQRCodeContent, atoi(srLoyaltyData.srL2DATA.srL2_AD_DATA.szQRCodeLen));
			/* 列印QR CODE */
			inPRINT_Buffer_QRcode(szPrintBuf, uszBuffer, srBhandle, _PRINT_DEFINE_X_03_);
		}
		
		/* 是否印補充內容 */
		if (memcmp(srLoyaltyData.srL2DATA.szSupInfFlag, "1", strlen("1")) == 0)
		{
			/*
			 * 補充資訊內容列印位置。(各優惠活動、優惠累計訊息或廣告資訊最後一行左靠)(若不列印則補零)
			‘1’=僅優惠活動或優惠累計訊息須列印
			‘2’=僅廣告資訊須列印
			‘3’=兩個都須列印
			‘4’=表示為其他狀況，仍須請端末機列印補充資訊內容
			*/
			if (memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "2", strlen("2")) == 0 || 
			    memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "3", strlen("3")) == 0)
			{
			
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memcpy(szPrintBuf, srLoyaltyData.srL2DATA.szSupInfContent, atoi(srLoyaltyData.srL2DATA.szSupInfLen));
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			
		}
		
		
		/* 如果是4其他狀況，則要印置中 */
		/* 是否印補充內容 */
		if (memcmp(srLoyaltyData.srL2DATA.szSupInfFlag, "1", strlen("1")) == 0)
		{
			/*
			 * 補充資訊內容列印位置。(各優惠活動、優惠累計訊息或廣告資訊最後一行左靠)(若不列印則補零)
			‘1’=僅優惠活動或優惠累計訊息須列印
			‘2’=僅廣告資訊須列印
			‘3’=兩個都須列印
			‘4’=表示為其他狀況，仍須請端末機列印補充資訊內容
			*/
			if (memcmp(srLoyaltyData.srL2DATA.szSupInfLocation, "4", strlen("4")) == 0)
			{
			
				inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memcpy(szPrintBuf, srLoyaltyData.srL2DATA.szSupInfContent, atoi(srLoyaltyData.srL2DATA.szSupInfLen));
				inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			}
			
		}
		
	}
	/* 目前L5與L1相同，直接套用L1的 僅差在補充資訊列印位置 */
	else if (pobTran->srTRec.uszRewardL5Bit == VS_TRUE)
	{
		/* 列印優惠或廣告資訊之個數 */
		if (atoi(srLoyaltyData.srL1DATA.szPrintRewardNum) > 0)
		{
			for (i = 0; i < atoi(srLoyaltyData.srL1DATA.szPrintRewardNum); i++)
			{

				/* 優惠活動之標題 */
				if (memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleFlag, "1", 1) == 0 && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleLen) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleContent, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szTitleLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}

				/* 一維條碼(一) */
				if ((memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Flag, "1", 1) == 0) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Content, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code1Len));
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}

				/* 一維條碼(二) */
				if (!memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Flag, "1", 1) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Content, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].sz39Code2Len));
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
					inPRINT_Buffer_PutIn("", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				}

				/* 優惠活動內容 */
				if (!memcmp(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentFlag, "1", 1) && (atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentLen) > 0))
				{
					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContent, atoi(srLoyaltyData.srL1DATA.srL1_AWARD_DATA[i].szContentLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}

				/* 是否印補充內容 */
				if (memcmp(srLoyaltyData.srL1DATA.szSupInfFlag, "1", strlen("1")) == 0)
				{
					/* 256向右位移7位等於1，以此類推 */
					if ((srLoyaltyData.srL1DATA.szSupInfLocation[0] >> ( 7 - i ) & 1) == 1)
					{

					}
					else
					{
						/* 不印跳過 */
						continue;
					}

					inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memcpy(szPrintBuf, srLoyaltyData.srL1DATA.szSupInfContent, atoi(srLoyaltyData.srL1DATA.szSupInfLen));
					inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 不需印補充內容，跳過 */
				}

			}
		}
		else
		{
			
		}
		
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TIDMID_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:24
Describe        :列印TID & MID
*/
int inCREDIT_PRINT_Tidmid_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* Get商店代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);

                /* 列印商店代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "商店代號：%s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* Get端末機代號 */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);

                /* 列印端末機代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "端末機代號：%s", szTemplate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }
        else
        {
                /* 橫式 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                inGetMerchantID(szPrintBuf);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 列印商店代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "商店代號");
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* Get端末機代號 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                inGetTerminalID(szPrintBuf);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 列印端末機代號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "端末機代號");
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:24
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[42 + 1], szTemplate2[84 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /*卡別、卡號*/
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 【需求單 - 106349】自有品牌判斷需求 */
		/* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		
		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* Uny交易不受TMS開關限制，且獲取卡號時已遮掩 */
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
			{
				szPrintBuf1[i] = 0x2A;
			}
			
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}
		}
                
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}
		else if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			strcat(szPrintBuf1, "(Q)");
		}
		else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
		{
			strcat(szPrintBuf1, "(Q)");
		}
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf1, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}
			
		}
		
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 城市 主機 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		
		/* 城市 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetCityName(szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "城市", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 主機 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_DiscardSpace(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "主機");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* 授權碼 檢查碼 */
		/* 前半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "授權碼", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 32, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 後半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			sprintf(szTemplate1, "%.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
		}
		else
		{
			/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (pobTran->srBRec.uszTakaTransBit == VS_TRUE)
		{
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
			sprintf(szPrintBuf2, "%s", "有效期");
		}
		else
		{
			memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
			sprintf(szPrintBuf2, "%s", "檢查碼");
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 櫃號 回覆碼 */
		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PADDING_RIGHT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		else
		{
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* 後半段(銀聯交易才印回覆碼) */
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%s", pobTran->srBRec.szRespCode);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);

				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
				sprintf(szPrintBuf2, "%s", "回覆碼");
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szPrintBuf2);

				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
                if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
		{

                        inRetVal = inPRINT_Buffer_PutIn("出發地機場(departure)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szFlightTicketPDS0523, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn("目的地機場(arrival)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szFlightTicketPDS0524, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        				
                        inRetVal = inPRINT_Buffer_PutIn("航班號碼(Flight No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        
                        
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                        {
                                sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                        }
                        else
                        {
                                sprintf(szTemplate2, "%s", " ");
                        }
                        sprintf(szPrintBuf, "%s", szTemplate2);
                        
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        				
                        memset(szTemplate1, 0x00, sizeof(szTemplate1));
			inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);

			if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
			{
				inRetVal = inPRINT_Buffer_PutIn("交易類型(transaction category)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn("交易類型(transaction category)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
                }
                
		/* TC */
		memset(szExamBit, 0x00, sizeof(szExamBit));
		inGetExamBit(szExamBit);
		/* Uny交易 */
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/8 下午 1:55 */
		if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
		{
			/* 取消或退貨，只印數字交易碼 */
			/* 最小字體 */
			if (pobTran->srBRec.uszVOIDBit == VS_TRUE ||
			    pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);
				
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			/* 正向交易，要印條碼和數字 */
			else
			{
				
				if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼(退貨請掃描) %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					/* 一維條碼 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szUnyTransCode);
					inPRINT_Buffer_Barcode(szPrintBuf, uszBuffer, srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_DEFAULT_);
				
					/* 數字 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易碼 %s", pobTran->srBRec.szUnyTransCode);

					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
		/* 實體卡交易和掃碼交易互斥 */
		else
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
			{
				/* 感應磁條 */
				if (!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3)				|| 
				    !memcmp(pobTran->srBRec.szAuthCode, "JCB", 3)				||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE	|| 
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_64_NEWJSPEEDY_MSD		||
				     pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
				{
					/* 商店聯卡號遮掩 */
					memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
					inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
					if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
					    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
					    strlen(pobTran->srBRec.szTxnNo) > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				else 
				{
					if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
					    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
					{
						if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "TC:%02X%02X%02X%02X%02X%02X%02X%02X",
											pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
											pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						/* 商店聯卡號遮掩 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}

						uszChangeLineBit = VS_FALSE;
						/* AID */
						if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
						{
							if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
							{
								/* CUP晶片要印 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
								else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
							else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
							{
								/* 銀聯閃付 */
								if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* AE晶片要印出AID */
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
									 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
									pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
								/* 驗測要印AP Label 和 AID */
								else if (szExamBit[0] == '1')
								{
									memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
									sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
									inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
									if (inRetVal != VS_SUCCESS)
										return (VS_ERROR);
									uszChangeLineBit = VS_TRUE;
								}
							}
						}

						/* M/C交易列印AP Lable (START) */
						if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* M/C交易列印AP Lable (END) */
						else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
						{
							if (pobTran->srEMVRec.in50_APLabelLen > 0)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
							else
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, "AP Label:");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
							inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						/* 一定要換行 */
						else
						{
							if (uszChangeLineBit == VS_TRUE)
							{
								memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
								sprintf(szPrintBuf, " ");
								inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
								if (inRetVal != VS_SUCCESS)
									return (VS_ERROR);
							}
						}

					}
					else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
					{
						/* 商店聯卡號遮掩 */
						/* 持卡人存根也要印 */
						memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
						inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
						if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
						    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
						    strlen(pobTran->srBRec.szTxnNo) > 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}
				}

			}
			else
			{
				/* 商店聯卡號遮掩 */
				/* 磁條卡列印交易編號 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
		}
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Cup_Amount_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:24
Describe        :列印銀聯AMOUNT
*/
int inCREDIT_PRINT_Cup_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
        char    szPrintBuf[84 + 1] = {0}, szTemplate[42 + 1] = {0};
	long    lnTempTxnAmt = 0;

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
		/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
                if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                {
                        lnTempTxnAmt = pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount;
                }
                else
                {
                        lnTempTxnAmt = pobTran->srBRec.lnTxnAmount;
                }
		
                /* 直式 */
                /* 金額 */
                if(pobTran->srBRec.inCode == _TIP_)
                {
                        /* 金額 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 總計 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf,"總計(Total) :%s", szTemplate);
                }
                else if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
                {
			if (pobTran->srBRec.inCode == _REFUND_		|| 
			    pobTran->srBRec.inCode == _INST_REFUND_	|| 
			    pobTran->srBRec.inCode == _REDEEM_REFUND_	||
			    pobTran->srBRec.inCode == _CUP_REFUND_	|| 
			    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "NT$ %ld", lnTempTxnAmt);
				inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			}
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "NT$ %ld", 0 - lnTempTxnAmt);
				inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			}
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
			if (pobTran->srBRec.inCode == _REFUND_		|| 
			    pobTran->srBRec.inCode == _INST_REFUND_	|| 
			    pobTran->srBRec.inCode == _REDEEM_REFUND_	||
			    pobTran->srBRec.inCode == _CUP_REFUND_	|| 
			    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
				inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			}
			else
			{
				/* 初始化 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
				inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

				/* 把前面的字串和數字結合起來 */
				sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
			}
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
        /* 橫式 */
        /* 負向交易 */
	else if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
                if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
                {
                        lnTempTxnAmt = pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount;

                }
                else
                {
                        lnTempTxnAmt = pobTran->srBRec.lnTxnAmount;
                }
		
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
		/* 因為是取消，所以要用orgcode判斷 */
                if (pobTran->srBRec.inOrgCode == _REFUND_		|| 
		    pobTran->srBRec.inOrgCode == _INST_REFUND_		|| 
		    pobTran->srBRec.inOrgCode == _REDEEM_REFUND_	||
		    pobTran->srBRec.inOrgCode == _CUP_REFUND_		|| 
		    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
		{
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  lnTempTxnAmt);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			
                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - lnTempTxnAmt));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

                /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
		if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
		{
			inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠後金額   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠金額　   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("==============================================================================================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("備註欄(Reference)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_RemarksInformation, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("優惠券號(Coupon ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_Coupon, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	/* 正向交易 */
	else
	{
		/* 總計 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.inCode == _REFUND_		|| 
		    pobTran->srBRec.inCode == _INST_REFUND_	|| 
		    pobTran->srBRec.inCode == _REDEEM_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_REFUND_	|| 
		    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}
		inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "總計(Total) :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
                /* (【需求單-113200】CASV3標準版支援銀聯優計劃專案 by Russell 2024/8/27 上午 11:03 */
		if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
		{
			inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", 0 - (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
			}
			else
			{
				sprintf(szPrintBuf, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠前金額   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanPreferentialAmount);
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠金額　   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (pobTran->srBRec.inOrgCode == _CUP_REFUND_		||
			    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)
			{
				sprintf(szPrintBuf, "%ld", 0 - pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			else
			{
				sprintf(szPrintBuf, "%ld", pobTran->srBRec.lnCUPUPlanDiscountedAmount);
			}
			inFunc_Amount_Comma(szPrintBuf, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			inPRINT_Buffer_PutIn("優惠後金額   : ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("==============================================================================================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("備註欄(Reference)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_RemarksInformation, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("優惠券號(Coupon ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn(pobTran->srBRec.szUPlan_Coupon, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 列印銀聯交易提示文字 */
	inPRINT_Buffer_PutGraphic((unsigned char*)_CUP_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inCupLegalHeight, _APPEND_);
	
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:23
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                /* 金額 */
                if(pobTran->srBRec.inCode == _TIP_)
                {
                        /* 金額 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 小費 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTipTxnAmount);
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "小費(Tips)  :%s", szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


                        /* 總計 */
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
                        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf,"總計(Total) :%s", szTemplate);
                }
                else if ((pobTran->srBRec.uszVOIDBit == VS_TRUE	&& 
			 (pobTran->srBRec.inOrgCode != _REFUND_ && pobTran->srBRec.inOrgCode != _INST_REFUND_ && pobTran->srBRec.inOrgCode != _REDEEM_REFUND_)) ||
			  pobTran->srBRec.inCode == _REFUND_	 || 
			  pobTran->srBRec.inCode == _INST_REFUND_|| 
			  pobTran->srBRec.inCode == _REDEEM_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", 0 - pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else if (pobTran->srBRec.inCode == _ADJUST_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnAdjustTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                else
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
                        sprintf(szTemplate, "NT$ %ld", pobTran->srBRec.lnTxnAmount);
                        inFunc_PAD_ASCII(szTemplate , szTemplate, ' ' , 14, _PADDING_LEFT_ );

                        /* 把前面的字串和數字結合起來 */
                        sprintf(szPrintBuf, "金額(Amount):%s", szTemplate);
                }
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                
		for (i = 0; i < 2; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
        }
        /* 橫式 */
        /* 負向交易 */
	else if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _REFUND_ || pobTran->srBRec.inOrgCode == _INST_REFUND_ || pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
                {
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			
                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

	}
	/* 正向交易 */
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTransFunc(szTemplate);
		if (szTemplate[6] == 'Y')	/* 檢查是否有開小費 */
		{
			/* 退貨金額為負數 */
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 預授不會有小費，所以拉出來 */
			else if (pobTran->srBRec.inCode == _PRE_AUTH_ || pobTran->srBRec.inCode == _PRE_COMP_)
			{
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "總計(Total) :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				/* 金額 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate);
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", "金額(Amount):");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				/* lnTipAmount為0表示非小費 */
				if (pobTran->srBRec.lnTipTxnAmount == 0L)
				{
					/* 小費 */
					inPRINT_Buffer_PutIn("小費(Tips)  :______________________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 總計 */
					inPRINT_Buffer_PutIn("總計(Total) :______________________________________", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				else
				{
					/* 小費 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTipTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "小費(Tips)  :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


					/* 總計 */
					/* 將NT$ ＋數字塞到szTemplate中來inpad */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
					inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", szTemplate);
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					/* 把前面的字串和數字結合起來 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", "總計(Total) :");
					inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				
			}
			
		}
		/* 小費沒開時 */
		else
		{
			/* 總計 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (pobTran->srBRec.inCode == _REFUND_ || pobTran->srBRec.inCode == _INST_REFUND_ || pobTran->srBRec.inCode == _REDEEM_REFUND_)
			{
				sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			}
			else
			{
				sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			}
			inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
			
	}
	
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Inst_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:23
Describe        :列印分期
*/
int inCREDIT_PRINT_Inst_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE)
	{
		/* 分期期數 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%02ld", pobTran->srBRec.lnInstallmentPeriod);
		inFunc_Amount_Comma(szTemplate1, "" , ' ', _SIGNED_NONE_, 18, _PADDING_LEFT_);
		sprintf(szTemplate, "%s  ", szTemplate1);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "期");
		inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
		
		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期期數   :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 首期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.lnTipTxnAmount > 0L)
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentDownPayment + pobTran->srBRec.lnTipTxnAmount));
		else
			sprintf(szTemplate, "%ld", pobTran->srBRec.lnInstallmentDownPayment);
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 19, _PADDING_RIGHT_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "首期金額   :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 每期金額 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentPayment));
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 19, _PADDING_RIGHT_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "每期金額   :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 分期手續費 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld", (pobTran->srBRec.lnInstallmentFormalityFee));
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 19, _PADDING_RIGHT_);
		inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

		/* 把前面的字串和數字結合起來 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "分期手續費 :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 分期警語*/
	inPRINT_Buffer_PutGraphic((unsigned char*)_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inInstHeight, _APPEND_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Redeem_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:23
Describe        :列印紅利
*/
int inCREDIT_PRINT_Redeem_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
	
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	if (pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		if (pobTran->srBRec.inOrgCode == _REDEEM_SALE_ || pobTran->srBRec.inOrgCode == _REDEEM_ADJUST_)
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_MINUS_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_MINUS_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "-%ld", (pobTran->srBRec.lnRedemptionPoints));
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
			sprintf(szTemplate, "%s  ",szTemplate1);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "點");
			inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if(pobTran->srBRec.inOrgCode == _REDEEM_REFUND_)
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
			sprintf(szTemplate, "%s  ",szTemplate1);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "點");
			inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			
			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		if (pobTran->srBRec.inCode == _REDEEM_REFUND_)
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnRedemptionPaidCreditAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_MINUS_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount)));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_MINUS_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "-%ld", (pobTran->srBRec.lnRedemptionPoints));
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
			sprintf(szTemplate, "%s  ",szTemplate1);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "點");
			inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
					
			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (pobTran->srBRec.inCode == _REDEEM_ADJUST_)
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
			sprintf(szTemplate, "%s  ",szTemplate1);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "點");
			inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
					
			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			/* 支付金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", pobTran->srBRec.lnRedemptionPaidCreditAmount + pobTran->srBRec.lnTipTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "支付金額　　 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵金額 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld", (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnRedemptionPaidCreditAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵金額 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利扣抵點數 */
			/* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnRedemptionPoints);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
			sprintf(szTemplate, "%s  ",szTemplate1);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "點");
			inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			
			/* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "紅利扣抵點數 :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 紅利剩餘點數 */
			if (pobTran->inTransactionCode == _REDEEM_SALE_ || pobTran->srBRec.inCode == _REDEEM_SALE_)
			{
				/* 紅利扣抵才印 */
				/* 將NT$ ＋數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnRedemptionPointsBalance));
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 15, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s  ",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

				memset(szTemplate, 0x00, sizeof(szTemplate));
				sprintf(szTemplate, "點");
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "紅利剩餘點數 :");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

		}

	}
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_Small
Date&Time       :2016/3/17 上午 10:23
Describe        :列印結尾
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}
				}
			}
			
                        /* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("X:      免簽名       ", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 要簽名 */
			else
			{
				/* 藉由TRT_FileName比對來組出bmp的檔名 */
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				
				memset(szSignature, 0x00, sizeof(szSignature));
				/* 因為用invoice所以不用inFunc_ComposeFileName */
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
				memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
				sprintf(szSignaturePath, "./fs_data/%s", szSignature);
				/* 有在signpad簽名*/
				if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS)
				{
					/* 電子簽名 */
					inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
				}
				/* 手簽 */
				else
				{
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				
				inPRINT_Buffer_PutIn("X:", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
                        
			inPRINT_Buffer_PutIn("------------------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			/* 持卡人姓名 */
			inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免簽名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
                else
                {
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免簽名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
                        inPRINT_Buffer_PutIn("　　　　　持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
                /* 列印警示語 */
                inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
		
		/* Print Slogan */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);
			
			if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
			    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
			    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
			{
				if (inCREDIT_PRINT_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_Small_SmartPay
Date&Time       :2016/3/17 上午 10:24
Describe        :列印DATA
*/
int inCREDIT_PRINT_FISC_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];;
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */

                /*卡別、卡號*/
                sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "卡別" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		
		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
			{
				szPrintBuf1[i] = 0x2A;
			}
			
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}
		}
		
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf1, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}
			
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}
		
		sprintf(szPrintBuf, "%s", szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 發卡行代碼 主機 */
		/* 發卡行代碼 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szFiscIssuerID[0], 3);
		sprintf(szPrintBuf, "%s %s", "發卡行代碼", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 31, _PADDING_RIGHT_);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 主機*/
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "主機", szTemplate1);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調單編號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		strcpy(szTemplate1, pobTran->srBRec.szFiscRRN);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s" , "調單編號", szTemplate1);
		
		/* 合併 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s",  szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PADDING_RIGHT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* 商店聯卡號遮掩 */
		memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
		inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
		    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
		    strlen(pobTran->srBRec.szTxnNo) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_FISC_Amount_ByBuffer_Small
Date&Time       :2017/5/16 下午 4:56
Describe        :列印FISC AMOUNT
*/
int inCREDIT_PRINT_FISC_Amount_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        /* 橫式 */
        /* 負向交易 */
	if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _FISC_REFUND_)
                {
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			
                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

	}
	/* 正向交易 */
	else
	{
		/* 總計 */
		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (pobTran->srBRec.inCode == _FISC_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}
		inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "總計(Total) :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);	
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_Small_DCC
Date&Time       :2016/3/17 上午 10:24
Describe        :列印DATA
*/
int inCREDIT_PRINT_DCC_Data_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
        int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[84 + 1], szPrintBuf2[84 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
        memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */

                /*卡別、卡號*/
                sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */
		/* "卡號 卡別" */
		/* "卡號" */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, "卡號", ' ', 31, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別值 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 20, _PADDING_LEFT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡別 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "卡別");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 卡號值 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		
		strcpy(szPrintBuf1, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
			{
				szPrintBuf1[i] = 0x2A;
			}
			
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf1) - 4); i ++)
					szPrintBuf1[i] = 0x2A;
			}
			
		}
                
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf1,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf1, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf1, "(W)");
					else
						strcat(szPrintBuf1,"(M)");
				}
				else
					strcat(szPrintBuf1,"(S)");
			}
			
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf1, "(W)");
			else
				strcat(szPrintBuf1, "(C)");
		}
		
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_WIDTH_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 交易別 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s", "交易");
		
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", szTemplate1);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%s %s", szPrintBuf1, szPrintBuf2);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 城市 主機 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		
		/* 城市 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetCityName(szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s %s", "城市", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 主機 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetHostLabel(szTemplate1);
		inFunc_DiscardSpace(szTemplate1);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "主機");
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 日期時間 批號 */
		/* 日期時間 */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szTemplate1, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		sprintf(szPrintBuf1, "%s %s", "日期/時間", szTemplate1);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%03ld", pobTran->srBRec.lnBatchNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 批號 */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "批號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* 授權碼 檢查碼 */
		/* 前半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szAuthCode[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "授權碼", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 32, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* 後半段 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
		}
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 9, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "檢查碼");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 序號 調閱號 */
		/* 序號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memcpy(szTemplate1, &pobTran->srBRec.szRefNo[0], 12);
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%s %s", "序號", szTemplate1);
		inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 29, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		
		/* 調閱號值 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		sprintf(szTemplate1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 8, _PADDING_LEFT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate1);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* "調閱號" */
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
		sprintf(szPrintBuf2, "%s", "調閱號");
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf2);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_02_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, &pobTran->srBRec.szStoreID[0], 23);
			memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
			sprintf(szPrintBuf1, "%s %s", "櫃號", szTemplate1);
			inFunc_PAD_ASCII(szPrintBuf1, szPrintBuf1, ' ', 33, _PADDING_RIGHT_);
			
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szPrintBuf1);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* TC */
		memset(szExamBit, 0x00, sizeof(szExamBit));
		inGetExamBit(szExamBit);
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "TC:%02X%02X%02X%02X%02X%02X%02X%02X",
									pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				
				uszChangeLineBit = VS_FALSE;
				/* AID */
				if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
				{
					if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
					{
						if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
						else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
							pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
					}
					else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
					{
						/* AE晶片要印出AID */
						if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
						    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
							 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
							pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
					}
				}
				
				/* M/C交易列印AP Lable (START) */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
				{
					if (pobTran->srEMVRec.in50_APLabelLen > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:");
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				/* M/C交易列印AP Lable (END) */
				else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
				{
					if (pobTran->srEMVRec.in50_APLabelLen > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:");
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				/* 驗測要印AP Label 和 AID */
				else if (szExamBit[0] == '1')
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
					inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				/* 一定要換行 */
				else
				{
					if (uszChangeLineBit == VS_TRUE)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, " ");
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}

			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 商店聯卡號遮掩 */
				/* 持卡人存根也要印 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
			
		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
		 		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		 		sprintf(szPrintBuf, "交易編號 %s", pobTran->srBRec.szTxnNo);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s", "產品代碼", pobTran->srBRec.szProductCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
                /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
                if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
		{
                        inRetVal = inPRINT_Buffer_PutIn("出發地機場(departure)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szFlightTicketPDS0523, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn("目的地機場(arrival)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szFlightTicketPDS0524, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        				
                        inRetVal = inPRINT_Buffer_PutIn("航班號碼(Flight No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        
                        
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                        {
                                sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                        }
                        else
                        {
                                sprintf(szTemplate2, "%s", " ");
                        }
                        sprintf(szPrintBuf, "%s", szTemplate2);
                        
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        				
                        memset(szTemplate1, 0x00, sizeof(szTemplate1));
			inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);

			if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
			{
				inRetVal = inPRINT_Buffer_PutIn("交易類型(transaction category)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn("交易類型(transaction category)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
                }
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer_Small
Date&Time       :2017/5/18 下午 1:32
Describe        :列印AMOUNT
*/
int inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
	int	inIRDU = 0;
        char    szPrintBuf[84 + 1] = {0}, szPrintBuf1[42 + 1] = {0}, szTemplate[42 + 1] = {0}, szTemplate1[42 + 1] = {0}, szTemplate2[42 + 1] = {0};
	char	szIRDU[12 + 1] = {0};			/* Inverted Rate Display Unit */
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTemplate1, 0x00, sizeof(szTemplate2));
	memset(szTemplate1, 0x00, sizeof(szTemplate2));
	
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("         Please select          ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn(" Transaction currency as below  ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 外幣     1.  [ X ]  EUR 8.18 上需空一行 */
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 外幣 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, "", szPrintBuf1);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "1.【X】%s %s", pobTran->srBRec.szDCC_FCAC, szPrintBuf1);						/* Foreign currcncy Alphabetic Code */	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 匯率 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "Exchange Rate:");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	
	memset(szExchangeAmt, 0x00, sizeof(szExchangeAmt));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 轉換費率(Currency Conversion Fee) */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%s", "轉換費率(Currency Conversion Fee)");
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s", szPrintBuf1);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* XX.XX %*/
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %%", szPrintBuf1);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* (Currency Conversion Fee on the exchange rate over a wholesale rate.) */
	/* 前半段 */
	inRetVal = inPRINT_Buffer_PutIn("(Currency Conversion Fee on the exchange ", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 後半段 */
	inRetVal = inPRINT_Buffer_PutIn("rate over a wholesale rate.)", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 台幣     2.  [    ]  NTD 264  上需空一行 by Russell 2020/10/29 下午 4:32 */
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 台幣 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTxnAmount);
	inFunc_Amount_Comma_DCC(szPrintBuf1, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, "0", "", szPrintBuf1);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 轉台幣跑另外一隻 */
	sprintf(szPrintBuf, "2.【 】%s %s", "NTD", szPrintBuf1);									/* Foreign currcncy Alphabetic Code */

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 有開小費要多印 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTransFunc(szTemplate);
	/* 檢查是否有開小費 */
	/* 預先授權完成必定不印小費 */
	if (szTemplate[6] == 'Y'	&&
	    pobTran->srBRec.inCode == _SALE_)	
	{
		/* 小費     小費（Tips）： EUR  _______________ 上需空一行 by Russell 2020/10/29 下午 3:20 */
		inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	
		/* 商店聯上的小費跟總計不需列印外幣 by Russell 2020/10/29 下午 3:19 */
		if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 小費 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "小費(Tips)   : %s______________________________________", "");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 總計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "總計(Total)  : %s______________________________________", "");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			/* 小費 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "小費(Tips)   : %s______________________________________", pobTran->srBRec.szDCC_FCAC);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 總計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "總計(Total)  : %s______________________________________", pobTran->srBRec.szDCC_FCAC);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		/* 不開小費不印下面這一段 */
	}
		
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer_Small
Date&Time       :2017/5/17 上午 11:04
Describe        :DCC 一段式 非SALE的其他交易
*/
int inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
	int	inIRDU = 0;
	char	szIRDU[12 + 1] = {0};		/* Inverted Rate Display Unit */
	char	szOutputAmt[50 + 1] = {0};
	char	szPrintBuf[84 + 1] = {0}, szPrintBuf1[42 + 1] = {0}, szTemplate[42 + 1] = {0};
	long	lnTipTotalAmt = 0l;
	
	/* 台幣(Local Amour)  上需空一行 by Russell 2020/10/29 下午 4:32 */
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 台幣 */
	inRetVal = inPRINT_Buffer_PutIn("台幣(Local Amount)", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	if (pobTran->srBRec.inCode == _TIP_)
	{
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf1, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "Amount:NTD %s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf1, "%ld", 0 - pobTran->srBRec.lnTipTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTipTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf1, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "Tips  :NTD %s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf1, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf1, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "Total :NTD %s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf1, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf1, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "Total :NTD %s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 斷行 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	
	if (pobTran->srBRec.inCode == _TIP_)
	{
		/* 金額 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		inPRINT_Buffer_PutIn("金額(Amount) :", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 小費 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		inPRINT_Buffer_PutIn("小費(Tips)   :", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 總計 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		inPRINT_Buffer_PutIn("總計(Total)  :", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("Total amount of Transaction Currency", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		/* 初始化 */
		lnTipTotalAmt = 0;
		memset(szTemplate, 0x00, sizeof(szTemplate));
		lnTipTotalAmt += atol(pobTran->srBRec.szDCC_FCA);
		lnTipTotalAmt += atol(pobTran->srBRec.szDCC_TIPFCA);
		sprintf(szTemplate, "%ld", lnTipTotalAmt);
		
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(szTemplate, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}
		else
		{
			inFunc_Amount_Comma_DCC(szTemplate, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		/* 總計 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		inPRINT_Buffer_PutIn("總計(Total)  :", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("Total amount of Transaction Currency", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_) || 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* Exchange Rate: 1 EUR = 39.52 NTD 上需空一行 by Russell 2020/10/29 下午 5:16 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 列印換算匯率比【一】 */
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);

	/* 列印換算匯率比【二】 */
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szOutputAmt);

	/* 匯率 */
	inRetVal = inPRINT_Buffer_PutIn("Exchange Rate:", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* Ex:1 USD = 30.0000 NTD */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szOutputAmt);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 轉換費率(Currency Conversion Fee) */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%s", "轉換費率(Currency Conversion Fee)");
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s", szPrintBuf1);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* XX.XX %*/
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %%", szPrintBuf1);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* (Currency Conversion Fee on the exchange rate over a wholesale rate.) */
	inRetVal = inPRINT_Buffer_PutIn("(Currency Conversion Fee on the", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("exchange rate over a wholesale rate.)", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 斷行 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer_Small
Date&Time       :2017/5/17 上午 11:05
Describe        :DCC 一段式詢價後轉台幣
*/
int inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;;
	int	inIRDU = 0;
	char	szPrintBuf[84 + 1] = {0}, szPrintBuf1[42 + 1] = {0}, szTemplate[42 + 1] = {0};
	char	szIRDU[12 + 1] = {0};			/* Inverted Rate Display Unit */
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */

	inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("         Please select          ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn(" Transaction currency as below  ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 外幣     1.  [ X ]  EUR 8.18 上需空一行 */
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 外幣 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, "", szPrintBuf1);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "1.【 】%s %s", pobTran->srBRec.szDCC_FCAC, szPrintBuf1);						/* Foreign currcncy Alphabetic Code */
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 匯率 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "Exchange Rate:");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	
	memset(szExchangeAmt, 0x00, sizeof(szExchangeAmt));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 轉換費率(Currency Conversion Fee) */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%s", "轉換費率(Currency Conversion Fee)");
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s", szPrintBuf1);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* XX.XX %*/
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %%", szPrintBuf1);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* (Currency Conversion Fee on the exchange rate over a wholesale rate.) */
	/* 前半段 */
	inRetVal = inPRINT_Buffer_PutIn("(Currency Conversion Fee on the exchange ", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 後半段 */
	inRetVal = inPRINT_Buffer_PutIn("rate over a wholesale rate.)", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 台幣     2.  [    ]  NTD 264  上需空一行 by Russell 2020/10/29 下午 4:32 */
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 台幣 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTxnAmount);
	inFunc_Amount_Comma_DCC(szPrintBuf1, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, "0", "", szPrintBuf1);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "2.【X】%s %s", "NTD", szPrintBuf1);									/* Foreign currcncy Alphabetic Code */

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 有開小費要多印 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTransFunc(szTemplate);
	/* 檢查是否有開小費 */
	/* 預先授權完成必定不印小費 */
	if (szTemplate[6] == 'Y'	&&
	    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE))
	{
		/* 小費     小費（Tips）： EUR  _______________ 上需空一行 by Russell 2020/10/29 下午 3:20 */
		inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 商店聯上的小費跟總計不需列印外幣 by Russell 2020/10/29 下午 3:19 */
		if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 小費 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "小費(Tips)   : %s______________________________________", "");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 總計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "總計(Total)  : %s______________________________________", "");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			/* 小費 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "小費(Tips)   : %s______________________________________", "NTD");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 總計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "總計(Total)  : %s______________________________________", "NTD");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_09X23_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		/* 不開小費不印下面這一段 */
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small
Date&Time       :2017/5/18 下午 1:34
Describe        :列印結尾
*/
int inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer_Small(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
		/* 免責宣言上需空一行 by Russell 2020/10/29 下午 3:21 */
		inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			/* Disclaimer */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) &&
				 (pobTran->srBRec.inCode == _SALE_	|| 
				  pobTran->srBRec.inCode == _TIP_	|| 
				  pobTran->srBRec.inCode == _PRE_COMP_	||
				 (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Cardholder expressly agrees to the Transaction ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Receipt Information by marking the “accept box”", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("below.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("[ ] ACCEPT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}
				}
			}

			/* 列印空白行 */
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

                        /* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("X:      免簽名       ", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			/* 要簽名 */
			else
			{
				/* 藉由TRT_FileName比對來組出bmp的檔名 */
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				
				memset(szSignature, 0x00, sizeof(szSignature));
				/* 因為用invoice所以不用inFunc_ComposeFileName */
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
				memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
				sprintf(szSignaturePath, "./fs_data/%s", szSignature);
				/* 有在signpad簽名*/
				if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS)
				{
					/* 電子簽名 */
					inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
				}
				/* 手簽 */
				else
				{
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				}
				
				inPRINT_Buffer_PutIn("X:", _PRT_HEIGHT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
                        
			inPRINT_Buffer_PutIn("------------------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			/* 持卡人姓名 */
			inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			/* Disclaimer */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) &&
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) &&
				 (pobTran->srBRec.inCode == _SALE_		|| 
				  pobTran->srBRec.inCode == _TIP_		|| 
				  pobTran->srBRec.inCode == _PRE_COMP_		||
				 (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Cardholder expressly agrees to the Transaction ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Receipt Information by marking the “accept box”", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("below.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("[ ] ACCEPT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免簽名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
                else
                {
			/* Disclaimer */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) &&
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) &&
				 (pobTran->srBRec.inCode == _SALE_		|| 
				  pobTran->srBRec.inCode == _TIP_		|| 
				  pobTran->srBRec.inCode == _PRE_COMP_		||
				 (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}
				}
			}
			
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免簽名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			/* 持卡人存根Card holder stub 上需空一行 by Russell 2020/10/29 下午 3:18 */
			inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
	
                        inPRINT_Buffer_PutIn("　　　　　持卡人存根 Card holder stub", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			inPRINT_Buffer_PutIn(pobTran->srBRec.szCardHolder, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
                /* 列印警示語 */
                inPRINT_Buffer_PutIn("            I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("        ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
		
		/* Print Slogan */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);
			
			if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
			    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
			    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
			{
				if (inCREDIT_PRINT_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle) != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }

        return (VS_SUCCESS);
}


/*
Function        :inCREDIT_PRINT_CHECK_ByBuffer
Date&Time       :2016/2/24 下午 3:47
Describe        :
*/
int inCREDIT_PRINT_Check_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int     inRecordCnt;

        inRecordCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
        /* 回傳VS_ERROR(回傳 -1 )會跳出，交易筆數小於0( VS_NoRecord 會回傳 -98 )會印空白簽單 */
        /* 其餘則回傳交易筆數*/

        return (inRecordCnt);
}


/*
Function        :inCREDIT_PRINT_TOP_ByBuffer
Date&Time       :2016/2/24 下午 3:48
Describe        :
*/
int inCREDIT_PRINT_Top_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        /* Get商店代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 16, _PADDING_LEFT_);
        sprintf(szPrintBuf, "商店代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* Get端末機代號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);
        sprintf(szPrintBuf, "端末機代號%s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        /* 列印日期 / 時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	sprintf(szPrintBuf, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
        /* 列印交易類別 */
        /* 結帳時預先列印總額和明細(pobTran->uszPrePrintBit == VS_TRUE)，不印交易類別*/
        if (pobTran->srBRec.inCode == _SETTLE_	&&
	    pobTran->uszPrePrintBit != VS_TRUE)
        {
                inRetVal = inPRINT_Buffer_PutIn("交易類別(Trans. Type)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "60 結帳 SETTLEMENT");
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* 列印批次號碼 */
        inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szTemplate, "%03ld", pobTran->srBRec.lnBatchNum);
        strcpy(szPrintBuf, szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        /* 列印主機 */
        inRetVal = inPRINT_Buffer_PutIn("主機(Host)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        inGetHostLabel(szPrintBuf);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmount_ByBuffer
Date&Time       :2016/2/24 下午 3:49
Describe        :列印總金額
*/
int inCREDIT_PRINT_TotalAmount_ByBuffer(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	ACCUM_TOTAL_REC *srAccumRec;
	
	srAccumRec = srAccumRecOrg;

        if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
	{
                /* 客製化098，結帳流程，預先列印，TITLE為總額 */
                if (pobTran->uszPrePrintBit == VS_TRUE)
                        inPRINT_Buffer_PutIn("總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
                else
                        inPRINT_Buffer_PutIn("結帳報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}
	else
        {
                inPRINT_Buffer_PutIn("總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
        }

        inPRINT_Buffer_PutIn("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (srAccumRec->lnTotalCount == 0)
        {
                if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Accum(pobTran) == VS_TRUE)
                {
                        /* 銷售 D */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", 0L);
                        sprintf(szTemplate, "%ld", 0L);
                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                        strcat(szPrintBuf, szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        
                        /* 銷售 免 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szPrintBuf, "銷售 免　%03lu   NT$", 0L);
                        sprintf(szTemplate, "%lld", 0LL);
                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                        strcat(szPrintBuf, szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
                else
                {
                        /* 銷售 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", 0L);
                        sprintf(szTemplate, "%ld", 0L);
                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                        strcat(szPrintBuf, szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", 0L);
                sprintf(szTemplate, "%ld", 0L);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
        else
        {
                if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Accum(pobTran) == VS_TRUE)
                {
                        /* 銷售 D */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnTotalSale_SignCount);
                        sprintf(szTemplate, "%lld", (srAccumRec->llTotalSale_SignAmount + srAccumRec->llTotalTips_SignAmount));
                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                        strcat(szPrintBuf, szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        
                        /* 銷售 免 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szPrintBuf, "銷售 免　%03lu   NT$", srAccumRec->lnTotalSale_NoSignCount);
                        sprintf(szTemplate, "%lld", (srAccumRec->llTotalSale_NoSignAmount + srAccumRec->llTotalTips_NoSignAmount));
                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                        strcat(szPrintBuf, szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
                else
                {
                        /* 銷售 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnTotalSaleCount);
                        sprintf(szTemplate, "%lld", (srAccumRec->llTotalSaleAmount + srAccumRec->llTotalTipsAmount));
                        inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                        strcat(szPrintBuf, szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 淨額 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByCard_ByBuffer
Date&Time       :2016/2/24 下午 3:49
Describe        :依卡別列印
*/
int inCREDIT_PRINT_TotalAmountByCard_ByBuffer(TRANSACTION_OBJECT *pobTran, void *srAccumRecOrg, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	char	szUnknownCardLabel[20 + 1];
	ACCUM_TOTAL_REC *srAccumRec;
	srAccumRec = srAccumRecOrg;

        inPRINT_Buffer_PutIn("卡別小計", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

        if (srAccumRec->llUCardTotalSaleAmount != 0L || srAccumRec->llUCardTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_TWIN_CARD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnUCardTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llUCardTotalSaleAmount + srAccumRec->llUCardTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnUCardTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llUCardTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnUCardTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnUCardTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llUCardTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llVisaTotalSaleAmount != 0L || srAccumRec->llVisaTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_VISA_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnVisaTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llVisaTotalSaleAmount + srAccumRec->llVisaTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnVisaTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llVisaTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnVisaTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnVisaTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llVisaTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llMasterTotalSaleAmount != 0L || srAccumRec->llMasterTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_MASTERCARD_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnMasterTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llMasterTotalSaleAmount + srAccumRec->llMasterTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnMasterTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llMasterTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnMasterTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnMasterTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llMasterTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llJcbTotalSaleAmount != 0L || srAccumRec->llJcbTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_JCB_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnJcbTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llJcbTotalSaleAmount + srAccumRec->llJcbTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnJcbTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llJcbTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnJcbTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnJcbTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llJcbTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llAmexTotalSaleAmount != 0L || srAccumRec->llAmexTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_AMEX_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnAmexTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llAmexTotalSaleAmount + srAccumRec->llAmexTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnAmexTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llAmexTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnAmexTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnAmexTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llAmexTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

        if (srAccumRec->llCupTotalSaleAmount != 0L || srAccumRec->llCupTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_CUP_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnCupTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llCupTotalSaleAmount + srAccumRec->llCupTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnCupTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llCupTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnCupTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llCupTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnCupTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llCupTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
	
	if (srAccumRec->llDinersTotalSaleAmount != 0L || srAccumRec->llDinersTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_DINERS_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnDinersTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llDinersTotalSaleAmount + srAccumRec->llDinersTotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnDinersTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llDinersTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnDinersTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llDinersTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnDinersTotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llDinersTotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
	
	if (srAccumRec->llFiscTotalSaleAmount != 0L || srAccumRec->llFiscTotalRefundAmount != 0L)
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", _CARD_TYPE_SMARTPAY_);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnFiscTotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llFiscTotalSaleAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnFiscTotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llFiscTotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnFiscTotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llFiscTotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", (unsigned long)0);
                sprintf(szTemplate, "%lld", (long long)0);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }

	/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
	/* 新增未知卡別小計 add by LingHsiung 2020/7/7 下午 2:26 */
	if (srAccumRec->llX0TotalSaleAmount != 0L || srAccumRec->llX0TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(0);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX0TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX0TotalSaleAmount + srAccumRec->llX0TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX0TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX0TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX0TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX0TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX0TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX0TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX1TotalSaleAmount != 0L || srAccumRec->llX1TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(1);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX1TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX1TotalSaleAmount + srAccumRec->llX1TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX1TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX1TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX1TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX1TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX1TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX1TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX2TotalSaleAmount != 0L || srAccumRec->llX2TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(2);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX2TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX2TotalSaleAmount + srAccumRec->llX2TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX2TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX2TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX2TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX2TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX2TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX2TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX3TotalSaleAmount != 0L || srAccumRec->llX3TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(3);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX3TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX3TotalSaleAmount + srAccumRec->llX3TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX3TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX3TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX3TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX3TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX3TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX3TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX4TotalSaleAmount != 0L || srAccumRec->llX4TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(4);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX4TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX4TotalSaleAmount + srAccumRec->llX4TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX4TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX4TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX4TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX4TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX4TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX4TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX5TotalSaleAmount != 0L || srAccumRec->llX5TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(5);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX5TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX5TotalSaleAmount + srAccumRec->llX5TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX5TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX5TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX5TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX5TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX5TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX5TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX6TotalSaleAmount != 0L || srAccumRec->llX6TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(6);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX6TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX6TotalSaleAmount + srAccumRec->llX6TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX6TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX6TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX6TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX6TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX6TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX6TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX7TotalSaleAmount != 0L || srAccumRec->llX7TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(7);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX7TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX7TotalSaleAmount + srAccumRec->llX7TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX7TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX7TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX7TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX7TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX7TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX7TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX8TotalSaleAmount != 0L || srAccumRec->llX8TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(8);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX8TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX8TotalSaleAmount + srAccumRec->llX8TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX8TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX8TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX8TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX8TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX8TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX8TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	if (srAccumRec->llX9TotalSaleAmount != 0L || srAccumRec->llX9TotalRefundAmount != 0L)
	{
		inLoadCDTXRec(9);
		memset(szUnknownCardLabel, 0x00, sizeof(szUnknownCardLabel));
		inGetUnknownCardLabel(szUnknownCardLabel);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡別 　　%s", szUnknownCardLabel);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 銷售 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "銷售 　　%03lu   NT$", srAccumRec->lnX9TotalSaleCount);
                sprintf(szTemplate, "%lld", (srAccumRec->llX9TotalSaleAmount + srAccumRec->llX9TotalTipsAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 退貨 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "退貨 　　%03lu   NT$", srAccumRec->lnX9TotalRefundCount);
                sprintf(szTemplate, "%lld", (0 - srAccumRec->llX9TotalRefundAmount));
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                /* 小計 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小計 　　%03lu   NT$", srAccumRec->lnX9TotalCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX9TotalAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                /* 小費 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnX9TotalTipsCount);
                sprintf(szTemplate, "%lld", srAccumRec->llX9TotalTipsAmount);
		inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
                strcat(szPrintBuf, szTemplate);
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}	
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer
Date&Time       :2016/2/24 下午 3:50
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByInstllment_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];

        inPRINT_Buffer_PutIn("分期交易總額", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnInstSaleCount);
	sprintf(szTemplate, "%lld", (srAccumRec->llInstSaleAmount + srAccumRec->llInstTipsAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnInstRefundCount);
	sprintf(szTemplate, "%lld", (0 - srAccumRec->llInstRefundAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 淨額 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnInstTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llInstTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 小費 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnInstTipsCount);
	sprintf(szTemplate, "%lld", srAccumRec->llInstTipsAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer
Date&Time       :2016/2/24 下午 3:50
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByRedemption_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	
        inPRINT_Buffer_PutIn("紅利扣抵總額", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("    筆數(CNT)      金額(AMOUNT)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 銷售 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "銷售 Ｄ　%03lu   NT$", srAccumRec->lnRedeemSaleCount);
	sprintf(szTemplate, "%lld", (srAccumRec->llRedeemSaleAmount + srAccumRec->llRedeemTipsAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "退貨 Ｒ　%03lu   NT$", srAccumRec->lnRedeemRefundCount);
	sprintf(szTemplate, "%lld", (0 - srAccumRec->llRedeemRefundAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 淨額 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "淨額 Ｔ　%03lu   NT$", srAccumRec->lnRedeemTotalCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRedeemTotalAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("    ------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 小費 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小費 　　%03lu   NT$", srAccumRec->lnRedeemTipsCount);
	sprintf(szTemplate, "%lld", srAccumRec->llRedeemTipsAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("-----------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 紅利扣抵總點數 */
	/* 初始化 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

	/* 將NT$ ＋數字塞到szTemplate中來inpad */
	sprintf(szTemplate, " %ld", (srAccumRec->lnRedeemTotalPoint));
	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 14, _PADDING_LEFT_);

	/* 把前面的字串和數字結合起來 */
	sprintf(szPrintBuf, "紅利扣抵總點數 :%s點", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 結束隔線 */
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_TotalAmountByOther
Date&Time       :2017/4/14 下午 1:26
Describe        :
*/
int inCREDIT_PRINT_TotalAmountByOther(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_ERROR;
	int	inSpace = 13;
	char	szESCMode[2 + 1] = {0};
	char	szPrintBuf[100 + 1] = {0};
	char	szTRTFileName[12 + 1] = {0};
	char	szTransFunc[20 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};

	/* 若找不到ESC或ESC沒開或水位為0，不送欄位NE */
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (memcmp(szESCMode, "Y", strlen("Y")) != 0)
	{
		return (VS_SUCCESS);
	}
	else
        {	
		memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		inGetTRTFileName(szTRTFileName);
		
		if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) ||
		    !memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
		{
			/* 結帳才印已上傳筆數 */
			if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
			{
				if (pobTran->uszPrePrintBit == VS_TRUE)
				{
					/* 麥當勞結帳時的總額 */
				}
				else
				{
					inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 電簽已上傳筆數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03lu", srAccumRec->lnESC_SuccessNum);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "電簽已上傳總筆數 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 電簽已上傳金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_SuccessAmount);
					inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "電簽已上傳總金額 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 電簽未上傳筆數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03lu", srAccumRec->lnESC_FailUploadNum);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "電簽未上傳總筆數 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 電簽未上傳金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_FailUploadAmount);
					inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "電簽未上傳總金額 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);


					/* 非電簽總筆數(BYPASS) */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03lu", srAccumRec->lnESC_BypassNum);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "非電簽總筆數 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 非電簽總金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_BypassAmount);
					inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "非電簽總金額 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					inRetVal = inPRINT_Buffer_PutIn("==========================================", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 紙本簽單總筆數 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%03lu", srAccumRec->lnESC_TotalFailULNum);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "紙本簽單總筆數 =");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					/* 紙本簽單總金額 */
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_TotalFailULAmount);
					inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "紙本簽單總金額 = ");
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					memset(szTransFunc, 0x00, sizeof(szTransFunc));
					inGetTransFunc(szTransFunc);

					if (szTransFunc[5] == 'Y')
					{
						/* ESC預先授權列印紙本及預先授權不納入結帳總額 */
						inRetVal = inPRINT_Buffer_PutIn("＊以上統計不含預先授權交易", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inRetVal = inPRINT_Buffer_PutIn("＊預先授權", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "　%03lu", srAccumRec->lnESC_PreAuthNum);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "　紙本簽單總筆數 = ");
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

						/* 紙本簽單總金額 */
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "%ld", (long)srAccumRec->llESC_PreAuthAmount);
						inFunc_Amount_Comma(szPrintBuf, "NT$ ", ' ', _SIGNED_NONE_, inSpace, _PADDING_LEFT_);
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "　紙本簽單總金額 = ");
						inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
				}
			}
                        
			/* ESC補強機制 */
			inCREDIT_PRINT_ESC_Reinforce_Count_ByBuffer(pobTran, uszBuffer, srFont_Attrib, srBhandle);
                }
        }

	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer
Date&Time       :2017/2/22 上午 10:07
Describe        :列印優惠兌換
*/
int inCREDIT_PRINT_Total_Loyalty_Redeem_ByBuffer(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	
        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 優惠兌換成功總數量 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "%ld 張", srAccumRec->lnLoyaltyRedeemSuccessCount);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "優惠兌換成功總數量");
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 優惠兌換取消總數量 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "%ld 張", srAccumRec->lnLoyaltyRedeemCancelCount);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "優惠兌換取消總數量");
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 優惠兌換合計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "%ld 張", srAccumRec->lnLoyaltyRedeemTotalCount);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "優惠兌換合計");
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Total_HG_ByBuffer
Date&Time       :2017/3/7 下午 5:33
Describe        :列印HG總額
*/
int inCREDIT_PRINT_Total_HG_ByBuffer(TRANSACTION_OBJECT *pobTran, HG_ACCUM_TOTAL_REC *srHGAccumRec, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char    szPrintBuf[84 + 1], szTemplate[84 + 1];
	char	szTRTFileName[16 + 1];
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	/* 選聯合印快樂購聯合集點卡，選HG印總額報表 */
	if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0)
	{
                inPRINT_Buffer_PutIn("快樂購聯合集點卡", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}
	else
	{
                inPRINT_Buffer_PutIn("總額報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}

	/* 紅利積點 */
	inPRINT_Buffer_PutIn("紅利積點", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 紅利積點 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "紅利積點 %03d   NT$", srHGAccumRec->inRewardCnt);
	sprintf(szTemplate, "%lu", srHGAccumRec->uslnRewardAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 點數扣抵 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "點數扣抵 %03d   NT$", srHGAccumRec->inOnlineRedeemCnt);
	sprintf(szTemplate, "%lu", srHGAccumRec->uslnOnlineRedeemAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 加價購 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "加價購   %03d   NT$", srHGAccumRec->inPointCertainCnt);
	sprintf(szTemplate, "%lu", srHGAccumRec->uslnPointCertainAmount);
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 小計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小計 　　      NT$");
	sprintf(szTemplate, "%lu", (srHGAccumRec->uslnRewardAmount + srHGAccumRec->uslnOnlineRedeemAmount + srHGAccumRec->uslnPointCertainAmount));
	inFunc_Amount_Comma(szTemplate, "" , ' ', _SIGNED_NONE_, 13, VS_TRUE);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	/* 空行 */
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

	/* 紅利扣抵 */
	inPRINT_Buffer_PutIn("紅利扣抵", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 點數扣抵 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "點數扣抵 %03d      ", srHGAccumRec->inOnlineRedeemCnt);
	sprintf(szTemplate, "%lu　點", srHGAccumRec->uslnOnlineRedeemPoint);
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 加價購 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "加價購   %03d      ", srHGAccumRec->inPointCertainCnt);
	sprintf(szTemplate, "%lu　點", srHGAccumRec->uslnPointCertainPoint);
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 點數兌換 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "點數兌換 %03d      ", srHGAccumRec->inFullRedemptionCnt);
	sprintf(szTemplate, "%lu　點", srHGAccumRec->uslnFullRedemptionPoint);
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 小計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "小計 　　         ");
	sprintf(szTemplate, "%lu　點", (srHGAccumRec->uslnOnlineRedeemPoint + srHGAccumRec->uslnPointCertainPoint + srHGAccumRec->uslnFullRedemptionPoint));
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	/* 空行 */
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

	/* 回饋退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "回饋退貨 %03d      ", srHGAccumRec->inRewardRefundCnt);
	sprintf(szTemplate, "%lu　點", srHGAccumRec->uslnRewardRefundPoint);
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	/* 扣抵退貨 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "扣抵退貨 %03d      ", srHGAccumRec->inRedeemRefundCnt);
	sprintf(szTemplate, "%ld　點", (0 - srHGAccumRec->uslnRedeemRefundPoint));
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	/* 空行 */
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	/* 合計 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	sprintf(szPrintBuf, "合計 　　         ");
	sprintf(szTemplate, "%lu　點", (srHGAccumRec->uslnOnlineRedeemPoint + srHGAccumRec->uslnPointCertainPoint + srHGAccumRec->uslnFullRedemptionPoint + srHGAccumRec->uslnRewardRefundPoint - srHGAccumRec->uslnRedeemRefundPoint));
	inFunc_PAD_ASCII(szTemplate, szTemplate , ' ', 16, _PADDING_LEFT_);
	strcat(szPrintBuf, szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

	/* 空行 */
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_END_ByBuffer
Date&Time       :2016/2/24 下午 3:50
Describe        :列印結尾
*/
int inCREDIT_PRINT_End_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	
        inPRINT_Buffer_PutIn("*** 列印完成 ***", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

        for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportMiddle_ByBuffer
Date&Time       :2016/2/24 下午 3:52
Describe        :
*/
int inCREDIT_PRINT_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szFuncEnable[1 + 1];			/* catch Y or N */
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
        inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
	inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 櫃號功能有開才印櫃號 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
        inGetStoreIDEnable(szFuncEnable);
        if (szFuncEnable[0] == 'Y')
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_)  ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPRINT_Buffer_PutIn("品群碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer
Date&Time       :2017/5/17 下午 2:20
Describe        :
*/
int inCREDIT_PRINT_NCCC_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_ERROR;
        char    szFuncEnable[1 + 1];			/* catch Y or N */
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 商店自存聯卡號遮掩 */
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("交易編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("檢查碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		/* SmartPay要印調單編號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetFiscFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("授權碼/調單編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 銀聯功能有開才印回覆碼 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetCUPFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("回覆碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inPRINT_Buffer_PutIn("品群碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}

		/* 優惠平台有開才印 */
		/* (需求單 - 107367)優惠查核改由ATS判斷 修改為含日期判斷 by Russell 2019/7/2 下午 4:27 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			
		}
		else
		{
			inRetVal = inNCCC_Loyalty_ASM_Flag();
			if (inRetVal == VS_SUCCESS)
			{
				inPRINT_Buffer_PutIn("優惠類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		/* SmartPay要印調單編號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetFiscFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("授權碼/調單編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 銀聯功能有開才印回覆碼 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetCUPFuncEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			inPRINT_Buffer_PutIn("回覆碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inPRINT_Buffer_PutIn("品群碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}

		/* 優惠平台有開才印 */
		/* (需求單 - 107367)優惠查核改由ATS判斷 修改為含日期判斷 by Russell 2019/7/2 下午 4:27 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			
		}
		else
			{
			inRetVal = inNCCC_Loyalty_ASM_Flag();
			if (inRetVal == VS_SUCCESS)
			{
				inPRINT_Buffer_PutIn("優惠類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_DetailReportMiddle_ByBuffer
Date&Time       :2017/5/17 下午 2:20
Describe        :
*/
int inCREDIT_PRINT_DCC_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szFuncEnable[1 + 1];			/* catch Y or N */
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	
	/* 商店自存聯卡號遮掩 */
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("交易編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("檢查碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inPRINT_Buffer_PutIn("品群碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	else
	{
		inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

		inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 櫃號功能有開才印櫃號 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetStoreIDEnable(szFuncEnable);
		if (szFuncEnable[0] == 'Y')
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inPRINT_Buffer_PutIn("品群碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	}
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DetailReportBottom_ByBuffer
Date&Time       :2016/2/24 下午 3:52
Describe        :
*/
int inCREDIT_PRINT_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i, j;
	int	inReadCnt = 0;
	int	inCardLen = 0;
	int	inRetVal = VS_SUCCESS;
        char	szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];
        char	szFuncEnable[1 + 1];			/* catch Y or N */
        char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
        char	szCustomerIndicator[3 + 1] = {0};
        char    szPrtMode[2 + 1] = {0};
	int	inFontSize_Option = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DetailReportBottom()_START");
	}
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        memset(szPrtMode, 0x00, sizeof(szPrtMode));
        inGetPrtMode(szPrtMode);
        
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	    
	{
		inFontSize_Option = _PRT_CUS_123_DETAIL_;
	}
	else
	{
		inFontSize_Option = _PRT_NORMAL2_;
	}
	
	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	/* 預設為無須重找 */
	guszEnormousNoNeedResetBit = VS_TRUE;

        for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
        {
                /*. 開始讀取每一筆交易記錄 .*/
                if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inReadCnt) != VS_SUCCESS)
                {
                        inRetVal = VS_ERROR;
                        break;
                }
		
		/* 優惠兌換 */
                if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_		||
		    pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
		{
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 交易別 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_)
			{
				sprintf(szPrintBuf, "優惠兌換");
			}
			else if (pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
			{
				sprintf(szPrintBuf, "兌換取消");
			}
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			continue;
		}
		else
		{
			/* 商店自存聯卡號遮掩 */
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
			{
				/* Invoice Number */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
				{
					sprintf(szTemplate1, "INV.  NO = %06ld", pobTran->srBRec.lnOrgInvNum);
				}
				else
				{
					sprintf(szTemplate1, "*INV. NO = %06ld", pobTran->srBRec.lnOrgInvNum);
				}
				
				/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetESCMode(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
				{
					strcat(szTemplate1, " <P>");
				}
				
				strcat(szPrintBuf, szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Print Amount */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));

				if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
				{
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						switch (pobTran->srBRec.inCode)
						{
							case _CASH_ADVANCE_ :
							case _SALE_OFFLINE_ :
							case _FORCE_CASH_ADVANCE_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							default :
								memset(szTemplate2, 0x00, sizeof(szTemplate2));
								inGetHostLabel(szTemplate2);
								sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
								break;
						} /* End switch () */
					}
					else
					{
						switch (pobTran->srBRec.inCode)
						{
							case _SALE_:
							case _INST_SALE_ :
							case _REDEEM_SALE_ :
							case _MAIL_ORDER_ :
							case _CUP_MAIL_ORDER_ :
							case _SALE_OFFLINE_ :
							case _PRE_COMP_ :
							case _PRE_AUTH_ :
							case _CUP_SALE_ :
							case _CUP_PRE_COMP_ :
							case _CUP_PRE_AUTH_ :
							case _FISC_SALE_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							case _TIP_ :
								sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
								break;
							case _REFUND_ :
							case _INST_REFUND_ :
							case _REDEEM_REFUND_ :
							case _CUP_REFUND_ :
							case _CUP_MAIL_ORDER_REFUND_ :
							case _FISC_REFUND_ :
								sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
								break;
							case _INST_ADJUST_ :
							case _REDEEM_ADJUST_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							case _ADJUST_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
								break;
							default :
								memset(szTemplate2, 0x00, sizeof(szTemplate2));
								inGetHostLabel(szTemplate2);
								sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
								break;
						} /* End switch () */
					}
				}
				else
				{
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						switch (pobTran->srBRec.inOrgCode)
						{
							case _CASH_ADVANCE_ :
							case _SALE_OFFLINE_ :
							case _FORCE_CASH_ADVANCE_ :
								sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
								break;
							default :
								memset(szTemplate2, 0x00, sizeof(szTemplate2));
								inGetHostLabel(szTemplate2);
								sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
								break;
						} /* End switch () */
					}
					else
					{
						switch (pobTran->srBRec.inOrgCode)
						{
							 case _SALE_:
							case _INST_SALE_ :
							case _REDEEM_SALE_ :
							case _MAIL_ORDER_ :
							case _CUP_MAIL_ORDER_ :
							case _SALE_OFFLINE_ :
							case _PRE_COMP_ :
							case _PRE_AUTH_ :
							case _CUP_SALE_ :
							case _CUP_PRE_COMP_ :
							case _CUP_PRE_AUTH_ :
							case _FISC_SALE_ :
								if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
								{
									sprintf(szTemplate1, "%ld", 0 - (pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
								}
								else
								{
									sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
								}
								break;
							/* NCCC小費不能取消 */
		//	                                case _TIP_ :
		//	                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
		//	                                        break;
							case _REFUND_ :
							case _INST_REFUND_ :
							case _REDEEM_REFUND_ :
							case _CUP_REFUND_ :
							case _CUP_MAIL_ORDER_REFUND_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							case _INST_ADJUST_ :
							case _REDEEM_ADJUST_ :
								sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
								break;
							case _ADJUST_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							default :
								memset(szTemplate2, 0x00, sizeof(szTemplate2));
								inGetHostLabel(szTemplate2);
								sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
								break;
						} /* End switch () */
					}
				}
				inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
				strcat(szPrintBuf, szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Trans Type */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
				sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
                                /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(pobTran) == VS_TRUE)
                                {
                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                                        if (inRetVal != VS_SUCCESS)
                                                return (VS_ERROR);
                                
                                
                                        /* 免簽名 */
                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "免簽名");
                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                                        if (inRetVal != VS_SUCCESS)
                                                return (VS_ERROR);
                                }
                                else
                                {
                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        if (inRetVal != VS_SUCCESS)
                                                return (VS_ERROR);
                                }
                                

				/* 卡號 */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					strcpy(szTemplate1, pobTran->srBRec.szPAN);
					if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
					{
						inCardLen = strlen(szTemplate1);

						for (j = 6; j < (inCardLen - 4); j ++)
							szTemplate1[j] = 0x2A;
					}
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "CARD  NO = %s", szTemplate1);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					strcpy(szTemplate1, pobTran->srBRec.szPAN);

					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
					for (i = 6; i < (strlen(szTemplate1) - 4); i ++)
					{
						szTemplate1[i] = 0x2A;
					}
					
					inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 19, _PADDING_RIGHT_);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "CARD  NO = %s",szTemplate1);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 交易序號Transaction No. */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				strcpy(szPrintBuf, "TXN.  NO = ");
				if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
					strcat(szPrintBuf, pobTran->srBRec.szTxnNo);
				
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				
				/* 檢查碼Check No */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					/* SmartPay不印檢查碼 */
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					if (strlen(pobTran->srBRec.szCheckNO) > 0)
					{
						strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
					}
					else
					{
						inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
					}
					sprintf(szPrintBuf, "CHECK NO = %s",szTemplate1);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
					
				/* Trans Date Time */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "DATE=%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "TIME=%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Approved No. & RRN NO. */
				/* SmartPay印調單編號 */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, "RRN NO. = ");
					strcat(szPrintBuf, pobTran->srBRec.szFiscRRN);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, "APPROVED CODE = ");
					strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* RESPONSE CODE */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetCUPFuncEnable(szFuncEnable);
				if (szFuncEnable[0] == 'Y')
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, "RESPONSE CODE = ");
					strcat(szPrintBuf, pobTran->srBRec.szRespCode);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* Store ID */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetStoreIDEnable(szFuncEnable);
				if (szFuncEnable[0] == 'Y')
				{
					/*開啟櫃號功能*/
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "STORE ID: %s", pobTran->srBRec.szStoreID);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
				       /*沒開啟櫃號功能，則不印櫃號*/
				}

				/* 列印優惠資訊 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
				{

				}
				else
				{
					if (inNCCC_Loyalty_ASM_Flag() != VS_SUCCESS)
					{
						
					}
					else
					{
						/* 原交易有優惠資訊，取消交易後，非小費及暫停優惠服務，則列印取消優惠資訊 */
						if (pobTran->srBRec.uszVOIDBit == VS_TRUE	&&
						   (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
						    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
						    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)	&&
						    atoi(pobTran->srBRec.szAwardNum) > 0	&&
						    pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
						{
							inRetVal = inPRINT_Buffer_PutIn("取消優惠", inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else if (pobTran->srBRec.uszVOIDBit != VS_TRUE		&&
							(pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
							 pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
							 pobTran->srBRec.uszRewardL5Bit == VS_TRUE))
						{
							inRetVal = inPRINT_Buffer_PutIn("優惠", inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}
				}
			}
			else
			{
				/* Invoice Number */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
				{
					sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
				}
				else
				{
					sprintf(szTemplate1, "*INV:%06ld", pobTran->srBRec.lnOrgInvNum);
				}
				
				/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetESCMode(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
				{
					strcat(szTemplate1, " <P>");
				}
				
				strcat(szPrintBuf, szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Print Amount */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));

				if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
				{
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						switch (pobTran->srBRec.inCode)
						{
							case _CASH_ADVANCE_ :
							case _SALE_OFFLINE_ :
							case _FORCE_CASH_ADVANCE_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							default :
								memset(szTemplate2, 0x00, sizeof(szTemplate2));
								inGetHostLabel(szTemplate2);
								sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
								break;
						} /* End switch () */
					}
					else
					{
						switch (pobTran->srBRec.inCode)
						{
							case _SALE_:
							case _INST_SALE_ :
							case _REDEEM_SALE_ :
							case _MAIL_ORDER_ :
							case _CUP_MAIL_ORDER_ :
							case _SALE_OFFLINE_ :
							case _PRE_COMP_ :
							case _PRE_AUTH_ :
							case _CUP_SALE_ :
							case _CUP_PRE_COMP_ :
							case _CUP_PRE_AUTH_ :
							case _FISC_SALE_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							case _TIP_ :
								sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
								break;
							case _REFUND_ :
							case _INST_REFUND_ :
							case _REDEEM_REFUND_ :
							case _CUP_REFUND_ :
							case _CUP_MAIL_ORDER_REFUND_ :
							case _FISC_REFUND_ :
								sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
								break;
							case _INST_ADJUST_ :
							case _REDEEM_ADJUST_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							case _ADJUST_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnAdjustTxnAmount);
								break;
							default :
								memset(szTemplate2, 0x00, sizeof(szTemplate2));
								inGetHostLabel(szTemplate2);
								sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
								break;
						} /* End switch () */
					}
				}
				else
				{
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
					{
						switch (pobTran->srBRec.inOrgCode)
						{
							case _CASH_ADVANCE_ :
							case _SALE_OFFLINE_ :
							case _FORCE_CASH_ADVANCE_ :
								sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
								break;
							default :
								memset(szTemplate2, 0x00, sizeof(szTemplate2));
								inGetHostLabel(szTemplate2);
								sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
								break;
						} /* End switch () */
					}
					else
					{
						switch (pobTran->srBRec.inOrgCode)
						{
							 case _SALE_:
							case _INST_SALE_ :
							case _REDEEM_SALE_ :
							case _MAIL_ORDER_ :
							case _CUP_MAIL_ORDER_ :
							case _SALE_OFFLINE_ :
							case _PRE_COMP_ :
							case _PRE_AUTH_ :
							case _CUP_SALE_ :
							case _CUP_PRE_COMP_ :
							case _CUP_PRE_AUTH_ :
							case _FISC_SALE_ :
								if (pobTran->srBRec.uszUPlanTransBit == VS_TRUE)
								{
									sprintf(szTemplate1, "%ld", 0 - (pobTran->srBRec.lnCUPUPlanDiscountedAmount + pobTran->srBRec.lnCUPUPlanPreferentialAmount));
								}
								else
								{
									sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
								}
								break;
							/* NCCC小費不能取消 */
		//	                                case _TIP_ :
		//	                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
		//	                                        break;
							case _REFUND_ :
							case _INST_REFUND_ :
							case _REDEEM_REFUND_ :
							case _CUP_REFUND_ :
							case _CUP_MAIL_ORDER_REFUND_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							case _INST_ADJUST_ :
							case _REDEEM_ADJUST_ :
								sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
								break;
							case _ADJUST_ :
								sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
								break;
							default :
								memset(szTemplate2, 0x00, sizeof(szTemplate2));
								inGetHostLabel(szTemplate2);
								sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
								break;
						} /* End switch () */
					}
				}
				inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
				strcat(szPrintBuf, szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

				if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

				/* Trans Type */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
				sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
				/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                                if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(pobTran) == VS_TRUE)
                                {
                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                                        if (inRetVal != VS_SUCCESS)
                                                return (VS_ERROR);
                                
                                
                                        /* 免簽名 */
                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "免簽名");
                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                                        if (inRetVal != VS_SUCCESS)
                                                return (VS_ERROR);
                                }
                                else
                                {
                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        if (inRetVal != VS_SUCCESS)
                                                return (VS_ERROR);
                                }

				/* 卡號、檢查碼 */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
                                        /* SmartPay不印檢查碼 */
                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        strcpy(szPrintBuf, pobTran->srBRec.szPAN);
                                        
                                        /* 客製化098，列印卡號要遮掩 */
                                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                                        {
                                                for (j = 6; j < (strlen(szPrintBuf) - 4); j ++)
                                                        szPrintBuf[j] = 0x2A;
                                        } 
                                        
                                        inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 25, _PADDING_RIGHT_);
                                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                        
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, pobTran->srBRec.szPAN);
					
                                        /* 客製化098，列印卡號要遮掩 */
                                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                                        {
                                                for (j = 6; j < (strlen(szPrintBuf) - 4); j ++)
                                                        szPrintBuf[j] = 0x2A;
                                        }   
                                        
                                        inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 19, _PADDING_RIGHT_);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);

					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					memset(szTemplate1, 0x00, sizeof(szTemplate1));
					/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/28 下午 2:42 */
					if (strlen(pobTran->srBRec.szCheckNO) > 0)
					{
						strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
					}
					else
					{
						inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
					}
					sprintf(szPrintBuf, "No.: %s",szTemplate1);
					inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* Trans Date Time */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
				inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				/* Approved No. & RRN NO. */
				/* SmartPay印調單編號 */
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, "RRN NO.:");
					strcat(szPrintBuf, pobTran->srBRec.szFiscRRN);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, "APPR: ");
					strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* RESPONSE CODE */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetCUPFuncEnable(szFuncEnable);
				if (szFuncEnable[0] == 'Y')
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					strcpy(szPrintBuf, "RESPONSE CODE: ");
					strcat(szPrintBuf, pobTran->srBRec.szRespCode);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* Store ID */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetStoreIDEnable(szFuncEnable);
				if (szFuncEnable[0] == 'Y')
				{
					/*開啟櫃號功能*/
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "STORE ID:%s", pobTran->srBRec.szStoreID);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				else
				{
				       /*沒開啟櫃號功能，則不印櫃號*/
				}

				/* 列印優惠資訊 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
				{

				}
				else
				{
					if (inNCCC_Loyalty_ASM_Flag() != VS_SUCCESS)
					{
						
					}
					else
					{
						/* 原交易有優惠資訊，取消交易後，非小費及暫停優惠服務，則列印取消優惠資訊 */
						if (pobTran->srBRec.uszVOIDBit == VS_TRUE	&&
						   (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
						    pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
						    pobTran->srBRec.uszRewardL5Bit == VS_TRUE)	&&
						    atoi(pobTran->srBRec.szAwardNum) > 0	&&
						    pobTran->srBRec.uszRewardSuspendBit != VS_TRUE)
						{
							inRetVal = inPRINT_Buffer_PutIn("取消優惠", inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
						else if (pobTran->srBRec.uszVOIDBit != VS_TRUE		&&
							(pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
							 pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
							 pobTran->srBRec.uszRewardL5Bit == VS_TRUE))
						{
							inRetVal = inPRINT_Buffer_PutIn("優惠", inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
						}
					}
				}
				
			}/* 商店聯卡號遮掩 END */

		}/* 優惠兌換 */

                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	} /* End for () .... */
	
	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DetailReportBottom()_END");
	}
	
        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer
Date&Time       :2017/2/9 下午 4:59
Describe        :
*/
int inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int	inReadCnt = 0;
	int	inRetVal = VS_SUCCESS;
        char	szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];
        char	szFuncEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];/* catch Y or N */
        char	szCustomerIndicator[3 + 1] = {0};
	int	inFontSize_Option = 0;
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer()_START");
	}
	
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inFontSize_Option = _PRT_CUS_123_DETAIL_;
	}
	else
	{
		inFontSize_Option = _PRT_NORMAL2_;
	}
	
	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	/* 預設為無須重找 */
	guszEnormousNoNeedResetBit = VS_TRUE;
	
        for (inReadCnt = 0; inReadCnt < inRecordCnt; inReadCnt ++)
        {
                /*. 開始讀取每一筆交易記錄 .*/
                if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, inReadCnt) != VS_SUCCESS)
                {
                        inRetVal = VS_ERROR;
                        break;
                }
		
		/* 2012-11-07 AM 10:09:10 add by kakab 新增DCC一段式交易(當筆轉台幣，已取消的紀錄不能印明細) */
		if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
	    		continue;

		/* 商店自存聯卡號遮掩 */
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0)
		{
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				sprintf(szTemplate1, "INV.  NO = %06ld", pobTran->srBRec.lnOrgInvNum);
			}
			else
			{
				sprintf(szTemplate1, "*INV. NO = %06ld", pobTran->srBRec.lnOrgInvNum);
			}
			
			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szTemplate1, " <P>");
			}
				
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Amount */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						break;
					case _REFUND_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_:
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					/* NCCC小費不能取消 */
//	                                case _TIP_ :
//	                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
//	                                        break;
					case _REFUND_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
			strcat(szPrintBuf, szTemplate1);
			
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Trans Type */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
			
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 卡號 */
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			strcpy(szTemplate1, pobTran->srBRec.szPAN);

			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
			for (i = 6; i < (strlen(szTemplate1) - 4); i ++)
				szTemplate1[i] = 0x2A;

			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 19, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "CARD  NO = %s",szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 交易序號Transaction No. */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "TXN.  NO = ");
			if (pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
				strcat(szPrintBuf, pobTran->srBRec.szTxnNo);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 檢查碼Check No */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			sprintf(szPrintBuf, "CHECK NO = %s",szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE=%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "TIME=%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Approved No. & RRN NO. */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "APPROVED CODE = ");
			strcat(szPrintBuf, pobTran->srBRec.szAuthCode);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Store ID */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetStoreIDEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				/*開啟櫃號功能*/
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "STORE ID: %s", pobTran->srBRec.szStoreID);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
			       /*沒開啟櫃號功能，則不印櫃號*/
			}

		}
		else
		{
			/* Invoice Number */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				sprintf(szTemplate1, "INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			else
			{
				sprintf(szTemplate1, "*INV:%06ld", pobTran->srBRec.lnOrgInvNum);
			}
			
			/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetESCMode(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
			{
				strcat(szTemplate1, " <P>");
			}
				
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Print Amount */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				switch (pobTran->srBRec.inCode)
				{
					case _SALE_:
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					case _TIP_ :
						sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
						break;
					case _REFUND_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			else
			{
				switch (pobTran->srBRec.inOrgCode)
				{
					case _SALE_:
					case _SALE_OFFLINE_ :
					case _PRE_COMP_ :
					case _PRE_AUTH_ :
						sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
						break;
					/* NCCC小費不能取消 */
//	                                case _TIP_ :
//	                                        sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount));
//	                                        break;
					case _REFUND_ :
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
						break;
					default :
						memset(szTemplate2, 0x00, sizeof(szTemplate2));
						inGetHostLabel(szTemplate2);
						sprintf(szTemplate1, "%s_AMT_VOID_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inCode);
						break;
				} /* End switch () */
			}
			inFunc_Amount_Comma(szTemplate1, "NT$" , 0x00, _SIGNED_NONE_, 16, _PADDING_RIGHT_);
			strcat(szPrintBuf, szTemplate1);
			
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Trans Type */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);
			sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);

                        /* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                        if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(pobTran) == VS_TRUE)
                        {
                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                                if (inRetVal != VS_SUCCESS)
                                        return (VS_ERROR);


                                /* 免簽名 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "免簽名");
                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                                if (inRetVal != VS_SUCCESS)
                                        return (VS_ERROR);
                        }
                        else
                        {
                                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                if (inRetVal != VS_SUCCESS)
                                        return (VS_ERROR);
                        }

			/* 卡號、檢查碼 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, pobTran->srBRec.szPAN);
                        
                        /* 客製化098，列印卡號要遮掩 */
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                        {
                                for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
                                        szPrintBuf[i] = 0x2A;
                        }  
                        
			inFunc_PAD_ASCII(szPrintBuf, szPrintBuf, ' ', 19, _PADDING_RIGHT_);
			
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
			}
			sprintf(szPrintBuf, "No.: %s",szTemplate1);
			
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Trans Date Time */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
			
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
			
			inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Approved No. & RRN NO. */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			strcpy(szPrintBuf, "APPR: ");
			strcat(szPrintBuf, pobTran->srBRec.szAuthCode);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* Store ID */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetStoreIDEnable(szFuncEnable);
			if (szFuncEnable[0] == 'Y')
			{
				/*開啟櫃號功能*/
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "STORE ID:%s", pobTran->srBRec.szStoreID);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
			       /*沒開啟櫃號功能，則不印櫃號*/
			}

		}/* 商店聯卡號遮掩 END */

                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        } /* End for () .... */

	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer()_END");
	}
	
        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_HG_DetailReportMiddle_ByBuffer
Date&Time       :2017/3/8 下午 1:35
Describe        :
*/
int inCREDIT_PRINT_HG_DetailReportMiddle_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szFuncEnable[1 + 1];			/* catch Y or N */
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        inPRINT_Buffer_PutIn("明細報表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

	inPRINT_Buffer_PutIn("調閱編號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易金額", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
        inPRINT_Buffer_PutIn("交易類別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        inPRINT_Buffer_PutIn("卡號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	inPRINT_Buffer_PutIn("交易日期", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn_Specific_X_Position("交易時間", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	
	inPRINT_Buffer_PutIn("授權碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 櫃號功能有開才印櫃號 */
	memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
        inGetStoreIDEnable(szFuncEnable);
        if (szFuncEnable[0] == 'Y')
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inPRINT_Buffer_PutIn("品群碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("櫃號", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}

        inPRINT_Buffer_PutIn("==========================================", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer
Date&Time       :2017/3/8 上午 9:52
Describe        :HG用明細紀錄
*/
int inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int	inRetVal = VS_SUCCESS;
        char	szPrintBuf[62 + 1], szTemplate1[62 + 1], szTemplate2[62 + 1];
        char	szFuncEnable[1 + 1];			/* catch Y or N */
	char	szCustomerIndicator[3 + 1] = {0};
	int	inFontSize_Option = 0;
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_HG_DetailReportBottom_ByBuffer()_START");
	}

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inFontSize_Option = _PRT_CUS_123_DETAIL_;
	}
	else
	{
		inFontSize_Option = _PRT_NORMAL2_;
	}
	
	/* 開始讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
	/* 預設為無須重找 */
	guszEnormousNoNeedResetBit = VS_TRUE;
	
        for (i = 0; i < inRecordCnt; i ++)
        {
                /*. 開始讀取每一筆交易記錄 .*/
                if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, i) != VS_SUCCESS)
                {
                        inRetVal = VS_ERROR;
                        break;
                }
		
		/* Invoice Number */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
		{
			sprintf(szTemplate1, "REF:%s/%06ld", pobTran->srBRec.szHGRefNo, pobTran->srBRec.lnOrgInvNum);
		}
		else
		{
			sprintf(szTemplate1, "*REF:%s/%06ld", pobTran->srBRec.szHGRefNo, pobTran->srBRec.lnOrgInvNum);
		}
		
		/* 【需求單 - 105259】總額明細報表及總額明細查詢補強機制 商店聯出紙本要印<P> */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetESCMode(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0 && pobTran->srBRec.inESCUploadStatus == _ESC_UPLOAD_STATUS_PAPER_)
		{
			strcat(szTemplate1, " <P>");
		}
				
                strcat(szPrintBuf, szTemplate1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		
		/* Print Amount */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szTemplate1, 0x00, sizeof(szTemplate1));

                if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
                {
                        switch (pobTran->srBRec.inHGCode)
			{
				case _HG_FULL_REDEMPTION_:	/* 點數兌換金額帶0 */
					sprintf(szTemplate1, "%d", 0);
                                        break;
				case _HG_POINT_CERTAIN_CASH_:
				case _HG_POINT_CERTAIN_GIFT_PAPER_:
				case _HG_POINT_CERTAIN_CREDIT_:
				case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				case _HG_POINT_CERTAIN_CUP_:
				case _HG_REWARD_CASH_:
				case _HG_REWARD_GIFT_PAPER_:
				case _HG_REWARD_CREDIT_:
				case _HG_REWARD_CREDIT_INSIDE_:
				case _HG_REWARD_CUP_:
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                        break;
				case _HG_ONLINE_REDEEM_CREDIT_:
				case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
				case _HG_ONLINE_REDEEM_CUP_:
				        /* 全額扣抵金額要顯示0 */
					if ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnHGRedeemAmount) == 0L)
						sprintf(szTemplate1, "%d", 0);
					else
						sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);

					break;
				case _HG_ONLINE_REDEEM_CASH_:
				case _HG_ONLINE_REDEEM_GIFT_PAPER_:
				        /* 點數扣抵 - 現金、禮卷 要顯示實際支付金額 */
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGAmount);
					break;
				case _HG_REWARD_REFUND_:
				case _HG_REDEEM_REFUND_:
					/* 不用印金額 */
					break;
				default :
					memset(szTemplate2, 0x00, sizeof(szTemplate2));
					inGetHostLabel(szTemplate2);
                                        sprintf(szTemplate1,"%s_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inHGCode);
					break;
			} /* End switch () */
			
		}
		else
		{
			switch (pobTran->srBRec.inHGCode)
			{
				case _HG_FULL_REDEMPTION_:
					sprintf(szTemplate1, "%d", 0);
                                        break;
				case _HG_POINT_CERTAIN_CASH_:
				case _HG_POINT_CERTAIN_GIFT_PAPER_:
				case _HG_POINT_CERTAIN_CREDIT_:
				case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				case _HG_POINT_CERTAIN_CUP_:
				case _HG_REWARD_CASH_:
				case _HG_REWARD_GIFT_PAPER_:
				case _HG_REWARD_CREDIT_:
				case _HG_REWARD_CREDIT_INSIDE_:
				case _HG_REWARD_CUP_:
					sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
					break;
				case _HG_ONLINE_REDEEM_CREDIT_:
				case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
				case _HG_ONLINE_REDEEM_CUP_:
				        /* 全額扣抵金額要顯示0 */
					if ((pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnHGRedeemAmount) == 0L)
						sprintf(szTemplate1, "%d", 0);
					else
						sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
					break;
                                case _HG_ONLINE_REDEEM_CASH_:
				case _HG_ONLINE_REDEEM_GIFT_PAPER_:
				        /* 點數扣抵 - 現金、禮卷 要顯示實際支付金額 */
					sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGAmount));
					break;
				case _HG_REWARD_REFUND_:
				case _HG_REDEEM_REFUND_:
					/* 不用印金額 */
					break;
				default :
					memset(szTemplate2, 0x00, sizeof(szTemplate2));
					inGetHostLabel(szTemplate2);
                                        sprintf(szTemplate1,"%s_VOID_AMT_ERR_inCode(%d)", szTemplate2, pobTran->srBRec.inHGCode);
					break;
			} /* End switch () */
				
                }
		
		/* 退貨不用印金額 */
		if (strlen(szTemplate1) > 0)
		{
			strcat(szPrintBuf, "NT$ ");
			inFunc_Amount_Comma(szTemplate1, "" , 0x00, _SIGNED_NONE_, 13, _PADDING_RIGHT_);
			strcat(szPrintBuf, szTemplate1);
		}
                
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                /* Trans Type */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                vdCREDIT_PRINT_HG_GetTransactionType(pobTran, szTemplate1, szTemplate2);
		sprintf(szPrintBuf, "%s %s", szTemplate1, szTemplate2);
		/* 【需求單-113172】免簽商店存根聯參數及短簽單調整需求 by Russell 2025/5/15 下午 2:41 */
                    if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(pobTran) == VS_TRUE)
                    {
                            inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                            if (inRetVal != VS_SUCCESS)
                                    return (VS_ERROR);


                            /* 免簽名 */
                            memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                            sprintf(szPrintBuf, "免簽名");
                            inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                            if (inRetVal != VS_SUCCESS)
                                    return (VS_ERROR);
                    }
                    else
                    {
                            inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                            if (inRetVal != VS_SUCCESS)
                                    return (VS_ERROR);
                    }

                /* 卡號 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcat(szPrintBuf, "PAN: ");
		strcat(szPrintBuf, pobTran->srBRec.szHGPAN);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		
                /* Trans Date Time */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "DATE: %.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "TIME: %.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                /* Approved No. */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "APPR:");
		strcat(szPrintBuf, pobTran->srBRec.szHGAuthCode);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_ , _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                /* Store ID */
                memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
                inGetStoreIDEnable(szFuncEnable);
                if (szFuncEnable[0] == 'Y')
                {
                        /*開啟櫃號功能*/
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        sprintf(szPrintBuf, "STORE ID:%s", pobTran->srBRec.szStoreID);
			
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize_Option, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
                }
                else
                {
                       /*沒開啟櫃號功能，則不印櫃號*/
                }
		
		/* HappyGo資訊 */
		inCREDIT_PRINT_HG_DetailReport_HGData_ByBuffer(pobTran, inRecordCnt, uszBuffer, srFont_Attrib, srBhandle);
		
                inPRINT_Buffer_PutIn("-------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        } /* End for () .... */
	
	/* 結束讀取 */
	inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inCREDIT_PRINT_DCC_DetailReportBottom_ByBuffer()_END");
	}
	
	return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_HG_DetailReport_HGData_ByBuffer
Date&Time       :2017/3/8 上午 11:45
Describe        :印HG明細資料
*/
int inCREDIT_PRINT_HG_DetailReport_HGData_ByBuffer(TRANSACTION_OBJECT *pobTran, int inRecordCnt, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
	char	szPrintBuf[62 + 1], szTemplate1[62 + 1];

	/*	HG_FULL_REDEMPTION		點數兌換		HG_PAY_CREDIT		信用卡
		HG_INQUIRY			點數查詢		HG_PAY_CASH		現金
		HG_REWARD			紅利積點		HG_PAY_GIFT_PAPER	禮券
		HG_ONLINE_REDEEM		點數扣抵		HG_PAY_CREDIT_INSIDE	HGI
		HG_POINT_CERTAIN		加價購		HG_PAY_CUP		銀聯卡

		HG_REWARD_CREDIT		紅利積點 + 信用卡
		HG_REWARD_CASH			紅利積點 + 現金
		HG_REWARD_GIFT_PAPER		紅利積點 + 禮券
		HG_REWARD_CREDIT_INSIDE		紅利積點 + HGI
		HG_REWARD_CUP			紅利積點 + 銀聯卡

		HG_ONLINE_REDEEM_CREDIT		點數扣抵 + 信用卡
		HG_ONLINE_REDEEM_CASH		點數扣抵 + 現金
		HG_ONLINE_REDEEM_GIFT_PAPER	點數扣抵 + 禮券
		HG_ONLINE_REDEEM_CREDIT_INSIDE	點數扣抵 + HGI
		HG_ONLINE_REDEEM_CUP		點數扣抵 + 銀聯卡

		HG_POINT_CERTAIN_CREDIT		加價購 + 信用卡
		HG_POINT_CERTAIN_CASH		加價購 + 現金
		HG_POINT_CERTAIN_GIFT_PAPER	加價購 + 禮券
		HG_POINT_CERTAIN_CREDIT_INSIDE	加價購 + HGI
		HG_POINT_CERTAIN_CUP		加價購 + 銀聯卡
	*/

	if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
	{
		if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_)
		{
			/* HG_FULL_REDEMPTION = 點數兌換 */
			/* 合計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			
			strcat(szPrintBuf, "合計　　　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			
			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
		{
			/* HG_CASH = 現金
			   HG_GIFT_PAPER = 禮券 */
			/* 扣抵點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			
			strcat(szPrintBuf, "扣抵點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			
			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_)
		{
			/* 點數扣抵   HG_CASH = 現金
				     HG_GIFT_PAPER = 禮券 */
			/* 商品金額 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "商品金額　　　");
			strcat(szPrintBuf, "NT$ ");
			/* 只列印帳單中的商品金額 */
			if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
			else
				sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount));
			inFunc_Amount_Comma(szTemplate1, "", ' ', _SIGNED_NONE_, 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 點數扣抵金額 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "點數扣抵金額　");
			strcat(szPrintBuf, "NT$ ");
			/* 只列印帳單中的商品金額 */
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRedeemAmount);
			inFunc_Amount_Comma(szTemplate1, "", ' ', _SIGNED_NONE_, 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 扣抵點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "扣抵點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else if (pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_)
		{
			/* 合計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "合計　　　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_REFUND_)
		{
			/* 合計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "合計　　　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
	}
	else	/* 負向交易 */
	{
		if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_)
		{
			/* HG_FULL_REDEMPTION = 點數兌換 */
			/* 合計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			
			strcat(szPrintBuf, "合計　　　　　");
			sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGTransactionPoint));
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			
			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
		{
			/* HG_CASH = 現金
			   HG_GIFT_PAPER = 禮券 */
			/* 扣抵點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			
			strcat(szPrintBuf, "扣抵點數　　　");
			sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGTransactionPoint));
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			
			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_)
		{

			/* 商品金額 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "商品金額　　　");
			strcat(szPrintBuf, "NT$ ");
			/* 只列印帳單中的商品金額 */
			if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
				sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnTxnAmount));
			else
				sprintf(szTemplate1, "%ld", (0 - (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount)));
			inFunc_Amount_Comma(szTemplate1, "NT$ ", ' ', _SIGNED_NONE_, 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 點數扣抵金額 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "點數扣抵金額　");
			strcat(szPrintBuf, "NT$ ");
			/* 只列印帳單中的商品金額 */
			sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGRedeemAmount));
			inFunc_Amount_Comma(szTemplate1, "", ' ', _SIGNED_NONE_, 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 扣抵點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "扣抵點數　　　");
			sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGTransactionPoint));
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else if (pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_)
		{
			/* 合計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "合計　　　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		else if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_REFUND_)
		{
			/* 合計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "合計　　　　　");
			sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGTransactionPoint));
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 剩餘點數 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			memset(szTemplate1, 0x00, sizeof(szTemplate1));

			strcat(szPrintBuf, "剩餘點數　　　");
			sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
			inFunc_PAD_ASCII(szTemplate1, szTemplate1, ' ', 12, _PADDING_LEFT_);
			strcat(szPrintBuf, szTemplate1);
			strcat(szPrintBuf, "　點");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Func7PrintParamTerm_ByBuffer
Date&Time       :2016/2/17 上午 10:47
Describe        :功能7列印參數，雖然這裡用兩個buffer印，但體感上和用一個buffer差不多，仍會等到印完才開始Put in
*/
int inCREDIT_PRINT_Func7PrintParamTerm_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_SUCCESS;
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	
	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_PRINT_Func7PrintParamTerm_ByBuffer START!");
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{

		inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);

		inRetVal = inPRINT_Buffer_PutIn("參數功能列表", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 列印日期 / 時間 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* ParamLOGO_ByBuffer */
		if (inCREDIT_PRINT_ParamLOGO_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* ParamTermInformation_ByBuffer */
		if (inCREDIT_PRINT_ParamTermInformation_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* ParamHostDetailParam_ByBuffer */
		if (inCREDIT_PRINT_ParamHostDetailParam_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【卡別參數檔】【特殊卡別參數檔】【非接觸式】  */
		if (inCREDIT_PRINT_ParamCardType_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【管理號碼】 */
		if (inCREDIT_PRINT_ParamManageNum_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【產品代碼】 */
		if (inCREDIT_PRINT_ProductCode_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【EMV CA Public Key】 */
		if (inCREDIT_PRINT_CAPublicKey_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印【系統參數檔】【共用參數檔】  */
		if (inCREDIT_PRINT_ParamSystemConfig_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		/* ParamLOGO_END_ByBuffer */
		if (inCREDIT_PRINT_ParamLOGO_END_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		if (inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1) != VS_SUCCESS)
			return (VS_ERROR);

		return (VS_SUCCESS);
	}
}

int inCREDIT_PRINT_ParamLOGO_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal;
        char	szFlag[10 + 1];
        char	szTxnType[20 + 1];
	char	szCustomerIndicator[3 + 1] = {0};

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        /* PRINT NCCC LOGO */
	inRetVal = inPRINT_Buffer_PutIn("NCCC LOGO ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		
	}
	else
	{
		inPRINT_Buffer_PutGraphic((unsigned char *)_BANK_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_);
	}

        /* PRINT MERCHANT LOGO */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtMerchantLogo(szFlag);

        if (szFlag[0] == 'Y')
        {
		inRetVal = inPRINT_Buffer_PutIn("LOGO ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inPRINT_Buffer_PutGraphic((unsigned char *)_MERCHANT_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inMerchantLogoHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("LOGO ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* PRINT MERCHANT NAME */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtMerchantName(szFlag);

        if (szFlag[0] == 'Y')
        {
		inRetVal = inPRINT_Buffer_PutIn("表頭 ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_NAME_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inTitleNameHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("表頭 ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* PRINT SLOGAN */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtSlogan(szFlag);

        if (szFlag[0] == 'Y')
        {
		inRetVal = inPRINT_Buffer_PutIn("SLOGAN ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_SLOGAN_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inSloganHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("SLOGAN ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* PRINT 分期付款條文 */
        memset(szFlag, 0x00, sizeof(szFlag));
        memset(szTxnType, 0x00, sizeof(szTxnType));
	
	inLoadHDTRec(0);	
        inGetTransFunc(szTxnType);

        if (szTxnType[7] == 0x59)
        {
		inRetVal = inPRINT_Buffer_PutIn("分期付款條文 ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inInstHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("分期付款條文 ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* PRINT 商店需求提示語 */
        memset(szFlag, 0x00, sizeof(szFlag));
        inGetPrtNotice(szFlag);

        if (szFlag[0] == 'Y')
        {
		inRetVal = inPRINT_Buffer_PutIn("商店需求提示語 ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_NOTICE_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inNoticeHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("商店需求提示語 ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }

        /* CUP 交易警語 */
        memset(szFlag, 0x00, sizeof(szFlag));
	inGetCUPFuncEnable(szFlag);
	
        if (memcmp(szFlag, "Y", strlen("Y")) == 0)
        {
		inRetVal = inPRINT_Buffer_PutIn("CUP交易警語 ：", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inPRINT_Buffer_PutGraphic((unsigned char *)_CUP_LEGAL_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inCupLegalHeight, _APPEND_);
        }
        else
        {
		inRetVal = inPRINT_Buffer_PutIn("CUP交易警語 ： 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
	
        return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamTermInformation_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char	szPrintBuffer[100 + 1];
	char	szTemplate[64 + 1];
	char	szIFESMode[2 + 1] = {0};
	
        inLoadTMSIPDTRec(0);
	inLoadTMSFTPRec(0);
	
	memset(szIFESMode, 0x00, sizeof(szIFESMode));
	inGetI_FES_Mode(szIFESMode);
        
	/* Terminal AP Name */
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szTemplate, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szTemplate);
	}
	sprintf(szPrintBuffer, "VERSION ID = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* Terminal AP Version */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
	if (strlen(gszTermVersionDate) > 0)
	{
		memcpy(szTemplate, gszTermVersionDate, strlen(gszTermVersionDate));
	}
	else
	{
		inGetTermVersionDate(szTemplate);
	}
	sprintf(szPrintBuffer, "VERSION DATE = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* 端末機通訊設定 */
        inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        inPRINT_Buffer_PutIn("端末機通訊設定", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* EDC IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTermIPAddress(szTemplate);
	sprintf(szPrintBuffer, "EDC IP = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* SUBNET MASK */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTermMASKAddress(szTemplate);
	sprintf(szPrintBuffer, "SUBNET MASK = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* DF GATEWAY */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTermGetewayAddress(szTemplate);
	sprintf(szPrintBuffer, "DF GATEWAY = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* TMS IP */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMS_IP_Primary(szTemplate);
	sprintf(szPrintBuffer, "TMS IP = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* PORT NO */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMS_PortNo_Primary(szTemplate);
	sprintf(szPrintBuffer, "Port No = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		/* TMS IP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		inGetTMS_IP_Second(szTemplate);
		sprintf(szPrintBuffer, "TMS IP2 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		/* PORT NO */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		inGetTMS_PortNo_Second(szTemplate);
		sprintf(szPrintBuffer, "Port No2 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* TMS TEL */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMSPhoneNumber(szTemplate);
	sprintf(szPrintBuffer,"TMS TEL = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* FTP IP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetTMS_IP_Primary(szTemplate);
	sprintf(szPrintBuffer, "FTP IP = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* FTP PORT NO */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetFTPPortNum(szTemplate);
	sprintf(szPrintBuffer, "Port No = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        if (memcmp(szIFESMode, "Y", 1) == 0)
	{
		/* FTP IP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		inGetTMS_IP_Second(szTemplate);
		sprintf(szPrintBuffer, "FTP IP2 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	/* PABX CODE */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
        inGetPABXCode(szTemplate);
	sprintf(szPrintBuffer, "PABX CODE = %s", szTemplate);
	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
        inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	
        return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamHostDetailParam_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0, j = 0;
	char	szCommunicationIndex[2 + 1] = {0};
	char	szPrintBuffer[100 + 1] = {0};
	char	szTxnType[20 + 1] = {0};
        char	szTemplate[64 + 1] = {0};
	char	szHostEnable[2 + 1] = {0};
	char	szI_FESMode[1 + 1] = {0};
	
	for (i = 0 ;; i ++)
	{
		if (inLoadHDTRec(i) < 0) /* 主機參數檔 */
			break;
		
		inGetCommunicationIndex(szCommunicationIndex);
		
		if (inLoadCPTRec(atoi(szCommunicationIndex) - 1) < 0)
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			inGetHostLabel(szTemplate);
			sprintf(szPrintBuffer, "(%s)主機無對應通訊參數檔", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			break;
		}
                
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostEnable(szTemplate);
                
                /* 主機功能開關關閉不列印 */
                if (!memcmp(szTemplate, "N", 1))
                        continue;

		/* 主機資料 */
                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                inGetHostLabel(szPrintBuffer);
                inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* Merchant ID */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMerchantID(szTemplate);
	 	sprintf(szPrintBuffer, "MID = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* Terminal ID */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTerminalID(szTemplate);
	 	sprintf(szPrintBuffer, "TID = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 第一授權撥接電話 */
                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostTelPrimary(szTemplate);
	 	sprintf(szPrintBuffer, "TEL #1 = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 第二授權撥接電話 */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostTelSecond(szTemplate);
	 	sprintf(szPrintBuffer, "TEL #2 = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* Call Bank 撥接電話 */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetReferralTel(szTemplate);
	 	sprintf(szPrintBuffer, "TEL #3 = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 第一授權主機 IP Address  */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostIPPrimary(szTemplate);
	 	sprintf(szPrintBuffer, "HOST IP = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 第一授權主機 Port No. */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetHostPortNoPrimary(szTemplate);
	 	sprintf(szPrintBuffer, "PORT NO. = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* IFES 才顯示第二組 */
		memset(szI_FESMode, 0x00, sizeof(szI_FESMode));
		inGetI_FES_Mode(szI_FESMode);
		if (memcmp(szI_FESMode, "Y", strlen("Y")) == 0)
		{
			/* 第二授權主機 IP Address  */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetHostIPSecond(szTemplate);
			sprintf(szPrintBuffer, "HOST IP2 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 第二授權主機 Port No. */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetHostPortNoSecond(szTemplate);
			sprintf(szPrintBuffer, "PORT NO. = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
                /* TPDU */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTPDU(szTemplate);
	 	sprintf(szPrintBuffer, "TPDU = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* NII */
	 	memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetNII(szTemplate);
	 	sprintf(szPrintBuffer, "NII = %s", szTemplate);
	 	inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	 	/* 交易開關 */
		memset(szTxnType, 0x00, sizeof(szTxnType));
		inGetTransFunc(szTxnType);
				inPRINT_Buffer_PutIn("　　　　　　　　　　　開　　關", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetHostLabel(szTemplate);

		if (!memcmp(szTemplate, "HG      ", 8))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			if (szTxnType[0] == 0x59)
				inPRINT_Buffer_PutIn("紅利積點　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("紅利積點　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[1] == 0x59)
				inPRINT_Buffer_PutIn("點數扣抵　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("點數扣抵　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[2] == 0x59)
				inPRINT_Buffer_PutIn("加價購　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("加價購　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[3] == 0x59)
				inPRINT_Buffer_PutIn("點數兌換　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("點數兌換　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[4] == 0x59)
				inPRINT_Buffer_PutIn("點數查詢　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("點數查詢　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[5] == 0x59)
				inPRINT_Buffer_PutIn("回饋退貨　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("回饋退貨　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[6] == 0x59)
				inPRINT_Buffer_PutIn("扣抵退貨　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("扣抵退貨　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[7] == 0x59)
				inPRINT_Buffer_PutIn("紅利積點人工輸入卡號　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("紅利積點人工輸入卡號　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[8] == 0x59)
				inPRINT_Buffer_PutIn("紅利積點列印簽單　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("紅利積點列印簽單　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(szTemplate, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)))
		{
			for (j = 0 ;; j ++)
                	{
                		if (inLoadTDTRec(j) < 0)
				{
                			break;
				}
				
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetTicket_HostName(szTemplate);
				memset(szHostEnable, 0x00, sizeof(szHostEnable));
				inGetTicket_HostEnable(szHostEnable);
                		memset(szTxnType, 0x00, sizeof(szTxnType));
				inGetTicket_HostTransFunc(szTxnType);

		                if (!memcmp(szTemplate, _HOST_NAME_IPASS_, strlen(_HOST_NAME_IPASS_)) && !memcmp(szHostEnable, "Y", strlen(szHostEnable)))
                		{
					inPRINT_Buffer_PutIn("一卡通", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[0] == 0x59)
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[1] == 0x59)
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[2] == 0x59)
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[3] == 0x59)
                				inPRINT_Buffer_PutIn("自動加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("自動加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[4] == 0x59)
						inPRINT_Buffer_PutIn("現金加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[5] == 0x59)
						inPRINT_Buffer_PutIn("現金加值取消　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值取消　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[7] == 0x59)
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[8] == 0x59)
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			        }
                		else if (!memcmp(szTemplate, _HOST_NAME_ECC_, strlen(_HOST_NAME_ECC_)) && !memcmp(szHostEnable, "Y", strlen(szHostEnable)))
                		{
					inPRINT_Buffer_PutIn("悠遊卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[0] == 0x59)
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[1] == 0x59)
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[2] == 0x59)
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[3] == 0x59)
                				inPRINT_Buffer_PutIn("自動加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("自動加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[4] == 0x59)
						inPRINT_Buffer_PutIn("現金加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[5] == 0x59)
						inPRINT_Buffer_PutIn("現金加值取消　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值取消　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[7] == 0x59)
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[8] == 0x59)
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                		}
                		else if (!memcmp(szTemplate, _HOST_NAME_ICASH_, strlen(_HOST_NAME_ICASH_)) && !memcmp(szHostEnable, "Y", strlen(szHostEnable)))
                		{
                		        inPRINT_Buffer_PutIn("愛金卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[0] == 0x59)
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[1] == 0x59)
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("購貨取消　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[2] == 0x59)
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[3] == 0x59)
                				inPRINT_Buffer_PutIn("自動加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("自動加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[4] == 0x59)
						inPRINT_Buffer_PutIn("現金加值　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[5] == 0x59)
						inPRINT_Buffer_PutIn("現金加值取消　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("現金加值取消　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[7] == 0x59)
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("餘額查詢　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                			if (szTxnType[8] == 0x59)
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                			else
						inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				}
			}
		}
		/* DCC 不印補登 */
		else if (!memcmp(szTemplate, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)))
		{
			if (szTxnType[0] == 0x59)
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[1] == 0x59)
				inPRINT_Buffer_PutIn("取消　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("取消　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[2] == 0x59)
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[3] == 0x59)
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[5] == 0x59)
				inPRINT_Buffer_PutIn("預先授權　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("預先授權　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[6] == 0x59)
				inPRINT_Buffer_PutIn("小費　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("小費　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (!memcmp(szTemplate, "NCCC    ", 8))
			{
				if (szTxnType[7] == 0x59)
					inPRINT_Buffer_PutIn("分期付款　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("分期付款　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[8] == 0x59)
					inPRINT_Buffer_PutIn("紅利扣抵　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("紅利扣抵　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[9] == 0x59)
					inPRINT_Buffer_PutIn("分期調帳　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("分期調帳　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[10] == 0x59)
					inPRINT_Buffer_PutIn("紅利調帳　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("紅利調帳　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[11] == 0x59)
					inPRINT_Buffer_PutIn("郵購(MO/TO) 　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("郵購(MO/TO) 　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else if (!memcmp(szTemplate, _HOST_NAME_EW_, strlen(_HOST_NAME_EW_)))
		{
			if (szTxnType[0] == 0x59)
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[3] == 0x59)
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
				inPRINT_Buffer_PutIn("查詢交易　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
                else if (!memcmp(szTemplate, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_)))
		{
			if (szTxnType[0] == 0x59)
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[1] == 0x59)
				inPRINT_Buffer_PutIn("取消　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("取消　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[2] == 0x59)
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			if (szTxnType[0] == 0x59)
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("銷售　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[1] == 0x59)
				inPRINT_Buffer_PutIn("取消　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("取消　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[2] == 0x59)
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("結帳　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[3] == 0x59)
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("退貨　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (!memcmp(szTemplate, "NCCC    ", 8))
			{
				if (szTxnType[4] != 0x30)
					inPRINT_Buffer_PutIn("補登　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("補登　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				if (szTxnType[4] == 0x59)
					inPRINT_Buffer_PutIn("補登　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("補登　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			if (szTxnType[5] == 0x59)
				inPRINT_Buffer_PutIn("預先授權　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("預先授權　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (szTxnType[6] == 0x59)
				inPRINT_Buffer_PutIn("小費　　　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("小費　　　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (!memcmp(szTemplate, "NCCC    ", 8))
			{
				if (szTxnType[7] == 0x59)
					inPRINT_Buffer_PutIn("分期付款　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("分期付款　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[8] == 0x59)
					inPRINT_Buffer_PutIn("紅利扣抵　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("紅利扣抵　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[9] == 0x59)
					inPRINT_Buffer_PutIn("分期調帳　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("分期調帳　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[10] == 0x59)
					inPRINT_Buffer_PutIn("紅利調帳　　　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("紅利調帳　　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

				if (szTxnType[11] == 0x59)
					inPRINT_Buffer_PutIn("郵購(MO/TO) 　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				else
					inPRINT_Buffer_PutIn("郵購(MO/TO) 　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}

		if (!memcmp(szTemplate, _HOST_NAME_EW_, strlen(_HOST_NAME_EW_)))
		{
			
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetManualKeyin(szTemplate);

			if (!memcmp(&szTemplate[0], "Y", 1))
				inPRINT_Buffer_PutIn("人工輸入卡號　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else
				inPRINT_Buffer_PutIn("人工輸入卡號　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetTipPercent(szTemplate);
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "小費限額 = %s", szTemplate);
			strcat(szPrintBuffer, "%");
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

                inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamCardType_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i, inSpace = 20;
	char	szPrintBuffer[100 + 1];
        char	szTemplate[64 + 1];
        char	szLowBinRange[11 + 1], szHighBinRange[11 + 1], szMinPANLength[2 + 1], szMaxPANLength[2 + 1];

	inPRINT_Buffer_PutIn("卡別參數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	for (i = 0 ;; i ++)
	{
		/* 卡別參數檔 */
		if (inLoadCDTRec(i) < 0)
			break;

                /* 卡別 CVV(4DBC) */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCardLabel(szTemplate);
		sprintf(szPrintBuffer, "%02d.%s", (i + 1), szTemplate);
		inFunc_PAD_ASCII(szPrintBuffer, szPrintBuffer, ' ', inSpace, _PADDING_RIGHT_);
		strcat(szPrintBuffer, "CVV(4DBC) = ");
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGet4DBCEnable(szTemplate);

		if (!memcmp(&szTemplate[0], "Y", 1))
			strcat(szPrintBuffer, "ON");
		else
			strcat(szPrintBuffer, "OFF");

		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /* 卡號範圍 卡號長度 */
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szLowBinRange, 0x00, sizeof(szLowBinRange));
                memset(szHighBinRange, 0x00, sizeof(szHighBinRange));
                memset(szMinPANLength, 0x00, sizeof(szMinPANLength));
                memset(szMaxPANLength, 0x00, sizeof(szMaxPANLength));
                inGetLowBinRange(szLowBinRange);
                inGetHighBinRange(szHighBinRange);
                inGetMinPANLength(szMinPANLength);
                inGetMaxPANLength(szMaxPANLength);
		sprintf(szPrintBuffer, "   %s ~ %s   %s ~ %s", szLowBinRange,
		                                               szHighBinRange,
		                                               szMinPANLength,
		                                               szMaxPANLength);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("非接觸卡別", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("　　　　　　　　　　　開　　關", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetVISAPaywaveEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("Visa Paywave　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("Visa Paywave　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetJCBJspeedyEnable(szTemplate);

        if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("JCB Jspeedy 　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("JCB Jspeedy 　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetMCPaypassEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("M/C Paypass 　　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("M/C Paypass 　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetSmartPayContactlessEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("FISC SmartPay 　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("FISC SmartPay 　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetCUPContactlessEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("CUP QuickPass 　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("CUP QuickPass 　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetAMEXContactlessEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("AE ExpressPay 　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("AE ExpressPay 　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetDFS_Contactless_Enable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("DFS           　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("DFS           　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetNCCC_Contactless_Enable(szTemplate);

	if (!memcmp(&szTemplate[0], "Y", 1))
		inPRINT_Buffer_PutIn("TWIN          　　　　● 　　　", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("TWIN          　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetSpecialCardRangeEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "N", 1))
		inPRINT_Buffer_PutIn("特殊卡別參數 : 無", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
	{
		inPRINT_Buffer_PutIn("特殊卡別參數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0 ;; i ++)
		{
			if (inLoadSCDTRec(i) < 0)
				break;

                        /* 活動起始日及結束日 */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			sprintf(szPrintBuffer, "%02d.", (i + 1));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignStartDate(szTemplate);
			strcat(szPrintBuffer, szTemplate);
			strcat(szPrintBuffer, " ~ ");
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignEndDate(szTemplate);
			strcat(szPrintBuffer, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                        /* 卡號範圍 */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetSCDTLowBinRange(szTemplate);
                        sprintf(szPrintBuffer, "   %s ~ ", szTemplate);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetSCDTHighBinRange(szTemplate);
                        strcat(szPrintBuffer, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                        /* 活動代碼 活動限額 */
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignNumber(szTemplate);
                        sprintf(szPrintBuffer, "   Campaign = %s  ", szTemplate);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetCampaignAmount(szTemplate);
                        strcat(szPrintBuffer, szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}

	inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamManageNum_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char			szCheckEnable[2 + 1];

        inLoadPWDRec(0);
	inPRINT_Buffer_PutIn("管理號碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("　　　　　　　　　　　　開　　關", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRebootPwdEnale(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("開機管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("開機管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetSalePwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("銷售管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("銷售管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetPreauthPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("預先授權管理號碼　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("預先授權管理號碼　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetInstallmentPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("分期管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("分期管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRedeemPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("紅利管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("紅利管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetMailOrderPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("郵購管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("郵購管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetOfflinePwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("補登管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("補登管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetInstallmentAdjustPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("分期調帳管理號碼　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("分期調帳管理號碼　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRedeemAdjustPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("紅利調帳管理號碼　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("紅利調帳管理號碼　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetVoidPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("取消管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("取消管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetSettlementPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("結帳管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("結帳管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetRefundPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("退貨管理號碼　　　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("退貨管理號碼　　　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szCheckEnable, 0x00, sizeof(szCheckEnable));
        inGetHGRefundPwdEnable(szCheckEnable);

	if (!memcmp(&szCheckEnable[0], "Y", 1))
		inPRINT_Buffer_PutIn("ＨＧ退貨管理號碼　　　　●　　　 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
		inPRINT_Buffer_PutIn("ＨＧ退貨管理號碼　　　　　　　● ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ProductCode_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int			i;
	char			szPrintBuf[100 + 1];
        char			szTemplate[42 + 1];

	if (inLoadCFGTRec(0) < 0)
	{
		return (VS_ERROR);
	}

        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetProductCodeEnable(szTemplate);

	if (!memcmp(&szTemplate[0], "N", 1))
                inPRINT_Buffer_PutIn("產品代碼 : 無 ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	else
	{
                inPRINT_Buffer_PutIn("產品代碼", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		for (i = 0 ;; i ++)
		{
			if (inLoadPCDRec(i) < 0) /* 產品代碼【PCodeDef.txt】 */
				break;

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetProductCodeIndex(szTemplate);
                        sprintf(szPrintBuf, "%s. ", szTemplate);
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetKeyMap(szTemplate);
                        strcat(szPrintBuf, szTemplate);
                        strcat(szPrintBuf, " ");
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetProductScript(szTemplate);
                        strcat(szPrintBuf, szTemplate);
                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}

	inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_CAPublicKey_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int			i;
	char			szPrintBuffer[100 + 1];
        char			szEMVCAPKIndex[2 + 1], szApplicationId[10 + 1], szCAPKIndex[2 + 1];
	
	inPRINT_Buffer_PutIn("EMV CAPK", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	for (i = 0 ;; i ++)
	{
                if (inLoadESTRec(i) < 0) /* 共用參數檔 */
                        break;

                memset(szEMVCAPKIndex, 0x00, sizeof(szEMVCAPKIndex));
                memset(szApplicationId, 0x00, sizeof(szApplicationId));
                memset(szCAPKIndex, 0x00, sizeof(szCAPKIndex));
                inGetEMVCAPKIndex(szEMVCAPKIndex);
                inGetCAPKApplicationId(szApplicationId);
                inGetCAPKIndex(szCAPKIndex);
                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                sprintf(szPrintBuffer, "%s. %s.%s", szEMVCAPKIndex, szApplicationId, szCAPKIndex);
                inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

        inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamSystemConfig_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char	szPrintBuffer[100 + 1] = {0};
        char	szTemplate[42 + 1] = {0};
        char	szTemplate2[42 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szShort_Receipt_Mode[1 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
	inGetShort_Receipt_Mode(szShort_Receipt_Mode);
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_REGULAR_);
	
	while (1)
	{
                inLoadTMSSCTRec(0);
		inPRINT_Buffer_PutIn("系統參數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetTMSInquireMode(szTemplate);
		sprintf(szPrintBuffer, "TMS詢問模式 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (!memcmp(&szTemplate[0], _TMS_INQUIRE_02_SCHEDHULE_SETTLE_, 1))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetTMSInquireStartDate(szTemplate);
			sprintf(szPrintBuffer, "TMS詢問起始日期 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetTMSInquireTime(szTemplate);
			sprintf(szPrintBuffer, "TMS詢問起始時間 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetTMSInquireGap(szTemplate);
			sprintf(szPrintBuffer, "TMS詢問間隔天數 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		break;
	}

	while (1)
	{
		/* 共用參數檔 */
		if (inLoadCFGTRec(0) < 0)
		{
			/* 共用參數檔不存在 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NO_FILE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
                        inDISP_Msg_BMP(&srDispMsgObj);
			
			inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
			
			return (VS_ERROR);
		}

		inPRINT_Buffer_PutIn("共用參數", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCustomIndicator(szTemplate);
		sprintf(szPrintBuffer, "1.客製化專屬參數 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetNCCCFESMode(szTemplate);
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inGetCloud_MFES(szTemplate2);
		
		if (!memcmp(szTemplate, "01", 2))
			inPRINT_Buffer_PutIn("2.NCCC FES 模式 = 集訊機", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szTemplate, "02", 2))
			inPRINT_Buffer_PutIn("2.NCCC FES 模式 = RFES", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szTemplate, "03", 2))
		{
			if (memcmp(szTemplate2, "Y", strlen("Y")) == 0)
			{
				inPRINT_Buffer_PutIn("2.NCCC FES 模式 = MFES(Cloud)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				inPRINT_Buffer_PutIn("2.NCCC FES 模式 = MFES", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
		}
		else if (!memcmp(szTemplate, "04", 2))
			inPRINT_Buffer_PutIn("2.NCCC FES 模式 = MPAS", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szTemplate, "05", 2))
			inPRINT_Buffer_PutIn("2.NCCC FES 模式 = ATS", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCommMode(szTemplate);

		if (memcmp(szTemplate, _COMM_MODEM_MODE_, 1) == 0)
			inPRINT_Buffer_PutIn("3.通訊模式 = DialUp", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _COMM_ETHERNET_MODE_, 1) == 0)
			inPRINT_Buffer_PutIn("3.通訊模式 = TCP/IP", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _COMM_GPRS_MODE_, 1) == 0)
			inPRINT_Buffer_PutIn("3.通訊模式 = GPRS", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _COMM_3G_MODE_, 1) == 0)
			inPRINT_Buffer_PutIn("3.通訊模式 = 3G", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _COMM_4G_MODE_, 1) == 0)
			inPRINT_Buffer_PutIn("3.通訊模式 = 4G", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetDialBackupEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("4.撥接備援 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("4.撥接備援 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetEncryptMode(szTemplate);

		if (memcmp(szTemplate, _NCCC_ENCRYPTION_NONE_, 1) == 0)
			inPRINT_Buffer_PutIn("5.加密模式 = 不加密", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
			inPRINT_Buffer_PutIn("5.加密模式 = tSAM加密", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _NCCC_ENCRYPTION_SOFTWARE_, 1) == 0)
			inPRINT_Buffer_PutIn("5.加密模式 = 軟體加密", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetSplitTransCheckEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("6.不可連續刷卡 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("6.不可連續刷卡 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCityName(szTemplate);
		sprintf(szPrintBuffer, "7.城市別 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetStoreIDEnable(szPrintBuffer);

		if (!memcmp(szPrintBuffer, "Y", 1))
                {
                        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        inGetMinStoreIDLen(szTemplate);
                        inGetMaxStoreIDLen(szTemplate2);
			sprintf(szPrintBuffer, "8.櫃號 = On   長度 = %s ~ %s", szTemplate, szTemplate2);
                }
		else
                {
                        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        inGetMinStoreIDLen(szTemplate);
                        inGetMaxStoreIDLen(szTemplate2);
			sprintf(szPrintBuffer, "8.櫃號 = Off  長度 = %s ~ %s", szTemplate, szTemplate2);
		}

                inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetECREnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("9.ECR連線= On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("9.ECR連線 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetSupECR_UDP(szTemplate);
		
		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("  UDP ECR連線= On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("  UDP ECR連線 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetPOS_IP(szTemplate);
		sprintf(szPrintBuffer, "  POS IP = \"%s\"", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetECRCardNoTruncateEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("10.ECR卡號遮掩 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("10.ECR卡號遮掩 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetECRExpDateReturnEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("11.ECR卡片有效期回傳 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("11.ECR卡片有效期回傳 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetProductCodeEnable(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("12.列印產品代碼 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("12.列印產品代碼 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetPrtSlogan(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("13.列印商店Slogan = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("13.列印商店Slogan = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (!memcmp(szTemplate, "Y", 1))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szTemplate2, 0x00, sizeof(szTemplate2));
                        inGetSloganStartDate(szTemplate);
                        inGetSloganEndDate(szTemplate2);
			sprintf(szPrintBuffer, "14.Slogan起迄日 = %s ~ %s", szTemplate, szTemplate2);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			memset(szTemplate, 0x00, sizeof(szTemplate));
                        inGetSloganPrtPosition(szTemplate);

			if (atoi(szTemplate) == _NCCC_SLOGAN_PRINT_DOWN_)
				inPRINT_Buffer_PutIn("15.Slogan列印位置 = 簽單下方", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			else if (atoi(szTemplate) == _NCCC_SLOGAN_PRINT_UP_)
				inPRINT_Buffer_PutIn("15.Slogan列印位置 = 簽單上方", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("14.Slogan起迄日 =", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("15.Slogan列印位置 =", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetPrtMode(szTemplate);
		sprintf(szPrintBuffer, "16.簽單列印張數 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCUPRefundLimit(szTemplate);
		sprintf(szPrintBuffer, "17.CUP退貨限額 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCUPKeyExchangeTimes(szTemplate);
		sprintf(szPrintBuffer, "18.CUP安全認證 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetMACEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("19.上傳MAC = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("19.上傳MAC = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetPinpadMode(szTemplate);

		if (!memcmp(szTemplate, "0", 1))
		{
		        inPRINT_Buffer_PutIn("20.密碼機 = None", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(szTemplate, "1", 1))
		{
		        inPRINT_Buffer_PutIn("20.密碼機 = Internal", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else if (!memcmp(szTemplate, "2", 1))
		{
		        inPRINT_Buffer_PutIn("20.密碼機 = External", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetFORCECVV2(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("21.強制輸入CVV2 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("21.強制輸入CVV2 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetElecCommerceFlag(szTemplate);

		if (!memcmp(szTemplate, "00", 2))
			inPRINT_Buffer_PutIn("22.EC參數 = 非郵購及定期性行業", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szTemplate, "01", 2))
			inPRINT_Buffer_PutIn("22.EC參數 = 郵購行業", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szTemplate, "02", 2))
			inPRINT_Buffer_PutIn("22.EC參數 = 定期性行業", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 端末機可下載封包數 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSPacketSize(szTemplate);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		sprintf(szPrintBuffer, "23.端末機可下載封包數 = %s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetDccFlowVersion(szTemplate);

                if (memcmp(szTemplate, _NCCC_DCC_FLOW_VER_NOT_SUPORTED_, 1) == 0)
                        inPRINT_Buffer_PutIn("24.DCC Flow Version = Not Supported DCC", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                else if (memcmp(szTemplate, _NCCC_DCC_FLOW_VER_BY_CARD_BIN_, 1) == 0)
                        inPRINT_Buffer_PutIn("24.DCC Flow Version = By BIN Select", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                else if (memcmp(szTemplate, _NCCC_DCC_FLOW_VER_BY_MANUAL_, 1) == 0)
                        inPRINT_Buffer_PutIn("24.DCC Flow Version = By Currency Select", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetSupDccVisa(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
                        inPRINT_Buffer_PutIn("25.Support DCC VISA = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                else
                        inPRINT_Buffer_PutIn("25.Support DCC VISA = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetSupDccMasterCard(szTemplate);

                if (!memcmp(szTemplate, "Y", 1))
                        inPRINT_Buffer_PutIn("26.Support DCC M/C = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                else
                        inPRINT_Buffer_PutIn("26.Support DCC M/C = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetBarCodeReaderEnable(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("27.BarCode Reader = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("27.BarCode Reader = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetCreditCardFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("28.刷卡兌換 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("28.刷卡兌換 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetCreditCardStartDate(szTemplate);
                inGetCreditCardEndDate(szTemplate2);
		sprintf(szPrintBuffer, "29.刷卡兌換起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetBarCodeFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("30.條碼兌換 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("30.條碼兌換 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetBarCodeStartDate(szTemplate);
                inGetBarCodeEndDate(szTemplate2);
		sprintf(szPrintBuffer, "31.條碼兌換起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetVoidRedeemFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("32.條碼兌換取消 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("32.條碼兌換取消 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetVoidRedeemStartDate(szTemplate);
                inGetVoidRedeemEndDate(szTemplate2);
		sprintf(szPrintBuffer, "33.條碼兌換取消起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
                memset(szTemplate, 0x00, sizeof(szTemplate));
                inGetASMFlag(szTemplate);

		if (!memcmp(szTemplate, "Y", 1))
			inPRINT_Buffer_PutIn("34.優惠功能 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("34.優惠功能 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
                memset(szTemplate2, 0x00, sizeof(szTemplate2));
                inGetASMStartDate(szTemplate);
                inGetASMEndDate(szTemplate2);
		sprintf(szPrintBuffer, "35.優惠功能參數起迄日 = %s~%s", szTemplate, szTemplate2);
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetStore_Stub_CardNo_Truncate_Enable(szTemplate);
		
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetContactlessReaderMode(szTemplate);
		if (memcmp(szTemplate, _CTLS_MODE_1_INTERNAL_, 1) == 0)
			inPRINT_Buffer_PutIn("36.感應 = 內建", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _CTLS_MODE_2_EXTERNAL_, 1) == 0)
			inPRINT_Buffer_PutIn("36.感應 = 外接", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
		        inPRINT_Buffer_PutIn("36.感應 = 無設定", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetPOS_ID(szTemplate);
			sprintf(szPrintBuffer, "37.POS ID : '%s'", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetSignPadMode(szTemplate);
		if (memcmp(szTemplate, _SIGNPAD_MODE_1_INTERNAL_, 1) == 0)
			inPRINT_Buffer_PutIn("38.簽名模式 = 內建", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _SIGNPAD_MODE_2_EXTERNAL_, 1) == 0)
			inPRINT_Buffer_PutIn("38.簽名模式 = 外接", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
		        inPRINT_Buffer_PutIn("38.簽名模式 = 無設定", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSDownloadMode(szTemplate);
		if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, 1) == 0)
			inPRINT_Buffer_PutIn("39.TMS下載模式 = ISO", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, 1) == 0)
			inPRINT_Buffer_PutIn("39.TMS下載模式 = FTP", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* 繳費項目 */
		memset(szTemplate, 0x00, sizeof(szTemplate));;
		inGetPayItemEnable(szTemplate);
		
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("40.Payitem Enable = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("40.Payitem Enable = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* ESC開關 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetESCMode(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
		{
		        inPRINT_Buffer_PutIn("41.ESC Enable = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
		        memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetESCReciptUploadUpLimit(szTemplate);
		        sprintf(szPrintBuffer, "   ESC Limit = %s ", szTemplate);
		        inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
			inPRINT_Buffer_PutIn("41.ESC Enable = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 商店自存聯卡號遮掩 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetStore_Stub_CardNo_Truncate_Enable(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("42.商店自存聯卡號遮掩 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("42.商店自存聯卡號遮掩 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 縮小簽單 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetShort_Receipt_Mode(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("43.縮小簽單功能 = On(Y)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_S_, strlen(_SHORT_RECEIPT_S_)))
			inPRINT_Buffer_PutIn("43.縮小簽單功能 = On(S)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
			inPRINT_Buffer_PutIn("43.縮小簽單功能 = On(U)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("43.縮小簽單功能 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		/* 整合性設備 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetIntegrate_Device(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("44.整合型週邊設備 = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("44.整合型週邊設備 = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* IFES */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetI_FES_Mode(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("45.I-FESMode = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("45.I-FESMode = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* DHCP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetDHCP_Mode(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("46.DHCP = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("46.DHCP = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* 電票功能及自動詢卡優先順序 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		inGetESVC_Priority(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			sprintf(szPrintBuffer, "47.電票順序 = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("47.電票順序 = 無", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
			
		/* CloudMFes */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCloud_MFES(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("48.CloudMFes = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("48.CloudMFes = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* 【需求單 - 108097】Card BIN由ATS查核 add by LingHsiung 2020/2/3 下午 5:19 */
		/* CARD BIN查核 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetBIN_CHECK(szTemplate);
		if (memcmp(szTemplate, "E", strlen("E")) == 0)
			inPRINT_Buffer_PutIn("49.BIN CHECK = EDC", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else if (memcmp(szTemplate, "A", strlen("A")) == 0)
			inPRINT_Buffer_PutIn("49.BIN CHECK = ATS", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("49.BIN CHECK = ???", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* 是否查核要查核病患/入住者ID*/
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCHECK_ID(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("50.CHECK_ID = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("50.CHECK_ID = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* 電子數位化簽單 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetCHESG_FLAG(szTemplate);
		if (memcmp(szTemplate, "Y", strlen("Y")) == 0)
			inPRINT_Buffer_PutIn("51.CHESG_FLAG = On", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("51.CHESG_FLAG = Off", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* 額外自行判別用 */
		/* ECR COMPORT */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRComPort(szTemplate);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		if (strlen(szTemplate) > 0)
		{
			sprintf(szPrintBuffer, "ECR COMPORT = %s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inPRINT_Buffer_PutIn("ECR COMPORT = 無", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		/* RS232 ECR Version */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRVersion(szTemplate);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		if (!memcmp(szTemplate, _ECR_RS232_VERSION_STANDARD_400_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "Standard 400");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_STANDARD_144_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "Standard 144");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_BUMPER_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "Bumper邦柏");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_STANDARD_KIOSK_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "Standard KIOSK");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_CARD_NO_HIDE_F8_B4_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "F8_B4 前8後4");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_DEXIAN_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "DEXIAN 德先");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_NICE_PLAZA_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "NICE_PLAZA 耐斯廣場");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_MCDONALDS_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "MCDONALDS 麥當勞");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_TK3C_EINVOICE_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "TK3C 燦坤3C");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_WELCOME_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "WELCOME 惠康");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_CHUNGHWA_TELECOM_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "CHUNGHWA_TELECOM 中華電信");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_UNIAIR_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "UNIAIR 立榮航空");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_TAICHUNG_SOGO_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "TAICHUNG_SOGO 台中SOGO");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_MIRAMAR_CINEMAS_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "MIRAMAR_CINEMAS 美麗華");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_EDA_144_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "EDA_144 義大144");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_EDA_400_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "EDA_400 義大400");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_CINEMARK_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "CINEMARK 喜滿客");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_082_IKEA_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "082_IKEA 宜家");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_038_BELLAVITA_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "038_Bellavita");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_005_FPG_, _ECR_RS232_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "005_FPG_台塑生醫");
		}
		else if (!memcmp(szTemplate, _ECR_RS232_VERSION_026_TAKA_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s, %s", szTemplate, "026_TAKA_大高");
		}
		else
		{
			sprintf(szPrintBuffer, "ECR RS232 Version = %s", szTemplate);
		}
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		/* UDP ECR Version */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetUDP_ECRVersion(szTemplate);
		memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
		if (!memcmp(szTemplate, _ECR_UDP_VERSION_STANDARD_400_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "Standard 400");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_STANDARD_144_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "Standard 144");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_BUMPER_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "Bumper邦柏");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_STANDARD_KIOSK_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "Standard KIOSK");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_CARD_NO_HIDE_F8_B4_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "F8_B4 前8後4");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_DEXIAN_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "DEXIAN 德先");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_NICE_PLAZA_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "NICE_PLAZA 耐斯廣場");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_MCDONALDS_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "MCDONALDS 麥當勞");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_TK3C_EINVOICE_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "TK3C 燦坤3C");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_WELCOME_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "WELCOME 惠康");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_CHUNGHWA_TELECOM_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "CHUNGHWA_TELECOM 中華電信");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_UNIAIR_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "UNIAIR 立榮航空");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_TAICHUNG_SOGO_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "TAICHUNG_SOGO 台中SOGO");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_MIRAMAR_CINEMAS_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "MIRAMAR_CINEMAS 美麗華");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_EDA_144_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "EDA_144 義大144");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_EDA_400_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "EDA_400 義大400");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_CINEMARK_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "CINEMARK 喜滿客");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_082_IKEA_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "082_IKEA 宜家");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_038_BELLAVITA_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "038_Bellavita");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_005_FPG_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "005_FPG_台塑生醫");
		}
		else if (!memcmp(szTemplate, _ECR_UDP_VERSION_026_TAKA_, _ECR_UDP_VERSION_LEN_))
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s, %s", szTemplate, "026_TAKA_大高");
		}
		else
		{
			sprintf(szPrintBuffer, "ECR UDP Version = %s", szTemplate);
		}
		inPRINT_Buffer_PutIn(szPrintBuffer, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		break;
	}
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);

	return (VS_SUCCESS);
}

int inCREDIT_PRINT_ParamLOGO_END_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (inPRINT_Buffer_PutGraphic((unsigned char *)_END_LOGO_, uszBuffer, srBhandle, 40, _APPEND_) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

	for (i = 0; i < 8; i++)
	{
	   inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_Printing_Time
Date&Time       :2016/10/19 上午 11:08
Describe        :列印"列印時間"
*/
int inCREDIT_PRINT_Printing_Time(TRANSACTION_OBJECT *pobTran, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int		inRetVal;
        char		szPrintBuf[84 + 1], szTemplate[42 + 1];
	RTC_NEXSYS	srRTC;	

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

	memset(&srRTC, 0x00, sizeof(srRTC));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        strcpy(szPrintBuf, "列印時間 :");
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "20%02d/%02d/%02d  ", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
        strcat(szPrintBuf, szTemplate);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        sprintf(szTemplate, "%02d:%02d:%02d", srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
        strcat(szPrintBuf, szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, inFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Func7PrintDCCParamTerm_ByBuffer
Date&Time       :2016/10/17 下午 5:27
Describe        :
*/
int inCREDIT_PRINT_Func7PrintDCCParamTerm_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
        int			i;
	int			inDCC_HostIndex = -1;					/* DCC HDT index */
	int			inRetVal;
	char			szTemplate[60 + 1];					/* 暫存陣列 */
        char			szPrintBuf[384 + 1];
	char			szDCCEnable[2 + 1];
	long			lnFileSize = 0;
	unsigned long		ulHandle;
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;						/* Date & Time */

	vdUtility_SYSFIN_LogMessage(AT, "inCREDIT_PRINT_Func7PrintDCCParamTerm_ByBuffer START!");
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		memset(szDCCEnable, 0x00, sizeof(szDCCEnable));
		inNCCC_DCC_GetDCC_Enable(pobTran->srBRec.inHDTIndex, szDCCEnable);
		if (memcmp(szDCCEnable, "Y", 1) != 0)
		{
			/* DCC分HDT 跟 Config 控制開關  所以兩個都要檢查 */
			/* 此功能已關閉 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

			return (VS_ERROR);
		}

		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* 不使用DCC Flow Version判斷 */

		/* 檢查是否做過【DCC參數下載】檢查是否有舊參數檔 */
		if (inNCCC_DCC_Already_Have_Parameter(pobTran) != VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_DCC_DL_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;
			
			inDISP_Msg_BMP(&srDispMsgObj);

			return (VS_ERROR);
		}

		/* 列印中 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);

		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		/* DCC參數功能列表 */
		inRetVal = inPRINT_Buffer_PutIn("DCC參數功能列表", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 列印時間 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal =inCREDIT_PRINT_Printing_Time(pobTran, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);


		/* Load DCC Host */
		if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCC_HostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}

		/* 主機參數檔 */
		if (inLoadHDTRec(inDCC_HostIndex) < 0)
		{
			return(VS_ERROR);
		}

		inRetVal = inCREDIT_PRINT_Tidmid_ByBuffer(pobTran, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* DCC Bin Ver */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetDCCBinVer(szTemplate);
		sprintf(szPrintBuf, "BIN Table Version = %s", szTemplate);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inFILE_OpenReadOnly(&ulHandle, (unsigned char*)_NCCC_DCC_FILE_EXCHANGE_RATE_SOURCE_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		lnFileSize = lnFILE_GetSize(&ulHandle,(unsigned char*) _NCCC_DCC_FILE_EXCHANGE_RATE_SOURCE_);
		inRetVal = inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inFILE_Read(&ulHandle, (unsigned char*)szPrintBuf, lnFileSize);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inFILE_Close(&ulHandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* Exchange Rate Source */
		inRetVal = inPRINT_Buffer_PutIn("Exchange Rate Source:", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* VISA Disclaimer */
		inRetVal = inPRINT_Buffer_PutIn("VISA Disclaimer:", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* M/C Disclaimer */
		inRetVal = inPRINT_Buffer_PutIn("M/C Disclaimer:", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, &srFont_Attrib, &srBhandle);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* DCC下載狀態結果第5~7項 不需印*/

		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		/* 列印完成 */
		if (inPRINT_Buffer_PutGraphic((unsigned char *)_END_LOGO_, uszBuffer, &srBhandle, 40, _APPEND_) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		for (i = 0; i < 8; i++)
		{
			inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}

		if (inPRINT_Buffer_OutPut(uszBuffer, &srBhandle) != VS_SUCCESS)
			return (VS_ERROR);

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_DCC_InquiryMessage_ByBuffer
Date&Time       :2016/9/2 下午 4:20
Describe        :列印詢價單
*/
int inCREDIT_PRINT_DCC_InquiryMessage_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int			i;
	int			inRetVal;
	int			inIRDU;							/* 表示多少單位的外幣轉換成台幣(ex:0 表示 1 單位外幣、1表示 10 單位外幣) */
	char			szIRDU[12 + 1];						/* Inverted Rate Display Unit */
	char			szOutputAmt[50 + 1];
	char			szPrintBuf[84 + 1];
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		do
		{
			/* Buffer初始化 */
			inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);

			/* Print MERCHANT NAME（表頭圖檔） 384*90 */
			inRetVal = inPRINT_Buffer_PutGraphic((unsigned char *)_NAME_LOGO_, uszBuffer1, &srBhandle1, gsrBMPHeight.inTitleNameHeight, _APPEND_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 列印商店代號、列印端末機代號 */
			inRetVal = inCREDIT_PRINT_Tidmid_ByBuffer(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 列印日期 / 時間 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			/* 列印換算匯率比【一】 */
			/* Inverted Rate Display Unit */
			inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
			memset(szIRDU, 0x00, sizeof(szIRDU));
			strcpy(szIRDU, "1");
			inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);

			/* 列印換算匯率比【二】 */
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szOutputAmt);

			/* 匯率 */
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("Foreign Exchange Rate", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			/* Ex:1 USD = 30.0000 NTD */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szOutputAmt);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 列印換算後交易金額 */
			memset(szOutputAmt, 0x00, sizeof(szOutputAmt));
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szOutputAmt);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "1. = %s", szOutputAmt);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			/* 列印本國貨幣金額 */
			memset(szOutputAmt, 0x00, sizeof(szOutputAmt));
			sprintf(szOutputAmt, "%ld", pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma_DCC(szOutputAmt, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, "0", "TWD", szOutputAmt);

			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "2. = %s", szOutputAmt);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			inPRINT_Buffer_PutIn("Select Currency #1 or #2", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);

			for (i = 0; i < 8; i++)
			{
				inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer1, &srFont_Attrib1, &srBhandle1, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1);

		} while (inRetVal != VS_SUCCESS);
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_DCC_Data_ByBuffer
Date&Time       :2016/9/7 下午 3:10
Describe        :
*/
int inCREDIT_PRINT_DCC_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int     inRetVal;
        char 	szPrintBuf[84 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szExamBit[10 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszChangeLineBit = VS_FALSE;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));

        if (inPrinttype_ByBuffer)
        {
                /* 直式 */

                /*卡別、卡號*/
                sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*日期、時間*/
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "日期　　：%s",pobTran->srBRec.szDate);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "時間　　：%s",pobTran->srBRec.szTime);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*調閱編號、批次號碼 */
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "調閱編號：%06ld",pobTran->srBRec.lnOrgInvNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "批次號碼：%06ld",pobTran->srBRec.lnBatchNum);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*交易類別*/
                inFunc_GetTransType(pobTran, szPrintBuf1, szTemplate2);
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "交易類別：%s",szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                /*授權碼、序號*/
                sprintf(szPrintBuf, "授權碼　：%s",pobTran->srBRec.szAuthCode);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "序號　　：%s",pobTran->srBRec.szRefNo);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        }
        else
        {
                /* 橫式 */

                /* 城市別(City) */
                inRetVal = inPRINT_Buffer_PutIn("城市別(City)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
                inGetCityName(szPrintBuf1);
                sprintf(szPrintBuf, "%s", szPrintBuf1);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 卡別 檢查碼 */
		inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("檢查碼(Check No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		/* 卡別 */
                inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		/* 檢查碼 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		if (strlen(pobTran->srBRec.szCheckNO) > 0)
		{
			strcpy(szTemplate1, pobTran->srBRec.szCheckNO);
		}
		else
		{
			inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate1, _EXP_ENCRYPT_);
		}
                memcpy(&szPrintBuf1[0], szTemplate1, strlen(szTemplate1));
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 卡號 */
                inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, pobTran->srBRec.szPAN);
		
		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		/* 卡號遮掩(一般卡號前6後4，U Card前3後5) */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
				szPrintBuf[i] = 0x2A;
		}
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 商店聯卡號遮掩 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
					szPrintBuf[i] = 0x2A;
			}
			
		}
		
		/* 過卡方式 */
		if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
		{
			if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				strcat(szPrintBuf,"(C)");
			else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
				strcat(szPrintBuf, "(T)");
			else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
			{
				if (pobTran->srBRec.uszManualBit == VS_TRUE)
				{
					/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
					/* 電文轉Manual Keyin但是簽單要印感應的W */
					if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
						strcat(szPrintBuf, "(W)");
					else
						strcat(szPrintBuf,"(M)");
				}
				else
					strcat(szPrintBuf,"(S)");
			}
			
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
				strcat(szPrintBuf, "(W)");
			else
				strcat(szPrintBuf, "(C)");
		}
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 主機別 & 交易別 */
		inRetVal = inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		inGetHostLabel(szTemplate1);
		sprintf(szPrintBuf1, "%s", szTemplate1);

		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		memset(szTemplate2, 0x00, sizeof(szTemplate2));
		inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

		sprintf(szPrintBuf2, "%s", szTemplate1);
		sprintf(szPrintBuf, "%s %s", szPrintBuf1 , szPrintBuf2);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		if (strlen(szTemplate2) > 0)
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate2);

			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
                

                /* 批次號碼、授權碼 */
		inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("授權碼(Auth Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		/* Batch Num */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		/* Auth Code */
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		memcpy(szPrintBuf1, pobTran->srBRec.szAuthCode, _AUTH_CODE_SIZE_);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 日期時間 */
                inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
		
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 序號 調閱編號 */
		inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
                inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

		
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
		inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		
		if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);

                /* 櫃號 */
		memset(szTemplate1, 0x00, sizeof(szTemplate1));
		inGetStoreIDEnable(szTemplate1);
		if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inRetVal = inPRINT_Buffer_PutIn("品群碼(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);

			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 原品群碼 */
				inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				
				char	szPOS_ID[6 + 1] = {0};
				memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
				inGetPOS_ID(szPOS_ID);

				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
				memcpy(&szTemplate1[12], szPOS_ID, 6);
				/* hardcode */
				inRetVal = inPRINT_Buffer_PutIn(szTemplate1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			}

			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		/* 產品代碼 */
		inGetProductCodeEnable(szProductCodeEnable);
		if (memcmp(szProductCodeEnable, "Y", 1) == 0)
		{
			inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
			
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			if (inRetVal != VS_SUCCESS)
				return (VS_ERROR);
		}
        
                /* 【需求單-113260】MasterCard機票交易需求 by Russell 2025/3/3 上午 11:55 */
                if (inNCCC_Func_Flight_Ticket_Allow_Print(pobTran) == VS_TRUE)
		{

                        inRetVal = inPRINT_Buffer_PutIn("出發地機場(departure)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szFlightTicketPDS0523, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn("目的地機場(arrival)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);

                        inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szFlightTicketPDS0524, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        				
                        inRetVal = inPRINT_Buffer_PutIn("航班號碼(Flight No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        
                        
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                        if (strlen(pobTran->srBRec.szFlightTicketPDS0530) > 0)
                        {
                                sprintf(szTemplate2, "%d", atoi(pobTran->srBRec.szFlightTicketPDS0530));
                        }
                        else
                        {
                                sprintf(szTemplate2, "%s", " ");
                        }
                        sprintf(szPrintBuf, "%s", szTemplate2);
                        
                        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                        if (inRetVal != VS_SUCCESS)
                                return (VS_ERROR);
                        				
                        memset(szTemplate1, 0x00, sizeof(szTemplate1));
			inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(pobTran, szTemplate1);

			if (!memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_)))		||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_ , max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_)))	||
			    !memcmp(szTemplate1, _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_, max(strlen(szTemplate1), strlen(_FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_))))
			{
				inRetVal = inPRINT_Buffer_PutIn("交易類型(transaction category)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			else
			{
				inRetVal = inPRINT_Buffer_PutIn("交易類型(transaction category)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", szTemplate1);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_CUS_126_FONT_10X20_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
                }
                
		/* TC */
		memset(szExamBit, 0x00, sizeof(szExamBit));
		inGetExamBit(szExamBit);
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
			    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
			{
				if (pobTran->srEMVRec.in9F26_ApplCryptogramLen > 0)
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "TC:%02X%02X%02X%02X%02X%02X%02X%02X",
									pobTran->srEMVRec.usz9F26_ApplCryptogram[0],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[1],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[2],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[3],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[4],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[5],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[6],
									pobTran->srEMVRec.usz9F26_ApplCryptogram[7]);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				
				/* 商店聯卡號遮掩 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				
				uszChangeLineBit = VS_FALSE;
				/* AID */
				if (strlen(pobTran->srBRec.szCUP_EMVAID) > 0)
				{
					if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
					{
						if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)) == 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)) == 0)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						/* 【需求單-111290】VISA置換Logo、DISCOVER簽單新增AID欄位、AE簽單新增AID和AP Label欄位需求 by Russell 2022/12/29 下午 5:57 */
						else if (memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_DINERS_, strlen(_CARD_TYPE_DINERS_)) == 0 &&
							pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
					}
					else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
					{
						/* AE晶片要印出AID */
						if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_50_EXPRESSSPAY || 
						    pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_52_EXPRESSSPAY_MAG_STRIPE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						else if (pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_20_PAYPASS_MAG_STRIPE || 
							 pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_21_PAYPASS_MCHIP)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						else if ((pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_42_DPAS	|| pobTran->srBRec.uszWAVESchemeID == SCHEME_ID_43_DISCOVER_DPAS_MAG_STRIPE) &&
							pobTran->srBRec.uszUCARDTransBit != VS_TRUE)
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
						/* 驗測要印AP Label 和 AID */
						else if (szExamBit[0] == '1')
						{
							memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
							sprintf(szPrintBuf, "AID:%s", pobTran->srBRec.szCUP_EMVAID); /* MVT中比對到的 */
							inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
							if (inRetVal != VS_SUCCESS)
								return (VS_ERROR);
							uszChangeLineBit = VS_TRUE;
						}
					}
				}

				/* M/C交易列印AP Lable (START) */
				if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_)))
				{
					if (pobTran->srEMVRec.in50_APLabelLen > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:");
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				/* M/C交易列印AP Lable (END) */
				else if (!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_AMEX_, strlen(_CARD_TYPE_AMEX_)))
				{
					if (pobTran->srEMVRec.in50_APLabelLen > 0)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
					else
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, "AP Label:");
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				/* 驗測要印AP Label 和 AID */
				else if (szExamBit[0] == '1')
				{
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "AP Label:%s", pobTran->srEMVRec.usz50_APLabel); /* 去掉 Tag Len , 直接拿 Value */
					inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
				/* 一定要換行 */
				else
				{
					if (uszChangeLineBit == VS_TRUE)
					{
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						sprintf(szPrintBuf, " ");
						inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_07_);
						if (inRetVal != VS_SUCCESS)
							return (VS_ERROR);
					}
				}
				
				/* 銀聯沒有DCC */
			}
			else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
			{
				/* 商店聯卡號遮掩 */
				/* 持卡人存根也要印 */
				memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
				inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
				if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
				    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
				    strlen(pobTran->srBRec.szTxnNo) > 0)
				{
					inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
					memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
					sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
					inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					if (inRetVal != VS_SUCCESS)
						return (VS_ERROR);
				}
			}
			
		}
		else
		{
			/* 商店聯卡號遮掩 */
			/* 磁條卡列印交易編號 */
			memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
			inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
			if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
			    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
			    strlen(pobTran->srBRec.szTxnNo) > 0)
			{
				inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
		}
		
		/* 斷行 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (inRetVal != VS_SUCCESS)
                        return (VS_ERROR);
        }

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_DCC_Amount_ByBuffer
Date&Time       :2016/9/8 上午 11:51
Describe        :
*/
int inCREDIT_PRINT_DCC_Amount_FOR_SALE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
	int	inIRDU = 0;
	char	szPrintBuf[84 + 1] = {0}, szPrintBuf1[42 + 1] = {0}, szTemplate[42 + 1] = {0};
	char	szIRDU[12 + 1] = {0};			/* Inverted Rate Display Unit */
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */
	
	inRetVal = inPRINT_Buffer_PutIn("----------------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("         Please select          ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn(" Transaction currency as below  ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 斷行 */
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 外幣 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, "", szPrintBuf1);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "1.【X】%s %s", pobTran->srBRec.szDCC_FCAC, szPrintBuf1);						/* Foreign currcncy Alphabetic Code */

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 匯率 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "Exchange Rate:");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	
	memset(szExchangeAmt, 0x00, sizeof(szExchangeAmt));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 轉換費率 */
	inRetVal = inPRINT_Buffer_PutIn("轉換費率(Currency Conversion Fee)", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* XX.XX %*/
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %%", szPrintBuf1);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	inRetVal = inPRINT_Buffer_PutIn("(Currency Conversion Fee on the", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("exchange rate over a wholesale rate.)", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 台幣上須空一行 by Russell 2020/10/29 下午 4:30 */
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 台幣 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTxnAmount);
	inFunc_Amount_Comma_DCC(szPrintBuf1, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, "0", "", szPrintBuf1);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 轉台幣跑另外一隻 */
	sprintf(szPrintBuf, "2.【 】%s %s", "NTD", szPrintBuf1);									/* Foreign currcncy Alphabetic Code */

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 有開小費要多印 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTransFunc(szTemplate);
	/* 檢查是否有開小費 */
	/* 預先授權完成必定不印小費 */
	if (szTemplate[6] == 'Y'	&&
	    pobTran->srBRec.inCode == _SALE_)
	{
		/* 斷行 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
		/* 商店聯不用印貨幣名稱 */
		if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 小費 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "小費(Tips)   : _____________________");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 總計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "總計(Total)  : _____________________");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			/* 小費 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "小費(Tips)   : %s_____________________", pobTran->srBRec.szDCC_FCAC);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 總計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "總計(Total)  : %s_____________________", pobTran->srBRec.szDCC_FCAC);
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
	}
	else
	{
		/* 不開小費不印下面這一段 */
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer
Date&Time       :2016/9/12 下午 2:17
Describe        :列印結尾
*/
int inCREDIT_PRINT_DCC_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_ERROR;
	char	szTemplate[42 + 1] = {0};
	char	szSignature[30 + 1] = {0};
	char	szSignaturePath[80 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
		/* 免責宣言上需空一行 by Russell 2020/10/29 下午 3:21 */
		inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_)
                {
			
			/* Disclaimer */
			/* 轉台幣也要印 */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) &&
				 (pobTran->srBRec.inCode == _SALE_	|| 
				  pobTran->srBRec.inCode == _TIP_	|| 
				  pobTran->srBRec.inCode == _PRE_COMP_	||
			         (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Cardholder expressly agrees to the Transaction ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Receipt Information by marking the “accept box”", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("below.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("[ ] ACCEPT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}
			
                        /* 簽名欄 */
			/* 免簽名 */
			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			/* 要簽名 */
			else
			{
				/* 藉由TRT_FileName比對來組出bmp的檔名 */
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				
				memset(szSignature, 0x00, sizeof(szSignature));
				/* 因為用invoice所以不用inFunc_ComposeFileName */
				inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
				memset(szSignaturePath, 0x00, sizeof(szSignaturePath));
				sprintf(szSignaturePath, "./fs_data/%s", szSignature);
				/* 圖檔存在、有在signpad簽名、且非重印（重印不出簽名） （目前簽名狀態存不了Batch，先把&& pobTran->srBRec.inSignStatus == _SIGN_SIGNED_此條件拿掉） */
				if (inFILE_Check_Exist((unsigned char *)szSignature) == VS_SUCCESS && pobTran->inRunOperationID != _OPERATION_REPRINT_)
				{
					/* 電子簽名 */
					inPRINT_Buffer_PutGraphic((unsigned char *)szSignaturePath, uszBuffer, srBhandle, _SIGNEDPAD_WIDTH_, _APPEND_);
				}
				/* 手簽 */
				else
				{
					/* a space 2 line */
					for (i = 0; i < 2; i++)
					{
						inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
					}
					
				}
				
			}
                        
                        inPRINT_Buffer_PutIn("X:________________________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("持卡人簽名(特店存根聯)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
                }
		else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			/* Disclaimer */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE && pobTran->srBRec.inCode == _TIP_)
			{
				inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Cardholder expressly agrees to the Transaction ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("Receipt Information by marking the “accept box”", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("below.", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
				inRetVal = inPRINT_Buffer_PutIn("[ ] ACCEPT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}

				}
			}

			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("　　　　　　　　　 商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
                else
                {
			/* Disclaimer */
			if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_VISA_, strlen(_CARD_TYPE_VISA_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_VISA_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
			else if ((!memcmp(pobTran->srBRec.szCardLabel, _CARD_TYPE_MASTERCARD_, strlen(_CARD_TYPE_MASTERCARD_))) && 
			    (pobTran->srBRec.inCode == _SALE_		|| 
			     pobTran->srBRec.inCode == _TIP_		|| 
			     pobTran->srBRec.inCode == _PRE_COMP_	||
			    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)))
			{
				if (inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle) == VS_ERROR)
					return (VS_ERROR);
			}
						
			/* 教育訓練模式 */
			memset(szDemoMode, 0x00, sizeof(szDemoMode));
			inGetDemoMode(szDemoMode);
			if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
			{
				if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
					}
				}
			}

			if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
			{
				/* 免簽名 */
				inRetVal = inPRINT_Buffer_PutIn("免      簽      名", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
			}
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        inPRINT_Buffer_PutIn("持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("Card holder stub", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			inPRINT_Buffer_PutIn("--------------------------------------------------------------------------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			/* 持卡人姓名 */
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate,"%s",pobTran->srBRec.szCardHolder);
			inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
                /* 警示語 */
                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		
		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
		
		/* Print Slogan */
		if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
		{
			if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
				return (VS_ERROR);
		}
		
		
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }

        return (VS_SUCCESS);
	
}

/*
Function        :inCREDIT_PRINT_DCC_Disclaimer_ByBuffer
Date&Time       :2016/9/12 下午 2:56
Describe        :印Disclaimer
*/
int inCREDIT_PRINT_DCC_Disclaimer_ByBuffer(char *szFilename, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int		i;
	int		inOffset;				/* 偏移 */
	int		inRetVal;
	int		inDataSegament;				/* 一次讀多少 */
	int		inPrintHandle = 0;			/* put in 到哪裡 */
	int		inLineLength = 0;			/* 一行印得下多少char */
	long		lnDataLength;				/* 資料長度 */
	long            lnReadLength;                           /* 剩餘讀取長度 */
	char		szPrintTemp[50 + 1];			/* 放印一行的陣列 */
	unsigned long	ulFile_Handle;
        unsigned char   *uszTemp;                               /* 暫存，放整筆CDT檔案 */

        if (inFILE_OpenReadOnly(&ulFile_Handle, (unsigned char *)szFilename) == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        /*
         * get data size 為了接下來動態宣告記憶體的大小需要知道多少 需先取得檔案Size
         * get size 失敗時，if條件成立，關檔並回傳VS_ERROR
         */
        lnDataLength = lnFILE_GetSize(&ulFile_Handle, (unsigned char *)szFilename);

        if (lnDataLength == VS_ERROR)
        {
                /* GetSize失敗 ，關檔 */
                inFILE_Close(&ulFile_Handle);

                return (VS_ERROR);
        }

        /*
         * allocate 記憶體
         * allocate時多分配一個byte以防萬一（ex:換行符號）
         */
        uszTemp = malloc(lnDataLength + 1);
        /* 初始化 uszTemp uszReadData */
        memset(uszTemp, 0x00, lnDataLength + 1);

	/* 算出一行可以放幾個char (PAPER_X_SIZE = 384) */
	inLineLength = PAPER_X_SIZE / (inFontSize / 0X0100);
	
	/*一次讀1024 */
	inDataSegament = 1024;
	
        /* seek 到檔案開頭 & 從檔案開頭開始read */
        if (inFILE_Seek(ulFile_Handle, 0, _SEEK_BEGIN_) == VS_SUCCESS)
        {
                lnReadLength = lnDataLength;

                for (i = 0;; ++i)
                {
                        /* 剩餘長度大於或等於inLineLength */
                        if (lnReadLength >= inDataSegament)
                        {
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[inDataSegament * i], inDataSegament) == VS_SUCCESS)
                                {
                                        /* 一次讀inDataSegament */
                                        lnReadLength -= inDataSegament;

                                        /* 當剩餘長度剛好為inLineLength，會剛好讀完 */
                                        if (lnReadLength == 0)
                                                break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                        /* 剩餘長度小於inLineLength */
                        else if (lnReadLength < inDataSegament)
                        {
                                /* 就只讀剩餘長度 */
                                if (inFILE_Read(&ulFile_Handle, &uszTemp[inDataSegament * i], inDataSegament) == VS_SUCCESS)
                                {
                                        break;
                                }
                                /* 讀失敗時 */
                                else
                                {
                                        /* Close檔案 */
                                        inFILE_Close(&ulFile_Handle);

                                        /* Free pointer */
                                        free(uszTemp);

                                        return (VS_ERROR);
                                }
                        }
                }/* end for loop */
        }
        /* seek不成功時 */
        else
        {
                /* 關檔並回傳 */
                inFILE_Close(&ulFile_Handle);
                /* Free pointer */
                free(uszTemp);

                /* Seek失敗，所以回傳Error */
                return (VS_ERROR);
        }

	/* 剩餘讀取長度 */
	lnReadLength = lnDataLength;
	
	while (inPrintHandle < (int)lnDataLength)
	{
		/* 為了去除句首的空白 */
		while (uszTemp[inPrintHandle] == 0x20)
		{
			inPrintHandle++;
			lnReadLength--;
		}
		
		/* 確認句末是否會斷到字*/
		if (lnReadLength >= inLineLength)
		{
			/* 不能斷行的狀況 */
			if (uszTemp[inPrintHandle + inLineLength] != 0x20 && uszTemp[inPrintHandle + inLineLength - 1] != 0x20)
			{
				memset(szPrintTemp, 0x00, sizeof(szPrintTemp));
				inOffset = 0;
				
				/* 若句末為非空白，則往前找直到可以段句不斷字 */
				while(uszTemp[inPrintHandle + inLineLength - inOffset] != 0x20)
				{
					inOffset++;
					/* 萬一整行都沒空白，直接印 */
					if ((inLineLength - inOffset) == 0)
					{
						inOffset = 0;
						break;
					}
				}
				memcpy(szPrintTemp, &uszTemp[inPrintHandle], (int)inLineLength - inOffset);
				inPrintHandle += inLineLength - inOffset;
				lnReadLength -= inLineLength - inOffset;
			}
			/* 可以直接斷行的狀況 */
			else
			{
				memset(szPrintTemp, 0x00, sizeof(szPrintTemp));
				memcpy(szPrintTemp, &uszTemp[inPrintHandle], (int)inLineLength);
				inPrintHandle += inLineLength;
				lnReadLength -= inLineLength;
			}
		}
		/* 只剩不到一行的狀況 */
		else
		{
			memset(szPrintTemp, 0x00, sizeof(szPrintTemp));
			memcpy(szPrintTemp, &uszTemp[inPrintHandle], (int)lnReadLength);
			inPrintHandle += lnReadLength;
			lnReadLength -= lnReadLength;
		}
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintTemp, inFontSize, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
		{
			/* [BUG] 移除檔案同步API SYNC()指令 
			 * [20260506] 補上列印失敗時進行 free() */
			free(uszTemp);
			return (VS_ERROR);
		}
		
	}
	
	
	/* release */
        /* 關檔 */
        inFILE_Close(&ulFile_Handle);

        /* Free pointer */
        free(uszTemp);
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer
Date&Time       :2017/2/8 下午 2:44
Describe        :DCC 一段式 非SALE的其他交易
*/
int inCREDIT_PRINT_DCC_Amount_NOT_FOR_SALE_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
	int	inIRDU = 0;
	char	szIRDU[12 + 1] = {0};		/* Inverted Rate Display Unit */
	char	szOutputAmt[50 + 1] = {0};
	char	szPrintBuf[84 + 1] = {0}, szPrintBuf1[42 + 1] = {0}, szTemplate[42 + 1] = {0};
	long	lnTipTotalAmt = 0l;
		
	/* 台幣 */
	inRetVal = inPRINT_Buffer_PutIn("台幣(Local Amount)", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	if (pobTran->srBRec.inCode == _TIP_)
	{
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf1, "%ld", 0 - pobTran->srBRec.lnTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf1, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "Amount:NTD %s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf1, "%ld", 0 - pobTran->srBRec.lnTipTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTipTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf1, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "Tips  :NTD %s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)	|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf1, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf1, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "Total :NTD %s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)		|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			sprintf(szPrintBuf1, "%ld", 0 - pobTran->srBRec.lnTotalTxnAmount);
		}
		else
		{
			sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTotalTxnAmount);
		}
		inFunc_Amount_Comma_DCC(szPrintBuf1, "", '\x00', _SIGNED_NONE_, 13, _PADDING_RIGHT_, "0", "", szPrintBuf1);

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "Total :NTD %s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 斷行 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	
	if (pobTran->srBRec.inCode == _TIP_)
	{
		/* 金額 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		inPRINT_Buffer_PutIn("金額(Amount) :", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)		|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 小費 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		inPRINT_Buffer_PutIn("小費(Tips)   :", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)		|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_TIPFCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 總計 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		inPRINT_Buffer_PutIn("總計(Total)  :", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("Total amount of Transaction Currency", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		/* 初始化 */
		lnTipTotalAmt = 0;
		memset(szTemplate, 0x00, sizeof(szTemplate));
		lnTipTotalAmt += atol(pobTran->srBRec.szDCC_FCA);
		lnTipTotalAmt += atol(pobTran->srBRec.szDCC_TIPFCA);
		sprintf(szTemplate, "%ld", lnTipTotalAmt);
		
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)		|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(szTemplate, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}
		else
		{
			inFunc_Amount_Comma_DCC(szTemplate, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		/* 總計 */
		/* 初始化 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		inPRINT_Buffer_PutIn("總計(Total)  :", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("Total amount of Transaction Currency", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		
		memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
		if ((pobTran->srBRec.uszVOIDBit == VS_TRUE && pobTran->srBRec.inOrgCode != _REFUND_)		|| 
		     pobTran->srBRec.inCode == _REFUND_)
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "-", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}
		else
		{
			inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, pobTran->srBRec.szDCC_FCAC, szPrintBuf1);
		}

		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szPrintBuf1);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* Exchange Rate: 1 EUR = 39.52 NTD 上需空一行 by Russell 2020/10/29 下午 5:16 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	/* 列印換算匯率比【一】 */
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);

	/* 列印換算匯率比【二】 */
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szOutputAmt);

	/* 匯率 */
	inRetVal = inPRINT_Buffer_PutIn("Exchange Rate:", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* Ex:1 USD = 30.0000 NTD */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szOutputAmt);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 轉換費率 */
	inRetVal = inPRINT_Buffer_PutIn("轉換費率(Currency Conversion Fee)", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* XX.XX %*/
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %%", szPrintBuf1);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("(Currency Conversion Fee on the", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("exchange rate over a wholesale rate.)", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer
Date&Time       :2017/2/8 下午 2:43
Describe        :DCC 一段式詢價後轉台幣
*/
int inCREDIT_PRINT_DCC_Amount_CHANGE_NTD_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_ERROR;
	int	inIRDU = 0;
	char	szPrintBuf[84 + 1] = {0}, szPrintBuf1[42 + 1] = {0}, szTemplate[42 + 1] = {0};
	char	szIRDU[12 + 1] = {0};			/* Inverted Rate Display Unit */
	char	szExchangeAmt[50 + 1] = {0};		/* 轉換金額 */

	inRetVal = inPRINT_Buffer_PutIn("----------------------------------------------------", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("         Please select          ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn(" Transaction currency as below  ", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 斷行 */
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 外幣 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_FCA, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_FCMU, "", szPrintBuf1);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "1.【 】%s %s", pobTran->srBRec.szDCC_FCAC, szPrintBuf1);						/* Foreign currcncy Alphabetic Code */
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 匯率 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "Exchange Rate:");
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* Inverted Rate Display Unit */
	inIRDU = atoi(pobTran->srBRec.szDCC_IRDU); 
	memset(szIRDU, 0x00, sizeof(szIRDU));
	strcpy(szIRDU, "1");
	inFunc_PAD_ASCII(szIRDU, szIRDU, '0', (inIRDU + 1), _PADDING_RIGHT_);
	
	memset(szExchangeAmt, 0x00, sizeof(szExchangeAmt));
	inFunc_Amount_Comma_DCC(pobTran->srBRec.szDCC_IRV, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, pobTran->srBRec.szDCC_IRMU, "", szExchangeAmt);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %s = %s NTD", szIRDU, pobTran->srBRec.szDCC_FCAC, szExchangeAmt);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 轉換費率 */
	inRetVal = inPRINT_Buffer_PutIn("轉換費率(Currency Conversion Fee)", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* XX.XX %*/
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inFunc_Currency_Conversion_Fee(pobTran->srBRec.szDCC_MPV, pobTran->srBRec.szDCC_MPDP, szPrintBuf1);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%s %%", szPrintBuf1);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	inRetVal = inPRINT_Buffer_PutIn("(Currency Conversion Fee on the", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("exchange rate over a wholesale rate.)", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 台幣上須空一行 by Russell 2020/10/29 下午 4:30 */
	inRetVal = inPRINT_Buffer_PutIn("  ", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 台幣 */
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%ld", pobTran->srBRec.lnTxnAmount);
	inFunc_Amount_Comma_DCC(szPrintBuf1, "", 0x00, _SIGNED_NONE_, 20, _PADDING_RIGHT_, "0", "", szPrintBuf1);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "2.【X】%s %s", "NTD", szPrintBuf1);									/* Foreign currcncy Alphabetic Code */

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 有開小費要多印 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTransFunc(szTemplate);
	/* 檢查是否有開小費 */
	/* 預先授權完成必定不印小費 */
	if (szTemplate[6] == 'Y'	&&
	    (pobTran->srBRec.inCode == _SALE_ && pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE))
	{
		/* 斷行 */
		inRetVal = inPRINT_Buffer_PutIn("", _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
		/* 商店聯不用印貨幣名稱 */
		if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
		{
			/* 小費 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "小費(Tips)   : _____________________");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 總計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "總計(Total)  : _____________________");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			/* 小費 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "小費(Tips)   : %s_____________________", "NTD");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			/* 總計 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "總計(Total)  : %s_____________________", "NTD");
			inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_10X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		/* 不開小費不印下面這一段 */
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_FISC_Data_ByBuffer
Date&Time       :2016/11/23 下午 3:58
Describe        :
*/
int inCREDIT_PRINT_FISC_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i;
	int     inRetVal;
	char 	szPrintBuf[84 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szStore_Stub_CardNo_Truncate_Enable[2 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	
	/* 發卡行代碼 */
	inRetVal = inPRINT_Buffer_PutIn("發卡行代碼", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memcpy(szPrintBuf1, &pobTran->srBRec.szFiscIssuerID[0], 3);
	sprintf(szPrintBuf, "%s", szPrintBuf1);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	
	/* 卡別 */
	inRetVal = inPRINT_Buffer_PutIn("卡別(Card Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 卡別 */
	inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szCardLabel, ' ', 12, _PADDING_RIGHT_);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	/* 卡號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcpy(szPrintBuf, pobTran->srBRec.szPAN);
	inRetVal = inPRINT_Buffer_PutIn("卡號(Card No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	strcpy(szPrintBuf, pobTran->srBRec.szPAN);
	/* 卡號遮掩 */
	if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
	{
		for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
			szPrintBuf[i] = 0x2A;
	}
	else if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		 pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
	{
		/* 商店聯卡號遮掩 */
		memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
		inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
		if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0 && pobTran->srBRec.uszTxNoCheckBit == VS_TRUE)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			for (i = 6; i < (strlen(szPrintBuf) - 4); i ++)
				szPrintBuf[i] = 0x2A;
		}

	}

	/* 過卡方式 */
	if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
	{
		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
			strcat(szPrintBuf,"(C)");
		else if (pobTran->srBRec.uszMobilePayBit == VS_TRUE)
			strcat(szPrintBuf, "(T)");
		else if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szPrintBuf, "(W)");
		else
		{
			if (pobTran->srBRec.uszManualBit == VS_TRUE)
			{
				/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
				/* 電文轉Manual Keyin但是簽單要印感應的W */
				if (pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
					strcat(szPrintBuf, "(W)");
				else
					strcat(szPrintBuf,"(M)");
			}
			else
				strcat(szPrintBuf,"(S)");
		}

	}
	else
	{
		if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
			strcat(szPrintBuf, "(W)");
		else
			strcat(szPrintBuf, "(C)");
	}

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 主機別 & 交易別 */
	inRetVal = inPRINT_Buffer_PutIn("主機別/交易類別(Host/Trans.Type)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	inGetHostLabel(szTemplate1);
	sprintf(szPrintBuf1, "%s", szTemplate1);

	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	memset(szTemplate2, 0x00, sizeof(szTemplate2));
	inFunc_GetTransType(pobTran, szTemplate1, szTemplate2);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szPrintBuf2, 0x00, sizeof(szPrintBuf2));

	sprintf(szPrintBuf2, "%s", szTemplate1);
	sprintf(szPrintBuf, "%s %s", szPrintBuf1 , szPrintBuf2);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	if (strlen(szTemplate2) > 0)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate2);
		
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 批次號碼 */
	inRetVal = inPRINT_Buffer_PutIn("批次號碼(Batch No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* Batch Num */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%03ld", pobTran->srBRec.lnBatchNum);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 日期時間 */
	inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);

	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 序號 調閱編號 */
	inRetVal = inPRINT_Buffer_PutIn("序號(Ref. No.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("調閱編號(Inv.No)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);


	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
	inFunc_PAD_ASCII(szPrintBuf, pobTran->srBRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%06ld", pobTran->srBRec.lnOrgInvNum);
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
		
	/* 調單編號 */
	inRetVal = inPRINT_Buffer_PutIn("調單編號(RRN NO.)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memcpy(szPrintBuf, pobTran->srBRec.szFiscRRN, _FISC_RRN_SIZE_);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);



	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inPRINT_Buffer_PutIn("品群碼(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 原品群碼 */
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
			/* hardcode */
			inRetVal = inPRINT_Buffer_PutIn(szTemplate1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}

		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* 商店聯卡號遮掩 */
	memset(szStore_Stub_CardNo_Truncate_Enable, 0x00, sizeof(szStore_Stub_CardNo_Truncate_Enable));
	inGetStore_Stub_CardNo_Truncate_Enable(szStore_Stub_CardNo_Truncate_Enable);
	if (memcmp(szStore_Stub_CardNo_Truncate_Enable, "Y", strlen("Y")) == 0	&& 
	    pobTran->srBRec.uszTxNoCheckBit == VS_TRUE				&&
	    strlen(pobTran->srBRec.szTxnNo) > 0)
	{
		inRetVal = inPRINT_Buffer_PutIn("交易編號(Transaction No.):", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", pobTran->srBRec.szTxnNo);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_FISC_Amount_ByBuffer
Date&Time       :2016/11/23 下午 3:58
Describe        :
 */
int inCREDIT_PRINT_FISC_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1];

        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	/* 橫式 */
        /* 負向交易 */
	if(pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		/* 橫式 */
                /* 金額 */
                /* 取消退貨是正數 */
                if (pobTran->srBRec.inOrgCode == _FISC_REFUND_)
                {
                        /* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
			
                        /* 把前面的字串和數字結合起來 */
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		else
		{
			/* 初始化 */
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

                        /* 將NT$ ＋數字塞到szTemplate中來inpad */
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
			inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_,  17, _PADDING_RIGHT_);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);

                        /* 把前面的字串和數字結合起來 */
                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", "總計(Total) :");
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
	}
	else
	{
		
		/* 初始化 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

		/* 將NT$ ＋數字塞到szTemplate中來inpad，退貨要負數 */
		if (pobTran->srBRec.inCode == _FISC_REFUND_)
		{
			sprintf(szTemplate, "%ld",  (0 - pobTran->srBRec.lnTxnAmount));
		}
		else
		{
			sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		}

		inFunc_Amount_Comma(szTemplate, "NT$" , '\x00', _SIGNED_NONE_, 17, _PADDING_RIGHT_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", szTemplate);
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_AMOUNT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);


		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%s", "總計(Total) :");
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);	
		
	}
	
	/* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_LOYALTY_REDEEM_Data
Date&Time       :2017/2/20 下午 3:41
Describe        :列印優惠兌換資料
*/
int inCREDIT_PRINT_LOYALTY_REDEEM_Data(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int			inRetVal;
	int			i = 0;
	char			szTemplate[16 + 1];
	char			szRewardL1L2L3[1650 + 1];	/* L5最常可到1618Bytes 取1650 */
	char			szPrintBuf[300 + 1];
	char			szPrintBuf1[300 + 1];
	char			szDemoMode[2 + 1] = {0};
	unsigned long		ulBufferSize;
	LOYALTY_L1L2L3_OBJECT	srLoyaltyData;
	
	/* 日期時間 */
	inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);

	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 空行 */
	for (i = 0; i < 2; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		/* 兌換核銷資訊(一)之內容。(內容可支援英數字、中文、換行。需補足空白至100 Bytes。) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "兌換精美餐具組完成。");
		inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);	

		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("***重印 REPRINT***", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}

		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "兌換成功 ");
		inDISP_ChineseFont_Color(szPrintBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);
		inDISP_Wait(2000);

		return (VS_SUCCESS);
	}	
	
	/* 優惠兌換內容 */
	ulBufferSize = sizeof(szRewardL1L2L3);
	memset(szRewardL1L2L3, 0x00, sizeof(szRewardL1L2L3));
	if (inNCCC_Loyalty_Read_Reward_Data(&ulBufferSize, szRewardL1L2L3, _REWARD_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	
	/* 放到結構中 */
	if (inNCCC_Loyalty_Data_Format(&srLoyaltyData, (int)ulBufferSize, szRewardL1L2L3) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 是否列印兌換核銷資訊(一) */
	if (memcmp(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelFlag, "1", strlen("1")) == 0)
	{
		/* 兌換核銷資訊(一)長度。(最大長度限制100 Bytes，右靠左補零) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelLen, 3);

		/* 兌換核銷資訊(一)之內容。(內容可支援英數字、中文、換行。需補足空白至100 Bytes。) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelContent, atoi(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelLen));
		inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	/* 是否列印兌換核銷資訊(二) */
	if (memcmp(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelFlag, "1", strlen("1")) == 0)
	{
		/* 兌換核銷資訊(二)長度。(最大長度限制100 Bytes，右靠左補零) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[1].szCancelLen, 3);

		/* 兌換核銷資訊(二)之內容。(內容可支援英數字、中文、換行。需補足空白至100 Bytes。) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[1].szCancelContent, atoi(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[1].szCancelLen));
		inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	/* 是否列印補充資訊 */
	if (memcmp(srLoyaltyData.srL3DATA.szSupInfFlag, "1", strlen("1")) == 0)
	{
		/* 補充資訊實際長度。(右靠左補零) */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, srLoyaltyData.srL3DATA.szSupInfLen, 3);

		/* 補充資訊內容 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memcpy(szPrintBuf, srLoyaltyData.srL3DATA.szSupInfContent, atoi(srLoyaltyData.srL3DATA.szSupInfLen));
		inPRINT_Buffer_PutIn_Format_Auto_Change_Line(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("***重印 REPRINT***", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}

	for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	
	if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
	{
		/* 是否列印兌換核銷資訊(一),  列印兌換核銷資訊(二)*/
		if (!memcmp(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelFlag, "1", 1) || !memcmp(srLoyaltyData.srL3DATA.srL3_CANCEL_DATA[0].szCancelFlag, "1", 1))
			sprintf(szPrintBuf, "兌換成功 ");
		else
			sprintf(szPrintBuf, "兌換失敗 ");
	}
	
	inDISP_ChineseFont_Color(szPrintBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);
	inDISP_Wait(2000);
		
	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_VOID_LOYALTY_REDEEM_Data
Date&Time       :2017/2/21 下午 12:00
Describe        :列印優惠兌換取消資料
*/
int inCREDIT_PRINT_VOID_LOYALTY_REDEEM_Data(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int			inRetVal;
	int			i = 0;
	char			szRewardL1L2L3[1650 + 1];	/* L5最常可到1618Bytes 取1650 */
	char			szPrintBuf[300 + 1];
	char			szPrintBuf1[300 + 1];
	char			szDemoMode[2 + 1] = {0};
	unsigned long		ulBufferSize;
	LOYALTY_L1L2L3_OBJECT	srLoyaltyData;
	
	/* 日期時間 */
	inRetVal = inPRINT_Buffer_PutIn("日期/時間(Date/Time)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%.4s/%.2s/%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6]);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf1, 0x00, sizeof(szPrintBuf1));
	sprintf(szPrintBuf1, "%.2s:%.2s",  &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);

	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	for (i = 0; i < 4; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("***重印 REPRINT***", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}

		for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "兌換取消成功 ");
		inDISP_ChineseFont_Color(szPrintBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);
		inDISP_Wait(2000);

		return (VS_SUCCESS);
	}
	
	/* 取消優惠內容 */
	ulBufferSize = sizeof(szRewardL1L2L3);
	memset(szRewardL1L2L3, 0x00, sizeof(szRewardL1L2L3));
	if (inNCCC_Loyalty_Read_Reward_Data(&ulBufferSize, szRewardL1L2L3, _REWARD_FILE_NAME_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 放到結構中 */
	if (inNCCC_Loyalty_Data_Format(&srLoyaltyData, (int)ulBufferSize, szRewardL1L2L3) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	/* 列印優惠倦內容 */
	if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
	{
		inCREDIT_PRINT_ESVC_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle);
	}
	else
	{
		inCREDIT_PRINT_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle);
	}

	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn("***重印 REPRINT***", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
	}

	for (i = 0; i < 8; i++)
	{
		inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	
	if (!memcmp(&pobTran->srBRec.szRespCode[0], "00", 2))
	{
		sprintf(szPrintBuf, "兌換取消成功 ");
	}
	else
	{
		sprintf(szPrintBuf, "兌換取消失敗 ");
	}
	
	inDISP_ChineseFont_Color(szPrintBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);
	inDISP_Wait(2000);

	return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_HG_Data_ByBuffer
Date&Time       :2017/02/10 上午 11:00
Describe        :
*/
int inCREDIT_PRINT_HG_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int     i;
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));

        /* HG卡號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        strcpy(szTemplate, pobTran->srBRec.szHGPAN);
        /* 卡號遮掩 */
        if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
        {
		/* HAPPG_GO 卡不掩飾 */
		if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
		{

		}
		else
		{
			for (i = 6; i < (strlen(szTemplate) - 4); i ++)
				szTemplate[i] = 0x2A;
		}
        }
        sprintf(szPrintBuf, "ＨＧ卡號　 %s", szTemplate);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 主機別 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "主機　　　 HAPPY GO");
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 交易類別 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        vdCREDIT_PRINT_HG_GetTransactionType(pobTran, szTemplate, szTemplate1);
        sprintf(szPrintBuf, "交易類別　 %s%s", szTemplate, szTemplate1);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 日期 & 時間 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "日期時間　 %.4s/%.2s/%.2s %.2s:%.2s", &pobTran->srBRec.szDate[0], &pobTran->srBRec.szDate[4], &pobTran->srBRec.szDate[6], &pobTran->srBRec.szTime[0], &pobTran->srBRec.szTime[2]);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 調閱編號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "調閱編號　 %s/%06ld", pobTran->srBRec.szHGRefNo, pobTran->srBRec.lnOrgInvNum);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 櫃號 */
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        inGetStoreIDEnable(szTemplate1);
        if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srBRec.szStoreID) > 0))
        {
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			sprintf(szPrintBuf, "品群碼　　　 %s", pobTran->srBRec.szStoreID);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "品群碼　　　 %s", pobTran->srBRec.szStoreID);
			inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
			memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
			sprintf(szPrintBuf, "%s", szTemplate1);
		}
		else
		{
			sprintf(szPrintBuf, "櫃號　　　 %s", pobTran->srBRec.szStoreID);
		}
                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        }
	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_PutIn(pobTran->srBRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
        
        /* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_HG_Multi_Data_ByBuffer
Date&Time       :2017/02/10 上午 11:00
Describe        :
*/
int inCREDIT_PRINT_HG_Multi_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
        int     i;
	
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
        /* 把前面的字串和數字結合起來 */
        inPRINT_Buffer_PutIn("快樂購聯合集點卡", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        /* 交易類別 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(szTemplate1, 0x00, sizeof(szTemplate1));
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        vdCREDIT_PRINT_HG_GetTransactionType(pobTran, szTemplate, szTemplate1);
        sprintf(szPrintBuf, "%s%s", szTemplate, szTemplate1);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	/* HG卡號 */
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        memset(szTemplate, 0x00, sizeof(szTemplate));
        strcpy(szTemplate, pobTran->srBRec.szHGPAN);
        /* 卡號遮掩 */
        if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
        {
		/* HAPPG_GO 卡不掩飾 */
		if (!memcmp(&pobTran->srBRec.szHGPAN[0], "9552", 4))
		{

		}
		else
		{
			for (i = 6; i < (strlen(szTemplate) - 4); i ++)
				szTemplate[i] = 0x2A;
		}
        }
        sprintf(szPrintBuf, "ＨＧ卡號　 %s", szTemplate);
        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        
	if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
	{
		switch (pobTran->srBRec.inHGCode)
		{
			case _HG_POINT_CERTAIN_CREDIT_:
			case _HG_POINT_CERTAIN_CUP_:
			case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				/* 扣抵點數 */                               
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
				/* 商品金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount));
				inFunc_Amount_Comma(szTemplate1, "NT$", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s",szTemplate1);  
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "商品金額　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 點數扣抵金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRedeemAmount);
				inFunc_Amount_Comma(szTemplate1, "NT$", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "點數扣抵金額:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 扣抵點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;

			default:
				break;
		}
	}
	/* 負向交易時 */
	else
	{
		switch (pobTran->srBRec.inHGCode)
		{
			case _HG_POINT_CERTAIN_CREDIT_:
			case _HG_POINT_CERTAIN_CUP_:
			case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				/* 扣抵點數 */                               
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
				/* 商品金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0 - (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount)));
				inFunc_Amount_Comma(szTemplate1, "NT$", '\x00', _SIGNED_MINUS_, 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "商品金額　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 點數扣抵金額 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", (0 - pobTran->srBRec.lnHGRedeemAmount));
				inFunc_Amount_Comma(szTemplate1, "NT$", '\x00', _SIGNED_MINUS_, 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "點數扣抵金額:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 扣抵點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "扣抵點數　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				/* 剩餘點數 */
				/* 將數字塞到szTemplate中來inpad */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
				inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
				sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
				/* 把前面的字串和數字結合起來 */
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
				sprintf(szPrintBuf, "剩餘點數　　:");
				inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				break;

			default:
				break;
		}
	}
        
        /* 斷行 */
	inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_HG_Amount_ByBuffer
Date&Time       :2017/02/10 上午 11:00
Describe        :
*/
int inCREDIT_PRINT_HG_Amount_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        char    szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[42 + 1];
        /* 
        HG_FULL_REDEMPTION 	點數兌換	HG_PAY_CREDIT		信用卡
        HG_INQUIRY		點數查詢	HG_PAY_CASH		現金
        HG_REWARD		紅利積點	HG_PAY_GIFT_PAPER	禮券
        HG_ONLINE_REDEEM	點數扣抵	HG_PAY_CREDIT_INSIDE	HGI
        HG_POINT_CERTAIN	加價購		HG_PAY_CUP		銀聯卡

        HG_REWARD_CREDIT		紅利積點 + 信用卡
        HG_REWARD_CASH			紅利積點 + 現金
        HG_REWARD_GIFT_PAPER		紅利積點 + 禮券
        HG_REWARD_CREDIT_INSIDE		紅利積點 + HGI
        HG_REWARD_CUP			紅利積點 + 銀聯卡

        HG_ONLINE_REDEEM_CREDIT		點數扣抵 + 信用卡
        HG_ONLINE_REDEEM_CASH		點數扣抵 + 現金
        HG_ONLINE_REDEEM_GIFT_PAPER	點數扣抵 + 禮券
        HG_ONLINE_REDEEM_CREDIT_INSIDE	點數扣抵 + HGI
        HG_ONLINE_REDEEM_CUP		點數扣抵 + 銀聯卡

        HG_POINT_CERTAIN_CREDIT		加價購 + 信用卡
        HG_POINT_CERTAIN_CASH		加價購 + 現金
        HG_POINT_CERTAIN_GIFT_PAPER	加價購 + 禮券
        HG_POINT_CERTAIN_CREDIT_INSIDE	加價購 + HGI
        HG_POINT_CERTAIN_CUP		加價購 + 銀聯卡
	*/
    
        if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        {
                switch (pobTran->srBRec.inHGCode)
		{
                        /* 點數兌換 */
                        case _HG_FULL_REDEMPTION_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;
                        /* 加價購 */
                        case _HG_POINT_CERTAIN_CASH_:
                        case _HG_POINT_CERTAIN_GIFT_PAPER_:
                                /* 合計 */                               
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
				inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 扣抵點數 */                               
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "扣抵點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;  
                        /* 點數扣抵 */
                        case _HG_ONLINE_REDEEM_CASH_:
                        case _HG_ONLINE_REDEEM_GIFT_PAPER_:
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGAmount);
				inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 商品金額 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount));
				inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "商品金額　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 點數扣抵金額 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRedeemAmount);
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "點數扣抵金額:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 扣抵點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "扣抵點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;       
                        /* 紅利積點 */
                        case _HG_REWARD_CASH_:
                        case _HG_REWARD_GIFT_PAPER_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;
                        /* 回饋退貨 */
                        case _HG_REWARD_REFUND_:
                                /* 合計 */                              
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                
                                if (pobTran->inTransactionCode != _VOID_)
                                {                                        
                                        /* 將數字塞到szTemplate中來inpad */
                                        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRefundLackPoint);
                                        inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                        sprintf(szTemplate, "%s點",szTemplate1);
					inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                        /* 把前面的字串和數字結合起來 */
                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "不足點數　　:");
                                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                }
                                break;
                        /* 扣抵退貨 */
                        case _HG_REDEEM_REFUND_:
                                 /* 合計 */                              
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;                                
                        default:
                                break;
                }
        }
        else 
        {
                switch (pobTran->srBRec.inHGCode)
		{
                        /* 點數兌換 */
                        case _HG_FULL_REDEMPTION_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                if (pobTran->srBRec.lnHGTransactionPoint == 0L)
                                {
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                }
                                else
                                {
                                        sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
                                }
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;
                        /* 加價購 */
                        case _HG_POINT_CERTAIN_CASH_:
                        case _HG_POINT_CERTAIN_GIFT_PAPER_:                                
                                /* 扣抵點數 */                               
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                if (pobTran->srBRec.lnHGTransactionPoint == 0L)
                                {
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                }
                                else
                                {
                                        sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
                                }
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "扣抵點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;  
                        /* 點數扣抵 */
                        case _HG_ONLINE_REDEEM_CASH_:
                        case _HG_ONLINE_REDEEM_GIFT_PAPER_:
			case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			case _HG_ONLINE_REDEEM_CUP_:
                                /* 商品金額 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
                                {
                                        if (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount == 0L)
                                        {
                                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                        }
                                        else
                                        {
                                                sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnTxnAmount);
                                        }
                                }
                                else
                                {
                                        if (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount == 0L)
                                        {
                                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount);
                                        }
                                        else
                                        {
                                                sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount);
                                        }
                                }
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "商品金額　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 點數扣抵金額 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (pobTran->srBRec.lnHGRedeemAmount == 0)
				{
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRedeemAmount);
				}
				else
				{
					sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGRedeemAmount);
				}
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "點數扣抵金額:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 扣抵點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
				if (pobTran->srBRec.lnHGTransactionPoint == 0)
				{
					sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
				}
				else
				{
					sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnHGTransactionPoint);
				}
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "扣抵點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);                             
                                break;       
                        /* 紅利積點 */
                        case _HG_REWARD_CASH_:
                        case _HG_REWARD_GIFT_PAPER_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                if (pobTran->srBRec.lnTxnAmount == 0L)
                                {
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnTxnAmount);
                                }
                                else
                                {
                                        sprintf(szTemplate1, "-%ld", pobTran->srBRec.lnTxnAmount);
                                }
                                
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;
                        /* 回饋退貨 */
                        case _HG_REWARD_REFUND_:
                                /* 合計 */                              
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", 0 - pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_); 
                                
                                if (pobTran->inTransactionCode != _VOID_)
                                {                                        
                                        /* 將數字塞到szTemplate中來inpad */
                                        memset(szTemplate, 0x00, sizeof(szTemplate));
                                        memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                        sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGRefundLackPoint);
                                        inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                        sprintf(szTemplate, "%s點",szTemplate1);
					inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                        /* 把前面的字串和數字結合起來 */
                                        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                        sprintf(szPrintBuf, "不足點數　　:");
                                        inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                }
                                break;
                        /* 扣抵退貨 */
                        case _HG_REDEEM_REFUND_:
                                /* 合計 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGTransactionPoint);
                                inFunc_Amount_Comma(szTemplate1, "NT$ ", '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "合計　　　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 斷行 */
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                /* 剩餘點數 */
                                /* 將數字塞到szTemplate中來inpad */
                                memset(szTemplate, 0x00, sizeof(szTemplate));
                                memset(szTemplate1, 0x00, sizeof(szTemplate1));
                                sprintf(szTemplate1, "%ld", pobTran->srBRec.lnHGBalancePoint);
                                inFunc_PAD_ASCII(szTemplate1, szTemplate1, '\x00', 16, _PADDING_RIGHT_);
                                sprintf(szTemplate, "%s點",szTemplate1);
				inPRINT_Buffer_PutIn(szTemplate, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_RIGHT_);
                                /* 把前面的字串和數字結合起來 */
                                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                                sprintf(szPrintBuf, "剩餘點數　　:");
                                inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                break;                                
                        default:
                                break;
                }
        }
        
        /* 斷行 */
        inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        
        return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_HG_ReceiptEND_ByBuffer
Date&Time       :2017/02/10 上午 11:00
Describe        :
*/
int inCREDIT_PRINT_HG_ReceiptEND_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        int	i;
	
        if (inPrinttype_ByBuffer)
        {
                /* 直式 */
                inPRINT_Buffer_PutIn("簽名欄:_____________________", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ ||
		    pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
                        inPRINT_Buffer_PutIn("*** 商店收據 Merchant Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_CUST_;

                }
                else if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
                {
                        inPRINT_Buffer_PutIn("*** 持卡人收據 Customer Copy ***", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        pobTran->srBRec.inPrintOption = _PRT_MERCH_;
                }

                inPRINT_Buffer_PutIn("I AGREE TO PAY TOTAL AMOUNT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                inPRINT_Buffer_PutIn("ACCORDING TO CARD ISSUER AGREEMENT", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }
        else
        {
                /* 橫式 */
                if (pobTran->srBRec.inPrintOption == _PRT_MERCH_ || pobTran->srBRec.inPrintOption == _PRT_MERCH_DUPLICATE_)
                {
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　　　   商店存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
                else
                {
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			inPRINT_Buffer_PutIn("　　　　　　　　　 持卡人存根", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                }
		
		if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
		{
			inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		}
		
		/* Print Notice */
		if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
		
                for (i = 0; i < 8; i++)
		{
			inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		
        }

        return (VS_SUCCESS);
}

/*
Function        :vdCREDIT_PRINT_HG_GetTransactionType
Date&Time       :2017/02/10 上午 11:00
Describe        :
*/
void vdCREDIT_PRINT_HG_GetTransactionType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1, char *szPrintBuf2)
{
        /* 
        HG_FULL_REDEMPTION 	點數兌換	HG_PAY_CREDIT		信用卡
        HG_INQUIRY		點數查詢	HG_PAY_CASH		現金
        HG_REWARD		紅利積點	HG_PAY_GIFT_PAPER	禮券
        HG_ONLINE_REDEEM	點數扣抵	HG_PAY_CREDIT_INSIDE	HGI
        HG_POINT_CERTAIN	加價購		HG_PAY_CUP		銀聯卡

        HG_REWARD_CREDIT		紅利積點 + 信用卡
        HG_REWARD_CASH			紅利積點 + 現金
        HG_REWARD_GIFT_PAPER		紅利積點 + 禮券
        HG_REWARD_CREDIT_INSIDE		紅利積點 + HGI
        HG_REWARD_CUP			紅利積點 + 銀聯卡

        HG_ONLINE_REDEEM_CREDIT		點數扣抵 + 信用卡
        HG_ONLINE_REDEEM_CASH		點數扣抵 + 現金
        HG_ONLINE_REDEEM_GIFT_PAPER	點數扣抵 + 禮券
        HG_ONLINE_REDEEM_CREDIT_INSIDE	點數扣抵 + HGI
        HG_ONLINE_REDEEM_CUP		點數扣抵 + 銀聯卡

        HG_POINT_CERTAIN_CREDIT		加價購 + 信用卡
        HG_POINT_CERTAIN_CASH		加價購 + 現金
        HG_POINT_CERTAIN_GIFT_PAPER	加價購 + 禮券
        HG_POINT_CERTAIN_CREDIT_INSIDE	加價購 + HGI
        HG_POINT_CERTAIN_CUP		加價購 + 銀聯卡
	*/
    
        if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
        {
                switch (pobTran->srBRec.inHGCode)
		{
                        case _HG_FULL_REDEMPTION_:
                                strcpy(szPrintBuf1, "01 點數兌換");
                                strcpy(szPrintBuf2, "");
                                break;                            
                        case _HG_REWARD_CREDIT_:
                        case _HG_REWARD_CREDIT_INSIDE_:
			case _HG_REWARD_REDEMPTION_CREDIT_:
			case _HG_REWARD_REDEMPTION_CREDIT_INSIDE_:
			case _HG_REWARD_INSTALLMENT_CREDIT_:
			case _HG_REWARD_INSTALLMENT_CREDIT_INSIDE_:
                        case _HG_REWARD_CUP_:
                                strcpy(szPrintBuf1, "00 紅利積點");

                                if (memcmp(&pobTran->srBRec.szHGRespCode[0], "00", 2))
                                {
                                        if (strlen(pobTran->srBRec.szHGRespCode) == 0)
                                                strcat(szPrintBuf1, "  拒絕交易 DF");
                                        else
                                        {
                                                strcat(szPrintBuf1, "  拒絕交易 ");
                                                strcat(szPrintBuf1, pobTran->srBRec.szHGRespCode);
                                        }
                                }

                                strcpy(szPrintBuf2, "");
                                break;                           
                        case _HG_REWARD_CASH_:
                                strcpy(szPrintBuf1, "00 紅利積點-現金");
                                strcpy(szPrintBuf2, "");
                                break;                            
                        case _HG_REWARD_GIFT_PAPER_:
                                strcpy(szPrintBuf1, "00 紅利積點-禮券");
                                strcpy(szPrintBuf2, "");
                                break;                                
                        case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CUP_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
				strcpy(szPrintBuf1, "03 點數扣抵");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_ONLINE_REDEEM_CASH_:
				strcpy(szPrintBuf1, "03 點數扣抵-現金");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_ONLINE_REDEEM_GIFT_PAPER_:
				strcpy(szPrintBuf1, "03 點數扣抵-禮券");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_POINT_CERTAIN_CREDIT_:
			case _HG_POINT_CERTAIN_CUP_:
			case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				strcpy(szPrintBuf1, "02 加價購");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_POINT_CERTAIN_CASH_:
				strcpy(szPrintBuf1, "02 加價購-現金");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_POINT_CERTAIN_GIFT_PAPER_:
				strcpy(szPrintBuf1, "02 加價購-禮券");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_REDEEM_REFUND_:
				strcpy(szPrintBuf1, "06 扣抵退貨");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_REWARD_REFUND_:
				strcpy(szPrintBuf1, "05 回饋退貨");
				strcpy(szPrintBuf2, "");
				break;
			default :
				strcpy(szPrintBuf1, "!!!HG_正向交易!!!");
				strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");
				break;
                }
        }
        else 
        {
                switch (pobTran->srBRec.inHGCode)
		{
                        case _HG_FULL_REDEMPTION_:
                                strcpy(szPrintBuf1, "07 取消點數兌換");
                                strcpy(szPrintBuf2, "");
                                break;                            
                        case _HG_REWARD_CREDIT_:
                        case _HG_REWARD_CREDIT_INSIDE_:
			case _HG_REWARD_REDEMPTION_CREDIT_:
			case _HG_REWARD_REDEMPTION_CREDIT_INSIDE_:
			case _HG_REWARD_INSTALLMENT_CREDIT_:
			case _HG_REWARD_INSTALLMENT_CREDIT_INSIDE_:
                        case _HG_REWARD_CUP_:
                                strcpy(szPrintBuf1, "07 取消紅利積點");
                                strcpy(szPrintBuf2, "");
                                break;                           
                        case _HG_REWARD_CASH_:
                                strcpy(szPrintBuf1, "07 取消紅利積點-現金");
                                strcpy(szPrintBuf2, "");
                                break;                            
                        case _HG_REWARD_GIFT_PAPER_:
                                strcpy(szPrintBuf1, "07 取消紅利積點-禮券");
                                strcpy(szPrintBuf2, "");
                                break;                                
                        case _HG_ONLINE_REDEEM_CREDIT_:
			case _HG_ONLINE_REDEEM_CUP_:
			case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
				strcpy(szPrintBuf1, "07 取消點數扣抵");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_ONLINE_REDEEM_CASH_:
				strcpy(szPrintBuf1, "07 取消點數扣抵-現金");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_ONLINE_REDEEM_GIFT_PAPER_:
				strcpy(szPrintBuf1, "07 取消點數扣抵-禮券");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_POINT_CERTAIN_CREDIT_:
			case _HG_POINT_CERTAIN_CUP_:
			case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
				strcpy(szPrintBuf1, "07 取消加價購");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_POINT_CERTAIN_CASH_:
				strcpy(szPrintBuf1, "07 取消加價購-現金");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_POINT_CERTAIN_GIFT_PAPER_:
				strcpy(szPrintBuf1, "07 取消加價購-禮券");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_REDEEM_REFUND_:
				strcpy(szPrintBuf1, "07 取消扣抵退貨");
				strcpy(szPrintBuf2, "");
				break;
			case _HG_REWARD_REFUND_:
				strcpy(szPrintBuf1, "07 取消回饋退貨");
				strcpy(szPrintBuf2, "");
				break;
			default :
				strcpy(szPrintBuf1, "!!!HG_正向交易!!!");
				strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");
				break;
                }
        }
}

/*
Function        :inCREDIT_PRINT_AutoSettle_Failed_ByBuffer
Date&Time       :2016/10/13 下午 6:27
Describe        :列印結帳失敗的資料
*/
int inCREDIT_PRINT_AutoSettle_Failed_ByBuffer()
{
	int			i;
	int			inRetVal;
	char			szHostName[8 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
	
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		inRetVal = inPRINT_Buffer_PutIn("******************************************", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		memset(szHostName, 0x00, sizeof(szHostName));
		inRetVal = inGetHostLabel(szHostName);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 去除空白，使列印置中 */
		inFunc_DiscardSpace(szHostName);

		inRetVal = inPRINT_Buffer_PutIn(szHostName, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		inRetVal = inPRINT_Buffer_PutIn("結帳失敗，請重試。", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		inRetVal = inPRINT_Buffer_PutIn("******************************************", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		for (i = 0; i < 8; i++)
		{
			inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
			if (inRetVal != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
		}

		if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer, &srBhandle)) != VS_SUCCESS)
			return (inRetVal);

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_TerminalTraceLog_ByBuffer
Date&Time       :2017/5/19 下午 4:20
Describe        :列印端末機的 TRACE LOG
*/
int inCREDIT_PRINT_TerminalTraceLog_ByBuffer(void)
{
	int		i, j = 0, inCnt = 1;
	int		inRetVal;
	int		inReadSize;
	char		*szReadData = NULL;
	char		szPrintBuf[256 + 1], szTemplate[42 + 1];
	unsigned long	ulHandle = 0;
	unsigned char	uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle	srBhandle;
	FONT_ATTRIB	srFont_Attrib;
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		inRetVal = inFILE_OpenReadOnly(&ulHandle, (unsigned char*)_TMS_TRACE_LOG_);
		if (inRetVal != VS_SUCCESS)
		{
			inPRINT_Buffer_PutIn("!!NO_TERM._TRACE_LOG!!", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inReadSize = (int)lnFILE_GetSize(&ulHandle, (unsigned char*)_TMS_TRACE_LOG_);
			szReadData = malloc(inReadSize + 1);
			inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);
			inRetVal = inFILE_Read(&ulHandle, (unsigned char*)szReadData, (unsigned long)inReadSize);
			if (inRetVal == VS_SUCCESS)
			{
				memset(szPrintBuf, 0x00, sizeof(szPrintBuf));

				for (i = 0; i < inReadSize; i ++)
				{
					szPrintBuf[j ++] = szReadData[i];
					if (szReadData[i] == 0x0A && szReadData[i - 1] == 0x0D)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%03d. =====================================", (inCnt ++));
						inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						szPrintBuf[j] = 0x00;
						szPrintBuf[j - 1] = 0x00;
						szPrintBuf[j - 2] = 0x00;
						inPRINT_Buffer_PutIn(szTemplate, _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
						memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
						j = 0;
					}
				}
			}
		}

		inFILE_Close(&ulHandle);
		free(szReadData);

		inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);

		return (VS_SUCCESS);
	}
}

/*
Function        :inCREDIT_PRINT_Data_ByBuffer_ESVC
Date&Time       :2018/1/8 下午 5:37
Describe        :列印DATA
*/
int inCREDIT_PRINT_Data_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inLen = 0;
	int     inRetVal = VS_SUCCESS;
	char 	szPrintBuf[84 + 1], szTemplate1[42 + 1];
	char	szTemplate[40 + 1];
	char	szProductCodeEnable[1 + 1];
	char	szTicketNeedNewBatch[2 + 1] = {0};
        char    szCustomerIndicator[3 + 1] = {0};

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 橫式 */
	/* 卡號、卡別 */
	inRetVal = inPRINT_Buffer_PutIn("卡號", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);

	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position("卡別", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
	        strcpy(szPrintBuf, "一卡通");
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
	        strcpy(szPrintBuf, "悠遊卡");
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
	        strcpy(szPrintBuf, "愛金卡");
	}
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 卡號值 & 批號 */
	/* 一卡通：遮掩後2 碼，遮掩字元為”*” */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inLen = strlen(pobTran->srTRec.szUID);
		memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
		szPrintBuf[inLen - 1] = 0x2A;
                szPrintBuf[inLen - 2] = 0x2A;
		strcat(szPrintBuf, "(W)");
	}
	/*	1.簽單卡號列印邏輯
		<T4800>票卡版號(Purse Version Number)=0，<T0200>列印晶片卡號(Card Id)
		<T0200>20 bytes，14th,15th,16th隱碼，17th,18th,19th,20th顯示
		<T4800>票卡版號(Purse Version Number)<>0，<T0211>列印外觀卡號(Purse Id)
		<T0211>16 bytes，6th,7th,8th,9th,10th,11th隱碼，12th,13th,14th,15th,16th顯示
		，遮掩字元為”*”
		以<T0200>悠遊卡卡號遮掩從後往前算第五碼，第六碼，第七碼遮掩
		範例:以8碼卡號為例，卡號11651733，1,6,5要遮掩，實際列印為1***1733
		1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
		0 0 0 0 0 0 0 0 0 1 1 6 5 1 7 3 3
		0 0 0 0 0 0 0 0 0 1 * * * 1 7 3 3
		範例:以10碼卡號為例，卡號6611651733，1,6,5要遮掩，實際列印為661***1733
		1th 2th 3th 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th
		0 0 0 0 0 0 0 6 6 1 1 6 5 1 7 3 3
		0 0 0 0 0 0 0 6 6 1 * * * 1 7 3 3

		2. 電子發票載具卡號為T0200 (不遮掩)
	*/
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                inLen = strlen(pobTran->srTRec.szUID);
                memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
            
                /* 客製化098，悠遊卡因為使用T0200，加密遮掩為倒數5、6、7 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        szPrintBuf[inLen - 5] = 0x2A;
                        szPrintBuf[inLen - 6] = 0x2A;
                        szPrintBuf[inLen - 7] = 0x2A;
                }
                else
                {
                        if (atoi(pobTran->srTRec.srECCRec.szPurseVersionNumber) == 0)
                        {
                                szPrintBuf[inLen - 5] = 0x2A;
                                szPrintBuf[inLen - 6] = 0x2A;
                                szPrintBuf[inLen - 7] = 0x2A;
                        }
                        else
                        {
                                /* 全部16 第6-11隱碼 */
                                szPrintBuf[inLen - 6] = 0x2A;
                                szPrintBuf[inLen - 7] = 0x2A;
                                szPrintBuf[inLen - 8] = 0x2A;
                                szPrintBuf[inLen - 9] = 0x2A;
                                szPrintBuf[inLen - 10] = 0x2A;
                                szPrintBuf[inLen - 11] = 0x2A;
                        }
                }    
		strcat(szPrintBuf, "(W)");
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		inLen = strlen(pobTran->srTRec.szUID);
		memcpy(szPrintBuf, pobTran->srTRec.szUID, inLen);
		/* 愛金卡：遮掩9-12 碼，遮掩字元為"*" */
		szPrintBuf[8] = 0x2A;
                szPrintBuf[9] = 0x2A;
		szPrintBuf[10] = 0x2A;
		szPrintBuf[11] = 0x2A;
		strcat(szPrintBuf, "(W)");		
	}
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "批號　　 %s", "");
	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	inGetBatchNum(szPrintBuf);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
		
	/* 交易別 & 特店代號 */	
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ || 
	    pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ ||
	    pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)
	        strcpy(szTemplate, "購貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ ||
		 pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ ||
		 pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)
	        strcpy(szTemplate, "退貨　　");
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ || 
		 pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
		 pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)
	        strcpy(szTemplate, "現金加值");  
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ || 
		 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
		 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
	        strcpy(szTemplate, "加值取消");              
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ || 
		 pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
		 pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
	        strcpy(szTemplate, "餘額查詢");  
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_ || 
		 pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_ ||
		 pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
		strcpy(szTemplate, "自動加值");         
	else
		strcpy(szTemplate, "　　");
	
	sprintf(szPrintBuf, "交易　　 %s", szTemplate);
	
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_ || pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szPrintBuf, "特店代號 %s", szTemplate1);

		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetIPASS_System_ID(szTemplate);
			memcpy(&szPrintBuf[0], szTemplate, 2);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetIPASS_SP_ID(szTemplate);
			memcpy(&szPrintBuf[2], szTemplate, 2);

			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetIPASS_Sub_Company_ID(szTemplate);
			memcpy(&szPrintBuf[4], szTemplate, 4);
		}        
		else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetECC_New_SP_ID(szTemplate);
			memcpy(&szPrintBuf[0], szTemplate, 8);
		}

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szPrintBuf, "門市代碼 %s", szTemplate1);

		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetICASH_Shop_ID(szTemplate);
		memcpy(&szPrintBuf[0], szTemplate, 8);

		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);		
	}
	
	/* 主機、調閱編號 */
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	inGetHostLabel(szTemplate);
	
	sprintf(szPrintBuf, "主機　　 %s", szTemplate);
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_ &&
	    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
	    pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "調閱號　 %s", "");
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, "%06ld", pobTran->srTRec.lnInvNum);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		/* 現在查詢餘額不印簽單 */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		sprintf(szPrintBuf, " ");
		inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 日期時間 */
	if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
        	memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%.4s/%.2s/%.2s  %.2s:%.2s:%.2s", 
			&pobTran->srTRec.srECCRec.szDate[0], 
			&pobTran->srTRec.srECCRec.szDate[4], 
			&pobTran->srTRec.srECCRec.szDate[6], 
			&pobTran->srTRec.srECCRec.szTime[0], 
			&pobTran->srTRec.srECCRec.szTime[2], 
			&pobTran->srTRec.srECCRec.szTime[4]);
        }
        else 
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
        	sprintf(szTemplate, "20%.2s/%.2s/%.2s  %.2s:%.2s:%.2s", 
			&pobTran->srTRec.szDate[0], 
			&pobTran->srTRec.szDate[2], 
			&pobTran->srTRec.szDate[4], 
			&pobTran->srTRec.szTime[0], 
			&pobTran->srTRec.szTime[2], 
			&pobTran->srTRec.szTime[4]);
        }
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "日期/時間 %s", szTemplate); 
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	/* 序號 & RF序號 */
        if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_ &&
	    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
	    pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	memset(szTemplate, 0x00, sizeof(szTemplate));
		/* 雖然電文RRN送12個byte，但RRN最後一碼是0x00，所以只看到11碼 */
		inFunc_PAD_ASCII(szTemplate, pobTran->srTRec.szRefNo, ' ', 12, _PADDING_RIGHT_);
    
		sprintf(szPrintBuf, "序號　　 %s", szTemplate);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
        	
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	sprintf(szPrintBuf, "RF序號　 %s", "");
        	inRetVal = inPRINT_Buffer_PutIn_Specific_X_Position(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_DEFINE_X_01_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	memcpy(szPrintBuf, pobTran->srTRec.szTicketRefundCode, 6);
		inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_SMALL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
        
        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
        {
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_Device2(szTemplate);
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	sprintf(szPrintBuf, "二代設備編號　 %s", szTemplate);
        	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        	
		memset(szTicketNeedNewBatch, 0x00, sizeof(szTicketNeedNewBatch));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTicket_NeedNewBatch(szTicketNeedNewBatch);
		if (memcmp(szTicketNeedNewBatch, "Y", strlen("Y")) == 0)
		{
			sprintf(szTemplate, "%.4s%.2s%.2s", &pobTran->srBRec.szDate[2], &pobTran->srBRec.szDate[6], _ECC_DEFAULT_BATCH_);
		}
		else
		{
			inGetTicket_Batch(szTemplate);
		}
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	sprintf(szPrintBuf, "悠遊卡批次號碼 %s", szTemplate);
        	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        	
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, pobTran->srTRec.srECCRec.szRRN);
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	sprintf(szPrintBuf, "RRN    　　　  %s", szTemplate);
        	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* 櫃號 */
	memset(szTemplate1, 0x00, sizeof(szTemplate1));
	inGetStoreIDEnable(szTemplate1);
	if ((memcmp(&szTemplate1[0], "Y", 1) == 0) && (strlen(pobTran->srTRec.szStoreID) > 0))
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inRetVal = inPRINT_Buffer_PutIn("品群碼(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn("櫃號(Store ID)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 原品群碼 */
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srTRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
			
			char	szPOS_ID[6 + 1] = {0};
			memset(szPOS_ID, 0x00, sizeof(szPOS_ID));
			inGetPOS_ID(szPOS_ID);

			memset(szTemplate1, 0x00, sizeof(szTemplate1));
			memcpy(szTemplate1, _HARDCODE_STORE_ID_CUS_021_TAKAWEL_, 12);
			memcpy(&szTemplate1[12], szPOS_ID, 6);
			/* hardcode */
			inRetVal = inPRINT_Buffer_PutIn(szTemplate1, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		else
		{
			inRetVal = inPRINT_Buffer_PutIn(pobTran->srBRec.szStoreID, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		}
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 產品代碼 */
	inGetProductCodeEnable(szProductCodeEnable);
	if (memcmp(szProductCodeEnable, "Y", 1) == 0)
	{
		inRetVal = inPRINT_Buffer_PutIn("產品代碼(Product Code)", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

		inRetVal = inPRINT_Buffer_PutIn(pobTran->srTRec.szProductCode, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* 斷行 */
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_NORMAL2_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (inRetVal);
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_IPASS
Date&Time       :2018/1/16 下午 3:01
Describe        :
*/
int inCREDIT_PRINT_Amount_ByBuffer_IPASS(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[42 + 1];
        
        if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_)
        {
                /* 自動加值金額 */
		inRetVal = inPRINT_Buffer_PutIn("自動加值 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
	        memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
                inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                /* 交易前餘額 */
        	inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        	if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	if (pobTran->srTRec.lnCardRemainAmount < 0)
		{
	                sprintf(szPrintBuf, "%ld", 0 - pobTran->srTRec.lnCardRemainAmount);
		}
	        else
		{
	                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnCardRemainAmount);
		}
	        
	        if (pobTran->srTRec.lnCardRemainAmount < 0)
		{
                        inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_MINUS_, 10, _PADDING_RIGHT_);
		}
                else
		{
                        inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
		}
                                
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
                if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);   
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                /* 交易後餘額 */
        	inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	if (pobTran->srTRec.lnCardRemainAmount < 0)
		{
	                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount - pobTran->srTRec.lnCardRemainAmount);
		}
	        else
		{
	                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount + pobTran->srTRec.lnCardRemainAmount);
		}
	                
	        inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
		
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);      
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
        else
        {
        	if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_)
        	{       
        	        if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
        	        {
                	        /* 自動加值金額 */
				inRetVal = inPRINT_Buffer_PutIn("自動加值 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				memset(szPrintBuf, 0x00, sizeof(szPrintBuf)); 
				sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
				inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
				inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);

				inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				if (inRetVal != VS_SUCCESS)
					return (VS_ERROR);
                        }
                }
                     
                /* 交易前餘額 */
        	/* 交易前餘額 */
        	inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
        	if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	if (pobTran->srTRec.lnTotalTopUpAmount > 0L)
        	{       
        	        if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
			{
        	                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount + (pobTran->srTRec.lnFinalBeforeAmt - 100000));
			}
        	        else
			{
        	                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt - pobTran->srTRec.lnTotalTopUpAmount);
			}
        	}
        	else      
        	{      
        	        if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
			{
        	                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt - 100000);
			}
        	        else
			{
        	                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
			}
        	}
        	
                if (pobTran->srTRec.lnFinalBeforeAmt > 100000)
		{
			inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_MINUS_, 10, _PADDING_RIGHT_);
		}
                else
		{
			inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
		}
                             
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);

                /* 交易金額 */
        	inRetVal = inPRINT_Buffer_PutIn("交易金額　　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTxnAmount);
                inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
                
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);  
                if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		/* 交易後餘額 */
        	inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
        	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        	
                if (pobTran->srTRec.lnFinalAfterAmt > 100000)
                {     
                        sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt - 100000);   
                        inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_MINUS_, 10, _PADDING_RIGHT_);
                }
                else
                {      
                        sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);     
                        inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
                }
                
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
               
        return (VS_SUCCESS);      
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_ECC
Date&Time       :2018/1/16 下午 3:05
Describe        :
*/
int inCREDIT_PRINT_Amount_ByBuffer_ECC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[42 + 1];

        if (pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ && pobTran->srTRec.lnTotalTopUpAmount > 0L)
        {
                /* 自動加值金額 */
	        inRetVal = inPRINT_Buffer_PutIn("自動加值 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
                inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
                
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
        
	/* 交易前餘額 */
	inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
	inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
        
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_); 
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 交易金額 */
	inRetVal = inPRINT_Buffer_PutIn("交易金額　　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTxnAmount);
	inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
        
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 交易後餘額 */
	inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);
	inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);        
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);      
}

/*
Function        :inCREDIT_PRINT_Amount_ByBuffer_ICASH
Date&Time       :2019/10/2 下午 4:56
Describe        :
*/
int inCREDIT_PRINT_Amount_ByBuffer_ICASH(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	inRetVal = VS_SUCCESS;
        char    szPrintBuf[42 + 1];

        if (pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_ && pobTran->srTRec.lnTotalTopUpAmount > 0L)
        {
                /* 自動加值金額 */
	        inRetVal = inPRINT_Buffer_PutIn("自動加值 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
                sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTotalTopUpAmount);
                inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
                
                inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
        
	/* 交易前餘額 */
	inRetVal = inPRINT_Buffer_PutIn("交易前餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalBeforeAmt);
	inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
        
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_); 
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 交易金額 */
	inRetVal = inPRINT_Buffer_PutIn("交易金額　　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnTxnAmount);
	inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);
        
	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

	/* 交易後餘額 */
	inRetVal = inPRINT_Buffer_PutIn("交易後餘額　 :  ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _CURRENT_LINE_, _PRINT_LEFT_);
	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	sprintf(szPrintBuf, "%ld", pobTran->srTRec.lnFinalAfterAmt);
	inFunc_Amount_Comma(szPrintBuf, "NT$ ", '\x00', _SIGNED_NONE_, 10, _PADDING_RIGHT_);

	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_RIGHT_);        
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);
	
	inRetVal = inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	if (inRetVal != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);      
}

/*
Function        :inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC
Date&Time       :2018/1/16 下午 3:07
Describe        :
*/
int inCREDIT_PRINT_ReceiptEND_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	char	szDemoMode[2 + 1] = {0};
	
        if (pobTran->srTRec.inTicketType == _TICKET_TYPE_IPASS_)
        {
                inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("備註：若有疑問請洽一卡通票證公司", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("客服專線：(07)791-2000", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
        }
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ECC_)
	{
		inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("備註：若有疑問請洽", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("悠遊卡公司專線：412-8880", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("(手機及金馬地區請加02)", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else if (pobTran->srTRec.inTicketType == _TICKET_TYPE_ICASH_)
	{
		inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("備註：若有疑問請洽愛金卡公司", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
                inRetVal = inPRINT_Buffer_PutIn("客服專線：0800-233-888", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	
	/* 教育訓練模式圖案列印 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		if (inPRINT_Buffer_PutGraphic((unsigned char*)_NCCC_DEMO_, uszBuffer, srBhandle, 50, _APPEND_) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inPRINT_PutGraphic(_NCCC_DEMO_) failed");
			}
		}
	}
	
        if (pobTran->srTRec.inPrintOption == _PRT_MERCH_)
        {
                inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("商店存根聯", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}
	else if (pobTran->srTRec.inPrintOption == _PRT_CUST_)
	{
	        inRetVal = inPRINT_Buffer_PutIn("------------------------------------------", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		
		inRetVal = inPRINT_Buffer_PutIn("顧客收執聯", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		inRetVal = inPRINT_Buffer_PutIn("重印 REPRINT", _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* Print Notice */
	if (inCREDIT_PRINT_Notice(pobTran, uszBuffer, srBhandle) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* 票正要印下面slogan by Russell 2018/12/6 上午 10:25 */
	/* Print Slogan */
	if (pobTran->srTRec.inPrintOption == _PRT_CUST_)
	{
		if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_DOWN_, uszBuffer, srBhandle) != VS_SUCCESS)
			return (VS_ERROR);
	}
        
	/* 【需求單-108215】電票交易支援優惠平台需求 by Russell 2020/5/5 下午 5:25 */
	/* 因為票證只會印最後一筆，有值就直接印了 */
	if (pobTran->srTRec.inPrintOption == _PRT_CUST_ && 
	   (pobTran->srTRec.uszRewardL1Bit == VS_TRUE	||
	    pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
	    pobTran->srTRec.uszRewardL5Bit == VS_TRUE))
	{
		inCREDIT_PRINT_ESVC_RewardAdvertisement(pobTran, uszBuffer, srFont_Attrib, srBhandle);
	}
	
        /* 列印空白行 */
	for (i = 0; i < 8; i++)
	{
		inRetVal = inPRINT_Buffer_PutIn(" ", _PRT_HEIGHT_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
	}

        return (VS_SUCCESS);      
}

/*
Function        :inCREDIT_PRINT_TotalReport_ByBuffer_ESVC
Date&Time       :2018/1/29 上午 11:20
Describe        :
*/
int inCREDIT_PRINT_TotalReport_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int			inPrintIndex = 0, inRetVal = 0;
	int			inYLen = 0;
	int			inFileLen = 0;
	char			szDebugMsg[100 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer1[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	unsigned char		uszFileName[30 + 1] = {0};
	BufferHandle		srBhandle1;
	FONT_ATTRIB		srFont_Attrib1;
	TICKET_ACCUM_TOTAL_REC	srAccumRec;
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		return (VS_SUCCESS);
	}
	/* (需求單 - 107227)邦柏科技自助作業客製化 不列印結帳條 2018/11/28 下午 3:03 by Russell */
	/* (需求單 - 107276)自助交易標準400做法 不列印結帳條 by Russell 2018/12/27 上午 11:20 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)   ||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	{
		return (VS_SUCCESS);
	}
	else
	{

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "----------------------------------------");
			inLogPrintf(AT, "inCREDIT_PRINT_TotalReport_ByBuffer_ESVC() START !");
		}

		if (pobTran->inTransactionCode == _SETTLE_)
		{
                        /* 客製化098，總額簽單不同 */
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                        {
                                if (pobTran->uszPrePrintBit == VS_TRUE)
                                        inPrintIndex = _TOTAL_REPORT_INDEX_098_ESVC_;
                                else
                                        inPrintIndex = _TOTAL_REPORT_INDEX_098_ESVC_SETTLE_;
                        }
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 先不使用粗體 */
				if (pobTran->uszPrePrintBit == VS_TRUE)
                                        inPrintIndex = _TOTAL_REPORT_INDEX_ESVC_;
                                else
					inPrintIndex = _TOTAL_REPORT_INDEX_ESVC_SETTLE_;
			}
                        else
                        {    
                                if (pobTran->uszPrePrintBit == VS_TRUE)
                                        inPrintIndex = _TOTAL_REPORT_INDEX_ESVC_;
                                else
                                        inPrintIndex = _TOTAL_REPORT_INDEX_ESVC_SETTLE_;
                        }
		}
		else
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_046_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
				 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 先不使用粗體 */
				inPrintIndex = _TOTAL_REPORT_INDEX_ESVC_;
			}
			else
			{
				inPrintIndex = _TOTAL_REPORT_INDEX_ESVC_;
			}
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "PrintIndex : %d", inPrintIndex);
			inLogPrintf(AT, szDebugMsg);
		}

		/* 開啟交易總的檔案 */
		memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		inRetVal = inACCUM_GetRecord_ESVC(pobTran, &srAccumRec);
		if (inRetVal == VS_NO_RECORD)
		{
			memset(&srAccumRec, 0x00, sizeof(srAccumRec));
		}
		else if (inRetVal == VS_ERROR)
		{
			return (VS_ERROR);
		}

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

		inPRINT_Buffer_Initial(uszBuffer1, _BUFFER_MAX_LINE_, &srFont_Attrib1, &srBhandle1);
		/* 列印LOGO */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportLogo != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportLogo(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
		/* 列印TID MID */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportTop != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportTop(pobTran, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
		/* 全部金額總計 */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmount != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmount(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 卡別金額總計 */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmountByCard != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inAmountByCard(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);

		/* 結束 */
		if (srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportEnd != NULL)
			if ((inRetVal = srTotalReport_ByBuffer_ESVC[inPrintIndex].inReportEnd(pobTran, &srAccumRec, uszBuffer1, &srFont_Attrib1, &srBhandle1)) != VS_SUCCESS)
				return (inRetVal);
		
		if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, &srBhandle1)) != VS_SUCCESS)
			return (inRetVal);
		
		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
		* 儲存結帳報表的buffer */
		if (pobTran->inRunOperationID == _OPERATION_SETTLE_)
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
			{
				pobTran->uszEverRich_Settle_RepeintBit = VS_TRUE;
				inCREDIT_PRINT_Dutyfree_Reprint_Report_ESVC_First(pobTran, uszBuffer1, &inYLen);
				inFileLen = PB_CANVAS_X_SIZE * (inYLen + 8);
				memset(uszFileName, 0x00, sizeof(uszFileName));
				inFunc_ComposeFileName(pobTran, (char*)uszFileName, _DUTY_FREE_REPRINT_TOTAL_REPORT_FILE_EXTENSION_, 0);
				inNCCC_Func_Duty_Free_Save_Total_Report_Buffer(uszFileName, uszBuffer1, inFileLen);
				pobTran->uszEverRich_Settle_RepeintBit = VS_FALSE;
			}
		}

		return (VS_SUCCESS);
	}
}
