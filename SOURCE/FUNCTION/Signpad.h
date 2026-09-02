
#define	SignBMPFile			"Signature.bmp"
#define	AmountBMPFile			"Amount.bmp"
#define TimeBMPFile			"Time.bmp"

#define	_SIGNATURE_BACKGROUND_LEFT_	"./fs_data/NEWUI/SIGNPAD_LEFT.bmp"
#define	_SIGNATURE_BACKGROUND_RIGHT_	"./fs_data/NEWUI/SIGNPAD_RIGHT.bmp"

#define _SIGNPAD_LEFT_			1
#define _SIGNPAD_RIGHT_			2

/* 簽名狀態 此編號按照ESC規格 要改請謹慎 */
#define _SIGN_NONE_			0		/* 應簽但還沒簽((初始狀態) */
#define	_SIGN_SIGNED_			1		/* 應簽而有簽 */
#define	_SIGN_NO_NEED_SIGN_		2		/* 免簽而沒簽 */
#define	_SIGN_BYPASS_			3		/* 應簽而沒簽 */

/* 簽名板線框寬度 */
#define _LINE_WIDTH_			4

#define _SIGNEDPAD_WIDTH_		180
#define _SIGNEDPAD_LENGTH_		320

#define _SIGNEDPAD_OUT_WIDTH_		_SIGNEDPAD_WIDTH_	+ (_LINE_WIDTH_ * 2)
#define _SIGNEDPAD_OUT_LENGTH_		_SIGNEDPAD_LENGTH_	+ (_LINE_WIDTH_ * 2)

/* 如果要置中，則x起始點為(_LCD_XSIZE_ - _SIGNEDPAD_OUT_WIDTH_) / 2 */
#define	_SIGNEDPAD_OUT_LEFT_X1_		(_LCD_XSIZE_ - _SIGNEDPAD_OUT_WIDTH_) / 2
#define	_SIGNEDPAD_OUT_LEFT_Y1_		(_LCD_YSIZE_ - _SIGNEDPAD_OUT_LENGTH_) / 2

#define	_SIGNEDPAD_OUT_RIGHT_X1_	(_LCD_XSIZE_ - _SIGNEDPAD_OUT_WIDTH_) / 2
#define	_SIGNEDPAD_OUT_RIGHT_Y1_	(_LCD_YSIZE_ - _SIGNEDPAD_OUT_LENGTH_) / 2

#define	_SIGNEDPAD_LEFT_X1_		_SIGNEDPAD_OUT_LEFT_X1_	+ _LINE_WIDTH_
#define	_SIGNEDPAD_LEFT_Y1_		_SIGNEDPAD_OUT_LEFT_Y1_	+ _LINE_WIDTH_
#define	_SIGNEDPAD_LEFT_X2_		_SIGNEDPAD_LEFT_X1_	+ _SIGNEDPAD_WIDTH_
#define	_SIGNEDPAD_LEFT_Y2_		_SIGNEDPAD_LEFT_Y1_	+ _SIGNEDPAD_LENGTH_

#define	_SIGNEDPAD_RIGHT_X1_		_SIGNEDPAD_OUT_RIGHT_X1_+ _LINE_WIDTH_
#define	_SIGNEDPAD_RIGHT_Y1_		_SIGNEDPAD_OUT_RIGHT_Y1_+ _LINE_WIDTH_
#define	_SIGNEDPAD_RIGHT_X2_		_SIGNEDPAD_RIGHT_X1_	+ _SIGNEDPAD_WIDTH_
#define	_SIGNEDPAD_RIGHT_Y2_		_SIGNEDPAD_RIGHT_Y1_	+ _SIGNEDPAD_LENGTH_

/* 沒用到 START */
//#define	_SIGNEDPAD_OUT_LEFT_X2_		_SIGNEDPAD_OUT_LEFT_X1_	+ _SIGNEDPAD_OUT_WIDTH_
//#define	_SIGNEDPAD_OUT_LEFT_Y2_		_SIGNEDPAD_OUT_LEFT_Y1_	+ _SIGNEDPAD_OUT_HIGHT_
//#define	_SIGNEDPAD_OUT_RIGHT_X2_	_SIGNEDPAD_OUT_RIGHT_X1_+ _SIGNEDPAD_OUT_WIDTH_
//#define	_SIGNEDPAD_OUT_RIGHT_Y2_	_SIGNEDPAD_OUT_RIGHT_Y1_+ _SIGNEDPAD_OUT_HIGHT_
/* 沒用到 END */

/* 此為自行定義的結構，用來紀錄簽名板相關數據 */
typedef struct
{
	int		inPosition;		/* 簽名板方向 */
	int		inSigned;		/* 是否已下筆 */
	unsigned long	ulSignTimeStart;	/* 紀錄開始簽名的時間 */
}SIGNPAD_OBJECT;

/* 功能用 */
int inSIGN_TouchSignature_Start(unsigned int uiX ,unsigned int uiY,unsigned int uiWidth ,unsigned int uiHeight,unsigned char *uszBMPFileName,unsigned long ulTimeout);
int inSIGN_TouchSignature_Internal_END(void);
int inSIGN_GetSignatureStatus(unsigned long *ulStatus , unsigned long *ulDuration);
int inSIGN_IsSigned(SIGNPAD_OBJECT *srSignpad);
int inSIGN_BMPConverter_Left(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName);
int inSIGN_BMPConverter_Right(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName);
int inSIGN_BMPConverter_OneColor(unsigned char *uszInputBMPFileName, unsigned char *uszOutputBMPFileName);
int inSIGN_Text_To_BMP(unsigned long ulwidth, unsigned long ulHeight, unsigned short usX, unsigned short usY, unsigned char *uszString,unsigned short uszFontSize,char *szBMPFileName);
int inSIGN_Text_To_BMPEx(unsigned short usX, unsigned short usY, unsigned char *uszString,unsigned short uszFontSize,char *szBMPFileName);
int inSIGN_Rotate_TextBMP(char *szFileName, int inRotate);


/* 流程 */
int inSIGN_TouchSignature_Flow(TRANSACTION_OBJECT *pobTran);
int inSIGN_TouchSignature_Internal(TRANSACTION_OBJECT *pobTran);
int inSIGN_TouchSignature_Internal_START(TRANSACTION_OBJECT *pobTran, SIGNPAD_OBJECT *srSignpad);
int inSIGN_TouchSignature_Internal_END(void);
int inSIGN_TimeoutCheck(SIGNPAD_OBJECT *srSignpad, int inTimeOut, unsigned char uszDispTimeout);
int inSIGN_TimeoutStart(unsigned long *ulRunTime);
int inSIGN_CheckSignature(TRANSACTION_OBJECT *pobTran);
int inSIGN_Check_NOSignature(TRANSACTION_OBJECT *pobTran);
int inSIGN_TouchSignature_Internal_Calibration(void);

/* 測試用 */
int inSIGN_TouchSignature_Test(void);
