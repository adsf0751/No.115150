/* 
 * File:   Print.h
 * Author: SuLH
 *
 * Created on 2015年6月8日, 上午 10:12
 */

#define d_LINE_DOT			12                  /* A line is 12 dots */

/* 定義中英文列印的字型 */
#if defined	_LOAD_KEY_AP_
	#define _PRT_CHINESE_FONE_1_		"ca_default.ttf"	/* 虹堡預設字體 */
#else
	#define _PRT_CHINESE_FONE_1_		"hkzht_p.ttf"		/* 華康中黑體(等寬) */
#endif

#define _PRT_CHINESE_FONE_2_		"pan.ttf"	/* 對卡號做特別加粗處理 */

/* 定義選擇中英文 */
#define _PRT_CHINESE_1_			1
#define _PRT_CHINESE_2_			2
#define _PRT_ENGLISH_			3

/* 定義字型Style */
#define _FONT_PRINT_REGULAR_		d_FONT_STYLE_NORMAL
#define _FONT_PRINT_ITALIC_		d_FONT_STYLE_ITALIC
#define _FONT_PRINT_BOLD_		d_FONT_STYLE_BOLD
#define _FONT_PRINT_REVERSE_		d_FONT_STYLE_REVERSE
#define _FONT_PRINT_UNDERLINE_		d_FONT_STYLE_UNDERLINE

/* 定義中英文列印字體大小 */
#define _PRT_NORMAL_                    d_FONT_8x16
#define _PRT_NORMAL2_                   0x080F			/* d_FONT_8x15(_PRT_DOUBLE_HEIGHT_WIDTH_的一半) */
#define _PRT_HEIGHT_SMALL_		0x0818			/* 08x30縮小簽單用 */
#define _PRT_WIDTH_SMALL_		0x1018			/* 16x30縮小簽單用 */
#define _PRT_HEIGHT_                    d_FONT_12x24
#define _PRT_WIDTH_                     d_FONT_16x16
#define _PRT_DOUBLE_HEIGHT_WIDTH_       d_FONT_16x30
#define _PRT_HEIGHT_HEIGHT_		0x0C30
#define _PRT_DOUBLE_WIDTH_WIDTH_	0x121E
#define _PRT_EI_HEIGHT_WIDTH_           0x203C			/* 32 x 60 */
#define _PRT_ISO_                       0x0A18
#define _PRT_12X30_			0x0C1E
#define _PRT_10X30_			0x0A1E
#define _PRT_09X23_			0x0917
#define _PRT_FOR_TEST_			0x0808
#define _PRT_046_DHW_			0x1028			/* 046使用 16*40 */
#define _PRT_046_HEIGHT_		0x0C20			/* 046使用 12*32 */
#define _PRT_046_NORMAL_		0x0828			/* 046使用 08*40 */
#define _PRT_DUTY_FREE_075_X_		0X203C			/* 昇恆昌客製化075,X要變大 */
#define _PRT_NORMAL_S_                  _PRT_NORMAL2_		/*  */
#define _PRT_AMOUNT_SMALL_		_PRT_WIDTH_SMALL_
#define _PRT_AMOUNT_			_PRT_DOUBLE_HEIGHT_WIDTH_
#define _PRT_CUS_123_DETAIL_		0x0A1E
#define _PRT_CUS_126_FONT_10X20_	0x0A14
#define _PRT_CUS_126_FONT_10X32_	0x0A20

/* EI */
#define _PRT_SH_                        0x1828                  /* 16 字 */
#define _PRT_H_                         0x1018                  /* 24 字 */
#define _PRT_B_                         0x091A                  /* 42 字 */
#define _PRT_S_                         0x0910                  /* 42 字 */

#define _PRT_EI_SH_                     "[TEXT-SH]"
#define _PRT_EI_H_                      "[TEXT-H*]"
#define _PRT_EI_B_                      "[TEXT-B*]"
#define _PRT_EI_S_                      "[TEXT-S*]"
#define _PRT_EI_QR_                     "[QR-2***]"
#define _PRT_EI_39C_                    "[39CODE*]"
#define _PRT_EI_ML_                     "[MLOGO**]"

/* Print buffer 使用 START */
#define _CURRENT_LINE_			0
#define _LAST_ENTRY_			1
/* inCover */
#define _COVER_				1	/* 用於以圖做的帳單，handle不下移 */
#define _APPEND_			2	/* 印完圖片，接著印下面*/

#define	_MAX_Y_LENGTH_			32
#define _BUFFER_MAX_LINE_		800	/* 自行定義足夠大的Buffer */

/* Aligned */
#define _PRINT_LEFT_			d_PRINTER_ALIGNLEFT
#define _PRINT_CENTER_			d_PRINTER_ALIGNCENTER
#define _PRINT_RIGHT_			d_PRINTER_ALIGNRIGHT
#define _PRINT_BY_X_			127

/* Print 定義 特殊 X座標 */
#define _PRINT_DEFINE_X_01_		192			/* 定義Print特定座標點 */
#define _PRINT_DEFINE_X_02_		216			/* 定義Print特定座標點 */
#define _PRINT_DEFINE_X_03_		120			/* QRCODE_45X45置中使用 */
#define _PRINT_DEFINE_X_04_		40			/* 筆數(CNT) */
#define _PRINT_DEFINE_X_05_		90			/* 總額報表筆數的位置 */
#define _PRINT_DEFINE_X_06_		100			/* 046電票總額報表筆數的位置 */
#define _PRINT_DEFINE_X_07_		176			/* 定義Print特定座標點(AP Label使用，否則會蓋到AE 的P Label) */
#define _PRINT_DEFINE_X_08_		90			/* 定義Print特定座標點(出發地機場)使用 */
#define _PRINT_DEFINE_X_09_		112			/* 信託交易日期 */
#define _PRINT_DEFINE_X_010_		256			/* 信託交易主機 */
#define _PRINT_DEFINE_X_011_		64			/* 信託交易交易類別 */
#define _PRINT_DEFINE_X_012_		128			/* 信託結帳交易類別 */
#define _PRINT_DEFINE_X_013_		128			/* 信託結帳筆數位置 */
#define _PRINT_DEFINE_X_014_		160			/* 信託結帳機構代碼筆數位置 */

/* Print buffer 使用 END */

#define FONT_ATTRIB			CTOS_FONT_ATTRIB
#define	QRCODE_INFO			CTOS_QRCODE_INFO
///* 定義圖檔名字 */(暫不使用)
//#define _NAME_NCCC_LOGO_		"NCCCLOGO.bmp"
//#define _NAME_MERCHANT_LOGO_		"BMPLOGO.bmp"
//#define _NAME_NAME_LOGO_		"BMPNAME.bmp"
//#define _NAME_SLOGAN_LOGO_		"BMPSLOGAN.bmp"
//#define _NAME_CUP_LEGAL_LOGO_		"CUPLEGAL.bmp"
//#define _NAME_LEGAL_LOGO_		"BMPLEGAL.bmp"
//#define _NAME_NOTICE_LOGO_		"BMPNOTICE.bmp"
        
/* 用來控制buffer位置的移動，bound是用來控制不要超出紙張或超出定義的Buffer，current表示handle現在於Buffer的什麼位置 */
/* POS Printer參數 */
#define _POS_PRINTER_BUFFER_		1024

/* 定義barcode種類 */
#define _PRINT_BARCODE_TYPE_DEFAULT_	_PRINT_BARCODE_TYPE_CODE39_
#define _PRINT_BARCODE_TYPE_CODE39_	0
#define _PRINT_BARCODE_TYPE_CODE128_	1

/* BarCode長、寬預設值 */
#define _PRINT_BARCODE_X_EXTEND_DEFAULT_	1
#define _PRINT_BARCODE_Y_EXTEND_DEFAULT_	3
#define _PRINT_BARCODE_Y_EXTEND_EI_		2

#define _NCCC_TEXT_LOGO_    "財團法人聯合信用卡處理中心"

typedef struct					
{
	int	inXbound;	/* 用來紀錄buffer的寬度，inXcurrent不應超過它 */
	int	inYbound;	/* 用來紀錄buffer的高度，inXcurrent不應超過它 */
	int	inXcurrent;	/* handle目前的X位置 */
	int	inYcurrent;	/* handle目前的Y位置 */
	int	inYcurrentMAX;	/* 紀錄當前這一行，最下面的位置(當同一行有印大字小字，用來表示底線在哪) */
	int	inYcover;	//如果有底圖，則要與inYcurrent比較那一個比較下面
	unsigned char	uszDetailPrint;	/* 列印明細顯示畫面用 避免多筆誤以為當機 */
}BufferHandle;

//typedef struct					
//{
//	int			inWay;			/* 方式：1:By 座標 2:靠左 or 靠右 */
//	int			intXPosition;		/* X軸位置 */
//	unsigned char		uszPrintPosition;	/* 靠左或靠右 */
//}PRINT_POSITION;
//
//typedef struct					
//{
//	BufferHandle		srBhandle;
//	FONT_ATTRIB		srFont_Attrib;		/* 字型資訊 */
//	PRINT_POSITION		srPrintPosition;
//}PRINT_OBJECT;

/* 用來控制PutIn在同一底線上，所以先把該行要Putin的字先存起來 */
typedef struct					
{	BufferHandle		srBhandle;
	char			szString[300];		/* 暫放的字句 (因優惠兌換資訊，所以上調到300) */
	FONT_ATTRIB		srFont_Attrib;		/* 字型資訊 */
	unsigned char		uszPrintPosition;	/* 靠左或靠右 */
	int			intXPosition;		/* X軸位置 */
	int			inFontStyle;		/* 字體字型 */
}BufferArrangeUnderLine;

/* 開機時會把圖檔的高度讀出，並放在裡面 */
typedef struct
{
	int	inBankLogoHeight;	/* 銀行LOGO高度 */
	int	inMerchantLogoHeight;	/* 商店LOGO高度 */
	int	inTitleNameHeight;	/* 表頭圖檔高度 */
	int	inSloganHeight;		/* 企業標語高度 */
	int	inCupLegalHeight;	/* CUP警語高度 */
	int	inInstHeight;		/* 分期警語高度 */
	int	inNoticeHeight;		/* 商店提示與高度 */
}BMPHeight;

typedef enum
{
	_BAUL_FONT_STYLE_UNCHANGE_ = 0,	/* 代表字體不做更改 */
	_BAUL_FONT_STYLE_REGULAR_,
	_BAUL_FONT_STYLE_ITALIC_,
	_BAUL_FONT_STYLE_BOLD_,
	_BAUL_FONT_STYLE_REVERSE_,
	_BAUL_FONT_STYLE_UNDERLINE_,
}BAUL_FONT;

int inPRINT_Initial(void);
int inPRINT_TTF_SetFont(int inLanguage);
int inPRINT_TTF_SetFont_Style(int inFontStyle);
int inPRINT_SpaceLine(int inLine);
int inPRINT_PutGraphic(unsigned char *uszFilename);
int inPRINT_ChineseFont(char *szStr, int inFontSize);
//int inPRINT_EnglishFont(char *szStr, int FontSize);		不建議再使用，為了輕量化，英文也用中文字型印
int inPRINT_TwoQR(char *szQR_1, char *szQR_2);
int inPRINT_Barcode(char *szBarcode);
int inPRINT_Buffer_Initial(unsigned char *uszBuffer, int inYLength, FONT_ATTRIB *szFont_Attrib, BufferHandle *bhandle);
int inPRINT_Buffer_PutIn(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition);
int inPRINT_Buffer_PutIn_Specific_X_Position(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, int intXPosition);
int inPRINT_Buffer_PutIn_Format_Auto_Change_Line(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition);
int inPRINT_Buffer_Sync_UnderLine(char* szString, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition, int intXPosition);
int inPRINT_Buffer_PutGraphic(unsigned char *uszFilename, unsigned char* uszBuffer1, BufferHandle *bhandle, int inGraphicYLength, int inCover);
int inPRINT_Buffer_OutPut(unsigned char *uszBuffer, BufferHandle *srBhadle);
int inPRINT_Buffer_GetHeightFlow(void);
int inPRINT_Buffer_GetHeight(unsigned char* szFileName, int* inHeight);
int inPRINT_Buffer_Barcode(char *szBarcode, unsigned char* uszBuffer /* = uszBuffer */, BufferHandle *srBhandle, unsigned short usX, unsigned char uszShowChar, int inBarCodeType /* = _PRINT_BARCODE_TYPE_DEFAULT_ */, int inExtend_X /* = _PRINT_BARCODE_X_EXTEND_DEFAULT_ */, int inExtend_Y /* = 3 */);
int inPRINT_Buffer_QRcode(char *szQRcode, unsigned char *uszBuffer, BufferHandle *srBhandle, unsigned short usX);
int inPRINT_Buffer_TwoQR(char *szQR_1, char *szQR_2, unsigned char *uszBuffer, BufferHandle *srBhandle);
int inPRINT_Buffer_BAUL_SetFont_Style(int inFontStyle);

/* Debug Printf 用 */
int inPRINT_ChineseFont_Format(char* szPrintBuffer, char* szPadData, int inOneLineLen, int inFontSize);

/*  */
int inPRINT_POS_Printer_Initial(void);
int inPRINT_POS_Printer_Wait_For_Tx_Ready(void);
int inPRINT_POS_Printer_Set_Font_Normal(void);
int inPRINT_POS_Printer_Check_Tx_OK(void);
int inPRINT_POS_Printer_Send_Data(char *szTx, int inTxLen);
int inPRINT_POS_Printer_Send_Data_Line(char *szTx, int inTxLen);
int inPRINT_POS_Printer_Cut_Paper(void);
int inPRINT_POS_Printer_DeInitial(void);

int inPRINT_Receipt_Test1(void);
int inPRINT_Receipt_Test2(void);
int inPRINT_EI_Test1(void);
int inPRINT_Buffer_Barcode_Test(void);
int inPRINT_Buffer_Barcode_Test2(void);
int inPRINT_Buffer_QRcode_Test(void);
