/*
 * File:   Display.h
 * Author: user
 *
 * Created on 2015年6月7日, 下午 4:59
 */

/* 這是用來伸縮座標用的，
 * 如果不變，
 * 則_DISTOUCH_MULTIPLIER_ 和 _DISTOUCH_DIVISOR_ 都填1 
 * 
 * 若要1/2倍
 * 則_DISTOUCH_MULTIPLIER_ 填 1
 *   _DISTOUCH_DIVISOR_    填 2 
 */
/* 注意
 * 如果要用_CASTLE_TYPE_做開關，
 *  一定要先include define_1.h，再include display.h，
 * 不然會有的座標有伸縮，有的沒伸縮 */
/* 現在改用ginHalfLCD做伸縮，_HALF_LCD_不使用 2020/11/16 下午 1:12 */
#if defined	_HALF_LCD_
	#define	_LCD_MULTIPLIER_X_	1
	#define	_LCD_DIVISOR_X_		1

	#define	_LCD_MULTIPLIER_Y_	1
	#define	_LCD_DIVISOR_Y_		2
#else
	#define	_LCD_MULTIPLIER_X_	1
	#define	_LCD_DIVISOR_X_		1
	
	#define	_LCD_MULTIPLIER_Y_	1
	#define	_LCD_DIVISOR_Y_		1
#endif

/* 顯示畫面的結構 */
#define _DISP_MSG_SIZE_         48

/* for ingenico 
#if _IWL250_
#define _MONITOR_MAX_SIZE_              230454
#define _MONITOR_QUARTER_3_SIZE_        172854
#define _MONITOR_HALF_SIZE_             115254
#define _MONITOR_QUARTER_SIZE_          57654
#elif _IWL280_
#define _MONITOR_MAX_SIZE_              172854
#define _MONITOR_QUARTER_3_SIZE_        129654
#define _MONITOR_HALF_SIZE_             86454
#define _MONITOR_QUARTER_SIZE_          43254
#else
#define _MONITOR_MAX_SIZE_              230454
#define _MONITOR_QUARTER_3_SIZE_        172854
#define _MONITOR_HALF_SIZE_             115254
#define _MONITOR_QUARTER_SIZE_          57654
#endif
 for ingenico end */

typedef struct
{
	int		inX;					/* 顯示畫面第幾列 */
	int		inY;					/* 顯示畫面第幾行 */
	int		inR_L;					/* 顯示靠左或靠右 */
	int		inMask;					/* 是否要遮掩 */
	int		inTimeout;				/* Timeout */
	int		inOutputLen;				/* 輸出字的總長度 */
	int		inMaxLen;				/* 限制最長長度 */
	int		inFoneSize;				/* 設定螢幕字型大小 */
	int		inMsgType;				/* ERROR顯示型態 */
	int		inMenuKeyIn;				/* MenuKeyIn第一位數 */
	int		inCanNotZero;				/* 設為VS_TRUE時，不接收0 */
	int		inCanNotBypass;				/* 設為VS_TRUE時，不可Bypass */
	int		inColor;				/* 輸入顯示的字串顏色 */
	int		inTouchSensorFunc;			/* 接受的觸控模式 */
	int		inDispPic1XPosition;			/* */
	int		inDispPic1YPosition;			/* */
	int		inErrMsg1Line;				/* 第一行錯誤訊息的顯示位置 */
	int		inErrMsg2Line;				/* 第二行錯誤訊息的顯示位置 */
	int		inErrMsg3Line;				/* 第三行錯誤訊息的顯示位置 */
	int		inErrMsg1FontSize;
	int		inErrMsg2FontSize;
	int		inErrMsg3FontSize;
	int		inBeepTimes;				/* 嗶聲次數 */
	int		inBeepInterval;				/* 嗶聲間隔時間(以毫秒紀錄) */
	int		inMsgPosition1;
	int		inMsgPosition2;
	int		inMsgPosition3;
	char	szPromptMsg[_DISP_MSG_SIZE_ + 1];	/* 提示字串 */
	char	szOutput[_DISP_MSG_SIZE_ + 1];		/* 輸出的字 */
	char	szMaskOutput[_DISP_MSG_SIZE_ + 1];	/* Mask輸出的字 */
	char	szErrMsg1[_DISP_MSG_SIZE_ + 1];		/* */
	char	szErrMsg2[_DISP_MSG_SIZE_ + 1];		/* */
	char	szErrMsg3[_DISP_MSG_SIZE_ + 1];		/* */
	char	szMinorUnit[2 + 1];			/* DCC使用，小數點幾位 */
	char	szCurrencyCode[3 + 1];			/* DCC使用，貨幣英文 */
	char	szDispPic1Name[50 + 1];			/* 要顯示的圖檔名字 */
	unsigned char	uszNotClearScreen;			/* 不清畫面 */
	unsigned char	uszUseCustomKeyAlphaBit;		/* 是否使用非defaulteAlphaKey */
	unsigned char   uszCustomKeyAlpha;                      /* CustomKeyAlpha */
} DISPLAY_OBJECT;

#define DISPLAY_OBJECT_SIZE		sizeof(DISPLAY_OBJECT)


/* 定義中英文的字型 */
#if defined _LOAD_KEY_AP_
	#define _CHINESE_FONE_1_                "ca_default.ttf" /* 微軟正黑體 */
#else
	#define _CHINESE_FONE_1_                "tsuyuan.ttf" /* 華康粗圓體 */
#endif

#if defined _LOAD_KEY_AP_
	#define _ENGLISH_FONE_1_                "ca_default.ttf"
#else 
	#define _ENGLISH_FONE_1_                "consola.ttf" 
#endif

/* 定義選擇中英文 */
#define _DISP_CHINESE_			1
#define _DISP_ENGLISH_			2

/* 定義字型Style */
#define _FONT_DISPLAY_REGULAR_		d_FONT_STYLE_NORMAL
#define _FONT_DISPLAY_ITALIC_		d_FONT_STYLE_ITALIC
#define _FONT_DISPLAY_BOLD_		d_FONT_STYLE_BOLD
#define _FONT_DISPLAY_REVERSE_		d_FONT_STYLE_REVERSE
#define _FONT_DISPLAY_UNDERLINE_	d_FONT_STYLE_UNDERLINE

/* HALF LCD用的字型大小 START */
/* 定義中英文FONE顯示的大小 */
#define _CHINESE_FONT_8X16_HALF_	0x0A1E	/* 8 X 8 */
#define _CHINESE_FONT_8X22_HALF_	0x0E1E	/* 8 X 22 */
#define _CHINESE_FONT_8X33_HALF_	0x0B1E	/* 8 X 33 */
#define _CHINESE_FONT_8X34_HALF_	0x0A1E	/* 8 X 33 */
#define _CHINESE_FONT_8X44_HALF_	0x081E	/* 8 X 44 */
#define _CHINESE_FONT_12X19_HALF_	0x1014	/* 12 X 19 */
#define _CHINESE_FONT_16X16_HALF_	0x0D0F	/* 16 X 16 */
#define _CHINESE_FONT_16X22_HALF_	0x0E0F	/* 16 X 22 */
#define _CHINESE_FONT_16X33_HALF_	0x0B0F	/* 16 X 33 */
#define _CHINESE_FONT_16X44_HALF_	0x080F	/* 16 X 44 */
#define _CHINESE_FONT_24X22_HALF_	0x0E0A	/* 24 X 22 */
#define _CHINESE_FONT_24X33_HALF_	0x0B0A	/* 24 X 22 */
#define _CHINESE_FONT_32X22_HALF_	0x0E08	/* 32 X 44 */

/* 改display字型的define，只有clean and build 才會生效 要0x133C才能印第八行 會有大小字問題純粹因為字太大 */
#define _ENGLISH_FONT_8X16_HALF_	0x131E	/* 8 X 16 */	/* 320 X 480 */	/* 這裡的8 * 16指的是一行可以放8列，一列可放16字，而不是pixel */
#define _ENGLISH_FONT_8X22_HALF_	0x101E	/* 8 X 22 */
#define _ENGLISH_FONT_8X33_HALF_	0x0C1E	/* 8 X 33 */
#define _ENGLISH_FONT_8X34_HALF_	0x0A1E	/* 8 X 33 */
#define _ENGLISH_FONT_8X44_HALF_	0x081E	/* 8 X 44 */
#define _ENGLISH_FONT_12X19_HALF_	0x1014	/* 12 X 19 */
#define _ENGLISH_FONT_16X16_HALF_	0x0D0F	/* 16 X 16 */
#define _ENGLISH_FONT_16X22_HALF_	0x0E0F	/* 16 X 22 */
#define _ENGLISH_FONT_16X33_HALF_	0x0B0F	/* 16 X 33 */
#define _ENGLISH_FONT_16X44_HALF_	0x080F	/* 16 X 44 */
#define _ENGLISH_FONT_24X22_HALF_	0x0E0A	/* 24 X 22 */
#define _ENGLISH_FONT_24X33_HALF_	0x0B0A	/* 24 X 22 */
#define _ENGLISH_FONT_32X22_HALF_	0x0E08	/* 32 X 44 */
/* HALF LCD用的字型大小 END */

/* FULL LCD用的字型大小 START */
#define _CHINESE_FONT_8X16_             0x143C	/* 8 X 8 */
#define _CHINESE_FONT_8X22_		0x0E3C	/* 8 X 22 */
#define _CHINESE_FONT_8X33_		0x0B3C	/* 8 X 33 */
#define _CHINESE_FONT_8X34_		0x0A3C	/* 8 X 34 */
#define _CHINESE_FONT_8X44_		0x083C	/* 8 X 44 */
#define _CHINESE_FONT_12X19_            0x1028	/* 12 X 19 */
#define _CHINESE_FONT_16X16_            0x0D1E	/* 16 X 16 */
#define _CHINESE_FONT_16X22_		0x0E1E	/* 16 X 22 */
#define _CHINESE_FONT_16X26_		0x0C1E	/* 16 X 26 */
#define _CHINESE_FONT_16X33_		0x0B1E	/* 16 X 44 */
#define _CHINESE_FONT_16X44_		0x081E	/* 16 X 44 */
#define _CHINESE_FONT_24X22_		0x0E14	/* 24 X 22 */
#define _CHINESE_FONT_24X33_		0x0B14	/* 24 X 22 */
#define _CHINESE_FONT_24X44_		0x0814	/* 24 X 44 */ /*new*/
#define _CHINESE_FONT_32X22_		0x0E0F	/* 32 X 44 */

/* 改display字型的define，只有clean and build 才會生效 要0x133C才能印第八行 會有大小字問題純粹因為字太大 */
#define _ENGLISH_FONT_8X16_             0x133C	/* 8 X 16 */	/* 320 X 480 */	/* 這裡的8 * 16指的是一行可以放8列，一列可放16字，而不是pixel */
#define _ENGLISH_FONT_8X22_		0x103C	/* 8 X 22 */
#define _ENGLISH_FONT_8X33_		0x0C3C	/* 8 X 33 */
#define _ENGLISH_FONT_8X34_		0x0A3C	/* 8 X 34 */
#define _ENGLISH_FONT_8X44_		0x083C	/* 8 X 44 */
#define _ENGLISH_FONT_12X19_            0x1028	/* 12 X 19 */
#define _ENGLISH_FONT_16X16_            0x0D1E	/* 16 X 16 */
#define _ENGLISH_FONT_16X22_		0x0E1E	/* 16 X 22 */
#define _ENGLISH_FONT_16X33_		0x0B1E	/* 16 X 22 */
#define _ENGLISH_FONT_16X44_		0x081E	/* 16 X 44 */
#define _ENGLISH_FONT_24X22_		0x0E14	/* 24 X 22 */
#define _ENGLISH_FONT_24X33_		0x0B14	/* 24 X 22 */
#define _ENGLISH_FONT_32X22_		0x0E0F	/* 32 X 44 */
/* FULL LCD用的字型大小 END */


typedef enum
{
	_FONTSIZE_8X16_,		/* 8 X 16 */
	_FONTSIZE_8X22_,		/* 8 X 22 */
	_FONTSIZE_8X33_,		/* 8 X 33 */
	_FONTSIZE_8X34_,		/* 8 X 34 */
	_FONTSIZE_8X44_,		/* 8 X 44 */
	_FONTSIZE_12X19_,		
	_FONTSIZE_16X16_,
	_FONTSIZE_16X22_,		/* 16 X 22 */
	_FONTSIZE_16X26_,		/* 16 X 26 */
	_FONTSIZE_16X33_,
	_FONTSIZE_16X44_,
	_FONTSIZE_24X22_,		/* 16 X 22 */
	_FONTSIZE_24X33_,
	_FONTSIZE_24X44_,       /* 24 X 44 */
	_FONTSIZE_32X22_,		/* 32 X 22 */
} FONTSIZE;

/* 定義中英文顯示的行數(Y Line) */
#define _LINE_8_1_			1
#define _LINE_8_2_			2
#define _LINE_8_3_			3
#define _LINE_8_4_			4
#define _LINE_8_5_			5
#define _LINE_8_6_			6
#define _LINE_8_7_			7
#define _LINE_8_8_			8

#define _LINE_16_1_			1
#define _LINE_16_2_			2
#define _LINE_16_3_			3
#define _LINE_16_4_			4
#define _LINE_16_5_			5
#define _LINE_16_6_			6
#define _LINE_16_7_			7
#define _LINE_16_8_			8
#define _LINE_16_9_			9
#define _LINE_16_10_			10
#define _LINE_16_11_			11
#define _LINE_16_12_			12
#define _LINE_16_13_			13
#define _LINE_16_14_			14
#define _LINE_16_15_			15
#define _LINE_16_16_			16

#define _LINE_12_1_			1
#define _LINE_12_2_			2
#define _LINE_12_3_			3
#define _LINE_12_4_			4
#define _LINE_12_5_			5
#define _LINE_12_6_			6
#define _LINE_12_7_			7
#define _LINE_12_8_			8
#define _LINE_12_9_			9
#define _LINE_12_10_			10
#define _LINE_12_11_			11
#define _LINE_12_12_			12

#define _LINE_24_1_			1
#define _LINE_24_2_			2
#define _LINE_24_3_			3
#define _LINE_24_4_			4
#define _LINE_24_5_			5
#define _LINE_24_6_			6
#define _LINE_24_7_			7
#define _LINE_24_8_			8
#define _LINE_24_9_			9
#define _LINE_24_10_			10
#define _LINE_24_11_			11
#define _LINE_24_12_			12
#define _LINE_24_13_			13
#define _LINE_24_14_			14
#define _LINE_24_15_			15
#define _LINE_24_16_			16
#define _LINE_24_17_			17
#define _LINE_24_18_			18
#define _LINE_24_19_			19
#define _LINE_24_20_			20
#define _LINE_24_21_			21
#define _LINE_24_22_			22
#define _LINE_24_23_			23
#define _LINE_24_24_			24

#define _LINE_32_1_			1
#define _LINE_32_2_			2
#define _LINE_32_3_			3
#define _LINE_32_4_			4
#define _LINE_32_5_			5
#define _LINE_32_6_			6
#define _LINE_32_7_			7
#define _LINE_32_8_			8
#define _LINE_32_9_			9
#define _LINE_32_10_			10
#define _LINE_32_11_			11
#define _LINE_32_12_			12
#define _LINE_32_13_			13
#define _LINE_32_14_			14
#define _LINE_32_15_			15
#define _LINE_32_16_			16
#define _LINE_32_17_			17
#define _LINE_32_18_			18
#define _LINE_32_19_			19
#define _LINE_32_20_			20
#define _LINE_32_21_			21
#define _LINE_32_22_			22
#define _LINE_32_23_			23
#define _LINE_32_24_			24
#define _LINE_32_25_			25
#define _LINE_32_26_			26
#define _LINE_32_27_			27
#define _LINE_32_28_			28
#define _LINE_32_29_			29
#define _LINE_32_30_			30
#define _LINE_32_31_			31
#define _LINE_32_32_			32

#define _LCD_XSIZE_			((320	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _LCD_YSIZE_			((480	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

/* 定義圖片y座標 */
#define _COORDINATE_Y_LINE_8_1_		0
#define _COORDINATE_Y_LINE_8_2_		(_LCD_YSIZE_ / 8) * 1
#define _COORDINATE_Y_LINE_8_3_		(_LCD_YSIZE_ / 8) * 2
#define _COORDINATE_Y_LINE_8_4_		(_LCD_YSIZE_ / 8) * 3
#define _COORDINATE_Y_LINE_8_5_		(_LCD_YSIZE_ / 8) * 4
#define _COORDINATE_Y_LINE_8_6_		(_LCD_YSIZE_ / 8) * 5
#define _COORDINATE_Y_LINE_8_7_		(_LCD_YSIZE_ / 8) * 6
#define _COORDINATE_Y_LINE_8_8_		(_LCD_YSIZE_ / 8) * 7

#define _COORDINATE_Y_LINE_16_1_	0
#define _COORDINATE_Y_LINE_16_2_	(_LCD_YSIZE_ / 16) * 1
#define _COORDINATE_Y_LINE_16_3_	(_LCD_YSIZE_ / 16) * 2
#define _COORDINATE_Y_LINE_16_4_	(_LCD_YSIZE_ / 16) * 3
#define _COORDINATE_Y_LINE_16_5_	(_LCD_YSIZE_ / 16) * 4
#define _COORDINATE_Y_LINE_16_6_	(_LCD_YSIZE_ / 16) * 5
#define _COORDINATE_Y_LINE_16_7_	(_LCD_YSIZE_ / 16) * 6
#define _COORDINATE_Y_LINE_16_8_	(_LCD_YSIZE_ / 16) * 7
#define _COORDINATE_Y_LINE_16_9_	(_LCD_YSIZE_ / 16) * 8
#define _COORDINATE_Y_LINE_16_10_	(_LCD_YSIZE_ / 16) * 9
#define _COORDINATE_Y_LINE_16_11_	(_LCD_YSIZE_ / 16) * 10
#define _COORDINATE_Y_LINE_16_12_	(_LCD_YSIZE_ / 16) * 11
#define _COORDINATE_Y_LINE_16_13_	(_LCD_YSIZE_ / 16) * 12
#define _COORDINATE_Y_LINE_16_14_	(_LCD_YSIZE_ / 16) * 13
#define _COORDINATE_Y_LINE_16_15_	(_LCD_YSIZE_ / 16) * 14
#define _COORDINATE_Y_LINE_16_16_	(_LCD_YSIZE_ / 16) * 15

#define _COORDINATE_Y_LINE_24_1_	0
#define _COORDINATE_Y_LINE_24_2_	(_LCD_YSIZE_ / 24) * 1
#define _COORDINATE_Y_LINE_24_3_	(_LCD_YSIZE_ / 24) * 2
#define _COORDINATE_Y_LINE_24_4_	(_LCD_YSIZE_ / 24) * 3
#define _COORDINATE_Y_LINE_24_5_	(_LCD_YSIZE_ / 24) * 4
#define _COORDINATE_Y_LINE_24_6_	(_LCD_YSIZE_ / 24) * 5
#define _COORDINATE_Y_LINE_24_7_	(_LCD_YSIZE_ / 24) * 6
#define _COORDINATE_Y_LINE_24_8_	(_LCD_YSIZE_ / 24) * 7
#define _COORDINATE_Y_LINE_24_9_	(_LCD_YSIZE_ / 24) * 8
#define _COORDINATE_Y_LINE_24_10_	(_LCD_YSIZE_ / 24) * 9
#define _COORDINATE_Y_LINE_24_11_	(_LCD_YSIZE_ / 24) * 10
#define _COORDINATE_Y_LINE_24_12_	(_LCD_YSIZE_ / 24) * 11
#define _COORDINATE_Y_LINE_24_13_	(_LCD_YSIZE_ / 24) * 12
#define _COORDINATE_Y_LINE_24_14_	(_LCD_YSIZE_ / 24) * 13
#define _COORDINATE_Y_LINE_24_15_	(_LCD_YSIZE_ / 24) * 14
#define _COORDINATE_Y_LINE_24_16_	(_LCD_YSIZE_ / 24) * 15
#define _COORDINATE_Y_LINE_24_17_	(_LCD_YSIZE_ / 24) * 16
#define _COORDINATE_Y_LINE_24_18_	(_LCD_YSIZE_ / 24) * 17
#define _COORDINATE_Y_LINE_24_19_	(_LCD_YSIZE_ / 24) * 18
#define _COORDINATE_Y_LINE_24_20_	(_LCD_YSIZE_ / 24) * 19
#define _COORDINATE_Y_LINE_24_21_	(_LCD_YSIZE_ / 24) * 20
#define _COORDINATE_Y_LINE_24_22_	(_LCD_YSIZE_ / 24) * 21
#define _COORDINATE_Y_LINE_24_23_	(_LCD_YSIZE_ / 24) * 22
#define _COORDINATE_Y_LINE_24_24_	(_LCD_YSIZE_ / 24) * 23

#define _COORDINATE_X_16_1_		(_LCD_XSIZE_ / 16) * 0
#define _COORDINATE_X_16_2_		(_LCD_XSIZE_ / 16) * 1
#define _COORDINATE_X_16_3_		(_LCD_XSIZE_ / 16) * 2
#define _COORDINATE_X_16_4_		(_LCD_XSIZE_ / 16) * 3
#define _COORDINATE_X_16_5_		(_LCD_XSIZE_ / 16) * 4
#define _COORDINATE_X_16_6_		(_LCD_XSIZE_ / 16) * 5
#define _COORDINATE_X_16_7_		(_LCD_XSIZE_ / 16) * 6
#define _COORDINATE_X_16_8_		(_LCD_XSIZE_ / 16) * 7
#define _COORDINATE_X_16_9_		(_LCD_XSIZE_ / 16) * 8
#define _COORDINATE_X_16_10_		(_LCD_XSIZE_ / 16) * 9
#define _COORDINATE_X_16_11_		(_LCD_XSIZE_ / 16) * 10
#define _COORDINATE_X_16_12_		(_LCD_XSIZE_ / 16) * 11
#define _COORDINATE_X_16_13_		(_LCD_XSIZE_ / 16) * 12
#define _COORDINATE_X_16_14_		(_LCD_XSIZE_ / 16) * 13
#define _COORDINATE_X_16_15_		(_LCD_XSIZE_ / 16) * 14
#define _COORDINATE_X_16_16_		(_LCD_XSIZE_ / 16) * 15

#define _COORDINATE_Y_CTLS_LOGO_Line_1_	((106	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _COORDINATE_Y_CTLS_LOGO_Line_2_	((164	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _COORDINATE_Y_CTLS_LOGO_Line_3_	((222	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _COORDINATE_Y_CTLS_LOGO_Line_4_	((280	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _COORDINATE_Y_CTLS_LOGO_Line_5_	((338	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _COORDINATE_X_CTLS_LOGO_3_1_	((27	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_3_2_	((124	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_3_3_	((221	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_4_1_	((5	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_4_2_	((85	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_4_3_	((165	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_4_4_	((245	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_5_1_	((20	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_5_2_	((80	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_5_3_	((140	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_5_4_	((200	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CTLS_LOGO_5_5_	((260	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

/* CTLS LOGO 大圖示 */
#define _COORDINATE_Y_CTLS_LOGO_BIG_Line_1_	((160	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _COORDINATE_Y_CTLS_LOGO_BIG_Line_2_	((205	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _COORDINATE_Y_CTLS_LOGO_BIG_Line_3_	((250	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _COORDINATE_Y_CTLS_LOGO_BIG_Line_4_	((285	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)
#define _COORDINATE_Y_CTLS_LOGO_BIG_Line_5_	((330	* _LCD_MULTIPLIER_Y_) / _LCD_DIVISOR_Y_)

#define _COORDINATE_X_CHOOSE_HOST_1_		((14	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CHOOSE_HOST_2_		((112	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_CHOOSE_HOST_3_		((210	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_RECEPT_LEFT_		0
#define _COORDINATE_X_RECEPT_CENTER_		(_LCD_XSIZE_ / 2)
#define _COORDINATE_X_RECEPT_LEFT_MIDDLE_	((80	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

#define _COORDINATE_X_MENU_1_			((14	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_MENU_2_			((119	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_MENU_3_			((224	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_REVIEW_STORE_ID_		((80	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)
#define _COORDINATE_X_REVIEW_STORE_ID_CUS_026_	((100	* _LCD_MULTIPLIER_X_) / _LCD_DIVISOR_X_)

/* Enter Start */
/* KeyBoard輸入型態宣告 */
#define KBD_DEC				0x0800				/* 0-9 acceptted */
#define KBD_UPPER			0x0400				/* A-Z and space */
#define KBD_LOWER			0x0200				/* a-z and space */
#define KBD_SYM				0x0100				/* !@#$...~ */
#define KBD_ALL				(KBD_DEC|KBD_UPPER|KBD_LOWER|KBD_SYM) /* convert all */
#define KBD_HIDE			(KBD_ALL | 0x0010)  		/* display '*' instead of real keystrokes */

#define _DISP_LEFT_			d_LCD_ALIGNLEFT			/* 原廠Display靠左 */
#define _DISP_RIGHT_			d_LCD_ALIGNRIGHT		/* 原廠Display靠右 */
#define _DISP_CENTER_			d_LCD_ALIGNCENTER		/* 原廠Display置中 */
/* Enter End */

/* ERROR Msg START */
#define	_NO_KEY_MSG_			0			/* 不按鍵 */	
#define _CLEAR_KEY_MSG_                 1                       /* 顯示請按清除鍵 */
#define _ENTER_KEY_MSG_                 2                       /* 請按確認鍵 */
#define _0_KEY_MSG_			3			/* 請按0確認 */
#define _REMOVE_CARD_MSG_		4			/* 拔卡觸發下一步 */
#define _ANY_KEY_MSG_			5			/* 請按任意鍵 */
/* ERROR Msg END */

/* for ingenico 
#define _TIMER_ID_       0x0000
#define _LABEL_0_ID_     0x0100
#define _BUTTON_VAL_ID_  0x0200
#define _BUTTON_CAN_ID_  0x0300
#define _BUTTON_DOWN_ID_ 0x0400
#define _BUTTON_UP_ID_   0x0500
#define _LAYOUT2_ID_     0x0600
#define _DRAWING_ID_     0x0700
#define _PICTURE_ID_     0x0800
#define _LAYOUT3_ID_     0x0900 
*/
// Result returns from the call back function
//==========================================
//#define RES_VALID_STATUS  0  // Valid key pressed or touched (Green key)
//#define RES_CANCEL_STATUS 1  // Red key pressed or touched (Red key)
//#define RES_DOWN_STATUS   2  // Down key pressed or touched
//#define RES_UP_STATUS     3  // Up key pressed or touched
//#define RES_TIMER_STATUS  4  // Timeout expiration
//#define RES_PRESS_STATUS  5  // Key pressed

/* for ingenico end */

/* 字體顏色 START */
/*
Color value. Encoding 00 BB GG RR, where  
-  RR is red color value,  
-  GG is green color value, 
-  BB is blue color value. 
*/
#define _COLOR_RED_				0x000000FF
#define _COLOR_GREEN_				0x0000FF00
#define _COLOR_BLUE_				0x00FF0000
#define _COLOR_YELLOW_				0x0000FFFF
#define _COLOR_LIGHTBLUE_			0x00F0B000
#define _COLOR_LIGHTYELLOW_			0x0066D9FF
#define _COLOR_WHITE_				0x00FFFFFF
#define _COLOR_BLACK_				0x00000000
#define _COLOR_BUTTON_GREEN_			0x00007342

/* 自定義 */
#define _COLOR_BUTTON_				0x006F2400
#define _COLOR_BUTTON_GREY_			0x00C3C3C3
/* 字體顏色 END */

#define _TIMER_NEXSYS_1_	TIMER_ID_1
#define _TIMER_NEXSYS_2_	TIMER_ID_2
#define _TIMER_NEXSYS_3_	TIMER_ID_3
#define _TIMER_NEXSYS_4_	TIMER_ID_4

#define _EDC_TIMEOUT_		-1		/* 由EDC.dat控制 */

#define _BACK_LIGHT_LCD_	d_BKLIT_LCD

/* Debug 定位用 */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT "F:"__FILE__ "|L:" TOSTRING(__LINE__) "|"

#define _MSG_BMP_DISP_POSITION_NOT_SET_	0
#define _MSG_BMP_DISP_POSITION_LEFT_	1
#define _MSG_BMP_DISP_POSITION_CENTER_	2
#define _MSG_BMP_DISP_POSITION_RIGHT_	3

int inDISP_Initial(void);
int inDISP_TTF_SetFont(int inLanguage, int inFont_Style);
int inDISP_ClearAll(void);
int inDISP_Clear_Area(int inXL,int inYL,int inXR,int inYR, int inFoneSize);
int inDISP_Clear_Box(int inXL,int inYL,int inXSize,int inYSize);
int inDISP_Clear_Line(int inLineT, int inLineB);
int inDISP_Decide_FontSize(int inFontSize, int inLanguage, unsigned short* usFontsize);
int inDISP_Select_FontSize(int inFontSize, int inLanguage);
int inDISP_ChineseFont(char *szMessage, int inFontSize, int inLINE, int inAligned);
int inDISP_ChineseFont_Color(char *szMessage, int inFontSize, int inLINE, int inColor, int inAlign);
int inDISP_ChineseFont_Point_Color(char *szMessage, int inFontSize,int inLINE, int inForeColor, int inBackColor, int inX);
int inDISP_ChineseFont_Point_Color_By_Graphic_Mode(char *szMessage, int inFontSize, int inForeColor, int inBackColor, int inX, int inY, unsigned char uszReverse);
int inDISP_EnglishFont(char *szMessage, int inFontSize, int in_LINE_, int inAlign);
int inDISP_EnglishFont_Color(char *szMessage, int inFontSize, int inLINE, int inColor, int inAlign);
int inDISP_EnglishFont_Point_Color(char *szMessage, int inFontSize,int inLINE, int inForeColor, int inBackColor, int inX);
int inDISP_PutGraphic(char *szFileName, int inX, int inY);

int inDISP_Enter8x16(DISPLAY_OBJECT  *srDispObj);
int inDISP_Enter8x16_NumDot(DISPLAY_OBJECT  *srDispObj);
int inDISP_Enter8x16_Character (DISPLAY_OBJECT  *srDispObj);
int inDISP_Enter8x16_Character_Mask (DISPLAY_OBJECT  *srDispObj);
int inDISP_Enter8x16_Character_Mask_And_DisTouch (DISPLAY_OBJECT  *srDispObj);
int inDISP_Enter8x16_GetAmount(DISPLAY_OBJECT  *srDispObj);
int inDISP_Enter8x16_GetDCCAmount(DISPLAY_OBJECT  *srDispObj);
int inDISP_Enter16X22_Character_Mask (DISPLAY_OBJECT  *srDispObj);
int inDISP_DisplayIdleMessage(void);
int inDISP_DisplayIdleMessage_NewUI(void);
int inDISP_ErrorMsg(DISPLAY_OBJECT  *srDispObj);
int inDISP_Msg_BMP(DISPLAY_OBJECT* srDispMsgObj);
int inDISP_EI_Msg_BMP(int index);
int inDISP_Enter8x16_MenuKeyIn(DISPLAY_OBJECT  *srDispObj);

unsigned char uszSWIPE_Open(void);
unsigned char uszSWIPE_Close(void);
unsigned char uszFlushSWIPEBuffer(void);
unsigned char uszKBD_Close(void);
unsigned char uszKBD_Open(unsigned char uszLs);
int inFlushKBDBuffer(void);

unsigned char uszKBD_GetKey(int timeout);
unsigned char uszKBD_Key(void);
unsigned char uszKBD_Key_Hit(void);
unsigned char uszKBD_Key_In(void);
int inKBD_Key_IsPressed(void);

int inTimerStart(int inTimerNbr, long lnDelay);
int inTimerStart_MicroSecond(int inTimerNbr, long lnDelay);
int inTimerGet(int inTimerNbr);
int inDISP_Timer_Start(int inTimerNumber, long lnDelay);
int inDISP_Timer_Start_MicroSecond(int inTimerNumber, long lnDelay);
int inDISP_LOGDisplay(char *szString, int inFontSize, int inLine, unsigned char uszStop);

/* Debug Printf 用 */
int inLogPrintf(char* szlocation, char *szStr, ...);
int inDISP_LogPrintf_Format(char* szPrintBuffer, char* szPadData, int inOneLineLen);

int inDISP_Wait(int inmSecond);
int inDISP_BEEP(int inCount, int inmSecond);
int inDISP_Sound(unsigned short usFreq, unsigned short usDuration);
int inDISP_TimeoutStart(int inTimeout);
int inDISP_TimeoutCheck(int inFontSize, int inLine, int inPlace);
int inDISP_LiveCountdown(int inTimeout, int inFontSize, int inLine, int inPlace);

int inDISP_BackLight_Set(unsigned char uszLED, unsigned char uszOnOff);
int inDISP_Display_QRCode(char* szDispMsg, int inX, int inY);
int inDISP_Display_Black_Back(unsigned short usX, unsigned short usY, unsigned short usXSize, unsigned short usYSize);
int inDISP_Display_White_Block(unsigned short usX, unsigned short usY, unsigned short usXSize, unsigned short usYSize);

int inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned(char *szMessage, int inFontSize, int inForeColor, int inBackColor, int inY, unsigned char uszReverse, int inAligned);
int inDISP_BackLightSetBrightness(unsigned char uszDevice, unsigned char uszValue);
int inDISP_PutGraphic_Rotate(char *szFileName, int inX, int inY, int inRotation);
int inDISP_TimeoutCheck_Not_Disp(void);
int inDISP_Enter8x16_Character_Mask_And_DisTouch_For_Flight_ticket(DISPLAY_OBJECT  *srDispObj);
int inDISP_Clear_Area_New(int inXL, int inYL, int inXR, int inYR, int inFoneSize);
/* [114197] 中心收單作業ESG永續發展優化  
* [20260515] 新增 inDISP_DisplayEsgQRCode() 功能 */
int inDISP_DisplayEsgQRCode(char* szDispMsg, int inX, int inY,int inSz,int inVersion);
