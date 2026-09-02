/* 
 * File:   NCCCescReceipt.h
 * Author: user
 *
 * Created on 2017年4月21日, 上午 11:20
 */

#define	_DATA_FLAG_			"||"
#define _ESC_FILE_RECEIPT_UTF8_		"ESC_E1_utf8.txt"
#define _ESC_FILE_RECEIPT_		"ESC_E1.txt"

/* ESC用圖片名（因ESC上傳不需要路徑） */
#define _ESC_BMP_NCCC_LOGO_             "NcccLogo.bmp"		/* 銀行LOGO */
#define _ESC_BMP_MERCHANT_LOGO_         "BmpLogo.bmp"		/* 商店LOGO */
#define _ESC_BMP_NOTICE_LOGO_		"BmpNotice.bmp"		/* 商店提示 */
#define _ESC_BMP_SLOGAN_LOGO_		"BmpSlogan.bmp"		/* 企業標語 */
#define _ESC_BMP_LEGAL_LOGO_		"BmpLegal.bmp"		/* 分期交易警語 */
#define _ESC_BMP_NAME_LOGO_		"BmpName.bmp"		/* 表頭文字檔 */
#define _ESC_BMP_CUP_LEGAL_LOGO_	"CUPLegal.bmp"		/* CUP交易警語 */
#define _ESC_BMP_SIGNATURE_		"BmpEsc.bmp"		/* 電子簽名圖檔 */

#define _ESC_DCC_V_			_NCCC_DCC_FILE_VISA_DISCLAIMER_
#define _ESC_DCC_M_			_NCCC_DCC_FILE_MASTERCARD_DISCLAIMER_

#define _ESC_SMALL_CARD_TYPE_MULTIPLE_	3
#define _ESC_SMALL_CARD_TYPE_OFFSET_	-25

#define _ESC_AID_TOTAL_LEN_		30
#define _ESC_AP_LABEL_TOTAL_SPACE_	31
#define _ESC_UPPER_MULTIPLE_		2
#define _ESC_LOWER_MULTIPLE_		1

typedef enum
{
	_REPORT_INDEX_ESC_NORMAL_ = 0,
	_REPORT_INDEX_ESC_NORMAL_DCC_FOR_SALE_,
	_REPORT_INDEX_ESC_NORMAL_DCC_NOT_FOR_SALE_,
	_REPORT_INDEX_ESC_NORMAL_DCC_CHANGE_TWD_,
	_REPORT_INDEX_ESC_NORMAL_FISC_,
	_REPORT_INDEX_ESC_NORMAL_HG_MULTIPLE_,
	_REPORT_INDEX_ESC_SMALL_,
	_REPORT_INDEX_ESC_DCC_FOR_SALE_SMALL_,
	_REPORT_INDEX_ESC_DCC_NOT_FOR_SALE_SMALL_,
	_REPORT_INDEX_ESC_DCC_CHANGE_TWD_SMALL_,
	_REPORT_INDEX_ESC_FISC_SMALL_,
	_REPORT_INDEX_ESC_HG_MULTIPLE_SMALL_,
	_REPORT_INDEX_ESC_NORMAL_027_,
	_REPORT_INDEX_ESC_NORMAL_FISC_027_,
	_REPORT_INDEX_ESC_NORMAL_HG_MULTIPLE_027_,
	_REPORT_INDEX_ESC_NORMAL_DCC_FOR_SALE_061_,
	_REPORT_INDEX_ESC_NORMAL_DCC_NOT_FOR_SALE_061_,
	_REPORT_INDEX_ESC_NORMAL_DCC_CHANGE_TWD_061_,
	_REPORT_INDEX_ESC_NORMAL_043_,
	_REPORT_INDEX_ESC_NORMAL_005_,
	_REPORT_INDEX_ESC_NORMAL_HG_MULTIPLE_005,
	_REPORT_INDEX_ESC_SMALL_S_,
	_REPORT_INDEX_ESC_DCC_FOR_SALE_SMALL_S_,
	_REPORT_INDEX_ESC_DCC_NOT_FOR_SALE_SMALL_S_,
	_REPORT_INDEX_ESC_DCC_CHANGE_TWD_SMALL_S_,
	_REPORT_INDEX_ESC_FISC_SMALL_S_,
	_REPORT_INDEX_ESC_HG_MULTIPLE_SMALL_S_,
} RECEIPT_INDEX_ESC;

/* 列印帳單 (START) */
typedef struct
{
	int (*inLogo)(TRANSACTION_OBJECT *, char *);			        /* LOGO */
	int (*inTop)(TRANSACTION_OBJECT *, char *);				/* TID and MID */
	int (*inData)(TRANSACTION_OBJECT *, char *);				/* DTAT */
	int (*inCUPAmount)(TRANSACTION_OBJECT *, char *);			/* CUP_AMOUNT */
	int (*inAmount)(TRANSACTION_OBJECT *, char *);				/* AMOUNT */
	int (*inInstallment)(TRANSACTION_OBJECT *, char *);			/* 分期資料 */
	int (*inRedemption)(TRANSACTION_OBJECT *, char *);			/* 紅利資料 */
	int (*inHappyGo)(TRANSACTION_OBJECT *, char *);				/* HAPPY_GO資料 */
	int (*inEnd)(TRANSACTION_OBJECT *, char *);				/* 簽名欄 */
} ESC_RECEIPT_TYPE_TABLE;
/* 列印帳單 (END) */

int inNCCC_ESC_Make_E1Data(TRANSACTION_OBJECT *pobTran);
int inNCCC_ESC_Open_File(unsigned long *ulHandle, char *szFileName, int inDelete);
int inNCCC_ESC_Close_File(unsigned long *ulHandle);
int inNCCC_ESC_Append_E1(unsigned long *ulESCHandle, char *szE1Temp, int inDataLen);
int inNCCC_ESC_PRINT_LOGO(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_TIDMID(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DATA(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_CUP_AMOUNT(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_AMOUNT(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_INST(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_REDEEM(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_ReceiptEND(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_HG_Multi_Data(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_ReceiptEND_027(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_ReceiptEND_061(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_Disclaimer_061_VISA(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_Disclaimer_061_MASTERCARD(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DATA_005(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_TIDMID_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DATA_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_CUP_AMOUNT_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_AMOUNT_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_INST_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_REDEEM_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_ReceiptEND_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_DATA_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_ReceiptEND_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_FISC_Data_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_FISC_Amount_Small(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DATA_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_AMOUNT_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_ReceiptEND_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_ReceiptEND_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_DATA_Small_S(TRANSACTION_OBJECT *pobTran, char *szPackData);

/* DCC */
int inNCCC_ESC_PRINT_DCC_DATA(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_AMOUNT_FOR_SALE(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_Amount_NOT_FOR_SALE(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_Amount_CHANGE_NTD(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_DCC_ReceiptEND(TRANSACTION_OBJECT *pobTran, char *szPackData);

/* SMARTPAY */
int inNCCC_ESC_PRINT_FISC_Data(TRANSACTION_OBJECT *pobTran, char *szPackData);
int inNCCC_ESC_PRINT_FISC_Amount(TRANSACTION_OBJECT *pobTran, char *szPackData);	      

/* 功能性 */
int inNCCC_ESC_PRINT_DCC_Disclaimer(char *szFilename, char *szPackData);
int inNCCC_ESC_PRINT_Format_Auto_Change_Line(char* szString, char *szPackData);