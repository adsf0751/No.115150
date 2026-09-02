#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/PRINT/PrtMsg.h"

extern BMPHeight	gsrBMPHeight;
extern unsigned short	gusPrintFontStyleRegular;

int inTEST_DISPLAY(void)
{
	//char szLine1[8+1];
	
	//memset(szLine1, "謝謝光臨謝謝惠顧", sizeof(szLine1));
	
	inDISP_ClearAll();
	inDISP_ChineseFont("謝謝光臨謝謝惠顧", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	inDISP_ChineseFont("一二三四五六七八", _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);
	inDISP_ChineseFont("ＡＢＣＤＥＦＧＨ", _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
	inDISP_ChineseFont("端末機顯示器測試", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
	inDISP_ChineseFont("謝謝光臨謝謝惠顧", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont("一二三四五六七八", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	inDISP_ChineseFont("ＡＢＣＤＥＦＧＨ", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
	inDISP_ChineseFont("端末機顯示器測試", _FONTSIZE_8X16_, _LINE_8_8_, _DISP_LEFT_);
	uszKBD_GetKey(30);
        
	inDISP_ClearAll();
	inDISP_ChineseFont("12345678901234567890123", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRSTUVW", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
	inDISP_ChineseFont("12345678901234567890123", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRSTUVW", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	inDISP_ChineseFont("12345678901234567890123", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRSTUVW", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	inDISP_ChineseFont("12345678901234567890123", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRSTUVW", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	uszKBD_GetKey(30);

	inDISP_ClearAll();
	inDISP_ChineseFont("1234567890123456789", _FONTSIZE_12X19_, _LINE_16_1_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRS", _FONTSIZE_12X19_, _LINE_16_2_, _DISP_LEFT_);
	inDISP_ChineseFont("1234567890123456789", _FONTSIZE_12X19_, _LINE_16_3_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRS", _FONTSIZE_12X19_, _LINE_16_4_, _DISP_LEFT_);
	inDISP_ChineseFont("1234567890123456789", _FONTSIZE_12X19_, _LINE_16_5_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRS", _FONTSIZE_12X19_, _LINE_16_6_, _DISP_LEFT_);
	inDISP_ChineseFont("1234567890123456789", _FONTSIZE_12X19_, _LINE_16_7_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRS", _FONTSIZE_12X19_, _LINE_16_8_, _DISP_LEFT_);
	inDISP_ChineseFont("1234567890123456789", _FONTSIZE_12X19_, _LINE_16_9_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRS", _FONTSIZE_12X19_, _LINE_16_10_, _DISP_LEFT_);
	inDISP_ChineseFont("1234567890123456789", _FONTSIZE_12X19_, _LINE_16_11_, _DISP_LEFT_);
	inDISP_ChineseFont("ABCDEFGHIJKLMNOPQRS", _FONTSIZE_12X19_, _LINE_16_12_, _DISP_LEFT_);
	uszKBD_GetKey(30);		
        
	inDISP_ClearAll();
	inDISP_EnglishFont("1234567890123456", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);
	inDISP_EnglishFont("ABCDEFGHIJKLMNOP", _FONTSIZE_8X16_, _LINE_8_2_, _DISP_LEFT_);
	inDISP_EnglishFont("1234567890123456", _FONTSIZE_8X16_, _LINE_8_3_, _DISP_LEFT_);
	inDISP_EnglishFont("ABCDEFGHIJKLMNOP", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
	inDISP_EnglishFont("1234567890123456", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_EnglishFont("ABCDEFGHIJKLMNOP", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	inDISP_EnglishFont("1234567890123456", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
	inDISP_EnglishFont("ABCDEFGHIJKLMNOP", _FONTSIZE_8X16_, _LINE_8_8_, _DISP_LEFT_);
	uszKBD_GetKey(30);
	
	inDISP_ClearAll();
	inDISP_PutGraphic("./fs_data/test/test_screen.bmp", 0, _COORDINATE_Y_LINE_8_1_);
	uszKBD_GetKey(30);
	
	return (VS_SUCCESS);
}

//int inTEST_PRINT_DEMO(void)
//{
//	
//	int		i, j;
//	int		inFont;
//	char		szPrintBuffer[100 + 1];
//	char		szNumber[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
//	char		szAlphaCapital[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
//	char		szAlpha[26] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
//	
//	{
//		inFont = _PRT_NORMAL_;
//		
//		inPRINT_ChineseFont("一二三四五六七八九十一二三", inFont);
//		for (i = 0; i < 26; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szAlpha[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		for (i = 0; i < 26; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szAlphaCapital[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		for (i = 0; i < 10; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szNumber[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		inPRINT_ChineseFont("::::::::::::::::::::::::::", inFont);
//		inPRINT_ChineseFont("--------------------------", inFont);
//		inPRINT_ChineseFont(",,,,,,,,,,,,,,,,,,,,,,,,,,", inFont);
//		inPRINT_ChineseFont("..........................", inFont);
//		inPRINT_ChineseFont("//////////////////////////", inFont);
//	}
//	
//	
//	{
//		inFont = _PRT_NORMAL2_;
//		
//		inPRINT_ChineseFont("一二三四五六七八九十一二三", inFont);
//		for (i = 0; i < 26; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szAlpha[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		for (i = 0; i < 26; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szAlphaCapital[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		for (i = 0; i < 10; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szNumber[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		inPRINT_ChineseFont("::::::::::::::::::::::::::", inFont);
//		inPRINT_ChineseFont("--------------------------", inFont);
//		inPRINT_ChineseFont(",,,,,,,,,,,,,,,,,,,,,,,,,,", inFont);
//		inPRINT_ChineseFont("..........................", inFont);
//		inPRINT_ChineseFont("//////////////////////////", inFont);
//	}
//	
//	
//	{
//		inFont = _PRT_HEIGHT_;
//		
//		inPRINT_ChineseFont("一二三四五六七八九十一二三", inFont);
//		for (i = 0; i < 26; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szAlpha[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		for (i = 0; i < 26; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szAlphaCapital[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		for (i = 0; i < 10; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szNumber[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//		
//		inPRINT_ChineseFont("::::::::::::::::::::::::::", inFont);
//		inPRINT_ChineseFont("--------------------------", inFont);
//		inPRINT_ChineseFont(",,,,,,,,,,,,,,,,,,,,,,,,,,", inFont);
//		inPRINT_ChineseFont("..........................", inFont);
//		inPRINT_ChineseFont("//////////////////////////", inFont);
//	}
//	
//	{
//		inFont = _PRT_HEIGHT_HEIGHT_;
//		
//		inPRINT_ChineseFont("一二三四五六七八九十一二三", inFont);
//		for (i = 0; i < 26; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szAlpha[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		for (i = 0; i < 26; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szAlphaCapital[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//
//		for (i = 0; i < 10; i++)
//		{
//			memset(szPrintBuffer, 0x00, sizeof(szPrintBuffer));
//			for (j = 0; j < 26; j++)
//			{
//				memcpy(&szPrintBuffer[j], &szNumber[i], 1);	
//			}
//			inPRINT_ChineseFont(szPrintBuffer, inFont);
//		}
//	
//		inPRINT_ChineseFont("::::::::::::::::::::::::::", inFont);
//		inPRINT_ChineseFont("--------------------------", inFont);
//		inPRINT_ChineseFont(",,,,,,,,,,,,,,,,,,,,,,,,,,", inFont);
//		inPRINT_ChineseFont("..........................", inFont);
//		inPRINT_ChineseFont("//////////////////////////", inFont);
//	}
//	
//	return (VS_SUCCESS);
//}

//int inTEST_Print0(void)
//{
//	char		szPrintBuf[84 + 1], szTemplate[42 + 1], szTemplate1[84 + 1], szTemplate2[84 + 1];
//	
////	inPRINT_ChineseFont("123456789012345678901234567890123456789012345678901234567890", _PRT_HEIGHT_);
////	inPRINT_ChineseFont("------------------------------------------------------------", _PRT_HEIGHT_);
////	inPRINT_ChineseFont("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", _PRT_HEIGHT_);
////	inPRINT_ChineseFont("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", _PRT_HEIGHT_);
//	int			inRetVal;
//	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
//	BufferHandle		srBhandle;
//	FONT_ATTRIB	srFont_Attrib;
//	
//	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
//	inPRINT_Buffer_PutGraphic((unsigned char*)"./fs_data/test.bmp", uszBuffer, &srBhandle, 210, _COVER_);
//	/* 橫式 */
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//
//	/* 列印商店代號 */
//	inPAD_ASCII(szTemplate, "6601000081", ' ', 15, _RIGHT_);
//	sprintf(szPrintBuf, "%s", szTemplate);
//	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, &srBhandle, &srFont_Attrib, _LAST_ENTRY_, 249);
//
//	if (inRetVal != VS_SUCCESS)
//		return (VS_ERROR);
//
//	/* Get端末機代號 */
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//
//	/* 列印端末機代號 */
//	inPAD_ASCII(szTemplate, "13995512", ' ', 8, _RIGHT_);
//	sprintf(szPrintBuf, "%s", szTemplate);
//	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, &srBhandle, &srFont_Attrib, _LAST_ENTRY_, 320);
//
//	if (inRetVal != VS_SUCCESS)
//		return (VS_ERROR);
//	
//	/* "卡號 卡別" */
//	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//	
//	inPAD_ASCII(szTemplate, "AMEX", ' ', 9, _RIGHT_);
//	sprintf(szTemplate2, "%s", szTemplate);
//	
//	/* 合併 */
//	sprintf(szPrintBuf, "%s", szTemplate2);
//	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, &srBhandle, &srFont_Attrib, _LAST_ENTRY_, 303);
//
//	if (inRetVal != VS_SUCCESS)
//		return (VS_ERROR);
//		
//	/* 卡號值 */
//	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//	memset(szTemplate1, 0x00, sizeof(szTemplate1));
//
////	strcpy(szTemplate1, pobTran->srBRec.szPAN);
//	strcpy(szTemplate1, "376348129192026");
//	/* 卡號遮掩 */
////	if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
////		memcpy(&szTemplate1[6], "******",6);
//
//	/* 奇卡號、偶卡號 */
////	if (strlen(pobTran->srBRec.szPAN) % 2 == 1)
////	{
//		strcat(szTemplate1, "(S)");
////	}
////	else
////	{
////		strcat(szTemplate1, "(C)");
////	}
//	sprintf(szPrintBuf, "%s", szTemplate1);
//
//	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_WIDTH_WIDTH_, uszBuffer, &srBhandle, &srFont_Attrib, _LAST_ENTRY_, 0);
//	
//
//	if (inRetVal != VS_SUCCESS)
//		return (VS_ERROR);
//	
//	/* 交易別 */
//	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//	memset(szTemplate1, 0x00, sizeof(szTemplate1));
//
////	inFunc_GetTransType(pobTran, szTemplate1);
//	sprintf(szPrintBuf, "%s", "00 一般交易 SALE");
//
//	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, &srBhandle, &srFont_Attrib, _LAST_ENTRY_, 44);
//
//	if (inRetVal != VS_SUCCESS)
//		return (VS_ERROR);
//		
//	/* 城市 主機 */
//	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	memset(szTemplate1, 0x00, sizeof(szTemplate1));
//	memset(szTemplate2, 0x00, sizeof(szTemplate2));
//
//	/* 前半段 */
////	inGetCityName(szTemplate);
//	sprintf(szTemplate1, "%s", "KIMEN-LIENCHIANG");
//	sprintf(szPrintBuf, "%s", szTemplate1);
//	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, &srBhandle, &srFont_Attrib, _CURRET_LINE_, 44);
//
//	if (inRetVal != VS_SUCCESS)
//		return (VS_ERROR);
//
//	/* 後半段*/
//	memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
//	memset(szTemplate, 0x00, sizeof(szTemplate));
//	memset(szTemplate1, 0x00, sizeof(szTemplate1));
//	memset(szTemplate2, 0x00, sizeof(szTemplate2));
//	
////	inGetHostLabel(szTemplate);
//	inPAD_ASCII(szTemplate, "GMMM", ' ', 9, _RIGHT_);
//	sprintf(szTemplate2, "%s", szTemplate);
//
//	sprintf(szPrintBuf, "%s", szTemplate2);
//	inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_NORMAL2_, uszBuffer, &srBhandle, &srFont_Attrib, _LAST_ENTRY_, 303);
//
//	if (inRetVal != VS_SUCCESS)
//		return (VS_ERROR);
//		
//	inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
//	
//	return (VS_SUCCESS);
//}

int inTEST_Print1(void)
{
	int			i;
	int			j;
	char			szPrintBuf[84 + 1] = {0}, szTemplate[42 + 1] = {0}, szTemp;
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	unsigned long		ulRunTime = 0;
	unsigned long		ulSecond = 0;
	unsigned long		ulMilliSecond = 0;
	RTC_NEXSYS		srRTCStart;
	RTC_NEXSYS		srRTCEnd;	
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_REGULAR_);
	
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
	inPRINT_Buffer_PutIn("================================================================================", _PRT_FOR_TEST_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutIn("一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十", _PRT_FOR_TEST_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	for (i = 1; i < 100; i++)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(&szTemp, 0x00, sizeof(szTemp));
		
		switch (i / 10)
		{
			case 0:
				strcat(&szTemp, "零");
				break;
			case 1:
				strcat(&szTemp, "壹");
				break;
			case 2:
				strcat(&szTemp, "貳");
				break;
			case 3:
				strcat(&szTemp, "參");
				break;
			case 4:
				strcat(&szTemp, "肆");
				break;
			case 5:
				strcat(&szTemp, "伍");
				break;
			case 6:
				strcat(&szTemp, "陸");
				break;
			case 7:
				strcat(&szTemp, "柒");
				break;
			case 8:
				strcat(&szTemp, "捌");
				break;
			case 9:
				strcat(&szTemp, "玖");
				break;
			default:
				strcat(&szTemp, "零");
				break;
		}
		strcat(szPrintBuf, &szTemp);
		
		memset(&szTemp, 0x00, sizeof(szTemp));
		switch (i % 10)
		{
			case 0:
				sprintf(&szTemp, "零");
				break;
			case 1:
				strcat(&szTemp, "壹");
				break;
			case 2:
				strcat(&szTemp, "貳");
				break;
			case 3:
				strcat(&szTemp, "參");
				break;
			case 4:
				strcat(&szTemp, "肆");
				break;
			case 5:
				strcat(&szTemp, "伍");
				break;
			case 6:
				strcat(&szTemp, "陸");
				break;
			case 7:
				strcat(&szTemp, "柒");
				break;
			case 8:
				strcat(&szTemp, "捌");
				break;
			case 9:
				strcat(&szTemp, "玖");
				break;
			default:
				strcat(&szTemp, "零");
				break;
		}
		strcat(szPrintBuf, &szTemp);
		
		strcat(szPrintBuf, "  ");
		
		for (j = 0; j < 21; j++)
		{
			strcat(szPrintBuf, &szTemp);
		}
		
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_FOR_TEST_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	inPRINT_Buffer_PutIn("================================================================================", _PRT_FOR_TEST_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	for (i = 0; i < 1; i++)
	{
		/* 列印日期 / 時間 */
		memset(&srRTCStart, 0x00, sizeof(srRTCStart));
		if (inFunc_GetSystemDateAndTime(&srRTCStart) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "Start Time： ");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d:%02d:%02d", srRTCStart.uszHour, srRTCStart.uszMinute, srRTCStart.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		ulRunTime = ulFunc_CalculateRunTime_Start();
		/* 列印 */
		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
		
		inFunc_GetRunTime(ulRunTime, &ulSecond, &ulMilliSecond);

		/* 列印日期 / 時間 */
		memset(&srRTCEnd, 0x00, sizeof(srRTCEnd));
		if (inFunc_GetSystemDateAndTime(&srRTCEnd) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szPrintBuf, "End Time： ");

		sprintf(szTemplate, "%02d:%02d:%02d", srRTCEnd.uszHour, srRTCEnd.uszMinute, srRTCEnd.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
	
		/* 經過時間(精度高) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szTemplate, "%s%lu.%lu%s", "Total Print Time = ", ulSecond, ulMilliSecond,"Sec");
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		
		inPRINT_SpaceLine(1);
	}
	
	inPRINT_SpaceLine(8);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
	return (VS_SUCCESS);
}

int inTEST_Print2(void)
{
	int			inTenPosition = 0;
	int			i;
	int			j;
	char			szPrintBuf[84 + 1] = {0}, szTemplate[42 + 1] = {0}, szTemp;
	char			szNumber1[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	char			szNumber2[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
	char			szAlphaCapital[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_] = {0};
	unsigned long		ulSecond = 0;
	unsigned long		ulMilliSecond = 0;
	unsigned long		ulRunTime = 0;
	RTC_NEXSYS		srRTCStart;
	RTC_NEXSYS		srRTCEnd;	
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	
	inPRINT_TTF_SetFont_Style(_FONT_PRINT_REGULAR_);
	
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
	inPRINT_Buffer_PutIn("================================================================================", _PRT_FOR_TEST_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	for (i = 1; i <= 100; i++)
	{
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		
		inTenPosition = ((i / 10) % 10);
		memset(&szTemp, 0x00, sizeof(szTemp));
		memcpy(&szTemp, &szNumber1[inTenPosition], 1);
		memcpy(&szPrintBuf[0], &szTemp, 1);
		
		memset(&szTemp, 0x00, sizeof(szTemp));
		memcpy(&szTemp, &szNumber1[i % 10], 1);
		memcpy(&szPrintBuf[1], &szTemp, 1);
		
		strcat(szPrintBuf, ".");
		
		if (i <= 10)
		{
			for (j = 0; j < 45; j++)
			{
				memset(&szTemp, 0x00, sizeof(szTemp));
				memcpy(&szTemp, &szNumber2[i - 1], 1);
				memcpy(&szPrintBuf[3 + j], &szTemp, 1);
			}
			
		}
		else if (i >= 11 && i <= 36)
		{
			for (j = 0; j < 45; j++)
			{
				memset(&szTemp, 0x00, sizeof(szTemp));
				memcpy(&szTemp, &szAlphaCapital[i - 11], 1);
				memcpy(&szPrintBuf[3 + j], &szTemp, 1);
			}
		}
		else if (i == 37 || i == 39)
		{
			memcpy(&szPrintBuf[3], "12345678901234567890123451234567890123456789012345", 45);
		}
		else if (i == 38 || i == 40)
		{
			memcpy(&szPrintBuf[3], "ABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXY", 45);
		}
		else if (i >= 41 && i <= 50)
		{
			for (j = 0; j < 45; j++)
			{
				memset(&szTemp, 0x00, sizeof(szTemp));
				memcpy(&szTemp, &szAlphaCapital[0], 1);
				memcpy(&szPrintBuf[3 + j], &szTemp, 1);
			}
		}
		else if (i >= 51 && i <= 60)
		{
			for (j = 0; j < 45; j++)
			{
				memset(&szTemp, 0x00, sizeof(szTemp));
				memcpy(&szTemp, &szAlphaCapital[1], 1);
				memcpy(&szPrintBuf[3 + j], &szTemp, 1);
			}
		}
		else if (i >= 61 && i <= 70)
		{
			for (j = 0; j < 45; j++)
			{
				memset(&szTemp, 0x00, sizeof(szTemp));
				memcpy(&szTemp, &szAlphaCapital[2], 1);
				memcpy(&szPrintBuf[3 + j], &szTemp, 1);
			}
		}
		else if (i >= 71 && i <= 80)
		{
			for (j = 0; j < 45; j++)
			{
				memset(&szTemp, 0x00, sizeof(szTemp));
				memcpy(&szTemp, &szAlphaCapital[3], 1);
				memcpy(&szPrintBuf[3 + j], &szTemp, 1);
			}
		}
		else if (i >= 81 && i <= 90)
		{
			for (j = 0; j < 45; j++)
			{
				memset(&szTemp, 0x00, sizeof(szTemp));
				memcpy(&szTemp, &szAlphaCapital[4], 1);
				memcpy(&szPrintBuf[3 + j], &szTemp, 1);
			}
		}
		else if (i >= 91 && i <= 100)
		{
			for (j = 0; j < 45; j++)
			{
				memset(&szTemp, 0x00, sizeof(szTemp));
				memcpy(&szTemp, &szAlphaCapital[5], 1);
				memcpy(&szPrintBuf[3 + j], &szTemp, 1);
			}
		}
		
		inPRINT_Buffer_PutIn(szPrintBuf, _PRT_FOR_TEST_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	}
	inPRINT_Buffer_PutIn("================================================================================", _PRT_FOR_TEST_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	
	for (i = 0; i < 1; i++)
	{
		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCStart) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "Start Time： ");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d:%02d:%02d", srRTCStart.uszHour, srRTCStart.uszMinute, srRTCStart.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		ulRunTime = ulFunc_CalculateRunTime_Start();
		/* 列印 */
		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
		
		inFunc_GetRunTime(ulRunTime, &ulSecond, &ulMilliSecond);

		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCEnd) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szPrintBuf, "End Time： ");

		sprintf(szTemplate, "%02d:%02d:%02d", srRTCEnd.uszHour, srRTCEnd.uszMinute, srRTCEnd.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
	
		/* 經過時間(精度高) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szTemplate, "%s%lu.%lu%s", "Total Print Time = ", ulSecond, ulMilliSecond,"Sec");
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		
		inPRINT_SpaceLine(1);
	}
	
	inPRINT_SpaceLine(8);
	
	inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
	
	return (VS_SUCCESS);
}

int inTEST_Print3(void)
{
	
	int			i;
	char			szPrintBuf[84 + 1], szTemplate[42 + 1];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	unsigned long		ulSecond = 0;
	unsigned long		ulMilliSecond = 0;
	unsigned long		ulRunTime = 0;
	RTC_NEXSYS		srRTCStart;
	RTC_NEXSYS		srRTCEnd;	
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
	
	/* NCCC LOGO */
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutGraphic((unsigned char*)_BANK_LOGO_, uszBuffer, &srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_);
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	for (i = 0; i < 1; i++)
	{
		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCStart) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "Start Time： ");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d:%02d:%02d", srRTCStart.uszHour, srRTCStart.uszMinute, srRTCStart.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		ulRunTime = ulFunc_CalculateRunTime_Start();
		/* 列印 */
		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
		
		inFunc_GetRunTime(ulRunTime, &ulSecond, &ulMilliSecond);

		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCEnd) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szPrintBuf, "End Time： ");

		sprintf(szTemplate, "%02d:%02d:%02d", srRTCEnd.uszHour, srRTCEnd.uszMinute, srRTCEnd.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
	
		/* 經過時間(精度高) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szTemplate, "%s%lu.%lu%s", "Total Print Time = ", ulSecond, ulMilliSecond,"Sec");
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		
		inPRINT_SpaceLine(1);
	}
	
	/* Full black */
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutGraphic((unsigned char*)"./fs_data/test/TEST_b.bmp", uszBuffer, &srBhandle, 240, _APPEND_);
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	for (i = 0; i < 1; i++)
	{
		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCStart) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "Start Time： ");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d:%02d:%02d", srRTCStart.uszHour, srRTCStart.uszMinute, srRTCStart.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		ulRunTime = ulFunc_CalculateRunTime_Start();
		/* 列印 */
		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
		
		inFunc_GetRunTime(ulRunTime, &ulSecond, &ulMilliSecond);

		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCEnd) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szPrintBuf, "End Time： ");

		sprintf(szTemplate, "%02d:%02d:%02d", srRTCEnd.uszHour, srRTCEnd.uszMinute, srRTCEnd.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
	
		/* 經過時間(精度高) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szTemplate, "%s%lu.%lu%s", "Total Print Time = ", ulSecond, ulMilliSecond,"Sec");
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		
		inPRINT_SpaceLine(1);
	}
	
	/* Half black white */
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutGraphic((unsigned char*)"./fs_data/test/TEST_bw.bmp", uszBuffer, &srBhandle, 240, _APPEND_);
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	for (i = 0; i < 1; i++)
	{
		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCStart) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "Start Time： ");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d:%02d:%02d", srRTCStart.uszHour, srRTCStart.uszMinute, srRTCStart.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		ulRunTime = ulFunc_CalculateRunTime_Start();
		/* 列印 */
		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
		
		inFunc_GetRunTime(ulRunTime, &ulSecond, &ulMilliSecond);

		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCEnd) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szPrintBuf, "End Time： ");

		sprintf(szTemplate, "%02d:%02d:%02d", srRTCEnd.uszHour, srRTCEnd.uszMinute, srRTCEnd.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
	
		/* 經過時間(精度高) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szTemplate, "%s%lu.%lu%s", "Total Print Time = ", ulSecond, ulMilliSecond,"Sec");
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		
		inPRINT_SpaceLine(1);
	}
	
	/* Full white */
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutGraphic((unsigned char*)"./fs_data/test/TEST_w.bmp", uszBuffer, &srBhandle, 240, _APPEND_);
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	for (i = 0; i < 1; i++)
	{
		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCStart) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "Start Time： ");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d:%02d:%02d", srRTCStart.uszHour, srRTCStart.uszMinute, srRTCStart.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		ulRunTime = ulFunc_CalculateRunTime_Start();
		/* 列印 */
		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
		
		inFunc_GetRunTime(ulRunTime, &ulSecond, &ulMilliSecond);

		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCEnd) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szPrintBuf, "End Time： ");

		sprintf(szTemplate, "%02d:%02d:%02d", srRTCEnd.uszHour, srRTCEnd.uszMinute, srRTCEnd.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
	
		/* 經過時間(精度高) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szTemplate, "%s%lu.%lu%s", "Total Print Time = ", ulSecond, ulMilliSecond,"Sec");
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		
		inPRINT_SpaceLine(1);
	}
	
	/* legal */
	inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	inPRINT_Buffer_PutGraphic((unsigned char*)_LEGAL_LOGO_, uszBuffer, &srBhandle, 160, _APPEND_);
	inPRINT_Buffer_PutIn("========================================", _PRT_NORMAL2_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
	for (i = 0; i < 1; i++)
	{
		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCStart) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		strcpy(szPrintBuf, "Start Time： ");
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%02d:%02d:%02d", srRTCStart.uszHour, srRTCStart.uszMinute, srRTCStart.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);

		ulRunTime = ulFunc_CalculateRunTime_Start();
		/* 列印 */
		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
		
		inFunc_GetRunTime(ulRunTime, &ulSecond, &ulMilliSecond);

		/* 列印日期 / 時間 */
		if (inFunc_GetSystemDateAndTime(&srRTCEnd) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szPrintBuf, "End Time： ");

		sprintf(szTemplate, "%02d:%02d:%02d", srRTCEnd.uszHour, srRTCEnd.uszMinute, srRTCEnd.uszSecond);
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
	
		/* 經過時間(精度高) */
		memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
		memset(szTemplate, 0x00, sizeof(szTemplate));

		sprintf(szTemplate, "%s%lu.%lu%s", "Total Print Time = ", ulSecond, ulMilliSecond,"Sec");
		strcat(szPrintBuf, szTemplate);
		inPRINT_ChineseFont(szPrintBuf, _PRT_HEIGHT_);
		
		inPRINT_SpaceLine(1);
	}
	
	inPRINT_SpaceLine(8);
	
	return (VS_SUCCESS);
}

/*
Function        :inNCCC_Test_Display_Picture
Date&Time       :2019/2/25 下午 2:34
Describe        :
*/
int inNCCC_Test_Display_Picture(void)
{
	inDISP_ClearAll();
	inDISP_PutGraphic("./fs_data/screen_test.bmp", 0, _COORDINATE_Y_LINE_8_1_);
	uszKBD_GetKey(30);
	
	return (VS_SUCCESS);
}
