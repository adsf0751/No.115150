#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <ctosapi.h>
#include <ctos_clapi.h>
#include <ctos_qrcode.h>
#include <emv_cl.h>
#include <iconv.h>
#include <pthread.h>
#include <fcntl.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/types.h>		//2019/3/5 上午 10:00 Added by Hachi
#include <sys/stat.h>		//2019/3/5 上午 10:00 Added by Hachi
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../PRINT/Print.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Menu.h"
#include "../EVENT/Flow.h"
#include "../EVENT/Event.h"
#include "../COMM/Comm.h"
#include "../COMM/TLS.h"
#include "../COMM/Bluetooth.h"
#include "../COMM/Ethernet.h"
#include "../COMM/GSM.h"
#include "../COMM/GPRS.h"
#include "../../HG/HGsrc.h"
#include "../../HG/HGiso.h"
#include "Sqlite.h"
#include "Accum.h"
#include "Batch.h"
#include "Card.h"
#include "CDT.h"
#include "CFGT.h"
#include "CPT.h"
#include "EDC.h"
#include "File.h"
#include "Function.h"
#include "FuncTable.h"
#include "HDT.h"
#include "HDPT.h"
#include "KMS.h"
#include "MVT.h"
#include "PWD.h"
#include "ECR.h"
#include "RS232.h"
#include "MultiFunc.h"
#include "PowerManagement.h"
#include "XML.h"
#include "VWT.h"
#include "SCDT.h"
#include "../COMM/WiFi.h"
#include "../../CREDIT/CreditprtByBuffer.h"
#include "../../CREDIT/Creditfunc.h"
#include "../../FISC/NCCCfisc.h"
#include "../../CTLS/CTLS.h"
#include "../../EMVSRC/EMVsrc.h"
#include "../../NCCC/NCCCats.h"
#include "../../NCCC/NCCCesc.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCdcc.h"
#include "../../NCCC/NCCCtmk.h"
#include "../../NCCC/NCCCtms.h"
#include "../../NCCC/NCCCtmsCPT.h"
#include "../../NCCC/NCCCtmsSCT.h"
#include "../../NCCC/NCCCtmsFTP.h"
#include "../../NCCC/NCCCtSAM.h"
#include "../../NCCC/NCCCloyalty.h"
#include "../../NCCC/NCCCTicketSrc.h"
#include "../../DINERS/DINERSsrc.h"
#include "../../ETicket/ticket.h"
#include "../../IPASS/IPASSFunc.h"
#include "../../ECC/ICER/stdAfx.h"
#include "../../ECC/ECC.h"
#include "TDT.h"
#include "Signpad.h"
#include "IPASSDT.h"
#include "ECCDT.h"
#include "ICASHDT.h"
#include "USB.h"
#include "Utility.h"
#include "EDC_Para_Table_Func.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))

extern  int     ginIdleMSRStatus, ginMenuKeyIn, ginIdleICCStatus;
extern  int     ginEventCode;
extern  int     ginDebug;  			/* Debug使用 extern */
extern  int     ginISODebug;  			/* Debug使用 extern */
extern	int	ginDisplayDebug;		/* Debug使用 extern */
extern	int	ginEngineerDebug;
extern	int	ginFindRunTime;
extern	int	ginESCDebug;
extern	int	ginESCHostIndex;
extern	int	ginFallback;
extern	char	gszTermVersionID[16 + 1];
extern	char	gszTermVersionDate[16 + 1];
extern	int	ginMachineType;
extern	int	ginAPVersionType;
extern  int	ginContactlessSupport;
extern  int	ginWiFiSupport;
extern	int	ginHalfLCD;
extern	int	ginTouchEnable;
extern	int	ginIdleDispFlag;
extern	int			ginRuntimeCnt;
extern	RUNTIME_ALL_RECORD	gsrRuntimeRecord;		/* 用來紀錄哪邊花比較多時間用的 */
extern	unsigned long		gulDemoHappyGoPoint;
extern	unsigned long		gulDemoRedemptionPointsBalance;
extern	unsigned long		gulDemoTicketPoint;
extern	char			gszTranDBPath[100 + 1];
extern	char			gszParamDBPath[100 + 1];
extern	ECR_TABLE		gsrECROb;
extern	unsigned char		guszCTLSInitiOK;
extern	char	gszReprintDBPath[100 + 1];
extern	unsigned short		gusPrintFontStyleRegular;
extern	int	ginNCCCDCCInvoice;
extern	unsigned long	gulPCIRebootTime;
extern	unsigned long	gulPCI_IdleTime;
extern	unsigned long	gulTotalROMSize;
extern  int             ginSSLErrCode;
extern	unsigned char	guszAlreadySelectMultiAIDBit;

char		gszDuplicatePAN[_PAN_SIZE_ + 1] = {};
unsigned long	gulRunTime;
EMV_CONFIG      EMVGlobConfig;			/* EMVsrc 有call back function 無法在CTLS include */ 
unsigned char	guszNoChooseHostBit = VS_FALSE;


/* [20260128_BUG_MDF][FUN] 重新編寫太陽日計算方式 */
static VS_BOOL blLeapYear(int inYear)
{
	if ((inYear % 4 == 0 && inYear % 100 != 0) ||
		(inYear % 400 == 0))
			return VS_TRUE;

	return VS_FALSE;
}


/*
Function	:inFunc_PAD_ASCII
Date&Time	:2015/6/24 下午 4:14
Describe	:靠左靠右補字或空白
 *注意		:若靠右，且pad 0x00會抓不到字串長度，建議不要包含中文字串，中文字串會算三個字元
*/
int inFunc_PAD_ASCII(char *szStr_out, char *szStr_in, char szPad_char, int inPad_size, int inAlign)
{
        int inPADlen = 0;
        char szTemplate[256 + 1];

        /* 第一步:計算要補多少字元 */
        inPADlen = inPad_size - strlen(szStr_in);

        /* 第二步:靠左靠右 */
	if (inPADlen > 0)
	{
                /* 將szTemplate初始化成要補的字元 */
                memset(szTemplate, szPad_char, sizeof(szTemplate));

        	if (inAlign == _PADDING_LEFT_)
                        memcpy(&szTemplate[inPADlen], &szStr_in[0], inPad_size); /* 字靠右 */
                else if (inAlign == _PADDING_RIGHT_)
                        memcpy(&szTemplate[0], &szStr_in[0], strlen(szStr_in)); /* 字靠左 */

                /* 補結束字元 & 傳回szStr_out */
                szTemplate[inPad_size] = '\0';
                memset(szStr_out, 0x00, sizeof(szStr_out));
                strcpy(szStr_out, szTemplate);
	}
        else if (inPADlen == 0)
        {
                /* inPADlen = 0 的狀況，szStr_in將字串給szStr_out */
                memset(szTemplate, 0x00, sizeof(szTemplate));
                memcpy(&szTemplate[0], &szStr_in[0], strlen(szStr_in));
                memset(szStr_out, 0x00, sizeof(szStr_out));
                strcpy(szStr_out, szTemplate);
        }
        else
        {
                /* inPADlen < 0 清空szStr_out Return Error */
                memset(szStr_out, 0x00, sizeof(szStr_out));
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inPADlen < 0");
		}
		
                return(VS_ERROR);
        }

	return (VS_SUCCESS);
}

/*
Function	:inFunc_ASCII_to_BCD
Date&Time	:2015/7/13 下午 2:02
Describe        :ASCII轉BCD，inLength指要轉換成的byte數(ASCII字串的1/2長度)
		注意:BCD的空間長度至少要為ASCII的1/2倍，否則會溢位。(2byte ascii可轉換成1byte bcd)
*/
int inFunc_ASCII_to_BCD(unsigned char *uszBCD, char *szASCII, int inLength)
{
        unsigned char uszTemplate = 0;
        int	i;

        /* 防呆 inLength須大於零 */
        if (inLength <= 0)
                return (VS_ERROR);

        for (i = 0; i < inLength ; i ++)
        {
                /* 字元做運算 */
                switch (*szASCII)
                {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                                uszTemplate = *szASCII - '0';
                                break;
                        case 'A':
                        case 'B':
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                                uszTemplate = *szASCII - 'A' + 10;
                                break;
                        case 'a':
                        case 'b':
                        case 'c':
                        case 'd':
                        case 'e':
                        case 'f':
                                uszTemplate = *szASCII - 'a' + 10;
                                break;
                        default:
                                break;
                }

                *uszBCD = (unsigned char) (uszTemplate << 4); /* 向左移動四位元 */
                szASCII ++;

                switch (*szASCII)
                {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                                uszTemplate = *szASCII - '0';
                                break;
                        case 'A':
                        case 'B':
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                                uszTemplate = *szASCII - 'A' + 10;
                                break;
                        case 'a':
                        case 'b':
                        case 'c':
                        case 'd':
                        case 'e':
                        case 'f':
                                uszTemplate = *szASCII - 'a' + 10;
                                break;
                        default:
                                break;
                }

                szASCII ++;
                *uszBCD = *uszBCD | uszTemplate;
                uszBCD ++;
        }

        return (VS_SUCCESS);
}

/*
Function	:inFunc_BCD_to_ASCII
Date&Time	:2015/7/13 下午 2:02
Describe	:BCD轉ASCII 注意:ASCII的空間長度要為BCD的兩倍，否則會溢位。
*/
int inFunc_BCD_to_ASCII(char *szASCII, unsigned char *uszBCD, int inLength)
{
	unsigned char uszTemplate, uszNibble;
        int i;

	for(i = 0; i < inLength; ++i)
	{
		uszTemplate = *uszBCD++;
                uszNibble = (uszTemplate / 16);         /* 第一個字  */

                /* 16進位A = 10進位的10，做運算 */
                if (uszNibble < 10)
                {
                    *szASCII = (char)('0' + uszNibble);
                    szASCII++;
		}
                else                                    /* A~F */
                {
                    *szASCII = (char)('A' + (uszNibble - 10));
                    szASCII++;
                }

                uszNibble = (uszTemplate % 16);         /* 第二個字  */

                if (uszNibble < 10)
                {
                        *szASCII = (char)('0' + uszNibble);
                        szASCII++;
		}
                else
                {
                        *szASCII = (char)('A' + (uszNibble - 10));
                        szASCII++;
                }
	}

	/* 不要補0比較好用 */
//	*szASCII = 0;

	return (VS_SUCCESS);
}

/*
Function	:inFunc_BCD_to_INT
Date&Time	:2015/7/24 下午 17:31
Describe	:BCD轉INT
*/
int inFunc_BCD_to_INT(int *inINT, unsigned char *uszBCD, int inLength)
{
        char szASCII[10 + 1];

        memset(szASCII, 0x00, sizeof(szASCII));
        /* 使用BCD轉ASCII的Function */
        inFunc_BCD_to_ASCII(&szASCII[0], &uszBCD[0], inLength);

        *inINT = atoi(szASCII); /* 用atoi將String轉成Int */

        return (VS_SUCCESS);
}

int inBCD_ASCII_test(void)
{
	int		i;
        char		szASCII[24 + 1] = {'\0'};
	char		szTemplate[256 + 1];
        unsigned char	uszBCD[24 + 1] = {0x31,0x32,0x33,0x34,0x35,0xaB,0xCD,0xef};
        
        memset(szTemplate, 0x00, sizeof(szTemplate));

        inLogPrintf(AT, "BCD Initial Value");
        for(i = 0; i < 8; ++i)
        {
                sprintf(szTemplate, "0x%02x", uszBCD[i]);
                inLogPrintf(AT, szTemplate);
        }

        inLogPrintf(AT, "-------");
        inLogPrintf(AT, "BCD to  ASCII");
        inFunc_BCD_to_ASCII(&szASCII[0], &uszBCD[0], 10);
        memcpy(szTemplate, szASCII, 20);
        inLogPrintf(AT, szTemplate);
        inLogPrintf(AT, "-------");
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memset(uszBCD, 0x00, sizeof(uszBCD));
        inLogPrintf(AT, "ASCII to BCD");
        inFunc_ASCII_to_BCD (&uszBCD[0], &szASCII[0], 10);

        for(i = 0; i < 10; ++i)
        {
                sprintf(szTemplate, "0x%x", uszBCD[i]);
                inLogPrintf(AT, szTemplate);
        }
        inLogPrintf(AT, "-----------------");

        strcpy(szASCII, "1234567890ABCDEF");

        for(i = 0; i < 20; ++i)
        {
                sprintf(szTemplate, "0x%x", szASCII[i]);
                inLogPrintf(AT, szTemplate);
        }

        inLogPrintf(AT, "-----------------");
        inLogPrintf(AT, "BCD to INT");
        inFunc_BCD_to_INT(&i, (unsigned char*)"\x17\x39\x58", 3);
        sprintf(szTemplate, "%d", i);
        inLogPrintf(AT, szTemplate);

        return (VS_SUCCESS);
}

int inBCD_ASCII_test2(void)
{
        char	szASCII[100 + 1];
        char	szBCD[100 + 1];
        char	szTemplate[256 + 1];

        memset(szTemplate, 0x00, sizeof(szTemplate));
	
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "5430450000001517D181220115646974");
	inLogPrintf(AT, szASCII);
	
	memset(szBCD, 0x00, sizeof(szBCD));
	inFunc_ASCII_to_BCD((unsigned char*)szBCD, szASCII, 32);
	memset(szASCII, 0x00, sizeof(szASCII));
	inFunc_BCD_to_ASCII(szASCII, (unsigned char*)szBCD, 32);
	inLogPrintf(AT, szASCII);

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Amount_Comma
Date&Time       :2016/9/5 下午 4:03
Describe        :
 *szAmt:	金額
 *szCurSymbol:	金額的符號 ex: ＄、 ￥
 *szPad_char:	要Pad的字元
 *inSigned:	如果有需要印出負0的需求
 *inWide:	最後字串的寬度
 *inAlign:	右靠左靠，True的話，右靠左補空白；False的話，左靠右補空白
 *注意:		若靠右，且pad 0x00會抓不到字串長度
*/
int inFunc_Amount_Comma(char *szAmt, char *szCurSymbol, char szPad_char, int inSigned, int inWide, int inAlign)
{
	int	inOffset = 0;		
	int	inNumberLen = 0;	/* 已放進szComma的數字數量 */
        int	inLen, inPoint = 0, i;	/* inPoint:現在szComma的長度 */
        int     inFinalLen;
        char	szComma[48 + 1];
	char	szTemplate[48 + 1];
	char	szUnsignedAmt[20 + 1];
      
	
	/* 輸入數字是負數 */
        memset(szUnsignedAmt, 0x00, sizeof(szUnsignedAmt));
	if (szAmt[0] == '-')
		strcpy(szUnsignedAmt, &szAmt[1]);
	else
		strcpy(szUnsignedAmt, &szAmt[0]);
	
	inLen = strlen(szUnsignedAmt);
	inOffset = inLen % 3;
	memset(szComma, 0x00, sizeof(szComma));
	
	for (i = 0; i < inLen; i ++)
	{
		szComma[inPoint ++] = szUnsignedAmt[i];
		inNumberLen ++;			/* 數字長度 */
		
		/* 每第三個數字加comma ，若已經是最後一位也不加comma */
		if (((inNumberLen - inOffset) % 3 == 0) &&
		     (i != inLen - 1))
			szComma[inPoint ++] = 0x2C; /* 補【,】 */
	}
	
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	
	/* Flag有On或是原傳入金額為負 */
	if (inSigned == _SIGNED_MINUS_ || szAmt[0] == '-')
		sprintf(szTemplate, "-%s", szComma);
	else
		sprintf(szTemplate, "%s", szComma);

        inFinalLen = strlen(szCurSymbol) + strlen(szTemplate);
        
        if (inFinalLen > inWide)
        {    
                /* 已經超出inWide，直接丟append */
        }
        else
        {    
                /* 補空白 */
                if (inAlign == _PADDING_LEFT_)
                        inFunc_PAD_ASCII(szTemplate, szTemplate, szPad_char, inWide - strlen(szCurSymbol), _PADDING_LEFT_);	/* 右靠左補空白 */
                else
                        inFunc_PAD_ASCII(szTemplate, szTemplate, szPad_char, inWide - strlen(szCurSymbol), _PADDING_RIGHT_);	/* 左靠右補空白 */
	}
        
	/* 金額符號 */
	if (strlen(szCurSymbol) != 0)
		sprintf(szAmt, "%s%s", szCurSymbol, szTemplate);
	else
		sprintf(szAmt, "%s", szTemplate);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Amount_Comma_DCC
Date&Time       :2016/9/5 下午 2:30
Describe        :根據minor unit來決定小數點
 *szAmt:	金額
 *szCurSymbol:	金額的符號 ex: ＄、 ￥(不輸入的話放"")
 *szPad_char:	要Pad的字元
 *inWide:	最後字串的寬度
 *inAlign:	右靠左靠，True的話，右靠左補空白；False的話，左靠右補空白
 *szMinorUnit:	有幾位是小數點以下的數字
 *szCurrCode:	幣別碼(不輸入的話放"")
 *szOutput:	輸出
 *注意:		若靠右，且pad 0x00會抓不到字串長度
*/
int inFunc_Amount_Comma_DCC(char *szAmt, char* szCurSymbol, char szPad_char, int inSigned, int inWide, int inAlign, char *szMinorUnit, char *szCurrCode, char* szOutput)
{
	int	i, inMUIndex, inCnt, inFCMU;
	int	inComma	;			/* 有幾位數中間要放comma */
	int     inFinalLen = 0;
	char	szTemplate[50 + 1], szTemplate2[50 + 1];
	char	szFCA[12 + 1], szComma[42 + 1];
	char	szAmount_Minus[13 + 1] = {};	/* 確認是否有負數用 */

	memset(szTemplate, 0x00, sizeof(szTemplate));
	strcpy(szTemplate, szAmt);
	/* 把金額移到最右並把其他欄位放數字0(為了去除金額前面的空白) */
	inFunc_PAD_ASCII(szTemplate, szTemplate, '0' , 12, _PADDING_LEFT_);

	/* 去除金額前面的空白 */
	for (i = 0; i < 12; i ++)
	{
		/* 加入i == 12 - 1 是為了避免0元不會複製金額的情況 */
		if ((szTemplate[i] != 0x30 && szTemplate[i] != 0x20) || 
		     i == 12 - 1)
		{
			memset(szAmount_Minus, 0x00, sizeof(szAmount_Minus));
			memcpy(szAmount_Minus, &szTemplate[i], (12 - i)); /* Currency Amount */
			break;
		}
	}
	
	memset(szFCA, 0x00, sizeof(szFCA));
	/* 輸入數字是負數 */
	if (szAmount_Minus[0] == '-')
	{
		memcpy(szFCA, &szAmount_Minus[1], 12);
	}
	else
	{
		memcpy(szFCA, &szAmount_Minus[0], 12);
	}

	inFCMU = atoi(szMinorUnit); /* Currency Minor Unit(表示有幾位數字是在小數點後) */

	/* 沒有小數點時(這一段在加小數點) */
	if (inFCMU == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, szFCA);
		/* 算有幾位數要塞comma */
		inComma = strlen(szTemplate);
	}
	else
	{
		/* 如果有數字的位數小於小數點位數(Ex:0.03 strlen(szFCA) = 1, inFCMU = 2) */
		if (strlen(szFCA) <= inFCMU)
			inFunc_PAD_ASCII(szFCA, szFCA, '0', (inFCMU + 1), _PADDING_LEFT_);

		inMUIndex = (strlen(szFCA) - inFCMU); /* 第幾位放小數點 */
		inCnt = 0;/* inCnt:szTemplate中的index  i:szFCA中的index */
		memset(szTemplate, 0x00, sizeof(szTemplate));

		for (i = 0; i < strlen(szFCA); i ++)
		{
			szTemplate[inCnt ++] = szFCA[i];
			
			/* 要放小數點 */
			if (inCnt == inMUIndex)
				szTemplate[inCnt ++] = 0x2E; /* 補【.】 */
		}

		/* 算有幾位數要塞comma */
		for (inComma = 0; inComma < strlen(szTemplate); inComma ++)
		{
			/* 小數點以下不加comma */
			if (szTemplate[inComma] == 0x2E)
				break;
		}
		
	}

	/* 這一段在加comma */
	memset(szComma, 0x00, sizeof(szComma));

	if (inComma <= 3)
		strcpy(szComma, szTemplate);
	else if (inComma >= 4 && inComma <= 6)
	{
		memset(szComma, 0x00, sizeof(szComma));
		inCnt = 0;

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			szComma[inCnt ++] = szTemplate[i];

			if (i == (inComma - 3 - 1))
				szComma[inCnt ++] = 0x2C;	/* 補【,】 */
		}
		
	}
	else if (inComma >= 7)
	{
		memset(szComma, 0x00, sizeof(szComma));
		inCnt = 0;

		for (i = 0; i < strlen(szTemplate); i ++)
		{
			szComma[inCnt ++] = szTemplate[i];

			if (i == (inComma - 3 - 1))
				szComma[inCnt ++] = 0x2C;		/* 補【,】 */
			else if (i == (inComma - 3 - 3 - 1))
				szComma[inCnt ++] = 0x2C;		/* 補【,】 */
		}
	}
	
	/* 塞完comma和dot後，若原本有負號，這個時候塞回來 */
	/* Flag有On或是原傳入金額為負 */
	memset(szTemplate2, 0x00, sizeof(szTemplate));
	if (inSigned == _SIGNED_MINUS_ || szAmount_Minus[0] == '-')
		sprintf(szTemplate2, "-%s", szComma);
	else
		sprintf(szTemplate2, "%s", szComma);
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	/* 金額符號 */
	if (strlen(szCurSymbol) != 0)
		sprintf(szTemplate, "%s%s", szCurSymbol, szTemplate2);
	else
		sprintf(szTemplate, "%s",szTemplate2);
	
	/* 幣別碼 */
	if (strlen(szCurrCode) != 0)
		sprintf(szOutput, "%s %s", szCurrCode, szTemplate);	/* Currcncy Alphabetic Code */
	else
		sprintf(szOutput, "%s", szTemplate);			/* Currcncy Alphabetic Code */
	
	/* 補空白 */
	inFinalLen = strlen(szOutput);
        
        if (inFinalLen > inWide)
        {    
                /* 已經超出inWide，直接丟append */
        }
        else
        { 
		if (inAlign == _PADDING_LEFT_)
			inFunc_PAD_ASCII(szOutput, szOutput, szPad_char, inWide, _PADDING_LEFT_);	/* 右靠左補空白 */
		else
			inFunc_PAD_ASCII(szOutput, szOutput, szPad_char, inWide, _PADDING_RIGHT_);	/* 左靠右補空白 */
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Currency_Conversion_Fee
Date&Time       :2016/9/7 下午 4:01
Describe        :根據minor unit來決定保留到小數點下第幾位
 *szMPV_In:	轉換費率值(Markup Percentage Value)
 *szMinorUnit:	有幾位是小數點以下的數字
 *szOutput:	輸出
*/
int inFunc_Currency_Conversion_Fee(char *szMPV_In, char *szMinorUnit, char* szOutput)
{	
	int	i;
	int	inMPV;				/* 要保留到小數點下第幾位 */
	char	szMPV[10 + 1];			/* Markup Percentage Value */

	/* 轉換費率 */
	memset(szMPV, 0x00, sizeof(szMPV));
	strcpy(szMPV, szMPV_In);		/* Markup Percentage Value */
	
	/* 要保留到小數點下第幾位 */
	inMPV = atoi(szMinorUnit);		/* Markup Percentage Decimal Point */

	/* 小數點後不保留的情況 */
	if (inMPV == 0)
	{
		for (i = 0; i < strlen(szMPV); i ++)
		{
			if (szMPV[i] == 0x2E)
			{
				szMPV[i] = 0x00;
				break;
			}
		}

		memset(szOutput, 0x00, sizeof(szOutput));
		strcpy(szOutput, szMPV);
	}
	else
	{
		memset(szOutput, 0x00, sizeof(szOutput));

		for (i = 0; i < strlen(szMPV); i ++)
		{
			if (szMPV[i] == 0x2E)
			{
				szMPV[i + 1 + inMPV] = 0x00;	/* 幾位小數點(中間的1是小數點) */
				break;
			}
		}

		strcpy(szOutput, szMPV);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields
Date&Time       :2017/6/13 下午 6:09
Describe        :只接收刷卡和人工輸入的分流
*/
int inFunc_GetCardFields_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szFunEnable[2 + 1];
	
	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);
	
	if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
	{
		inRetVal = inFunc_GetCardFields_Txno(pobTran);
	}
	else
	{
		inRetVal = inFunc_GetCardFields(pobTran);
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_GetCardFields
Date&Time       :2017/1/20 下午 4:29
Describe        :
*/
int inFunc_GetCardFields(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
        char	szKey = 0;
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szTemplate[3 + 1] = {0};
	long	lnTimeout = 0;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE && pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
                
	}
	else
	{
                /* 如果有卡號就直接跳走 */
                if (strlen(pobTran->srBRec.szPAN) > 0)
                {
                        return (VS_SUCCESS);
                }
	}

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;
	
        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
        
        /* Mirror Message */
        if (pobTran->uszECRBit == VS_TRUE)
	{
                inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_REDEEM_REFUND_);
        }
	
	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);
	
        while (1)
        {
                /* idle 刷卡 或 一般刷卡 */
	        if (ginIdleMSRStatus == VS_TRUE  || ginEventCode == _SWIPE_EVENT_)
	        {
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
                            
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

                        /* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

  	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
			{
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
	                        return (VS_ERROR);
			}
                        
			/* 第七步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
	
			break;
	        }
                /* idle手動輸入或一般輸入 */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}
			
			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&& 
			   (pobTran->inTransactionCode != _REFUND_		&& 
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&& 
			    pobTran->inTransactionCode != _CUP_PRE_COMP_	&&
			    pobTran->inTransactionCode != _HG_REWARD_REFUND_	&&
			    pobTran->inTransactionCode != _HG_REDEEM_REFUND_))
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

				return (VS_ERROR);
			}
			
			/* 客製化038 bellavita 退貨 預先授權 預先授權完成 不能人工輸入 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->inTransactionCode == _REFUND_		|| 
				    pobTran->inTransactionCode == _INST_REFUND_		||
				    pobTran->inTransactionCode == _REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _PRE_AUTH_		||
				    pobTran->inTransactionCode == _PRE_COMP_		||
				    pobTran->inTransactionCode == _CUP_REFUND_		||
				    pobTran->inTransactionCode == _CUP_INST_REFUND_	||
				    pobTran->inTransactionCode == _CUP_REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _CUP_PRE_AUTH_	||
				    pobTran->inTransactionCode == _CUP_PRE_COMP_	||
				    pobTran->inTransactionCode == _HG_REWARD_REFUND_	||
				    pobTran->inTransactionCode == _HG_REDEEM_REFUND_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

					return (VS_ERROR);
				}
			}
			
			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);
			
			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

                        /* 第三步選擇輸入檢查碼或有效期 */
                        inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

                        /* 第四步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			break;
                }
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示請刷卡 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inDISP_PutGraphic(_GET_CARD_CUP_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		else
		{
			inDISP_PutGraphic(_GET_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		
		/* 客製化123，過卡時設為較最亮 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
		}
			
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
	
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;
			
				inDISP_Msg_BMP(&srDispMsgObj);
                                
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
				
				/* 顯示請刷卡 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inDISP_PutGraphic(_GET_CARD_CUP_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
					inDISP_PutGraphic(_GET_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
				}
			}
			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout or Cancel */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* Timeout or Cancel */
					return (VS_USER_CANCEL);
				}
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				/* 客製化005，只有退貨支援Menu keyin */
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 銀聯一般交易不能輸入卡號 */
					if (pobTran->inTransactionCode == _CUP_SALE_)
					{
						continue;
					}
					else
					{
						ginEventCode = _MENUKEYIN_EVENT_;
					}
				}
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_Txno
Date&Time       :2017/6/13 下午 6:14
Describe        :
*/
int inFunc_GetCardFields_Txno(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
        char	szKey = 0;
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szTemplate[3 + 1] = {0};
	long	lnTimeout = 0;

        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_Txno START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields_Txno() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE && pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		
	}
	else
	{
                /* 如果有卡號就直接跳走 */
                if (strlen(pobTran->srBRec.szPAN) > 0)
                {
                        return (VS_SUCCESS);
                }
	}
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;
	
        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
        
        /* Mirror Message */
        if (pobTran->uszECRBit == VS_TRUE)
	{
                inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_REDEEM_REFUND_);
        }
	
	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);
	
        while (1)
        {
                /* idle 刷卡 或 一般刷卡 */
	        if (ginIdleMSRStatus == VS_TRUE  || ginEventCode == _SWIPE_EVENT_)
	        {
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
                                
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

                        /* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

  	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
			{
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
	                        return (VS_ERROR);
			}
                        
			/* 第七步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
	
                        break;
	        }
                /* idle手動輸入或一般輸入 */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&& 
			   (pobTran->inTransactionCode != _REFUND_		&& 
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&& 
			    pobTran->inTransactionCode != _CUP_PRE_COMP_	&&
			    pobTran->inTransactionCode != _HG_REWARD_REFUND_	&&
			    pobTran->inTransactionCode != _HG_REDEEM_REFUND_))
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

				return (VS_ERROR);
			}
			
			/* 客製化038 bellavita 退貨 預先授權 預先授權完成 不能人工輸入 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->inTransactionCode == _REFUND_		|| 
				    pobTran->inTransactionCode == _INST_REFUND_		||
				    pobTran->inTransactionCode == _REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _PRE_AUTH_		||
				    pobTran->inTransactionCode == _PRE_COMP_		||
				    pobTran->inTransactionCode == _CUP_REFUND_		||
				    pobTran->inTransactionCode == _CUP_INST_REFUND_	||
				    pobTran->inTransactionCode == _CUP_REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _CUP_PRE_AUTH_	||
				    pobTran->inTransactionCode == _CUP_PRE_COMP_	||
				    pobTran->inTransactionCode == _HG_REWARD_REFUND_	||
				    pobTran->inTransactionCode == _HG_REDEEM_REFUND_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

					return (VS_ERROR);
				}
			}
			
			inRetVal = inCREDIT_Func_Get_Card_Number_Txno_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);
			
			

			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

                        /* 第三步選擇輸入檢查碼或有效期 */
                        /* 部份交易不能允許輸入檢查碼 */
			if (pobTran->srBRec.inCode == _SALE_		||
			    pobTran->srBRec.inCode == _CUP_SALE_	||
			    pobTran->srBRec.inCode == _INST_SALE_	||
			    pobTran->srBRec.inCode == _REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _PRE_AUTH_	||
			    pobTran->srBRec.inCode == _CUP_INST_SALE_	||
			    pobTran->srBRec.inCode == _CUP_REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
			{
				inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			}
			else
			{
				inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			}
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

                        /* 第四步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        break;
                }
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示請刷銀聯卡 人工輸入按0 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inDISP_PutGraphic(_GET_CARD_CUP_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		/* 顯示請刷卡 人工輸入按0 */
		else
		{
			inDISP_PutGraphic(_GET_CARD_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		
		/* 客製化123，過卡時設為較最亮 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
		}
		
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
	
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;
			
				inDISP_Msg_BMP(&srDispMsgObj);
                                
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
				
				/* 顯示請刷銀聯卡 人工輸入按0 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inDISP_PutGraphic(_GET_CARD_CUP_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				/* 顯示請刷卡 人工輸入按0 */
				else
				{
					inDISP_PutGraphic(_GET_CARD_TXNO_, 0, _COORDINATE_Y_LINE_8_4_);
				}
			}
			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				/* 客製化005，只有退貨支援Menu keyin */
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 銀聯一般交易不能輸入卡號 */
					if (pobTran->inTransactionCode == _CUP_SALE_)
					{
						continue;
					}
					else
					{
						/* key in事件 人工輸入請按0*/
						if (szKey == '0')
						{
							ginEventCode = _MENUKEYIN_EVENT_;
						}
					}
				}
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_ICC
Date&Time       :2018/5/29 下午 3:50
Describe        :
*/
int inFunc_GetCardFields_ICC(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int	inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char	szKey = 0;
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szTemplate[3 + 1] = {0};
	long	lnTimeout = 0;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC START!");
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields_ICC() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE && pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		
	}
	else
	{
                /* 如果有卡號就直接跳走 */
                if (strlen(pobTran->srBRec.szPAN) > 0)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC PAN > 0 END!");
                        return (VS_SUCCESS);
                }
	}

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
        
        /* Mirror Message */
        if (pobTran->uszECRBit == VS_TRUE)
	{
                inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_REDEEM_);
        }
	
	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);
	
        while (1)
        {

                /* idle 刷卡 或 一般刷卡 */
	        if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
	        {
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
                                
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_GetBin failed");
                                return(VS_ERROR);
                        }

			/* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
                                {
                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_ValidTrack2_PAN failed");
					return (VS_ERROR);
                                }
			}
			
	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCheckFallbackFlag failed END!");
                                
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
	                        return (VS_ERROR);
			}
                        
	                /* 第七步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_ValidTrack2_ExpDate failed END!");
                                
				return (VS_ERROR);
                        }
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inNCCC_Func_Decide_MEG_TRT failed END!");
                                
				return (inRetVal);
			}

                        break;
	        }
                /* idle手動輸入或一般輸入 */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}
			
			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&& 
			   (pobTran->inTransactionCode != _REFUND_		&& 
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&& 
			    pobTran->inTransactionCode != _CUP_PRE_COMP_	&&
			    pobTran->inTransactionCode != _HG_REWARD_REFUND_	&&
			    pobTran->inTransactionCode != _HG_REDEEM_REFUND_))
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC ManualKeyin failed END!");
                                
				pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
				return (VS_ERROR);
			}
			
			/* 客製化038 bellavita 退貨 預先授權 預先授權完成 不能人工輸入 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->inTransactionCode == _REFUND_		|| 
				    pobTran->inTransactionCode == _INST_REFUND_		||
				    pobTran->inTransactionCode == _REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _PRE_AUTH_		||
				    pobTran->inTransactionCode == _PRE_COMP_		||
				    pobTran->inTransactionCode == _CUP_REFUND_		||
				    pobTran->inTransactionCode == _CUP_INST_REFUND_	||
				    pobTran->inTransactionCode == _CUP_REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _CUP_PRE_AUTH_	||
				    pobTran->inTransactionCode == _CUP_PRE_COMP_	||
				    pobTran->inTransactionCode == _HG_REWARD_REFUND_	||
				    pobTran->inTransactionCode == _HG_REDEEM_REFUND_)
				{
                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC cus_038_bellavita failed END!");
					
                                        pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
					return (VS_ERROR);
				}
			}
			
			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCREDIT_Func_Get_Card_Number failed END!");
                                
				return (inRetVal);
			}
			
			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_Generate_Special_Card failed END!");
                                
				return (VS_ERROR);
			}
			
                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_GetBin failed END!");
                                
                                return(VS_ERROR);
                        }
			
                        /* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
                                {
                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_ValidTrack2_PAN failed END!");
					return (VS_ERROR);
                                }
			}

                        /* 第三步選擇輸入檢查碼或有效期 */
                        /* 部份交易不能允許輸入檢查碼 */
			if (pobTran->srBRec.inCode == _SALE_		||
			    pobTran->srBRec.inCode == _CUP_SALE_	||
			    pobTran->srBRec.inCode == _INST_SALE_	||
			    pobTran->srBRec.inCode == _REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _PRE_AUTH_	||
			    pobTran->srBRec.inCode == _CUP_INST_SALE_	||
			    pobTran->srBRec.inCode == _CUP_REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
			{
				inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			}
			else
			{
				inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			}
			if (inRetVal != VS_SUCCESS)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCREDIT_Func_Get_Exp_Date_Or_inCREDIT_Func_Get_CheckNO_ExpDate_Flow failed END!");
				return (inRetVal);
                        }

                        /* 第四步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_ValidTrack2_ExpDate failed END!");
				return (VS_ERROR);
                        }
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inNCCC_Func_Decide_MEG_TRT failed END!");
				return (inRetVal);
			}
			
                        break;
                }
                /* idle 插晶片卡 */
                else if  (ginIdleICCStatus == VS_TRUE || ginEventCode == _EMV_DO_EVENT_)
                {
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
                        {
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inEMV_GetEMVCardData failed END!");
                                return (VS_ERROR);
                        }
                        else
                        {
				if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				{
					if (inEMV_GetCardNoFlow(pobTran) != VS_SUCCESS)
                                        {
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inEMV_GetCardNoFlow failed END!");
						return (VS_ERROR);
                                        }

					/* 第三步驟 判斷card bin 讀HDT */
					if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                        {
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_GetBin failed END!");
						return (VS_ERROR);
                                        }

					/* 第四步驟檢核PAN module 10 */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetModule10Check(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
						if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
                                                {
                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_ValidTrack2_PAN failed END!");
							return (VS_ERROR);
                                                }
					}

					/* 第五步驟檢核ExpDate */
					if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
                                        {
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inCARD_ValidTrack2_ExpDate_failed END!");
						return (VS_ERROR);
                                        }
				}
				else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					
					if (inFISC_Read_Card_Data_Flow(pobTran) != VS_SUCCESS)
                                        {
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inFISC_Read_Card_Data_Flow_failed END!");
						return (VS_ERROR);
                                        }
					
					inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				}
				/* TRT在inEMV_GetEMVCardData裡面selectAID時設定 */
				/* 這邊是HardCode，如果之後有不同的收單行，可能要用AID來分 (EX:AE晶片卡)*/
                        }

			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inNCCC_Func_Decide_ICC_TRT failed END!");
				return (inRetVal);
			}
			
                        break;
                }

		
		/* 顯示請刷銀聯卡或插卡 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inDISP_PutGraphic(_GET_CARD_CUP_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		/* 顯示請刷卡或插卡 */
		else
		{
			inDISP_PutGraphic(_GET_CARD_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
		}
		
		/* 客製化123，過卡時設為較最亮 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
		}
		
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;
			
				inDISP_Msg_BMP(&srDispMsgObj);
                                
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
		
				/* 顯示請刷銀聯卡或插卡 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inDISP_PutGraphic(_GET_CARD_CUP_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				/* 顯示請刷卡或插卡 */
				else
				{
					inDISP_PutGraphic(_GET_CARD_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
				}
			}

			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC timeout END!");
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC user_cancel END!");
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				/* 客製化005，只有退貨支援Menu keyin */
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					if (pobTran->inTransactionCode == _CUP_SALE_)
					{
						continue;
					}
					else
					{
						/* key in事件 */
						ginEventCode = _MENUKEYIN_EVENT_;
					}
				}
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
				
		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC END!");
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_CTLS
Date&Time       :2016/11/2 上午 11:40
Describe        :進這隻必定CTLS Enable為開
*/
int inFunc_GetCardFields_CTLS(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int		inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	unsigned short	usLen = 0;  /* [115150] */
	char		szKey = -1;
	char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szBatchNum[6 + 1] = {0};
	char		szSTANNum[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	char		szCTLSEnable[2 + 1] = {0};
	char		szCUPContactlessEnable[1 + 1] = {0};
	char		szECR_RS232_Version[2 + 1] = {0};
	char		szECR_UDP_Version[2 + 1] = {0};
	long		lnTimeout = 0;
	unsigned long   ulCTLS_RetVal = -1;

	vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS START!");
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields_CTLS() START !");
	}

	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	memset(szECR_RS232_Version, 0x00, sizeof(szECR_RS232_Version));
	inGetECRVersion(szECR_RS232_Version);
	memset(szECR_UDP_Version, 0x00, sizeof(szECR_UDP_Version));
	inGetUDP_ECRVersion(szECR_UDP_Version);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);

		memset(szSTANNum, 0x00, sizeof(szSTANNum));
		inGetSTANNum(szSTANNum);
		pobTran->srBRec.lnSTANNum = atol(szSTANNum);

		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetInvoiceNum(szInvoiceNum);
		pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
		
		vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS UNY END!");
                
		return (VS_SUCCESS);
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		/* 取卡號有效期 */
		if (pobTran->in57_Track2Len > 0)
		{
			inRetVal = inCARD_unPackCard(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_unPackCard Failed");
				return (VS_NO_CARD_BIN);
			}
		}
		else
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS in57_Track2Len <= 0");
			return (VS_NO_CARD_BIN);
		}
			
		/* 判斷card bin 讀HDT */
		if (inCARD_GetBin(pobTran) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_GetBin failed");
			return (VS_NO_CARD_BIN);
		}

                /* 檢核PAN module 10 */
		memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
		inGetModule10Check(szFuncEnable);
		if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
		{
			/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
			if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_ValidTrack2_PAN failed");
				return (VS_CARD_PAN_ERROR);
			}
		}

                /* 檢核ExpDate */
		if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_ValidTrack2_ExpDate failed");
			return (VS_CARD_EXP_ERR);
		}

		vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS CUPEMVQR END!");
		return (VS_SUCCESS);
	}
	else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE && pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		
	}
	else
	{
		/* 如果有卡號就直接跳走 */
		if (strlen(pobTran->srBRec.szPAN) > 0)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS PAN > 0 END!");
			return (VS_SUCCESS);
		}
	}
	
	/* 過卡方式參數初始化  */
	pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
	ginEventCode = -1;
	
	/* Send CTLS Readly for Sale Command */
	if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_ICC(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inFunc_GetCardFields_ICC failed");
		}
		
		return (inRetVal);
	}
	
	/* 顯示對應交易別的感應畫面 */
	inCTLS_Decide_Display_Image(pobTran);

	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
	EMVCL_StartIdleLEDBehavior(NULL);

        /* 顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
		inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
	}
	
	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}

        /* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
        
	/* Mirror Message */
	if (pobTran->uszECRBit == VS_TRUE)
	{
		inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_);
	}

	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);
	/* 立既顯示倒數秒數 */
	inDISP_LiveCountdown(lnTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	
	while (1)
        {
                /* （idle畫面刷卡此function不會發生） or 刷卡事件發生 */
	        if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
	        {
                        /* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
                        inFunc_ResetTitle(pobTran);
                        
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_unPackCard failed");
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
                                
				/* Unpack失敗，再開感應太麻煩 */
	                        return(VS_ERROR);
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_GetBin failed");
                                return(VS_ERROR);
			}

                        /* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_ValidTrack2_PAN failed");
					return (VS_ERROR);
				}
			}
			
	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCheckFallbackFlag failed");
				
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
	                        return (VS_ERROR);
			}

	                /* 第七步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_ValidTrack2_ExpDate failed");
				return (VS_ERROR);
			}
                        
                        /* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inNCCC_Func_Decide_MEG_TRT failed");
				return (inRetVal);
			}
			
                        break;
	        }
                /* （idle畫面人工輸入此function不會發生）or Menu Keyin */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 取消感應交易 */
			inCTLS_CancelTransacton_Flow();
			
			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}
			
			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&& 
			   (pobTran->inTransactionCode != _REFUND_		&& 
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&& 
			    pobTran->inTransactionCode != _CUP_PRE_COMP_	&&
			    pobTran->inTransactionCode != _HG_REWARD_REFUND_	&&
			    pobTran->inTransactionCode != _HG_REDEEM_REFUND_))
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inGetManualKeyin func closed (%s)", szFuncEnable);
				return (VS_ERROR);
			}
			
			/* 客製化038 bellavita 退貨 預先授權 預先授權完成 不能人工輸入 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->inTransactionCode == _REFUND_		|| 
				    pobTran->inTransactionCode == _INST_REFUND_		||
				    pobTran->inTransactionCode == _REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _PRE_AUTH_		||
				    pobTran->inTransactionCode == _PRE_COMP_		||
				    pobTran->inTransactionCode == _CUP_REFUND_		||
				    pobTran->inTransactionCode == _CUP_INST_REFUND_	||
				    pobTran->inTransactionCode == _CUP_REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _CUP_PRE_AUTH_	||
				    pobTran->inTransactionCode == _CUP_PRE_COMP_	||
				    pobTran->inTransactionCode == _HG_REWARD_REFUND_	||
				    pobTran->inTransactionCode == _HG_REDEEM_REFUND_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

					vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS cus038 (%s)", szCustomerIndicator);
					return (VS_ERROR);
				}
			}
			
			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCREDIT_Func_Get_Card_Number failed");
				
				return (inRetVal);
			}
			
			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_Generate_Special_Card failed");
				
				return (VS_ERROR);
			}
			
                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_GetBin failed");
				
                                return(VS_ERROR);
			}
			
                        /* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_ValidTrack2_PAN failed");
					
					return (VS_ERROR);
				}
			}

                        /* 第三步選擇輸入檢查碼或有效期 */
			/* 部份交易不能允許輸入檢查碼 */
			if (pobTran->srBRec.inCode == _SALE_		||
			    pobTran->srBRec.inCode == _CUP_SALE_	||
			    pobTran->srBRec.inCode == _INST_SALE_	||
			    pobTran->srBRec.inCode == _REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _PRE_AUTH_	||
			    pobTran->srBRec.inCode == _CUP_INST_SALE_	||
			    pobTran->srBRec.inCode == _CUP_REDEEM_SALE_	||
			    pobTran->srBRec.inCode == _CUP_PRE_AUTH_)
			{
				inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			}
			else
			{
				inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			}
			if (inRetVal != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCREDIT_Func_Get_Exp_Date or inCREDIT_Func_Get_CheckNO_ExpDate_Flow failed");
				return (inRetVal);
			}

                        /* 第四步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_ValidTrack2_ExpDate or inCREDIT_Func_Get_CheckNO_ExpDate_Flow failed");
				return (VS_ERROR);
			}

                        /* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inNCCC_Func_Decide_MEG_TRT failed");
				return (inRetVal);
			}

                        break;
                }
                /* （idle 插晶片卡此function不會發生）or 晶片卡事件 */
                else if  (ginIdleICCStatus == VS_TRUE || ginEventCode == _EMV_DO_EVENT_)
                {
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
                        inFunc_ResetTitle(pobTran);
                        
                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inEMV_GetEMVCardData failed");
                                return (VS_ERROR);
			}
                        else
                        {
				if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
				{
					if (inEMV_GetCardNoFlow(pobTran) != VS_SUCCESS)
					{
						vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inEMV_GetCardNoFlow failed");
						return (VS_ERROR);
					}

					/* 第三步驟 判斷card bin 讀HDT */
					if (inCARD_GetBin(pobTran) != VS_SUCCESS)
						return (VS_ERROR);

					/* 第四步驟檢核PAN module 10 */
					memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
					inGetModule10Check(szFuncEnable);
					if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
					{
						/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
						if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						{
							vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inEMV_GetCardNoFlow failed");
							return (VS_ERROR);
						}
					}

					/* 第五步驟檢核ExpDate */
					if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					{
						vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_ValidTrack2_ExpDate failed");
						return (VS_ERROR);
					}
				}
				else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					
					if (inFISC_Read_Card_Data_Flow(pobTran) != VS_SUCCESS)
                                        {
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inFISC_Read_Card_Data_Flow_failed END!");
						return (VS_ERROR);
                                        }
					
					inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				}
				/* Host、TRT在inEMV_GetEMVCardData裡面selectAID時設定 */
				/* 如果之後有不同的收單行，可能要用AID來分 (EX:AE晶片卡)*/
                        }
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inNCCC_Func_Decide_ICC_TRT failed");
				return (inRetVal);
			}

                        break;
                } 
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_SALE(pobTran);
			
			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS d_EMVCL_RC_NO_CARD");
				/* Timeout沒卡 */
				return (VS_TIMEOUT);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS d_EMVCL_RC_MORE_CARDS");
				
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_MULTI_CARD_;
				
				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS d_EMVCL_NON_EMV_CARD");
					return (VS_WAVE_ERROR);
				}
                               
				/* 轉 FISC_SALE */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* 判斷card bin 讀HDT */
				if (inCARD_GetBin(pobTran) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_GetBin failed");
					return (VS_NO_CARD_BIN);
				}

				/* 檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					{
						vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_ValidTrack2_PAN failed");
						return (VS_CARD_PAN_ERROR);
					}
				}

				/* 檢核ExpDate */
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
                                {
                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCARD_ValidTrack2_ExpDate failed");
					return (VS_CARD_EXP_ERR);
                                }
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Sale Command */
				if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
                                {
                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inCTLS_SendReadyForSale_Flow failed");
					return (VS_WAVE_ERROR);
                                }
				
				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS 感應失敗,0x%08lX", ulCTLS_RetVal);
                                
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "感應失敗,0x%08lX", ulCTLS_RetVal);
					inLogPrintf(AT, szDebugMsg);
				}
				
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}
				
				return (VS_WAVE_ERROR);
			}
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS inNCCC_Func_Decide_CTLS_TRT failed");
				return (inRetVal);
			}
			
			inFunc_ResetTitle(pobTran);
			
                        break;
		}
		/* 走票證流程 */
		else if (ginEventCode == _TICKET_EVENT_)
		{
			if (inMENU_Check_ETICKET_Enable(0) == VS_SUCCESS)
			{
				/* 取消感應交易 */
				inCTLS_CancelTransacton_Flow();
			
				pobTran->inTransactionCode = _TICKET_DEDUCT_;
				pobTran->srTRec.inCode = pobTran->inTransactionCode;
				pobTran->srTRec.uszESVCTransBit = VS_TRUE;
				inFunc_ResetTitle(pobTran);
				inRetVal = inFLOW_RunOperation(pobTran, _OPERATION_TICKET_DEDUCT_);

				break;
			}
			else
			{
				/* 沒開票證，不做反應 */
			}
		}
		/* 走ECR流程 [115150] */
        else if(ginEventCode == _ECR_EVENT_)
		{
			if( (inRetVal = inECR_Receive_Transaction(pobTran)) != VS_SUCCESS)
			{
				inLogPrintf(AT,"inECR_Receive_Transaction Error,inRetVal is %d",inRetVal);
				vdUtility_SYSFIN_LogMessage(AT, "inECR_Receive_Transaction Error,inRetVal is %d",inRetVal);
			}
			else
			{	
				/* 收到的第二段ECR只處理交易別:90(終止EDC交易) */
				if(pobTran->inTransactionCode == _VOID_TRANS_)
				{
					/* 規格Trans Type:90 不送CR */
					/* 這邊寫法參考inNCCC_DCC_CHECK */
					pobTran->szAgreeEsgBill[0] = 'N';
					pobTran->uszEsgOptrionBit = VS_TRUE;
					pobTran->szEsgSendCR[0] = 'N';
					/* 傳Response 回pos機 */
					if( (inRetVal = inECR_Send_Transaction(pobTran)) != VS_SUCCESS )
					{	
						/* 這邊回寫給pos機失敗 好像沒設定pobTran->inECRErrorMsg? */
						inLogPrintf(AT,"inECR_Receive_Transaction Error,inRetVal is %d",inRetVal);
						vdUtility_SYSFIN_LogMessage(AT, "inECR_Receive_Transaction Error,inRetVal is %d",inRetVal);
					}
					else
					{
						inFunc_ResetTitle(pobTran);
						DISPLAY_OBJECT	srDispMsgObj;
						memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
						snprintf(srDispMsgObj.szErrMsg1,sizeof(srDispMsgObj.szErrMsg1),"使用者終止交易");
						inDISP_ErrorMsg(&srDispMsgObj);
						return (VS_ERROR);
					}
				}
			}
		}
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			if (guszAlreadySelectMultiAIDBit != VS_TRUE)
			{
				inMSR_RetVal = inCARD_MSREvent();
				if (inMSR_RetVal == VS_SUCCESS)
				{
					/* 刷卡事件 */
					ginEventCode = _SWIPE_EVENT_;
				}
				/* 回復錯誤訊息蓋掉的圖 */
				else if (inMSR_RetVal == VS_SWIPE_ERROR)
				{
					inFunc_ResetTitle(pobTran);
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;

					inDISP_Msg_BMP(&srDispMsgObj);

					/* Mirror Message */
					if (pobTran->uszECRBit == VS_TRUE)
					{
						inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
					}

					/* 顯示對應交易別的感應畫面 */
					inCTLS_Decide_Display_Image(pobTran);

					/* 回復虛擬燈號 */
					EMVCL_ShowVirtualLED(NULL);

					/* 顯示金額 */
					if (pobTran->srBRec.lnTxnAmount > 0)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
						inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
						inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
					}

				}
			}
			
			/* ------------偵測晶片卡---------------- */
			if (guszAlreadySelectMultiAIDBit != VS_TRUE)
			{
				inEMV_RetVal = inEMV_ICCEvent();
				if (inEMV_RetVal == VS_SUCCESS)
				{
					/* 晶片卡事件 */
					ginEventCode = _EMV_DO_EVENT_;
				}
			}
			
			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForSales_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}
			/* ------------偵測觸發ECR交易------------------ [115150]*/
			if (pobTran->uszECRBit == VS_TRUE)
			{
				if (inECR_Receive_Check(&usLen) == VS_SUCCESS)
				{
					ginEventCode =  _ECR_EVENT_;
				}
			}
			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();
			
			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}
			
			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS Timeout");
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
					inCTLS_CancelTransacton_Flow();
					vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS cus107 (%s)", szCustomerIndicator);
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				/* 客製化005，只有退貨支援Menu keyin */
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else if (guszAlreadySelectMultiAIDBit == VS_TRUE)
				{
					continue;
				}
				else
				{
					if (pobTran->inTransactionCode == _CUP_SALE_)
					{
						continue;
					}
					else
					{
						/* key in事件 */
						ginEventCode = _MENUKEYIN_EVENT_;
					}
				}
			}
			else if (szKey == _KEY_DOT_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)            ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)    ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)         ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
                                else if (pobTran->uszTK3C_NoHotkeyBit == VS_TRUE)
                                {
                                        /* 阻擋來自TK3C ECR發動的Sale交易發動時，電票熱鍵事件 */
                                        continue;
                                }
				else if (guszAlreadySelectMultiAIDBit == VS_TRUE)
				{
					continue;
				}
				else
				{
					/* 分期、紅利、銀聯、HappyGo混合 不能轉票證 */
					/* ECR且不送Hotkey，不能進票證 */
					if (pobTran->srBRec.uszInstallmentBit == VS_TRUE	||
					    pobTran->srBRec.uszRedeemBit == VS_TRUE		||
					    pobTran->srBRec.uszCUPTransBit == VS_TRUE		||
					    pobTran->srBRec.uszHappyGoMulti == VS_TRUE		||
					    pobTran->srBRec.inCode == _PRE_AUTH_		||
					    (pobTran->uszECRBit == VS_TRUE && memcmp(&gsrECROb.srTransData.szField_05[0], "0", 1) != 0))
					{

					}
					else
					{
						/* 走票證流程 */
						ginEventCode = _TICKET_EVENT_;
					}
				}
			}
			else if (szKey == _KEY_FUNCTION_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)            ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)    ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)         ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_,_CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else if (guszAlreadySelectMultiAIDBit == VS_TRUE)
				{
					continue;
				}
				else
				{
					if (pobTran->srBRec.inCode == _SALE_)
					{
						/* 要有開銀聯感應功能才能開 */
						if (!memcmp(szCTLSEnable, "Y", 1)		&&
						    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
						    guszCTLSInitiOK == VS_TRUE)
						{
							/* 轉成銀聯一般交易的畫面 */
							pobTran->inTransactionCode = _CUP_SALE_;
							pobTran->srBRec.inCode = pobTran->inTransactionCode;
							pobTran->inRunTRTID = _TRT_CUP_SALE_;
							/* 標示是CUP交易 */
							pobTran->srBRec.uszCUPTransBit = VS_TRUE;

							/* 顯示對應交易別的感應畫面 */
							inCTLS_Decide_Display_Image(pobTran);

							/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
							EMVCL_StartIdleLEDBehavior(NULL);

							/* 顯示金額 */
							if (pobTran->srBRec.lnTxnAmount > 0)
							{
								memset(szTemplate, 0x00, sizeof(szTemplate));
								sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
								inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
								inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
							}
						}
					}
				}
			}
			
			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/
		
	} /* while (1) 對事件做回應迴圈...*/

	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_ResetAll();
	}
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_CTLS END!");
        
        return (inRetVal);
}

/*
Function        :inFunc_GetCardFields_Refund_CTLS_Flow
Date&Time       :2017/6/27 下午 4:40
Describe        :感應退貨的分流
*/
int inFunc_GetCardFields_Refund_CTLS_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szFunEnable[2 + 1] = {0};
	
	memset(szFunEnable, 0x00, sizeof(szFunEnable));
	inGetStore_Stub_CardNo_Truncate_Enable(szFunEnable);
	
	if (memcmp(szFunEnable, "Y", strlen("Y")) == 0)
	{
		inRetVal = inFunc_GetCardFields_Refund_CTLS_Txno(pobTran);
	}
	else
	{
		inRetVal = inFunc_GetCardFields_Refund_CTLS(pobTran);
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_GetCardFields_Refund_CTLS
Date&Time       :2017/6/27 下午 4:26
Describe        :進這隻必定CTLS Enable為開，感應退貨
*/
int inFunc_GetCardFields_Refund_CTLS(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char		szKey = -1;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szBatchNum[6 + 1] = {0};
	char		szSTANNum[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	char		szCTLSEnable[2 + 1] = {0};
	char		szCUPContactlessEnable[1 + 1] = {0};
	long		lnTimeout = 0;
        unsigned long   ulCTLS_RetVal = 0x00;

        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_Refund_CTLS START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields_Refund_CTLS() START !");
	}
	
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);

		memset(szSTANNum, 0x00, sizeof(szSTANNum));
		inGetSTANNum(szSTANNum);
		pobTran->srBRec.lnSTANNum = atol(szSTANNum);

		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetInvoiceNum(szInvoiceNum);
		pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
			
		return (VS_SUCCESS);
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
        {
		/* 取卡號有效期 */
		if (pobTran->in57_Track2Len > 0)
		{
			inRetVal = inCARD_unPackCard(pobTran);
			if (inRetVal != VS_SUCCESS)
			    return (VS_NO_CARD_BIN);
		}
		else
		{
			return (VS_NO_CARD_BIN);
		}
			
                /* 判斷card bin 讀HDT */
                if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                       return (VS_NO_CARD_BIN);

                /* 檢核PAN module 10 */
                memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
                inGetModule10Check(szFuncEnable);
                if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
                {
                        /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                        if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
                                return (VS_CARD_PAN_ERROR);
                }

                /* 檢核ExpDate */
                if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
                        return (VS_CARD_EXP_ERR);
			
		return (VS_SUCCESS);
        }
	else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE && pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		
	}
	else
	{
		/* 有可能第一段是正向，第二段是退貨 */
		if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
		{
			/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
			/* 端末設備支援以感應方式進行退貨交易，並讀取卡號及有效期，以人工輸入卡號方式之電文上傳至ATS主機取的授權。 */
			/* 將contactless flag OFF掉並把manual keyin flag ON */
			pobTran->srBRec.uszContactlessBit = VS_FALSE;
			pobTran->srBRec.uszManualBit = VS_TRUE;
			pobTran->srBRec.uszRefundCTLSBit = VS_TRUE;
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE;
		}
		
		/* 如果有卡號就直接跳走 */
		if (strlen(pobTran->srBRec.szPAN) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
        ginEventCode = -1;
	
	/* Send CTLS Readly for Refund Command */
	if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields(pobTran);
		return (inRetVal);
	}
	
	/* 顯示對應交易別的感應畫面 */
	inCTLS_Decide_Display_Image(pobTran);
	
	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        EMVCL_StartIdleLEDBehavior(NULL);
        
        /* 顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
		inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
	}
	
	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
        
        /* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
        
        /* Mirror Message */
        if (pobTran->uszECRBit == VS_TRUE)
	{
                inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_REFUND_);
        }
	
	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);
	/* 立既顯示倒數秒數 */
	inDISP_LiveCountdown(lnTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	
	while (1)
        {
                /* （idle畫面刷卡此function不會發生） or 刷卡事件發生 */
	        if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
	        {
                        /* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
                        inFunc_ResetTitle(pobTran);
                        
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
                                
				/* Unpack失敗，再開感應太麻煩 */
	                        return(VS_ERROR);
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

                        /* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
			
	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
			{
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
	                        return (VS_ERROR);
			}

	                /* 第七步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
                        
                        break;
	        }
                /* （idle畫面人工輸入此function不會發生）or Menu Keyin */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
			
			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}
			
			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&& 
			   (pobTran->inTransactionCode != _REFUND_		&& 
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&&
			    pobTran->inTransactionCode != _CUP_PRE_COMP_	&&
			    pobTran->inTransactionCode != _HG_REWARD_REFUND_	&&
			    pobTran->inTransactionCode != _HG_REDEEM_REFUND_))
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

				return (VS_ERROR);
			}
			
			/* 客製化038 bellavita 退貨 預先授權 預先授權完成 不能人工輸入 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->inTransactionCode == _REFUND_		|| 
				    pobTran->inTransactionCode == _INST_REFUND_		||
				    pobTran->inTransactionCode == _REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _PRE_AUTH_		||
				    pobTran->inTransactionCode == _PRE_COMP_		||
				    pobTran->inTransactionCode == _CUP_REFUND_		||
				    pobTran->inTransactionCode == _CUP_INST_REFUND_	||
				    pobTran->inTransactionCode == _CUP_REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _CUP_PRE_AUTH_	||
				    pobTran->inTransactionCode == _CUP_PRE_COMP_	||
				    pobTran->inTransactionCode == _HG_REWARD_REFUND_	||
				    pobTran->inTransactionCode == _HG_REDEEM_REFUND_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

					return (VS_ERROR);
				}
			}
			
			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);
			
                        /* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

                        /* 第三步選擇輸入檢查碼或有效期 */
                        inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

                        /* 第四步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        break;
                }
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_Refund(pobTran);
			
			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;
				
				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);
                               
				/* 轉 FISC_Refund */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* 判斷card bin 讀HDT */
				if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                       return (VS_ERROR);

				/* 檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 檢核ExpDate */
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Refund Command */
				if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
				
				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}
								
				return (VS_ERROR);
			}
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			inFunc_ResetTitle(pobTran);

			break;
		}
                
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;
			
				inDISP_Msg_BMP(&srDispMsgObj);
                                
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
				
				/* 顯示對應交易別的感應畫面 */
				inCTLS_Decide_Display_Image(pobTran);
				
				/* 回復虛擬燈號 */
				EMVCL_ShowVirtualLED(NULL);
				
				/* 顯示金額 */
				if (pobTran->srBRec.lnTxnAmount > 0)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
					inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
				}
			}
			
			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForRefund_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}

			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();
			
			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
					inCTLS_CancelTransacton_Flow();
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				/* (需求單 - 109327)V3機型開發支援Vx520的客製化參數需求 (002-耐斯廣場/王子大飯店) 退貨交易不能輸入卡號 by Russell 2020/10/6 下午 4:46 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* key in事件 */
					ginEventCode = _MENUKEYIN_EVENT_;
				}
			}
			else if (szKey == _KEY_FUNCTION_)
			{
				if (pobTran->srBRec.inCode == _REFUND_)
				{
					/* 要有開銀聯感應功能才能開 */
					if (!memcmp(szCTLSEnable, "Y", 1)		&&
					    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
					    guszCTLSInitiOK == VS_TRUE)
					{
						/* 只有ECR144 才能轉 */
						if (pobTran->uszECRBit == VS_TRUE	&&
						    gsrECROb.srSetting.inCustomerLen == _ECR_7E1_Standard_Data_Size_)
						{
							/* 轉成銀聯一般交易的畫面 */
							pobTran->inTransactionCode = _CUP_REFUND_;
							pobTran->srBRec.inCode = pobTran->inTransactionCode;
							pobTran->inRunTRTID = _TRT_CUP_REFUND_;
							/* 標示是CUP交易 */
							pobTran->srBRec.uszCUPTransBit = VS_TRUE;

							/* 顯示對應交易別的感應畫面 */
							inCTLS_Decide_Display_Image(pobTran);

							/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
							EMVCL_StartIdleLEDBehavior(NULL);

							/* 顯示金額 */
							if (pobTran->srBRec.lnTxnAmount > 0)
							{
								memset(szTemplate, 0x00, sizeof(szTemplate));
								sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
								inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
								inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
							}
						}
					}
				}
			}
			
			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/
		
		
	} /* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_Refund_CTLS_Txno
Date&Time       :2017/6/27 下午 4:26
Describe        :進這隻必定CTLS Enable為開，感應退貨加交易編號
*/
int inFunc_GetCardFields_Refund_CTLS_Txno(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char		szKey = -1;
        char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szBatchNum[6 + 1] = {0};
	char		szSTANNum[6 + 1] = {0};
	char		szInvoiceNum[6 + 1] = {0};
	char		szCTLSEnable[2 + 1] = {0};
	char		szCUPContactlessEnable[1 + 1] = {0};
	long		lnTimeout = 0;
        unsigned long   ulCTLS_RetVal = 0;

        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_Refund_CTLS_Txno START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields_Refund_CTLS_Txno() START !");
	}
	
	memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
	inGetContactlessEnable(szCTLSEnable);
	memset(szCUPContactlessEnable, 0x00, sizeof(szCUPContactlessEnable));
	inGetCUPContactlessEnable(szCUPContactlessEnable);
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->srBRec.uszUnyTransBit == VS_TRUE)
	{
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);

		memset(szSTANNum, 0x00, sizeof(szSTANNum));
		inGetSTANNum(szSTANNum);
		pobTran->srBRec.lnSTANNum = atol(szSTANNum);

		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetInvoiceNum(szInvoiceNum);
		pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
			
		return (VS_SUCCESS);
	}
	else if (pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
        {
		/* 取卡號有效期 */
		if (pobTran->in57_Track2Len > 0)
		{
			inRetVal = inCARD_unPackCard(pobTran);
			if (inRetVal != VS_SUCCESS)
			    return (VS_NO_CARD_BIN);
		}
		else
		{
			return (VS_NO_CARD_BIN);
		}
			
                /* 判斷card bin 讀HDT */
                if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                       return (VS_NO_CARD_BIN);

                /* 檢核PAN module 10 */
                memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
                inGetModule10Check(szFuncEnable);
                if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
                {
                        /* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
                        if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
                                return (VS_CARD_PAN_ERROR);
                }

                /* 檢核ExpDate */
                if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
                        return (VS_CARD_EXP_ERR);
			
		return (VS_SUCCESS);
        }
	else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE && pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		
	}
	else
	{
		/* 有可能第一段是正向，第二段是退貨 */
		if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
		{
			/* 【需求單 - 105244】端末設備支援以感應方式進行退貨交易 */
			/* 端末設備支援以感應方式進行退貨交易，並讀取卡號及有效期，以人工輸入卡號方式之電文上傳至ATS主機取的授權。 */
			/* 將contactless flag OFF掉並把manual keyin flag ON */
			pobTran->srBRec.uszContactlessBit = VS_FALSE;
			pobTran->srBRec.uszManualBit = VS_TRUE;
			pobTran->srBRec.uszRefundCTLSBit = VS_TRUE;
			pobTran->srBRec.uszNoSignatureBit = VS_FALSE;
		}
		
		/* 如果有卡號就直接跳走 */
		if (strlen(pobTran->srBRec.szPAN) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;
	/* 初始化 ginEventCode */
        ginEventCode = -1;

	/* 顯示對應交易別的感應畫面 */
	inCTLS_Decide_Display_Image(pobTran);
				
	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        EMVCL_StartIdleLEDBehavior(NULL);
        
        /* 顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
		inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
	}
	
	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
        
	/* Send CTLS Readly for Refund Command */
	if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_Txno(pobTran);
		return (inRetVal);
	}
	
        /* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
        
        /* Mirror Message */
        if (pobTran->uszECRBit == VS_TRUE)
	{
                inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_REFUND_);
        }
	
	/* 剩餘倒數時間 開始 */
	inDISP_TimeoutStart(lnTimeout);
	/* 立既顯示倒數秒數 */
	inDISP_LiveCountdown(lnTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	
	while (1)
        {
                /* （idle畫面刷卡此function不會發生） or 刷卡事件發生 */
	        if (ginIdleMSRStatus == VS_TRUE || ginEventCode == _SWIPE_EVENT_)
	        {
                        /* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
                        inFunc_ResetTitle(pobTran);
                        
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
                                
				/* Unpack失敗，再開感應太麻煩 */
	                        return(VS_ERROR);
	                }

                        /* 第三步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);

                        /* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
			
	                /* 第六步驟檢核SeviceCode */
                        if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
			{
				pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
	                        return (VS_ERROR);
			}

	                /* 第七步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
                        
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
                        break;
	        }
                /* （idle畫面人工輸入此function不會發生）or Menu Keyin */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
			
			/* 人工輸入卡號開關 */
			/* 退貨交易及預先授權/預先授權完成交易部參考人工輸入功能開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0		&& 
			   (pobTran->inTransactionCode != _REFUND_		&& 
			    pobTran->inTransactionCode != _INST_REFUND_		&&
			    pobTran->inTransactionCode != _REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _PRE_AUTH_		&&
			    pobTran->inTransactionCode != _PRE_COMP_		&&
			    pobTran->inTransactionCode != _CUP_REFUND_		&&
			    pobTran->inTransactionCode != _CUP_INST_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_REDEEM_REFUND_	&&
			    pobTran->inTransactionCode != _CUP_PRE_AUTH_	&&
			    pobTran->inTransactionCode != _CUP_PRE_COMP_	&&
			    pobTran->inTransactionCode != _HG_REWARD_REFUND_	&&
			    pobTran->inTransactionCode != _HG_REDEEM_REFUND_))
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

				return (VS_ERROR);
			}
			
			/* 客製化038 bellavita 退貨 預先授權 預先授權完成 不能人工輸入 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_))
			{
				if (pobTran->inTransactionCode == _REFUND_		|| 
				    pobTran->inTransactionCode == _INST_REFUND_		||
				    pobTran->inTransactionCode == _REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _PRE_AUTH_		||
				    pobTran->inTransactionCode == _PRE_COMP_		||
				    pobTran->inTransactionCode == _CUP_REFUND_		||
				    pobTran->inTransactionCode == _CUP_INST_REFUND_	||
				    pobTran->inTransactionCode == _CUP_REDEEM_REFUND_	||
				    pobTran->inTransactionCode == _CUP_PRE_AUTH_	||
				    pobTran->inTransactionCode == _CUP_PRE_COMP_	||
				    pobTran->inTransactionCode == _HG_REWARD_REFUND_	||
				    pobTran->inTransactionCode == _HG_REDEEM_REFUND_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

					return (VS_ERROR);
				}
			}
			
			inRetVal = inCREDIT_Func_Get_Card_Number_Txno_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);
			
			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

                        /* 第三步選擇輸入檢查碼或有效期 */
                        inRetVal = inCREDIT_Func_Get_CheckNO_ExpDate_Flow(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

                        /* 第四步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
                        break;
                }
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_Refund(pobTran);
			
			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;
				
				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);
                               
				/* 轉 FISC_Refund */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* 判斷card bin 讀HDT */
				if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                       return (VS_ERROR);

				/* 檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 檢核ExpDate */
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Refund Command */
				if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
				
				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}
								
				return (VS_ERROR);
			}
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			inFunc_ResetTitle(pobTran);

			break;
		}
                
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;
			
				inDISP_Msg_BMP(&srDispMsgObj);
                                
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
				
				/* 顯示對應交易別的感應畫面 */
				inCTLS_Decide_Display_Image(pobTran);
				
				/* 回復虛擬燈號 */
				EMVCL_ShowVirtualLED(NULL);
				
				/* 顯示金額 */
				if (pobTran->srBRec.lnTxnAmount > 0)
				{
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
					inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
					inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
				}
				
			}
			
			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForRefund_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}

			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();
			
			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
					inCTLS_CancelTransacton_Flow();
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				/* (需求單 - 109327)V3機型開發支援Vx520的客製化參數需求 (002-耐斯廣場/王子大飯店) 退貨交易不能輸入卡號 by Russell 2020/10/6 下午 4:46 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_002_NICE_PLAZA_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 銀聯一般交易不能輸入卡號 */
					if (pobTran->inTransactionCode == _CUP_SALE_)
					{
						continue;
					}
					else
					{
						/* key in事件 人工輸入請按0*/
						if (szKey == '0')
						{
							ginEventCode = _MENUKEYIN_EVENT_;
						}
					}
				}
			}
			else if (szKey == _KEY_FUNCTION_)
			{
				if (pobTran->srBRec.inCode == _REFUND_)
				{
					/* 要有開銀聯感應功能才能開 */
					if (!memcmp(szCTLSEnable, "Y", 1)		&&
					    !memcmp(szCUPContactlessEnable, "Y", 1)	&&
					    guszCTLSInitiOK == VS_TRUE)
					{
						/* 只有ECR144 才能轉 */
						if (pobTran->uszECRBit == VS_TRUE	&&
						    gsrECROb.srSetting.inCustomerLen == _ECR_7E1_Standard_Data_Size_)
						{
							/* 轉成銀聯一般交易的畫面 */
							pobTran->inTransactionCode = _CUP_REFUND_;
							pobTran->srBRec.inCode = pobTran->inTransactionCode;
							pobTran->inRunTRTID = _TRT_CUP_REFUND_;
							/* 標示是CUP交易 */
							pobTran->srBRec.uszCUPTransBit = VS_TRUE;

							/* 顯示對應交易別的感應畫面 */
							inCTLS_Decide_Display_Image(pobTran);

							/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
							EMVCL_StartIdleLEDBehavior(NULL);

							/* 顯示金額 */
							if (pobTran->srBRec.lnTxnAmount > 0)
							{
								memset(szTemplate, 0x00, sizeof(szTemplate));
								sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
								inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
								inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
							}
						}
					}
				}
			}
			
			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/
		
		
	} /* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_FISC
Date&Time       :2017/4/5 上午 11:43
Describe        :SmartPay選單進入使用 只能用晶片
*/
int inFunc_GetCardFields_FISC(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char	szKey = 0;
	char	szCustomerIndicator[3 + 1] = {0};
	char	szTemplate[3 + 1] = {0};
	long	lnTimeout = 0;

        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_FISC START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields_FISC() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE && pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		
	}
	else
	{
                /* 如果有卡號就直接跳走 */
                if (strlen(pobTran->srBRec.szPAN) > 0)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_FISC PAN > 0 END!");
                        return (VS_SUCCESS);
                }
	}
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
	
	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);
	
        while (1)
        {
                /* 插晶片卡 */
                if  (ginEventCode == _EMV_DO_EVENT_)
                {
                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
                                return (VS_ERROR);
                        else
                        {
                                /* SmartPay晶片卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        /* Mirror Message */
                                        if (pobTran->uszECRBit == VS_TRUE)
                                        {
                                                inECR_SendMirror(pobTran, _MIRROR_MSG_NOT_SUP_CARD_);
                                        }
        
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_CARD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
                                        inDISP_Msg_BMP(&srDispMsgObj);
                                        return (VS_ERROR);
                                }
                                else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					
					if (inFISC_Read_Card_Data_Flow(pobTran) != VS_SUCCESS)
                                        {
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inFISC_Read_Card_Data_Flow_failed END!");
						return (VS_ERROR);
                                        }
					
					inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				}
                                pobTran->srBRec.inHDTIndex = 0;
                        }
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        break;
                }

		
		/* 顯示請刷卡或插卡 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_GET_FISC_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
		
		/* 客製化123，過卡時設為較最亮 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
		}
			
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			/* ManuKeyin 為數字鍵時 */
			else if(szKey >= '0' && szKey <= '9')
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_) ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 清鍵盤buffer */
					inFlushKBDBuffer();
				}
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
				
		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
	
	return (VS_SUCCESS);
}
/*
Function        :inFunc_GetCardFields_FISC_CTLS
Date&Time       :2017/9/8 下午 2:16
Describe        :SmartPay選單進入 感應
*/
int inFunc_GetCardFields_FISC_CTLS(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	long		lnTimeout = 0;
	char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
        char		szKey = 0;
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned long   ulCTLS_RetVal = 0;
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFunc_GetCardFields_FISC_CTLS() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_FISC_CTLS START!");

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE && pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		ginEventCode = _NONE_EVENT_;
	}
	else
	{
                /* 如果有卡號就直接跳走 */
                if (strlen(pobTran->srBRec.szPAN) > 0)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_FISC_CTLS PAN > 0 END!");
                        return (VS_SUCCESS);
                }
	}
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

        if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
        {

        }
        else
        {
                /* 顯示請插金融卡或感應 */
                inDISP_ClearAll();
                inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 ＜消費扣款＞ */
                inDISP_PutGraphic(_CTLS_FISCICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 顯示請插金融卡或感應 */

                /* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
                EMVCL_StartIdleLEDBehavior(NULL);

                /* 顯示金額 */
                if (pobTran->srBRec.lnTxnAmount > 0)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
                        inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
                        inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
                }
        }
        
	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
	
	/* Send CTLS Readly for Sale Command */
	if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_FISC(pobTran);
		return (inRetVal);
	}
	
	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
	
	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);
	/* 立既顯示倒數秒數 */
	inDISP_LiveCountdown(lnTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	
        while (1)
        {
                /* 插晶片卡 */
                if  (ginEventCode == _EMV_DO_EVENT_)
                {
			/* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
			
                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
			{
                                return (VS_ERROR);
			}
                        else
                        {
                                /* SmartPay晶片卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        /* Mirror Message */
                                        if (pobTran->uszECRBit == VS_TRUE)
                                        {
                                                inECR_SendMirror(pobTran, _MIRROR_MSG_NOT_SUP_CARD_);
                                        }
                                        
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_CARD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
                                        inDISP_Msg_BMP(&srDispMsgObj);
                                        return (VS_ERROR);
                                }
                                else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					
					if (inFISC_Read_Card_Data_Flow(pobTran) != VS_SUCCESS)
                                        {
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inFISC_Read_Card_Data_Flow_failed END!");
						return (VS_ERROR);
                                        }
					
					inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				}
                                pobTran->srBRec.inHDTIndex = 0;
                        }
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        break;
                }
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_SALE(pobTran);
			
			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;
				
				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);
                               
				/* 轉 FISC_Refund */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* SmartPay卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        /* Mirror Message */
                                        if (pobTran->uszECRBit == VS_TRUE)
                                        {
                                                inECR_SendMirror(pobTran, _MIRROR_MSG_NOT_SUP_CARD_);
                                        }
                                        
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_CARD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
                                        inDISP_Msg_BMP(&srDispMsgObj);
                                        return (VS_ERROR);
                                }
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Refund Command */
				if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
				
				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}
				return (VS_WAVE_ERROR);
			}
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			inFunc_ResetTitle(pobTran);

			break;
		}

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}
			
			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForSales_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

                        if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
                        {
                                /* 感應倒數時間 && Display Countdown */
                                if (inDISP_TimeoutCheck_Not_Disp() == VS_TIMEOUT)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inDISP_TimeoutCheck_Not_Disp Timeout");
                                        }
                                        /* 感應時間到Timeout */
                                        szKey = _KEY_TIMEOUT_;
                                }
                        }
                        else
                        {
                                /* 感應倒數時間 && Display Countdown */
                                if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inDISP_TimeoutCheck_Not_Disp Timeout");
                                        }
                                        /* 感應時間到Timeout */
                                        szKey = _KEY_TIMEOUT_;
                                }
                        }

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			/* ManuKeyin 為數字鍵時 */
			else if(szKey >= '0' && szKey <= '9')
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 清鍵盤buffer */
					inFlushKBDBuffer();
				}
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
				
		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_FISC_CTLS_Refund
Date&Time       :2017/9/8 下午 2:16
Describe        :SmartPay選單進入 感應退貨
*/
int inFunc_GetCardFields_FISC_CTLS_Refund(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
	long		lnTimeout = 0;
	char		szTemplate[_DISP_MSG_SIZE_ + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
        char		szKey = 0;
	unsigned long   ulCTLS_RetVal = 0;

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields_FISC_CTLS_Refund() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE && pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		ginEventCode = _NONE_EVENT_;
	}
	else
	{
                /* 如果有卡號就直接跳走 */
                if (strlen(pobTran->srBRec.szPAN) > 0)
                {
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_FISC_CTLS_Refund PAN > 0 END!");
                        return (VS_SUCCESS);
                }
	}
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 顯示請插金融卡或感應 */
	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 ＜退費交易＞ */
	inDISP_PutGraphic(_CTLS_FISCICC_RF_, 0, _COORDINATE_Y_LINE_8_3_);		/* 顯示請插金融卡或感應 */
	
	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        EMVCL_StartIdleLEDBehavior(NULL);
        
        /* 顯示金額 */
	if (pobTran->srBRec.lnTxnAmount > 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%ld",  pobTran->srBRec.lnTxnAmount);
		inFunc_Amount_Comma(szTemplate, "NT$ " , '\x00', _SIGNED_NONE_, 16, _PADDING_RIGHT_);
		inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X16_, _LINE_8_3_, _COLOR_RED_, _DISP_RIGHT_);
	}
        
	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
	
	/* Send CTLS Readly for Refund Command */
	if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_FISC(pobTran);
		return (inRetVal);
	}
	
	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
	
	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);
	/* 立既顯示倒數秒數 */
	inDISP_LiveCountdown(lnTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	
        while (1)
        {
                /* 插晶片卡 */
                if  (ginEventCode == _EMV_DO_EVENT_)
                {
			/* 取消感應交易 */
                        inCTLS_CancelTransacton_Flow();
			
                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
			{
                                return (VS_ERROR);
			}
                        else
                        {
                                /* SmartPay晶片卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        /* Mirror Message */
                                        if (pobTran->uszECRBit == VS_TRUE)
                                        {
                                                inECR_SendMirror(pobTran, _MIRROR_MSG_NOT_SUP_CARD_);
                                        }
                                        
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_CARD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
                                        inDISP_Msg_BMP(&srDispMsgObj);
                                        return (VS_ERROR);
                                }
                                else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					
					if (inFISC_Read_Card_Data_Flow(pobTran) != VS_SUCCESS)
                                        {
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_ICC inFISC_Read_Card_Data_Flow_failed END!");
						return (VS_ERROR);
                                        }
					
					inFLOW_RunFunction(pobTran, _FISC_CARD_POWER_OFF_);
				}
                                pobTran->srBRec.inHDTIndex = 0;
                        }

			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_ICC_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
                        break;
                }
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_Refund(pobTran);
			
			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;
				
				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);
                               
				/* 轉 FISC_Refund */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* SmartPay卡 */
                                if (pobTran->srBRec.uszFiscTransBit != VS_TRUE)
                                {
                                        /* Mirror Message */
                                        if (pobTran->uszECRBit == VS_TRUE)
                                        {
                                                inECR_SendMirror(pobTran, _MIRROR_MSG_NOT_SUP_CARD_);
                                        }
                                        
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_CARD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
                                        inDISP_Msg_BMP(&srDispMsgObj);
                                        return (VS_ERROR);
                                }
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Refund Command */
				if (inCTLS_SendReadyForRefund_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
				
				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}
				return (VS_ERROR);
			}
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			inFunc_ResetTitle(pobTran);

			break;
		}

		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測晶片卡---------------- */
			inEMV_RetVal = inEMV_ICCEvent();
			if (inEMV_RetVal == VS_SUCCESS)
			{
				/* 晶片卡事件 */
				ginEventCode = _EMV_DO_EVENT_;
			}
			
			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForRefund_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}

			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			/* ManuKeyin 為數字鍵時 */
			else if(szKey >= '0' && szKey <= '9')
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 清鍵盤buffer */
					inFlushKBDBuffer();
				}
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
				
		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_Loyalty_Redeem_Swipe
Date&Time       :2017/1/20 下午 4:29
Describe        :優惠兌換顯示刷卡及請輸入卡號，僅手動輸入檢核卡號，刷卡不檢核
*/
int inFunc_GetCardFields_Loyalty_Redeem_Swipe(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inNCCCIndex = -1;
	int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
        char	szKey = 0;
	char	szBatchNum [6 + 1] = {0}, szInvoiceNum [6 + 1] = {0}, szSTANNum [6 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szTemplate[3 + 1] = {0};
	long	lnTimeout = 0;
	
	vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_Loyalty_Redeem_Swipe START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields_Loyalty_Redeem_Swipe() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}

        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請刷卡或輸入卡號 */
	inDISP_PutGraphic(_GET_CARD_AWARD_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
		
	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
        
        /* Mirror Message */
        if (pobTran->uszECRBit == VS_TRUE)
        {
                inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_REDEEM_REFUND_);
        }
	
	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);
	
        while (1)
        {
                /* idle 刷卡 或 一般刷卡 */
	        if (ginIdleMSRStatus == VS_TRUE  || ginEventCode == _SWIPE_EVENT_)
	        {
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
                                
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }

                        /* 第三步驟 不讀CDT 直接指定Host */
			inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
                        pobTran->srBRec.inHDTIndex = inNCCCIndex;   
			
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			memset(szBatchNum, 0x00, sizeof(szBatchNum));
			inGetBatchNum(szBatchNum);
			pobTran->srBRec.lnBatchNum = atol(szBatchNum);

			memset(szSTANNum, 0x00, sizeof(szSTANNum));
			inGetSTANNum(szSTANNum);
			pobTran->srBRec.lnSTANNum = atol(szSTANNum);

			memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
			inGetInvoiceNum(szInvoiceNum);
			pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
			
			memcpy(pobTran->szL3_AwardWay, "4", 1);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
                        break;
	        }
                /* idle手動輸入或一般輸入 */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}
			
			/* 人工輸入卡號開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

				return (VS_ERROR);
			}
			
			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);
			
                        /* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
			inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);
			
			/* ‘5’=以卡號當作兌換資訊，於端末機上手動輸入 */
			memcpy(pobTran->szL3_AwardWay, "5", 1);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        break;
                }
		
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
	
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;
			
				inDISP_Msg_BMP(&srDispMsgObj);
                                
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
				
				/* 顯示請刷卡或輸入卡號 */
				inDISP_PutGraphic(_GET_CARD_AWARD_, 0, _COORDINATE_Y_LINE_8_4_);
			}
			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				/* 客製化005，只有退貨支援Menu keyin */
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 銀聯一般交易不能輸入卡號 */
					if (pobTran->inTransactionCode == _CUP_SALE_)
					{
						continue;
					}
					else
					{
						/* key in事件 */
						ginEventCode = _MENUKEYIN_EVENT_;
					}
				}
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_Loyalty_Redeem_CTLS
Date&Time       :2017/11/9 下午 2:51
Describe        :優惠兌換顯示請刷卡、感應卡片或輸入卡號
*/
int inFunc_GetCardFields_Loyalty_Redeem_CTLS(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inNCCCIndex = -1;
	int		inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int		inCTLS_RetVal = -1;	/* 感應卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char		szKey = 0;
	char		szBatchNum [6 + 1] = {0}, szInvoiceNum [6 + 1] = {0}, szSTANNum [6 + 1] = {0};
	char		szFuncEnable[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szTemplate[3 + 1] = {0};
	long		lnTimeout = 0;
        unsigned long   ulCTLS_RetVal = 0x00;

	vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_Loyalty_Redeem_CTLS START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetCardFields_Loyalty_Redeem_CTLS() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 如果有卡號就直接跳走 */
	if (strlen(pobTran->srBRec.szPAN) > 0)
	{
		return (VS_SUCCESS);
	}
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;
	
	/* Send CTLS Readly for Sale Command */
	if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
	{
		/* 如果Send 失敗，轉成沒感應的界面 */
		inRetVal = inFunc_GetCardFields_ICC(pobTran);
		return (inRetVal);
	}
	
	inDISP_ClearAll();
	inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 ＜HG卡分期付款＞ */
	/* 顯示請刷卡或輸入卡號 */
	inDISP_PutGraphic(_CTLS_AWARD_, 0, _COORDINATE_Y_LINE_8_3_);
	
	/* 顯示虛擬LED(不閃爍)，但inCTLS_ReceiveReadyForSales(V3機型)也會顯示LED（閃爍），所以沒有實質作用 */
        EMVCL_StartIdleLEDBehavior(NULL);
	
	/* 只抓卡號，不用金額 */

	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
	
	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
	
	/* inSecond剩餘倒數時間 */
	inDISP_TimeoutStart(lnTimeout);
	/* 立既顯示倒數秒數 */
	inDISP_LiveCountdown(lnTimeout,_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_);
	
        while (1)
        {
                /* idle 刷卡 或 一般刷卡 */
	        if (ginIdleMSRStatus == VS_TRUE  || ginEventCode == _SWIPE_EVENT_)
	        {
			/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
			inCTLS_CancelTransacton_Flow();
				
	                /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
                                
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }

                        /* 第三步驟 不讀CDT 直接指定Host */
			inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
                        pobTran->srBRec.inHDTIndex = inNCCCIndex;   
			
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			memset(szBatchNum, 0x00, sizeof(szBatchNum));
			inGetBatchNum(szBatchNum);
			pobTran->srBRec.lnBatchNum = atol(szBatchNum);

			memset(szSTANNum, 0x00, sizeof(szSTANNum));
			inGetSTANNum(szSTANNum);
			pobTran->srBRec.lnSTANNum = atol(szSTANNum);

			memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
			inGetInvoiceNum(szInvoiceNum);
			pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
			
			/* ‘4’=以卡號當作兌換資訊，於端末機上刷卡輸入 */
			memcpy(pobTran->szL3_AwardWay, "4", 1);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
  	                               
                        break;
	        }
                /* idle手動輸入或一般輸入 */
                else if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
			inCTLS_CancelTransacton_Flow();
				
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}
			
			/* 人工輸入卡號開關 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;

				return (VS_ERROR);
			}
			
			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);
			
                        /* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			memset(pobTran->srBRec.szExpDate, 0x00, sizeof(pobTran->srBRec.szExpDate));
			inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);
			
			/* ‘5’=以卡號當作兌換資訊，於端末機上手動輸入 */
			memcpy(pobTran->szL3_AwardWay, "5", 1);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        break;
                }
		/* 感應事件 */
		else if (ginEventCode == _SENSOR_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* 收到的retuen不是等待資料的狀態就去解析 */
			ulCTLS_RetVal = ulCTLS_CheckResponseCode_SALE(pobTran);
			
			if (ulCTLS_RetVal == d_EMVCL_RC_NO_CARD)
                        {
				/* Timeout沒卡 */
				return (VS_ERROR);
			}
			else if (ulCTLS_RetVal == d_EMVCL_RC_MORE_CARDS)
			{
				pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_;
				
				return (VS_WAVE_ERROR);
			}
                        /* 這邊要切SmartPay */
                        else if (ulCTLS_RetVal == d_EMVCL_NON_EMV_CARD)
                        {
				if (pobTran->srBRec.inTxnResult != VS_SUCCESS)
					 return (VS_ERROR);
                               
				/* 轉 FISC_SALE */
				/* FISC incode 在 inFISC_CTLSProcess內設定 */
                        }
                        /* 走信用卡流程 */
                        else if (ulCTLS_RetVal == d_EMVCL_RC_DATA)
                        {
				/* 判斷card bin 讀HDT */
				if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                       return (VS_ERROR);

				/* 檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 檢核ExpDate */
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
                        }
			/*  Two Tap 流程 */
			else if (ulCTLS_RetVal == d_EMVCL_RC_SEE_PHONE)
			{
				/* 重Send命令，等第二次感應 */
				/* Send CTLS Readly for Sale Command */
				if (inCTLS_SendReadyForSale_Flow(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
				
				ginEventCode = -1;
				continue;
			}
			/* 感應錯誤 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "感應失敗,0x%08lX", ulCTLS_RetVal);
					inLogPrintf(AT, szDebugMsg);
				}
				
				if (pobTran->inErrorMsg == _ERROR_CODE_V3_NONE_)
				{
					pobTran->inErrorMsg = _ERROR_CODE_V3_WAVE_ERROR_;
				}
				
				return (VS_WAVE_ERROR);
			}
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_CTLS_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			inFunc_ResetTitle(pobTran);
			
			/* ‘ˋ’=以卡號當作兌換資訊，於端末機上刷卡輸入 */
			/* 因為NCCC沒有定義感應，所以先帶4 */
			memcpy(pobTran->szL3_AwardWay, "4", 1);

			break;
		}
		
		
			
		/* 進迴圈前先清MSR BUFFER */
		inCARD_Clean_MSR_Buffer();
	
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測刷卡------------------*/
			inMSR_RetVal = inCARD_MSREvent();
			if (inMSR_RetVal == VS_SUCCESS)
			{
				/* 刷卡事件 */
				ginEventCode = _SWIPE_EVENT_;
			}
			/* 回復錯誤訊息蓋掉的圖 */
			else if (inMSR_RetVal == VS_SWIPE_ERROR)
			{
				inFunc_ResetTitle(pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;
			
				inDISP_Msg_BMP(&srDispMsgObj);
                                
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
				
				inDISP_ClearAll();
				inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0, _COORDINATE_Y_LINE_8_2_);	/* 第二層顯示 ＜HG卡分期付款＞ */
				/* 顯示請刷卡或輸入卡號 */
				inDISP_PutGraphic(_CTLS_AWARD_, 0, _COORDINATE_Y_LINE_8_3_);
			}
			
			/* ------------偵測感應卡------------------ */
			inCTLS_RetVal = inCTLS_ReceiveReadyForSales_Flow(pobTran);
			if (inCTLS_RetVal == VS_SUCCESS)
			{
				/* 感應卡事件 */
				ginEventCode = _SENSOR_EVENT_;
			}
			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 感應倒數時間 && Display Countdown */
			if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
			{
				/* 感應時間到Timeout */
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
				inCTLS_CancelTransacton_Flow();
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 因CTLS Timeout參數在XML內，所以將CTLS API Timeout時間拉長至60秒，程式內直接用取消交易來模擬TimeOut效果 */
					inCTLS_CancelTransacton_Flow();
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				/* 客製化005，只有退貨支援Menu keyin */
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* 銀聯一般交易不能輸入卡號 */
					if (pobTran->inTransactionCode == _CUP_SALE_)
					{
						continue;
					}
					else
					{
						/* key in事件 */
						ginEventCode = _MENUKEYIN_EVENT_;
					}
				}
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetBarCodeFlow
Date&Time       :2017/2/18 下午 1:58
Describe        :取得優惠條碼
*/
int inFunc_GetBarCodeFlow(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inNCCCIndex = -1;
        char		szKey = 0;
	char		szBatchNum [6 + 1] = {0}, szInvoiceNum [6 + 1] = {0}, szSTANNum [6 + 1] = {0};
	char		szBarCodeReaderEnable[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szTemplate[3 + 1] = {0};
	long		lnTimeout = 0;
	unsigned char	uszBarCodeReaderEnable = VS_FALSE;
	
	vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetBarCodeFlow START!");
	
	/* 看是否有開啟BarcodeReader，在這邊看TMS開關而不在迴圈中檢查，增進效率 */
	inGetBarCodeReaderEnable(szBarCodeReaderEnable);
	if (memcmp(szBarCodeReaderEnable, "Y", strlen("Y")) == 0)
	{
		uszBarCodeReaderEnable = VS_TRUE;
	}

	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 不讀CDT 直接指定Host */
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
	pobTran->srBRec.inHDTIndex = inNCCCIndex;   

	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	inGetBatchNum(szBatchNum);
	pobTran->srBRec.lnBatchNum = atol(szBatchNum);

	memset(szSTANNum, 0x00, sizeof(szSTANNum));
	inGetSTANNum(szSTANNum);
	pobTran->srBRec.lnSTANNum = atol(szSTANNum);

	memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
	inGetInvoiceNum(szInvoiceNum);
	pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
	
	if (ginEventCode == _ECR_EVENT_)
	{
		/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
		if (ginFallback == VS_TRUE)
		{
			inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
		}

		/* ‘1’=以條碼當作兌換資訊，透過收銀機條碼資訊 */
		memcpy(pobTran->szL3_AwardWay, "1", 1);

		if (strlen(pobTran->szL3_Barcode1) > 0	||
		    strlen(pobTran->szL3_Barcode2) > 0)
		{
			return (VS_SUCCESS);
		}
	}
	
	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			ginEventCode = _ECR_EVENT_;
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 顯示請掃描條碼或輸入條碼 */
	inDISP_PutGraphic(_GET_BARCODE_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
	
	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);
		
	while (memcmp(pobTran->szL3_Barcode1, "11", strlen("11")) != 0	&&
	       memcmp(pobTran->szL3_Barcode1, "21", strlen("21")) != 0)	/* 輸入Barcode1迴圈(若輸入值不合法，就繼續輸入) */
        {
                /* idle手動輸入或一般輸入 */
                if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}
			
			/* 抓BarCode1 */
			inRetVal = inCREDIT_Func_Get_Barcode1(pobTran);
			
			/* 重置輸入時第一個字元 */
			szKey = -1;
			pobTran->inMenuKeyin = 0;
			
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			/* ‘3’=以條碼當作兌換資訊，手動於端末機輸入兌換(核銷)條碼。 */
			memcpy(pobTran->szL3_AwardWay, "3", 1);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* 跳出Barcode1 迴圈 */
			break;
                }
		else if (ginEventCode == _BARCODE_READER_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* ‘2’=以條碼當作兌換資訊，端末機接BarCode Reader掃描兌換(核銷)條碼。 */
			memcpy(pobTran->szL3_AwardWay, "2", 1);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			/* 跳出Barcode1 迴圈 */
			break;;
		}
		
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* Cancel */
				return (VS_USER_CANCEL);
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}
			
			/* ------------偵測Barcode Reader------------------ */
			/* 之後有接Barcode Reader用 */
			if (0/* 這裡放Barcode Reader讀成功的條件 */)
			{
				/* 端末機收到BarCode Reader */
				ginEventCode = _BARCODE_READER_EVENT_;
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/

	}/* 對事件做回應迴圈... */
	
	
	/* 判斷BarCode1後決定是否抓BarCode 2 */
	if (memcmp(pobTran->szL3_Barcode1, "11", strlen("11")) == 0)
	{
		/* 客製化123，過卡完時設為較暗 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inGetScreenBrightness(szTemplate);
			if (strlen(szTemplate) > 0)
			{
				inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
			}
		}
	
		/* 若"11"，則只有一段條碼*/
		return (VS_SUCCESS);
	}
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	/* 請掃描或輸入第二段條碼？ */
	inDISP_PutGraphic(_GET_BARCODE_2_, 0, _COORDINATE_Y_LINE_8_4_);
	
	/* 抓BarCode2 */
        while (memcmp(pobTran->szL3_Barcode2, "22", strlen("22")) != 0)		/* 輸入Barcode2迴圈(若輸入值不合法，就繼續輸入) */
        {
                /* idle手動輸入或一般輸入 */
                if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}
			
			/* 抓BarCode2 */
			inRetVal = inCREDIT_Func_Get_Barcode2(pobTran);
			
			/* 重置輸入時第一個字元 */
			szKey = -1;
			pobTran->inMenuKeyin = 0;
			
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			/* ‘3’=以條碼當作兌換資訊，手動於端末機輸入兌換(核銷)條碼。 */
			memcpy(pobTran->szL3_AwardWay, "3", 1);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			/* 跳出Barcode1 迴圈 */
			break;
                }
		else if (ginEventCode == _BARCODE_READER_EVENT_)
		{
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* ‘2’=以條碼當作兌換資訊，端末機接BarCode Reader掃描兌換(核銷)條碼。 */
			memcpy(pobTran->szL3_AwardWay, "2", 1);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			/* 跳出Barcode1 迴圈 */
			break;
		}
		
		while (1)
		{
			ginEventCode = -1;
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			/* ManuKeyin 為數字鍵時 */
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				/* 客製化005，只有退貨支援Menu keyin */
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* key in事件 */
					ginEventCode = _MENUKEYIN_EVENT_;
				}
			}
			
			/* ------------偵測Barcode Reader------------------ */
			/* 之後有接Barcode Reader用 */
			if (uszBarCodeReaderEnable == VS_TRUE)
			{
				if (0/* 這裡放Barcode Reader讀成功的條件 */)
				{
					/* 端末機收到BarCode Reader */
					ginEventCode = _BARCODE_READER_EVENT_;
				}
				
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/

	}/* 對事件做回應迴圈... */
	
        /* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_HG
Date&Time       :2017/3/1 下午 1:14
Describe        :HG選單進入使用 只能用晶片或磁條 
*/
int inFunc_GetCardFields_HG(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = 0;
        int	inMSR_RetVal = -1;	/* 磁條事件的反應，怕和其他用到inRetVal的搞混所以獨立出來 */
	int	inEMV_RetVal = -1;	/* 晶片卡事件的反應，怕和其他用到inRetVal的搞混所以獨立出來*/
        char	szKey = 0;
	char	szTransFunc[20 + 1] = {0};
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szTemplate[3 + 1] = {0};
	long	lnTimeout = 0;
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_HG START!");
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;
        
	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
        
        /* Mirror Message */
        if (pobTran->uszECRBit == VS_TRUE)
        {
                inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_REDEEM_);
        }
	
        inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);
	
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        if (pobTran->inRunOperationID == _OPERATION_HG_REFUND_)
        {
                /* 顯示請刷HG卡 */
                inDISP_PutGraphic(_GET_HG_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
        }
        else 
        {
                /* 顯示請刷或插HG卡 */
                inDISP_PutGraphic(_GET_HG_CARD_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
        }
	
	/* 客製化123，過卡時設為較最亮 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
        
        /* 進迴圈前先清MSR BUFFER */
        inCARD_Clean_MSR_Buffer();
        
        while (1) 
        {
                /* 一般刷卡 */
                if (ginEventCode == _SWIPE_EVENT_) 
                {
                        /* 第一步驟GetTrack123 */
	                inCARD_GetTrack123(pobTran);

	                /* 第二步驟unPackTrack資料 */
	                if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
	                {
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }
                                
				/* Unpack失敗，把flag關掉可以再手動輸入 */
	                        ginIdleMSRStatus = VS_FALSE;
	                        continue;
	                }
			/* 因HG混合交易如果HG非手輸但信用卡手輸 帳單匯印HG卡的持卡人姓名*/
			memset(pobTran->srBRec.szCardHolder, 0x00, sizeof(pobTran->srBRec.szCardHolder));
			
                        /* 將收到的卡號複製到szHGPAN中 */
                        strcpy(pobTran->srBRec.szHGPAN, pobTran->srBRec.szPAN);
			
			/* 第三步驟 判斷HG card bin */
                        if (inCARD_GetBin_HG(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);
			
			/* 純HG不檢核 */
			if (memcmp(pobTran->srBRec.szHGCardLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
			{

			}
			else
			{
				/* 第二步驟檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 第四步驟檢核ExpDate */
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
			
                        break;
                }
		/* Menu Keyin */
                else if (ginEventCode == _MENUKEYIN_EVENT_)
                {
			if (inHG_Func_SetHost_HG(pobTran) != VS_SUCCESS)
			{
				/* 如果沒有HG HOST 直接跳出，不回傳ERROR */
				return (VS_SUCCESS);
			}
			
			memset(szTransFunc, 0x00, sizeof(szTransFunc));
			if (inGetTransFunc(szTransFunc) != VS_SUCCESS)
			    return (VS_ERROR);

			/* 人工輸入卡號開關 */
			/* 這裡是HG所以可以在輸入卡號前就判別 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetManualKeyin(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) != 0)
			{
				/* 紅利積點人工輸入卡號 */
				if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_ && memcmp(&szTransFunc[7], "Y", 1) == 0)
				{
					
				}
				/* 回饋退貨和扣抵退貨要可以輸入 */
				else if (pobTran->inTransactionCode == _HG_REWARD_REFUND_ || 
					 pobTran->inTransactionCode == _HG_REDEEM_REFUND_)
				{
					/* 客製化038 bellavita 退貨 預先授權 預先授權完成 不能人工輸入 */
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_))
					{
						/* Mirror Message */
						if (pobTran->uszECRBit == VS_TRUE)
						{
							inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
						}
						
						pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
						inRetVal = VS_ERROR;
						
						return (inRetVal);
					}
				}
				/* 其他都擋下來 */
				else
				{
                                        /* Mirror Message */
                                        if (pobTran->uszECRBit == VS_TRUE)
                                        {
                                                inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
                                        }
					
					/* 客製化038 bellavita 退貨 預先授權 預先授權完成 不能人工輸入 */
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_038_BELLAVITA_, _CUSTOMER_INDICATOR_SIZE_))
					{
						pobTran->inErrorMsg = _ERROR_CODE_V3_FUNC_CLOSE_;
						inRetVal = VS_ERROR;
					}
					else
					{
						/* 請依正確卡別操作 */
						DISPLAY_OBJECT	srDispMsgObj;
						memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
						strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_FOLLOW_RIGHT_OPT_);
						srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
						srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
						srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
						strcpy(srDispMsgObj.szErrMsg1, "");
						srDispMsgObj.inErrMsg1Line = 0;
						srDispMsgObj.inBeepTimes = 1;
						srDispMsgObj.inBeepInterval = 0;
						inRetVal = inDISP_Msg_BMP(&srDispMsgObj);
					}

					return (inRetVal);
				}
			}
			/* 有開人工輸入 */
			else
			{
				
			}
						
			/* 表示是手動輸入 */
			pobTran->uszHGManualBit = VS_TRUE;

			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}

			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}

			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}

			/* 將收到的卡號複製到szHGPAN中 */
			strcpy(pobTran->srBRec.szHGPAN, pobTran->srBRec.szPAN);

			/* 第一步驟 判斷HG card bin 讀HDT */
			if (inCARD_GetBin_HG(pobTran) != VS_SUCCESS)
				return(VS_ERROR);

			/* 純HG不檢核 */
			if (memcmp(pobTran->srBRec.szHGCardLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
			{

			}
			else
			{
				/* 第二步驟檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					if (inCARD_ValidTrack2_HGPAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 第三步輸入有效期 */
				inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
				if (inRetVal != VS_SUCCESS)
					return (inRetVal);

				/* 第四步驟檢核ExpDate */
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			break;
                }
                /* 晶片卡事件 */
                else if (ginEventCode == _EMV_DO_EVENT_)
                {       
                        /* 在這裡做所有HG退貨插卡檢核 */
                        if (pobTran->inRunOperationID == _OPERATION_HG_REFUND_) 
                        {
                                return (VS_ERROR);
                        }
                        
                        /* 取得晶片卡資料 */
                        if (inEMV_GetEMVCardData(pobTran) != VS_SUCCESS)
                                return (VS_ERROR);
			
			/* 目前只有點數查詢需要先取卡號，此流程可以取代inEMV_Process中分析卡號的那段，但暫時不全面更換 */
			if (inEMV_GetCardNoFlow(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 因HG混合交易如果HG非手輸但信用卡手輸 帳單匯印HG卡的持卡人姓名*/
			memset(pobTran->srBRec.szCardHolder, 0x00, sizeof(pobTran->srBRec.szCardHolder));

                        /* 將收到的卡號複製到szHGPAN中 */
                        strcpy(pobTran->srBRec.szHGPAN, pobTran->srBRec.szPAN);
			
			/* 第三步驟 判斷HG card bin */
                        if (inCARD_GetBin_HG(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);
			
			/* 純HG不檢核 */
			if (memcmp(pobTran->srBRec.szHGCardLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
			{

			}
			else
			{
				/* 第二步驟檢核PAN module 10 */
				memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
				inGetModule10Check(szFuncEnable);
				if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
				{
					/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
					if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
						return (VS_ERROR);
				}

				/* 第四步驟檢核ExpDate */
				if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}
			
			break;
                }
     
                while (1)
                {
                        ginEventCode = -1;
                        /* ------------偵測刷卡------------------*/
                        inMSR_RetVal = inCARD_MSREvent();
                        if (inMSR_RetVal == VS_SUCCESS)
                        {
                                /* 刷卡事件 */
                                ginEventCode = _SWIPE_EVENT_;
                        }
                        /* 回復錯誤訊息蓋掉的圖 */
                        else if (inMSR_RetVal == VS_SWIPE_ERROR)
                        {
                                inFunc_ResetTitle(pobTran);
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 1000;
			
                                inDISP_Msg_BMP(&srDispMsgObj);
                                
                                /* Mirror Message */
                                if (pobTran->uszECRBit == VS_TRUE)
                                {
                                        inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
                                }

                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                if (pobTran->inRunOperationID == _OPERATION_HG_REFUND_)
                                {
                                        /* 顯示請刷HG卡 */
                                        inDISP_PutGraphic(_GET_HG_CARD_, 0, _COORDINATE_Y_LINE_8_4_);
                                }
                                else 
                                {
                                        /* 顯示請刷或插HG卡 */
                                        inDISP_PutGraphic(_GET_HG_CARD_ICC_, 0, _COORDINATE_Y_LINE_8_4_);
                                }
                        }

                        /* ------------偵測晶片卡---------------- */
                        inEMV_RetVal = inEMV_ICCEvent();
                        if (inEMV_RetVal == VS_SUCCESS)
                        {
                                /* 晶片卡事件 */
                                ginEventCode = _EMV_DO_EVENT_;
                        }
                        
                        /* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			else if ((szKey >= '0' && szKey <= '9') || (szKey == _KEY_ENTER_))
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				/* 客製化005，只有退貨支援Menu keyin */
				else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* key in事件 */
					ginEventCode = _MENUKEYIN_EVENT_;
				}
			}
                        
                        /* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
                }
        }
	
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCreditCardFields_HG
Date&Time       :2017/1/20 下午 07:14
Describe        :HG交易時取得信用卡資訊
*/
int inFunc_GetCreditCardFields_HG(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal;
        char    szCTLSEnable[2 + 1];		/* 觸控是否打開 */
        char	szTMSOK[2 + 1];
	char	szFuncEnable[2 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCreditCardFields_HG START!");
        
        if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "inFunc_GetCreditCardFields_HG START");
        }
        
        if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
	{
                if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_ || pobTran->srBRec.lnHGPaymentType == _HG_PAY_CUP_)
                {
			/* 不是選HGI，需要重置信用卡狀態 */
			pobTran->srBRec.inChipStatus = 0;
			
                        
			if (inCREDIT_Func_GetAmount(pobTran) != VS_SUCCESS)			// 輸入金額
				return (VS_ERROR);

			if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
			{
				if (inHG_Func_GetPoint(pobTran) != VS_SUCCESS)			// 輸入使用點數
					return (VS_ERROR);
			}
			
			if (inCREDIT_Func_GetProductCode(pobTran) != VS_SUCCESS)		// 輸入櫃號
				return (VS_ERROR);

			if (inCREDIT_Func_GetStoreID(pobTran) != VS_SUCCESS)			// 輸入櫃號
				return (VS_ERROR);

                        /* 如果lnHGPaymentType不是_HG_PAY_CREDIT_INSIDE_，則清除信用卡號 */
                        memset(pobTran->srBRec.szPAN, 0x00, sizeof(pobTran->srBRec.szPAN));
                        ginEventCode = -1;
                        
                        memset(szTMSOK, 0x00, sizeof(szTMSOK));
                        inGetTMSOK(szTMSOK);
                        
                        memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
                        inGetContactlessEnable(szCTLSEnable);                       
                        
                        if (!memcmp(szCTLSEnable, "Y", 1) && !memcmp(szTMSOK, "Y", 1))
                        {
                                inRetVal = inFunc_GetCardFields_CTLS(pobTran);
                        }
                        else
                        {       
                                inRetVal = inFunc_GetCardFields_ICC(pobTran);
                        }
                        
                        if (inRetVal == VS_SUCCESS) // 插卡、刷卡、感應
                        {
                                pobTran->srBRec.inHGCreditHostIndex = pobTran->srBRec.inHDTIndex;
                        }
                        else
                        {
                                return (VS_ERROR);
                        }
		
                }
		/* HGI不過卡，但仍要獲得卡片資訊 */
		else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_)
		{
			/* 第二步驟unPackTrack資料 */
			if (pobTran->uszHGManualBit == VS_TRUE)
			{
				pobTran->srBRec.uszManualBit = VS_TRUE;
			}
			else
			{
				if (inCARD_unPackCard(pobTran) != VS_SUCCESS)
				{
					/* Mirror Message */
					if (pobTran->uszECRBit == VS_TRUE)
					{
						inECR_SendMirror(pobTran, _MIRROR_MSG_GET_CARD_RETRY_);
					}

					return (VS_ERROR);
				}
			}

			/* 第三步驟 判斷card bin 讀HDT */
			if (inCARD_GetBin(pobTran) != VS_SUCCESS)
				return(VS_ERROR);

			/* 第四步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

			/* 第六步驟檢核SeviceCode */
			/* 如果過HG不是是用插卡，而且用HGI就需要檢核SeviceCode (避免晶片卡用刷卡過交易) */
			if (pobTran->srBRec.inChipStatus != _EMV_CARD_)
			{
				if (inCheckFallbackFlag(pobTran) != VS_SUCCESS)
				{
					pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
					return (VS_ERROR);
				}
			}

			/* 第七步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
		}
	
        }
                
        if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "inFunc_GetCreditCardFields_HG END");
        }
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetCardFields_MailOrder
Date&Time       :2017/6/7 下午 6:22
Describe        :
*/
int inFunc_GetCardFields_MailOrder(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
        char	szKey = 0;
	char	szFuncEnable[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szTemplate[3 + 1] = {0};
	long	lnTimeout = 0;

	vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetCardFields_MailOrder START!");
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        /* 過卡方式參數初始化  */
        pobTran->srBRec.uszManualBit = VS_FALSE;

        /* 如果ginEventCode為數字鍵流程即為IDLE MenuKeyIn */
        if (ginEventCode >= '0' && ginEventCode <= '9')
        {
                /* 表示是idle手動輸入，這邊才on flag是因為有可能idle輸入金額 */
                ginMenuKeyIn = VS_TRUE;
        }

	/* 設定Timeout */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_107_BUMPER_GET_CARD_TIMEOUT_;
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
	{
		lnTimeout = _CUSTOMER_005_FPG_GET_CARD_TIMEOUT_;
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE)
		{
			lnTimeout = _ECR_RS232_GET_CARD_TIMEOUT_;
		}
		else
		{
			lnTimeout = 30;
		}
	}
	
	inDISP_Timer_Start(_TIMER_GET_CARD_, lnTimeout);
	
        while (1)
        {
                
                /* idle手動輸入或一般輸入 */
                if (ginMenuKeyIn == VS_TRUE || ginEventCode == _MENUKEYIN_EVENT_)
                {
			/* 表示是手動輸入 */
                        pobTran->srBRec.uszManualBit = VS_TRUE;
			
			/* 若啟動FALLBACK，而且下一次不是刷卡事件，關FALLBACK */
			if (ginFallback == VS_TRUE)
			{
				inEMV_SetICCReadFailure(VS_FALSE);		/* 關閉FALL BACK */
			}
			
			/* V3 Enter key 代碼為 'A' 所以要限制 */
			if (szKey >= '0' && szKey <= '9')
			{
				pobTran->inMenuKeyin = (int)szKey;
			}
			
			/* 人工輸入卡號開關，由程式預設支援人工輸入卡號功能 */
			
			inRetVal = inCREDIT_Func_Get_Card_Number(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			if (inCARD_Generate_Special_Card(pobTran->srBRec.szPAN) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			
                        /* 第一步驟 判斷card bin 讀HDT */
                        if (inCARD_GetBin(pobTran) != VS_SUCCESS)
                                return(VS_ERROR);
			
			/* 第二步驟檢核PAN module 10 */
			memset(szFuncEnable, 0x00, sizeof(szFuncEnable));
			inGetModule10Check(szFuncEnable);
			if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
			{
				if (inCARD_ValidTrack2_PAN(pobTran) != VS_SUCCESS)
					return (VS_ERROR);
			}

                        /* 第三步有效期 */
                        inRetVal = inCREDIT_Func_Get_Exp_Date(pobTran);
			if (inRetVal != VS_SUCCESS)
				return (inRetVal);

                        /* 第四步驟檢核ExpDate */
			if (inCARD_ValidTrack2_ExpDate(pobTran) != VS_SUCCESS)
				return (VS_ERROR);
			
			/* 決定TRT */
			inRetVal = inNCCC_Func_Decide_MEG_TRT(pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}

                        break;
                }
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 客製化123，過卡時設為較最亮 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
		}
			
		while (1)
		{
			ginEventCode = -1;
			
			
			/* ------------偵測key in------------------ */
			szKey = -1;
			szKey = uszKBD_Key();

			/* 檢查TIMEOUT */
			if (inTimerGet(_TIMER_GET_CARD_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_TIMEOUT_)
			{
				/* Timeout */
				return (VS_TIMEOUT);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				/* (需求單 - 107276)自助交易標準做法 鎖按鍵 by Russell 2019/3/21 下午 4:57 */
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
				{
					continue;
				}
				else
				{
					/* Cancel */
					return (VS_USER_CANCEL);
				}
			}
			
			/* 客製化005，只有退貨支援Menu keyin */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_))
			{
				continue;
			}
			else
			{
				/* key in事件 */
				ginEventCode = _MENUKEYIN_EVENT_;
			}

			/* 有事件發生，跳出迴圈做對應反應 */
			if (ginEventCode != -1)
			{
				break;
			}
			
		}/* while (1) 偵測事件迴圈...*/

	}/* while (1) 對事件做回應迴圈...*/
        
	/* 客製化123，過卡完時設為較暗 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetScreenBrightness(szTemplate);
		if (strlen(szTemplate) > 0)
		{
			inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szTemplate));
		}
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintReceipt_ByBuffer_Flow
Date&Time       :2017/3/22 上午 11:07
Describe        :印帳單分流
*/
int inFunc_PrintReceipt_ByBuffer_Flow(TRANSACTION_OBJECT *pobTran)
{
	char		szESCMode[2 + 1] = {0};
	char		szTRTFileName[16 + 1] = {0};
	char		szTxnType[20 + 1] = {0};
	char		szFESMode[2 + 1] = {0};
	char		szDispBuf[50 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	
       vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintReceipt_ByBuffer_Flow START!");
 
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_PrintReceipt_ByBuffer_Flow() START !");
		inLogPrintf(AT, "szAgreeEsgBill[%s]", pobTran->szAgreeEsgBill);
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_WatchAll();
	}
	
	   
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		/* (需求單 - 107227)邦柏科技自助作業客製化 
		* 收銀機發動一般信用卡交易，EDC不提示使用者確認流程及畫面(EX：請按確認鍵、請核對持卡人簽名等流程)。
		* by Russell 2018/12/8 下午 8:42 */
		/* (需求單 - 107276)自助交易標準400做法 EDC不提示使用者確認流程及畫面 by Russell 2018/12/27 上午 11:20 */
	       if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                   !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	       {
		       memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		       inGetTRTFileName(szTRTFileName);

		       if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		       {
			       inNCCC_Ticket_107_Bumper_Display_Transaction_Result(pobTran);
		       }
		       else
		       {
			       inFunc_Display_Msg_Instead_For_Print_Receipt(pobTran);
		       }
	       }
	       else
	       {
		       memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		       inGetTRTFileName(szTRTFileName);

		       if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		       {
			       inNCCC_Ticket_107_Bumper_Display_Transaction_Result(pobTran);
		       }
		       else
		       {
			       inFunc_Display_Msg_Instead_For_Print_Receipt(pobTran);
		       }
	       }
	}
	else
	{
		/* (需求單 - 107227)邦柏科技自助作業客製化 
		* 收銀機發動一般信用卡交易，EDC不提示使用者確認流程及畫面(EX：請按確認鍵、請核對持卡人簽名等流程)。
		* by Russell 2018/12/8 下午 8:42 */
		/* (需求單 - 107276)自助交易標準400做法 EDC不提示使用者確認流程及畫面 by Russell 2018/12/27 上午 11:20 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)         ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
		{
		       memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		       inGetTRTFileName(szTRTFileName);

		       if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		       {
			       inNCCC_Ticket_107_Bumper_Display_Transaction_Result(pobTran);
		       }
		       else
		       {
			       inFunc_Display_Msg_Instead_For_Print_Receipt(pobTran);
		       }
	       }
	       else
	       {
			/* MPAS ECR回傳成功且MP為Y，不印簽單 */
			memset(szFESMode, 0x00, sizeof(szFESMode));
			inGetNCCCFESMode(szFESMode);
			if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
			{
                                /* 客製化098，電票列印簽單規則不同 */
                                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)    ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                                {
                                        if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                                        {
                                                /* 票證無論MPAS的簽單Flag 一律走下面出簽單 */
                                                if (pobTran->srTRec.uszMPASReprintBit == VS_TRUE)
                                                {
                                                        /* 退貨交易、加值取消交易：EDC一律印出簽帳單 */
                                                }
                                                else
                                                {
                                                        /* 【需求單 - 107226】	小額收銀機連線版-支援優惠平台需求 by Russell 2019/10/7 下午 2:01 */
                                                        if (pobTran->srTRec.uszRewardL1Bit == VS_TRUE	|| 
                                                            pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
                                                            pobTran->srTRec.uszRewardL5Bit == VS_TRUE)
                                                        {
                                                                inCREDIT_PRINT_MPAS_RewardAdvertisement(pobTran);
                                                        }
                                                        inNCCC_Ticket_098_Mcdonalds_Display_Transaction_Result(pobTran);

                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintReceipt_ByBuffer_Flow END!");
                                                        
                                                        return (VS_SUCCESS);
                                                }
                                        }
                                        else
                                        {
                                                /* 優惠兌換不受Table ID:MP回多少來判斷， 但是收銀機未回傳仍要印簽單 */
                                                if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_	&&
                                                    pobTran->srBRec.uszMPASReprintBit == VS_FALSE)
                                                {
                                                        /* 回傳ECR成功 秀兌換完成 */
                                                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                                        memset(szDispBuf, 0x00, sizeof(szDispBuf));
                                                        sprintf(szDispBuf, "兌換成功 ");
                                                        inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);
                                                        inDISP_Wait(3000);
                                                        
                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintReceipt_ByBuffer_Flow END!");

                                                        return (VS_SUCCESS);
                                                }
                                                else if (pobTran->srBRec.uszMPASReprintBit == VS_TRUE		||
                                                         pobTran->srBRec.uszMPASTransBit != VS_TRUE		||
                                                         pobTran->srBRec.uszVOIDBit == VS_TRUE			||
                                                         pobTran->srBRec.inCode == _REDEEM_REFUND_		||
                                                         pobTran->srBRec.inCode == _INST_REFUND_		||
                                                         pobTran->srBRec.inCode == _CUP_REFUND_			||
                                                         pobTran->srBRec.inCode == _REFUND_			||
                                                         pobTran->srBRec.inCode == _FISC_REFUND_		||
                                                         pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_	||
                                                         pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
                                                {
                                                        /* 取消交易、退貨交易：EDC一律印出簽帳單 */
                                                }
                                                else
                                                {
                                                        /* 【需求單 - 107226】	小額收銀機連線版-支援優惠平台需求 by Russell 2019/10/7 下午 2:01 */
                                                        if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
                                                            pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
                                                            pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
                                                        {
                                                                inCREDIT_PRINT_MPAS_RewardAdvertisement(pobTran);
                                                        }

                                                        /* 避免優惠兌換出現"交易已完成" */
                                                        if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_	||
                                                            pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
                                                        {

                                                        }
                                                        else
                                                        {
                                                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                                                
                                                                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                                                                {
                                                                        /* SmartPay交易要提示調閱編號FiscRRN */
                                                                        inDISP_PutGraphic(_CHECK_SIGNATURE_7_, 0, _COORDINATE_Y_LINE_8_4_);

                                                                        memset(szDispBuf, 0x00, sizeof(szDispBuf));
                                                                        sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
                                                                        inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
                                                                }
                                                                else
                                                                {
                                                                        /* 交易已完成 授權碼 */
                                                                        inDISP_PutGraphic(_MSG_MP_COMPLETE_, 0, _COORDINATE_Y_LINE_8_5_);
                                                                        memset(szDispBuf, 0x00, sizeof(szDispBuf));
                                                                        sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
                                                                        inDISP_EnglishFont_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                                                                }

                                                                inDISP_Wait(3000);
                                                        }

                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintReceipt_ByBuffer_Flow END!");
                                                        
                                                        return (VS_SUCCESS);
                                                }
                                        }
                                }
                                else
                                {
                                        if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                                        {
                                                /* 票證無論MPAS的簽單Flag 一律走下面出簽單 */
                                                if (pobTran->srTRec.uszMPASReprintBit == VS_TRUE		||
                                                    pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_		||
                                                    pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_		||
                                                    pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_		||
                                                    pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_	||
                                                    pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_	||
                                                    pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
                                                {
                                                        /* 退貨交易、加值取消交易：EDC一律印出簽帳單 */
                                                }
                                                else
                                                {
                                                        /* 【需求單 - 107226】	小額收銀機連線版-支援優惠平台需求 by Russell 2019/10/7 下午 2:01 */
                                                        if (pobTran->srTRec.uszRewardL1Bit == VS_TRUE	|| 
                                                            pobTran->srTRec.uszRewardL2Bit == VS_TRUE	||
                                                            pobTran->srTRec.uszRewardL5Bit == VS_TRUE)
                                                        {
                                                                inCREDIT_PRINT_MPAS_RewardAdvertisement(pobTran);
                                                        }
                                                        inNCCC_Ticket_107_Bumper_Display_Transaction_Result(pobTran);

                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintReceipt_ByBuffer_Flow END!");
                                                        
                                                        return (VS_SUCCESS);
                                                }
                                        }
                                        else
                                        {
                                                /* 優惠兌換不受Table ID:MP回多少來判斷， 但是收銀機未回傳仍要印簽單 */
                                                if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_	&&
                                                    pobTran->srBRec.uszMPASReprintBit == VS_FALSE)
                                                {
                                                        /* 回傳ECR成功 秀兌換完成 */
                                                        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                                        memset(szDispBuf, 0x00, sizeof(szDispBuf));
                                                        sprintf(szDispBuf, "兌換成功 ");
                                                        inDISP_ChineseFont_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);
                                                        inDISP_Wait(3000);
                                                        
                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintReceipt_ByBuffer_Flow END!");

                                                        return (VS_SUCCESS);
                                                }
                                                else if (pobTran->srBRec.uszMPASReprintBit == VS_TRUE		||
                                                         pobTran->srBRec.uszMPASTransBit != VS_TRUE		||
                                                         pobTran->srBRec.uszVOIDBit == VS_TRUE			||
                                                         pobTran->srBRec.inCode == _REDEEM_REFUND_		||
                                                         pobTran->srBRec.inCode == _INST_REFUND_		||
                                                         pobTran->srBRec.inCode == _CUP_REFUND_			||
                                                         pobTran->srBRec.inCode == _REFUND_			||
                                                         pobTran->srBRec.inCode == _FISC_REFUND_		||
                                                         pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_	||
                                                         pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
                                                {
                                                        /* 取消交易、退貨交易：EDC一律印出簽帳單 */
                                                }
                                                else
                                                {
                                                        /* 【需求單 - 107226】	小額收銀機連線版-支援優惠平台需求 by Russell 2019/10/7 下午 2:01 */
                                                        if (pobTran->srBRec.uszRewardL1Bit == VS_TRUE	|| 
                                                            pobTran->srBRec.uszRewardL2Bit == VS_TRUE	||
                                                            pobTran->srBRec.uszRewardL5Bit == VS_TRUE)
                                                        {
                                                                inCREDIT_PRINT_MPAS_RewardAdvertisement(pobTran);
                                                        }

                                                        /* 避免優惠兌換出現"交易已完成" */
                                                        if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_	||
                                                            pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
                                                        {

                                                        }
                                                        else
                                                        {
                                                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                                                
                                                                if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                                                                {
                                                                        /* SmartPay交易要提示調閱編號FiscRRN */
                                                                        inDISP_PutGraphic(_CHECK_SIGNATURE_7_, 0, _COORDINATE_Y_LINE_8_4_);

                                                                        memset(szDispBuf, 0x00, sizeof(szDispBuf));
                                                                        sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
                                                                        inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
                                                                }
                                                                else
                                                                {
                                                                        /* 交易已完成 授權碼 */
                                                                        inDISP_PutGraphic(_MSG_MP_COMPLETE_, 0, _COORDINATE_Y_LINE_8_5_);
                                                                        memset(szDispBuf, 0x00, sizeof(szDispBuf));
                                                                        sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
                                                                        inDISP_EnglishFont_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_, _DISP_RIGHT_);
                                                                }

                                                                inDISP_Wait(3000);
                                                        }

                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintReceipt_ByBuffer_Flow END!");
                                                        
                                                        return (VS_SUCCESS);
                                                }
                                        }
                                }
			}


			memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
			inGetTRTFileName(szTRTFileName);

			if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0	||
			    memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
			{
				/* (6)	ESC功能僅包含中心NCCC Host(含HappyGo混合交易)及DCC Host之交易(不含大來卡) */
				/* ESC開關 */
				memset(szESCMode, 0x00, sizeof(szESCMode));
				inGetESCMode(szESCMode);

				/* 沒ESC的狀況，直接印紙本，不須另外判斷 */
				if (szESCMode[0] != 'Y')
				{
					/* 優惠兌換和優惠取消 重印只印一聯 */
					if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ || pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
					{

						inFunc_NCCC_PrintReceipt_ByBuffer_Loyalty_Redeem(pobTran);

					}
					else
					{
						inFunc_NCCC_PrintReceipt_ByBuffer(pobTran);
					}

				}
				/* 有ESC的狀況 */
				else
				{
					/* 優惠兌換和優惠取消不支援ESC */
					if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ || pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
					{

						inFunc_NCCC_PrintReceipt_ByBuffer_Loyalty_Redeem(pobTran);
					}
					else
					{
						inFunc_NCCC_PrintReceipt_ByBuffer_ESC(pobTran);
					}

				}

			}
			/* 純HG交易，混合交易看NCCC */
			else if (memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
			{
				if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
				{
					/* 紅利積點現金及禮券可以由【TMS】控制是否列印帳單 */
					if (pobTran->srBRec.inHGCode == _HG_REWARD_CASH_	||
					    pobTran->srBRec.inHGCode == _HG_REWARD_GIFT_PAPER_)
					{
						memset(szTxnType, 0x00, sizeof(szTxnType));
						inGetTransFunc(szTxnType);
						if (szTxnType[8] == 'Y')
						{
							inFunc_NCCC_PrintReceipt_ByBuffer_HappyGo_Single(pobTran);
						}
						else
						{

						}

					}
					else
					{
						inFunc_NCCC_PrintReceipt_ByBuffer_HappyGo_Single(pobTran);
					}
				}
				else
				{
					vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintReceipt_ByBuffer_Flow HG_Flow_error");
					return (VS_ERROR);
				}
			}
			/* 電票交易 */
			else if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
			{
				inFunc_NCCC_PrintReceipt_ByBuffer_ESVC(pobTran);
			}
                        /* 信託交易 */
			else if (memcmp(szTRTFileName, _TRT_FILE_NAME_TRUST_, strlen(_TRT_FILE_NAME_TRUST_)) == 0)
			{
				inFunc_PrintReceipt_ByBuffer_Trust(pobTran);
			}
			/* 其他Host，如DINERS */
			else
			{
				inFunc_PrintReceipt_ByBuffer(pobTran);
			}
	       }/* 客製化 */
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_PrintReceipt_ByBuffer_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintReceipt_ByBuffer_Flow END!");
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintReceipt_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :標準，For Diners使用
*/
int inFunc_PrintReceipt_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;
	
	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);
	
	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 列印商店聯 */
	/* 客製化123不印商店聯且印簽單要提示音 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BEEP(1, 0);
	}
	else if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(pobTran) == VS_TRUE)
	{
			/* TMS參數Print_Mode 4=兩聯免簽不印商店聯 */
	}
	else
	{
		/* 簽單模式不是0就印商店聯 */
		if (memcmp(szPrtMode, "0", strlen("0")) != 0)
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			while (1)
			{
				pobTran->srBRec.inPrintOption = _PRT_MERCH_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					/* 成功就砍簽名圖檔 */
					inFunc_Delete_Signature(pobTran);

					break;
				}
			}
		}

		/* 三聯式簽單，中間要多印一聯客戶聯 */
		if (memcmp(szPrtMode, "3", strlen("3")) == 0)
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			/* 列印商店存根聯 */
			while (1)
			{
				pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
					break;
			}
		}

		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("交易已完成", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont("請列印第二聯", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			/* 請按確認或清除鍵 */
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 20251215_BUG_MDF][UI] 新增銀聯提示用顯示畫面 */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* 顯示請按確認及清除鍵 */
				inDISP_PutGraphic(_TR_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_5_);
				inFunc_DisplayCupWarning(pobTran);
			}else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_) ||
					  !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 041、043不顯示免簽名 */
				/* 提示檢核簽名和授權碼 */
				inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
			}
			else
			{
				if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
				{
					/* 提示免簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
					/* [113110-信託需求][UI] 修改列印顯提示圖,不再提示持卡人核對簽名字樣 2025/11/27 */
					if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
					{
						/* 顯示授權碼 */
						inDISP_PutGraphic(_TR_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
					}else
					{
						/* 提示檢核簽名和授權碼 */
						inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
					}
				}
			}
			/* [113110-信託需求][UI] 信託交易,不用顯示授權碼 2025/11/27 */
			if (pobTran->srTrustRec.uszTrustTransBit != VS_TRUE)
			{
				memset(szDispBuf, 0x00, sizeof(szDispBuf));
				sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
				inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
			}
		}

		/* 清除keyboad Buffer */
		inFlushKBDBuffer();

		/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
		/* 客製化107顯示訊息TimeOut 3秒 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		}
		/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
		}

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
			{
				inDISP_BEEP(1, 0);
				inDISP_Wait(250);
			}

			if (uszKey == _KEY_ENTER_			|| 
			    uszKey == _KEY_TIMEOUT_			||
			    inChoice == _CUSTReceipt_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_CANCEL_			||
				 inChoice == _CUSTReceipt_Touch_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		/* 因為這裡交易已完成，所以一定回傳成功*/
		if (inRetVal == VS_USER_CANCEL)
		{
			return (VS_SUCCESS);
		}
	}
	
        /* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0    ||
            !memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, strlen(_PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_)))
	{
		
		if(inFunc_PrintEsgCustReceipt(pobTran) != VS_TRUE)
		{		
			while (1)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 20251215_BUG_MDF][UI] 新增銀聯提示用顯示畫面 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inFunc_DisplayCupWarning(pobTran);
					/* [20251219_BUG_MDF][UI] 因列印時需顯示"列印帳單中"新增圖檔顯示,位置有異動需依照需求調整 */
					/* 列印帳單中 */
					inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_8_);

				}else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_) ||
						  !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
				{
					/* 041、043不顯示免簽名 */
					/* 提示檢核簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
					if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
					{
						/* 提示免簽名和授權碼 */
						inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
					}
					else
					{
						/* [113110-信託需求][UI] 修改列印顯提示圖,不再提示持卡人核對簽名字樣 2025/11/27 */
						if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
						{
							/* 顯示授權碼 */
							inDISP_PutGraphic(_TR_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
						}else
						{
							/* 提示檢核簽名和授權碼 */
							inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
						}
					}
				}
				/* [113110-信託需求][UI] 信託交易,不用顯示授權碼 2025/11/27 */
				if (pobTran->srTrustRec.uszTrustTransBit != VS_TRUE)
				{
					memset(szDispBuf, 0x00, sizeof(szDispBuf));
					sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
					inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
				}
				pobTran->srBRec.inPrintOption = _PRT_CUST_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
					inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
					break;
			}
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_PrintReceipt_ByBuffer_Loyalty_Redeem
Date&Time       :2017/2/21 下午 5:01
Describe        :優惠兌換專用
*/
int inFunc_NCCC_PrintReceipt_ByBuffer_Loyalty_Redeem(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szPrtMode[2 + 1];
	
	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);
	
	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
	
	/* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
	if (memcmp(szPrtMode, "0", strlen("0")) != 0)
	{
		/* 列印帳單中 */
		inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
		
		while (1)
		{
			pobTran->srBRec.inPrintOption = _PRT_MERCH_;
			inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				/* 成功就砍簽名圖檔 */
				inFunc_Delete_Signature(pobTran);

				break;
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_PrintReceipt_ByBuffer
Date&Time       :2017/3/22 下午 1:46
Describe        :NCCC專用
*/
int inFunc_NCCC_PrintReceipt_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_NCCC_PrintReceipt_ByBuffer() START !");
	}
	
	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);
	
	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 列印商店聯 */
	/* 客製化123不印商店聯且印簽單要提示音 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BEEP(1, 0);
	}
	else if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(pobTran) == VS_TRUE)
	{
			/* TMS參數Print_Mode 4=兩聯免簽不印商店聯 */
	}
	else 
	{
		/* 簽單模式不是0就印商店聯 */
		if (memcmp(szPrtMode, "0", strlen("0")) != 0)
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			while (1)
			{
				pobTran->srBRec.inPrintOption = _PRT_MERCH_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					/* 成功就砍簽名圖檔 */
					inFunc_Delete_Signature(pobTran);

					break;
				}

			}
		}

		/* 三聯式簽單，中間要多印一聯客戶聯 */
		if (memcmp(szPrtMode, "3", strlen("3")) == 0)
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			/* 列印商店存根聯 */
			while (1)
			{
				pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					break;
				}
			}
		}

		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("交易已完成", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont("請列印第二聯", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			/* 請按確認或清除鍵 */
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 20251215_BUG_MDF][UI] 新增銀聯提示用顯示畫面 */
			if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
			{
				/* 顯示請按確認及清除鍵 */
				inDISP_PutGraphic(_TR_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_5_);
				inFunc_DisplayCupWarning(pobTran);
				
			}else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
			{
				/* SmartPay交易要提示調閱編號FiscRRN */
				inDISP_PutGraphic(_CHECK_SIGNATURE_3_, 0, _COORDINATE_Y_LINE_8_4_);

				memset(szDispBuf, 0x00, sizeof(szDispBuf));
				sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
				inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
			}
			else
			{
				if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_) ||
				    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
				{
					/* 041、043不顯示免簽名 */
					/* 提示檢核簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
					if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
					{
						/* 提示免簽名和授權碼 */
						inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
					}
					else
					{
						/* [113110-信託需求][UI] 修改列印顯提示圖,不再提示持卡人核對簽名字樣 2025/11/27 */
						if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
						{
							/* 顯示授權碼 */
							inDISP_PutGraphic(_TR_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
						}else
						{
							/* 提示檢核簽名和授權碼 */
							inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
						}
					}
				}
				/* [113110-信託需求][UI] 信託交易,不用顯示授權碼 2025/11/27 */
				if (pobTran->srTrustRec.uszTrustTransBit != VS_TRUE)
				{
					memset(szDispBuf, 0x00, sizeof(szDispBuf));
					sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
					inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
				}
			}
		}

		/* 清除keyboad Buffer */
		inFlushKBDBuffer();

		/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
		/* 客製化107顯示訊息TimeOut 2秒 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		}
		/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
		}
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
			{
				inDISP_BEEP(1, 0);
				inDISP_Wait(250);
			}

			if (uszKey == _KEY_ENTER_			|| 
			    uszKey == _KEY_TIMEOUT_			||
			    inChoice == _CUSTReceipt_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_CANCEL_			||
				 inChoice == _CUSTReceipt_Touch_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		/* 因為這裡交易已完成，所以一定回傳成功*/
		if (inRetVal == VS_USER_CANCEL)
		{
			return (VS_SUCCESS);
		}
	}

        /* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
		memcmp(szPrtMode, "3", strlen("3")) == 0	||
		!memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, strlen(_PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_)))
	{
		if(inFunc_PrintEsgCustReceipt(pobTran) != VS_TRUE)
		{
			while (1)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 20251215_BUG_MDF][UI] 新增銀聯提示用顯示畫面 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inFunc_DisplayCupWarning(pobTran);
					/* [20251219_BUG_MDF][UI] 因列印時需顯示"列印帳單中"新增圖檔顯示,位置有異動需依照需求調整 */
					/* 列印帳單中 */
					inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_8_);
				}else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					/* SmartPay交易要提示調閱編號FiscRRN */
					inDISP_PutGraphic(_CHECK_SIGNATURE_4_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispBuf, 0x00, sizeof(szDispBuf));
					sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
					inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
				}
				else
				{
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_) ||
						!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
					{
						/* 041、043不顯示免簽名 */
						/* 提示檢核簽名和授權碼 */
						inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
					}
					else
					{
						if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
						{
							/* 提示免簽名和授權碼 */
							inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
						}
						else
						{
							/* [113110-信託需求][UI] 修改列印顯提示圖,不再提示持卡人核對簽名字樣 2025/11/27 */
							if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
							{
								/* 顯示授權碼 */
								inDISP_PutGraphic(_TR_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
							}else
							{
								/* 提示檢核簽名和授權碼 */
								inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
							}
						}
					}
					/* [113110-信託需求][UI] 信託交易,不用顯示授權碼 2025/11/27 */
					if (pobTran->srTrustRec.uszTrustTransBit != VS_TRUE)
					{
						memset(szDispBuf, 0x00, sizeof(szDispBuf));
						sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
						inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
					}
				}

				pobTran->srBRec.inPrintOption = _PRT_CUST_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
					inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					break;
				}
			}
		}
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_NCCC_PrintReceipt_ByBuffer() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_PrintReceipt_ByBuffer_ESC
Date&Time       :2017/3/22 下午 1:45
Describe        :有ESC判斷
*/
int inFunc_NCCC_PrintReceipt_ByBuffer_ESC(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1];
	char		szPrtMode[2 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_NCCC_PrintReceipt_ByBuffer_ESC() START !");
	}
	
	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);
	
	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 客製化123不印商店聯且印簽單要提示音 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BEEP(1, 0);
	}
        else if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(pobTran) == VS_TRUE)
        {
                /* TMS參數Print_Mode 4=兩聯免簽不印商店聯 */
        }
	else 
	{
		if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_NOT_SUPPORTED_	||
		    pobTran->srBRec.inESCUploadMode == _ESC_STATUS_OVERLIMIT_		||
		   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_		&&
		    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_)			||
		    inNCCC_ESC_MerchantCopy_Check(pobTran) == VS_SUCCESS)
		{
			/* 列印商店聯 */
			/* 簽單模式不是0就印商店聯 */
			if (memcmp(szPrtMode, "0", strlen("0")) != 0)
			{
				/* 列印帳單中 */
				inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

				while (1)
				{
					pobTran->srBRec.inPrintOption = _PRT_MERCH_;
					inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal == VS_PRINTER_PAPER_OUT ||
					    inRetVal == VS_PRINTER_OVER_HEAT)
						continue;
					else
					{
						/* 如果不是因為加印(之後還要上傳ESC用)才出紙本，印完紙本就砍簽名圖檔 */
						if (inNCCC_ESC_MerchantCopy_Check(pobTran) != VS_SUCCESS)
						{
							inFunc_Delete_Signature(pobTran);
						}

						break;
					}

				}
			}

			if (inNCCC_ESC_MerchantCopy_Check(pobTran) == VS_SUCCESS	||
			   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
			    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_))
			{
				/* 三聯式簽單，中間要多印一聯客戶聯 */
				if (memcmp(szPrtMode, "3", strlen("3")) == 0)
				{
					/* 列印帳單中 */
					inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

					/* 列印商店存根聯 */
					while (1)
					{
						pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
						inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

						/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
						if (inRetVal == VS_PRINTER_PAPER_OUT ||
						    inRetVal == VS_PRINTER_OVER_HEAT)
							    continue;
						else
						{
							break;
						}
					}
				}
			}

			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("交易已完成", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
				inDISP_ChineseFont("請列印第二聯", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
				/* 請按確認或清除鍵 */
				inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
			}
			else
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				
				/* 20251215_BUG_MDF][UI] 新增銀聯提示用顯示畫面 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					/* 顯示請按確認及清除鍵 */
					inDISP_PutGraphic(_TR_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_5_);
					inFunc_DisplayCupWarning(pobTran);
				}else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					/* SmartPay交易要提示調閱編號FiscRRN */
					inDISP_PutGraphic(_CHECK_SIGNATURE_3_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispBuf, 0x00, sizeof(szDispBuf));
					sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
					inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
				}
				else
				{
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_) ||
						!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
					{
						/* 041、043不顯示免簽名 */
						/* 提示檢核簽名和授權碼 */
						inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
					}
					else
					{
						if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
						{
							/* 提示免簽名和授權碼 */
							inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
						}
						else
						{
							/* [113110-信託需求][UI] 修改列印顯提示圖,不再提示持卡人核對簽名字樣 2025/11/27 */
							if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
							{
								/* 顯示授權碼 */
								inDISP_PutGraphic(_TR_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
							}else
							{
								/* 提示檢核簽名和授權碼 */
								inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
							}
						}
					}

					/* [113110-信託需求][UI] 信託交易,不用顯示授權碼 2025/11/27 */
					if (pobTran->srTrustRec.uszTrustTransBit != VS_TRUE)
					{
						memset(szDispBuf, 0x00, sizeof(szDispBuf));
						sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
						inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
					}
				}
			}

			/* 清除keyboad Buffer */
			inFlushKBDBuffer();

			/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
			/* 客製化107顯示訊息TimeOut 2秒 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
				inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
			}
			/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
				inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			}
			else
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
				inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
			}
			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				uszKey = uszKBD_Key();

				if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
				{
					inDISP_BEEP(1, 0);
					inDISP_Wait(250);
				}

				if (uszKey == _KEY_ENTER_			|| 
				    uszKey == _KEY_TIMEOUT_			||
				    inChoice == _CUSTReceipt_Touch_ENTER_)
				{
					inRetVal = VS_SUCCESS;
					break;
				}
				else if (uszKey == _KEY_CANCEL_			||
				     inChoice == _CUSTReceipt_Touch_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					break;
				}
			}
			/* 清空Touch資料 */
			inDisTouch_Flush_TouchFile();

			/* 因為這裡交易已完成，所以一定回傳成功*/
			if (inRetVal == VS_USER_CANCEL)
			{
				return (VS_SUCCESS);
			}
		}
	}
	
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
		memcmp(szPrtMode, "3", strlen("3")) == 0	||
		!memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, strlen(_PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_)))
	{
		if(inFunc_PrintEsgCustReceipt(pobTran) != VS_TRUE)
		{
			/* 列印顧客聯 */
			while (1)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 20251215_BUG_MDF][UI] 新增銀聯提示用顯示畫面 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inFunc_DisplayCupWarning(pobTran);
				}else if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					/* SmartPay交易要提示調閱編號FiscRRN */
					inDISP_PutGraphic(_CHECK_SIGNATURE_4_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispBuf, 0x00, sizeof(szDispBuf));
					sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
					inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
				}
				else
				{
					if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_) ||
						!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
					{
						/* 041、043不顯示免簽名 */
						/* 提示檢核簽名和授權碼 */
						inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
					}
					else
					{
						if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
						{
							/* 提示免簽名和授權碼 */
							inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
						}
						else
						{
							/* [113110-信託需求][UI] 修改列印顯提示圖,不再提示持卡人核對簽名字樣 2025/11/27 */
							if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
							{
								/* 顯示授權碼 */
								inDISP_PutGraphic(_TR_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
							}else
							{
								/* 提示檢核簽名和授權碼 */
								inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
							}
						}
					}
					/* [113110-信託需求][UI] 信託交易,不用顯示授權碼 2025/11/27 */
					if (pobTran->srTrustRec.uszTrustTransBit != VS_TRUE)
					{
						memset(szDispBuf, 0x00, sizeof(szDispBuf));
						sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
						inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
					}
				}

				/* 20251215_BUG_MDF][UI] 新增銀聯提示用顯示畫面 */
				if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
				{
					inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_8_);
				}else
				{
					inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);
				}

				pobTran->srBRec.inPrintOption = _PRT_CUST_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
					inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					break;
				}
			}
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_NCCC_PrintReceipt_ByBuffer_ESC() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintReceipt_ByBuffer_HappyGo_Single
Date&Time       :2017/3/22 上午 11:08
Describe        :
*/
int inFunc_NCCC_PrintReceipt_ByBuffer_HappyGo_Single(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;
	
	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);
	
	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	/* 列印商店聯 */
	/* 客製化123不印商店聯且印簽單要提示音 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BEEP(1, 0);
	}
        else if (inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(pobTran) == VS_TRUE)
        {
                /* TMS參數Print_Mode 4=兩聯免簽不印商店聯 */
        }
	else 
	{
		/* 簽單模式不是0就印商店聯 */
		if (memcmp(szPrtMode, "0", strlen("0")) != 0)
		{
			/* 列印帳單中 */
			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

			while (1)
			{
				pobTran->srBRec.inPrintOption = _PRT_MERCH_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					/* 成功就砍簽名圖檔 */
					inFunc_Delete_Signature(pobTran);

					break;
				}
			}
		}

		if (inNCCC_ESC_MerchantCopy_Check(pobTran) == VS_SUCCESS	||
		   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
		    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_))
		{
			/* 三聯式簽單，中間要多印一聯客戶聯 */
			if (memcmp(szPrtMode, "3", strlen("3")) == 0)
			{
				/* 列印帳單中 */
				inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

				/* 列印商店存根聯 */
				while (1)
				{
					pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
					inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal == VS_PRINTER_PAPER_OUT ||
					    inRetVal == VS_PRINTER_OVER_HEAT)
						continue;
					else
					{
						break;
					}
				}
			}
		}

		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("交易已完成", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
			inDISP_ChineseFont("請列印第二聯", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
			/* 請按確認或清除鍵 */
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
		}
		else
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			/* 提示列印客戶存根 */
			inDISP_PutGraphic(_CHECK_PRINT_CUST_RECEIPT_, 0, _COORDINATE_Y_LINE_8_4_);
		}

		/* 清除keyboad Buffer */
		inFlushKBDBuffer();

		/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
		/* 客製化107顯示訊息TimeOut 2秒 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
		}
		/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
		}
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			uszKey = uszKBD_Key();

			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
			{
				inDISP_BEEP(1, 0);
				inDISP_Wait(250);
			}

			if (uszKey == _KEY_ENTER_			|| 
			    uszKey == _KEY_TIMEOUT_			||
			    inChoice == _CUSTReceipt_Touch_ENTER_)
				break;
			else if (uszKey == _KEY_CANCEL_			||
				 inChoice == _CUSTReceipt_Touch_CANCEL_)
				return (VS_SUCCESS);
			else
				continue;
		}
	}

        /* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0	||
            !memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, strlen(_PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_)))
	{
		while (1)
		{
			if (pobTran->srBRec.uszHappyGoSingle != TRUE)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
				{
					/* 提示免簽名和授權碼 */
					inDISP_PutGraphic(_CHECK_SIGNATURE_6_, 0, _COORDINATE_Y_LINE_8_4_);
				}
				else
				{
                                        /* [113110-信託需求][UI] 修改列印顯提示圖,不再提示持卡人核對簽名字樣 2025/11/27 */
                                        if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
                                        {
                                            /* 顯示授權碼 */
                                            inDISP_PutGraphic(_TR_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
                                        }else
                                        {
                                            /* 提示檢核簽名和授權碼 */
                                            inDISP_PutGraphic(_CHECK_SIGNATURE_2_, 0, _COORDINATE_Y_LINE_8_4_);
                                        }
				}
			}
			else
			{
				/* 列印中 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);
			}

			if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
			{
				/* 純HappyGo不提示授權碼 */
			}
			else
			{
                                /* [113110-信託需求][UI] 信託交易,不用顯示授權碼 2025/11/27 */
                                if (pobTran->srTrustRec.uszTrustTransBit != VS_TRUE)
                                {
                                    memset(szDispBuf, 0x00, sizeof(szDispBuf));
                                    sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
                                    inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
                                }
			}
			
			pobTran->srBRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_PrintReceipt_ByBuffer_ESVC
Date&Time       :2018/1/8 下午 3:02
Describe        :
*/
int inFunc_NCCC_PrintReceipt_ByBuffer_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NONE_;
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szFesMode[2 + 1] = {0};
	unsigned char   uszKey = 0x00;
	
	/*  黑名單鎖卡不應印簽單 */
	if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
	{
	        return (VS_ERROR);
	}
	
	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);
	
	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szFesMode, 0x00, sizeof(szFesMode));
	inGetNCCCFESMode(szFesMode);
	/* 列印商店聯 */
	/* 客製化123不印商店聯且印簽單要提示音 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BEEP(1, 0);
		
		/* 提示訊息 */
		inNCCC_Ticket_Display_Transaction_Result(pobTran);

		/* 因為查詢不印，所以只顯示確認鍵 */
		if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
		    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
		    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
		{
			inTouchSensorFunc = _Touch_BATCH_END_;
			
			/* 清除keyboad Buffer */
			inFlushKBDBuffer();
		
			/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出顧客聯 */
			inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			
			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				uszKey = uszKBD_Key();

				if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
				{
					inDISP_BEEP(1, 0);
					inDISP_Wait(250);
				}

				if (uszKey == _KEY_ENTER_			|| 
				    uszKey == _KEY_TIMEOUT_			||
				    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
					break;
				else
					continue;
			}
			
			/* 清掉觸控殘值 */
			inDisTouch_Flush_TouchFile();
		}
	}
	else 
	{
		/* 簽單模式不是0就印商店聯 */
		if (memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0	&&
		    pobTran->uszECRBit == VS_TRUE)
		{
			/* MPAS 電票回傳ECR成功時，不印商店聯 */
			/* 交易別由inFunc_PrintReceipt_ByBuffer_Flow 篩選 */
		}
		else
		{
			if (memcmp(szPrtMode, "0", strlen("0")) != 0)
			{
				if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
				    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
				    pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 列印帳單中 */
					inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
				}

				while (1)
				{
					pobTran->srTRec.inPrintOption = _PRT_MERCH_;
					inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal == VS_PRINTER_PAPER_OUT ||
					    inRetVal == VS_PRINTER_OVER_HEAT)
						continue;
					else
					{
						break;
					}
				}
			}

			/* 三聯式簽單，中間要多印一聯客戶聯 */
			if (memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0	&&
			    pobTran->uszECRBit == VS_TRUE)
			{
				/* MPAS 電票回傳ECR成功時，不印商店聯 */
				/* 交易別由inFunc_PrintReceipt_ByBuffer_Flow 篩選 */
			}
			else
			{
				if (memcmp(szPrtMode, "3", strlen("3")) == 0)
				{
					if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
					    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
					    pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
					{
						inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 列印帳單中 */
						inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
					}

					/* 列印商店存根聯 */
					while (1)
					{
						pobTran->srTRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
						inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

						/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
						if (inRetVal == VS_PRINTER_PAPER_OUT ||
						    inRetVal == VS_PRINTER_OVER_HEAT)
							continue;
						else
						{
							break;
						}
					}
				}
			}

			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("交易已完成", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
				inDISP_ChineseFont("請列印第二聯", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
				/* 請按確認或清除鍵 */
				inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
			}
			else
			{
				/* 提示訊息 */
				inNCCC_Ticket_Display_Transaction_Result(pobTran);
			}

			/* 清除keyboad Buffer */
			inFlushKBDBuffer();

			/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出顧客聯 */
			/* 客製化107顯示訊息TimeOut 2秒 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
				inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
			}
			/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
				inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			}
			else
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
				inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
			}

			/* 客製化107.111移除請按確認鍵跟請按清除鍵 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)        ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
			{
				/* 因為查詢不印，所以只顯示確認鍵 */
				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_	||
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
				{
					while (1)
					{
						uszKey = uszKBD_Key();

						if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
						{
							uszKey = _KEY_TIMEOUT_;
						}

						if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
						{
							inDISP_BEEP(1, 0);
							inDISP_Wait(250);
						}

						if (uszKey == _KEY_TIMEOUT_)
							break;
						else
							continue;
					}
				}
				else
				{
					while (1)
					{
						uszKey = uszKBD_Key();

						if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
						{
							uszKey = _KEY_TIMEOUT_;
						}

						if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
						{
							inDISP_BEEP(1, 0);
							inDISP_Wait(250);
						}

						if (uszKey == _KEY_TIMEOUT_)
						{
							break;
						}
						else
						{
							continue;
						}
					}
				}
			}
			else
			{
				/* 因為查詢不印，所以只顯示確認鍵 */
				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
				{
					inTouchSensorFunc = _Touch_BATCH_END_;

					while (1)
					{
						inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
						uszKey = uszKBD_Key();

						if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
						{
							uszKey = _KEY_TIMEOUT_;
						}

						if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
						{
							inDISP_BEEP(1, 0);
							inDISP_Wait(250);
						}

						if (uszKey == _KEY_ENTER_			|| 
						    uszKey == _KEY_TIMEOUT_			||
						    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
							break;
						else
							continue;
					}
					/* 清掉觸控殘值 */
					inDisTouch_Flush_TouchFile();
				}
				else
				{
					inTouchSensorFunc = _Touch_CUST_RECEIPT_;

					while (1)
					{
						inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
						uszKey = uszKBD_Key();

						if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
						{
							uszKey = _KEY_TIMEOUT_;
						}

						if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
						{
							inDISP_BEEP(1, 0);
							inDISP_Wait(250);
						}

						if (uszKey == _KEY_ENTER_			|| 
						    uszKey == _KEY_TIMEOUT_			||
						    inChoice == _CUSTReceipt_Touch_ENTER_)
						{
							break;
						}
						else if (uszKey == _KEY_CANCEL_			||
							 inChoice == _CUSTReceipt_Touch_CANCEL_)
						{
							inRetVal = VS_USER_CANCEL;
							break;
						}
						else
						{
							continue;
						}
					}

					/* 清掉觸控殘值 */
					inDisTouch_Flush_TouchFile();

					/* 按取消不印顧客聯 */
					if (inRetVal == VS_USER_CANCEL)
					{
						return (VS_SUCCESS);
					}
				}
			}
		}
	}

	/* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0    ||
            !memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, strlen(_PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_)))
	{
		while (1)
		{
			/* 列印中 */
			/* 詢卡不用印 */
			if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
			    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
			    pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
			{
				inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
				inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);
			}

			pobTran->srTRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}
	
	/* For MPAS ECR先出簽單再顯示 */
	if (memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0	&&
	    pobTran->uszECRBit == VS_TRUE)
	{
		/* 提示訊息 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)   ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                        inNCCC_Ticket_098_Display_Transaction_Result_End(pobTran);
                else
                        inNCCC_Ticket_Display_Transaction_Result(pobTran);
		
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 3);
		
		while (1)
		{
			uszKey = uszKBD_Key();
			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}
			
			if (uszKey == _KEY_TIMEOUT_)
			{
				break;
			}
			else if (uszKey == _KEY_ENTER_)
			{
				/* 只有查餘額可以按enter鍵跳出 */
				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_	||
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
				{
					break;
				}
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_PrintReceipt_ByBuffer_098_ESVC
Date&Time       :2018/1/8 下午 3:02
Describe        :Reprint用
*/
int inFunc_NCCC_PrintReceipt_ByBuffer_098_ESVC(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NONE_;
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	char		szFesMode[2 + 1] = {0};
	unsigned char   uszKey = 0x00;
        unsigned char   uszPrintBit = VS_TRUE;
 
	/*  黑名單鎖卡不應印簽單 */
	if (pobTran->srTRec.uszBlackListBit == VS_TRUE)
	{
	        return (VS_ERROR);
	}

	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);
	
	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szFesMode, 0x00, sizeof(szFesMode));
	inGetNCCCFESMode(szFesMode);
	/* 列印商店聯 */
	/* 簽單模式不是0就印商店聯 */
//	if (memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0	&&
//	    pobTran->uszECRBit == VS_TRUE)
//	{
                if (pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_		||
                    pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_		||
                    pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_		||
                    pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_	||
                    pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_	||
                    pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
                        uszPrintBit = VS_FALSE;
                else
                        uszPrintBit = VS_TRUE; 
//	}
        
        if (uszPrintBit == VS_TRUE)
        {
		if (memcmp(szPrtMode, "0", strlen("0")) != 0)
		{
			if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
			    pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
			    pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				/* 列印帳單中 */
				inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
			}

			while (1)
			{
				pobTran->srTRec.inPrintOption = _PRT_MERCH_;
				inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

				/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
				if (inRetVal == VS_PRINTER_PAPER_OUT ||
				    inRetVal == VS_PRINTER_OVER_HEAT)
					continue;
				else
				{
					break;
				}
			}
		}
	
		/* 三聯式簽單，中間要多印一聯客戶聯 */
                if (memcmp(szPrtMode, "3", strlen("3")) == 0)
                {
                        if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
                            pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
                            pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
                        {
                                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                                /* 列印帳單中 */
                                inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);
                        }

                        /* 列印商店存根聯 */
                        while (1)
                        {
                                pobTran->srTRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
                                inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

                                /* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
                                if (inRetVal == VS_PRINTER_PAPER_OUT ||
                                    inRetVal == VS_PRINTER_OVER_HEAT)
                                        continue;
                                else
                                {
                                        break;
                                }
                        }
                }

		/* 提示訊息 */
		inNCCC_Ticket_Display_Transaction_Result(pobTran);
	
		/* 清除keyboad Buffer */
		inFlushKBDBuffer();

		/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出顧客聯 */
                inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
                inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);

                /* 因為查詢不印，所以只顯示確認鍵 */
                if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_ ||
                    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
                    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
                {
                        inTouchSensorFunc = _Touch_BATCH_END_;

                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                                uszKey = uszKBD_Key();

                                if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
                                {
                                        uszKey = _KEY_TIMEOUT_;
                                }

                                if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
                                {
                                        inDISP_BEEP(1, 0);
                                        inDISP_Wait(250);
                                }

                                if (uszKey == _KEY_ENTER_			|| 
                                    uszKey == _KEY_TIMEOUT_			||
                                    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
                                        break;
                                else
                                        continue;
                        }
                        /* 清掉觸控殘值 */
                        inDisTouch_Flush_TouchFile();
                }
                else
                {
                        inTouchSensorFunc = _Touch_CUST_RECEIPT_;

                        while (1)
                        {
                                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
                                uszKey = uszKBD_Key();

                                if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
                                {
                                        uszKey = _KEY_TIMEOUT_;
                                }

                                if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
                                {
                                        inDISP_BEEP(1, 0);
                                        inDISP_Wait(250);
                                }

                                if (uszKey == _KEY_ENTER_			|| 
                                    uszKey == _KEY_TIMEOUT_			||
                                    inChoice == _CUSTReceipt_Touch_ENTER_)
                                {
                                        break;
                                }
                                else if (uszKey == _KEY_CANCEL_			||
                                         inChoice == _CUSTReceipt_Touch_CANCEL_)
                                {
                                        inRetVal = VS_USER_CANCEL;
                                        break;
                                }
                                else
                                {
                                        continue;
                                }
                        }

                        /* 清掉觸控殘值 */
                        inDisTouch_Flush_TouchFile();

                        /* 按取消不印顧客聯 */
                        if (inRetVal == VS_USER_CANCEL)
                        {
                                return (VS_SUCCESS);
                        }
                }
	}    

	/* 列印顧客聯 */
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0    ||
            !memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, strlen(_PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_)))
	{
		
		while (1)
		{
			/* 列印中 */
			/* 詢卡不用印 */
			if (pobTran->srTRec.inCode != _TICKET_IPASS_INQUIRY_	&&
				pobTran->srTRec.inCode != _TICKET_EASYCARD_INQUIRY_ &&
				pobTran->srTRec.inCode != _TICKET_ICASH_INQUIRY_)
			{
				inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
				inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);
			}

			pobTran->srTRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
				inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}
	
	/* For MPAS ECR先出簽單再顯示 */
	if (memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0	&&
	    pobTran->uszECRBit == VS_TRUE)
	{
		/* 提示訊息 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                        inNCCC_Ticket_098_Display_Transaction_Result_End(pobTran);
                else
                        inNCCC_Ticket_Display_Transaction_Result(pobTran);
		
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 3);
		
		while (1)
		{
			uszKey = uszKBD_Key();
			if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}
			
			if (uszKey == _KEY_TIMEOUT_)
			{
				break;
			}
			else if (uszKey == _KEY_ENTER_)
			{
				/* 只有查餘額可以按enter鍵跳出 */
				if (pobTran->srTRec.inCode == _TICKET_IPASS_INQUIRY_	||
				    pobTran->srTRec.inCode == _TICKET_EASYCARD_INQUIRY_ ||
				    pobTran->srTRec.inCode == _TICKET_ICASH_INQUIRY_)
				{
					break;
				}
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_RePrintReceipt_ByBuffer_Flow
Date&Time       :2017/2/21 下午 4:52
Describe        :重印帳單分流
*/
int inFunc_RePrintReceipt_ByBuffer_Flow(TRANSACTION_OBJECT *pobTran)
{
	int		inTMSUploadRetVal = VS_SUCCESS;
	char		szTemplate[6 + 1];
	char		szESCMode[2 + 1];
	char		szTRTFileName[16 + 1];
	char		szTxnType[20 + 1];
	char		szFESMode[2 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	char		szSQL[200 + 1] = {0};
	char		szTableName[30 + 1] = {0};
	DUTYFREE_REPRINT_DATA	srReprintData;
	DUTYFREE_REPRINT_TITLE	srReprintTitle;
	SQLITE_ALL_TABLE	srALL;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_RePrintReceipt_ByBuffer_Flow START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_RePrintReceipt_ByBuffer_Flow() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		/* (需求單 - 107227)邦柏科技自助作業客製化 
		* 收銀機發動一般信用卡交易，EDC不提示使用者確認流程及畫面(EX：請按確認鍵、請核對持卡人簽名等流程)。
		* by Russell 2018/12/8 下午 8:42 */
		/* (需求單 - 107276)自助交易標準400做法 EDC不提示使用者確認流程及畫面 by Russell 2018/12/27 上午 11:20 */
	       if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)     ||
		   !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
	       {
		       memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		       inGetTRTFileName(szTRTFileName);

		       if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		       {
			       inNCCC_Ticket_107_Bumper_Display_Transaction_Result(pobTran);
			       return (VS_SUCCESS);
		       }
		       else
		       {
			       inFunc_Display_Msg_Instead_For_Print_Receipt(pobTran);
			       return (VS_SUCCESS);
		       }
	       }
	       else
	       {
			memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		       inGetTRTFileName(szTRTFileName);

		       if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		       {
			       inNCCC_Ticket_107_Bumper_Display_Transaction_Result(pobTran);
			       return (VS_SUCCESS);
		       }
		       else
		       {
			       inFunc_Display_Msg_Instead_For_Print_Receipt(pobTran);
			       return (VS_SUCCESS);
		       }
	       }
	}
	else
	{
		/* (需求單 - 107227)邦柏科技自助作業客製化 
		* 收銀機發動一般信用卡交易，EDC不提示使用者確認流程及畫面(EX：請按確認鍵、請核對持卡人簽名等流程)。
		* by Russell 2018/12/8 下午 8:42 */
		/* (需求單 - 107276)自助交易標準400做法 EDC不提示使用者確認流程及畫面 by Russell 2018/12/27 上午 11:20 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)     ||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
		{
		       memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
		       inGetTRTFileName(szTRTFileName);

		       if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		       {
			       inNCCC_Ticket_107_Bumper_Display_Transaction_Result(pobTran);
			       return (VS_SUCCESS);
		       }
		       else
		       {
			       inFunc_Display_Msg_Instead_For_Print_Receipt(pobTran);
			       return (VS_SUCCESS);
		       }
		}
		else
		{
			/* 判斷MP為Y重印簽單能不能印 */
			memset(szFESMode, 0x00, sizeof(szFESMode));
			inGetNCCCFESMode(szFESMode);
			if (memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0)
			{
                                /* 客製化098，電票列印簽單規則不同 */
                                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)    ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                                {
                                        if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                                        {
                                                /* 電票一律印出簽帳單 */
                                        }
                                        else
                                        {
						/* (需求單-112115)-收銀機連線之小額特店進行小額交易時, 特店使用端末機重印簽單功能將持卡人存根聯提供給持卡人需求 by Russell 2023/9/28 下午 5:37 */
                                        }
                                }
                                else
                                {
                                        if (pobTran->srTRec.uszESVCTransBit == VS_TRUE)
                                        {
                                                /* (需求單-112115)-收銀機連線之小額特店進行小額交易時, 特店使用端末機重印簽單功能將持卡人存根聯提供給持卡人需求 by Russell 2023/9/28 下午 5:37 */
                                        }
                                        else
                                        {
                                                /* (需求單-112115)-收銀機連線之小額特店進行小額交易時, 特店使用端末機重印簽單功能將持卡人存根聯提供給持卡人需求 by Russell 2023/9/28 下午 5:37 */
                                        }
                                }
			}

			memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
			inGetTRTFileName(szTRTFileName);

			if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0	||
			    memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
			{
				/* (6)	ESC功能僅包含中心NCCC Host(含HappyGo混合交易)及DCC Host之交易(不含大來卡) */
				/* ESC開關 */
				memset(szESCMode, 0x00, sizeof(szESCMode));
				inGetESCMode(szESCMode);

				/* 沒ESC的狀況，直接印紙本，不須另外判斷 */
				if (szESCMode[0] != 'Y')
				{
					/* 優惠兌換和優惠取消 重印只印一聯 */
					if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ || pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inGetInvoiceNum(szTemplate);

						/* invoice Num減一即最後一筆 */
						if ((atoi(szTemplate) - 1) != pobTran->srBRec.lnOrgInvNum)
						{
							/* 此筆無法重印 */
							DISPLAY_OBJECT	srDispMsgObj;
							memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
							strcpy(srDispMsgObj.szDispPic1Name, _ERR_REPRINT_);
							srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
							srDispMsgObj.inMsgType = _NO_KEY_MSG_;
							srDispMsgObj.inTimeout = 3;
							strcpy(srDispMsgObj.szErrMsg1, "");
							srDispMsgObj.inErrMsg1Line = 0;
							inDISP_Msg_BMP(&srDispMsgObj);
							
							return (VS_SUCCESS);
						}
						else
						{
							inFunc_NCCC_PrintReceipt_ByBuffer_Loyalty_Redeem(pobTran);
						}

					}
					else
					{
						inFunc_NCCC_PrintReceipt_ByBuffer(pobTran);
					}

				}
				/* 有ESC的狀況 */
				else
				{
					/* 優惠兌換和優惠取消 重印只印一聯 */
					if (pobTran->srBRec.inCode == _LOYALTY_REDEEM_ || pobTran->srBRec.inCode == _VOID_LOYALTY_REDEEM_)
					{
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inGetInvoiceNum(szTemplate);

						/* invoice Num減一即最後一筆 */
						if ((atoi(szTemplate) - 1) != pobTran->srBRec.lnOrgInvNum)
						{
							/* 此筆無法重印 */
							DISPLAY_OBJECT	srDispMsgObj;
							memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
							strcpy(srDispMsgObj.szDispPic1Name, _ERR_REPRINT_);
							srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
							srDispMsgObj.inMsgType = _NO_KEY_MSG_;
							srDispMsgObj.inTimeout = 3;
							strcpy(srDispMsgObj.szErrMsg1, "");
							srDispMsgObj.inErrMsg1Line = 0;
							inDISP_Msg_BMP(&srDispMsgObj);
							
							return (VS_SUCCESS);
						}
						else
						{
							inFunc_NCCC_PrintReceipt_ByBuffer_Loyalty_Redeem(pobTran);
						}

					}
					else
					{
						inFunc_NCCC_RePrintReceipt_ByBuffer_ESC(pobTran);
					}
				}

			}
			/* 純HG交易，混合交易看NCCC */
			else if (memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
			{
				if (pobTran->srBRec.uszHappyGoSingle == VS_TRUE)
				{
					/* 紅利積點現金及禮券可以由【TMS】控制是否列印帳單 */
					if (pobTran->srBRec.inHGCode == _HG_REWARD_CASH_	||
					    pobTran->srBRec.inHGCode == _HG_REWARD_GIFT_PAPER_)
					{
						memset(szTxnType, 0x00, sizeof(szTxnType));
						inGetTransFunc(szTxnType);
						if (szTxnType[8] == 'Y')
						{
							inFunc_NCCC_PrintReceipt_ByBuffer_HappyGo_Single(pobTran);
						}
						else
						{

						}

					}
					else
					{
						inFunc_NCCC_PrintReceipt_ByBuffer_HappyGo_Single(pobTran);
					}

				}
				else
				{
					/* 請選擇信用卡主機 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_PLS_SELECT_CREDIT_HOST_;
					return (VS_ERROR);
				}
			}
			else if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
			{
                                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_) ||
                                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
                                {
                                        inFunc_NCCC_PrintReceipt_ByBuffer_098_ESVC(pobTran);
                                }
                                else
                                {
                                        inFunc_NCCC_PrintReceipt_ByBuffer_ESVC(pobTran);
                                }
			}
			/* 其他Host，如DINERS */
			else
			{
				inFunc_PrintReceipt_ByBuffer(pobTran);
			}
		}
	}
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化075) by Russell 2022/4/22 下午 2:27
	 * 重印成功明細須儲存 */
	if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_) || 
	     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)) &&
	     pobTran->srBRec.inCode != _LOYALTY_REDEEM_ && 
	     pobTran->srBRec.inCode != _VOID_LOYALTY_REDEEM_ &&
	     pobTran->srTRec.uszESVCTransBit != VS_TRUE)
	{
		memset(&srReprintData, 0x00, sizeof(DUTYFREE_REPRINT_DATA));
		
		/* 將明細包進結構內 */
		inBATCH_Reprint_Data_Prepare(pobTran, &srReprintData);
		/* 將結構存進資料庫內 */
		inBATCH_Reprint_Data_Save(pobTran, &srReprintData);
		
		/* 預設為上傳失敗，待成功時將那一筆消除 */
		memset(&srReprintTitle, 0x00, sizeof(DUTYFREE_REPRINT_TITLE));
		memset(&srALL, 0x00, sizeof(SQLITE_ALL_TABLE));
		inBatch_Table_Link_Reprint_Title(&srReprintTitle, &srALL, _LS_READ_);
		inSqlite_Get_Table_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_REPRINT_TITLE_, 0, &srALL);
		sprintf(srReprintTitle.szTMSUpdateFailNum, "%03d", atoi(srReprintTitle.szTMSUpdateFailNum) + 1);
		memset(&srALL, 0x00, sizeof(SQLITE_ALL_TABLE));
		inBatch_Table_Link_Reprint_Title(&srReprintTitle, &srALL, _LS_UPDATE_);
		inSqlite_Insert_Or_Replace_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_REPRINT_TITLE_, 0, &srALL);

		/* 昇恆昌處理重印帳單上傳至〈TMS〉 */
		/* 當筆上傳失敗時，要記錄 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("重印紀錄上傳中", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		
		inTMSUploadRetVal = inNCCCTMS_DUTY_FREE_ReturnTxnReceipt(pobTran);
		if (inTMSUploadRetVal == VS_SUCCESS)
		{
			/* 將失敗的筆數改為成功，並把那一筆刪除 */
			sprintf(srReprintTitle.szTMSUpdateFailNum, "%03d", atoi(srReprintTitle.szTMSUpdateFailNum) - 1);
			sprintf(srReprintTitle.szTMSUpdateSuccessNum, "%03d", atoi(srReprintTitle.szTMSUpdateSuccessNum) + 1);
			memset(&srALL, 0x00, sizeof(SQLITE_ALL_TABLE));
			inBatch_Table_Link_Reprint_Title(&srReprintTitle, &srALL, _LS_UPDATE_);
			inSqlite_Insert_Or_Replace_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_REPRINT_TITLE_, 0, &srALL);
			
			/* 刪除最後insert的一筆 */
			inGetTRTFileName(szTRTFileName);
			if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_CREDIT_))))
			{
				sprintf(szTableName, _TABLE_NAME_REPRINT_NCCC_);
			}
			else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, max(strlen(szTRTFileName), strlen(_TRT_FILE_NAME_DCC_))))
			{
				sprintf(szTableName, _TABLE_NAME_REPRINT_DCC_);
			}
			sprintf(szSQL, "WHERE inTableID = (SELECT MAX(inTableID) FROM %s)", szTableName);
			inSqlite_Delete_Record_By_External_SQL(gszReprintDBPath, szTableName, szSQL);
		}
		else
		{
			/* 已在前面紀錄為失敗 */
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_RePrintReceipt_ByBuffer_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_NCCC_RePrintReceipt_ByBuffer_ESC
Date&Time       :2017/3/22 下午 1:45
Describe        :和inFunc_NCCC_PrintReceipt_ByBuffer_ESC相同，但拿掉商店聯Copy的判斷
*/
int inFunc_NCCC_RePrintReceipt_ByBuffer_ESC(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CUST_RECEIPT_;
	char		szDispBuf[_DISP_MSG_SIZE_ + 1] = {0};
	char		szPrtMode[2 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char   uszKey = 0x00;
	
	/* 為了恢復被signpad清掉的title */
	inFunc_ResetTitle(pobTran);
	
	memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 客製化123不印商店聯且印簽單要提示音 */
	/* ESC簽名流程時被斷電重開機，開機時候會印兩聯簽單，改成只印一聯 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_BEEP(1, 0);
	}
	else 
	{
		if (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_NOT_SUPPORTED_	||
		    pobTran->srBRec.inESCUploadMode == _ESC_STATUS_OVERLIMIT_		||
		   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_		&&
		    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_))
		{
			/* 列印商店聯 */
			/* 簽單模式不是0就印商店聯 */
			if (memcmp(szPrtMode, "0", strlen("0")) != 0)
			{
				/* 列印帳單中 */
				inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

				while (1)
				{
					pobTran->srBRec.inPrintOption = _PRT_MERCH_;
					inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

					/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
					if (inRetVal == VS_PRINTER_PAPER_OUT ||
					    inRetVal == VS_PRINTER_OVER_HEAT)
						continue;
					else
					{
						/* 成功就砍簽名圖檔 */
						inFunc_Delete_Signature(pobTran);

						break;
					}

				}
			}

			if (inNCCC_ESC_MerchantCopy_Check(pobTran) == VS_SUCCESS	||
			   (pobTran->srBRec.inESCUploadMode == _ESC_STATUS_SUPPORTED_	&&
			    pobTran->srBRec.inSignStatus == _SIGN_BYPASS_))
			{
				/* 列印帳單中 */
				inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);

				/* 三聯式簽單，中間要多印一聯客戶聯 */
				if (memcmp(szPrtMode, "3", strlen("3")) == 0)
				{
					/* 列印商店存根聯 */
					while (1)
					{
						pobTran->srBRec.inPrintOption = _PRT_MERCH_DUPLICATE_;
						inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

						/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
						if (inRetVal == VS_PRINTER_PAPER_OUT ||
						    inRetVal == VS_PRINTER_OVER_HEAT)
						    continue;
						else
						{
							break;
						}
					}
				}
			}

			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_026_TAKA_, _CUSTOMER_INDICATOR_SIZE_) ||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_021_TAKAWEL_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("交易已完成", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
				inDISP_ChineseFont("請列印第二聯", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
				/* 請按確認或清除鍵 */
				inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
			}
			else
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
				{
					/* SmartPay交易要提示調閱編號FiscRRN */
					inDISP_PutGraphic(_CHECK_SIGNATURE_3_, 0, _COORDINATE_Y_LINE_8_4_);

					memset(szDispBuf, 0x00, sizeof(szDispBuf));
					sprintf(szDispBuf, "%s", pobTran->srBRec.szFiscRRN);
					inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 9);
				}
				else
				{
					if (pobTran->srBRec.uszNoSignatureBit == VS_TRUE && pobTran->srBRec.inCode != _TIP_)
					{
						/* 提示免簽名和授權碼 */
						inDISP_PutGraphic(_CHECK_SIGNATURE_5_, 0, _COORDINATE_Y_LINE_8_4_);
					}
					else
					{
                                                /* [113110-信託需求][UI] 修改列印顯提示圖,不再提示持卡人核對簽名字樣 2025/11/27 */
                                                if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
                                                {
                                                    /* 顯示授權碼 */
                                                    inDISP_PutGraphic(_TR_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
                                                }else
                                                {
                                                    /* 提示檢核簽名和授權碼 */
                                                    inDISP_PutGraphic(_CHECK_SIGNATURE_1_, 0, _COORDINATE_Y_LINE_8_4_);
                                                }
					}
                                        /* [113110-信託需求][UI] 信託交易,不用顯示授權碼 2025/11/27 */
                                        if (pobTran->srTrustRec.uszTrustTransBit != VS_TRUE)
                                        {
                                            memset(szDispBuf, 0x00, sizeof(szDispBuf));
                                            sprintf(szDispBuf, "%s", pobTran->srBRec.szAuthCode);
                                            inDISP_EnglishFont_Point_Color(szDispBuf, _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_RED_,_COLOR_WHITE_, 11);
                                        }
				}
			}

			/* 清除keyboad Buffer */
			inFlushKBDBuffer();

			/* 第一聯印完後15秒未按確認或清除鍵則端末機嗶嗶聲提示，響15秒後自動出第二聯 */
			/* 客製化107顯示訊息TimeOut 2秒 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
				inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_107_BUMPER_DISPLAY_TIMEOUT_);
			}
			/* (需求單 - 107276)自助交易標準400做法 by Russell 2018/12/27 上午 11:20 */
			else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_))
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
				inDISP_Timer_Start(_TIMER_NEXSYS_3_, _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_);
			}
			else
			{
				inDISP_Timer_Start(_TIMER_NEXSYS_2_, 30);
				inDISP_Timer_Start(_TIMER_NEXSYS_3_, 15);
			}

			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
				uszKey = uszKBD_Key();

				if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
				{
					inDISP_BEEP(1, 0);
					inDISP_Wait(250);
				}

				if (uszKey == _KEY_ENTER_			|| 
				    uszKey == _KEY_TIMEOUT_			||
				    inChoice == _CUSTReceipt_Touch_ENTER_)
				{
					inRetVal = VS_SUCCESS;
					break;
				}
				else if (uszKey == _KEY_CANCEL_			||
				     inChoice == _CUSTReceipt_Touch_CANCEL_)
				{
					inRetVal = VS_USER_CANCEL;
					break;
				}
			}
			/* 清空Touch資料 */
			inDisTouch_Flush_TouchFile();

			/* 因為這裡交易已完成，所以一定回傳成功*/
			if (inRetVal == VS_USER_CANCEL)
			{
				return (VS_SUCCESS);
			}
		}
	}
	
	/* 三聯式簽單，兩聯以上就要印顧客聯 */
	if (memcmp(szPrtMode, "2", strlen("2")) == 0	||
	    memcmp(szPrtMode, "3", strlen("3")) == 0    ||
            !memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, strlen(_PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_)))
	{
		/* 列印顧客聯 */
		while (1)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PRINTING_, 0, _COORDINATE_Y_LINE_8_7_);

			pobTran->srBRec.inPrintOption = _PRT_CUST_;
			inRetVal = inCREDIT_PRINT_Receipt_ByBuffer(pobTran);

			/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
			if (inRetVal == VS_PRINTER_PAPER_OUT ||
			    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
			else
			{
				break;
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrePrintReport_ByBuffer
Date&Time       :2016/2/24 下午 4:26
Describe        :FuncTable.c預先列印簽單，
 *		總額顯示按0之前
*/
int inFunc_PrePrintReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
        char	szCustomerIndicator[3 + 1] = {0};
        
	vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrePrintReport_ByBuffer START!");
	
	if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_PrePrintReport_ByBuffer() START !");
	}
        
	/* 個別結帳 */
        if (pobTran->uszAutoSettleBit != VS_TRUE)
	{
                /* 處理列印Title問題 */
                pobTran->uszPrePrintBit = VS_TRUE;

                memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
                inGetCustomIndicator(szCustomerIndicator);

                /* 客製化098 結帳前先列印結帳報表與明細報表 by Tusin - 2021/7/2 下午 2:23 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)	||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        /* 不須讀主機，不管連動或單結應該都已經確定主機 */
                        inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_);          
                        inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_DETAIL_REPORT_BY_BUFFER_);
                }

                pobTran->uszPrePrintBit = VS_FALSE;
                
                if (ginDebug == VS_TRUE)
                {
                        if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
                            inLogPrintf(AT, "pobTran->srBRec.uszFiscTransBit = VS_TRUE)");
                        else
                            inLogPrintf(AT, "pobTran->srBRec.uszFiscTransBit != VS_TRUE)");
                }
        }
        
        if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_PrePrintReport_ByBuffer() END !");
                inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrePrintReport_ByBuffer
Date&Time       :2016/2/24 下午 4:26
Describe        :FuncTable.c預先列印簽單
 *		總額顯示按0之後
*/
int inFunc_PrePrintReport_Auto_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int	inNCCCIndex = -1;
	int	inHGIndex = -1;
        char	szCustomerIndicator[3 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrePrintReport_Auto_ByBuffer START!");
        
	if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_PrePrintReport_Auto_ByBuffer() START !");
	}
    
	/* 客製化047個別結帳無須加印總額與明細功能 */
        if (pobTran->uszAutoSettleBit == VS_TRUE)
	{
                /* 處理列印Title問題 */
                pobTran->uszPrePrintBit = VS_TRUE;

                memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
                inGetCustomIndicator(szCustomerIndicator);

                /* 客製化098 結帳前先列印結帳報表與明細報表 by Tusin - 2021/7/2 下午 2:23 */
                if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)	||
                    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                {
                        /* 不須讀主機，不管連動或單結應該都已經確定主機 */
                        inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_);
                        inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_DETAIL_REPORT_BY_BUFFER_);
                }
		/* 客製化047 結帳前先列印明細報表再結帳報表 by Russell 2022/4/27 下午 4:00 */
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_047_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			/* 不須讀主機，不管連動或單結應該都已經確定主機 */
			inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_DETAIL_REPORT_BY_BUFFER_);
			inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_);

			/* NCCC主機的話，後面要加印HG */
			inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
			if (pobTran->srBRec.inHDTIndex == inNCCCIndex)
			{
				inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHGIndex);
				pobTran->srBRec.inHDTIndex = inHGIndex;
				inLoadHDTRec(pobTran->srBRec.inHDTIndex);
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_DETAIL_REPORT_BY_BUFFER_);
				inFLOW_RunFunction(pobTran, _FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_);
				pobTran->srBRec.inHDTIndex = inNCCCIndex;
				inLoadHDTRec(pobTran->srBRec.inHDTIndex);
				inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
			}
		}

                pobTran->uszPrePrintBit = VS_FALSE;
        }
        
        if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_PrePrintReport_Auto_ByBuffer() END !");
                inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintTotalReport_ByBuffer
Date&Time       :2016/2/24 下午 4:26
Describe        :FuncTable.c列印總額帳單
*/
int inFunc_PrintTotalReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szTRTFileName[16 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_PrintTotalReport_ByBuffer START!");
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	while (1)
        {
		if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer_NCCC(pobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer_DCC(pobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer_HG(pobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer_ESVC(pobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_TAKA_, strlen(_TRT_FILE_NAME_TAKA_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer_TAKA(pobTran);
		}
                else if (memcmp(szTRTFileName, _TRT_FILE_NAME_TRUST_, strlen(_TRT_FILE_NAME_TRUST_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer_TRUST(pobTran);
		}
		else
		{
			inRetVal = inCREDIT_PRINT_TotalReport_ByBuffer(pobTran);
		}
		
		/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
		if (inRetVal == VS_PRINTER_PAPER_OUT ||
		    inRetVal == VS_PRINTER_OVER_HEAT)
			continue;
		else
		{
			break;
		}
	}
        
        /* 標記為已做過 */
        if (pobTran->inRunTRTID == _TRT_SETTLE_ ||
	    pobTran->inRunTRTID == _TRT_TICKET_ECC_SETTLE_)
        {
                inNCCC_Func_Settlement_XML_Edit(_SETTLENMENT_RECOVER_XML_TAG_FUNCTION_PRINT_TOTAL_REPORT_BY_BUFFER_, "Y");
        }
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_PrintDetailReport_ByBuffer
Date&Time       :2015/8/20 上午 10:24
Describe        :FuncTable.c列印明細帳單
*/
int inFunc_PrintDetailReport_ByBuffer(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szTRTFileName[16 + 1];
        TRANSACTION_OBJECT	TemppobTran;
        
	/* 修改結帳前印明細報表，會把交易紀錄內的Flag On起來的問題 */
        memset((char *)&TemppobTran, 0x00, sizeof(TRANSACTION_OBJECT));
        memcpy((char *)&TemppobTran, (char *)pobTran, sizeof(TRANSACTION_OBJECT));
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	while (1)
        {
		if (memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_NCCC_DetailReport_ByBuffer(&TemppobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_DetailReport_ByBuffer(&TemppobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_DetailReport_ByBuffer(&TemppobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_DetailReport_ByBuffer_ESVC(&TemppobTran);
		}
		else if (memcmp(szTRTFileName, _TRT_FILE_NAME_TAKA_, strlen(_TRT_FILE_NAME_TAKA_)) == 0)
		{
			inRetVal = inCREDIT_PRINT_DetailReport_ByBuffer_TAKA(&TemppobTran);
		}
		else
		{
			inRetVal = inCREDIT_PRINT_DetailReport_ByBuffer(&TemppobTran);
		}
	 
		/* 沒紙就重印、過熱也重印、理論上要成功，若非以上二原因則為程式BUG */
		if (inRetVal == VS_PRINTER_PAPER_OUT ||
		    inRetVal == VS_PRINTER_OVER_HEAT)
				continue;
		else
		{
			break;
		}
	}
	
        return (VS_SUCCESS);
}


/*
Function        :inGetTransType
Date&Time       :2015/8/10 上午 10:24
Describe        :取得交易別
*/
int inFunc_GetTransType(TRANSACTION_OBJECT *pobTran, char *szPrintBuf1, char* szPrintBuf2)
{
	char	szDebugMsg[100 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	char	szShort_Receipt_Mode[1 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szShort_Receipt_Mode, 0x00, sizeof(szShort_Receipt_Mode));
	inGetShort_Receipt_Mode(szShort_Receipt_Mode);
	
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (!memcmp(szShort_Receipt_Mode, "Y", 1) ||
		    !memcmp(szShort_Receipt_Mode, "S", 1))
		{
			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				/* 正向交易 */
				switch (pobTran->srBRec.inCode)
				{
					case _CASH_ADVANCE_ :
						strcpy(szPrintBuf1, "05 預借現金 CASH ADVANCE");
						break;
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
						strcpy(szPrintBuf1, "05 交易補登 FORCE");
						strcpy(szPrintBuf2, "");
						break;
					default :
						strcpy(szPrintBuf1, "!!!NCCC_正向交易!!!");
						strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "inCode = %d", pobTran->srBRec.inCode);
							inLogPrintf(AT, szDebugMsg);
						}
						break;
				}
			}
			else
			{
				/* 負向交易 */
				switch (pobTran->srBRec.inOrgCode)
				{
					case _CASH_ADVANCE_ :
						strcpy(szPrintBuf1, "13 取消-預借現金 VOID CASH ADVANCE");
						break;
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
						strcpy(szPrintBuf1, "13 取消-交易補登 VOID FORCE");
						break;
					default :
						strcpy(szPrintBuf1, "!!!NCCC_負向交易!!!");
						strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");
						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "inOrgCode = %d", pobTran->srBRec.inOrgCode);
							inLogPrintf(AT, szDebugMsg);
						}
						break;
				}
			}
		}
		else
		{
			if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
			{
				/* 正向交易 */
				switch (pobTran->srBRec.inCode)
				{
					case _CASH_ADVANCE_ :
						strcpy(szPrintBuf1, "05 預借現金 CASH ");
						strcpy(szPrintBuf2, "ADVANCE");
						break;
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
						strcpy(szPrintBuf1, "05 交易補登 FORCE");
						strcpy(szPrintBuf2, "");
						break;
					default :
						strcpy(szPrintBuf1, "!!!NCCC_正向交易!!!");
						strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "inCode = %d", pobTran->srBRec.inCode);
							inLogPrintf(AT, szDebugMsg);
						}
						break;
				}
			}
			else
			{
				/* 負向交易 */
				switch (pobTran->srBRec.inOrgCode)
				{
					case _CASH_ADVANCE_ :
						strcpy(szPrintBuf1, "13 取消-預借現金 ");
						strcpy(szPrintBuf2, "VOID CASH ADVANCE");
						break;
					case _SALE_OFFLINE_ :
					case _FORCE_CASH_ADVANCE_ :
						strcpy(szPrintBuf1, "13 取消-交易補登 ");
						strcpy(szPrintBuf2, "VOID FORCE");
						break;
					default :
						strcpy(szPrintBuf1, "!!!NCCC_負向交易!!!");
						strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "inOrgCode = %d", pobTran->srBRec.inOrgCode);
							inLogPrintf(AT, szDebugMsg);
						}
						break;
				}
			}
		}
	}
	else
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_) &&
		    (!memcmp(szShort_Receipt_Mode, "Y", 1) || !memcmp(szShort_Receipt_Mode, "S", 1)))
		{
			if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
			{
				/* 正向交易 */
				switch (pobTran->srTrustRec.inCode)
				{
					case _TRUST_SALE_ :
						strcpy(szPrintBuf1, "00 信託交易 SALE");
						strcpy(szPrintBuf2, "");
						break;
					case _TRUST_VOID_ :
						strcpy(szPrintBuf1, "01 信託交易取消 VOIDㄋ");
						strcpy(szPrintBuf2, "");
						break;
					default :
						strcpy(szPrintBuf1, "!!!NCCC_信託交易!!!");
						strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "inCode = %d", pobTran->srBRec.inCode);
							inLogPrintf(AT, szDebugMsg);
						}
					break;
				}
			}
			else
			{
				if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
				{
					/* 正向交易 */
					switch (pobTran->srBRec.inCode)
					{		
						case _SALE_ :
						case _CUP_SALE_ :
							strcpy(szPrintBuf1, "00 一般交易 SALE");
							strcpy(szPrintBuf2, "");
							break;
						case _SALE_OFFLINE_ :
						case _FORCE_CASH_ADVANCE_ :
							strcpy(szPrintBuf1, "00 交易補登 FORCE");
							strcpy(szPrintBuf2, "");
							break;
						case _REFUND_ :
						case _CUP_REFUND_ :
							strcpy(szPrintBuf1, "04 退貨 REFUND");
							strcpy(szPrintBuf2, "");
							break;
						case _CUP_MAIL_ORDER_REFUND_ :
							/* 【需求單 - 103375】銀聯卡MOTO-Recurring add by LingHsiung 2015-10-06 下午 04:35:05 */
							/*
								2015-10-13 (週二) 上午 10:59 吳升文 回覆
								簽單與明細中，銀聯郵購退貨交易及取消銀聯郵購退貨交易的交易類別依下面顯示(中英文)
								銀聯郵購退貨 04 退貨-郵購 REFUND MAIL ORDER
								取消銀聯郵購退貨12 取消-退貨-郵購VOID REFUND MAIL ORDER
							*/
							strcpy(szPrintBuf1, "04 退貨-郵購 REFUND MAIL ORDER");
							break;
						case _INST_SALE_ :
						case _CUP_INST_SALE_:
							strcpy(szPrintBuf1, "00 一般交易-分期付款 INST.");
							break;
						case _INST_REFUND_ :
						case _CUP_INST_REFUND_:
							strcpy(szPrintBuf1, "04 退貨-分期付款 REFUND INST.");
							break;
						case _REDEEM_SALE_ :
						case _CUP_REDEEM_SALE_:
							strcpy(szPrintBuf1, "00 一般交易-紅利扣抵 REDEEM");
							break;
						case _REDEEM_REFUND_ :
						case _CUP_REDEEM_REFUND_:
							strcpy(szPrintBuf1, "04 退貨-紅利扣抵 REFUND REDEEM");
							break;
						case _TIP_ :
							strcpy(szPrintBuf1, "11 小費交易 TIPS");
							break;
						case _REDEEM_ADJUST_ :
							strcpy(szPrintBuf1, "00 後台調帳-紅利扣抵 ADJUST REDEEM");
							break;
						case _INST_ADJUST_ :
							strcpy(szPrintBuf1, "00 後台調帳-分期付款 ADJUST INST.");
							break;
						case _PRE_AUTH_ :
						case _CUP_PRE_AUTH_ :
							strcpy(szPrintBuf1, "01 預先授權 PREAUTH");
							break;
						case _PRE_COMP_ :
						case _CUP_PRE_COMP_ :
							strcpy(szPrintBuf1, "02 預先授權完成 PREAUTH COMPLETE");
							break;
						case _MAIL_ORDER_ :
						case _CUP_MAIL_ORDER_ :
							strcpy(szPrintBuf1, "03 郵購 MAIL ORDER");
							break;
						case _CASH_ADVANCE_ :
							strcpy(szPrintBuf1, "05 預借現金交易 CASH ADVANCE");
							break;
						case _FISC_SALE_ :
							strcpy(szPrintBuf1, "2541 消費扣款");
							strcpy(szPrintBuf2, "");
							break;
						case _FISC_REFUND_ :
							strcpy(szPrintBuf1, "2543 退費交易");
							strcpy(szPrintBuf2, "");
							break;
						default :
							strcpy(szPrintBuf1, "!!!NCCC_正向交易!!!");
							strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");

							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "inCode = %d", pobTran->srBRec.inCode);
								inLogPrintf(AT, szDebugMsg);
							}
							break;
					}
				}
				else
				{
					/* 負向交易 */
					switch (pobTran->srBRec.inOrgCode)
					{
						case _SALE_ :
						case _TIP_ :
						case _CUP_SALE_ :
							strcpy(szPrintBuf1, "11 取消 VOID");
							break;
						case _SALE_OFFLINE_ :
						case _FORCE_CASH_ADVANCE_ :
							strcpy(szPrintBuf1, "11 取消-交易補登 VOID FORCE");
							break;
						case _REFUND_ :
						case _CUP_REFUND_ :
							strcpy(szPrintBuf1, "12 取消-退貨 VOID REFUND");
							break;
						case _CUP_MAIL_ORDER_REFUND_ :
							strcpy(szPrintBuf1, "12 取消-退貨-郵購 VOID REFUND MAIL ORDER");
							break;
						case _INST_SALE_ :
						case _CUP_INST_SALE_:
							strcpy(szPrintBuf1, "11 取消-分期付款 VOID INST.");
							break;
						case _REDEEM_SALE_ :
						case _CUP_REDEEM_SALE_:
							strcpy(szPrintBuf1, "11 取消-紅利扣抵 VOID REDEEM");
							break;
						case _REDEEM_ADJUST_ :
							strcpy(szPrintBuf1, "11 取消-後台調帳-紅利 VOID ADJUST REDEEM");
							break;
						case _INST_ADJUST_ :
							strcpy(szPrintBuf1, "11 取消-後台調帳-分期 VOID ADJUST INST.");
							break;
						case _REDEEM_REFUND_ :
						case _CUP_REDEEM_REFUND_ :
							strcpy(szPrintBuf1, "12 取消-退貨-紅利扣抵 VOID REFUND REDEEM");
							break;
						case _INST_REFUND_ :
						case _CUP_INST_REFUND_:
							strcpy(szPrintBuf1, "12 取消-退貨-分期付款 VOID REFUND INST.");
							break;
						case _PRE_AUTH_ :
						case _CUP_PRE_AUTH_ :
							strcpy(szPrintBuf1, "11 取消-預先授權 VOID PREAUTH");
							break;
						case _CUP_PRE_COMP_ :
						case _PRE_COMP_ :
							strcpy(szPrintBuf1, "11 取消-預先授權完成 VOID PREAUTH COMPLETE");
							break;
						case _CASH_ADVANCE_ :
							strcpy(szPrintBuf1, "13 取消-預借現金 VOID CASH ADVANCE");
							break;
						case _MAIL_ORDER_ :
						case _CUP_MAIL_ORDER_ :
							strcpy(szPrintBuf1, "11 取消-郵購 VOID MAIL ORDER");
							break;
						case _FISC_SALE_ :
							strcpy(szPrintBuf1, "2542 沖正交易");
							strcpy(szPrintBuf2, "");
							break;
						/* SmartPay退費交易不能取消 */
						default :
							strcpy(szPrintBuf1, "!!!NCCC_負向交易!!!");
							strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");

							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "inOrgCode = %d", pobTran->srBRec.inOrgCode);
								inLogPrintf(AT, szDebugMsg);
							}
							break;
					}
				}
			}
		}
		else
		{
			if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
			{
				/* 正向交易 */
				switch (pobTran->srTrustRec.inCode)
				{
					case _TRUST_SALE_ :
						strcpy(szPrintBuf1, "00 信託交易");
						strcpy(szPrintBuf2, "SALE");
						break;
					case _TRUST_VOID_ :
						strcpy(szPrintBuf1, "01 信託交易取消");
						strcpy(szPrintBuf2, "VOID");
						break;
					default :
						strcpy(szPrintBuf1, "!!!NCCC_信託交易!!!");
						strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");

						if (ginDebug == VS_TRUE)
						{
							memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
							sprintf(szDebugMsg, "inCode = %d", pobTran->srBRec.inCode);
							inLogPrintf(AT, szDebugMsg);
						}
					break;
				}
			}
			else
			{
				if (pobTran->srBRec.uszVOIDBit == VS_FALSE)
				{
					/* 正向交易 */
					switch (pobTran->srBRec.inCode)
					{		
						case _SALE_ :
						case _CUP_SALE_ :
							strcpy(szPrintBuf1, "00 一般交易 SALE");
							strcpy(szPrintBuf2, "");
							break;
						case _SALE_OFFLINE_ :
						case _FORCE_CASH_ADVANCE_ :
							strcpy(szPrintBuf1, "00 交易補登 FORCE");
							strcpy(szPrintBuf2, "");
							break;
						case _REFUND_ :
						case _CUP_REFUND_ :
							strcpy(szPrintBuf1, "04 退貨 REFUND");
							strcpy(szPrintBuf2, "");
							break;
						case _CUP_MAIL_ORDER_REFUND_ :
							/* 【需求單 - 103375】銀聯卡MOTO-Recurring add by LingHsiung 2015-10-06 下午 04:35:05 */
							/*
								2015-10-13 (週二) 上午 10:59 吳升文 回覆
								簽單與明細中，銀聯郵購退貨交易及取消銀聯郵購退貨交易的交易類別依下面顯示(中英文)
								銀聯郵購退貨 04 退貨-郵購 REFUND MAIL ORDER
								取消銀聯郵購退貨12 取消-退貨-郵購VOID REFUND MAIL ORDER
							*/
							strcpy(szPrintBuf1, "04 退貨-郵購 ");
							strcpy(szPrintBuf2, "REFUND MAIL ORDER");
							break;
						case _INST_SALE_ :
						case _CUP_INST_SALE_:
							strcpy(szPrintBuf1, "00 一般交易-分期付款 ");
							strcpy(szPrintBuf2, "INST.");
							break;
						case _INST_REFUND_ :
						case _CUP_INST_REFUND_:
							strcpy(szPrintBuf1, "04 退貨-分期付款 ");
							strcpy(szPrintBuf2, "REFUND INST.");
							break;
						case _REDEEM_SALE_ :
						case _CUP_REDEEM_SALE_:
							strcpy(szPrintBuf1, "00 一般交易-紅利扣抵 ");
							strcpy(szPrintBuf2, "REDEEM");
							break;
						case _REDEEM_REFUND_ :
						case _CUP_REDEEM_REFUND_:
							strcpy(szPrintBuf1, "04 退貨-紅利扣抵 ");
							strcpy(szPrintBuf2, "REFUND REDEEM");
							break;
						case _TIP_ :
							strcpy(szPrintBuf1, "11 小費交易 TIPS");
							strcpy(szPrintBuf2, "");
							break;
						case _REDEEM_ADJUST_ :
							strcpy(szPrintBuf1, "00 後台調帳-紅利扣抵");
							strcpy(szPrintBuf2, "ADJUST REDEEM");
							break;
						case _INST_ADJUST_ :
							strcpy(szPrintBuf1, "00 後台調帳-分期付款");
							strcpy(szPrintBuf2, "ADJUST INST.");
							break;
						case _PRE_AUTH_ :
						case _CUP_PRE_AUTH_ :
							strcpy(szPrintBuf1, "01 預先授權 PREAUTH");
							strcpy(szPrintBuf2, "");
							break;
						case _PRE_COMP_ :
						case _CUP_PRE_COMP_ :
							strcpy(szPrintBuf1, "02 預先授權完成 ");
							strcpy(szPrintBuf2, "PREAUTH COMPLETE");
							break;
						case _MAIL_ORDER_ :
						case _CUP_MAIL_ORDER_ :
							strcpy(szPrintBuf1, "03 郵購 MAIL ORDER");
							strcpy(szPrintBuf2, "");
							break;
						case _CASH_ADVANCE_ :
							strcpy(szPrintBuf1, "05 預借現金交易 ");
							strcpy(szPrintBuf2, "CASH ADVANCE");
							break;
						case _FISC_SALE_ :
							strcpy(szPrintBuf1, "2541 消費扣款");
							strcpy(szPrintBuf2, "");
							break;
						case _FISC_REFUND_ :
							strcpy(szPrintBuf1, "2543 退費交易");
							strcpy(szPrintBuf2, "");
							break;
						case _TRUST_SALE_ :
							strcpy(szPrintBuf1, "00 信託交易");
							strcpy(szPrintBuf2, "SALE");
							break;
						default :
							strcpy(szPrintBuf1, "!!!NCCC_正向交易!!!");
							strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");

							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "inCode = %d", pobTran->srBRec.inCode);
								inLogPrintf(AT, szDebugMsg);
							}
							break;
					}
				}
				else
				{
					/* 負向交易 */
					switch (pobTran->srBRec.inOrgCode)
					{
						case _SALE_ :
						case _TIP_ :
						case _CUP_SALE_ :
							strcpy(szPrintBuf1, "11 取消 VOID");
							strcpy(szPrintBuf2, "");
							break;
						case _SALE_OFFLINE_ :
						case _FORCE_CASH_ADVANCE_ :
							strcpy(szPrintBuf1, "11 取消-交易補登 ");
							strcpy(szPrintBuf2, "VOID FORCE");
							break;
						case _REFUND_ :
						case _CUP_REFUND_ :
							strcpy(szPrintBuf1, "12 取消-退貨 ");
							strcpy(szPrintBuf2, "VOID REFUND");
							break;
						case _CUP_MAIL_ORDER_REFUND_ :
							strcpy(szPrintBuf1, "12 取消-退貨-郵購");
							strcpy(szPrintBuf2, "VOID REFUND MAIL ORDER");
							break;
						case _INST_SALE_ :
						case _CUP_INST_SALE_:
							strcpy(szPrintBuf1, "11 取消-分期付款 ");
							strcpy(szPrintBuf2, "VOID INST.");
							break;
						case _REDEEM_SALE_ :
						case _CUP_REDEEM_SALE_:
							strcpy(szPrintBuf1, "11 取消-紅利扣抵 ");
							strcpy(szPrintBuf2, "VOID REDEEM");
							break;
						case _REDEEM_ADJUST_ :
							strcpy(szPrintBuf1, "11 取消-後台調帳-紅利");
							strcpy(szPrintBuf2, "VOID ADJUST REDEEM");
							break;
						case _INST_ADJUST_ :
							strcpy(szPrintBuf1, "11 取消-後台調帳-分期");
							strcpy(szPrintBuf2, "VOID ADJUST INST.");
							break;
						case _REDEEM_REFUND_ :
						case _CUP_REDEEM_REFUND_ :
							strcpy(szPrintBuf1, "12 取消-退貨-紅利扣抵");
							strcpy(szPrintBuf2, "VOID REFUND REDEEM");
							break;
						case _INST_REFUND_ :
						case _CUP_INST_REFUND_:
							strcpy(szPrintBuf1, "12 取消-退貨-分期付款");
							strcpy(szPrintBuf2, "VOID REFUND INST.");
							break;
						case _PRE_AUTH_ :
						case _CUP_PRE_AUTH_ :
							strcpy(szPrintBuf1, "11 取消-預先授權 ");
							strcpy(szPrintBuf2, "VOID PREAUTH");
							break;
						case _CUP_PRE_COMP_ :
						case _PRE_COMP_ :
							strcpy(szPrintBuf1, "11 取消-預先授權完成");
							strcpy(szPrintBuf2, "VOID PREAUTH COMPLETE");
							break;
						case _CASH_ADVANCE_ :
							strcpy(szPrintBuf1, "13 取消-預借現金 ");
							strcpy(szPrintBuf2, "VOID CASH ADVANCE");
							break;
						case _MAIL_ORDER_ :
						case _CUP_MAIL_ORDER_ :
							strcpy(szPrintBuf1, "11 取消-郵購 ");
							strcpy(szPrintBuf2, "VOID MAIL ORDER");
							break;
						case _FISC_SALE_ :
							strcpy(szPrintBuf1, "2542 沖正交易");
							strcpy(szPrintBuf2, "");
							break;
						case _TRUST_VOID_ :
							strcpy(szPrintBuf1, "01 信託交易取消");
							strcpy(szPrintBuf2, "VOID");
							break;
						/* SmartPay退費交易不能取消 */
						default :
							strcpy(szPrintBuf1, "!!!NCCC_負向交易!!!");
							strcpy(szPrintBuf2, "!!!交易類別錯誤!!!");

							if (ginDebug == VS_TRUE)
							{
								memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
								sprintf(szDebugMsg, "inOrgCode = %d", pobTran->srBRec.inOrgCode);
								inLogPrintf(AT, szDebugMsg);
							}
							break;
					}
				}
			}
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetTransType_ESVC
Date&Time       :2018/1/31 下午 1:44
Describe        :取得交易別
*/
int inFunc_GetTransType_ESVC(TRANSACTION_OBJECT *pobTran, char *szPrintBuf)
{
	char	szDebugMsg[100 + 1];
	
	if (pobTran->srTRec.inCode == _TICKET_IPASS_AUTO_TOP_UP_ || 
	    pobTran->srTRec.inCode == _TICKET_EASYCARD_AUTO_TOP_UP_ ||
	    pobTran->srTRec.inCode == _TICKET_ICASH_AUTO_TOP_UP_)
		strcat(szPrintBuf, "自動加值"); 
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_TOP_UP_ || 
		 pobTran->srTRec.inCode == _TICKET_EASYCARD_TOP_UP_ ||
		 pobTran->srTRec.inCode == _TICKET_ICASH_TOP_UP_)       
		strcat(szPrintBuf, "手動加值");         
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_REFUND_ || 
		 pobTran->srTRec.inCode == _TICKET_EASYCARD_REFUND_ ||
		 pobTran->srTRec.inCode == _TICKET_ICASH_REFUND_)       
		strcat(szPrintBuf, "退貨　　"); 
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_DEDUCT_ || 
	         pobTran->srTRec.inCode == _TICKET_EASYCARD_DEDUCT_ ||
	         pobTran->srTRec.inCode == _TICKET_ICASH_DEDUCT_)    
		strcat(szPrintBuf, "購貨　　"); 
	else if (pobTran->srTRec.inCode == _TICKET_IPASS_VOID_TOP_UP_ || 
		 pobTran->srTRec.inCode == _TICKET_EASYCARD_VOID_TOP_UP_ ||
		 pobTran->srTRec.inCode == _TICKET_ICASH_VOID_TOP_UP_)
		strcpy(szPrintBuf, "加值取消");             
	else
	{
		strcat(szPrintBuf, "NO Incode");
		
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "inCode = %d", pobTran->srTRec.inCode);
			inLogPrintf(AT, szDebugMsg);
		}
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_UpdateInvNum
Date&Time       :2015/10/15 上午 10:00
Describe        :Invoice Number加1，並且寫入HDPT.dat
*/
int inFunc_UpdateInvNum(TRANSACTION_OBJECT *pobTran)
{
	int	inNCCC_HostIndex = -1;
	int	inDCC_HostIndex = -1;
	int	inHG_HostIndex = -1;
        char    szInvNum[6 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
        long    lnInvNum = 0;
	long    lnHGInvNum = 0;
	long    lnMaxInv = 0;
	unsigned char		uszFileName[15 + 1] = {};
	BYTE			bNeedSettleBit = VS_FALSE;
	TRANSACTION_OBJECT	pobTempTran = {0};
        
	vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum START!");
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
	{
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
		{
			lnMaxInv = 999;
		}
                /* 【需求單-113110】信託資訊交換平台需求 by Russell 2025/10/7 下午 3:48 */
                else if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
                {
                        lnMaxInv = 999;
                }
		else
		{
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
			{
				lnMaxInv = 997;
			}
			else
			{
				lnMaxInv = 600;
			}
		}
		
	}
	else
	{
		lnMaxInv = 999999;
	}

	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inLoadHDPTRec_failed(%d)", pobTran->srBRec.inHDTIndex);
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }

        lnInvNum = pobTran->srBRec.lnOrgInvNum;
        lnInvNum ++;

        memset(szInvNum, 0x00, sizeof(szInvNum));
        sprintf(szInvNum, "%06ld", lnInvNum);   /* 因為HDPT要補滿6位數，因此補0 */
        if (inSetInvoiceNum(szInvNum) == VS_ERROR)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSetInvoiceNum_failed(%s)", szInvNum);
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSaveHDPTRec_failed(%d)", pobTran->srBRec.inHDTIndex);
		/* 鎖機 */
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }
	
	/* 如果是HG混合交易，則原先Host先在上面累加，接這在下面累加HG 的invoice Number */
	if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
	{
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHG_HostIndex);
		
		if (inLoadHDPTRec(inHG_HostIndex) == VS_ERROR)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inLoadHDPTRec(%d)", inHG_HostIndex);
			/* 鎖機 */
			inFunc_EDCLock(AT);

			return (VS_ERROR);
		}

		memset(szInvNum, 0x00, sizeof(szInvNum));
		inGetInvoiceNum(szInvNum);
		
		lnHGInvNum = atol(szInvNum);
		lnHGInvNum ++;

		sprintf(szInvNum, "%06ld", lnHGInvNum);   /* 因為HDPT要補滿6位數，因此補0 */
		
		if (inSetInvoiceNum(szInvNum) == VS_ERROR)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSetInvoiceNum(%s)", szInvNum);
			/* 鎖機 */
			inFunc_EDCLock(AT);

			return (VS_ERROR);
		}
		
		if (inSaveHDPTRec(inHG_HostIndex) == VS_ERROR)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSaveHDPTRec(%d)", inHG_HostIndex);
			/* 鎖機 */
			inFunc_EDCLock(AT);

			return (VS_ERROR);
		}
		
		/* Load回來 */
		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inLoadHDPTRec(%d)", pobTran->srBRec.inHDTIndex);
			/* 鎖機 */
			inFunc_EDCLock(AT);

			return (VS_ERROR);
		}
		
	}

	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	/* NCCC DCC HG 三個Host index同步 */
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCC_HostIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCC_HostIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHG_HostIndex);
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
		
	/* NCCC 或 DCC 或 HG 更新invoice number就需要同步，
	 * 如果HG混合交易(含NCCC、DINERS)，則原Host Update Invoice Number，HG在接著Update，
	 * 如果為純HG交易，則會選HG Update*/
	if (pobTran->srBRec.inHDTIndex == inNCCC_HostIndex	|| 
	    pobTran->srBRec.inHDTIndex == inDCC_HostIndex	||
	    pobTran->srBRec.inHDTIndex == inHG_HostIndex	||
	    pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
	{
		if (inNCCC_Func_Sync_InvoiceNumber(pobTran) != VS_SUCCESS)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum failed inNCCC_Func_Sync_InvoiceNumber");
			/* 應同步而未同步要鎖機 */
			inFunc_EDCLock(AT);

			return (VS_ERROR);
		}

	}
	
	/* 如果超過最高上限，需要強制結帳 */
        if (lnInvNum > lnMaxInv	||
	   (lnHGInvNum > lnMaxInv && pobTran->srBRec.uszHappyGoMulti == VS_TRUE))
	{
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/7/24 下午 6:17 */
		/* 電子錢包自動歸一 */
		if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
		{
			memset(szInvNum, 0x00, sizeof(szInvNum));
			sprintf(szInvNum, "%06d", 1);   /* 因為HDPT要補滿6位數，因此補0 */

			if (inSetInvoiceNum(szInvNum) == VS_ERROR)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSetInvoiceNum(%s)", szInvNum);
				/* 鎖機 */
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}

			if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
			{
                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSaveHDPTRec(%d)", pobTran->srBRec.inHDTIndex);
				/* 鎖機 */
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}
			
			pobTran->uszUpdateBatchNumBit = VS_TRUE;
		}
		else
		{
			/* 客製化042超過上限要自動歸1 */
			if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
			    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
			{
				memset(szInvNum, 0x00, sizeof(szInvNum));
				sprintf(szInvNum, "%06d", 1);   /* 因為HDPT要補滿6位數，因此補0 */

				if (inSetInvoiceNum(szInvNum) == VS_ERROR)
				{
                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSetInvoiceNum(%s)", szInvNum);
					/* 鎖機 */
					inFunc_EDCLock(AT);

					return (VS_ERROR);
				}

				if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
				{
                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSaveHDPTRec(%d)", pobTran->srBRec.inHDTIndex);
					/* 鎖機 */
					inFunc_EDCLock(AT);

					return (VS_ERROR);
				}
			}
			else
			{
				/* NCCC超過筆數的強制結帳要同步 */
				if (pobTran->srBRec.inHDTIndex == inNCCC_HostIndex	|| 
				    pobTran->srBRec.inHDTIndex == inDCC_HostIndex	||
				    pobTran->srBRec.inHDTIndex == inHG_HostIndex	||
				    pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
				{
					/* NCCC */
					/* 確認有沒有帳，有帳才要結 */
					bNeedSettleBit = VS_FALSE;
					inLoadHDTRec(inNCCC_HostIndex);
					inLoadHDPTRec(inNCCC_HostIndex);
					memset(&pobTempTran, 0x00, sizeof(pobTempTran));
					pobTempTran.srBRec.inHDTIndex = inNCCC_HostIndex;

					/* Check REVERSAL */
					memset(uszFileName, 0x00, sizeof(uszFileName));
					if (inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
					{
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inFunc_ComposeFileName _REVERSAL_FILE_EXTENSION_ failed");
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "組filename失敗：%s", uszFileName);
						}
						return (VS_ERROR);
					}
					if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
					{
						 /* 表示要結帳 */
						 bNeedSettleBit = VS_TRUE;
					}

					/* Check ADVICE */
					memset(uszFileName, 0x00, sizeof(uszFileName));
					if (inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
					{
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inFunc_ComposeFileName _ADVICE_FILE_EXTENSION_ failed");
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "組filename失敗：%s", uszFileName);
						}
						return (VS_ERROR);
					}
					if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
					{
						/* 表示要結帳 */
						bNeedSettleBit = VS_TRUE;
					}

					/* Check ADVICE ESC */
					memset(uszFileName, 0x00, sizeof(uszFileName));
					if (inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
					{
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inFunc_ComposeFileName _ADVICE_ESC_FILE_EXTENSION_ failed");
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "組filename失敗：%s", uszFileName);
						}
						return (VS_ERROR);
					}
					if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
					{
						/* 表示要結帳 */
						bNeedSettleBit = VS_TRUE;
					}

					/* Check Table Exist */
					if (inSqlite_Check_Table_Exist_Flow(&pobTempTran, _TN_BATCH_TABLE_) == VS_SUCCESS)
					{
						/* 表示要結帳 */
						bNeedSettleBit = VS_TRUE;
					}

					if (bNeedSettleBit == VS_TRUE)
					{
						if (inLoadHDPTRec(inNCCC_HostIndex) == VS_ERROR)
						{
                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inLoadHDPTRec(%d) failed", inNCCC_HostIndex);
							/* 鎖機 */
							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}

						inSetMustSettleBit("Y"); /* 有交易筆數限制，表示要結帳 */

						if (inSaveHDPTRec(inNCCC_HostIndex) == VS_ERROR)
						{
                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSaveHDPTRec(%d) failed", inNCCC_HostIndex);
							/* 鎖機 */
							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
					}

					/* DCC */
					/* 確認有沒有帳，有帳才要結 */
					bNeedSettleBit = VS_FALSE;
					inLoadHDTRec(inDCC_HostIndex);
					inLoadHDPTRec(inDCC_HostIndex);
					memset(&pobTempTran, 0x00, sizeof(pobTempTran));
					pobTempTran.srBRec.inHDTIndex = inDCC_HostIndex;

					/* Check REVERSAL */
					memset(uszFileName, 0x00, sizeof(uszFileName));
					if (inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
					{
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inFunc_ComposeFileName _REVERSAL_FILE_EXTENSION_ failed");
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "組filename失敗：%s", uszFileName);
						}
						return (VS_ERROR);
					}
					if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
					{
						 /* 表示要結帳 */
						 bNeedSettleBit = VS_TRUE;
					}

					/* Check ADVICE */
					memset(uszFileName, 0x00, sizeof(uszFileName));
					if (inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
					{
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inFunc_ComposeFileName _ADVICE_FILE_EXTENSION_ failed");
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "組filename失敗：%s", uszFileName);
						}
						return (VS_ERROR);
					}
					if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
					{
						/* 表示要結帳 */
						bNeedSettleBit = VS_TRUE;
					}

					/* Check ADVICE ESC */
					memset(uszFileName, 0x00, sizeof(uszFileName));
					if (inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
					{
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inFunc_ComposeFileName _ADVICE_ESC_FILE_EXTENSION_ failed");
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "組filename失敗：%s", uszFileName);
						}
						return (VS_ERROR);
					}
					if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
					{
						/* 表示要結帳 */
						bNeedSettleBit = VS_TRUE;
					}

					/* Check Table Exist */
					if (inSqlite_Check_Table_Exist_Flow(&pobTempTran, _TN_BATCH_TABLE_) == VS_SUCCESS)
					{
						/* 表示要結帳 */
						bNeedSettleBit = VS_TRUE;
					}

					if (bNeedSettleBit == VS_TRUE)
					{
						if (inLoadHDPTRec(inDCC_HostIndex) == VS_ERROR)
						{
                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inLoadHDPTRec(%d) failed", inDCC_HostIndex);
							/* 鎖機 */
							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}

						inSetMustSettleBit("Y"); /* 有交易筆數限制，表示要結帳 */

						if (inSaveHDPTRec(inDCC_HostIndex) == VS_ERROR)
						{
                                                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSaveHDPTRec(%d) failed", inDCC_HostIndex);
							/* 鎖機 */
							inFunc_EDCLock(AT);

							return (VS_ERROR);
						}
					}

					/* Load回來 */
					if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
					{
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inLoadHDTRec(%d) failed", pobTran->srBRec.inHDTIndex);
						/* 鎖機 */
						inFunc_EDCLock(AT);

						return (VS_ERROR);
					}
					if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
					{
                                                 vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inLoadHDPTRec(%d) failed", pobTran->srBRec.inHDTIndex);
						/* 鎖機 */
						inFunc_EDCLock(AT);

						return (VS_ERROR);
					}
				}
				else
				{
					inSetMustSettleBit("Y"); /* 有交易筆數限制，表示要結帳 */

					if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
					{
                                                vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum inSaveHDPTRec(%d) failed", pobTran->srBRec.inHDTIndex);
						/* 鎖機 */
						inFunc_EDCLock(AT);

						return (VS_ERROR);
					}
				}
			}
		}
	}
	
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
        /* 之後補上強制結帳的動作 */
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateInvNum END!");

        return (VS_SUCCESS);
}

/*
Function        :inFunc_UpdateBatchNum
Date&Time       :2015/10/15 上午 10:00
Describe        :Batch Number加1，並且寫入HDPT.dat
*/
int inFunc_UpdateBatchNum(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inNCCC_HostIndex = -1;
	int	inDCC_HostIndex = -1;
        char    szBatchNum[6 + 1] = {0};
	char	szSwitch[2 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
        long    lnBatchNum = 0;
	long    lnMaxBatch = 0;
	
	vdUtility_SYSFIN_LogMessage(AT, "inFunc_UpdateBatchNum START!");
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_))
		{
			lnMaxBatch = 2;
		}
		else
		{
			lnMaxBatch = 999;
		}
	}
	else
	{
		lnMaxBatch = 999999;
	}

        memset(szBatchNum, 0x00, sizeof(szBatchNum));
	inRetVal = inGetBatchNum(szBatchNum);
        if (inRetVal == VS_ERROR)
        {
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inGetBatchNum fail");
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }

        lnBatchNum = atol(szBatchNum);

        lnBatchNum ++;

	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCC_HostIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCC_HostIndex);
	
        /* 超過999999，歸一 */
        if (lnBatchNum > lnMaxBatch)
	{
		memset(szSwitch, 0x00, sizeof(szSwitch));
		inFunc_Get_ResetBatchNum_Switch(szSwitch);
		if (memcmp(szSwitch, _RESET_BATCH_NONE_, 1) == 0)
		{
			if (pobTran->srBRec.inHDTIndex == inNCCC_HostIndex)
			{
				inFunc_Set_ResetBatchNum_Switch(_RESET_BATCH_NCCC_FIRST_);
			}
			else if (pobTran->srBRec.inHDTIndex == inDCC_HostIndex)
			{
				inFunc_Set_ResetBatchNum_Switch(_RESET_BATCH_DCC_FIRST_);
			}
		}
                lnBatchNum = 1;
	}

        memset(szBatchNum, 0x00, sizeof(szBatchNum));
        sprintf(szBatchNum, "%06ld", lnBatchNum);   /* 因為HDPT要補滿6位數，因此補0 */
        if (inSetBatchNum(szBatchNum) == VS_ERROR)
        {
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inSetBatchNum fail");
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inSaveHDPTRec fail");
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }
	
	/* 現在需要同步的Host有NCCC DCC HG(因為HG不結帳，所以只要NCCC或DCC變動，直接跟NCCC) */
	/* 連動結帳要等到DCC再同步 */
	if (pobTran->uszAutoSettleBit == VS_TRUE)
	{
		if (pobTran->srBRec.inHDTIndex == inNCCC_HostIndex)
		{
			/* 如果是HG 就同步Batch Number */
			if (inHG_Func_Sync_BatchNumber(pobTran) != VS_SUCCESS)
			{
				/* 應同步而未同步要鎖機 */
				inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
				inUtility_StoreTraceLog_OneStep("inHG_Func_Sync_BatchNumber fail");
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}
		}
		else if (pobTran->srBRec.inHDTIndex == inDCC_HostIndex)
		{
			/* 如果是DCC or NCCC 就同步Batch Number */
			if (inNCCC_DCC_Sync_BatchNumber(pobTran) != VS_SUCCESS)
			{
				/* 應同步而未同步要鎖機 */
				inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
				inUtility_StoreTraceLog_OneStep("inNCCC_DCC_Sync_BatchNumber fail");
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}

			/* 如果是HG 就同步Batch Number */
			if (inHG_Func_Sync_BatchNumber(pobTran) != VS_SUCCESS)
			{
				/* 應同步而未同步要鎖機 */
				inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
				inUtility_StoreTraceLog_OneStep("inHG_Func_Sync_BatchNumber fail");
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}
		}
	}
	else
	{
		if (pobTran->srBRec.inHDTIndex == inNCCC_HostIndex	|| 
		    pobTran->srBRec.inHDTIndex == inDCC_HostIndex)
		{
			/* 如果是DCC or NCCC 就同步Batch Number */
			if (inNCCC_DCC_Sync_BatchNumber(pobTran) != VS_SUCCESS)
			{
				/* 應同步而未同步要鎖機 */
				inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
				inUtility_StoreTraceLog_OneStep("inNCCC_DCC_Sync_BatchNumber fail");
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}

			/* 如果是HG 就同步Batch Number */
			if (inHG_Func_Sync_BatchNumber(pobTran) != VS_SUCCESS)
			{
				/* 應同步而未同步要鎖機 */
				inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
				inUtility_StoreTraceLog_OneStep("inHG_Func_Sync_BatchNumber fail");
				inFunc_EDCLock(AT);

				return (VS_ERROR);
			}

		}
	}
        
        /* 標記為已做過 */
        if (pobTran->inRunTRTID == _TRT_SETTLE_ ||
	    pobTran->inRunTRTID == _TRT_TICKET_ECC_SETTLE_)
        {
                inNCCC_Func_Settlement_XML_Edit(_SETTLENMENT_RECOVER_XML_TAG_FUNCTION_UPDATE_BATCH_NUM_, "Y");
        }
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteBatch
Date&Time       :2016/10/4 上午 11:49
Describe        :
*/
int inFunc_DeleteBatch(TRANSACTION_OBJECT *pobTran)
{
	int			i = 0;
	int			inRetVal = VS_ERROR;
        char			szHostName[8 + 1] = {0};
	char			szTRTFName[16 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
        char                    szDirPath[100 + 1] = {0};
        char                    szFileHostName[8 + 1] = {0};
        unsigned char		uszFileName[20 + 1] = {0};
	unsigned char		uszNotDeleteAllSignatureBit = VS_FALSE;
	TRANSACTION_OBJECT	pobTranTemp = {};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFunc_DeleteBatch() START !");
        }
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)	||
	     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)) && 
	     (pobTran->inRunOperationID != _OPERATION_FUN5_TMS_DOWNLOAD_ && 
	      pobTran->inRunOperationID != _OPERATION_FUN6_TMS_DOWNLOAD_ && 
	      pobTran->inRunOperationID != _OPERATION_TMS_SCHEDULE_DOWNLOAD_))
	{
		uszNotDeleteAllSignatureBit = VS_TRUE;
	}

	if (pobTran->uszNotDispMsgBit == VS_TRUE)
	{
		
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_RESET_BATCH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <清除批次> */
	}
	
	memset(szHostName, 0x00, sizeof(szHostName));
	if (inGetHostLabel(szHostName) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (pobTran->uszNotDispMsgBit == VS_TRUE)
	{
		
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示刪除批次請勿關機 */
		inDISP_PutGraphic(_DELETE_BATCH_, 0, _COORDINATE_Y_LINE_8_6_);
		/* 第二行中文提示 */
		inDISP_ChineseFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
		/* 延遲一秒 */
		inDISP_Wait(1000);
	}
	
	/* 刪除該host所有的簽名圖檔 因為調帳的簽名圖檔不一樣所以要多砍 */
	/* 這邊改incode只是為了組檔名用的， 所以砍完要設定回來 */
	memset(uszFileName, 0x00, sizeof(uszFileName));
	
	/* Load Batch Record */
	/* 這邊用pobTranTemp是避免影響後面流程(連動結帳)，會把之前的資料讀出來 */
	memset(&pobTranTemp, 0x00, sizeof(pobTranTemp));
	memcpy(&pobTranTemp, pobTran, sizeof(TRANSACTION_OBJECT));
	
	if (uszNotDeleteAllSignatureBit == VS_TRUE)
	{
		
	}
	else
	{
                memset(szFileHostName, 0x00, sizeof(szFileHostName));
                inFunc_HostName_DecideByTRT(&pobTranTemp, szFileHostName);
                memset(szDirPath, 0x00, sizeof(szDirPath));
                snprintf(szDirPath, sizeof(szDirPath), "%s%s", _AP_ROOT_PATH_, _FS_DIR_NAME_);
                inNCCC_Func_Delete_Signature_By_Shell_Command(szDirPath, szFileHostName);
	}
	
	/* 取出TRT FileName */
        memset(szTRTFName, 0x00, sizeof(szTRTFName));
        if (inGetTRTFileName(szTRTFName) == VS_ERROR)
                return (VS_ERROR);
	/* 票證要砍四次 */
	if (!memcmp(szTRTFName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
	{
		inRetVal = inSqlite_Drop_Table_Flow(&pobTranTemp, _TN_BATCH_TABLE_TICKET_);
		if (inRetVal != VS_SUCCESS)
		{

		}
		
		for (i = 0; i < _TDT_INDEX_MAX_; i++)
		{
			switch (i)
			{
				case 0:
					pobTranTemp.srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
					break;
				case 1:
					pobTranTemp.srTRec.inTDTIndex = _TDT_INDEX_01_ECC_;
					break;
				case 2:
					pobTranTemp.srTRec.inTDTIndex = _TDT_INDEX_02_ICASH_;
					break;
				default:
					pobTranTemp.srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
					break;
			}
			inLoadTDTRec(pobTranTemp.srTRec.inTDTIndex);
			
			/* Delete REVERSAL */
			memset(uszFileName, 0x00, sizeof(uszFileName));
			/* Reversal各家存各家的 */
			pobTranTemp.uszESVCFileNameByCardBit = VS_TRUE;
			inRetVal = inFunc_ComposeFileName(&pobTranTemp, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6);
			pobTranTemp.uszESVCFileNameByCardBit = VS_FALSE;
			if (inRetVal != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			if (inFILE_Delete(uszFileName) == VS_SUCCESS)
			{
				inSetTicket_ReversalBit("N");
				inSaveTDTRec(pobTranTemp.srTRec.inTDTIndex);
			}

			/* Delete ADVICE */
			memset(uszFileName, 0x00, sizeof(uszFileName));
			if (inFunc_ComposeFileName(&pobTranTemp, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			inFILE_Delete(uszFileName);

			/* Delete ADVICE ESC */
			memset(uszFileName, 0x00, sizeof(uszFileName));
			if (inFunc_ComposeFileName(&pobTranTemp, (char*)uszFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
			{
				return (VS_ERROR);
			}
			inFILE_Delete(uszFileName);

			/* 票證advice分開存，要分開砍 */
			pobTranTemp.uszESVCFileNameByCardBit = VS_TRUE;
			inRetVal = inSqlite_Drop_Table_Flow(&pobTranTemp, _TN_BATCH_TABLE_TICKET_ADVICE_);
			pobTranTemp.uszESVCFileNameByCardBit = VS_FALSE;
			if (inRetVal != VS_SUCCESS)
			{

			}
		}
	}
        else if (!memcmp(szTRTFName, _TRT_FILE_NAME_TRUST_, strlen(_TRT_FILE_NAME_TRUST_)))
        {
                /* Delete REVERSAL DialBackup */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(&pobTranTemp, (char*)uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Delete(uszFileName) == VS_SUCCESS)
		{

		}
		
		/* Delete REVERSAL */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(&pobTranTemp, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Delete(uszFileName) == VS_SUCCESS)
		{
			inSetSendReversalBit("N");
			inSaveHDPTRec(pobTranTemp.srBRec.inHDTIndex);
		}

		/* Delete ADVICE */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(&pobTranTemp, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		inFILE_Delete(uszFileName);

		/* 刪除該host的Table */
		inRetVal = inSqlite_Drop_Table_Flow(pobTran, _TN_BATCH_TABLE_TRUST_);
                if (inRetVal != VS_SUCCESS)
                {

                }
        }
	else
	{
		/* Delete REVERSAL DialBackup */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(&pobTranTemp, (char*)uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Delete(uszFileName) == VS_SUCCESS)
		{

		}
		
		/* Delete REVERSAL */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(&pobTranTemp, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		if (inFILE_Delete(uszFileName) == VS_SUCCESS)
		{
			inSetSendReversalBit("N");
			inSaveHDPTRec(pobTranTemp.srBRec.inHDTIndex);
		}

		/* Delete ADVICE */
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(&pobTranTemp, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		inFILE_Delete(uszFileName);

		/* 刪除該host的Table */
		inFunc_DeleteBatchTable(&pobTranTemp);
	}
	
	/* 特例處理 START */
	/* 如果是NCCC要多砍優惠平台檔案和HappyGo的帳 */
	if (memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
	{
		/* 優惠平台檔案 */
		if (inFILE_Check_Exist((unsigned char*)_REWARD_FILE_NAME_) == VS_SUCCESS)
		{
			inFILE_Delete((unsigned char*)_REWARD_FILE_NAME_);
		}
		
		/* ESC暫存檔 */
		pobTranTemp.uszFileNameNoNeedHostBit = VS_TRUE;
		pobTranTemp.uszFileNameNoNeedNumBit = VS_TRUE;
		inSqlite_Drop_Table_Flow(&pobTranTemp, _TN_BATCH_TABLE_ESC_TEMP_);
		pobTranTemp.uszFileNameNoNeedHostBit = VS_FALSE;
		pobTranTemp.uszFileNameNoNeedNumBit = VS_FALSE;
	}
	/* 票證要刪除殘存檔 */
	else if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
	{
		inFile_Unlink_File("ICERAPI_CMAS.adv", _ECC_FOLDER_PATH_);
		inFile_Unlink_File("ICERAPI_CMAS.rev", _ECC_FOLDER_PATH_);
		inFile_Unlink_File("ICERAPI_CMAS.rev.bak", _ECC_FOLDER_PATH_);
	}
	/* 特例處理 END */
	
	/* 關閉Settle Bit */
	inSetMustSettleBit("N");
	inSaveHDPTRec(pobTranTemp.srBRec.inHDTIndex);
	
	/* 關閉續傳 Bit */
	inSetCLS_SettleBit("N");
	inSaveHDPTRec(pobTranTemp.srBRec.inHDTIndex);
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFunc_DeleteBatch() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteBatch_Flow
Date&Time       :2017/3/7 下午 1:53
Describe        :刪除批次，處理例外
*/
int inFunc_DeleteBatch_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inHGIndex = -1;
	int	inOrgIndex = -1;
        char    szHostName[8 + 1] = {0};
	char	szFuncEable[2 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFunc_DeleteBatch_Flow() START !");
        }

        vdUtility_SYSFIN_LogMessage(AT, "inFunc_DeleteBatch_Flow START!");
        
	/* 刪除批次 */
	inFunc_DeleteBatch(pobTran);
	
	memset(szHostName, 0x00, sizeof(szHostName));
	if (inGetHostLabel(szHostName) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	/* 特例處理 START */
	/* 如果是NCCC要多砍優惠平台檔案和HappyGo的帳 */
	if (memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
	{
		/* HappyGo的帳 */
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHGIndex);
		if (inHGIndex == -1)
		{

		}
		else
		{
			inOrgIndex = pobTran->srBRec.inHDTIndex;
			pobTran->srBRec.inHDTIndex = inHGIndex;

			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			memset(szFuncEable, 0x00, sizeof(szFuncEable));
			inGetHostEnable(szFuncEable);
			if (memcmp(szFuncEable, "Y", strlen("Y")) == 0)
			{
				inFunc_DeleteBatch(pobTran);
			}

			pobTran->srBRec.inHDTIndex = inOrgIndex;
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		}
	}
	
	/* 特例處理 END */
	
	/* 標記為已做過 */
        if (pobTran->inRunTRTID == _TRT_SETTLE_ ||
	    pobTran->inRunTRTID == _TRT_TICKET_ECC_SETTLE_)
        {
                inNCCC_Func_Settlement_XML_Edit(_SETTLENMENT_RECOVER_XML_TAG_FUNCTION_DELETE_BATCH_FLOW_, "Y");
        }
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFunc_DeleteBatch_Flow() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        
	/* 清除畫面 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteAccum
Date&Time       :2016/10/4 上午 11:49
Describe        :
*/
int inFunc_DeleteAccum(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
        char            szHostName[8 + 1];
        unsigned char   uszFileName[20 + 1];

	if (pobTran->uszNotDispMsgBit == VS_TRUE)
	{
		
	}
	else
	{
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_RESET_BATCH_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <清除批次> */
	}
	
	memset(szHostName, 0x00, sizeof(szHostName));
	inRetVal = inGetHostLabel(szHostName);
        if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inGetHostLabel fail");
		inFunc_EDCLock(AT);
		
		return (inRetVal);
	}

	if (pobTran->uszNotDispMsgBit == VS_TRUE)
	{
		
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		/* 顯示刪除批次請勿關機 */
		inDISP_PutGraphic(_DELETE_BATCH_, 0, _COORDINATE_Y_LINE_8_6_);
		/* 第二行中文提示 */
		inDISP_ChineseFont(szHostName, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

		/* 延遲一秒 */
		inDISP_Wait(1000);
	}

        /* Delete amt */
        memset(uszFileName, 0x00, sizeof(uszFileName));
	inRetVal = inFunc_ComposeFileName(pobTran, (char*)uszFileName, _ACCUM_FILE_EXTENSION_, 6);
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inFunc_ComposeFileName fail");
		inFunc_EDCLock(AT);
		
		return (VS_ERROR);
	}
	
        inRetVal = inFILE_Delete(uszFileName);
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inFILE_Delete fail");
		inFunc_EDCLock(AT);
		
		return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteAccum_Flow
Date&Time       :2017/3/7 下午 2:00
Describe        :刪除金額檔，並處理例外
*/
int inFunc_DeleteAccum_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inHGIndex = -1;
	int	inOrgIndex = -1;
        char    szHostName[8 + 1] = {0};
	char	szFuncEable[2 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_DeleteAccum_Flow START!");

	inRetVal = inFunc_DeleteAccum(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inFunc_DeleteAccum fail");
		inFunc_EDCLock(AT);
		
		return (inRetVal);
	}
	
	memset(szHostName, 0x00, sizeof(szHostName));
	inRetVal = inGetHostLabel(szHostName);
	if (inRetVal != VS_SUCCESS)
	{
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inGetHostLabel fail");
		inFunc_EDCLock(AT);
		
		return (VS_ERROR);
	}
	/* 特例處理 START */
	/* 如果是NCCC要多砍HappyGo總帳檔案 */
	if (memcmp(szHostName, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
	{
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHGIndex);
		if (inHGIndex == -1)
		{
			
		}
		else
		{
			inOrgIndex = pobTran->srBRec.inHDTIndex;
			pobTran->srBRec.inHDTIndex = inHGIndex;

			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);

			memset(szFuncEable, 0x00, sizeof(szFuncEable));
			inGetHostEnable(szFuncEable);
			if (memcmp(szFuncEable, "Y", strlen("Y")) == 0)
			{
				inRetVal = inFunc_DeleteAccum(pobTran);
				if (inRetVal != VS_SUCCESS)
				{
					inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
					inUtility_StoreTraceLog_OneStep("inFunc_DeleteAccum fail");
					inFunc_EDCLock(AT);

					return (inRetVal);
				}
			}

			pobTran->srBRec.inHDTIndex = inOrgIndex;
			inLoadHDTRec(pobTran->srBRec.inHDTIndex);
			inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		}
	}
	/* 特例處理 END */

        /* 標記為已做過 */
        if (pobTran->inRunTRTID == _TRT_SETTLE_ ||
	    pobTran->inRunTRTID == _TRT_TICKET_ECC_SETTLE_)
        {
            inNCCC_Func_Settlement_XML_Edit(_SETTLENMENT_RECOVER_XML_TAG_FUNCTION_DELETE_ACCUM_FLOW_, "Y");
        }
        
	if (pobTran->uszNotDispMsgBit == VS_TRUE)
	{
		
	}
	else
	{
		/* 清除畫面 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inFunc_CreateAllBatchTable
Date&Time       :2016/12/7 上午 9:54
Describe        :建立所有Host有開的Table
*/
int inFunc_CreateAllBatchTable(void)
{
	int			i;
	int			inRetVal;
	char			szHostEnable[2 + 1];
	TRANSACTION_OBJECT	pobTran;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_CreateAllBatchTable() START !");
	}
	
	memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
	
	for (i = 0; ; i++)
	{
		if (inLoadHDTRec(i) < 0)
		{
			break;
		}
		pobTran.srBRec.inHDTIndex = i;
		
		inGetHostEnable(szHostEnable);
		
		if (memcmp(szHostEnable, "Y", 1) == 0)
		{
			inRetVal = inFunc_CreateNewBatchTable(&pobTran);
			if (inRetVal != VS_SUCCESS)
			{
				inFunc_EDCLock(AT);
				
				return (VS_ERROR);
			}
			
		}
		else
		{
			/* 沒開Host跳過 */
			continue;
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_CreateAllBatchTable() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteAllBatchTable
Date&Time       :2016/12/7 上午 9:54
Describe        :刪除所有Host的Table
*/
int inFunc_DeleteAllBatchTable(void)
{
	int			i;
	TRANSACTION_OBJECT	pobTran;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_DeleteAllBatchTable() START !");
	}
	
	memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
	
	for (i = 0; ; i++)
	{
		if (inLoadHDTRec(i) < 0)
		{
			break;
		}
		
		pobTran.srBRec.inHDTIndex = i;

		inFunc_DeleteBatchTable(&pobTran);

	}
	
	/* ESC暫存檔 */
	pobTran.uszFileNameNoNeedHostBit = VS_TRUE;
	pobTran.uszFileNameNoNeedNumBit = VS_TRUE;
	inSqlite_Drop_Table_Flow(&pobTran, _TN_BATCH_TABLE_ESC_TEMP_);
	pobTran.uszFileNameNoNeedHostBit = VS_FALSE;
	pobTran.uszFileNameNoNeedNumBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_DeleteAllBatchTable() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CreateNewBatchTable
Date&Time       :2016/12/7 上午 9:54
Describe        :結帳完建新的Batch Table
*/
int inFunc_CreateNewBatchTable(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	/* 結帳完建新的Batch Table */
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_EMV_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	inRetVal = inBATCH_Create_BatchTable_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_DeleteBatchTable
Date&Time       :2016/12/7 上午 9:54
Describe        :刪除Batch Table
*/
int inFunc_DeleteBatchTable(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
	inRetVal = inSqlite_Drop_Table_Flow(pobTran, _TN_BATCH_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		
	}
	
	inRetVal = inSqlite_Drop_Table_Flow(pobTran, _TN_EMV_TABLE_);
	if (inRetVal != VS_SUCCESS)
	{
		
	}
	
	inRetVal = inSqlite_Drop_Table_Flow(pobTran, _TN_BATCH_TABLE_ESC_AGAIN_);
	if (inRetVal != VS_SUCCESS)
	{
		
	}
	
	inRetVal = inSqlite_Drop_Table_Flow(pobTran, _TN_BATCH_TABLE_ESC_FAIL_);
	if (inRetVal != VS_SUCCESS)
	{
		
	}
	
	/* Delete ADVICE ESC */
	inRetVal = inSqlite_Drop_Table_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_);
	if (inRetVal != VS_SUCCESS)
	{
		
	}
	
	/* Delete ADVICE ESC */
	inRetVal = inSqlite_Drop_Table_Flow(pobTran, _TN_BATCH_TABLE_ESC_ADVICE_EMV_);
	if (inRetVal != VS_SUCCESS)
	{
		
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_ResetBatchInvNum
Date&Time       :2016/10/4 上午 11:50
Describe        :結完帳時用或手動砍批
*/
int inFunc_ResetBatchInvNum(TRANSACTION_OBJECT *pobTran)
{
	int	inNCCC_HostIndex = -1;
	int	inDCC_HostIndex = -1;
	int	inHG_HostIndex = -1;
        char    szInvNum[6 + 1];

	vdUtility_SYSFIN_LogMessage(AT, "inFunc_ResetBatchInvNum START!");
	
        /* GET_HOST_NUM時就已經load HDPT了 */

        /* 結完帳，InvNum歸一 */
        memset(szInvNum, 0x00, sizeof(szInvNum));
        sprintf(szInvNum, "%06d", 1);   /* 因為HDPT要補滿6位數，因此補0 */

        if (inSetInvoiceNum(szInvNum) == VS_ERROR)
        {
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inSetInvoiceNum fail");
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }
	
	/* 票證結帳需要歸一，其他Host沒影響所以一起歸一 */
	if (inSetTicket_InvNum(szInvNum) == VS_ERROR)
        {
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inSetTicket_InvNum fail");
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }
	
	/* ATS結帳〈930000〉 – 前次不平帳上傳失敗 */
	if (inSetCLS_SettleBit("N") == VS_ERROR)
        {
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inSetCLS_SettleBit fail");
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
		inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
		inUtility_StoreTraceLog_OneStep("inSaveHDPTRec fail");
		inFunc_EDCLock(AT);
		
                return (VS_ERROR);
        }
	
	/* NCCC DCC HG 三個Host index同步 */
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCC_HostIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCC_HostIndex);
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHG_HostIndex);
	
	/* NCCC 或 DCC 或 HG 更新invoice number就需要同步，
	 * 如果HG混合交易(含NCCC、DINERS)，則原Host Update Invoice Number，HG在接著Update，
	 * 如果為純HG交易，則會選HG Update*/
	if (pobTran->srBRec.inHDTIndex == inNCCC_HostIndex	|| 
	    pobTran->srBRec.inHDTIndex == inDCC_HostIndex	||
	    pobTran->srBRec.inHDTIndex == inHG_HostIndex	||
	    pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
	{
		if (inNCCC_Func_Sync_Reset_InvoiceNumber(pobTran) != VS_SUCCESS)
		{
			inUtility_StoreTraceLog_OneStep("%s:%d", __func__, __LINE__);
			inUtility_StoreTraceLog_OneStep("inNCCC_Func_Sync_Reset_InvoiceNumber fail");
			/* 應同步而未同步要鎖機 */
			inFunc_EDCLock(AT);

			return (VS_ERROR);
		}

	}
	
	ginNCCCDCCInvoice = 0;
	
        /* 標記為已做過 */
        if (pobTran->inRunTRTID == _TRT_SETTLE_ ||
	    pobTran->inRunTRTID == _TRT_TICKET_ECC_SETTLE_)
        {
                inNCCC_Func_Settlement_XML_Edit(_SETTLENMENT_RECOVER_XML_TAG_FUNCTION_REST_BATCH_INV_, "Y");
        }
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetSystemDateAndTime
Date&Time       :2016/10/21 上午 11:42
Describe        :Get出端末機系統日期時間，放到自定義的結構
*/
int inFunc_GetSystemDateAndTime(RTC_NEXSYS *srRTC)
{
	int		inRetVal = 0;
	char		szDebugMsg[100 + 1] = {0};
	CTOS_RTC	srSysRTC;
	
	memset(&srSysRTC, 0x00, sizeof(srSysRTC));
	inRetVal = CTOS_RTCGet(&srSysRTC);
	
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RTCGet Failed inRetVal: 0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		srRTC->uszYear = srSysRTC.bYear;
		srRTC->uszMonth = srSysRTC.bMonth;
		srRTC->uszDay = srSysRTC.bDay;
		srRTC->uszHour = srSysRTC.bHour;
		srRTC->uszMinute = srSysRTC.bMinute;
		srRTC->uszSecond = srSysRTC.bSecond;
		
		/* 不一定每一種機型都Get的到星期幾，所以可以斟酌是否要這個 */
		srRTC->uszDoW = srSysRTC.bDoW;

		return (VS_SUCCESS);
	}
	
}

/*
Function        :inFunc_SyncPobTran_Date_Include_Year
Date&Time       :2018/2/8 上午 9:40
Describe        :將傳進的RTC的時間部份，傳進放進來的Buffer,因為只傳pointer，所以要求傳進長度避免爆掉
 *		:這個會連20一起填
*/
int inFunc_SyncPobTran_Date_Include_Year(char *szDate,int inTimeBufferLen, RTC_NEXSYS *srRTC)
{	
	if (inTimeBufferLen >= 8 + 1)
	{
		sprintf(szDate, "20%02d%02d%02d",  srRTC->uszYear, srRTC->uszMonth, srRTC->uszDay);
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_SyncPobTran_Date
Date&Time       :2018/2/8 上午 9:40
Describe        :將傳進的RTC的時間部份，傳進放進來的Buffer,因為只傳pointer，所以要求傳進長度避免爆掉
*/
int inFunc_SyncPobTran_Date(char *szDate,int inTimeBufferLen, RTC_NEXSYS *srRTC)
{	
	if (inTimeBufferLen >= 6 + 1)
	{
		sprintf(szDate, "%02d%02d%02d",  srRTC->uszYear, srRTC->uszMonth, srRTC->uszDay);
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_SyncPobTran_Time
Date&Time       :2018/2/8 上午 9:40
Describe        :將傳進的RTC的時間部份，傳進放進來的Buffer,因為只傳pointer，所以要求傳進長度避免爆掉
*/
int inFunc_SyncPobTran_Time(char *szTime,int inTimeBufferLen, RTC_NEXSYS *srRTC)
{	
	if (inTimeBufferLen >= 6 + 1)
	{
		sprintf(szTime, "%02d%02d%02d",  srRTC->uszHour, srRTC->uszMinute, srRTC->uszSecond);
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_Sync_BRec_Date_Time
Date&Time       :2018/2/8 上午 9:44
Describe        :同步BRec日期和時間
*/
int inFunc_Sync_BRec_Date_Time(TRANSACTION_OBJECT *pobTran, RTC_NEXSYS *srRTC)
{	
	/* 同步到pobTran */
	memset(pobTran->srBRec.szDate, 0x00, sizeof(pobTran->srBRec.szDate));
	memset(pobTran->srBRec.szOrgDate, 0x00, sizeof(pobTran->srBRec.szOrgDate));
	memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
	memset(pobTran->srBRec.szOrgTime, 0x00, sizeof(pobTran->srBRec.szOrgTime));
	
	if (inFunc_SyncPobTran_Date_Include_Year(pobTran->srBRec.szDate, sizeof(pobTran->srBRec.szDate),  srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_SyncPobTran_Date_Include_Year(pobTran->srBRec.szOrgDate, sizeof(pobTran->srBRec.szOrgDate),  srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_SyncPobTran_Time(pobTran->srBRec.szTime, sizeof(pobTran->srBRec.szTime),  srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_SyncPobTran_Time(pobTran->srBRec.szOrgTime, sizeof(pobTran->srBRec.szOrgTime),  srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_TRec_Date_Time
Date&Time       :2018/2/8 上午 9:44
Describe        :同步TRec日期和時間
*/
int inFunc_Sync_TRec_Date_Time(TRANSACTION_OBJECT *pobTran, RTC_NEXSYS *srRTC)
{	
	/* 同步到pobTran */
	memset(pobTran->srTRec.szDate, 0x00, sizeof(pobTran->srTRec.szDate));
	memset(pobTran->srTRec.szOrgDate, 0x00, sizeof(pobTran->srTRec.szOrgDate));
	memset(pobTran->srTRec.szTime, 0x00, sizeof(pobTran->srTRec.szTime));
	memset(pobTran->srTRec.szOrgTime, 0x00, sizeof(pobTran->srTRec.szOrgTime));
	
	if (inFunc_SyncPobTran_Date(pobTran->srTRec.szDate, sizeof(pobTran->srTRec.szDate),  srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_SyncPobTran_Date(pobTran->srTRec.szOrgDate, sizeof(pobTran->srTRec.szOrgDate),  srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_SyncPobTran_Time(pobTran->srTRec.szTime, sizeof(pobTran->srTRec.szTime),  srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inFunc_SyncPobTran_Time(pobTran->srTRec.szOrgTime, sizeof(pobTran->srTRec.szOrgTime),  srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_SetEDCDateTime
Date&Time       :2016/11/25 下午 1:50
Describe        :更新端末機日期時間 輸入YYYYMMDD HHMMSS
*/
int inFunc_SetEDCDateTime(char *szDate, char *szTime)
{
        int		inYear, inMonth, inDay, inHour, inMinute, inSecond;
	char		szTemplate[8 + 1];
	char		szDebugMsg[100 + 1];
	unsigned short	usRet;
        CTOS_RTC	SetRTC;
        
        memset(szTemplate, 0x00, sizeof(szTemplate));
        /* 年 */
        memcpy(&szTemplate[0], &szDate[2], 2);
        inYear = atoi(szTemplate);
        SetRTC.bYear = inYear;
        /* 月 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(&szTemplate[0], &szDate[4], 2);
        inMonth = atoi(szTemplate);
        SetRTC.bMonth = inMonth;
        /* 日 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(&szTemplate[0], &szDate[6], 2);
        inDay = atoi(szTemplate);
        SetRTC.bDay = inDay;
        /* 小時 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(&szTemplate[0], &szTime[0], 2);
        inHour = atoi(szTemplate);
        SetRTC.bHour = inHour;
        /* 分鐘 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(&szTemplate[0], &szTime[2], 2);
        inMinute = atoi(szTemplate);
        SetRTC.bMinute = inMinute;
        /* 秒 */
        memset(szTemplate, 0x00, sizeof(szTemplate));
        memcpy(&szTemplate[0], &szTime[4], 2);
        inSecond = atoi(szTemplate);
        SetRTC.bSecond = inSecond;
        
        /* 更改EDC時間 */
        usRet = CTOS_RTCSet(&SetRTC);
	
	if (usRet != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "時間同步失敗 錯誤代碼：%x", usRet);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Fun3EditDateTime
Date&Time       :2017/7/6 上午 10:28
Describe        :
*/
int inFunc_Fun3EditDateTime(void)
{
        int		inRetVal;
        char		szDate[8 + 1], szTime[6 + 1];
	char		szDispayMsg[8 + 1];
        unsigned char   uszKey;
        DISPLAY_OBJECT  srDispObj;
	RTC_NEXSYS	stRTC;
        
	memset(&stRTC, 0x00, sizeof(stRTC));
	inFunc_GetSystemDateAndTime(&stRTC);
		
        /* 日期設定 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_SET_DATE_, 0, _COORDINATE_Y_LINE_8_4_);
	
	memset(szDispayMsg, 0x00, sizeof(szDispayMsg));
	sprintf(szDispayMsg, "20%02d%02d%02d", stRTC.uszYear, stRTC.uszMonth, stRTC.uszDay);
	inDISP_ChineseFont(szDispayMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	
        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 8;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
		strcpy(srDispObj.szOutput, szDispayMsg);
		srDispObj.inOutputLen = strlen(szDispayMsg);
		inDISP_ChineseFont_Color(srDispObj.szOutput, _FONTSIZE_8X22_, _LINE_8_7_, srDispObj.inColor, _DISP_RIGHT_);
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) != 8)
                {
                        inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                        continue;
                }
                else
                {
                        memset(szDate, 0x00, sizeof(szDate));
                        memcpy(&szDate[0], &srDispObj.szOutput[0], 8);
                        break;
                }
        }
        
        /* 檢核日期 */
        inRetVal = inFunc_CheckValidDate_Include_Year(szDate);
        
        if (inRetVal != VS_SUCCESS)
        {
                /* 提示錯誤訊息 */
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_PutGraphic(_ERR_DATE_TIME_, 0, _COORDINATE_Y_LINE_8_4_);
                inDISP_BEEP(1, 0);
                
                while (1)
                {
                        uszKey = uszKBD_GetKey(30);
                        
                        if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
                                return (VS_ERROR);
                        else
                                continue;
                }
        }
        
        /* 時間設定 */
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_SET_TIME_, 0, _COORDINATE_Y_LINE_8_4_);
	
	memset(szDispayMsg, 0x00, sizeof(szDispayMsg));
	sprintf(szDispayMsg, "%02d%02d%02d", stRTC.uszHour, stRTC.uszMinute, stRTC.uszSecond);
	inDISP_ChineseFont(szDispayMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
        
        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 6;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
		strcpy(srDispObj.szOutput, szDispayMsg);
		srDispObj.inOutputLen = strlen(szDispayMsg);
		inDISP_ChineseFont_Color(srDispObj.szOutput, _FONTSIZE_8X22_, _LINE_8_7_, srDispObj.inColor, _DISP_RIGHT_);
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) != 6)
                {
                        inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                        continue;
                }
                else
                {
                        memset(szTime, 0x00, sizeof(szTime));
                        memcpy(&szTime[0], &srDispObj.szOutput[0], 6);
                        break;
                }
        }
        
        /* 檢核時間 */
        inRetVal = inFunc_CheckValidTime(szTime);
        
        if (inRetVal != VS_SUCCESS)
        {
                /* 提示錯誤訊息 */
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                inDISP_PutGraphic(_ERR_DATE_TIME_, 0, _COORDINATE_Y_LINE_8_4_);
                inDISP_BEEP(1, 0);
                
                while (1)
                {
                        uszKey = uszKBD_GetKey(30);
                        
                        if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
                                return (VS_ERROR);
                        else
                                continue;
                }
        }
        
        inFunc_SetEDCDateTime(szDate, szTime);
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckValidDate_Include_Year
Date&Time       :2016/11/25 下午 2:59
Describe        :確認日期是否合法
*/
int inFunc_CheckValidDate_Include_Year(char *szValidDate)
{
    	char	szTemplate[14 + 1];
    	int	inDay = 0, inMon = 0, inYear = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_CheckValidDate_Include_Year() START !");
	}
	
	/* 小於八碼不合法 */
	if (strlen(szValidDate) != 8)
	{
		return (VS_ERROR);
	}

    	/* Get 4-Digit Year */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[0], 4);
    	inYear = atoi(szTemplate);
    	/*  Get 2-Digit Month */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[4], 2);
    	inMon = atoi(szTemplate);
    	/*  Get 2-Digit Day */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[6], 2);
    	inDay = atoi(szTemplate);

    	if ((inYear < 2000) || (inYear > 2088))
        	return (VS_ERROR);

    	if ((inMon < 1) || (inMon > 12))
        	return (VS_ERROR);

    	if ((inDay < 1) || (inDay > 31))
        	return (VS_ERROR);

        switch (inMon)
        {
                case 4 :
                case 6 :
                case 9 :
                case 11 :
                        if (inDay > 30)
                            return (VS_ERROR);
                        break;
                case 2:
                        if (inYear % 4 == 0) /* 判斷閏年 */
                        {
                                if (inDay > 29)
                                {
                                        return (VS_ERROR);
                                }
                        }
                        else
                        {
                                if (inDay > 28)
                                {
                                        return (VS_ERROR);
                                }
                        }
                        break;
                default :
                        break;
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_CheckValidDate_Include_Year() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

    	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckValidOriDate
Date&Time       :2018/2/13 下午 3:36
Describe        :確認原交易日期是否合法
*/
int inFunc_CheckValidOriDate(char *szValidDate)
{
	int		inDay = 0, inMon = 0, inYear = 0;
    	char		szTemplate[8 + 1];
	RTC_NEXSYS	srRTC = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_CheckValidOriDate() START !");
	}
	
	/* 小於四碼不合法，例如一月二十 要輸入0120而不能輸入120 */
	if (strlen(szValidDate) != 4)
	{
		return (VS_ERROR);
	}
	
    	/*  Get 2-Digit Month */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[0], 2);
    	inMon = atoi(szTemplate);
    	/*  Get 2-Digit Day */
    	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szValidDate[2], 2);
    	inDay = atoi(szTemplate);
	
	/* 判斷去年、今年 */
	inFunc_GetSystemDateAndTime(&srRTC);
	if (inMon == srRTC.uszMonth)
	{
		if (inDay > srRTC.uszDay)
		{
			/* 去年 */
			inYear = srRTC.uszYear - 1;
		}
		else
		{
			/* 今年 */
			inYear = srRTC.uszYear;
		}
	}
	else if (inMon < srRTC.uszMonth)
	{
		/* 今年 */
		inYear = srRTC.uszYear;
	}
	else
	{
		/* 去年 */
		inYear = srRTC.uszYear - 1;
	}

    	if ((inMon < 1) || (inMon > 12))
        	return (VS_ERROR);

    	if ((inDay < 1) || (inDay > 31))
        	return (VS_ERROR);

        switch (inMon)
        {
                case 4 :
                case 6 :
                case 9 :
                case 11 :
                        if (inDay > 30)
                            return (VS_ERROR);
                        break;
                case 2:
                        if (inYear % 4 == 0) /* 判斷閏年 */
                        {
                                if (inDay > 29)
                                {
                                        return (VS_ERROR);
                                }
                        }
                        else
                        {
                                if (inDay > 28)
                                {
                                        return (VS_ERROR);
                                }
                        }
                        break;
                default :
                        break;
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_CheckValidOriDate() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

    	return (VS_SUCCESS);
}

int inFunc_CheckValidTime(char *szTime)
{
	char	szTemplate[4 + 1];
	int 	inMins = 0, inHours = 0, inSecs = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_CheckValidTime() START !");
	}
	
	/* 小於六碼不合法 */
	if (strlen(szTime) != 6)
	{
		return (VS_ERROR);
	}

   	/*  Get 2-Digit Hours */
   	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szTime[0], 2);
    	inHours = atoi(szTemplate);
    	/*  Get 2-Digit Minutes */
   	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szTime[2], 2);
    	inMins = atoi(szTemplate);
    	/*  Get 2-Digit Seconds */
   	memset(szTemplate, 0x00, sizeof(szTemplate));
    	memcpy(&szTemplate[0], &szTime[4], 2);
    	inSecs = atoi(szTemplate);

	if ((inHours < 0) || (inHours > 23))
		return (VS_ERROR);

	if ((inMins < 0) || (inMins > 59))
		return (VS_ERROR);

	if ((inSecs < 0) || (inSecs > 59))
		return (VS_ERROR);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_CheckValidTime() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_SunDay_Sum
Date&Time       :2016/2/24 下午 1:13
Describe        :輸入Date，算出太陽日，2000年起算，YYYYMMDD
 * [20260128_BUG_MDF][FUN] 重新編寫太陽日計算方式
*/
int inFunc_SunDay_Sum(char *szDate)
{		

	int inYear, inMonth, inDay;
	int inSolarDay = 0;
	int inLeapDays;
	char szBuf[5];

	if (szDate == NULL || strlen(szDate) != 8)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "SunDay_Sum Data[%s] Error",szDate);
		/*[260414] 移除記錄,因此記錄在主畫面會多次進入 */
		//vdUtility_SYSFIN_LogMessage(AT, "SunDay_Sum Data[%s] Error",szDate);
		return VS_ERROR;
	}

	/* 解析 YYYY */
	memset(szBuf, 0, sizeof(szBuf));
	memcpy(szBuf, szDate, 4);
	inYear = atoi(szBuf);

	/* 解析 MM */
	memset(szBuf, 0, sizeof(szBuf));
	memcpy(szBuf, szDate + 4, 2);
	inMonth = atoi(szBuf);

	/* 解析 DD */
	memset(szBuf, 0, sizeof(szBuf));
	memcpy(szBuf, szDate + 6, 2);
	inDay = atoi(szBuf);

	/* 基本合法性檢查 */
	if (inYear < 1 || inMonth < 1 || inMonth > 12 || inDay < 1 || inDay > 31)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "SunDay_Sum inYear[%d] inMonth[%d] inDay[%d] Error", inYear, inMonth, inDay);
		/*[260414] 移除記錄,因此記錄在主畫面會多次進入 */
		//vdUtility_SYSFIN_LogMessage(AT, "SunDay_Sum inYear[%d] inMonth[%d] inDay[%d] Error", inYear, inMonth, inDay);
		return VS_ERROR;
	}

	/* 前一年為止的總天數 */
	inLeapDays = (inYear - 1) / 4
				- (inYear - 1) / 100
				+ (inYear - 1) / 400;

	inSolarDay = 365 * (inYear - 1) + inLeapDays;

	/* 當年度各月累積天數（非閏年） */
	static const int sr_inMonthDays[12] = {
		0,   31,  59,  90, 120, 151,
		181, 212, 243, 273, 304, 334
	};

	inSolarDay += sr_inMonthDays[inMonth - 1];

	/* 閏年且已過 2 月 */
	if (blLeapYear(inYear) && inMonth > 2)
		inSolarDay += 1;

	/* 加上日期 */
	inSolarDay += inDay;

	if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "SunDay_Sum RetVal[%d]", inSolarDay);
	return inSolarDay;
}

/*
Function        :inFunc_SunDay_Sum_Check_In_Range
Date&Time       :2017/2/2 下午 3:27
Describe        :輸入Date，算出太陽日，2000年起算，YYYYMMDD，若Date1在Date2和Date3內(含那一天)，Return VS_SUCCESS，否則回傳VS_ERROR
*/
int inFunc_SunDay_Sum_Check_In_Range(char *szDate1, char *szDate2, char *szDate3)
{
	int	i;
	int	inSumDay1, inSumDay2, inSumDay3;
	char	szYear[4 + 1];
	
	/* 先檢核日期是否為合法字元 */
	for (i = 0; i < 8; i ++)
	{
		if (szDate1[i] < '0' || szDate1[i] > '9')
		{
			return (VS_ERROR);
		}
			
	}
	
	for (i = 0; i < 8; i ++)
	{
		if (szDate2[i] < '0' || szDate2[i] > '9')
		{
			return (VS_ERROR);
		}
			
	}
	
	for (i = 0; i < 8; i ++)
	{
		if (szDate3[i] < '0' || szDate3[i] > '9')
		{
			return (VS_ERROR);
		}
			
	}
	
	/* 檢核年是否小於2000年 */
	memset(szYear, 0x00, sizeof(szYear));
	memcpy(szYear, szDate1, 4);
	if (atoi(szYear) < 2000)
	{
		return (VS_ERROR);
	}
	
	memset(szYear, 0x00, sizeof(szYear));
	memcpy(szYear, szDate2, 4);
	if (atoi(szYear) < 2000)
	{
		return (VS_ERROR);
	}
	
	memset(szYear, 0x00, sizeof(szYear));
	memcpy(szYear, szDate3, 4);
	if (atoi(szYear) < 2000)
	{
		return (VS_ERROR);
	}
	
	/* 算出太陽日 */
	inSumDay1 = inFunc_SunDay_Sum(szDate1);
	inSumDay2 = inFunc_SunDay_Sum(szDate2);
	inSumDay3 = inFunc_SunDay_Sum(szDate3);
	
	if ((inSumDay1 < inSumDay2) || (inSumDay1 > inSumDay3))
	{
		return (VS_ERROR);
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inFuncGetTimeToUnix
Date&Time       :2017/12/21 下午 4:43
Describe        :輸入本地時間和本地時差，目標時差，輸出格林威治的時間，從1970開始算的秒數
 *		 回傳結果為是否成功
*/
int inFuncGetTimeToUnix(RTC_NEXSYS *srRTC_In, char *szOutput, int inSrcTimeZone, int inDestTimeZone)
{
	int		inDaysOfMonth[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
        int		inTemp = 0, inDiffTimeZone = 0;
        int		inLeapYearCount = 0;
        char		szTemp[12] = {0};
	unsigned long	ulSec = 0;

        /* 當地與目標時差 */
	/* src:+8 Dest:0 目標日少於當地8小時 */
	/* src:-8 Dest:+8 目標日多於當地16小時 */
        inDiffTimeZone = inDestTimeZone - inSrcTimeZone;

        /* 計算閏年 */
        inTemp = (2000 + srRTC_In->uszYear) - 1970;
        inLeapYearCount = 0;

        inLeapYearCount = ((inTemp + 2) / 4);		/* 經過幾個閏年 */

        if ((inTemp + 2) % 4 == 0)			/* 是閏年 */
        {
            if (srRTC_In->uszMonth <= 2)    /* 尚未超過02/29,要減1天回來 */
            {
                inLeapYearCount--;
            }
            else if((srRTC_In->uszMonth == 2) && (srRTC_In->uszDay == 29)) /* 剛好02/29,要加1天 */
            {
                inLeapYearCount++;
            }
        }

        ulSec = inLeapYearCount * 1+ inTemp * 365 + inDaysOfMonth[srRTC_In->uszMonth - 1] + srRTC_In->uszDay - 1;
	
	/* 換算成小時，把時差減回來 */
        ulSec = ulSec * 24 + srRTC_In->uszHour + inDiffTimeZone;
	/* 換算成分鐘，並把分鐘加上去 */
        ulSec = ulSec * 60 + srRTC_In->uszMinute;
	/* 換算成秒數，並把秒數加上去 */
        ulSec = ulSec * 60 + srRTC_In->uszSecond;

        memset(szTemp, 0x00, sizeof(szTemp));
        sprintf(szTemp, "%lu", ulSec);
        memcpy(&szOutput[0], &szTemp[0], 10);

        return (VS_SUCCESS);
}

/*
Function        :inFuncGetUnixTimeToLocalTime
Date&Time       :2019/4/12 上午 10:41
Describe        :輸入本地時間和時差，輸出格林威治的時間，從1970開始算的秒數
 *		 回傳結果為是否成功
*/
int inFuncGetUnixTimeToLocalTime(RTC_NEXSYS *srRTC_Out, char *szInput, int inSrcTimeZone, int inDestTimeZone)
{
	int		i = 0;
	int		inDaysOfMonth[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	int		inDaysOfMonth_Leap[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
        int		inTemp = 0, inDiffTimeZone = 0;
	int		inTempYear = 0;
	unsigned long	ulSec = 0;
	unsigned char	uszLeapYearBit = VS_FALSE;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "lnFuncGetUnixTimeToLocalTime() START !");
	}

	memset(srRTC_Out, 0x00, sizeof(RTC_NEXSYS));
	
        /* 當地與目標時差 */
	/* src:+8 Dest:0 目標日少於當地8小時 */
	/* src:-8 Dest:+8 目標日多於當地16小時 */
        inDiffTimeZone = inDestTimeZone - inSrcTimeZone;

	/* 和1970差多少秒 */
	ulSec = atol(szInput);
	/* 切換到目標時間 */
	ulSec = ulSec + inDiffTimeZone * 60 * 60;
	
	/* 先同步到閏年的後一年 */
	ulSec -=  2 * (365 * 24 * 60 * 60);
	inTempYear += 2;
	
	/* 計算閏年 */
	inTemp = 0;
	inTemp = (ulSec / ((365 * 4 + 1) * 24 * 60 * 60));
	ulSec -= inTemp * ((365 * 4 + 1) * 24 * 60 * 60);
	inTempYear += inTemp * 4;
	
	/* 計算剩下的年 */
	/* 如果剩下的時間超過一年要再多扣一天，因為會在計算剩下的年那個階段把閏年當成365天來計算 */
	if (ulSec > (366 * 24 * 60 * 60))
	{
		ulSec -= (24 * 60 * 60);
	}
	
	inTemp = 0;
	inTemp = ulSec / (365 * 24 * 60 * 60);
	ulSec -= inTemp * (365 * 24 * 60 * 60);
	inTempYear += inTemp;
	
	
	
	srRTC_Out->uszYear = (inTempYear + 1970) - 2000;
	if (((srRTC_Out->uszYear + 2000) % 4) == 0)
	{
		uszLeapYearBit = VS_TRUE;
	}
	
	/* 計算月份 */
	inTemp = 0;
	inTemp = ulSec / (24 * 60 * 60);
	
	for (i = 0; i < 12; i++)
	{
		if (uszLeapYearBit == VS_TRUE)
		{
			if (inDaysOfMonth_Leap[i] > inTemp)
			{
				/* 就是當月 */
				i = i;
				break;
			}
			else if (inDaysOfMonth_Leap[i] == inTemp)
			{
				i++;
				break;
			}
			else/* inDaysOfMonth[i] < inTemp*/
			{
				continue;
			}
		}
		else
		{
			if (inDaysOfMonth[i] > inTemp)
			{
				/* 就是當月 */
				i = i;
				break;
			}
			else if (inDaysOfMonth[i] == inTemp)
			{
				/* 1號 */
				i++;
				break;
			}
			else/* inDaysOfMonth[i] < inTemp*/
			{
				continue;
			}
		}
	}
	ulSec -= inTemp * (24 * 60 * 60);
	srRTC_Out->uszMonth = i;
	if (uszLeapYearBit == VS_TRUE)
	{
		if (inTemp - inDaysOfMonth_Leap[i - 1] == 0)
		{
			srRTC_Out->uszDay = 1;
		}
		else
		{
			srRTC_Out->uszDay = inTemp - inDaysOfMonth_Leap[i - 1] + 1;
		}
	}
	else
	{
		if (inTemp - inDaysOfMonth[i - 1] == 0)
		{
			srRTC_Out->uszDay = 1;
		}
		else
		{
			srRTC_Out->uszDay = inTemp - inDaysOfMonth[i - 1] + 1;
		}
	}
	
	/* 計算時 */
	inTemp = 0;
	inTemp = ulSec / (60 * 60);
	ulSec -= inTemp * (60 * 60);
	srRTC_Out->uszHour = inTemp;
		
	/* 計算分 */
	inTemp = 0;
	inTemp = ulSec / (60);
	ulSec -= inTemp * (60);
	srRTC_Out->uszMinute = inTemp;
		
	/* 計算秒 */
	inTemp = 0;
	inTemp = ulSec;
	ulSec -= inTemp;
        srRTC_Out->uszSecond = inTemp;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "20%u.%u.%u %u:%u:%u", srRTC_Out->uszYear, 
							srRTC_Out->uszMonth, 
							srRTC_Out->uszDay, 
							srRTC_Out->uszHour, 
							srRTC_Out->uszMinute, 
							srRTC_Out->uszSecond);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "lnFuncGetUnixTimeToLocalTime() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_String_Dec_to_Hex_Little2Little
Date&Time       :2018/1/2 下午 6:23
Describe        :把十進位的數字字串轉成16進位的字串
*/
int inFunc_String_Dec_to_Hex_Little2Little(char *szInput, char *szOutput)
{
        char		szTemp[20 + 1] = {0};
	char		szDebugMsg[100 + 1];
	unsigned long	ulData = 0;

        ulData = strtoul(szInput, NULL, 10);
	
        memset(szTemp, 0x00, sizeof(szTemp));
        sprintf(szTemp, "%02X%02X%02X%02X", ((unsigned char*)&ulData)[0], ((unsigned char*)&ulData)[1], ((unsigned char*)&ulData)[2], ((unsigned char*)&ulData)[3]);
        memcpy(&szOutput[0], &szTemp[0], 8);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Unix time:%lu", ulData);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Output:%s", szOutput);
		inLogPrintf(AT, szDebugMsg);
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_String_Hex_to_Dec_Big2Little
Date&Time       :2018/1/17 上午 10:29
Describe        :把16進位的數字字串轉成十進位的字串
*/
int inFunc_String_Hex_to_Dec_Big2Little(char *szInput, char *szOutput, int inInputLen)
{
	int		i = 0;
	int		inTemp = 0;
        char		szTemp[100 + 1] = {0};
	char		szDebugMsg[100 + 1];
	unsigned long	ulData = 0;

	for (i = 0; *(szInput + i) != 0x00; i++)
	{
		if (*(szInput + i) >= '0' && *(szInput + i) <= '9')
		{
			inTemp = *(szInput + i) - '0';
		}
		else if (*(szInput + i) >= 'A' && *(szInput + i) <= 'F')
		{
			inTemp = *(szInput + i) - 'A';
			inTemp += 10;
		}
		else if (*(szInput + i) >= 'a' && *(szInput + i) <= 'f')
		{
			inTemp = *(szInput + i) - 'a';
			inTemp += 10;
		}
		else
		{
			return (VS_ERROR);
		}
		ulData = (16 * ulData) + inTemp;
		
	};
	
        memset(szTemp, 0x00, sizeof(szTemp));
        sprintf(szTemp, "%lu", ulData);
        strcpy(szOutput, szTemp);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Input:%s", szInput);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Output:%s", szOutput);
		inLogPrintf(AT, szDebugMsg);
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_String_Hex_to_Dec_Little2Little
Date&Time       :2019/4/12 下午 2:55
Describe        :把16進位的數字字串轉成十進位的字串
*/
int inFunc_String_Hex_to_Dec_Little2Little(char *szInput, char *szOutput, int inInputLen)
{
	int		i = 0;
	int		inTemp = 0;
	int		inPhase = 0;
        char		szTemp[100 + 1] = {0};
	char		szDebugMsg[100 + 1];
	char		szReverseInput[100 + 1] = {0};
	unsigned long	ulData = 0;

	inPhase = (inInputLen / 2);
	for (i = 0; i < inPhase; i++)
	{
		szReverseInput[2 * i] = *(szInput + (2 * (inPhase - 1 - i)));
		szReverseInput[2 * i + 1] = *(szInput + (2 * (inPhase - 1 - i) + 1));
	}
	
	/* 倒回來 */
	for (i = 0; *(szReverseInput + i) != 0x00; i++)
	{
		if (*(szReverseInput + i) >= '0' && *(szReverseInput + i) <= '9')
		{
			inTemp = *(szReverseInput + i) - '0';
		}
		else if (*(szReverseInput + i) >= 'A' && *(szReverseInput + i) <= 'F')
		{
			inTemp = *(szReverseInput + i) - 'A';
			inTemp += 10;
		}
		else if (*(szReverseInput + i) >= 'a' && *(szReverseInput + i) <= 'f')
		{
			inTemp = *(szReverseInput + i) - 'a';
			inTemp += 10;
		}
		else
		{
			return (VS_ERROR);
		}
		ulData = (16 * ulData) + inTemp;
		
	};
	
        memset(szTemp, 0x00, sizeof(szTemp));
        sprintf(szTemp, "%lu", ulData);
        strcpy(szOutput, szTemp);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Input:%s", szInput);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "ReverseInput:%s", szReverseInput);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Output:%s", szOutput);
		inLogPrintf(AT, szDebugMsg);
	}

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Big5toUTF8
Date&Time       :2016/3/8 下午 1:48
Describe        :BIG5編碼中文字轉UTF8編碼 (至現回報訊息用到)
*/
int inFunc_Big5toUTF8(char *szUTF8, char *szBIG5)
{
#ifndef	_LOAD_KEY_AP_
        iconv_t cd = NULL;
        size_t  stUTF8Len, stBIG5Len;
	char    szInput[4096 + 1] = {0}, *szInp = szInput;
        char    szOutput[2 * sizeof(szInput) + 1], *szOutp = szOutput;
        char    szTemplate[64 + 1];
        int	conv = 0;
        
        if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inFunc_Big5toUTF8() START!");
	
        cd = iconv_open("UTF-8", "BIG5");
        
	if (cd == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			sprintf(szTemplate, "iconv_open Error !! cd = %d", (int)cd);
			inLogPrintf(AT, szTemplate);
		}
		iconv_close(cd);
		
		return (VS_ERROR);
	}
        
        memset(szInput, 0x00, sizeof(szInput));
        memcpy(&szInput[0], &szBIG5[0], strlen(szBIG5));
        memset(szOutput, 0x00, sizeof(szOutput));
        
        stBIG5Len = strlen(szInput);
        stUTF8Len = stBIG5Len * 2;
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "stBIG5Len = %d", stBIG5Len);
                inLogPrintf(AT, szTemplate);
                sprintf(szTemplate, "stUTF8Len = %d", stUTF8Len);
                inLogPrintf(AT, szTemplate);       
        }
        
        conv = iconv(cd, &szInp, &stBIG5Len, &szOutp, &stUTF8Len);
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "iconv = %d", conv);
                inLogPrintf(AT, szTemplate);
                inLogPrintf(AT, szOutput);
        }
        
        memcpy(&szUTF8[0], &szOutput[0], strlen(szOutput));
        
        iconv_close(cd);
#endif
        
	return (VS_SUCCESS);
}

/*
Function        :inFunc_UTF8toBig5
Date&Time       :2017/4/27 下午 5:27
Describe        :UTF8編碼轉BIG5編碼中文字
*/
int inFunc_UTF8toBig5(char *szBIG5, char *szUTF8)
{
#ifndef	_LOAD_KEY_AP_
        iconv_t cd = NULL;
        size_t  stUTF8Len, stBIG5Len;
        char    szOutput[1024 + 1], *szOutp = szOutput;
        char    szInput[1024 + 1], *szInp = szInput;
        char    szTemplate[64 + 1];
        int	conv = 0;
        
        if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inFunc_Big5toUTF8() START!");

        cd = iconv_open("BIG5", "UTF-8");
        
        if (cd == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			sprintf(szTemplate, "iconv_open Error !! cd = %d", (int)cd);
			inLogPrintf(AT, szTemplate);
		}
		iconv_close(cd);
		
		return (VS_ERROR);
	}
        
        memset(szInput, 0x00, sizeof(szInput));
        memcpy(&szInput[0], &szUTF8[0], strlen(szUTF8));
        memset(szOutput, 0x00, sizeof(szOutput));
        
        stUTF8Len = strlen(szInput);
        stBIG5Len = stUTF8Len;
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "stBIG5Len = %d", stBIG5Len);
                inLogPrintf(AT, szTemplate);
                sprintf(szTemplate, "stUTF8Len = %d", stUTF8Len);
                inLogPrintf(AT, szTemplate);       
        }
        
        conv = iconv(cd, &szInp, &stUTF8Len, &szOutp, &stBIG5Len);
        
        if (ginDebug == VS_TRUE)
        {
                sprintf(szTemplate, "iconv = %d", conv);
                inLogPrintf(AT, szTemplate);
                inLogPrintf(AT, szOutput);
        }
        
        memcpy(&szBIG5[0], &szOutput[0], strlen(szOutput));
        
        iconv_close(cd);
#endif
        
	return (VS_SUCCESS);
}

void callbackFun(unsigned int inTotalProgress, unsigned int inCapProgress, unsigned char *uszCurMCI, unsigned char *uszCurCAP)
{
	char strBuf[64] = {0};
    
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (strlen((char*)uszCurMCI) > 0)
	{
		memset(strBuf, 0x00, sizeof(strBuf));
		sprintf(strBuf, "%s", uszCurMCI);
		inDISP_ChineseFont(strBuf, _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);
	}

	if (strlen((char*)uszCurCAP) > 0)
	{
		memset(strBuf, 0x00, sizeof(strBuf));
		sprintf(strBuf, "%s", uszCurCAP);
		inDISP_ChineseFont(strBuf, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
	}
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	
	inDISP_ChineseFont("總進度/單檔進度", _FONTSIZE_16X22_, _LINE_16_15_, _DISP_LEFT_);
	memset(strBuf, 0x00, sizeof(strBuf));
	sprintf(strBuf, "%3d%% / %3d%%", inTotalProgress, inCapProgress);
	inDISP_ChineseFont(strBuf, _FONTSIZE_16X22_, _LINE_16_16_, _DISP_LEFT_);
}

/*
Function	:inFunc_CalculateRunTimeGlobal_Start
Date&Time	:2016/3/7 上午 10:43
Describe	:用來看精度秒以下的RunTime
*/
int inFunc_CalculateRunTimeGlobal_Start(void)
{	
	gulRunTime = CTOS_TickGet();
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_GetRunTimeGlobal
Date&Time	:2016/3/7 上午 10:43
Describe	:用來看精度秒以下的RunTime
*/
int inFunc_GetRunTimeGlobal(unsigned long *ulSecond, unsigned long *ulMilliSecond)
{
	char		szDebugMsg[100 + 1];
	unsigned long	ulRunTimeStart, ulRunTimeEnd, ulInterval;
	
	ulRunTimeStart = 0;
	ulRunTimeEnd = 0;
	ulInterval = 0;
	*ulMilliSecond = 0;
	*ulSecond = 0;
	
	
	ulRunTimeStart = gulRunTime;
	ulRunTimeEnd = CTOS_TickGet();
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "%lu %lu", ulRunTimeStart, ulRunTimeEnd);
		inLogPrintf(AT, szDebugMsg);
	}
	
	
	ulInterval = ulRunTimeEnd - ulRunTimeStart;
	*ulSecond = ulInterval / 100;
	*ulMilliSecond = 10 * (ulInterval % 100);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_WatchRunTime
Date&Time	:2017/3/17 上午 11:18
Describe	:用來看精度秒以下的RunTime
*/
int inFunc_WatchRunTime(void)
{	
	char		szDebugMsg[100 + 1];
	unsigned long	ulSecond = 0;
	unsigned long 	ulMilliSecond = 0;
	
	inFunc_GetRunTimeGlobal(&ulSecond, &ulMilliSecond);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Run Time %lu.%03lu", ulSecond, ulMilliSecond);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Run Time %lu.%03lu", ulSecond, ulMilliSecond);
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_RecordTime_ResetAll
Date&Time       :2018/10/25 下午 2:04
Describe        :
*/
int inFunc_RecordTime_ResetAll()
{
	int	i = 0;
	
	for (i = 0; i < ginRuntimeCnt; i++)
	{
		memset(gsrRuntimeRecord.srMyRunTimeRecord[i].szComment, 0x00, sizeof(gsrRuntimeRecord.srMyRunTimeRecord[i].szComment));
		gsrRuntimeRecord.srMyRunTimeRecord[i].ulSecond = 0;
		gsrRuntimeRecord.srMyRunTimeRecord[i].ulMilliSecond = 0;
	}
	ginRuntimeCnt = 0;
	inFunc_CalculateRunTimeGlobal_Start();
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_RecordTime_Append
Date&Time       :2018/10/25 下午 2:10
Describe        :
*/
int inFunc_RecordTime_Append(char* szComment, ...)
{
	char	szCommentTemp[100 + 1] = {0};
	va_list	list;
	
	va_start(list, szComment);
	vsprintf(szCommentTemp, szComment, list);
	va_end(list);
	
	if (ginRuntimeCnt < _RUNTIME_RECORD_MAX_COUNT_)
	{
		inFunc_RecordTime_Save(ginRuntimeCnt, szCommentTemp);
		ginRuntimeCnt++;

		return (VS_SUCCESS);
	}
	else
	{
		inLogPrintf(AT, "Reach Record Max Count");
		
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_RecordTime_WatchAll
Date&Time       :2018/10/25 下午 2:15
Describe        :
*/
int inFunc_RecordTime_WatchAll()
{
	int		i = 0;
#ifndef _DISP_REC_	
	unsigned char	uszKey = 0;
#endif	
	for (i = 0; i < ginRuntimeCnt; i++)
	{
		
		inFunc_RecordTime_Watch(i);
#ifndef _DISP_REC_
		uszKey = uszKBD_GetKey(600);
		if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else
		{
			continue;
		}
#endif
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_RecordTime_Save
Date&Time       :2017/8/29 上午 9:42
Describe        :
*/
int inFunc_RecordTime_Save(int inIndex, char* szComment)
{
	unsigned long	ulSecond = 0;
	unsigned long 	ulMilliSecond = 0;
	
	inFunc_GetRunTimeGlobal(&ulSecond, &ulMilliSecond);
	strcpy(gsrRuntimeRecord.srMyRunTimeRecord[inIndex].szComment, szComment);
	gsrRuntimeRecord.srMyRunTimeRecord[inIndex].ulSecond = ulSecond;
	gsrRuntimeRecord.srMyRunTimeRecord[inIndex].ulMilliSecond = ulMilliSecond;
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "%d Record Save", inIndex);
		inLogPrintf(AT, "%lu.%03lu", ulSecond, ulMilliSecond);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_RecordTime_Watch
Date&Time       :2017/8/29 上午 9:42
Describe        :
*/
int inFunc_RecordTime_Watch(int inIndex)
{
	char	szDebugMsg[100 + 1] = {0};
#ifdef _DISP_REC_
	memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	sprintf(szDebugMsg, "%s", gsrRuntimeRecord.srMyRunTimeRecord[inIndex].szComment);
	inLogPrintf(AT, szDebugMsg);
	
	memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	sprintf(szDebugMsg, "%d Time %lu.%03lu", inIndex, gsrRuntimeRecord.srMyRunTimeRecord[inIndex].ulSecond, gsrRuntimeRecord.srMyRunTimeRecord[inIndex].ulMilliSecond);
	inLogPrintf(AT, szDebugMsg);
#else	
	memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	sprintf(szDebugMsg, "%s", gsrRuntimeRecord.srMyRunTimeRecord[inIndex].szComment);
	inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X44_, _LINE_16_15_, VS_FALSE);
	
	memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	sprintf(szDebugMsg, "%d Time %lu.%03lu", inIndex, gsrRuntimeRecord.srMyRunTimeRecord[inIndex].ulSecond, gsrRuntimeRecord.srMyRunTimeRecord[inIndex].ulMilliSecond);
	inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_FALSE);
#endif
	return (VS_SUCCESS);
}

/*
Function	:inFunc_CalculateRunTime_Start
Date&Time	:2017/4/27 下午 1:18
Describe	:用來看精度秒以下的RunTime
*/
unsigned long ulFunc_CalculateRunTime_Start()
{	unsigned long	ulRunTime;

	ulRunTime = CTOS_TickGet();
	
	return (ulRunTime);
}

/*
Function	:inFunc_GetRunTime
Date&Time	:2017/4/27 下午 1:18
Describe	:用來看精度秒以下的RunTime
*/
int inFunc_GetRunTime(unsigned long ulRunTime, unsigned long *ulSecond, unsigned long *ulMilliSecond)
{
//	char		szDebugMsg[100 + 1];
	unsigned long	ulRunTimeStart, ulRunTimeEnd, ulInterval;
	
	ulRunTimeStart = 0;
	ulRunTimeEnd = 0;
	ulInterval = 0;
	*ulMilliSecond = 0;
	*ulSecond = 0;
	
	
	ulRunTimeStart = ulRunTime;
	ulRunTimeEnd = CTOS_TickGet();
	
//	if (ginDebug == VS_TRUE)
//	{
//		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
//		sprintf(szDebugMsg, "%lu %lu", ulRunTimeStart, ulRunTimeEnd);
//		inLogPrintf(AT, szDebugMsg);
//	}
	
	
	ulInterval = ulRunTimeEnd - ulRunTimeStart;
	*ulSecond = ulInterval / 100;
	*ulMilliSecond = 10 * (ulInterval % 100);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_How_Many_Not_Ascii
Date&Time       :2017/2/15 下午 4:56
Describe        :判斷字串中不是英數字符號的數目 小心0x00
*/
int inFunc_How_Many_Not_Ascii(char *szData)
{
	int	i = 0;
	int	inLen = 0;
	int	inNotAscii = 0;
	
	inLen = strlen(szData);
	
	for (i = 0; i < inLen; i++)
	{
		if (szData[i] > 127)
		{
			inNotAscii++;
		}
	}
	
	return inNotAscii;
}

/*
Function	:inFunc_ChooseLoadFileWay
Date&Time	:2016/3/24 下午 2:02
Describe        :
*/
int inFunc_ChooseLoadFileWay(void)
{
	/* 用USB(待開發) */
	
	/* 用SD卡 */
	inFunc_SDLoadFile();
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_SDLoadFile
Date&Time	:2016/3/24 下午 2:02
Describe        :用SD卡載資料的特性:一次可load多個檔案，可以在程式運行中再插入SD卡
 *		目前無法做錯誤判斷
*/
int inFunc_SDLoadFile(void)
{
	int		inRetVal;
	char		szFileName[15];	/* 檔案名稱最多15字*/
        DISPLAY_OBJECT  srDispObj;

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 15;
	srDispObj.inColor = _COLOR_RED_;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        /* 輸入檔案名稱 */
        inDISP_ChineseFont("輸入檔案名稱:", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);

	memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
	srDispObj.inOutputLen = 0;
	
        inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);
	memset(szFileName, 0x00, sizeof(szFileName));
	memcpy(szFileName, &srDispObj.szOutput[0], srDispObj.inMaxLen);
		
	/* fs_data資料夾內移到pub(若資料夾內沒資料則開檔失敗回傳錯誤) */
	inRetVal = inFunc_Data_Copy(szFileName, _FS_DATA_PATH_, "", _SD_PATH_);
		
	if (ginDebug == VS_TRUE)
	{
		/* 用teraterm可用來看資料夾內狀態 */
		inFunc_ShellCommand_Popen("ls", VS_FALSE);
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Data_Move
Date&Time	:2018/4/23 下午 4:26
Describe        :此function將主程式資料夾中的特定檔案存到，用system call 需要花50毫秒，建議改用inFile_Move_File
*/
int inFunc_Data_Move(char *szSrcFileName, char* szSource, char* szDesFileName, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Move START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FName: %s", szSrcFileName);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* mv 來源檔(source) 目標檔(destination) (等同rename)*/
	/* mv source1 source2 source3 .... directory (複製到目標資料夾)*/
	/* 可用來變更檔名 */
	/* 預設會直接覆蓋 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (strlen(szSrcFileName) != 0)
	{
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "mv ");
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}
		
		strcat(szCommand, szSrcFileName);
		
		/* 空格 */
		strcat(szCommand, " ");
		
		if (strlen(szDestination) != 0)
		{
			strcat(szCommand, szDestination);
		}
		
		if (strlen(szDesFileName) != 0)
		{
			strcat(szCommand, szDesFileName);
		}
	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
	inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
		
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FName : %s", szSrcFileName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFunc_Move END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Dir_Make
Date&Time	:2018/4/23 下午 3:48
Describe        :建立資料夾
 *		-p ：幫助你直接將所需要的目錄(包含上層目錄)遞迴建立起來！
*/
int inFunc_Dir_Make(char *szDirName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Make_Dir START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "DirName: %s", szDirName);
		inLogPrintf(AT, szDebugMsg);
	}
		
	/* mkdir 資料夾路徑 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (strlen(szDirName) != 0)
	{
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "mkdir -p  ");
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}
		
		strcat(szCommand, szDirName);
	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "DirName : %s", szDirName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFunc_Make_Dir END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Data_Delete
Date&Time	:2018/4/23 下午 3:48
Describe        :刪除檔案
*/
int inFunc_Data_Delete(char* szParameter, char *szFileName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Delete_Data START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
	}
		
	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (strlen(szFileName) != 0)
	{
		sprintf(szCommand, "rm  ");
		
		if (strlen(szParameter) != 0)
		{
			strcat(szCommand, szParameter);
			strcat(szCommand, " ");
		}
		
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}

		/* 目標檔名 */
		strcat(szCommand, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFunc_Delete_Data END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Data_Copy
Date&Time	:2018/4/24 上午 10:30
Describe        :複製檔案
*/
int inFunc_Data_Copy(char *szSrcFileName, char* szSource, char* szDesFileName, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Copy_Data START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szSrcFileName);
		inLogPrintf(AT, szDebugMsg);
	}
		
	/* 組命令 */
	if (strlen(szSrcFileName) != 0)
	{
		/* cp szSource/szSrcFileName szDestination/szDesFileName */
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "cp ");
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}
		
		strcat(szCommand, szSrcFileName);
		
		/* 空格 */
		strcat(szCommand, " ");
		
		if (strlen(szDestination) != 0)
		{
			strcat(szCommand, szDestination);
		}
		
		if (strlen(szDesFileName) != 0)
		{
			strcat(szCommand, szDesFileName);
		}
	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szSrcFileName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFunc_Copy_Data END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Data_Rename
Date&Time	:2018/5/25 下午 4:02
Describe        :更改檔名
*/
int inFunc_Data_Rename(char *szOldFileName, char* szSource, char *szNewFileName, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Rename START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szOldFileName);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 組命令 */
	if (strlen(szOldFileName) != 0)
	{
		/* mv szSource/szOldFileName szDestination/szNewFileName */
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "mv ");
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}
		
		strcat(szCommand, szOldFileName);
		
		/* 空格 */
		strcat(szCommand, " ");
		
		if (strlen(szDestination) != 0)
		{
			strcat(szCommand, szDestination);
		}
		
		if (strlen(szNewFileName) != 0)
		{
			strcat(szCommand, szNewFileName);
		}
	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "NewFNaeme : %s", szNewFileName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFunc_Rename_Data END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Data_GZip
Date&Time	:2018/6/19 下午 2:13
Describe        :GZip檔案
*/
int inFunc_Data_GZip(char* szParameter, char *szFileName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_GZip_Data START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
	}
		
	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (strlen(szFileName) != 0)
	{
		sprintf(szCommand, "gzip  ");
		
		if (strlen(szParameter) != 0)
		{
			strcat(szCommand, szParameter);
			strcat(szCommand, " ");
		}
		
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}

		/* 目標檔名 */
		strcat(szCommand, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFunc_GZip_Data END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Data_GUnZip
Date&Time	:2018/6/19 下午 2:29
Describe        :GUnZip檔案
*/
int inFunc_Data_GUnZip(char* szParameter, char *szFileName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_GUnZip_Data START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
	}
		
	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (strlen(szFileName) != 0)
	{
		sprintf(szCommand, "gunzip  ");
		
		if (strlen(szParameter) != 0)
		{
			strcat(szCommand, szParameter);
			strcat(szCommand, " ");
		}
		
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}

		/* 目標檔名 */
		strcat(szCommand, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFunc_GUnZip_Data END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Unzip
Date&Time	:2018/6/19 下午 2:36
Describe        :解壓縮
 	-c：将解压缩的结果显示到屏幕上，并对字符做适当的转换；
	-f：更新现有的文件；
	-l：显示压缩文件内所包含的文件；
	-p：与-c参数类似，会将解压缩的结果显示到屏幕上，但不会执行任何的转换；
	-t：检查压缩文件是否正确；
	-u：与-f参数类似，但是除了更新现有的文件外，也会将压缩文件中的其他文件解压缩到目录中；
	-v：执行时显示详细的信息；
	-z：仅显示压缩文件的备注文字；
	-a：对文本文件进行必要的字符转换；
	-b：不要对文本文件进行字符转换；
	-C：压缩文件中的文件名称区分大小写；
	-j：不处理压缩文件中原有的目录路径；
	-L：将压缩文件中的全部文件名改为小写；
	-M：将输出结果送到more程序处理；
	-n：解压缩时不要覆盖原有的文件；
	-o：不必先询问用户，unzip执行后覆盖原有的文件；
	-P<密码>：使用zip的密码选项；
	-q：执行时不显示任何信息；
	-s：将文件名中的空白字符转换为底线字符；
	-V：保留VMS的文件版本信息；
	-X：解压缩时同时回存文件原来的UID/GID；
	-d<目录>：指定文件解压缩后所要存储的目录；
	-x<文件>：指定不要处理.zip压缩文件中的哪些文件；
	-Z：unzip-Z等于执行zipinfo指令。

	将压缩文件text.zip在当前目录下解压缩。

	unzip test.zip
	将压缩文件text.zip在指定目录/tmp下解压缩，如果已有相同的文件存在，要求unzip命令不覆盖原先的文件。
	************重要*******************
	這裡指的當前目錄為主程式根目錄，非壓縮檔所在目錄

	unzip -n test.zip -d /tmp
	查看压缩文件目录，但不解压。

	unzip -v test.zip
	将压缩文件test.zip在指定目录/tmp下解压缩，如果已有相同的文件存在，要求unzip命令覆盖原先的文件。

	unzip -o test.zip -d tmp/
 */
int inFunc_Unzip(char* szParameter1, char* szOldFileName, char* szSource, char* szParameter2, char* szDestination)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Unzip START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szOldFileName);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 組命令 */
	if (strlen(szOldFileName) != 0)
	{
		/* mv szSource/szOldFileName szDestination/szNewFileName */
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "unzip ");
		
		if (strlen(szParameter1) != 0)
		{
			strcat(szCommand, szParameter1);
			strcat(szCommand, " ");
		}
		
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}
		
		strcat(szCommand, szOldFileName);
		
		/* 空格 */
		strcat(szCommand, " ");
		
		if (strlen(szParameter2) != 0)
		{
			strcat(szCommand, szParameter2);
			strcat(szCommand, " ");
		}
		
		if (strlen(szDestination) != 0)
		{
			strcat(szCommand, szDestination);
		}
		
	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{	
		inLogPrintf(AT, "inFunc_Unzip END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_ls
Date&Time       :2018/6/29 上午 9:43
Describe        :檔案目錄檢視
	-a  ：全部的檔案，連同隱藏檔( 開頭為 . 的檔案) 一起列出來(常用)
	-A  ：全部的檔案，連同隱藏檔，但不包括 . 與 .. 這兩個目錄
	-d  ：僅列出目錄本身，而不是列出目錄內的檔案資料(常用)
	-f  ：直接列出結果，而不進行排序 (ls 預設會以檔名排序！)
	-F  ：根據檔案、目錄等資訊，給予附加資料結構，例如：
	      *:代表可執行檔； /:代表目錄； =:代表 socket 檔案； |:代表 FIFO 檔案；
	-h  ：將檔案容量以人類較易讀的方式(例如 GB, KB 等等)列出來；
	-i  ：列出 inode 號碼，inode 的意義下一章將會介紹；
	-l  ：長資料串列出，包含檔案的屬性與權限等等資料；(常用)
	-n  ：列出 UID 與 GID 而非使用者與群組的名稱 (UID與GID會在帳號管理提到！)
	-r  ：將排序結果反向輸出，例如：原本檔名由小到大，反向則為由大到小；
	-R  ：連同子目錄內容一起列出來，等於該目錄下的所有檔案都會顯示出來；
	-S  ：以檔案容量大小排序，而不是用檔名排序；
	-t  ：依時間排序，而不是用檔名。
*/
int inFunc_ls(char* szParameter1, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_ls START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Path: %s", szSource);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 組命令 */
	if (strlen(szSource) != 0)
	{
		memset(szCommand, 0x00, sizeof(szCommand));
		sprintf(szCommand, "ls ");
		
		if (strlen(szParameter1) != 0)
		{
			strcat(szCommand, szParameter1);
			strcat(szCommand, " ");
		}
		
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}
	}
	/* 沒有來源檔 */
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_System(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{	
		inLogPrintf(AT, "inFunc_ls END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Data_Chmod
Date&Time	:2018/9/5 上午 11:45
Describe        :更改檔案權限
 *		 mode : 許可權設定字串，格式如下 : [ugoa...][[+-=][rwxX]...][,...]，其中 
 *		u 表示該檔案的擁有者，g 表示與該檔案的擁有者屬於同一個群體(group)者，o 表示其他以外的人，a 表示這三者皆是。 
 *		+ 表示增加許可權、- 表示取消許可權、= 表示唯一設定許可權。 
 *		r 表示可讀取，w 表示可寫入，x 表示可執行，X 表示只有當該檔案是個子目錄或者該檔案已經被設定過為可執行。 
 *		-c : 若該檔案許可權確實已經更改，才顯示其更改動作 
 *		-f : 若該檔案許可權無法被更改也不要顯示錯誤訊息 
 *		-v : 顯示許可權變更的詳細資料 
 *		-R : 對目前目錄下的所有檔案與子目錄進行相同的許可權變更(即以遞回的方式逐個變更) 
 *		--help : 顯示輔助說明 
 *		--version : 顯示版本 
 *		
 *		範例 :將檔案 file1.txt 設為所有人皆可讀取 : 
 *		chmod ugo+r file1.txt 
 *		
 *		將檔案 file1.txt 設為所有人皆可讀取 : 
 *		chmod a+r file1.txt 
 * 
 *		將檔案 file1.txt 與 file2.txt 設為該檔案擁有者，與其所屬同一個群體者可寫入，但其他以外的人則不可寫入 : 
 *		chmod ug+w,o-w file1.txt file2.txt 
 *		
 *		將 ex1.py 設定為只有該檔案擁有者可以執行 : 
 *		chmod u+x ex1.py 
 *		
 *		將目前目錄下的所有檔案與子目錄皆設為任何人可讀取 : 
 *		chmod -R a+r * 
 *		
 *		此外chmod也可以用數字來表示許可權如 chmod 777 file 
 *		語法為：chmod abc file 
 *		
 *		其中a,b,c各為一個數字，分別表示User、Group、及Other的許可權。 
 *		
 *		r=4，w=2，x=1 
 *		若要rwx屬性則4+2+1=7； 
 *		若要rw-屬性則4+2=6； 
 *		若要r-x屬性則4+1=7。 
 *		
 *		範例： 
 *		chmod a=rwx file 
 *		
 *		和 
 *		chmod 777 file 
 *		
 *		效果相同 
 *		chmod ug=rwx,o=x file 
 *		
 *		和 
 *		chmod 771 file 
 *		
 *		效果相同 
*/
int inFunc_Data_Chmod(char* szParameter, char *szFileName, char* szSource)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Data_Chmod START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
	}
		
	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (strlen(szFileName) != 0)
	{
		sprintf(szCommand, "chmod  ");
		
		if (strlen(szParameter) != 0)
		{
			strcat(szCommand, szParameter);
			strcat(szCommand, " ");
		}
		
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}

		/* 目標檔名 */
		strcat(szCommand, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFunc_Data_Chmod END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Data_Dir_Get_Size
Date&Time	:2020/7/2 上午 9:57
Describe        :取得資料夾檔案大小
*/
int inFunc_Data_Dir_Get_Size(char* szParameter, char *szFileName, char* szSource, char* szSize)
{
	int	inRetVal = VS_SUCCESS;
	int	inFd = 0;
	int	inReadTempCnt = 0;
	int	inFileSize = 0;
	int	inCurrentCnt = 0;
	char	szDebugMsg[100 + 1] = {0};
	char	szCommand[100 + 1] = {0};	/* Shell Command*/
	char	szReadBuffer[100 + 1] = {0};		/* 檔名buffer */
	char	szReadTempBuffer[100 + 1] = {0};	/* 讀取用buffer */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Data_Dir_Get_Size START!");
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme: %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
	}
		
	/* rm 資料夾路徑/檔名 */
	/* 組命令 */
	memset(szCommand, 0x00, sizeof(szCommand));
	if (strlen(szFileName) != 0)
	{
		sprintf(szCommand, "du  ");
		
		if (strlen(szParameter) != 0)
		{
			strcat(szCommand, szParameter);
			strcat(szCommand, " ");
		}
		
		if (strlen(szSource) != 0)
		{
			strcat(szCommand, szSource);
		}

		/* 目標檔名 */
		strcat(szCommand, szFileName);
	}
	else
	{
		return (VS_ERROR);
	}
	
	/* 執行命令 */
        inRetVal = inFunc_ShellCommand_Popen(szCommand, VS_TRUE);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	inFile_Linux_Open_In_Fs_Data(&inFd, _SHELL_RESPONSE_TEMP_);
	inFile_Linux_Get_FileSize_By_LSeek(inFd, &inFileSize);
	inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
	
	do
	{
		/* 取得檔案大小 */
		memset(szReadBuffer, 0x00, sizeof(szReadBuffer));
		do
		{
			memset(szReadTempBuffer, 0x00, sizeof(szReadTempBuffer));
			inReadTempCnt = 1;
			inFile_Linux_Read(inFd, szReadTempBuffer, &inReadTempCnt);
			inCurrentCnt++;

			/* 行的尾部或是檔案的尾部 */
			/* */
			if (szReadTempBuffer[0] == 0x0A	||
			    szReadTempBuffer[0] == 0x00	||
			    szReadTempBuffer[0] == 0x09)
			{
				break;
			}
			else
			{
				strcat(szReadBuffer, szReadTempBuffer);
			}
			
		} while (inCurrentCnt < inFileSize);
		
		/* 達到檔案尾部 */
		if (szReadTempBuffer[0] == 0x00)
		{
			break;
		}
		
		strcat(szSize, szReadBuffer);
		break;

	}while (1);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "FNaeme : %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Size : %s", szSize);
		inLogPrintf(AT, szDebugMsg);
		
		inLogPrintf(AT, "inFunc_Data_Dir_Get_Size END!");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckFile_In_SD
Date&Time       :2017/1/9 下午 3:01
Describe        :
*/
int inFunc_CheckFile_In_SD_Partial()
{
	char		szPath[100 + 1] = {0};
	char		szPath2[100 + 1] = {0};
	char		szPath3[100 + 1] = {0};
	char		szPath4[100 + 1] = {0};
	char		szPath5[100 + 1] = {0};
	char		szSrcPath2[100 + 1] = {0};
	char		szSrcPath3[100 + 1] = {0};
	char		szSrcPath4[100 + 1] = {0};
	char		szSrcPath5[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szDirName2[100 + 1] = {0};
	char		szDirName3[100 + 1] = {0};
	char		szDirName4[100 + 1] = {0};
	char		szDirName5[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	DISPLAY_OBJECT	srDispMsgObj = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (inFunc_Check_SDCard_Mounted() != VS_SUCCESS)
	{
		memset(&srDispMsgObj, 0x00, sizeof(DISPLAY_OBJECT));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "未掛載SD");
		srDispMsgObj.inErrMsg1Line = _LINE_8_7_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_ChineseFont("SD已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);

	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%s_Check/", _SD_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	sprintf(szDirName2, "%s/%s", szDirName, _FS_DIR_NAME_);
	sprintf(szDirName3, "%s/%s", szDirName, "ICERData");
	sprintf(szDirName4, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szDirName5, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	inFunc_Dir_Make(szDirName, szPath);
	inFunc_Dir_Make(szDirName2, szPath);
	inFunc_Dir_Make(szDirName3, szPath);
	inFunc_Dir_Make(szDirName4, szPath);
	inFunc_Dir_Make(szDirName5, szPath);

	memset(szSrcPath2, 0x00, sizeof(szPath2));
	memset(szSrcPath3, 0x00, sizeof(szPath3));
	memset(szSrcPath4, 0x00, sizeof(szPath4));
	memset(szSrcPath5, 0x00, sizeof(szPath5));
	
	sprintf(szSrcPath2, "%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_);
	sprintf(szSrcPath3, "-r %s/%s/", _AP_ROOT_DIR_NAME_, "ICERData");
	sprintf(szSrcPath4, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szSrcPath5, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	memset(szPath, 0x00, sizeof(szPath));
	memset(szPath2, 0x00, sizeof(szPath2));
	memset(szPath3, 0x00, sizeof(szPath3));
	memset(szPath4, 0x00, sizeof(szPath4));
	memset(szPath5, 0x00, sizeof(szPath5));
	
	sprintf(szPath, "%s%s_Check/%02d%02d%02d_%02d%02d%02d_Nexsys%s/", _SD_PATH_, szModelName, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	sprintf(szPath2, "%s%s/", szPath, _FS_DIR_NAME_);
	sprintf(szPath3, "%s%s/", szPath, "ICERData");
	sprintf(szPath4, "%s%s/%s/", szPath, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szPath5, "%s%s/%s/", szPath, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	inFunc_Data_Copy("*", _AP_ROOT_PATH_, "", szPath);
	inFunc_Data_Copy("*", szSrcPath2, "", szPath2);
	inFunc_Data_Copy("*", szSrcPath3, "", szPath3);
	inFunc_Data_Copy("*", szSrcPath4, "", szPath4);
	inFunc_Data_Copy("*", szSrcPath5, "", szPath5);
	
	/* 因為外接設備,仍需要使用同步 */
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckFile_In_SD_ALL
Date&Time       :2018/6/4 下午 1:58
Describe        :
*/
int inFunc_CheckFile_In_SD_ALL()
{
	char		szPath[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	DISPLAY_OBJECT	srDispMsgObj = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (inFunc_Check_SDCard_Mounted() != VS_SUCCESS)
	{
		memset(&srDispMsgObj, 0x00, sizeof(DISPLAY_OBJECT));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "未掛載SD");
		srDispMsgObj.inErrMsg1Line = _LINE_8_7_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_ChineseFont("SD已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);
	
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%s_Check/", _SD_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	
	inFunc_Dir_Make(szDirName, szPath);

	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s_Check/%02d%02d%02d_%02d%02d%02d_Nexsys%s/", _SD_PATH_, szModelName, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	inFunc_Data_Copy("* -r", _AP_ROOT_PATH_, "", szPath);
	
	/* 因為外接設備,仍需要使用同步 */
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckFile_In_USB_Partial
Date&Time       :2019/2/18 上午 10:35
Describe        :
*/
int inFunc_CheckFile_In_USB_Partial(void)
{
	int		inOrgUSBMode = 0;
	int		inRetVal = VS_ERROR;
	int		inTimeout = 30;		/* 大約8秒即掛載 */
	char		szPath[100 + 1] = {0};
	char		szPath2[100 + 1] = {0};
	char		szPath3[100 + 1] = {0};
	char		szPath4[100 + 1] = {0};
	char		szPath5[100 + 1] = {0};
	char		szSrcPath2[100 + 1] = {0};
	char		szSrcPath3[100 + 1] = {0};
	char		szSrcPath4[100 + 1] = {0};
	char		szSrcPath5[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szDirName2[100 + 1] = {0};
	char		szDirName3[100 + 1] = {0};
	char		szDirName4[100 + 1] = {0};
	char		szDirName5[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	unsigned char	uszKey = 0x00;
	unsigned char	uszCheckUSBBit = VS_FALSE;
	unsigned char	uszCheckTimeoutBit = VS_FALSE;
	unsigned char	uszCheckKeyBit = VS_FALSE;
	RTC_NEXSYS	srRTC = {0};
	DISPLAY_OBJECT	srDispMsgObj = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("等待掛載中 Timeout:", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
	inUSB_Get_Host_Device_Mode(&inOrgUSBMode);
	inUSB_SelectMode(_USB_MODE_HOST_);
	inDISP_TimeoutStart(inTimeout);
	
	while (1)
	{
		inRetVal = inDISP_TimeoutCheck(_FONTSIZE_8X22_, _LINE_8_7_, _DISP_RIGHT_);
		if (inRetVal == VS_TIMEOUT)
		{
			uszCheckTimeoutBit = VS_TRUE;
		}
		
		uszKey = uszKBD_Key();
		if (uszKey == _KEY_CANCEL_)
		{
			uszCheckKeyBit = VS_TRUE;
		}
		
		inRetVal = inFunc_Check_USB_Mounted();
		if (inRetVal == VS_SUCCESS)
		{
			uszCheckUSBBit = VS_TRUE;
		}
		
		if (uszCheckTimeoutBit == VS_TRUE	|| 
		    uszCheckUSBBit == VS_TRUE		||
		    uszCheckKeyBit == VS_TRUE)
		{
			break;
		}
	}
	
	if (uszCheckUSBBit != VS_TRUE)
	{
		memset(&srDispMsgObj, 0x00, sizeof(DISPLAY_OBJECT));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "未掛載USB");
		srDispMsgObj.inErrMsg1Line = _LINE_8_7_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		inUSB_SelectMode(inOrgUSBMode);
		
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_ChineseFont("USB已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中，請稍後", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);

	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);

	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%s_Check/", _USB_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	sprintf(szDirName2, "%s/%s", szDirName, _FS_DIR_NAME_);
	sprintf(szDirName3, "%s/%s", szDirName, "ICERData");
	sprintf(szDirName4, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szDirName5, "%s/%s/%s", szDirName, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	inFunc_Dir_Make(szDirName, szPath);
	inFunc_Dir_Make(szDirName2, szPath);
	inFunc_Dir_Make(szDirName3, szPath);
	inFunc_Dir_Make(szDirName4, szPath);
	inFunc_Dir_Make(szDirName5, szPath);

	memset(szSrcPath2, 0x00, sizeof(szPath2));
	memset(szSrcPath3, 0x00, sizeof(szPath3));
	memset(szSrcPath4, 0x00, sizeof(szPath4));
	memset(szSrcPath5, 0x00, sizeof(szPath5));
	
	sprintf(szSrcPath2, "%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_);
	sprintf(szSrcPath3, "-r %s/%s/", _AP_ROOT_DIR_NAME_, "ICERData");
	sprintf(szSrcPath4, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szSrcPath5, "%s/%s/%s/", _AP_ROOT_DIR_NAME_, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	
	memset(szPath, 0x00, sizeof(szPath));
	memset(szPath2, 0x00, sizeof(szPath2));
	memset(szPath3, 0x00, sizeof(szPath3));
	memset(szPath4, 0x00, sizeof(szPath4));
	memset(szPath5, 0x00, sizeof(szPath5));
	
	sprintf(szPath, "%s%s_Check/%02d%02d%02d_%02d%02d%02d_Nexsys%s/", _USB_PATH_, szModelName, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	sprintf(szPath2, "%s%s/", szPath, _FS_DIR_NAME_);
	sprintf(szPath3, "%s%s/", szPath, "ICERData");
	sprintf(szPath4, "%s%s/%s/", szPath, _FS_DIR_NAME_, _CA_DIR_NAME_);
	sprintf(szPath5, "%s%s/%s/", szPath, _FS_DIR_NAME_, _EMV_EMVCL_DIR_NAME_);
	inFunc_Data_Copy("*", _AP_ROOT_PATH_, "", szPath);
	inFunc_Data_Copy("*", szSrcPath2, "", szPath2);
	inFunc_Data_Copy("*", szSrcPath3, "", szPath3);
	inFunc_Data_Copy("*", szSrcPath4, "", szPath4);
	inFunc_Data_Copy("*", szSrcPath5, "", szPath5);
	
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	inUSB_SelectMode(inOrgUSBMode);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckFile_In_USB_ALL
Date&Time       :2019/2/18 上午 10:35
Describe        :
*/
int inFunc_CheckFile_In_USB_ALL(void)
{
	int		inOrgUSBMode = 0;
	int		inRetVal = VS_ERROR;
	int		inTimeout = 30;
	char		szPath[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	unsigned char	uszKey = 0x00;
	unsigned char	uszCheckUSBBit = VS_FALSE;
	unsigned char	uszCheckTimeoutBit = VS_FALSE;
	unsigned char	uszCheckKeyBit = VS_FALSE;
	RTC_NEXSYS	srRTC = {0};
	DISPLAY_OBJECT	srDispMsgObj = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("等待掛載中 Timeout:", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
	inUSB_Get_Host_Device_Mode(&inOrgUSBMode);
	inUSB_SelectMode(_USB_MODE_HOST_);
	inDISP_TimeoutStart(inTimeout);
	
	while (1)
	{
		inRetVal = inDISP_TimeoutCheck(_FONTSIZE_8X22_, _LINE_8_7_, _DISP_RIGHT_);
		if (inRetVal == VS_TIMEOUT)
		{
			uszCheckTimeoutBit = VS_TRUE;
		}
		
		uszKey = uszKBD_Key();
		if (uszKey == _KEY_CANCEL_)
		{
			uszCheckKeyBit = VS_TRUE;
		}
		
		inRetVal = inFunc_Check_USB_Mounted();
		if (inRetVal == VS_SUCCESS)
		{
			uszCheckUSBBit = VS_TRUE;
		}
		
		if (uszCheckTimeoutBit == VS_TRUE	|| 
		    uszCheckUSBBit == VS_TRUE		||
		    uszCheckKeyBit == VS_TRUE)
		{
			break;
		}
	}
	
	if (uszCheckUSBBit != VS_TRUE)
	{
		memset(&srDispMsgObj, 0x00, sizeof(DISPLAY_OBJECT));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "未掛載USB");
		srDispMsgObj.inErrMsg1Line = _LINE_8_7_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		inUSB_SelectMode(inOrgUSBMode);
		
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("USB已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中，請稍後", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%s_Check/", _USB_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	
	inFunc_Dir_Make(szDirName, szPath);

	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s_Check/%02d%02d%02d_%02d%02d%02d_Nexsys%s/", _USB_PATH_, szModelName, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	inFunc_Data_Copy("* -r", _AP_ROOT_PATH_, "", szPath);
	
	/* 因為外接設備,仍需要使用同步 */
	sync();
		
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	inUSB_SelectMode(inOrgUSBMode);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_ShellCommand_System
Date&Time	:2016/3/25 上午 10:45
Describe        :可輸入字串來執行shell指令，利用system函數，另一種方法使用popen，能比較有效的抓取錯誤訊息(待開發)
*/
int inFunc_ShellCommand_System(char *szCommand)
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (szCommand == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "command 為空pointer");
		}
		return (VS_ERROR);
	}
	
	inRetVal = system(szCommand);
	
	/* 在C 程式裡其中 ret 值要除以256才會得到與shell 傳回相符的值 */
	inRetVal >>= 8;

	if (inRetVal != 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Failed :");
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Error Number : %d", errno);
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "ReturnValue : %d", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Success :");
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_ShellCommand_Popen
Date&Time	:2016/12/30 上午 9:24
Describe        :使用popen，能比較有效的抓取錯誤訊息
 *		uszSaveBit:是否刪除上次暫存，並儲存新內容
*/
int inFunc_ShellCommand_Popen(char *szCommand, unsigned char uszSaveBit)
{
	char		szDebugMsg[1024 *1024 + 1];
	FILE		*pFilePointer;
	unsigned long	ulHandle = 0;
	
	if (szCommand == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "command 為空pointer");
		}
		return (VS_ERROR);
	}
	
	pFilePointer = popen(szCommand, "r");
	
	if (pFilePointer == NULL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Failed :");
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	else
	{
		/* 要存紀錄 */
		if (uszSaveBit == VS_TRUE)
		{
			/* 將shell的回應存在檔案裡面 */
			inFILE_Delete((unsigned char*)_SHELL_RESPONSE_TEMP_);
			inFILE_Create(&ulHandle, (unsigned char*)_SHELL_RESPONSE_TEMP_);
			inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_);
		}
		
		while (fgets(szDebugMsg, sizeof(szDebugMsg), pFilePointer))
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, szDebugMsg);
			}
			
			/* 要存紀錄 */
			if (uszSaveBit == VS_TRUE)
			{
				inFILE_Write(&ulHandle, (unsigned char*)szDebugMsg, strlen(szDebugMsg));
				inFILE_Seek(ulHandle, 0, _SEEK_END_);
			}
		}
		
		pclose(pFilePointer);
		/* 要存紀錄 */
		if (uszSaveBit == VS_TRUE)
		{
			inFILE_Close(&ulHandle);
		}
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Success :");
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "%s", szCommand);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}	

/*
Function        :inFunc_ShellCommand_TextFile_UTF8toBig5
Date&Time       :2017/4/27 下午 4:16
Describe        :
*/
int inFunc_ShellCommand_TextFile_UTF8toBig5(char *szUTF8FileName, char *szBIG5FileName)
{
	int	inReVal = VS_ERROR;
	char	szCommand[100 + 1];
	
	memset(szCommand, 0x00, sizeof(szCommand));
	/* example: iconv -f UTF-8 -t BIG-5 utf8.txt > big5.txt */
	sprintf(szCommand, "%s %s %s %s%s %s%s", "convmv", "-f UTF-8 -t BIG-5", "fs_data/", szUTF8FileName, ">", "fs_data/", szBIG5FileName);
	
	inReVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	
	if (inReVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_ClearAP
Date&Time       :2017/5/8 下午 5:20
Describe        :
*/
int inFunc_ClearAP(char *szFileName)
{
	int	inReVal;
	char	szCommand[100 + 1];
	
	memset(szCommand, 0x00, sizeof(szCommand));
	sprintf(szCommand, "%s %s%s", "rm", "../pub/", szFileName);
	
	inReVal = inFunc_ShellCommand_Popen(szCommand, VS_FALSE);
	
	if (inReVal != (VS_SUCCESS))
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Clear_AP_Dump
Date&Time       :2018/6/19 下午 1:56
Describe        :
*/
int inFunc_Clear_AP_Dump(void)
{
	int	inReVal = VS_ERROR;
	
	inReVal = inFunc_Data_Delete("-r", "*", _APP_UPDATE_PATH_);
	if (inReVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_DiscardSpace
Date&Time	:2016/3/29 下午 3:59
Describe        :去除host後面的空白
*/
int inFunc_DiscardSpace(char *szTemplate)
{
	int	i;
	int	inStringCnt = 0;
	char	szString[1024 + 1] = {0};
	
	do
	{
		if (*(szTemplate + inStringCnt) == 0x00)
		{
			break;
		}
		else if (inStringCnt > 1024)
		{
			break;
		}
		
		inStringCnt++;
	}while(1);
	
	memset(szString, 0x00, sizeof(szString));
	strcat(szString, szTemplate);
	
	for (i = (inStringCnt - 1); i >= 0; i --)
	{
		if (szString[i] != ' ')
			break;
	}
	
	/* 全空白，不做改動 */
	if (i == 0)
	{
		return (VS_SUCCESS);
	}
	
	memset(szTemplate, 0x00, inStringCnt);
	memcpy(szTemplate, szString, i + 1);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Align_Center
Date&Time	:2016/3/29 下午 3:59
Describe        :使未滿八個字的Host，前後補空白置中，Display用
*/
int inFunc_Align_Center(char *szTemplate)
{
	int	i, j;
	int	inSpaceNum = 0;
	int	inHalfSpaceNum = 0;
	char	szHost[8 + 1];
	
	memset(szHost, 0x00, sizeof(szHost));
	strcat(szHost, szTemplate);
	
	for (i = 7; i >= 0; i --)
	{
		if (szHost[i] != ' ')
			break;
	}
	
	/* Host全空白，不做改動 */
	if (i == 0)
	{
		return (VS_SUCCESS);
	}
	
	memset(szTemplate, 0x00, 8);
	if (i + 1 < 8)
	{
		inSpaceNum = 8 - (i + 1);
		inHalfSpaceNum = inSpaceNum / 2;
		/* 補前面空白 */
		for (j = 0; j < inHalfSpaceNum; j++)
		{
			strcat(szTemplate, "0");
		}
		
		/* 塞中間Host */
		strcat(szTemplate, szHost);
		
		/* 塞後面空白 */
		for (j = 0; j < inSpaceNum - inHalfSpaceNum; j++)
		{
			strcat(szTemplate, "0");
		}
	}
	else
	{
		memcpy(szTemplate, szHost, i + 1);
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_CheckFullSpace
Date&Time	:2016/7/5 下午 5:07
Describe        :確認陣列中有值的地方是否全是0X20，確認電文中的值使用
*/
int inFunc_CheckFullSpace(char* szString)
{
	int	i;
	
	for (i = 0; i < strlen(szString); i++)
	{
		if (szString[i] != 0x20)
		{
			return (VS_FALSE);
		}
	}
	
	return	(VS_TRUE);
}

/*
Function	:inFunc_CheckFullSpaceWithLen
Date&Time	:2021/9/27 下午 3:01
Describe        :確認陣列中有值的地方是否全是0X20，確認電文中的值使用
*/
int inFunc_CheckFullSpaceWithLen(char* szString, int inLen)
{
	int	i;
	
	for (i = 0; i < inLen; i++)
	{
		if (szString[i] != 0x20)
		{
			return (VS_FALSE);
		}
	}
	
	return	(VS_TRUE);
}

/*
Function	:inFunc_HostName_DecideByTRT
Date&Time	:2016/4/14 下午 3:19
Describe	:用TRT決定HostName
*/
int inFunc_HostName_DecideByTRT(TRANSACTION_OBJECT * pobTran, char *szHostName)
{
	char	szTRTFName[16 + 1];
	char	szDebugMsg[100 + 1];
	char	szHostLabel[8 + 1];
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFunc_HostName_DecideByTRT()_START");
        }
	
	/* 取出TRT FileName */
        memset(szTRTFName, 0x00, sizeof(szTRTFName));
        if (inGetTRTFileName(szTRTFName) == VS_ERROR)
                return (VS_ERROR);

	/* 組檔名，交易存檔檔案名稱 = (Host Name + Batch Number + .bat) */
        memset(szHostName, 0x00, sizeof(szHostName));
        if (!memcmp(szTRTFName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
	{
		memset(szHostLabel, 0x00, sizeof(szHostLabel));
		inGetHostLabel(szHostLabel);
		
		if  (0)
		{
			
		}
		else if (memcmp(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
		{
			sprintf(szHostName, "%s", _FILE_NAME_CREDIT_NCCC_);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "檔名error TRT:%s Host:%s", szTRTFName, szHostLabel);
			}
			
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			strcpy(srDispMsgObj.szErrMsg1, "檔名流程錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
			return (VS_ERROR);
		}
	}
        else if (!memcmp(szTRTFName, _TRT_FILE_NAME_REDEMPTION_, strlen(_TRT_FILE_NAME_REDEMPTION_)))
                sprintf(szHostName, "%s", _FILE_NAME_REDEMPTION_);
        else if (!memcmp(szTRTFName, _TRT_FILE_NAME_INSTALLMENT_, strlen(_TRT_FILE_NAME_INSTALLMENT_)))
                sprintf(szHostName, "%s", _FILE_NAME_INSTALLMENT_);
        else if (!memcmp(szTRTFName, _TRT_FILE_NAME_AMEX_, strlen(_TRT_FILE_NAME_AMEX_)))
                sprintf(szHostName, "%s", _FILE_NAME_AMEX_);
        else if (!memcmp(szTRTFName, _TRT_FILE_NAME_DINERS_, strlen(_TRT_FILE_NAME_DINERS_)))
                sprintf(szHostName, "%s", _FILE_NAME_DINERS_);
        else if (!memcmp(szTRTFName, _TRT_FILE_NAME_UNION_PAY_, strlen(_TRT_FILE_NAME_UNION_PAY_)))
                sprintf(szHostName, "%s", _FILE_NAME_UNION_PAY_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_DCC_, strlen(_TRT_FILE_NAME_DCC_)))
                sprintf(szHostName, "%s", _FILE_NAME_DCC_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_ESC_, strlen(_TRT_FILE_NAME_ESC_)))
                sprintf(szHostName, "%s", _FILE_NAME_ESC_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_HG_, strlen(_TRT_FILE_NAME_HG_)))
                sprintf(szHostName, "%s", _FILE_NAME_HG_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_ESVC_, strlen(_TRT_FILE_NAME_ESVC_)))
	{
		if (pobTran->uszESVCFileNameByCardBit == VS_TRUE)
		{
			if (pobTran->srTRec.inTDTIndex == _TDT_INDEX_00_IPASS_)
			{
				sprintf(szHostName, "%s", _FILE_NAME_IPASS_);
			}
			else if (pobTran->srTRec.inTDTIndex == _TDT_INDEX_01_ECC_)
			{
				sprintf(szHostName, "%s", _FILE_NAME_EASYCARD_);
			}
			else if (pobTran->srTRec.inTDTIndex == _TDT_INDEX_02_ICASH_)
			{
				sprintf(szHostName, "%s", _FILE_NAME_ICASH_);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "檔名error TRT:%s Host:%d", szTRTFName, pobTran->srTRec.inTDTIndex);
				}
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = 0;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 1;
				strcpy(srDispMsgObj.szErrMsg1, "檔名流程錯誤");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
		
				inDISP_Msg_BMP(&srDispMsgObj);
				return (VS_ERROR);
			}
		}
		else
		{
			sprintf(szHostName, "%s", _FILE_NAME_ESVC_);
		}
	}
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_TAKA_, strlen(_TRT_FILE_NAME_TAKA_)))
                sprintf(szHostName, "%s", _FILE_NAME_TAKA_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_EW_, strlen(_TRT_FILE_NAME_EW_)))
                sprintf(szHostName, "%s", _FILE_NAME_EW_);
	else if (!memcmp(szTRTFName, _TRT_FILE_NAME_TRUST_, strlen(_TRT_FILE_NAME_TRUST_)))
                sprintf(szHostName, "%s", _FILE_NAME_TRUST_);
        else
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "Can't Compose Filename : TRTFName = %s !!", szTRTFName);
                        inLogPrintf(AT, szDebugMsg);
                }
                return (VS_ERROR);
        }
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFunc_HostName_DecideByTRT()_END");
		inLogPrintf(AT, "----------------------------------------");
        }
	
	return VS_SUCCESS;
}

/*
Function	:inFunc_ComposeFileName
Date&Time	:2016/4/14 下午 1:13
Describe        :覺得很常用，而且方便之後維護這部份
*/
int inFunc_ComposeFileName(TRANSACTION_OBJECT *pobTran, char *szFileName, char *szFileExtension, int inBatchNumWidth)
{
	char	szDebugMsg[84 + 1];
	char    szHostName[8 + 1];
	char	szSprintfArgument[10 + 1];
	char	szTemplate[16 + 1];
	long    lnBatchNum = 0;                             /* 放HDPT中的Batch number */
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFunc_ComposeFileName()_START");
        }
	
	if (inBatchNumWidth > 0)
	{
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inGetBatchNum(szTemplate) == VS_ERROR)
			return (VS_ERROR);

		lnBatchNum = atol(szTemplate);

		/* 若BatchNum小於1，則不合法 */
		if (lnBatchNum < 1)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "illegal BatchNum.(batchNum = %ld)ERROR!", lnBatchNum);
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}
	}
	
	/* 沒有寬度，不考慮批次 */
	if (inBatchNumWidth > 0)
	{
		/* "%s%06lu%s" */
		memset(szSprintfArgument, 0x00, sizeof(szSprintfArgument));
		sprintf(szSprintfArgument, "%s%d%s", "%s%0", inBatchNumWidth, "ld%s");
	}
	else
	{
		/* "%s%06lu%s" */
		memset(szSprintfArgument, 0x00, sizeof(szSprintfArgument));
		sprintf(szSprintfArgument, "%s%s", "%s", "%s");
	}
	
	/* 藉由TRT_FileName比對來組出amt的檔名 */
	memset(szHostName, 0x00, sizeof(szHostName));
	if (pobTran->uszFileNameNoNeedHostBit == VS_TRUE)
	{
		strcpy(szHostName, "ALL");
	}
	else
	{
		inFunc_HostName_DecideByTRT(pobTran, szHostName);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Bef Intgreat %s", szHostName);
		inLogPrintf(AT, szDebugMsg);
		
		/* printf遇到szSprintfArgument內的字串，會認錯，以為後面有引數 */
//		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//		sprintf(szDebugMsg, "%s", szSprintfArgument);
//		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 組成FileName */
	if (inBatchNumWidth > 0)
	{
		sprintf(szFileName, szSprintfArgument, szHostName, lnBatchNum, szFileExtension);
	}
	else
	{
		sprintf(szFileName, szSprintfArgument, szHostName, szFileExtension);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "Get ComposeFileName is %s", szFileName);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
        {
		inLogPrintf(AT, "inFunc_ComposeFileName()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_ComposeFileName_InvoiceNumber
Date&Time	:2016/5/4 上午 10:38
Describe        :考慮到同一InvoiceNumber會有取消、調帳等操作，會有多個簽名圖檔
*/
int inFunc_ComposeFileName_InvoiceNumber(TRANSACTION_OBJECT *pobTran, char *szFileName, char *szFileExtension, int inInvNumWidth)
{
	char	szDebugMsg[84 + 1];
	char    szHostName[8 + 1];
	char	szSprintfArgument[30 + 1];
	long    lnInvNum;                             /* 放HDPT中的Batch number */
	
	if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inFunc_ComposeFileName_InvoiceNumber()_START");
        }
	
	/* 將pobTran中的BatchNum取出 */
        lnInvNum = pobTran->srBRec.lnOrgInvNum;
	
	/* 若BatchNum小於1，則不合法 */
        if (lnInvNum < 1)
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "illegal InvoiceNum.(InvoiceNum = %ld)ERROR!", lnInvNum);
                        inLogPrintf(AT, szDebugMsg);
                }
                return (VS_ERROR);
        }
	
	/* "%s%06lu%s" */
	memset(szSprintfArgument, 0x00, sizeof(szSprintfArgument));
	switch (pobTran->srBRec.inCode)
	{
		case _VOID_:
		case _CUP_VOID_:
		case _CUP_PRE_AUTH_VOID_:
		case _CUP_PRE_COMP_VOID_:
			sprintf(szSprintfArgument, "%s%d%s%s%s", "%s%0", inInvNumWidth, "ld", "_1", "%s");
			break;
		case _TIP_ :
			sprintf(szSprintfArgument, "%s%d%s%s%s", "%s%0", inInvNumWidth, "ld", "_2", "%s");
			break;
		case _ADJUST_ :
			sprintf(szSprintfArgument, "%s%d%s%s%s", "%s%0", inInvNumWidth, "ld", "_3", "%s");
			break;
		default :
			sprintf(szSprintfArgument, "%s%d%s%s%s", "%s%0", inInvNumWidth, "ld", "", "%s");
			break;
	}
	
	
	/* 藉由TRT_FileName比對來組出amt的檔名 */
	memset(szHostName, 0x00, sizeof(szHostName));
	inFunc_HostName_DecideByTRT(pobTran, szHostName);
	
	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", szHostName);
		inLogPrintf(AT, szDebugMsg);
		
		/* printf遇到szSprintfArgument內的字串，會認錯，以為後面有引數 */
//		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//		sprintf(szDebugMsg, "%s", szSprintfArgument);
//		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 組成FileName */
	/* 檔名最長15，所以清15 */
	memset(szFileName, 0x00, 15);
        sprintf(szFileName, szSprintfArgument, szHostName, lnInvNum, szFileExtension);
	
	if (ginDebug == VS_TRUE)
	{
		char szDebugMsg[100 + 1];

		memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		sprintf(szDebugMsg, "%s", szFileName);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (ginDebug == VS_TRUE)
        {
		inLogPrintf(AT, "inFunc_ComposeFileName_InvoiceNumber()_END");
                inLogPrintf(AT, "----------------------------------------");
        }
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_REFERRAL_GetManualApproval
Date&Time	:2016/4/19 下午 8:08
Describe        :Call Bank使用 輸入授權碼
*/
int inFunc_REFERRAL_GetManualApproval(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = 0;
	int		inChoice = 0;
    	int		i = 0;
    	char		szDispMsg[128 + 1];
    	char		szTemplate[100];
    	char		szLocalPhoneNum[30 + 1];
        char		szCommMode[2 + 1], szCallBankEnable[2 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
        unsigned char   uszKey;
	unsigned char	uszCheckReferralNo = VS_TRUE;		/* 確認電話號碼是否合法 */		
	unsigned char	uszRetryAgain = VS_FALSE;		/* 表示要重試 */

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inFunc_REFERRAL_GetManualApproval START!!");
        
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/*
		交易在這裡結束，交易不論成功或失敗都在這裡斷線
		交易以前做法是要全部斷網路線，新的做法只要關【SOCKET】，【GPRS】同樣的做法
	*/
	inCOMM_End(pobTran);

	if (pobTran->uszECRBit == VS_TRUE)
	{
		inECR_SendError(pobTran, _ECR_RESPONSE_CODE_CALLBANK_);
	}

	if (inLoadCPTRec(0) < 0)
	{
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "inLoadCPTRec(0)_ERR");
                        inLogPrintf(AT, szTemplate);
                }
                
		return (VS_ERROR);
	}

	memset(szLocalPhoneNum, 0x00, sizeof(szLocalPhoneNum));
        /* 聯合版本只給區域號碼 */
        inGetReferralTel(szLocalPhoneNum);
        
	/*
		含區號的電話號碼。(左靠右補空白)
		3碼區域號+12碼電話號碼
		例：
		台北〈0227191919〉
		端末機依TMS下載的第三支電話中區域碼判斷是否需要 將Referral Telephone no.前的區碼過濾掉
		Table ID”NA”(Call Bank Referral Telephone no.)下載的電話號碼中間不會 有空白或’-’
	*/
	if (!memcmp(&pobTran->szReferralPhoneNum[0], &szLocalPhoneNum[0], strlen(szLocalPhoneNum)))
	{
		/* 表示區碼相同 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		strcpy(szTemplate, &pobTran->szReferralPhoneNum[strlen(szLocalPhoneNum)]);
		memset(pobTran->szReferralPhoneNum, 0x00, sizeof(pobTran->szReferralPhoneNum));
		strcpy(pobTran->szReferralPhoneNum, szTemplate);
	}

        
	
	/* CallBank Enable */
	memset(szCallBankEnable, 0x00, sizeof(szCallBankEnable));
        inGetCallBankEnable(szCallBankEnable);
	if (szCallBankEnable[0] != 'Y')
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
		/* 【需求單-111190】行動支付交易顯示畫面調整V3排除以下客製化外須開發(下列客製化保持原規則) :
		001、006、010、011、013、019、020、024、028、031、032、036、040~044、048、060、065、068、073、077、078、081、084、087、089、092~094、100、104~107、109、111~119
		 * 因需求顯示回應碼
		*/
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_001_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_006_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_010_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_011_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_013_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_019_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_020_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_024_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_028_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_031_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_032_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_036_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_040_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_044_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_048_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_060_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_065_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_068_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_073_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_077_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_078_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_081_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_084_ON_US_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_087_ON_US_NO_ID_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_089_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_092_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_093_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_094_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_100_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_104_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_106_ON_US_BCA_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_109_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_112_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_113_H_AND_M_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_114_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_115_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_116_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_117_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_118_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_119_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE && pobTran->srBRec.inCode == _SALE_)
			{
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, "%s", pobTran->srBRec.szRespCode);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			}
		}
		
		inDISP_PutGraphic(_ERR_CALL_BANK_, 0, _COORDINATE_Y_LINE_8_5_);		/* 請聯絡銀行 */
		
		inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);			/* 請按確認鍵 */
		inDISP_BEEP(1, 0);
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_O_LINE8_8_);
			uszKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (inChoice == _Touch_O_LINE8_8_ENTER_BUTTON_	||
			    uszKey == _KEY_ENTER_			||
			    uszKey == _KEY_TIMEOUT_)
			{
				break;
			}
			else
			{
				continue;
			}
		}   

		/* 使用者終止交易 */
		pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
		return (VS_USER_CANCEL);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
		/* 【需求單-111190】行動支付交易顯示畫面調整V3排除以下客製化外須開發(下列客製化保持原規則) :
		001、006、010、011、013、019、020、024、028、031、032、036、040~044、048、060、065、068、073、077、078、081、084、087、089、092~094、100、104~107、109、111~119
		 * 因需求顯示回應碼
		*/
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_001_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_006_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_010_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_011_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_013_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_019_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_020_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_024_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_028_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_031_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_032_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_036_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_040_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_041_CASH_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_042_BDAU1_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_043_BDAU9_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_044_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_048_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_060_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_065_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_068_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_073_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_077_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_078_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_081_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_084_ON_US_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_087_ON_US_NO_ID_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_089_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_092_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_093_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_094_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_100_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_104_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_106_ON_US_BCA_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_109_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_112_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_113_H_AND_M_, _CUSTOMER_INDICATOR_SIZE_)	||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_114_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_115_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_116_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_117_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_118_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_119_, _CUSTOMER_INDICATOR_SIZE_)		||
		!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
		}
		else
		{
			if (pobTran->srBRec.uszContactlessBit == VS_TRUE && pobTran->srBRec.inCode == _SALE_)
			{
				memset(szDispMsg, 0x00, sizeof(szDispMsg));
				sprintf(szDispMsg, "%s", pobTran->srBRec.szRespCode);
				inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			}
		}
		inDISP_PutGraphic(_ERR_CALL_BANK_, 0, _COORDINATE_Y_LINE_8_5_);			/* 請聯絡銀行 */
	
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "TEL : %s", pobTran->szReferralPhoneNum);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);		/* 顯示電話號碼 */
	
		/* （十一）
		 * CUP 之 Preauth 交易，若主機回覆 Call bank 時，端末機請轉為 “拒絕交易” 訊息，
		 * CUP 之一般交易，若主機回覆 Call bank 時，端末機請提示完撥號訊息後，
		 * 繼續顯示請聯絡銀行，按【清除】回主畫面，且不可接續以補登流程完成交易。
		 */
		if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
		{
			inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_); /* 請按清除鍵 */
			inDISP_BEEP(1, 0);
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
			
			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_X_LINE8_8_);
				uszKey = uszKBD_Key();
				
				/* Timeout */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (inChoice == _Touch_X_LINE8_8_CANCEL_BUTTON_	||
				    uszKey == _KEY_CANCEL_			||
				    uszKey == _KEY_TIMEOUT_)
				{
					break;
				}
				else
				{
					continue;
				}
			}   

			/* 使用者終止交易 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
			return (VS_USER_CANCEL);          
		}
		else
		{
			/* 請按確認或清除鍵 */
			inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
			inDISP_BEEP(3, 500);
			inFlushKBDBuffer();
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

			while (1)
			{
				inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_OX_LINE8_8_);
				uszKey = uszKBD_Key();

				/* Timeout */
				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					uszKey = _KEY_TIMEOUT_;
				}

				if (inChoice == _Touch_OX_LINE8_8_CANCEL_BUTTON_	||
				    uszKey == _KEY_CANCEL_				|| 
				    uszKey == _KEY_TIMEOUT_)
				{
					/* 使用者終止交易 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
					return (VS_USER_CANCEL);
				}
				else if (inChoice == _Touch_OX_LINE8_8_ENTER_BUTTON_	||
					 uszKey == _KEY_ENTER_)
				{
					break;
				}
				else
				{
					continue;
				}
			}
		}
		/* 去除電話號碼中非數字的字元 */
		vdFunc_FilterTel(pobTran->szReferralPhoneNum);

		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);

		if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) != 0 &&
		    memcmp(szCommMode, _COMM_3G_MODE_, 1) != 0	&&
		    memcmp(szCommMode, _COMM_4G_MODE_, 1) != 0)
		{
			/* 沒電話號碼 */
			if (strlen(pobTran->szReferralPhoneNum) == 0)
				uszCheckReferralNo = VS_FALSE;

			/* 檢查【Referral】號碼 */
			for (i = 0; i < strlen(pobTran->szReferralPhoneNum); i ++)
			{
				/* 如果號碼不是數字 */
				if ((pobTran->szReferralPhoneNum[i] >= 0x3A) || (pobTran->szReferralPhoneNum[i] <= 0x2F))
				{
					uszCheckReferralNo = VS_FALSE;
					break;
				}
			}

			if (uszCheckReferralNo == VS_TRUE)
			{
				/* 撥號 */
				inRetVal = inNCCC_Func_Dial_VoiceLine((unsigned char*)pobTran->szReferralPhoneNum, strlen(pobTran->szReferralPhoneNum));
				if (inRetVal == VS_USER_CANCEL)
				{
					/* 使用者終止交易 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
					return (VS_USER_CANCEL);
				}
				else
				{
					/* 繼續執行*/
				}
			}
		}

		/* 顯示授權碼補登 */
		inDISP_ClearAll();
		inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CALL_BANK_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 授權碼補登 */
		/* 輸入管理號碼 */
		/* 第一層輸入密碼 */
		if (inFunc_CheckCustomizePassword(_ACCESS_WITH_CUSTOM_, _CALL_BANK_) != VS_SUCCESS)
			return (VS_ERROR);

		/* 提示防詐騙訊息 */
		inRetVal = inNCCC_Func_Disclaim_Auth(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			/* 使用者終止交易 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
			return (VS_USER_CANCEL);
		}

		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
		{
			/* 偵測晶片插卡 */
			if (inEMV_ICCEvent() != VS_SUCCESS)
			{
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;

				return (VS_ERROR);
			}

			inFunc_REFERRAL_DisplayPan(pobTran);
			inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_8_); /* 請按確認建 */
			inDISP_BEEP(1, 0);
			inFlushKBDBuffer();

			while (1)
			{
				uszKey = uszKBD_GetKey(30);

				if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
				{
					/* 使用者終止交易 */
					pobTran->inErrorMsg = _ERROR_CODE_V3_USER_CANCEL_;
					return (VS_USER_CANCEL);  
				}
				else if (uszKey == _KEY_ENTER_)
				{
					break;
				}
				else
				{
					continue;
				}
			}
		}

		/* 清空授權碼，避免因原交易的授權碼跳過 */
		memset(pobTran->srBRec.szAuthCode, 0x00, sizeof(pobTran->srBRec.szAuthCode));
		
		while(1)
		{

			/* 輸入授權碼 */
			if (inCREDIT_Func_GetAuthCode(pobTran) != VS_SUCCESS)
				return (VS_ERROR);

			/* 檢核授權碼字元是否合法 */
			if (inFunc_Check_AuthCode_Validate(pobTran->srBRec.szAuthCode) != VS_SUCCESS)
			{
				inDISP_BEEP(1, 0);
				uszRetryAgain = VS_TRUE;
			}

			/* 2012-05-24 AM 11:38:23 add by kakab 修正call bank授權碼輸入時須檢查是否為全0，以免後續錯誤 */
			if (uszRetryAgain == VS_TRUE ||
			    (pobTran->srBRec.uszCUPTransBit != VS_TRUE && !memcmp(pobTran->srBRec.szAuthCode, "000000", 6)))
			{
				/* 2011-04-30 PM 04:47:08 有亂碼要有提示語  在回到輸入畫面 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("授權碼檢查錯誤", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
				inDISP_BEEP(2, 500);

				continue;
			}

			if (strlen(pobTran->srBRec.szAuthCode) >= 2)
			{
				/* 授權碼輸入六碼後 按消字鍵再按【ENTER】數字後有亂碼 */
				pobTran->srBRec.szAuthCode[6] = 0x00;
				break; /* 授權碼最少要 2 碼 */
			}
		} /* End while () .... */

		if (pobTran->srBRec.inChipStatus == _EMV_CARD_)
		{
			/* 偵測晶片插卡 */
			if (inEMV_ICCEvent() != VS_SUCCESS)
			{
				/* 晶片卡被取出 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_EMV_CARD_OUT_;

				return (VS_ERROR);
			}
		}
	}/* 授權碼補登開啟 END */

	/* [114197] 中心收單作業ESG永續發展優化 
	 * [260610] 交易轉Call Bank，強制列印紙本簽單，因此在輸入授權碼後，設定szAgreeEsgBill為不同意
	 * [260612] 新增 uszEsgOptrionBit 條件 */
	pobTran->szAgreeEsgBill[0] = 'N';
	pobTran->uszEsgOptrionBit = VS_TRUE;
	pobTran->szEsgSendCR[0] = 'N';
	inLogPrintf(AT, "After Input Auth Code,Set szAgreeEsgBill to N");
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inFunc_REFERRAL_GetManualApproval END!!");

	return (VS_SUCCESS);
}

/*
Function	:vdFunc_FilterTel
Date&Time	:2016/7/25 上午 11:44
Describe        :去除電話號碼中非數字之字元
*/
void vdFunc_FilterTel(char *tel)
{
    	int  i , j ;
    	char temp_tel[17] ;

    	memset ( temp_tel , 0x00 , 17 ) ;
    	j = 0 ;

    	for ( i = 0 ; i < 17 ; i ++ )
    	{
        	if ( ( *( tel + i ) >= '0' ) && (  *( tel + i ) <= '9' ) )
        	{
            		temp_tel[j] = *( tel + i ) ;
            		j ++ ;
        	}
    	}

    	for ( i = 0 ; i < 17 ; i ++ )
    	{
        	*( tel + i ) = temp_tel[i] ;
    	}
}

/*
Function	:inFunc_REFERRAL_DisplayPan
Date&Time	:2016/7/25 上午 11:44
Describe        :顯示卡號
*/
int inFunc_REFERRAL_DisplayPan(TRANSACTION_OBJECT *pobTran)
{
	char	szDispMsg[21 + 1];
	char	szExpDate[4 + 3];
	char	szFinalPAN[_PAN_UCARD_SIZE_ + 1];

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("請確認卡號及有效期", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
	/* Disp Card Number */
	if (strlen(pobTran->srBRec.szExpDate) > 0)
	{
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);

		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		sprintf(szDispMsg, " %s",szFinalPAN);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
	}
		
	if (strlen(pobTran->srBRec.szExpDate) > 0)
	{
		memset(szExpDate, 0x00, sizeof(szExpDate));
		strcpy(szExpDate, pobTran->srBRec.szExpDate);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "MM/'YY = %.2s/'%.2s", szExpDate + 2, szExpDate);
                inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_CENTER_);
	}

	return (VS_SUCCESS);
}

/*
Function	:inFunc_DebugSwitch
Date&Time	:2016/4/22 下午 2:08
Describe        :功能鍵進入 Debug的開關設定
*/
int inFunc_DebugSwitch(void)
{
        int		inRetVal;
        char		szTemplate[64 + 1];
	char		szDebug[50 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
        
        /* Debug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
        if (ginDebug == VS_TRUE)
                strcpy(szTemplate, "Debug 開關 : ON");
        else
                strcpy(szTemplate, "Debug 開關 : OFF");
 
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第一個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[0] = '0';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_Debug_Switch();
				
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
                                /* 只設定第一個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[0] = '1';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_Debug_Switch();
				
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }
        
        /* ISODebug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
        if (ginISODebug == VS_TRUE)
                strcpy(szTemplate, "ISODebug: ON");
        else
                strcpy(szTemplate, "ISODebug: OFF");
 
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第二個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[1] = '0';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_ISODebug_Switch();
				
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				/* 只設定第二個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[1] = '1';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_ISODebug_Switch();
				
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }
	
	/* DisplayDebug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
        if (ginDisplayDebug == VS_TRUE)
                strcpy(szTemplate, "DisplayDebug: ON");
        else
                strcpy(szTemplate, "DisplayDebug: OFF");
 
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第三個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[2] = '0';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_DisplayDebug_Switch();
				
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				/* 只設定第三個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[2] = '1';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_DisplayDebug_Switch();
				
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }
	
	/* EngineerDebug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
        if (ginEngineerDebug == VS_TRUE)
                strcpy(szTemplate, "EngineerDebug: ON");
        else
                strcpy(szTemplate, "EngineerDebug: OFF");
 
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第四個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[3] = '0';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_EngineerDebug_Switch();
				
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				/* 只設定第四個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[3] = '1';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_EngineerDebug_Switch();
				
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }
	
	/* ginFindRunTime */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
        if (ginFindRunTime == VS_TRUE)
                strcpy(szTemplate, "ginFindRunTime: ON");
        else
                strcpy(szTemplate, "ginFindRunTime: OFF");
 
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				ginFindRunTime = VS_FALSE;
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				ginFindRunTime = VS_TRUE;
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }
	
	/* ESCDebug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
	memset(szDebug, 0x00, sizeof(szDebug));
	inGetISODebug(szDebug);
        if (szDebug[_ISO_DEBUG_INDEX_ESCDEBUG_] == '1')
                strcpy(szTemplate, "ESC Debug 開關: ON");
        else
                strcpy(szTemplate, "ESC Debug 開關: OFF");
 
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("1 = ON , 0 = OFF", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);

        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
				/* 只設定第六個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[_ISO_DEBUG_INDEX_ESCDEBUG_] = '0';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_ESCDebug_Switch();
				
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
				/* 只設定第六個位置 */
				memset(szTemplate, 0x00, sizeof(szTemplate));
				inGetISODebug(szTemplate);
				szTemplate[_ISO_DEBUG_INDEX_ESCDEBUG_] = '1';
				
				inSetISODebug(szTemplate);
				inSaveEDCRec(0);
				inFunc_Sync_ESCDebug_Switch();
				
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }
        
        return (VS_SUCCESS);
}

/*
Function	:inFunc_EDCLock
Date&Time	:2016/9/1 下午 4:36
Describe        :設定EDCLOCK為1，並重開機
*/
int inFunc_EDCLock(char* szlocation)
{
	char	szDebugMsg[100 + 1];
	
	inSetEDCLOCK("Y");
	inSaveEDCRec(0);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "！！鎖機！！");
		inLogPrintf(AT, szDebugMsg);
	}
        
        inUtility_StoreTraceLog_OneStep("Lock: %s", szlocation);
	
	DISPLAY_OBJECT	srDispMsgObj;
	memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
	strcpy(srDispMsgObj.szDispPic1Name, _ERR_EDC_LOCK_);
	srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
	srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
	srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
	strcpy(srDispMsgObj.szErrMsg1, "");
	srDispMsgObj.inErrMsg1Line = 0;
	srDispMsgObj.inBeepTimes = 1;
	srDispMsgObj.inBeepInterval = 0;
	inDISP_Msg_BMP(&srDispMsgObj);
	
	inFunc_Reboot();
	
	
	return	(VS_SUCCESS);
}

/*
Function	:inFunc_Check_EDCLock
Date&Time	:2016/6/2 上午 11:54
Describe        :確認是否鎖機
*/
int inFunc_Check_EDCLock(void)
{
	int	i;
	char	szEDCLock[2 + 1];
	char	szHostName[8 + 1];
	
	memset(szEDCLock, 0x00, sizeof(szEDCLock));
	inLoadEDCRec(0);
	inGetEDCLOCK(szEDCLock);
	if (szEDCLock[0] == 'Y')
	{
		for (i = 0 ;; i ++)
		{
			if (inLoadHDTRec(i) < 0)	/* 主機參數檔【HostDef.txt】 */
				break;
			
			memset(szHostName, 0x00, sizeof(szHostName));
			inGetHostLabel(szHostName);
			if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0	||
			    memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
			{
				continue;
			}
			
			inLoadHDPTRec(i);
			inSetMustSettleBit("Y");
			inSaveHDPTRec(i);
		}
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_EDC_LOCK_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}
	else
	{
		return	(VS_SUCCESS);
	}
	
}

/*
Function        :inFunc_Unlock_EDCLock_Flow
Date&Time       :2017/3/28 下午 2:07
Describe        :
*/
int inFunc_Unlock_EDCLock_Flow()
{
	char	szKey;
	
	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("1.只解鎖EDC", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("2.解鎖EDC並清除全部批次", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
		
		szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
		if (szKey == _KEY_1_)
		{
			inFunc_Unlock_EDCLock();
			break;
		}
		else if (szKey == _KEY_2_)
		{
			inFunc_Unlock_EDCLock_And_Delete_Batch();
			break;
		}
		else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
		{
			break;
		}
		
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Unlock_EDCLock
Date&Time	:2016/8/16 下午 3:14
Describe        :解鎖鎖機狀態
*/
int inFunc_Unlock_EDCLock(void)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey;
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("EDC UNLOCK？", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
	inDISP_BEEP(1, 0);
	
	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		if (uszKey == _KEY_0_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			return (VS_USER_CANCEL);
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			return (VS_TIMEOUT);
		}
		
	}
	
	
	inSetEDCLOCK("N");
	inSaveEDCRec(0);
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("EDC UNLOCK OK", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	
	inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
        inDISP_BEEP(1, 0);
                
	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		if (uszKey == _KEY_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		
	}
	
	return	(inRetVal);
}

/*
Function	:inFunc_Unlock_EDCLock_And_Delete_Batch
Date&Time	:2017/3/28 下午 2:11
Describe        :解鎖鎖機狀態並清除全部批次
*/
int inFunc_Unlock_EDCLock_And_Delete_Batch(void)
{
	int			i;
	int			inRetVal = VS_SUCCESS;
	char			szHostEnable[2 + 1];
	char			szHostName[42 + 1];
	unsigned char		uszKey;
	TRANSACTION_OBJECT	pobTran;
	
	memset(&pobTran, 0x00, sizeof(pobTran));
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("EDC UNLOCK &", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_ChineseFont("DELETE BATCH？", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	inDISP_PutGraphic(_ERR_0_, 0, _COORDINATE_Y_LINE_8_7_);
	inDISP_BEEP(1, 0);
	
	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		if (uszKey == _KEY_0_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			return (VS_USER_CANCEL);
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			return (VS_TIMEOUT);
		}
		
	}
	
	inSetEDCLOCK("N");
	inSaveEDCRec(0);
	
	for (i = 0;; ++i)
        {
                /* 先LoadHDT */
                if (inLoadHDTRec(i) == VS_ERROR)
                {
                        /* 當找不到第i筆資料會回傳VS_ERROR */
                        break;
                }
		
		/* ESC不該出現在選單上 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}
		
		/* HG不該出現在選單上(結帳、小費、交易查詢) */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
		{
			continue;
		}

                /* GET HOST Enable */
                memset(szHostEnable, 0x00, sizeof(szHostEnable));
                if (inGetHostEnable(szHostEnable) == VS_ERROR)
                {	
                        return (VS_ERROR);
                }
                else
                {
                        if (memcmp(szHostEnable, "Y", 1) != 0)
                        {
                                /* 如果HostEnable != Y，就continue */
                                continue;
                        }

			pobTran.srBRec.inHDTIndex = i;
			
                        /* 如果主機有開，才loadHDPT */
                        if (inLoadHDPTRec(pobTran.srBRec.inHDTIndex) == VS_ERROR)
                        {
                                /* 當找不到第i筆資料會回傳VS_ERROR */
                                return (VS_ERROR);
                        }
			
			inFLOW_RunOperation(&pobTran, _OPERATION_DELETE_BATCH_);
                }
		
        }/* End of For loop */
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("EDC UNLOCK &", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
	inDISP_ChineseFont("DELETE BATCH OK", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
	inDISP_PutGraphic(_ERR_OK_, 0, _COORDINATE_Y_LINE_8_7_);
        inDISP_BEEP(1, 0);
                
	while (1)
	{
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);

		if (uszKey == _KEY_ENTER_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_SUCCESS;
			break;
		}
		
	}
	
	return	(inRetVal);
}

/*
Function        :inFunc_Set_TMSOK_Flow
Date&Time       :2017/3/28 下午 2:54
Describe        :
*/
int inFunc_Set_TMSOK_Flow()
{
	char	szKey;
	
	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color("1.設定TMSOK", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("2.設定Table 參數", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
		
		szKey = uszKBD_GetKey(30);
		if (szKey == _KEY_1_)
		{
			inFunc_Set_TMSOK();
		}
		else if (szKey == _KEY_2_)
		{
			inFunc_Edit_Table();
		}
		else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
		{
			break;
		}
		
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Set_TMSOK
Date&Time	:2016/8/16 下午 4:14
Describe        :設定TMSOK
*/
int inFunc_Set_TMSOK(void)
{
	int		inRetVal;
        char		szTemplate[64 + 1];
	char		szTMSOK[1 + 1];
        DISPLAY_OBJECT  srDispObj;
	
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	
	/* Debug */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        memset(szTemplate, 0x00, sizeof(szTemplate));
	
        sprintf(szTemplate, "TMSOK : %s", szTMSOK);
 
        inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	inDISP_ChineseFont("1 = Y , 0 = N", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
        while (1)
        {
                memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
                srDispObj.inMaxLen = 1;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inColor = _COLOR_RED_;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                        return (VS_ERROR);

                if (strlen(srDispObj.szOutput) > 0)
                {
                        if (srDispObj.szOutput[0] == '0')
                        {
                                inSetTMSOK("N");
				inSaveEDCRec(0);
                                break;
                        }
                        else if (srDispObj.szOutput[0] == '1')
                        {
                                inSetTMSOK("Y");
				inSaveEDCRec(0);
                                break;
                        }
                        else
                        {
                                inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
                                continue;
                        }
                }
                else
                {
                        break;
                }
        }
        
	return	(VS_SUCCESS);
}

/*
Function	:inFunc_Edit_Table
Date&Time	:2017/3/28 下午 3:00
Describe        :設定Table
*/
int inFunc_Edit_Table(void)
{
	int	inRetVal;
	
	do
	{
		inRetVal = inCFGT_Edit_CFGT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inEDC_Edit_EDC_Table();
	} while (inRetVal != VS_USER_CANCEL);

	do
	{
		inRetVal = inHDPT_Edit_HDPT_Table();
	} while (inRetVal != VS_USER_CANCEL);	
	
	do
	{
		inRetVal = inCDT_Edit_CDT_Table();
	} while (inRetVal != VS_USER_CANCEL);	
	
	do
	{
		inRetVal = inHDT_Edit_HDT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inCPT_Edit_CPT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inTMSCPT_Edit_TMSCPT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inTMSSCT_Edit_TMSSCT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inTMSFTP_Edit_TMSFTP_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inTDT_Edit_TDT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inIPASSDT_Edit_IPASSDT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inECCDT_Edit_ECCDT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inPWD_Edit_PWD_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inMVT_Edit_MVT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inVWT_Edit_VWT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	do
	{
		inRetVal = inSCDT_Edit_SCDT_Table();
	} while (inRetVal != VS_USER_CANCEL);
	
	
	return	(VS_SUCCESS);
}

/*
Function        :inFunc_Edit_Table_Tag
Date&Time       :2017/3/28 下午 4:29
Describe        :
*/
int inFunc_Edit_Table_Tag(TABLE_GET_SET_TABLE* srTable)
{
	int		i;
	int		inRetVal;
	char		szKey;
	char		szTemplate[22 + 1];
        DISPLAY_OBJECT  srDispObj;
	
	for (i = 0; strlen(srTable[i].szTag) > 0; i++)
	{
		memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
		memset(szTemplate, 0x00, sizeof(szTemplate));
		srDispObj.inY = _LINE_8_8_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 18;
		srDispObj.inColor = _COLOR_BLACK_;
		srDispObj.inTimeout = 30;
		inRetVal = srTable[i].inGetFunctionPoint(szTemplate);
		
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont_Color(srTable[i].szTag, _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("更改按0 跳過按Enter", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
		while (1)
		{
			szKey = uszKBD_GetKey(30);
			if (szKey == _KEY_0_)
			{
				while (1)
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_ChineseFont_Color("確認按Enter放棄按Cancel", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);
					
					srDispObj.inOutputLen = strlen(szTemplate);
					memcpy(srDispObj.szOutput, szTemplate, srDispObj.inOutputLen);
					inDISP_ChineseFont_Color(srDispObj.szOutput, _FONTSIZE_8X16_, _LINE_8_8_, _COLOR_BLACK_, _DISP_RIGHT_);
					
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
					{
						i --;
						if (i < 0)
						{
							i = 0;
						}
						
						break;
					}
					else if (srDispObj.inOutputLen >= 0)
					{
						inRetVal = srTable[i].inSetFunctionPoint(srDispObj.szOutput);
						if (inRetVal != VS_SUCCESS)
						{
							return (inRetVal);
						}
						
						i --;
						break;
					}
					
				}
				
				break;
			}
			else if (szKey == _KEY_CANCEL_ )
			{
				inRetVal = VS_USER_CANCEL;

				return	(inRetVal);
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;

				return	(inRetVal);
			}
			else if (szKey == _KEY_ENTER_)
			{
				break;
			}
			else if (szKey == _KEY_CLEAR_)
			{
				if ((i - 2 + 1) >= 0)
				{
					i -= 2;
				}
				else
				{
					i = 0;
					do
					{
						i++;
					}
					while (strlen(srTable[i + 2].szTag) > 0);
				}
				break;
			}

		}
		
		/* 到Table底端時，循環從第一個繼續 */
		if (strlen(srTable[i + 1].szTag) == 0)
		{
			i = -1;
		}
	}
	
	return	(VS_SUCCESS);
}

/*
Function	:inFunc_Reboot
Date&Time	:2016/6/3 下午 2:42
Describe        :重新開機
*/
int inFunc_Reboot()
{
	CTOS_SystemReset();
	
	return	(VS_SUCCESS);
}

/*
Function	:inFunc_Exit_AP
Date&Time	:2017/9/29 上午 9:30
Describe        :離開程式
*/
int inFunc_Exit_AP()
{
	inDISP_ChineseFont("請稍候...", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
					
	exit(0);
	
	return	(VS_SUCCESS);
}

/*
Function        :inFunc_GetSeriaNumber
Date&Time       :2016/10/11 下午 6:28
Describe        :szSerialNumber sizr一定要大於16
*/
int inFunc_GetSeriaNumber(char* szSerialNumber)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszTemplate[16 + 1] = {0};
	
	memset(uszTemplate, 0x00, sizeof(uszTemplate));
	/* The last digit is the check code of factory serial number. The check code is calculated by exclusive-OR (XOR) the first 15 digits. */
	inRetVal = CTOS_GetFactorySN(uszTemplate);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inFunc_GetSeriaNumber() ERROR!0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	memset(szSerialNumber, 0x00, 16);
	memcpy(szSerialNumber, uszTemplate, 16);
	
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetSystemInfo
Date&Time       :2018/1/11 下午 3:37
Describe        :Note Minimum output buffer size is 17 bytes.
*/
int inFunc_GetSystemInfo(unsigned char uszID, unsigned char *uszBuffer)
{
	int		inRetVal = 0;
	char		szDebugMsg[100 + 1];
	
	memset(uszBuffer, 0x00, 17);
	inRetVal = CTOS_GetSystemInfo(uszID, uszBuffer);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inFunc_GetSystemInfo(%d) ERROR!0x%04X", uszID, inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetOSVersion
Date&Time       :2018/1/11 下午 3:37
Describe        :
*/
int inFunc_GetOSVersion(unsigned char *uszBuffer)
{
	int	inRetVal = 0;
	char	szDebugMsg[100 + 1];
	
	inRetVal = inFunc_GetSystemInfo(ID_ROOTFS, uszBuffer);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inFunc_GetOSVersion() ERROR!");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_Debug_Switch
Date&Time       :2016/12/6 上午 10:37
Describe        :
*/
int inFunc_Sync_Debug_Switch(void)
{
	char	szISODebug[100 + 1];
	
	memset(szISODebug, 0x00, sizeof(szISODebug));
	inGetISODebug(szISODebug);
	if (memcmp(&szISODebug[0], "1", 1) == 0)
	{
		ginDebug = VS_TRUE;
	}
	else
	{
		ginDebug = VS_FALSE;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_ISODebug_Switch
Date&Time       :2017/3/28 下午 2:49
Describe        :
*/
int inFunc_Sync_ISODebug_Switch(void)
{
	char		szISODebug[100 + 1];
	unsigned char	uszAutoBit = VS_FALSE;
	unsigned char	uszLEDOnBit = VS_FALSE;
	unsigned long	ulAutoOffTime = 0;
	
	memset(szISODebug, 0x00, sizeof(szISODebug));
	inGetISODebug(szISODebug);
	
	if (memcmp(&szISODebug[1], "1", 1) == 0)
	{
		ginISODebug = VS_TRUE;
	}
	else
	{
		ginISODebug = VS_FALSE;
	}
	
	/* 開啟電文debug，會讓悠遊卡印電文時間隔太久導致天線關閉問題，所以開啟時將天線自動關閉時間延長至15秒 */
	if (ginISODebug == VS_TRUE)
	{
		/* 調整CTLS Power Mode 設定，for 悠遊卡開啟電文debug太久會會自動關閉天線的問題 */
		CTOS_CLGetPowerModeParm(&uszAutoBit, &ulAutoOffTime, &uszLEDOnBit);	/* 取出預設值 */
		ulAutoOffTime = _ADJUSTED_CTLS_AUTO_OFF_TIME_;	/* 設定15秒，單位是ms */
		CTOS_CLSetPowerModeParm(uszAutoBit, ulAutoOffTime, uszLEDOnBit);	/* 設定回去 */
	}
	else
	{
		CTOS_CLGetPowerModeParm(&uszAutoBit, &ulAutoOffTime, &uszLEDOnBit);	/* 取出預設值 */
		ulAutoOffTime = _DEFAULT_CTLS_AUTO_OFF_TIME_;	/* 單位是ms */
		CTOS_CLSetPowerModeParm(uszAutoBit, ulAutoOffTime, uszLEDOnBit);	/* 設定回去 */
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_DisplayDebug_Switch
Date&Time       :2017/7/6 上午 10:11
Describe        :
*/
int inFunc_Sync_DisplayDebug_Switch(void)
{
	char	szDisplayDebug[100 + 1];
	
	memset(szDisplayDebug, 0x00, sizeof(szDisplayDebug));
	inGetISODebug(szDisplayDebug);
	
	if (memcmp(&szDisplayDebug[2], "1", 1) == 0)
	{
		ginDisplayDebug = VS_TRUE;
	}
	else
	{
		ginDisplayDebug = VS_FALSE;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_EngineerDebug_Switch
Date&Time       :2018/6/14 下午 6:36
Describe        :
*/
int inFunc_Sync_EngineerDebug_Switch(void)
{
	char	szEngineerDebug[100 + 1];
	
	memset(szEngineerDebug, 0x00, sizeof(szEngineerDebug));
	inGetISODebug(szEngineerDebug);
	
	if (memcmp(&szEngineerDebug[3], "1", 1) == 0)
	{
		ginEngineerDebug = VS_TRUE;
	}
	else
	{
		ginEngineerDebug = VS_FALSE;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Sync_ESCDebug_Switch
Date&Time       :2023/3/16 下午 12:04
Describe        :
*/
int inFunc_Sync_ESCDebug_Switch(void)
{
	char	szESCDebug[100 + 1];
	
	memset(szESCDebug, 0x00, sizeof(szESCDebug));
	inGetISODebug(szESCDebug);
	
	if (memcmp(&szESCDebug[_ISO_DEBUG_INDEX_ESCDEBUG_], "1", 1) == 0)
	{
		ginESCDebug = VS_TRUE;
	}
	else
	{
		ginESCDebug = VS_FALSE;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Is_Portable_Type
Date&Time       :2016/12/20 上午 11:43
Describe        :確認是否為Portable機型
*/
int inFunc_Is_Portable_Type(void)
{
	char		szDebugMsg[100 + 1];
	unsigned char	uszPortable;
	unsigned char	uszPCI;
	unsigned short	usHWSupport;
	unsigned short	usRetVal;
	
	usRetVal = CTOS_HWSettingGet(&uszPortable, &uszPCI, &usHWSupport);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_HWSettingGet Err: 0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	if (uszPortable == d_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "This is portable type");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_TRUE);
	}
	else
	{
		return (VS_FALSE);
	}
	
}

/*
Function        :inFunc_Is_Cradle_Attached
Date&Time       :2016/12/20 上午 11:43
Describe        :確認是否Portable機型有接在底座上
*/
int inFunc_Is_Cradle_Attached(void)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_CradleAttached();
	if (usRetVal == d_YES)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_Cradle is Attached");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_Cradle is not Attached");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
}

/*
Function	:inFunc_Find_Specific_HDTindex
Date&Time	:2016/9/1 上午 11:10
Describe        :傳進特定HostName，找這個Host在HDT.dat中的index，如果沒找到，則inHostIndex回傳-1，不用切Host，也不抓HostEnable時用
*/
int inFunc_Find_Specific_HDTindex(int inOrgIndex, char *szHostName, int *inHostIndex)
{
	int	i = 0;
	char	szTemplate[42 + 1];
	char	szDebugMsg[100 + 1];
	
	do
        {
                /* 按順序load每一個Host */
                if (inLoadHDTRec(i) < 0)
                {
			/* 沒找到index，設為-1 */
			*inHostIndex = -1;
			
			/* 代表有可回復的Host(保險機制) */
			if (inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				inLoadHDTRec(inOrgIndex);
			}
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inFunc_Find_Specific_HDTindex Fail :%s", szHostName);
				inLogPrintf(AT, szDebugMsg);
			}

			/* 如果return VS_ERROR 代表table沒有該Host */
                        return (VS_ERROR);
                }
		
		/* 理論上只用8byte位置 */
		memset(szTemplate, 0x00, 8);
		inGetHostLabel(szTemplate);
		if (!memcmp(szTemplate, szHostName, strlen(szHostName)))
		{
			/* 找到的index */
			*inHostIndex = i;
                        if (ginDebug == VS_TRUE)
                        {
                            inLogPrintf(AT, "%s found,Index:%d",szHostName,*inHostIndex);
                        }
			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				inLoadHDTRec(inOrgIndex);
			}
			return (VS_SUCCESS);
			
		}
                i++;
		
        } while (1);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Find_Specific_HDTindex
Date&Time	:2016/9/1 上午 11:10
Describe        :傳進特定HDTIndex，找這個Index在HDT.dat中的真正的index(因為DCC所以會調動順序)，如果沒找到，則inHostIndex回傳-1，不用切Host，也不抓HostEnable時用
*/
int inFunc_Find_Specific_HDTindex_ByCDTIndex(int inOrgIndex, char *szHDTIndex, int *inHostIndex)
{
	int	i = 0;
	char	szTemplate[42 + 1];
	
	do
        {
                /* 按順序load每一個Host */
                if (inLoadHDTRec(i) < 0)
                {
			/* 沒找到index，設為-1 */
			*inHostIndex = -1;
			
			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				inLoadHDTRec(inOrgIndex);
			}

			/* 如果return VS_ERROR 代表table沒有該Host */
                        return (VS_ERROR);
                }
		
		/* 理論上只用2byte位置 */
		memset(szTemplate, 0x00, 2);
		inGetHostIndex(szTemplate);
		if (!memcmp(szTemplate, szHDTIndex, strlen(szHDTIndex)))
		{
			/* 找到的index */
			*inHostIndex = i;
			
			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				inLoadHDTRec(inOrgIndex);
			}
			return (VS_SUCCESS);
			
		}
                i++;
		
        } while (1);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Find_Specific_HDPTindex_Flow
Date&Time	:2019/5/22 下午 2:39
Describe        :傳進特定HostName，找這個Host在HDT.dat中的index，如果沒找到，則inHostIndex回傳-1，不用切Host，也不抓HostEnable時用
*/
int inFunc_Find_Specific_HDPTindex_Flow(int inOrgIndex, char *szTRTName, int *inTRTIndex)
{
	int inRetVal = VS_ERROR;
	
	if (_PARAMETER_SAVE_WAY_DAT_ == _PARAMETER_SAVE_WAY_DAT_SQLITE_)
	{
		inRetVal = inFunc_Find_Specific_HDPTindex_SQLite(inOrgIndex, szTRTName, inTRTIndex);
	}
	else
	{
		inRetVal = inFunc_Find_Specific_HDPTindex(inOrgIndex, szTRTName, inTRTIndex);
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Find_Specific_HDPTindex_SQLite
Date&Time	:2019/5/22 下午 2:46
Describe        :傳進特定HostName，找這個Host在HDT.dat中的index，如果沒找到，則inHostIndex回傳-1，不用切Host，也不抓HostEnable時用
*/
int inFunc_Find_Specific_HDPTindex_SQLite(int inOrgIndex, char *szTRTName, int *inTRTIndex)
{
	int			inRetVal = VS_SUCCESS;
	int			inTemp = 0;
	char			szQuerySql[200 + 1] = {0};
	SQLITE_ALL_TABLE	srAll;

	memset(&srAll, 0x00, sizeof(srAll));
	inRetVal = inHDPT_Table_Link_HDPTRec(&srAll, _LS_READ_);
	
	memset(szQuerySql, 0x00, sizeof(szQuerySql));
	sprintf(szQuerySql, "SELECT inTableID FROM %s WHERE szTRTFileName = '%s' ORDER BY inTableID ASC LIMIT 1", _HDPT_TABLE_NAME_, szTRTName);
	
	inRetVal = inSqlite_Get_Data_By_External_SQL(gszParamDBPath, &srAll, szQuerySql);
	
	if (inRetVal == VS_SUCCESS)
	{
		/* int第一個固定放intable ID */
		inTemp = *(int32_t*)(srAll.srInt[0].pTagValue);
		*inTRTIndex = inTemp - 1;
	}
	else
	{
		*inTRTIndex = -1;
	}
	
	/* 回覆原本的Host */
	inLoadHDPTRec(inOrgIndex);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Find_Specific_HDPTindex
Date&Time	:2016/9/1 上午 11:10
Describe        :傳進特定HostName，找這個Host在HDT.dat中的index，如果沒找到，則inHostIndex回傳-1，不用切Host，也不抓HostEnable時用
*/
int inFunc_Find_Specific_HDPTindex(int inOrgIndex, char *szTRTName, int *inTRTIndex)
{
	int	i = 0;
	char	szTemplate[42 + 1];
	
	do
        {
                /* 按順序load每一個Host */
                if (inLoadHDPTRec(i) < 0)
                {
			/* 沒找到index，設為-1 */
			*inTRTIndex = -1;
			
			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				inLoadHDPTRec(inOrgIndex);
			}

			/* 如果return VS_ERROR 代表table沒有該Host */
                        return (VS_ERROR);
                }
		
		/* 理論上只用12byte位置 */
		memset(szTemplate, 0x00, 12);
		inGetTRTFileName(szTemplate);
		if (!memcmp(szTemplate, szTRTName, strlen(szTRTName)))
		{
			/* 找到的index */
			*inTRTIndex = i;
			
			/* 代表有可回復的Host(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的Host */
				inLoadHDPTRec(inOrgIndex);
			}
			return (VS_SUCCESS);
			
		}
                i++;
		
        } while (1);
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_Find_Specific_CDTindex
Date&Time	:2016/11/25 下午 1:08
Describe        :傳進特定卡別，找這個卡別在CDT.dat中的index，如果沒找到，則inCDTIndex回傳-1
*/
int inFunc_Find_Specific_CDTindex(int inOrgIndex, char *szCardLabel, int *inCDTIndex)
{
	int	i = 0;
	char	szTemplate[42 + 1];
	
	do
        {
                /* 按順序load每一個CDTRec */
                if (inLoadCDTRec(i) < 0)
                {
			/* 沒找到index，設為-1 */
			*inCDTIndex = -1;
			
			/* 代表有可回復的CDT(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的CDT */
				inLoadCDTRec(inOrgIndex);
			}

			/* 如果return VS_ERROR 代表table沒有該Host */
                        return (VS_ERROR);
                }
		
		/* 理論上只用8byte位置 */
		memset(szTemplate, 0x00, 20);
		inGetCardLabel(szTemplate);
		if (!memcmp(szTemplate, szCardLabel, strlen(szCardLabel)))
		{
			/* 找到的index */
			*inCDTIndex = i;
			
			/* 代表有可回復的CDT(保險機制) */
			if(inOrgIndex >= 0)
			{
				/* 回覆原本的CDT */
				inLoadCDTRec(inOrgIndex);
			}
			return (VS_SUCCESS);
			
		}
                i++;
		
        } while (1);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Get_HDPT_General_Data
Date&Time       :2017/2/6 下午 1:08
Describe        :Load到Host必Load的Batch Number, STAN Number, Invoice Number
*/
int inFunc_Get_HDPT_General_Data(TRANSACTION_OBJECT * pobTran)
{
	int	inRetVal;
	char	szBatchNum [6 + 1], szInvoiceNum [6 + 1], szSTANNum [6 + 1];
	
	inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
        memset(szBatchNum, 0x00, sizeof(szBatchNum));
        inRetVal = inGetBatchNum(szBatchNum);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
        pobTran->srBRec.lnBatchNum = atol(szBatchNum);

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        inGetSTANNum(szSTANNum);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
        pobTran->srBRec.lnSTANNum = atol(szSTANNum);

        memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
        inGetInvoiceNum(szInvoiceNum);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
        pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_Linkage_Function_Enable
Date&Time       :2016/11/25 下午 1:28
Describe        :確認該TMS的連動開關
 *		 確認該卡別是否有在CDT並連動到EDC開關
*/
int inFunc_Check_Linkage_Function_Enable(TRANSACTION_OBJECT * pobTran)
{
	int	inCDTIndex = -1;
	char	szESCReciptUploadUpLimit[4 + 1];
	char	szDebugMsg[100 + 1];
	
	/* 是否接收CUP卡 */
	if (inFunc_Find_Specific_CDTindex(pobTran->srBRec.inCDTIndex, _CARD_TYPE_CUP_, &inCDTIndex) == VS_SUCCESS)
	{
		if (inSetCUPFuncEnable("Y") != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		
	}
	else
	{
		if (inSetCUPFuncEnable("N") != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		
	}
	
	/* 是否接受金融卡 */
	if (inFunc_Find_Specific_CDTindex(pobTran->srBRec.inCDTIndex, _CARD_TYPE_SMARTPAY_, &inCDTIndex) == VS_SUCCESS)
	{
		if (inSetFiscFuncEnable("Y") != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		
	}
	else
	{
		if (inSetFiscFuncEnable("N") != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		
	}
	
	inLoadCFGTRec(0);
	memset(szESCReciptUploadUpLimit, 0x00, sizeof(szESCReciptUploadUpLimit));
	inGetESCReciptUploadUpLimit(szESCReciptUploadUpLimit);

	/* 若水位大於0，則開ESC，不觀察Host開關 */
	if (atoi(szESCReciptUploadUpLimit) > 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "ESC Mode : Y");
			inLogPrintf(AT, szDebugMsg);
		}

		if (inSetESCMode("Y") != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "水位不大於0，不送NE");
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (inSetESCMode("N") != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
		if (inSaveEDCRec(0) != VS_SUCCESS)
		{
			inFunc_EDCLock(AT);
		}
	}
	
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckCustomizePassword
Date&Time       :
Describe        :
*/
int inFunc_CheckCustomizePassword(int inPasswordLevel, int inCode)
{
	char		szTerminalPwd[16 + 1] = {0};
        char		szPWDEnable[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szDury_Free_Reprint_Pwd[4 + 1] = {0};
	char		szDury_Free_Detail_Pwd[4 + 1] = {0};
	int		inTableID = 0;
	int		inRetVal = 0;
        DISPLAY_OBJECT  srDispObj;
	SQLITE_ALL_TABLE	srAll;

	if (inPasswordLevel == _ACCESS_FREELY_)
		return (VS_SUCCESS);

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTerminalPwd, 0x00, sizeof(szTerminalPwd));
        memset(szPWDEnable, 0x00, sizeof(szPWDEnable));

	if (inPasswordLevel == _ACCESS_WITH_CUSTOM_)
	{
                if (inLoadPWDRec(0) < 0)
                        return (VS_SUCCESS);

		switch (inCode)
		{
			case _SALE_ :
                        case _CUP_SALE_:
                        case _FISC_SALE_:
			case _HG_REWARD_:
			case _HG_ONLINE_REDEEM_:
			case _HG_POINT_CERTAIN_:
			case _CASH_ADVANCE_:
                        case _TRUST_SALE_:
                                inGetSalePwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetSalePwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
                        case _INST_SALE_ :
			case _CUP_INST_SALE_ :
                                inGetInstallmentPwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetInstallmentPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
                        case _REDEEM_SALE_ :
			case _CUP_REDEEM_SALE_ :
                                inGetRedeemPwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetRedeemPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
                        case _PRE_AUTH_ :
                        case _CUP_PRE_AUTH_ :
			case _PRE_COMP_ :
			case _CUP_PRE_COMP_ :
                                inGetPreauthPwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetPreauthPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
			case _VOID_ :
			case _CUP_VOID_ :
			case _CUP_PRE_AUTH_VOID_ :
			case _CUP_PRE_COMP_VOID_ :
			case _FISC_VOID_ :
                        case _TRUST_VOID_:
                                inGetVoidPwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetVoidPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
			case _REFUND_ :
                        case _INST_REFUND_ :
                        case _REDEEM_REFUND_ :
			case _CUP_REFUND_ :
			case _FISC_REFUND_:
			case _CUP_MAIL_ORDER_REFUND_:
                                inGetRefundPwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetRefundPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
			case _SETTLE_ :
                                inGetSettlementPwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetSettlementPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
			case _SALE_OFFLINE_ :
			case _FORCE_CASH_ADVANCE_:
                                inGetOfflinePwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetOfflinePwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
			case _INST_ADJUST_ :
                                inGetInstallmentAdjustPwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetInstallmentAdjustPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
                        case _REDEEM_ADJUST_ :
                                inGetRedeemAdjustPwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetRedeemAdjustPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
                        case _POWER_ON_ :
                                inGetRebootPwdEnale(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
					inDISP_ClearAll();
					inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                                        inDISP_PutGraphic(_MENU_POWER_ON_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 開機 */
                                        inGetRebootPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
			case _MAIL_ORDER_:
			case _CUP_MAIL_ORDER_:
				inGetMailOrderPwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetMailOrderPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
			case _CALL_BANK_:
				inGetCallBankForcePwdEnable(szPWDEnable);
                                
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetCallBankForcePwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
				break;
			case _TIP_:
				return (VS_SUCCESS);
				break;
			case _LOYALTY_REDEEM_:
				return (VS_SUCCESS);
				break;
			case _VOID_LOYALTY_REDEEM_:
				return (VS_SUCCESS);
				break;
			case _LOYALTY_REDEEM_REFUND_:
				return (VS_SUCCESS);
				break;
			case _HG_REWARD_REFUND_:
			case _HG_REDEEM_REFUND_:
				inGetHGRefundPwdEnable(szPWDEnable);
				
                                if (!memcmp(szPWDEnable, "Y", 1))
                                {
                                        inGetHGRefundPwd(szTerminalPwd);
                                        break;
                                }
                                else
                                {
                                        return (VS_SUCCESS);
                                }
				break;
			default :
				vdUtility_SYSFIN_LogMessage(AT, "Not Set this PW Incode : %d", inCode);
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "Not Set this PW Incode : %d", inCode);
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
				break;
		}
                
                
	}
	else if (inPasswordLevel == _ACCESS_WITH_MANAGER_PASSWORD_ ||
		 inPasswordLevel == _ACCESS_WITH_FUNC_PASSWORD_ ||
		 inPasswordLevel == _ACCESS_WITH_MERCHANT_PASSWORD_ ||
		 inPasswordLevel == _ACCESS_WITH_SUPER_PASSWORD_)
	{
		if (inPasswordLevel == _ACCESS_WITH_MANAGER_PASSWORD_)
		{
			inGetManagerPassword(szTerminalPwd);
		}
		else if (inPasswordLevel == _ACCESS_WITH_MERCHANT_PASSWORD_)
		{
			inGetMerchantPassword(szTerminalPwd);
		}
		else if (inPasswordLevel == _ACCESS_WITH_FUNC_PASSWORD_)
		{
			inGetFunctionPassword(szTerminalPwd);
		}
		else if (inPasswordLevel == _ACCESS_WITH_SUPER_PASSWORD_)
		{
			inGetSuperPassword(szTerminalPwd);
		}
	}
        else if (inPasswordLevel == _ACCESS_WITH_EDC_UNLOACK_PASSWORD_)
	{
		strcpy(szTerminalPwd, "332865625");
	}
	else if (inPasswordLevel == _ACCESS_WITH_FUNC2_PASSWORD_)
	{
		strcpy(szTerminalPwd, "73989707");
	}
	else if (inPasswordLevel == _ACCESS_WITH_DUTY_FREE_REPRINT_PASSWORD_)
	{
		memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
		memset(szDury_Free_Reprint_Pwd, 0x00, sizeof(szDury_Free_Reprint_Pwd));
		memset(szDury_Free_Detail_Pwd, 0x00, sizeof(szDury_Free_Detail_Pwd));
		inTableID = 0;
		inNCCC_Func_Table_Link_Duty_Free_Pwd(&srAll, _LS_READ_, &inTableID, szDury_Free_Reprint_Pwd, szDury_Free_Detail_Pwd);
		inSqlite_Get_Table_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_DUTY_FREE_PWD_, 0, &srAll);
		if (strlen(szDury_Free_Reprint_Pwd) > 0)
		{
			strcpy(szTerminalPwd, szDury_Free_Reprint_Pwd);
		}
		else
		{
			strcpy(szTerminalPwd, _DUTY_FREE_REPRINT_PWD_DEFAULT_);
		}
	}
	else if (inPasswordLevel == _ACCESS_WITH_DUTY_FREE_DETAIL_PASSWORD_)
	{
		memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
		memset(szDury_Free_Reprint_Pwd, 0x00, sizeof(szDury_Free_Reprint_Pwd));
		memset(szDury_Free_Detail_Pwd, 0x00, sizeof(szDury_Free_Detail_Pwd));
		inTableID = 0;
		inNCCC_Func_Table_Link_Duty_Free_Pwd(&srAll, _LS_READ_, &inTableID, szDury_Free_Reprint_Pwd, szDury_Free_Detail_Pwd);
		inSqlite_Get_Table_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_DUTY_FREE_PWD_, 0, &srAll);
		if (strlen(szDury_Free_Reprint_Pwd) > 0)
		{
			strcpy(szTerminalPwd, szDury_Free_Detail_Pwd);
		}
		else
		{
			strcpy(szTerminalPwd, _DUTY_FREE_DETAIL_PWD_DEFAULT_);
		}
	}
	else
        {
		vdUtility_SYSFIN_LogMessage(AT, "No such PasswordLevel.");
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "No such PasswordLevel.");
		}

                return (VS_ERROR);
        }

	if (strlen(szTerminalPwd) <= 0) /* 表示沒有密碼 */
		return (VS_SUCCESS);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (inPasswordLevel == _ACCESS_WITH_CUSTOM_)
                inDISP_PutGraphic(_GET_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_); /* 請輸入管理號碼 */
	else if (inPasswordLevel == _ACCESS_WITH_DUTY_FREE_REPRINT_PASSWORD_	||
		 inPasswordLevel == _ACCESS_WITH_DUTY_FREE_DETAIL_PASSWORD_)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("請輸入", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("管理碼後按確認鍵", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);
	}
	else
        {
                inDISP_PutGraphic(_GET_SYSTEM_PWD_, 0, _COORDINATE_Y_LINE_8_4_); /* 輸入系統管理號碼 */
        }

	while (1)
	{
                inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

                /* 設定顯示變數 */
                srDispObj.inMaxLen = 16;
                srDispObj.inY = _LINE_8_7_;
                srDispObj.inR_L = _DISP_RIGHT_;
                srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTimeout = 30;

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckCustomizePassword Timeout_Or_UserCancel(%d) END!", inRetVal);
			return (VS_ERROR);
		}

		if (srDispObj.inOutputLen == strlen(szTerminalPwd))
		{
			if (!memcmp(szTerminalPwd, srDispObj.szOutput, strlen(szTerminalPwd)))
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				break;
			}
                        else 
			{
                            inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			}
		}
                else 
		{
                        inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		}
	}

	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetHostNum
Date&Time       :2015/10/06 上午 10:00
Describe        :Host選單，如果只有一個主機就直接LoadHDT和HDPT，選定HostNum，並load HDT和HDPT
*/
int inFunc_GetHostNum(TRANSACTION_OBJECT *pobTran)
{
	int		inOpenHostCnt = 0;      /* 記錄有幾個Host有開 */
	int		inLine = 0;             /* 第幾行 */
	int		i, j = 0;               /* j是inHostIndex陣列索引 */
	int		inHostIndex[12 + 1];    /* 記錄HostEnable為Y的HostIndex */
	int		inLine1Index = 0;       /* szLine1的index */
	int		inLine2Index = 0;       /* szLine2的index */
	int		inLine3Index = 0;       /* szLine3的index */
	int		inLine4Index = 0;
	int		inKey = 0;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_12X19_OPT_;
	int		inRetVal = VS_SUCCESS;
	char		szKey;
	char		szHostEnable[2 + 1];
	char		szHostName[42 + 1];
	char		szTemp[48 + 1];
	char		szLine1[48 + 1];        /* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
	char		szLine2[48 + 1];        /* 存第二行要顯示的Host */
	char		szLine3[48 + 1];        /* 存第三行要顯示的Host */
	char		szLine4[48 + 1];
	char		szBatchNum[6 + 1];
	char		szTimeout[4 + 1];
	char		szDebugMsg[42 + 1];
	DISPLAY_OBJECT  srDispObj;
	
	/* 如果要連動結帳，跳過選Host流程 */
	if (pobTran->uszAutoSettleBit == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	/* 銀聯選NCCC */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		pobTran->srBRec.inHDTIndex = 0;
		
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		 
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}
	
	/* SmartPay選NCCC */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		pobTran->srBRec.inHDTIndex = 0;
		
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		 
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}
		
	
	/* 如果已經有HostNum，跳過選Host流程(ECR發動) */
	if (pobTran->srBRec.inHDTIndex != -1)
	{
		return (VS_SUCCESS);
	}
	
	/* 以上是特例，如果都沒有就手動選Host */

        memset(szLine1, 0x00, sizeof(szLine1));
        memset(szLine2, 0x00, sizeof(szLine2));
        memset(szLine3, 0x00, sizeof(szLine3));
        memset(szLine4, 0x00, sizeof(szLine4));
        memset(szTimeout, 0x00, sizeof(szTimeout));
        memset(inHostIndex, 0x00, sizeof(inHostIndex));
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	
        for (i = 0;; ++i)
        {
                /* 先LoadHDT */
                if (inLoadHDTRec(i) == VS_ERROR)
                {
                        /* 當找不到第i筆資料會回傳VS_ERROR */
                        break;
                }
		
		/* ESC不該出現在選單上 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}
		
		/* HG不該出現在選單上(結帳、小費、交易查詢) */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
		{
			if (pobTran->inRunOperationID == _OPERATION_SETTLE_		||
			    pobTran->inRunOperationID == _OPERATION_TIP_		||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_		||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_TOTAL_	||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_DETAIL_	||
			    pobTran->inRunOperationID == _OPERATION_DELETE_BATCH_)
			{
				continue;
			}
			
		}

                /* GET HOST Enable */
                memset(szHostEnable, 0x00, sizeof(szHostEnable));
                if (inGetHostEnable(szHostEnable) == VS_ERROR)
                {	
                        return (VS_ERROR);
                }
                else
                {
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%d HostEnable: %s", i, szHostEnable);
				inLogPrintf(AT, szDebugMsg);
			}
			
                        if (memcmp(szHostEnable, "Y", 1) != 0)
                        {
                                /* 如果HostEnable != Y，就continue */
                                continue;
                        }

                        /* 如果主機有開，才loadHDPT */
                        if (inLoadHDPTRec(i) == VS_ERROR)
                        {
                                /* 當找不到第i筆資料會回傳VS_ERROR */
                                return (VS_ERROR);
                        }

                        inOpenHostCnt ++;       /* 記錄有幾個Host有開 */
                        inLine ++;              /* 第幾行 */

                        /* 記錄HostEnable為Y的HostIndex，減1是因為HostIndex從01開始 */
                        inHostIndex[j] = i;
                        j++;
			
			memset(szHostName, 0x00, sizeof(szHostName));
			inGetHostLabel(szHostName);
			
                        /* 用szTRTFileName來決定要顯示的Host Name */
                        memset(szTemp, 0x00, sizeof(szTemp));
                        sprintf(szTemp, "%d. %s ", inOpenHostCnt, szHostName);

                        /* 每一行顯示的內容先存在陣列裡 */
                        switch (inLine)
                        {
                                case 1:
                                        memcpy(&szLine1[inLine1Index], szTemp, strlen(szTemp));
                                        inLine1Index += strlen(szTemp);
                                        break;
                                case 2:
                                        memcpy(&szLine2[inLine2Index], szTemp, strlen(szTemp));
                                        inLine2Index += strlen(szTemp);
                                        break;
                                case 3:
                                        memcpy(&szLine3[inLine3Index], szTemp, strlen(szTemp));
                                        inLine3Index += strlen(szTemp);
                                        break;
                                case 4:
                                        memcpy(&szLine4[inLine4Index], szTemp, strlen(szTemp));
                                        inLine4Index += strlen(szTemp);
                                        break;
                                default:
                                        break;
                        }
			
                }
		
        }/* End of For loop */

        /* 當inOpenHostCnt = 0，表示主機都沒開或者inLoadHDT有問題 */
        if (inOpenHostCnt == 0)
        {
                /* 主機選擇錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

                return (VS_ERROR);
        }

        if (inOpenHostCnt == 1)
        {
                /* 只有開一個Host */
                if (inLoadHDTRec(inHostIndex[0]) == VS_ERROR)
                {
                        /* 主機選擇錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

                        return (VS_ERROR);
                }

                if (inLoadHDPTRec(inHostIndex[0]) == VS_ERROR)
                {
                        /* 主機選擇錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

                        return (VS_ERROR);
                }
                else
                {
                        pobTran->srBRec.inHDTIndex = inHostIndex[0];
                        memset(szBatchNum, 0x00, sizeof(szBatchNum));
                        inGetBatchNum(szBatchNum);
                        pobTran->srBRec.lnBatchNum = atol(szBatchNum);
                }

                return (VS_SUCCESS);
        }
        else
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                /* 顯示請選擇中心? */
                inDISP_PutGraphic(_CHOCE_HOST_, 0, _COORDINATE_Y_LINE_8_4_);
            
                /*有開多個Host */
                inDISP_ChineseFont(szLine1, _FONTSIZE_12X19_, _LINE_12_6_, _DISP_LEFT_);
                inDISP_ChineseFont(szLine2, _FONTSIZE_12X19_, _LINE_12_7_, _DISP_LEFT_);
                inDISP_ChineseFont(szLine3, _FONTSIZE_12X19_, _LINE_12_8_, _DISP_LEFT_);
                inDISP_ChineseFont(szLine4, _FONTSIZE_12X19_, _LINE_12_9_, _DISP_LEFT_);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, szLine1);
			inLogPrintf(AT, szLine2);
			inLogPrintf(AT, szLine3);
			inLogPrintf(AT, szLine4);
		}
        }
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
        while (1)
        {
		inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();
		
		/* 轉成數字判斷是否在inOpenHostCnt的範圍內 */
		inKey = 0;
		/* 有觸摸*/
		if (inChoice != _DisTouch_No_Event_)
		{
			switch (inChoice)
			{
				case _OPTTouch12X19_LINE_6_:
					inKey = 1;
					inDisTouch_Reverse_Back_Key(inTouchSensorFunc, inChoice);
					break;
				case _OPTTouch12X19_LINE_7_:
					inKey = 2;
					inDisTouch_Reverse_Back_Key(inTouchSensorFunc, inChoice);
					break;
				case _OPTTouch12X19_LINE_8_:
					inKey = 3;
					inDisTouch_Reverse_Back_Key(inTouchSensorFunc, inChoice);
					break;
				case _OPTTouch12X19_LINE_9_:
					inKey = 4;
					inDisTouch_Reverse_Back_Key(inTouchSensorFunc, inChoice);
					break;
				default:
					inKey = 0;
					break;
			}
		}
		/* 有按按鍵 */
		else if (szKey != 0)
		{
			switch (szKey)
			{
				case _KEY_1_:
					inKey = 1;
					break;
				case _KEY_2_:
					inKey = 2;
					break;
				case _KEY_3_:
					inKey = 3;
					break;
				case _KEY_4_:
					inKey = 4;
					break;
				default:
					inKey = 0;
					break;
			}
		}
		
		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}

                if (szKey == _KEY_CANCEL_)
                {
                        inRetVal = VS_USER_CANCEL;
			break;
                }
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (inKey >= 1 && inKey <= inOpenHostCnt)
                {
                        if (inLoadHDTRec(inHostIndex[inKey - 1]) == VS_ERROR)
                        {
                                /* 主機選擇錯誤 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
                                inDISP_Msg_BMP(&srDispMsgObj);

				inRetVal = VS_ERROR;
				break;
                        }

                        if (inLoadHDPTRec(inHostIndex[inKey - 1]) == VS_ERROR)
                        {
                                /* 主機選擇錯誤 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
                                inDISP_Msg_BMP(&srDispMsgObj);

				inRetVal = VS_ERROR;
				break;
                        }
                        else
                        {
				if (ginDebug == VS_TRUE)
				{
					memset(szTemp, 0x00, sizeof(szTemp));
					sprintf(szTemp, "%d", inHostIndex[inKey - 1]);
					inLogPrintf(AT, szTemp);
				}
                                pobTran->srBRec.inHDTIndex = inHostIndex[inKey - 1];
                                memset(szBatchNum, 0x00, sizeof(szBatchNum));
                                inGetBatchNum(szBatchNum);
                                pobTran->srBRec.lnBatchNum = atol(szBatchNum);
				
				inRetVal = VS_SUCCESS;
                                break;
                        }
                }
        }
	
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();

	return (inRetVal);
}

/*
Function        :inFunc_GetHostNum_NewUI
Date&Time       :2017/10/31 下午 5:09
Describe        :Host選單，如果只有一個主機就直接LoadHDT和HDPT，選定HostNum，並load HDT和HDPT
*/
int inFunc_GetHostNum_NewUI(TRANSACTION_OBJECT *pobTran)
{
	int		inOpenHostCnt = 0;      /* 記錄有幾個Host有開 */
	int		i, j = 0;               /* j是inHostIndex陣列索引 */
	int		inHostIndex[12 + 1];    /* 記錄HostEnable為Y的HostIndex */
	int		inKey = 0;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_CHOOSE_HOST_;
	int		inRetVal = VS_SUCCESS;
	int		inRetVal2 = VS_ERROR;	/* inFunc_Load_Last_Txn_Host使用 */
	int		inLastHDTIndex = -1;
        int             inFindHDTIndex = -1;
	char		szKey = 0;
	char		szHostEnable[2 + 1] = {0};
	char		szHostName[42 + 1] = {0};
	char		szTemp[48 + 1] = {0};
	char		szLine1[48 + 1] = {0};		/* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
	char		szLine2[48 + 1] = {0};		/* 存第二行要顯示的Host */
	char		szLine3[48 + 1] = {0};		/* 存第三行要顯示的Host */
	char		szLine4[48 + 1] = {0};
	char		szLine5[48 + 1] = {0};
	char		szLine6[48 + 1] = {0};
	char		szTemp2[48 + 1] = {0};
	char		szLine1_2[48 + 1] = {0};	/* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
	char		szLine2_2[48 + 1] = {0};	/* 存第二行要顯示的Host */
	char		szLine3_2[48 + 1] = {0};	/* 存第三行要顯示的Host */
	char		szLine4_2[48 + 1] = {0};
	char		szLine5_2[48 + 1] = {0};
	char		szLine6_2[48 + 1] = {0};
	char		szBatchNum[6 + 1] = {0};
	char		szTimeout[4 + 1] = {0};
	char		szDebugMsg[42 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetHostNum_NewUI START!");
	
	/* 如果要連動結帳，跳過選Host流程 */
	/* ECR列印總額、明細，直接連動 */
	if (pobTran->uszAutoSettleBit == VS_TRUE							||
	   (pobTran->uszECRBit == VS_TRUE && pobTran->inRunOperationID == _OPERATION_TOTAL_REPORT_)	||
	   (pobTran->uszECRBit == VS_TRUE && pobTran->inRunOperationID == _OPERATION_DETAIL_REPORT_))
	{
		guszNoChooseHostBit = VS_TRUE;
		return (VS_SUCCESS);
	}
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 銀聯選NCCC */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		pobTran->srBRec.inHDTIndex = 0;
		
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		 
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}
	
	/* SmartPay選NCCC */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		pobTran->srBRec.inHDTIndex = 0;
		
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		 
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}
        
        /* 信託重印直接選 */
	if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE &&
            pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
                inFindHDTIndex = -1;
                inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_TRUST_, &inFindHDTIndex);
		pobTran->srBRec.inHDTIndex = inFindHDTIndex;
		
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		 
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}
		
	
	/* 如果已經有HostNum，跳過選Host流程(ECR發動) */
	if (pobTran->srBRec.inHDTIndex != -1)
	{
		guszNoChooseHostBit = VS_TRUE;
		return (VS_SUCCESS);
	}
	
	/* 以上是特例，如果都沒有就手動選Host */

        memset(szLine1, 0x00, sizeof(szLine1));
        memset(szLine2, 0x00, sizeof(szLine2));
        memset(szLine3, 0x00, sizeof(szLine3));
        memset(szLine4, 0x00, sizeof(szLine4));
	memset(szLine5, 0x00, sizeof(szLine5));
	memset(szLine6, 0x00, sizeof(szLine6));
	memset(szLine1_2, 0x00, sizeof(szLine1_2));
        memset(szLine2_2, 0x00, sizeof(szLine2_2));
        memset(szLine3_2, 0x00, sizeof(szLine3_2));
        memset(szLine4_2, 0x00, sizeof(szLine4_2));
	memset(szLine5_2, 0x00, sizeof(szLine5_2));
	memset(szLine6_2, 0x00, sizeof(szLine6_2));
        memset(szTimeout, 0x00, sizeof(szTimeout));
        memset(inHostIndex, 0x00, sizeof(inHostIndex));
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	
        for (i = 0;; ++i)
        {
                /* 先LoadHDT */
                if (inLoadHDTRec(i) == VS_ERROR)
                {
                        /* 當找不到第i筆資料會回傳VS_ERROR */
                        break;
                }
		
		/* ESC不該出現在選單上 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}

		/* HG不該出現在選單上(結帳、小費、交易查詢) */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
		{
			if (pobTran->inRunOperationID == _OPERATION_SETTLE_		||
			    pobTran->inRunOperationID == _OPERATION_TIP_		||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_		||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_TOTAL_	||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_DETAIL_	||
			    pobTran->inRunOperationID == _OPERATION_DELETE_BATCH_)
			{
				continue;
			}

		}

		/* ESVC不該出現在一般交易選單上`，目前只有結帳需要，採正向表列 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
		{
			if (pobTran->inRunOperationID != _OPERATION_SETTLE_		&&
			    pobTran->inRunOperationID != _OPERATION_DELETE_BATCH_	&&
			    pobTran->inRunOperationID != _OPERATION_REVIEW_		&&
			    pobTran->inRunOperationID != _OPERATION_REVIEW_TOTAL_	&&
			    pobTran->inRunOperationID != _OPERATION_REVIEW_DETAIL_	&&
			    pobTran->inRunOperationID != _OPERATION_TOTAL_REPORT_	&&
			    pobTran->inRunOperationID != _OPERATION_DETAIL_REPORT_	&&
			    pobTran->inRunOperationID != _OPERATION_REPRINT_)
			{
				continue;
			}
		}
		
		/* EW不該出現在一般交易選單上 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_EW_, strlen(_HOST_NAME_EW_)) == 0)
		{
			if (pobTran->inRunOperationID == _OPERATION_EDIT_HOST_BATCH_NUM_)
			{
				
			}
			else
			{
				continue;
			}
		}
                
                /* TR不該出現在一般交易選單上`，目前只有結帳需要，採正向表列 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_)) == 0)
		{
			if (pobTran->inRunOperationID != _OPERATION_SETTLE_		&&
			    pobTran->inRunOperationID != _OPERATION_DELETE_BATCH_       &&
                            pobTran->inRunOperationID != _OPERATION_EDIT_HOST_BATCH_NUM_)
			{
				continue;
			}
		}
		
                /* GET HOST Enable */
                memset(szHostEnable, 0x00, sizeof(szHostEnable));
                if (inGetHostEnable(szHostEnable) == VS_ERROR)
                {	
                        return (VS_ERROR);
                }
                else
                {
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%d HostEnable: %s", i, szHostEnable);
				inLogPrintf(AT, szDebugMsg);
			}
			
                        if (memcmp(szHostEnable, "Y", 1) != 0)
                        {
                                /* 如果HostEnable != Y，就continue */
                                continue;
                        }

                        /* 如果主機有開，才loadHDPT */
                        if (inLoadHDPTRec(i) == VS_ERROR)
                        {
                                /* 當找不到第i筆資料會回傳VS_ERROR */
                                return (VS_ERROR);
                        }
			
                        inOpenHostCnt ++;       /* 記錄有幾個Host有開 */

                        /* 記錄HostEnable為Y的HostIndex，減1是因為HostIndex從01開始 */
                        inHostIndex[j] = i;
                        j++;
			
			memset(szHostName, 0x00, sizeof(szHostName));
			inGetHostLabel(szHostName);
			
                        /* 用szTRTFileName來決定要顯示的Host Name */
                        memset(szTemp, 0x00, sizeof(szTemp));
			inFunc_DiscardSpace(szHostName);
			/* ESVC要顯示電子票證 */
			if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
			{
				sprintf(szTemp, "%s", "電子票");
				memset(szTemp2, 0x00, sizeof(szTemp2));
				sprintf(szTemp2, "證   %d", inOpenHostCnt);
			}
			else if (memcmp(szHostName, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_)) == 0)
			{ /* [20251215_BUG_MDF][FUNC] 結帳信託按鈕顯示要改成中文 */
				sprintf(szTemp, "%s", "信託");
				memset(szTemp2, 0x00, sizeof(szTemp2));
				sprintf(szTemp2, "     %d", inOpenHostCnt);
			}else
			{
				sprintf(szTemp, "%s", szHostName);
				memset(szTemp2, 0x00, sizeof(szTemp2));
				sprintf(szTemp2, "     %d", inOpenHostCnt);
			}
			
                        /* 每一行顯示的內容先存在陣列裡 */
                        switch (inOpenHostCnt)
                        {
                                case 1:
                                        memcpy(&szLine1[0], szTemp, strlen(szTemp));
					
					memcpy(szLine1_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 2:
                                        memcpy(&szLine2[0], szTemp, strlen(szTemp));
					
					memcpy(szLine2_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 3:
                                        memcpy(&szLine3[0], szTemp, strlen(szTemp));
					
					memcpy(szLine3_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 4:
                                        memcpy(&szLine4[0], szTemp, strlen(szTemp));
					
					memcpy(szLine4_2, szTemp2, strlen(szTemp2));
                                        break;
				case 5:
                                        memcpy(&szLine5[0], szTemp, strlen(szTemp));
					
					memcpy(szLine5_2, szTemp2, strlen(szTemp2));
                                        break;
				case 6:
                                        memcpy(&szLine6[0], szTemp, strlen(szTemp));
					
					memcpy(szLine6_2, szTemp2, strlen(szTemp2));
                                        break;
                                default:
                                        break;
                        }
                }
		
        }/* End of For loop */
	
	/* 【需求單 - 108128】	單機重印前筆簽單流程新增畫面 by Russell 2019/8/15 上午 11:40 */
	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		/* 上筆簽單Host */
		inLastHDTIndex = -1;
		inRetVal2 = inFunc_Load_Last_Txn_Host(&inLastHDTIndex);

		if (inRetVal2 != VS_SUCCESS || 
		    inLastHDTIndex < 0)
		{
			
		}
		else
		{
			inHostIndex[inOpenHostCnt] = inLastHDTIndex;
		}
		
		inOpenHostCnt ++;       /* 記錄有幾個Host有開 */
		
		memset(szTemp, 0x00, sizeof(szTemp));
		sprintf(szTemp, "%s", "上筆");
		memset(szTemp2, 0x00, sizeof(szTemp2));
		sprintf(szTemp2, "簽單 %d", inOpenHostCnt);
		
		/* 每一行顯示的內容先存在陣列裡 */
		switch (inOpenHostCnt)
		{
			case 1:
				memcpy(szLine1, szTemp, strlen(szTemp));

				memcpy(szLine1_2, szTemp2, strlen(szTemp2));
				break;
			case 2:
				memcpy(szLine2, szTemp, strlen(szTemp));

				memcpy(szLine2_2, szTemp2, strlen(szTemp2));
				break;
			case 3:
				memcpy(szLine3, szTemp, strlen(szTemp));

				memcpy(szLine3_2, szTemp2, strlen(szTemp2));
				break;
			case 4:
				memcpy(szLine4, szTemp, strlen(szTemp));

				memcpy(szLine4_2, szTemp2, strlen(szTemp2));
				break;
			case 5:
				memcpy(szLine5, szTemp, strlen(szTemp));

				memcpy(szLine5_2, szTemp2, strlen(szTemp2));
				break;
			case 6:
				memcpy(szLine6, szTemp, strlen(szTemp));

				memcpy(szLine6_2, szTemp2, strlen(szTemp2));
				break;
			default:
				break;
		}
	}
	

        /* 當inOpenHostCnt = 0，表示主機都沒開或者inLoadHDT有問題 */
        if (inOpenHostCnt == 0)
        {
                /* 主機選擇錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

                return (VS_ERROR);
        }

        if (inOpenHostCnt == 1)
        {
                /* 只有開一個Host */
                if (inLoadHDTRec(inHostIndex[0]) == VS_ERROR)
                {
                        /* 主機選擇錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

                        return (VS_ERROR);
                }

                if (inLoadHDPTRec(inHostIndex[0]) == VS_ERROR)
                {
                        /* 主機選擇錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

                        return (VS_ERROR);
                }
                else
                {
                        pobTran->srBRec.inHDTIndex = inHostIndex[0];
                        memset(szBatchNum, 0x00, sizeof(szBatchNum));
                        inGetBatchNum(szBatchNum);
                        pobTran->srBRec.lnBatchNum = atol(szBatchNum);
                }
		
                inRetVal = VS_SUCCESS;
		guszNoChooseHostBit = VS_TRUE;
        }
        else
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
		switch (inOpenHostCnt)
		{
			case	2:
				inDISP_PutGraphic(_CHOOSE_HOST_2_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			case	3:
				inDISP_PutGraphic(_CHOOSE_HOST_3_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			case	4:
				inDISP_PutGraphic(_CHOOSE_HOST_4_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			case	5:
				inDISP_PutGraphic(_CHOOSE_HOST_5_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			case	6:
				inDISP_PutGraphic(_CHOOSE_HOST_6_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			default:
				inDISP_PutGraphic(_CHOOSE_HOST_6_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
		}
		
                /*有開多個Host */
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
		
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
		
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
		
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);
		
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);
		
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, szLine1);
			inLogPrintf(AT, szLine2);
			inLogPrintf(AT, szLine3);
			inLogPrintf(AT, szLine4);
			inLogPrintf(AT, szLine5);
			inLogPrintf(AT, szLine6);
		}
	
		/* (需求單-112222)-IKEA客製化新增功能需求 by Russell 2023/9/8 下午 4:25 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
		}
		else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
		}
		else
		{
			inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		}

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* 轉成數字判斷是否在inOpenHostCnt的範圍內 */
			inKey = 0;
			/* 有觸摸*/
			if (inChoice != _DisTouch_No_Event_)
			{
				switch (inChoice)
				{
					case _NEWUI_CHOOSE_HOST_Touch_HOST_1_:
						inKey = 1;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_2_:
						inKey = 2;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_3_:
						inKey = 3;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_4_:
						inKey = 4;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_5_:
						inKey = 5;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_6_:
						inKey = 6;
						break;
					default:
						inKey = 0;
						break;
				}
			}
			/* 有按按鍵 */
			else if (szKey != 0)
			{
				switch (szKey)
				{
					case _KEY_1_:
						inKey = 1;
						break;
					case _KEY_2_:
						inKey = 2;
						break;
					case _KEY_3_:
						inKey = 3;
						break;
					case _KEY_4_:
						inKey = 4;
						break;
					case _KEY_5_:
						inKey = 5;
						break;
					case _KEY_6_:
						inKey = 6;
						break;
					default:
						inKey = 0;
						break;
				}
			}

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			else if (inKey >= 1 && inKey <= inOpenHostCnt)
			{
				if (pobTran->inRunOperationID == _OPERATION_REPRINT_ &&
				    inKey == inOpenHostCnt)
				{
					/* 上筆簽單Host */
					/* 取得上筆交易Host失敗*/
					if (inRetVal2 != VS_SUCCESS || 
					    inLastHDTIndex < 0)
					{
						 inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						DISPLAY_OBJECT	srDispMsgObj;
						memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
						strcpy(srDispMsgObj.szDispPic1Name, _ERR_RECORD_);
						srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
						srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
						srDispMsgObj.inTimeout = 2;
						strcpy(srDispMsgObj.szErrMsg1, "");
						srDispMsgObj.inErrMsg1Line = 0;
						srDispMsgObj.inBeepTimes = 1;
						srDispMsgObj.inBeepInterval = 0;

						inDISP_Msg_BMP(&srDispMsgObj);
						return (VS_ERROR);
					}
					else
					{
						
					}
					/* 上筆簽單調閱編號 */
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
				}
				
				if (inLoadHDTRec(inHostIndex[inKey - 1]) == VS_ERROR)
				{
					/* 主機選擇錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);

					inRetVal = VS_ERROR;
					break;
				}

				if (inLoadHDPTRec(inHostIndex[inKey - 1]) == VS_ERROR)
				{
					/* 主機選擇錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);

					inRetVal = VS_ERROR;
					break;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szTemp, 0x00, sizeof(szTemp));
						sprintf(szTemp, "%d", inHostIndex[inKey - 1]);
						inLogPrintf(AT, szTemp);
					}
					pobTran->srBRec.inHDTIndex = inHostIndex[inKey - 1];
					memset(szBatchNum, 0x00, sizeof(szBatchNum));
					inGetBatchNum(szBatchNum);
					pobTran->srBRec.lnBatchNum = atol(szBatchNum);
					
					/* 票證 */
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetHostLabel(szHostName);
					if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
					{
						pobTran->srTRec.uszESVCTransBit = VS_TRUE;
					}

					inRetVal = VS_SUCCESS;
					break;
				}
			}
		}

		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}

	return (inRetVal);
}

/*
Function        :inFunc_All_Host_Settle_Check_Display
Date&Time       :2016/10/4 上午 11:59
Describe        :確認全部Host是否要先結帳，要下TMS前檢查用
*/
int inFunc_All_Host_Settle_Check_Display(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	
        vdUtility_SYSFIN_LogMessage(AT, " inFunc_All_Host_Settle_Check_Display START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_All_Host_Settle_Check_Display() START !");
	}
	
	inRetVal = inFunc_All_Host_Settle_Check(pobTran, VS_TRUE);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_All_Host_Settle_Check_Display() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_All_Host_Settle_Check
Date&Time       :2022/6/16 下午 4:55
Describe        :確認全部Host是否要先結帳，要下TMS前檢查用
*/
int inFunc_All_Host_Settle_Check(TRANSACTION_OBJECT *pobTran, unsigned char uszDisplayBit)
{
	int			i = 0;
	int			j = 0;
	int			inRetVal = 0;
        char			szMustSettleBit[2 + 1] = {};
	char			szHostEnable[2 + 1] = {};
	char			szHostName[40 + 1] = {};
	char			szEffectiveDate[8 + 1] = {0};
	unsigned char		uszFileName[15 + 1] = {};
	BYTE			bNeedSettleBit = VS_FALSE;
	TRANSACTION_OBJECT	pobTempTran = {0};
	RTC_NEXSYS		srRTC = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_All_Host_Settle_Check() START !");
	}
	
	memset(&pobTempTran, 0x00, sizeof(pobTempTran));
	pobTempTran.inRunOperationID = pobTran->inRunOperationID;
	
	for (i = 0 ;; i ++)
	{
		if (inLoadHDTRec(i) < 0)	/* 主機參數檔【HostDef.txt】 */
			break;
		
		pobTempTran.srBRec.inHDTIndex = i;
		memset(szHostEnable, 0x00, sizeof(szHostEnable));
		inGetHostEnable(szHostEnable);
		if (!memcmp(szHostEnable, "N", 1))
		{
			/* 基本上只有有開的Host才會有是否要結帳的問題，不過為了預防萬一，多檢查該Host是否有開 */
			continue;
		}
		
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		inFunc_DiscardSpace(szHostName);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Host Name : %s", szHostName);
		}
		/* 跳過ESC */
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}
		else if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
		{
			sprintf(szHostName, _HOST_NAME_CREDIT_NCCC_);
		}
		
		inLoadHDPTRec(i);
		memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
		inGetMustSettleBit(szMustSettleBit);
		
		if (!memcmp(szMustSettleBit, "Y", 1))
		{
			/* 表示要結帳 */
			bNeedSettleBit = VS_TRUE;
			break;
		}
		
		/* 票證要檢查四次 */
		if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
		{
			for (j = 0; j < _TDT_INDEX_MAX_; j++)
			{
				switch (j)
				{
					case 0:
						pobTempTran.srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
						break;
					case 1:
						pobTempTran.srTRec.inTDTIndex = _TDT_INDEX_01_ECC_;
						break;
					case 2:
						pobTempTran.srTRec.inTDTIndex = _TDT_INDEX_02_ICASH_;
						break;
					default:
						pobTempTran.srTRec.inTDTIndex = _TDT_INDEX_00_IPASS_;
						break;
				}
				inLoadTDTRec(pobTempTran.srTRec.inTDTIndex);
				
				/* Check REVERSAL */
				memset(uszFileName, 0x00, sizeof(uszFileName));
				pobTempTran.uszESVCFileNameByCardBit = VS_TRUE;
				inRetVal = inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6);
				pobTempTran.uszESVCFileNameByCardBit = VS_FALSE;
				if (inRetVal != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "組filename失敗：%s", uszFileName);
					}
					return (VS_ERROR);
				}
				if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
				{
					/* 表示要結帳 */
					bNeedSettleBit = VS_TRUE;
					break;
				}

				/* Check ADVICE */
				memset(uszFileName, 0x00, sizeof(uszFileName));
				pobTempTran.uszESVCFileNameByCardBit = VS_TRUE;
				inRetVal = inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6);
				pobTempTran.uszESVCFileNameByCardBit = VS_FALSE;
				if (inRetVal != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "組filename失敗：%s", uszFileName);
					}
					return (VS_ERROR);
				}
				if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
				{
					/* 表示要結帳 */
					bNeedSettleBit = VS_TRUE;
					break;
				}

				/* Check ADVICE ESC */
				memset(uszFileName, 0x00, sizeof(uszFileName));
				pobTempTran.uszESVCFileNameByCardBit = VS_TRUE;
				inRetVal = inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _ADVICE_ESC_FILE_EXTENSION_, 6);
				pobTempTran.uszESVCFileNameByCardBit = VS_FALSE;
				if (inRetVal != VS_SUCCESS)
				{
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "組filename失敗：%s", uszFileName);
					}
					return (VS_ERROR);
				}
				if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
				{
					/* 表示要結帳 */
					bNeedSettleBit = VS_TRUE;
					break;
				}
			}
			if (bNeedSettleBit == VS_TRUE)
			{
				/* 表示要結帳 */
				break;
			}
			
			/* Check Table Exist */
			if (inSqlite_Check_Table_Exist_Flow(&pobTempTran, _TN_BATCH_TABLE_) == VS_SUCCESS)
			{
				/* 表示要結帳 */
				bNeedSettleBit = VS_TRUE;
				break;
			}
		}
		else
		{
			/* Check REVERSAL */
		       memset(uszFileName, 0x00, sizeof(uszFileName));
		       if (inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
		       {
			       if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "組filename失敗：%s", uszFileName);
				}
				return (VS_ERROR);
		       }
		       if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		       {
				/* 表示要結帳 */
				bNeedSettleBit = VS_TRUE;
				break;
		       }

		       /* Check ADVICE */
		       memset(uszFileName, 0x00, sizeof(uszFileName));
		       if (inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _ADVICE_FILE_EXTENSION_, 6) != VS_SUCCESS)
		       {
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "組filename失敗：%s", uszFileName);
				}
				return (VS_ERROR);
		       }
		       if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		       {
				/* 表示要結帳 */
				bNeedSettleBit = VS_TRUE;
				break;
		       }

		       /* Check ADVICE ESC */
		       memset(uszFileName, 0x00, sizeof(uszFileName));
		       if (inFunc_ComposeFileName(&pobTempTran, (char*)uszFileName, _ADVICE_ESC_FILE_EXTENSION_, 6) != VS_SUCCESS)
		       {
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "組filename失敗：%s", uszFileName);
				}
				return (VS_ERROR);
		       }
		       if (inFILE_Check_Exist(uszFileName) == VS_SUCCESS)
		       {
				/* 表示要結帳 */
				bNeedSettleBit = VS_TRUE;
				break;
		       }
		       
		       /* Check Table Exist */
			if (inSqlite_Check_Table_Exist_Flow(&pobTempTran, _TN_BATCH_TABLE_) == VS_SUCCESS)
			{
				/* 表示要結帳 */
				bNeedSettleBit = VS_TRUE;
				break;
			}
		}
	}
	
	if (bNeedSettleBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "%s，有帳務，不可更新", szHostName);
		}
		
		if (uszDisplayBit == VS_TRUE)
		{
			/* 在idle時，只有秒位數為50可以顯示 */
			if (pobTempTran.inRunOperationID == _OPERATION_IDLE_CHECK_)
			{
				/* 取得參數生效日期 */
				inLoadTMSCPTRec(0);
				inGetTMSEffectiveDate(szEffectiveDate);
				memset(&srRTC, 0x00, sizeof(srRTC));
				inFunc_GetSystemDateAndTime(&srRTC);

				/* 參數立即生效時，要結帳訊息只能在50秒顯示，不然會過於頻繁 */
				if (!memcmp(&szEffectiveDate[0], "00000000", 8))
				{
					if (srRTC.uszSecond == 50)
					{
						/* 表示要結帳 */
						DISPLAY_OBJECT	srDispMsgObj;
						memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
						strcpy(srDispMsgObj.szDispPic1Name, _ERR_MUST_SETTLE_);
						srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
						srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
						srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
						strcpy(srDispMsgObj.szErrMsg1, szHostName);
						srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
						srDispMsgObj.inBeepTimes = 1;
						srDispMsgObj.inBeepInterval = 0;
						inDISP_Msg_BMP(&srDispMsgObj);

						ginIdleDispFlag = VS_TRUE;
					}
					else
					{

					}
				}
				else
				{
					/* 表示要結帳 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_MUST_SETTLE_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, szHostName);
					srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);

					ginIdleDispFlag = VS_TRUE;
				}
			}
			else
			{
				/* 表示要結帳 */
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_MUST_SETTLE_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, szHostName);
				srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);
			}
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "無帳務，可更新");
		}
	}
	
	/* load回來 */
	if (pobTran->srBRec.inHDTIndex >= 0)
	{
		inLoadHDTRec(pobTran->srBRec.inHDTIndex);
		inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
	}
	else
	{
		inLoadHDTRec(0);
		inLoadHDPTRec(0);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_All_Host_Settle_Check() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	if (bNeedSettleBit == VS_TRUE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inFunc_Display_Error
Date&Time       :2017/9/20 下午 12:00
Describe        :為了只顯示一次錯誤訊息
*/
int inFunc_Display_Error(TRANSACTION_OBJECT *pobTran)
{	
	int	inTimeout = _EDC_TIMEOUT_;
	char	szDebugMsg[100 + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
        char    szTemplate[42 + 1];
	
	/* 沒有設定Error code 跳過 */
	if (pobTran->inErrorMsg == 0x00)
	{
		return (VS_SUCCESS);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Error Msg : %d", pobTran->inErrorMsg);
		inLogPrintf(AT, szDebugMsg);
	}
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        memset(szTemplate, 0x00, sizeof(szTemplate));
        inGetHostLabel(szTemplate);
	
	if  (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)	||
	     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
	{
		if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
		{
			inTimeout = 2;
		}
		else
		{
			inTimeout = _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_;
		}
	}
	else if  (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		  !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inTimeout = _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_;
	}
	else
	{
		if (pobTran->srEWRec.uszEWTransBit == VS_TRUE)
		{
			inTimeout = 2;
		}
		else
		{
			inTimeout = _EDC_TIMEOUT_;
		}
	}
	
	/* 統一在這裡顯示錯誤訊息 */
	/* 結帳出錯一律顯示結帳失敗 */
	if (pobTran->inRunTRTID == _TRT_SETTLE_	|| 
	    pobTran->inErrorMsg == _ERROR_CODE_V3_SETTLE_NOT_SUCCESS_ ||
            pobTran->inErrorMsg == _ERROR_CODE_V3_SETTLE_RETRY_)
	{
                if ((!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                     !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)) &&
                    (!memcmp(szTemplate, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) || 
                     !memcmp(szTemplate, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_))) &&
                    pobTran->inErrorMsg == _ERROR_CODE_V3_COMM_)
                {
                        /* 客製化098 結帳時因為Retry，通訊異常時會有錯誤訊息，加上需求的訊息，導致兩次錯誤訊息出現，所以遮掩第一次多餘訊息 
                           NCCC、DCC才會重試，電票因為悠遊卡主導所以無法實作，所以不阻擋連線時錯誤訊息
                        */
                }
                else
                {    
                        DISPLAY_OBJECT	srDispMsgObj;
                        memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));

                        if (pobTran->inErrorMsg == _ERROR_CODE_V3_SETTLE_RETRY_)
                                strcpy(srDispMsgObj.szDispPic1Name, _ERR_SETTLE_FAILED_RETRY_);
                        else
                                strcpy(srDispMsgObj.szDispPic1Name, _ERR_SETTLE_FAILED_); 
                        
                        srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;

                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                        {
                                srDispMsgObj.inMsgType = _NO_KEY_MSG_;
                                srDispMsgObj.inTimeout = 2;
                        }
                        else
                        {    
                                /* 連動結帳時，不顯示請按清除鍵 */
                                if (pobTran->uszAutoSettleBit == VS_TRUE)
                                {
                                        srDispMsgObj.inMsgType = _NO_KEY_MSG_;
                                        srDispMsgObj.inTimeout = 2;
                                }
                                else
                                {
                                        srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
                                        srDispMsgObj.inTimeout = inTimeout;
                                }
                        }

                        strcpy(srDispMsgObj.szErrMsg1, "");
                        srDispMsgObj.inErrMsg1Line = 0;
                        srDispMsgObj.inBeepTimes = 1;
                        srDispMsgObj.inBeepInterval = 0;

                        inDISP_Msg_BMP(&srDispMsgObj);
                }
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_USER_CANCEL_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_USER_TERMINATE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_USER_TERMINATE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_COMM_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE						||
		    memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0		||
		    pobTran->inRunOperationID == _OPERATION_EDC_BOOTING_				||
		    pobTran->inRunOperationID == _OPERATION_ESVC_AUTO_SIGNON_)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CONNECT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			if (ginSSLErrCode > 0)
                        {
                                sprintf(srDispMsgObj.szErrMsg1, "SSL Error(%d)", ginSSLErrCode);
                                srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
                                srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
                                ginSSLErrCode = 0;
                        }
                        else
                        {
                                strcpy(srDispMsgObj.szErrMsg1, "");
                                srDispMsgObj.inErrMsg1Line = 0;
                        }
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);			/* 通訊失敗 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CONNECT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
                        
                        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_)     ||
                            !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))
                        {
                                srDispMsgObj.inTimeout = _CUSTOMER_105_MCDONALDS_DISPLAY_TIMEOUT_;
                        }
                        else
                        {
                                srDispMsgObj.inTimeout = inTimeout;
                        }
                      
                        if (ginSSLErrCode > 0)
                        {
                                sprintf(srDispMsgObj.szErrMsg1, "SSL Error(%d)", ginSSLErrCode);
                                srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
                                srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
                                ginSSLErrCode = 0;
                        }
                        else
                        {
                                strcpy(srDispMsgObj.szErrMsg1, "");
                                srDispMsgObj.inErrMsg1Line = 0;
                        }
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_ISO_PACK_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_ISO_UNPACK_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_ISO_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);			/* 電文錯誤 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_ISO_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);				/* 電文錯誤 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_01_READ_FAIL_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&& 
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inFISC_CTLS_LED_TONE(VS_ERROR);
		}
		
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);			/* 讀卡失敗 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_CARD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);			/* 讀卡失敗 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_02_6982_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&& 
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inFISC_CTLS_LED_TONE(VS_ERROR);
		}
		
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_FAIL_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);			/* 卡片失效 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_FAIL_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);			/* 卡片失效 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_06_TMS_NOT_SUPPORT_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&& 
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inFISC_CTLS_LED_TONE(VS_ERROR);
		}
		
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_SMARTPAY_CTLS_CARD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 不接受此感應卡 請改插卡 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_SMARTPAY_CTLS_CARD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 不接受此感應卡 請改插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_07_AMT_OVERLIMIT_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&& 
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inFISC_CTLS_LED_TONE(VS_ERROR);
		}
		
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_OVER_SMARTPAY_CTLS_LIMIT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);	/* 超過感應限額 請改插卡 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_OVER_SMARTPAY_CTLS_LIMIT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);	/* 超過感應限額 請改插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_09_NOT_RIGHT_INCODE_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&& 
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inFISC_CTLS_LED_TONE(VS_ERROR);
		}
                
                /* Mirror Message */
                if (pobTran->uszECRBit == VS_TRUE)
                {
                        inECR_SendMirror(pobTran, _MIRROR_MSG_WRONG_CARD_ERROR_);
                }
		
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_FOLLOW_RIGHT_OPT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 請依正確卡別操作 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_FOLLOW_RIGHT_OPT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);	/* 請依正確卡別操作 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_03_TIME_ERROR_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_04_MAC_TAC_		||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_05_NO_INCODE_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_08_NO_CARD_BIN_	||
		 pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_10_LOGON_FAIL_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&& 
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inFISC_CTLS_LED_TONE(VS_ERROR);
		}
		
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_RECEIVE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);			/* 感應失敗 請改插卡或刷卡 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_RECEIVE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 感應失敗 請改插卡或刷卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_11_FISC_FALLBACK_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&& 
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inFISC_CTLS_LED_TONE(VS_ERROR);
		}
		
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "請改插金融卡");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			inDISP_Msg_BMP(&srDispMsgObj);			/* 請改插金融卡 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "請改插金融卡");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);			/* 請改插金融卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_)
	{
		/* 感應燈號及聲響 */
		if (pobTran->srBRec.uszFiscTransBit == VS_TRUE	&& 
		    pobTran->srBRec.uszContactlessBit == VS_TRUE)
		{
			inFISC_CTLS_LED_TONE(VS_ERROR);
		}
		
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_RECEIVE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);			/* 感應失敗 請改插卡或刷卡 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_RECEIVE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 感應失敗 請改插卡或刷卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_CTLS_)
	{
		/* 感應失敗 請改插卡或刷卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_RECEIVE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);			/* 感應失敗 請改插卡或刷卡 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_RECEIVE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 感應失敗 請改插卡或刷卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_CTLS_REAL_)
	{
		/* 亮紅燈 */
		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 不接受此感應卡 請改刷卡或插卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_CTLS_CARD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_CTLS_CARD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_)
	{
		/* 亮紅燈 */
		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 超過感應限額 請改插卡或刷卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_OVER_SMARTPAY_CTLS_LIMIT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 超過感應限額 請改插卡 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_OVER_SMARTPAY_CTLS_LIMIT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 超過感應限額 請改插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_TIP_TOO_BIG_)
	{
		/* 亮紅燈 */
		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 金額小於手續費 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "金額小於手續費");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "金額小於手續費");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_)
	{
		/* 感應失敗 請改插卡或刷卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_RECEIVE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_RECEIVE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_)
	{
		/* 感應失敗 超過一張卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "多卡感應錯誤");
			strcpy(srDispMsgObj.szErrMsg2, "請重新感應");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inErrMsg2Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "多卡感應錯誤");
			strcpy(srDispMsgObj.szErrMsg2, "請重新感應");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inErrMsg2Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC)
	{
		/* 亮紅燈 */
		inFISC_CTLS_LED_TONE(VS_ERROR);
		/* 不接受此感應卡 請改刷卡或插卡 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_CTLS_CARD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_SUP_CTLS_CARD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);		/* 不接受此感應卡 請改刷卡或插卡 */
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FUNC_CLOSE_)
	{
		/* 此功能已關閉 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_FUNC_CLOSE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_FUNC_CLOSE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_CTLS_DATA_SHORT_)
	{
		/* 感應資料不足 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_DATA_SHORT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CTLS_DATA_SHORT_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WAVE_ERROR_Z1_)
	{
		/* 拒絕交易 Z1 */
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_USE_MS_OR_ICC_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "拒絕交易 Z1");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_USE_MS_OR_ICC_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "拒絕交易 Z1");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TICKET_AMOUNT_TOO_MUCH_IN_ONE_TRANSACTION_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "金額超過單筆上限");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "金額超過單筆上限");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TICKET_AMOUNT_NOT_ENOUGH_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "餘額不足");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "餘額不足");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TRT_NOT_FOUND_)
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "找不到TRT");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "找不到TRT");
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	/* 手續費金額有誤 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_ECR_INST_FEE_NOT_0_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_INSTFEE_NOT_0_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_4_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_INSTFEE_NOT_0_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_4_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_INSTFEE_NOT_0_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_4_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	/* 晶片卡被取出 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_CARD_OUT_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_REMOVED_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_REMOVED_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CARD_REMOVED_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_FALLBACK_)
	{
		if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
		{
			/* 聯合不顯示訊息 */
		}
		else
		{
			if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_USE_MS_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 0;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 3;
				srDispMsgObj.inBeepInterval = 2000;
			
				inDISP_Msg_BMP(&srDispMsgObj);
			}
			else if (pobTran->uszECRBit == VS_TRUE)
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_USE_MS_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 1;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
		
				inDISP_Msg_BMP(&srDispMsgObj);
			}
			else
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, _ERR_USE_MS_);
				srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 1;
				strcpy(srDispMsgObj.szErrMsg1, "");
				srDispMsgObj.inErrMsg1Line = 0;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				
				inDISP_Msg_BMP(&srDispMsgObj);
			}
		}
	}
	/* 請改讀晶片卡 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_PLS_READ_EMV_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_EMV_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_EMV_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_EMV_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	/* 請讀晶片，無晶片 請持卡人洽發卡行 */
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EMV_PLS_READ_EMV_CUP_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_EMV_CUP_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_EMV_CUP_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_READ_EMV_CUP_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_SAM_ENCRYPTED_FAIL_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_SAM_ENCRYPTED_FAIL_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_SAM_ENCRYPTED_FAIL_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_PAN_SIZE_ERROR_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PAN_SIZE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PAN_SIZE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PAN_SIZE_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_PAN_TOO_LONG_ERROR_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "卡號長度錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "卡號長度錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "卡號長度錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_EXP_DATE_NOT_VALID_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_EXP_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_EXP_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_EXP_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_ECR_UNPACK_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, pobTran->szErrorMsgBuff1);
			srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			strcpy(srDispMsgObj.szErrMsg2, pobTran->szErrorMsgBuff2);
			srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
			srDispMsgObj.inErrMsg2FontSize = _FONTSIZE_8X22_;
			strcpy(srDispMsgObj.szErrMsg3, "接收資料錯誤");
			srDispMsgObj.inErrMsg3Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 2000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, pobTran->szErrorMsgBuff1);
			srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			strcpy(srDispMsgObj.szErrMsg2, pobTran->szErrorMsgBuff2);
			srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
			srDispMsgObj.inErrMsg2FontSize = _FONTSIZE_8X22_;
			strcpy(srDispMsgObj.szErrMsg3, "接收資料錯誤");
			srDispMsgObj.inErrMsg3Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, pobTran->szErrorMsgBuff1);
			srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			strcpy(srDispMsgObj.szErrMsg2, pobTran->szErrorMsgBuff2);
			srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
			srDispMsgObj.inErrMsg2FontSize = _FONTSIZE_8X22_;
			strcpy(srDispMsgObj.szErrMsg3, "接收資料錯誤");
			srDispMsgObj.inErrMsg3Line = _LINE_8_6_;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TXN_FAIL_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "交易失敗");
			srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "交易失敗");
			srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "交易失敗");
			srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TXN_CALL_BANK_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CALL_BANK_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inErrMsg1FontSize = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CALL_BANK_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inErrMsg1FontSize = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CALL_BANK_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_5_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inErrMsg1FontSize = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TXN_TIMEOUT_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "交易逾時");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "交易逾時");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "交易逾時");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_TXN_NOT_MEMBER_CARD_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "非參加機構卡片");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "非參加機構卡片");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "非參加機構卡片");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_OPERATION_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "操作錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "操作錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "操作錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_PLS_SELECT_CREDIT_HOST_)
	{
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "請選擇信用卡主機");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "請選擇信用卡主機");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "請選擇信用卡主機");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FUNC_NOT_SUPPORT_)
	{
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_082_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = 30;
			strcpy(srDispMsgObj.szErrMsg1, "此功能不支援");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			/* 此功能不支援 */
			if (pobTran->uszECRBit == VS_TRUE	||
			    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = 0;
				srDispMsgObj.inMsgType = _NO_KEY_MSG_;
				srDispMsgObj.inTimeout = 2;
				strcpy(srDispMsgObj.szErrMsg1, "此功能不支援");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;

				inDISP_Msg_BMP(&srDispMsgObj);
			}
			else
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = 0;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = inTimeout;
				strcpy(srDispMsgObj.szErrMsg1, "此功能不支援");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);
			}
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_NOT_SUPPORT_THIS_PAY_ITEM_)
	{
		/* 不支援該繳費項目 */
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "不支援該繳費項目");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "不支援該繳費項目");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "不支援該繳費項目");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_AUTH_CODE_NOT_VALID_)
	{
		/* 拒絕交易 */
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "拒絕交易");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "拒絕交易");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "拒絕交易");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_FPG_FTC_ICC_CHECK_CARD)
	{
		/* 請檢查聯名卡 */
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "請檢查聯名卡");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "請檢查聯名卡");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "請檢查聯名卡");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_WRITE_KEY_INIT_FAIL_)
	{
		/* 初始化失敗請報修 */
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "初始化失敗請報修");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "初始化失敗請報修");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X16_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "初始化失敗請報修");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_SELECT_AID_TIMEOUT_)
	{
		/* 處理逾時，請重新交易 */
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "處理逾時，請重新交易");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "處理逾時，請重新交易");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "處理逾時，請重新交易");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else if (pobTran->inErrorMsg == _ERROR_CODE_V3_DO_NOT_CUP_KEY_)
	{
                if (pobTran->srBRec.uszContactlessBit == VS_TRUE ||
                    pobTran->srBRec.uszRefundCTLSBit == VS_TRUE)
                {
                        /* 亮紅燈 */
                        inFISC_CTLS_LED_TONE(VS_ERROR);
                }
                
		/* 請勿按銀聯鍵 */
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_PRESS_CUP_BUTTON_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_PRESS_CUP_BUTTON_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_NOT_PRESS_CUP_BUTTON_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
        else if (pobTran->inErrorMsg == _ERROR_CODE_V3_ID_INVALID_)
	{       
		/* ID身份證字號錯誤 */
		if (pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "ID身份證字號錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
		
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else if (pobTran->uszECRBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "ID身份證字號錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "ID身份證字號錯誤");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inErrMsg1FontSize = _FONTSIZE_8X22_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	else
	{
		if (pobTran->uszECRBit == VS_TRUE	||
		    pobTran->uszMultiFuncSlaveBit == VS_TRUE)
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "交易失敗");
			srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
			sprintf(srDispMsgObj.szErrMsg2, "未定義錯誤 : %d", pobTran->inErrorMsg);
			srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
                        if (strlen(pobTran->szUnpredictErrorLogMsg) > 0)
                        {
                                sprintf(srDispMsgObj.szErrMsg3, "%s", pobTran->szUnpredictErrorLogMsg);
                                srDispMsgObj.inErrMsg3Line = _LINE_8_6_;
                        }
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = inTimeout;
			strcpy(srDispMsgObj.szErrMsg1, "交易失敗");
			srDispMsgObj.inErrMsg1Line = _LINE_8_4_;
			sprintf(srDispMsgObj.szErrMsg2, "未定義錯誤 : %d", pobTran->inErrorMsg);
			srDispMsgObj.inErrMsg2Line = _LINE_8_5_;
                        if (strlen(pobTran->szUnpredictErrorLogMsg) > 0)
                        {
                                sprintf(srDispMsgObj.szErrMsg3, "%s", pobTran->szUnpredictErrorLogMsg);
                                srDispMsgObj.inErrMsg3Line = _LINE_8_6_;
                        }
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	
	/* 為了強調Timeout時間 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	/* 顯示完就清空(為了只顯示一次) */
	pobTran->inErrorMsg = 0x00;
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_IdleCheckALL_DateAndTime
Date&Time       :2016/10/21 下午 1:47
Describe        :在 Idle下Check時間觸發的事件,進入func檢查排程時間，若時間合法(沒過期)，將日期（沒有的部份要補0）傳出，若比szEarlyDateTime還要早，則替換，如此可得到最接近的排程
*/
int inFunc_IdleCheckALL_DateAndTime(int *inEvent)
{
	char			szTemplate[16 + 1] = {0};
	char			szDCCDownloadMode[2 + 1] = {0};
	char			szESCMode[2 + 1] = {0};
	char			szCloseBatchBit[2 + 1] = {0};
	char			szTMSOK[2 + 1] = {0};
	char			szDEMOMode[2 + 1] = {0};
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszUpdateBit = VS_FALSE;
	TRANSACTION_OBJECT	pobTran;
	
	memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
	pobTran.inRunOperationID = _OPERATION_IDLE_CHECK_;
	
	/* 預設為_NONE_EVENT_ */
	*inEvent = _NONE_EVENT_;
	
	inFunc_Check_PCI_Reboot();
	
	/* TMS未下載成功就不跑排程 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	/* 未下載過TMS，不跑流程 */
	if (memcmp(&szTMSOK[0], "Y", 1) != 0)
	{
		return (VS_SUCCESS);
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);

	/* 教育訓練版本不跑排程 */
	memset(szDEMOMode, 0x00, sizeof(szDEMOMode));
	inGetDemoMode(szDEMOMode);
	if (memcmp(szDEMOMode, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	
	/* TMS參數詢問 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSInquireMode(szTemplate);
	/* TMS Inquire Mode = 2 由TMS安排時間自動詢問 */
	if (!memcmp(&szTemplate[0], _TMS_INQUIRE_02_SCHEDHULE_SETTLE_, 1))
	{
		/* 參數詢問 */
		if (inNCCCTMS_Schedule_Inquire_Date_Time_Check() == VS_SUCCESS)
		{
			*inEvent = _TMS_SCHEDULE_INQUIRE_EVENT_;

			return (VS_SUCCESS);
		}
	}
	
	/* TMS排程下載 */
	inLoadTMSCPTRec(0);
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSDownloadFlag(szTemplate);
	/* TMS Download Flag = 2 由TMS排程下載 */
	if (!memcmp(szTemplate, _TMS_DOWNLOAD_FLAG_SCHEDULE_, strlen(_TMS_DOWNLOAD_FLAG_SCHEDULE_)))
	{
		/* 排程時間檢查 */
		if (inNCCCTMS_Schedule_Download_Date_Time_Check(&pobTran) == VS_SUCCESS)
		{
			*inEvent = _TMS_SCHEDULE_DOWNLOAD_EVENT_;

			return (VS_SUCCESS);
		}
	}
	
	/* TMS參數生效 */
	/* 有File List檔案代表有參數要更新 */
	/* 代表有參數列表，有機會更新，接著檢查時間 */
	/* 只有走ISO才要排程更新 */
	if (inNCCCTMS_Check_FileList_Flow(&pobTran) == VS_SUCCESS)
	{
		/* ISO8583 */
		if (pobTran.uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_NONE_)
		{
			if (inNCCCTMS_Schedule_Effective_Date_Time_Check(&pobTran) == VS_SUCCESS)
			{
				/* 檢查時間到時，檢查FileList 及 下載檔案是否合法 */
				if (inNCCCTMS_CheckAllDownloadFile_Flow(&pobTran) != VS_SUCCESS)
				{
					
				}
				else
				{
					/* 檢查是否有帳 */
					inLoadTMSCPTRec(0);
					inGetTMSEffectiveCloseBatch(szCloseBatchBit);
					if (memcmp(szCloseBatchBit, "Y", strlen("Y")) == 0)
					{
						if (inFLOW_RunFunction(&pobTran, _FUNCTION_All_HOST_MUST_SETTLE_) != VS_SUCCESS)
						{

						}
						else
						{
							uszUpdateBit = VS_TRUE;
						}
					}
					else
					{
						uszUpdateBit = VS_TRUE;
					}
					
					if (uszUpdateBit == VS_TRUE)
					{
						inDISP_ClearAll();
						inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
						inDISP_PutGraphic(_MENU_TMS_DOWNLOAD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜TMS參數下載＞ */
						inDISP_PutGraphic(_TMS_UPDATING_, 0, _COORDINATE_Y_LINE_8_4_);

						/* 重開機更新 */
						inFunc_Reboot();
					}
				}
			}
		}
		/* FTPS */
		else
		{
			
		}
	}
	
	/* DCC排程下載時間檢查 */
	if (inNCCC_DCC_AutoDownload_Check() == VS_SUCCESS)
	{
		*inEvent = _DCC_SCHEDULE_EVENT_;
		
		return (VS_SUCCESS);
	}
	
	/* 整點提示DCC下載 */
	memset(szDCCDownloadMode, 0x00, sizeof(szDCCDownloadMode));
	inGetDCCDownloadMode(szDCCDownloadMode);
	if (memcmp(szDCCDownloadMode, _NCCC_DCC_DOWNLOAD_MODE_HOUR_, 1) == 0)
	{
		if (inNCCC_DCC_TMS_Schedule_Hour_Check() == VS_SUCCESS)
		{
			*inEvent = _DCC_HOUR_NOTIFY_EVENT_;
			
			return (VS_SUCCESS);
		}
	}
	
	/* 每五分鐘檢查一次是否有ESC未上傳 */
	/* 先確認ESC功能有沒有開 */
	memset(szESCMode, 0x00, sizeof(szESCMode));
	inGetESCMode(szESCMode);
	if (memcmp(&szESCMode[0], "Y", 1) == 0)
	{
		/* 檢查是否已離上次上傳超過五分鐘 */
		if (inNCCC_ESC_Func_Upload_Idle_Check_Time() == VS_SUCCESS)
		{
			*inEvent = _ESC_IDLE_UPLOAD_EVENT_;

			return (VS_SUCCESS);
		}
	}
	
	/* 自動排程SignOn */
	/* 有開票證才檢查 */
	if (inNCCC_Ticket_Check_ESVC_Need_SignOn_Flow() == VS_SUCCESS)
	{
		if (inNCCC_Ticket_Schedule_Auto_SignOn_Time_Check() == VS_SUCCESS)
		{
			*inEvent = _ESVC_AUTO_SIGNON_EVENT_;

			return (VS_SUCCESS);
		}
	}
        
        /* 檢查是否要更換log檔名 */
        vdUtility_SYSFIN_check_log_date();
		
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckTermStatus
Date&Time       :2016/10/26 下午 1:38
Describe        :確認機器狀態，包括是否已下TMS參數和DCC參數
*/
int inFunc_CheckTermStatus(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_ERROR;
	char		szTMSOK[2 + 1] = {0};
	char		szEncryptionMode[2 + 1] = {0};
	char		szTSAMRegisterEnable[2 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szHostEnable[2 + 1] = {0};
	char		szDCCInit[2 + 1] = {0};
	char		szFESMode[2 + 1] = {0};
	unsigned char	uszSlot = 0x00;
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus START");
	
	/* 檢查是否已經鎖機 */
	if (inFunc_Check_EDCLock() != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus EDC Lock Stop");
		return (VS_ERROR);
	}

	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) != 0)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus TMSOK:(%s)", szTMSOK);
		
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TMS_DWL_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}

	/* 檢查是否做過【DCC參數下載】 */
	/* 檢查HDT是否有DCC這個HOST並Enable */
	/* 有DCC這個Host才要繼續檢查 */
	memset(szHostEnable, 0x00, sizeof(szHostEnable));
	inNCCC_DCC_GetDCC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable);
		
	if (memcmp(szHostEnable, "Y", 1) == 0)
	{
		/* 檢查CFGT內的DCC開關
		* DCC詢價版本, 預設值 = 0, 0 = 不支援DCC, 1 = 直接於詢價時由 DCC 依 Card Bin 回覆其外幣幣別及金額, 2 = 於EDC選擇交易幣別詢價 */
		/* (需求單-111060)-DCC流程修改需求 by Russell 2022/4/11 下午 3:26 */
		/* 不參考DCC FlowVersion */

		/* DCC目前只支援ATS和MFES */
		memset(szFESMode, 0x00, sizeof(szFESMode));
		inGetNCCCFESMode(szFESMode);
		if (memcmp(szFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_)) == 0	||
		    memcmp(szFESMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_)) == 0)
		{

		}
		else
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus Not support DCC FES(%s)", szFESMode);
			
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, "");
			srDispMsgObj.inDispPic1YPosition = 0;
			srDispMsgObj.inMsgType = _0_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "此版不支援DCC");
			srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

			return (VS_ERROR);
		}

		/* 代表有開DCC功能，如果是新裝機，就擋(因沒有舊參數仍要可以做信用卡交易) */
		memset(szDCCInit, 0x00, sizeof(szDCCInit));
		inGetDCCInit(szDCCInit);
		if (memcmp(szDCCInit, "1", 1) == 0)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus Not support DCC szDCCInit(%s)", szDCCInit);
			
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_DCC_DL_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

			return (VS_ERROR);
		}
	}
	else
	{
		/* 沒DCC跳過不檢查 */
	}
	
	inRetVal = inNCCC_Func_Check_TMS_Setting_Compatible(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus Check_TMS_Setting_Compatible Failed");
		
		return (VS_ERROR);
	}

	/* 檢查是否有 註冊TSAM 和是否有放入SLOT1 SAM卡 */
	memset(szEncryptionMode, 0x00, sizeof(szEncryptionMode));
	inGetEncryptMode(szEncryptionMode);
	if (memcmp(szEncryptionMode, _NCCC_ENCRYPTION_TSAM_, 1) == 0)
	{
		/* 抓tSAM Slot */
		inRetVal = inNCCC_tSAM_Decide_tSAM_Slot(&uszSlot);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus decide tSAM Slot Failed ");
			
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inNCCC_tSAM_Decide_tSAM_Slot Failed");
				inLogPrintf(AT, szDebugMsg);
			}
		}
		
		/* 失敗(沒放入SAM卡) */
		inRetVal = inNCCC_tSAM_Check_SAM_In_Slot_By_Global_Variable(uszSlot);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus Check SAM Slot Failed uszSlot(%d)", uszSlot);
			
			/* 請檢查SAM卡 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHECK_SAM_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = 1;
			
			char	szHostEnable[1 + 1] = {0};
			inNCCC_Ticket_GetESVC_Enable(pobTran->srBRec.inHDTIndex, szHostEnable);
			if (szHostEnable[0] == 'N')
			{
				sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM1");
			}
			else
			{
				sprintf(srDispMsgObj.szErrMsg1, "SAM卡請放SAM%d", uszSlot);
			}
			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

			return (VS_ERROR);
		}
	
		/* 檢查是否有註冊SAM卡 */
		memset(szTSAMRegisterEnable, 0x00, sizeof(szTSAMRegisterEnable));
		if (inGetTSAMRegisterEnable(szTSAMRegisterEnable) != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus get szTSAMRegisterEnable failed ");
			
			return (VS_ERROR);
		}
		
		if (memcmp(szTSAMRegisterEnable, "Y", 1) != 0)
		{
			vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus Not Registered szTSAMRegisterEnable(%s)", szTSAMRegisterEnable);
			
			/* 請註冊SAM卡 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PLEASE_REGISTER_SAM_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
			
			return (VS_ERROR);
		}
		
	}
	
	/* 檢核是否有Inv重複問題 */
	if (inNCCC_Func_Check_Inv_Problem_Before_Txn(pobTran) != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus inNCCC_Func_Check_Inv_Problem_Before_Txn Failed");
		return (VS_ERROR);
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus END");

	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckTermStatus_Ticket
Date&Time       :2018/1/17 下午 2:18
Describe        :確認機器狀態，包括是否已下TMS參數
*/
int inFunc_CheckTermStatus_Ticket(TRANSACTION_OBJECT *pobTran)
{
	int	inESVCIndex = -1;
	char	szTMSOK[2 + 1] = {0};
	char	szSettleBit[2 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckTermStatus_Ticket START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_CheckTermStatus_Ticket() START !");
	}
	
	/* 檢查是否已經鎖機 */
	if (inFunc_Check_EDCLock() != VS_SUCCESS)
		return (VS_ERROR);

	/* 檢查是否做過【參數下載】 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) != 0)
	{
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_TMS_DWL_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}
	
	inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_ESVC_, &inESVCIndex);
		
	if (inLoadHDTRec(inESVCIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	if (inLoadHDPTRec(inESVCIndex) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	/* 檢查是否要結帳 */
	memset(szSettleBit, 0x00, sizeof(szSettleBit));
	inGetMustSettleBit(szSettleBit);
	if (memcmp(szSettleBit, "Y", strlen("Y")) == 0)
	{
		/* 表示要結帳 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_MUST_SETTLE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "電子票證");
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
		
		return (VS_ERROR);
	}
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		pobTran->srTRec.inTicketType = _TICKET_TYPE_NONE_;

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inFunc_CheckTermStatus_Ticket() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
	else
	{
		pobTran->srTRec.inTicketType = _TICKET_TYPE_NONE_;

		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inFunc_CheckTermStatus_Ticket() END !");
			inLogPrintf(AT, "----------------------------------------");
		}

		return (VS_SUCCESS);
	}
}

/*
Function        :inFunc_ResetTitle
Date&Time       :2016/9/7 下午 1:02
Describe        :重新顯示交易別標題
*/
int inFunc_ResetTitle(TRANSACTION_OBJECT *pobTran)
{
	int	inDisplay = VS_FALSE;
	
        inDISP_ClearAll();
	
	/* 自助加油機不顯示Title */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		return (VS_SUCCESS);
	}
        
	/* 考慮到_TRT_SALE_ICC_和_TRT_SALE_CTLS_無法區分交易別，改以inTransactionCode來判斷 */
        if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{                           
                /* 如果OPT是重印，要顯示重印才對 */
                inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                inDISP_PutGraphic(_MENU_REPRINT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜重印簽單＞ */
                inDisplay = VS_TRUE;

		return (VS_SUCCESS);
	}
	else if (pobTran->inTransactionCode == _TICKET_DEDUCT_)
	{
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */
		inDisplay = VS_TRUE;
		
		return (VS_SUCCESS);
	}
	else if (pobTran->inRunOperationID == _OPERATION_TICKET_INQUIRY_)
	{
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜餘額查詢＞ */
		inDisplay = VS_TRUE;
		
		return (VS_SUCCESS);
	}
	else if (pobTran->inRunOperationID == _OPERATION_TICKET_TOP_UP_)
	{
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜餘額加值＞ */
		inDisplay = VS_TRUE;
		
		return (VS_SUCCESS);
	}
        else if (pobTran->inRunOperationID == _OPERATION_HG_) 
        {
                switch(pobTran->srBRec.lnHGTransactionType) 
                {
                        case    _HG_REWARD_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);         /* 第三層顯示 ＜HG卡紅利積點＞ */
				inDisplay = VS_TRUE;
                                break;                        
                        case    _HG_ONLINE_REDEEM_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_HAPPYGO_ONLINE_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);  /* 第三層顯示 ＜HG卡加價購＞ */
				inDisplay = VS_TRUE;
                                break;
                        case    _HG_POINT_CERTAIN_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_HAPPYGO_POINT_CERTAIN_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);  /* 第三層顯示 ＜HG卡點數抵扣＞ */
				inDisplay = VS_TRUE;
                                break;
                        case    _HG_FULL_REDEMPTION_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_HAPPYGO_REDEMPTION_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);     /* 第三層顯示 ＜HG卡點數兌換＞ */
				inDisplay = VS_TRUE;
                                break;
                        case    _HG_INQUIRY_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_HAPPYGO_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);        /* 第三層顯示 ＜HG卡點數查詢＞ */
				inDisplay = VS_TRUE;
                                break;
                        default :
				break;       
                }
                /* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
        }
	else if (pobTran->inRunOperationID == _OPERATION_HG_I_R_ && pobTran->srBRec.lnHGTransactionType == _HG_REWARD_)
	{
		switch(pobTran->inTransactionCode) 
                {
                        case    _REDEEM_SALE_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡紅利扣抵＞ */
				inDisplay = VS_TRUE;
                                break;                        
                        case    _INST_SALE_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_INS_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜HG卡分期付款＞ */
				inDisplay = VS_TRUE;
                                break;
                        default :
				break;       
                }
                /* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	}
	else if (pobTran->inRunOperationID == _OPERATION_HG_REFUND_)
	{
		switch(pobTran->srBRec.lnHGTransactionType) 
                {
                        case    _HG_REWARD_REFUND_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_HAPPYGO_REWARD_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜HG卡回饋退貨＞ */
				inDisplay = VS_TRUE;
                                break;                        
                        case    _HG_REDEEM_REFUND_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                                inDISP_PutGraphic(_MENU_HAPPYGO_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜HG卡扣抵退貨＞ */
				inDisplay = VS_TRUE;
                                break;
                        default :
				break;       
                }
                /* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	}
	else if (pobTran->inRunOperationID == _OPERATION_SDK_READ_CARD_)
	{
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDisplay = VS_TRUE;
		/* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	}
	
	if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
	{
		switch (pobTran->inTransactionCode)
		{
			case	_SALE_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜DCC一般交易＞ */
				inDisplay = VS_TRUE;
				break;
			case	_VOID_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜DCC取消交易＞ */
				inDisplay = VS_TRUE;
				break;
			case	_REFUND_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜DCC退貨交易＞ */
				inDisplay = VS_TRUE;
				break;
			case	_TIP_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_TIP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜DCC小費交易＞ */
				inDisplay = VS_TRUE;
				break;
			case	_DCC_RATE_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_DCC_RATE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜DCC詢價交易＞ */
				inDisplay = VS_TRUE;
				break;
			case	_PRE_COMP_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜預先授權完成＞ */
				inDisplay = VS_TRUE;
				break;
			case	_SETTLE_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */
				inDisplay = VS_TRUE;
				break;
			default :
				break;
				
		}
		/* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	}
	
	if (pobTran->srBRec.uszNCCCDCCRateBit == VS_TRUE)
	{
		switch (pobTran->inTransactionCode)
		{
			case	_SALE_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
				inDisplay = VS_TRUE;
				break;
			case	_VOID_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */
				inDisplay = VS_TRUE;
				break;
			case	_TIP_:
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_TIP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜小費交易＞ */
				inDisplay = VS_TRUE;
				break;
			default :
				break;
		}
		
		/* 有Display就跳出否則繼續跑下面 */
		if (inDisplay == VS_TRUE)
			return (VS_SUCCESS);
	}
	
	switch (pobTran->inTransactionCode)
	{
		case	_SALE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_VOID_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_REFUND_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */
			inDisplay = VS_TRUE;
			break;
		case	_REDEEM_SALE_:
		case	_CUP_REDEEM_SALE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利扣抵＞ */
			inDisplay = VS_TRUE;
			break;
		case	_REDEEM_REFUND_:
		case	_CUP_REDEEM_REFUND_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利退貨＞ */
			inDisplay = VS_TRUE;
			break;
		case	_REDEEM_ADJUST_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_REDEEM_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜紅利調帳＞ */
			inDisplay = VS_TRUE;
			break;
		case	_INST_SALE_:
		case	_CUP_INST_SALE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期付款＞ */
			inDisplay = VS_TRUE;
			break;
		case	_INST_REFUND_:
		case	_CUP_INST_REFUND_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期退貨＞ */
			inDisplay = VS_TRUE;
			break;
		case	_INST_ADJUST_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_INST_ADJUST_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜分期調帳＞ */
			inDisplay = VS_TRUE;
			break;
		case	_SETTLE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜結帳交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_SALE_OFFLINE_:
			if (pobTran->srBRec.uszReferralBit == VS_TRUE)
			{
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_CALL_BANK_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 授權碼補登 */
			}
			else
			{
				inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
				inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
			}
			inDisplay = VS_TRUE;
			break;
		case	_TIP_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TIP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜小費交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_PRE_AUTH_:
		case	_CUP_PRE_AUTH_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權＞ */
			inDisplay = VS_TRUE;
			break;
		case	_ADJUST_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        inDISP_PutGraphic(_MENU_ADJUST_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜後台調帳＞ */
			inDisplay = VS_TRUE;
			break;
		case	_CUP_SALE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯一般交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_CUP_VOID_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯取消＞ */
			inDisplay = VS_TRUE;
			break;
		case	_CUP_REFUND_:
		case	_CUP_MAIL_ORDER_REFUND_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_CUP_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜銀聯退貨> */
			inDisplay = VS_TRUE;
			break;
		case	_CUP_PRE_AUTH_VOID_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_AUTH_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權取消＞ */
			inDisplay = VS_TRUE;
			break;
		case	_PRE_COMP_:
		case	_CUP_PRE_COMP_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權完成＞ */
			inDisplay = VS_TRUE;
			break;
		case	_CUP_PRE_COMP_VOID_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_PRE_COMP_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜預先授權完成取消＞ */
			inDisplay = VS_TRUE;
			break;
		case	_FISC_SALE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_FISC_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款＞ */
			inDisplay = VS_TRUE;
			break;
		case	_FISC_VOID_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_FISC_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜消費扣款沖正＞ */
			inDisplay = VS_TRUE;
			break;
		case	_FISC_REFUND_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_FISC_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜退費交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_LOYALTY_REDEEM_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_LOYALTY_REDEEM_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜優惠兌換＞ */
			inDisplay = VS_TRUE;
			break;
		case	_VOID_LOYALTY_REDEEM_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_VOID_LOYALTY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜兌換取消＞ */
			inDisplay = VS_TRUE;
			break;
		case	_MAIL_ORDER_:
		case	_CUP_MAIL_ORDER_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_MAIL_ORDER_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜郵購交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_CASH_ADVANCE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
					/* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
					inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);                        
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("預借現金", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDisplay = VS_TRUE;
			break;
		case	_FORCE_CASH_ADVANCE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
                        /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("交易補登", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDisplay = VS_TRUE;
			break;
		default :
			break;
	}
	
	/* 票證相關*/
	switch (pobTran->inTransactionCode)
	{
		case	_TICKET_IPASS_LOGON_:
		case	_TICKET_IPASS_REGISTER_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_NEWUI_MENE_PAGE_4_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜電子票證＞ */
			inDisplay = VS_TRUE;
			break;
		case	_TICKET_IPASS_DEDUCT_:
		case	_TICKET_EASYCARD_DEDUCT_:
		case	_TICKET_ICASH_DEDUCT_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_DEDUCT_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜購貨交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_TICKET_IPASS_REFUND_:
		case	_TICKET_EASYCARD_REFUND_:
		case	_TICKET_ICASH_REFUND_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_REFUND_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜退貨交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_TICKET_IPASS_INQUIRY_:
		case	_TICKET_EASYCARD_INQUIRY_:
		case	_TICKET_ICASH_INQUIRY_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_INQUIRY_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜餘額查詢＞ */
			inDisplay = VS_TRUE;
			break;
		case	_TICKET_IPASS_TOP_UP_:
		case	_TICKET_EASYCARD_TOP_UP_:
		case	_TICKET_ICASH_TOP_UP_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜加值交易＞ */
			inDisplay = VS_TRUE;
			break;
		case	_TICKET_IPASS_VOID_TOP_UP_:
		case	_TICKET_EASYCARD_VOID_TOP_UP_:
		case	_TICKET_ICASH_VOID_TOP_UP_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
			inDISP_PutGraphic(_MENU_TICKET_VOID_TOP_UP_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜加值取消＞ */
			inDisplay = VS_TRUE;
			break;
		default :
			break;
	}
	
	/* 電子錢包相關 */
	switch (pobTran->inTransactionCode)
	{
		case	_EW_SALE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                        /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("電子錢包", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
			inDisplay = VS_TRUE;
			break;
		case	_EW_REFUND_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                        /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("電子錢包", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
			inDisplay = VS_TRUE;
			break;
		case	_EW_INQUIRY_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                        /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("電子錢包", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
			inDisplay = VS_TRUE;
			break;
		default :
			break;
	}
	
	/* 信託交易 */
	switch (pobTran->inTransactionCode)
	{
		case	_TRUST_SALE_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                        /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			if (strlen(pobTran->srBRec.szAuthCode) > 0)
			{
				if (!memcmp(pobTran->srBRec.szAuthCode, "S000", 4))
				{
					/* 信託交易完成 */
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託交易完成", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
				}
				else
				{
					/* 信託交易失敗 */
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託交易失敗", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
				}
			}
			else
			{
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託交易", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
			}
			inDisplay = VS_TRUE;
			break;
		case	_TRUST_VOID_:
			inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
                        /* [113110-信託需求][UI] 修改抬頭黑底長度不夠問題,加入 inDISP_Display_Black_Back()  2025/11/20 */
                        inDISP_Display_Black_Back(0, _COORDINATE_Y_LINE_8_3_, _LCD_XSIZE_, _LCD_YSIZE_/ 8);
			inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("                ", _FONTSIZE_8X16_, _COLOR_BLACK_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);
			if (strlen(pobTran->srBRec.szAuthCode) > 0)
			{
				if (!memcmp(pobTran->srBRec.szAuthCode, "S000", 4))
				{
					/* 信託交易完成 */
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託交易取消完成", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
				}
				else
				{
					/* 信託交易失敗 */
					inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託交易取消失敗", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
				}
			}
			else
			{
				inDISP_ChineseFont_Point_Color_By_Graphic_Mode_Aligned("信託交易取消", _FONTSIZE_8X16_, _COLOR_WHITE_, _COLOR_BLACK_, _COORDINATE_Y_LINE_8_3_, VS_FALSE, _DISP_CENTER_);		/* 第三層顯示 ＜電子錢包＞ */
			}
			inDisplay = VS_TRUE;
			break;
		default :
			break;
	}
                 
        /* 如果真的找不到至少可以顯示一般交易 */
	if (inDisplay == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			char	szDebugMsg[100 + 1];

			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Reset Title Fail");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SALE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般交易＞ */
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inFunc_EditPWD_Flow
Date&Time       :2016/11/25 下午 2:03
Describe        :功能6的設定管理號碼
*/
int inFunc_EditPWD_Flow(void)
{
	int	inRetVal = VS_SUCCESS;
	int	inChoice = 0;
	int	inTouchSensorFunc = _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_;
	char	szKey = 0x00;
	char	szCustomerIndicator[3 + 1] = {0};
        
        if (inLoadPWDRec(0) < 0)
        {
                /* 此功能已關閉 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_FUNC_CLOSE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
                return (VS_ERROR);
        }
	
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
        inDISP_ClearAll();
	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
        inDISP_PutGraphic(_MENU_SET_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 設定管理號碼 */
        inDISP_PutGraphic(_FUNC6_PWD_MENU_, 0, _COORDINATE_Y_LINE_8_4_);
        inDISP_BEEP(1, 0);
        
	/* (需求單-112222)-IKEA客製化新增功能需求 by Russell 2023/9/8 下午 4:25 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_123_IKEA_SELECT_MENU_TIMEOUT_);
	}
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _CUSTOMER_124_EVER_RICH_SELECT_MENU_TIMEOUT_);
	}
	else
	{
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
	}
        while (1)
        {
                inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
		szKey = uszKBD_Key();

		/* Timeout */
		if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			szKey = _KEY_TIMEOUT_;
		}
                
		/* 變更管理號碼 */
                if (szKey == _KEY_1_			||
		    inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_)
                {
			inRetVal = inFunc_Edit_Manager_Pwd();
			break;
                }
		/* 交易功能管理 */
                else if (szKey == _KEY_2_		||
			 inChoice == _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_)
                {
			inRetVal = inFunc_Edit_TransFuc_Pwd();
			break;
                }
                else if (szKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (szKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
        }
	/* 清空Touch資料 */
	inDisTouch_Flush_TouchFile();
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_Edit_Manager_Pwd
Date&Time       :2017/8/25 下午 5:24
Describe        :
*/
int inFunc_Edit_Manager_Pwd()
{
	int		inRetVal = VS_SUCCESS;
	char		szPWDOld[4 + 1], szPWDNew[4 + 1];
	char		szInitialManagerEnable[2 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	DISPLAY_OBJECT  srDispObj;
	
	memset(szInitialManagerEnable, 0x00, sizeof(szInitialManagerEnable));
        inGetInitialManagerEnable(szInitialManagerEnable);
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 變更管理者號碼 */
	if (memcmp(szInitialManagerEnable, "Y", 1) != 0)
	{
		/* 此功能已關閉 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_FUNC_CLOSE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		}
		else
		{
			srDispMsgObj.inTimeout = 30;
		}
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		return (VS_ERROR);
	}
	
	/* 請輸入預設管理者號碼 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_GET_MANAGER_ORG_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
	memset(szPWDOld, 0x00, sizeof(szPWDOld));
	inGetInitialManagerPwd(szPWDOld);

	while (1)
	{
		inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

		/* 設定顯示變數 */
		srDispObj.inMaxLen = 4;
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (srDispObj.inOutputLen == 4)
			break;
		else 
			continue;
	}

	if (!memcmp(&srDispObj.szOutput[0], &szPWDOld[0], 4))
	{
		/* 輸入正確 */
	}
	else
	{
		/* 輸入錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
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

	while (1)
	{
		/* 原管理者號碼  新管理者號碼? */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_SET_NEW_MANAGER_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		/* 提示原始密碼 */
		inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

		while (1)
		{
			inDISP_BEEP(1, 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

			/* 設定顯示變數 */
			srDispObj.inMaxLen = 4;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inMask = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (srDispObj.inOutputLen == 4)
				break;
			else 
				continue;
		}

		memset(szPWDNew, 0x00, sizeof(szPWDNew));
		memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

		/* 請再次輸入新管理者號碼 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_SET_MANAGER_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

		while (1)
		{
			inDISP_BEEP(1, 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

			/* 設定顯示變數 */
			srDispObj.inMaxLen = 4;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inMask = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;
			
			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (srDispObj.inOutputLen == 4)
				break;
			else 
				continue;
		}

		if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
		{
			/* 修改成功 圖片 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
			/* 儲存管理者號碼 */
			inSetInitialManagerPwd(szPWDNew);
			inSavePWDRec(0);

			return (VS_SUCCESS);
		}
		else
		{
			/* 輸入錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;
			
			inDISP_Msg_BMP(&srDispMsgObj);
			continue;
		}
	}
	
	return (VS_SUCCESS);
}

int inFunc_Edit_TransFuc_Pwd()
{
	int		inRetVal = VS_SUCCESS;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_CHECK_PWD_EDIT_;
	char		szPWDOld[4 + 1], szPWDNew[4 + 1];
        char		szManagerPWD[4 + 1];
        char		szKey = 0x00;
	char		szInitialManagerEnable[2 + 1];
        char		szRebootPwdEnale[2 + 1], 
			szSalePwdEnable[2 + 1], 
			szPreauthPwdEnable[2 + 1], 
			szInstallmentPwdEnable[2 + 1], 
			szRedeemPwdEnable[2 + 1],
			szOfflinePwdEnable[2 + 1], 
			szInstallmentAdjustPwdEnable[2 + 1], 
			szRedeemAdjustPwdEnable[2 + 1], 
			szVoidPwdEnable[2 + 1],
			szSettlementPwdEnable[2 + 1], 
			szRefundPwdEnable[2 + 1],
			szCallBankForcePwdEnable[2 + 1];
	char		szCustomerIndicator[3 + 1] = {0};
	VS_BOOL		fEditPWD = VS_FALSE;
        DISPLAY_OBJECT  srDispObj;
	
	/* Get 各交易密碼開關 */
        memset(szInitialManagerEnable, 0x00, sizeof(szInitialManagerEnable));
        inGetInitialManagerEnable(szInitialManagerEnable);
        memset(szRebootPwdEnale, 0x00, sizeof(szRebootPwdEnale));
        inGetRebootPwdEnale(szRebootPwdEnale);
        memset(szSalePwdEnable, 0x00, sizeof(szSalePwdEnable));
        inGetSalePwdEnable(szSalePwdEnable);
        memset(szPreauthPwdEnable, 0x00, sizeof(szPreauthPwdEnable));
        inGetPreauthPwdEnable(szPreauthPwdEnable);
        memset(szInstallmentPwdEnable, 0x00, sizeof(szInstallmentPwdEnable));
        inGetInstallmentPwdEnable(szInstallmentPwdEnable);
        memset(szRedeemPwdEnable, 0x00, sizeof(szRedeemPwdEnable));
        inGetRedeemPwdEnable(szRedeemPwdEnable);
        memset(szOfflinePwdEnable, 0x00, sizeof(szOfflinePwdEnable));
        inGetOfflinePwdEnable(szOfflinePwdEnable);
        memset(szInstallmentAdjustPwdEnable, 0x00, sizeof(szInstallmentAdjustPwdEnable));
        inGetInstallmentAdjustPwdEnable(szInstallmentAdjustPwdEnable);
        memset(szRedeemAdjustPwdEnable, 0x00, sizeof(szRedeemAdjustPwdEnable));
        inGetRedeemAdjustPwdEnable(szRedeemAdjustPwdEnable);
        memset(szVoidPwdEnable, 0x00, sizeof(szVoidPwdEnable));
        inGetVoidPwdEnable(szVoidPwdEnable);
        memset(szSettlementPwdEnable, 0x00, sizeof(szSettlementPwdEnable));
        inGetSettlementPwdEnable(szSettlementPwdEnable);
        memset(szRefundPwdEnable, 0x00, sizeof(szRefundPwdEnable));
        inGetRefundPwdEnable(szRefundPwdEnable);
	memset(szCallBankForcePwdEnable, 0x00, sizeof(szCallBankForcePwdEnable));
        inGetCallBankForcePwdEnable(szCallBankForcePwdEnable);
	memset(szCustomerIndicator, 0x00 , sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (memcmp(szInitialManagerEnable, "Y", 1) != 0)
	{
		/* 此功能已關閉 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_FUNC_CLOSE_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		}
		else
		{
			srDispMsgObj.inTimeout = 30;
		}
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		return (VS_ERROR);
	}
	
	/* 請輸入管理者號碼 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_GET_MANAGER_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
	memset(szManagerPWD, 0x00, sizeof(szManagerPWD));
	inGetInitialManagerPwd(szManagerPWD);

	while (1)
	{
		inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

		/* 設定顯示變數 */
		srDispObj.inMaxLen = 4;
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (srDispObj.inOutputLen == 4)
			break;
		else 
			continue;
	}

	if (!memcmp(&srDispObj.szOutput[0], &szManagerPWD[0], 4))
	{
		 /* 輸入正確 */
	}
	else
	{
		/* 輸入錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
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

	if (!memcmp(&szSalePwdEnable[0], "Y", 1) ||
	    !memcmp(&szPreauthPwdEnable[0], "Y", 1) ||
	    !memcmp(&szInstallmentPwdEnable[0], "Y", 1) ||
	    !memcmp(&szRedeemPwdEnable[0], "Y", 1))
	{
		/* 正項交易 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SALE_PWD_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 正向交易 */
		/* 按0修改按確認跳下一步 */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inTouchSensorFunc = _Touch_CHECK_PWD_EDIT_;
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_ERROR;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改正項交易密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetSalePwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;
					
					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;

					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					/* 儲存管理號碼 */
					inSetSalePwd(szPWDNew);
					inSetPreauthPwd(szPWDNew);
					inSetInstallmentPwd(szPWDNew);
					inSetRedeemPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szRefundPwdEnable[0], "Y", 1))
	{
		/* 退貨交易 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜一般退貨＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				fEditPWD = VS_TRUE;
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				fEditPWD = VS_FALSE;
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改退貨交易密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetRefundPwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					/* 儲存管理號碼 */
					inSetRefundPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szVoidPwdEnable[0], "Y", 1))
	{
		/* 取消交易 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_VOID_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 ＜取消交易＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改取消交易密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetVoidPwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					/* 儲存管理號碼 */
					inSetVoidPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szOfflinePwdEnable[0], "Y", 1) ||
	    !memcmp(&szInstallmentAdjustPwdEnable[0], "Y", 1) ||
	    !memcmp(&szRedeemAdjustPwdEnable[0], "Y", 1))
	{
		/* 交易補登 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SALE_OFFLINE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜交易補登＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改交易補登密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetOfflinePwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					/* 儲存管理號碼 */
					inSetOfflinePwd(szPWDNew);
					inSetInstallmentAdjustPwd(szPWDNew);
					inSetRedeemAdjustPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}
	
	if (!memcmp(&szCallBankForcePwdEnable[0], "Y", 1))
	{
		/* 交易補登 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_CALL_BANK_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);	/* 第三層顯示 ＜授權碼補登＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改授權碼補登密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetCallBankForcePwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					/* 儲存管理號碼 */
					inSetOfflinePwd(szPWDNew);
					inSetInstallmentAdjustPwd(szPWDNew);
					inSetRedeemAdjustPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szSettlementPwdEnable[0], "Y", 1))
	{
		/* 結帳交易 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);			/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_SETTLE_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);			/* 第三層顯示 ＜結帳交易＞ */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改結帳交易密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetSettlementPwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
					
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					/* 儲存管理號碼 */
					inSetSettlementPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}

	if (!memcmp(&szRebootPwdEnale[0], "Y", 1))
	{
		/* 開機密碼 */
		inDISP_ClearAll();
		inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);		/* 第一層顯示 LOGO */
		inDISP_PutGraphic(_MENU_POWER_ON_TITLE_, 0,  _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 開機 */
		inDISP_PutGraphic(_CHECK_EDIT_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, 30);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_0_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_TRUE;
				break;
			}
			else if (szKey == _KEY_ENTER_			||
				 inChoice == _CHECKEditPWD_Touch_ENTER_)
			{
				inRetVal = VS_SUCCESS;
				fEditPWD = VS_FALSE;
				break;
			}
			else if (szKey == _KEY_CANCEL_ || szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_ERROR;
			}
		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();

		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

		/* 修改開機密碼 */
		if (fEditPWD == VS_TRUE)
		{
			memset(szPWDOld, 0x00, sizeof(szPWDOld));
			inGetRebootPwd(szPWDOld);

			while (1)
			{
				/* 原管理號碼  新管理號碼? */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_, 0, _COORDINATE_Y_LINE_8_4_);
				/* 提示原始密碼 */
				inDISP_EnglishFont(szPWDOld, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				memset(szPWDNew, 0x00, sizeof(szPWDNew));
				memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

				/* 請再次輸入新管理號碼 */
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_PutGraphic(_SET_NEW_TXN_PWD_AGAIN_, 0, _COORDINATE_Y_LINE_8_4_);

				while (1)
				{
					inDISP_BEEP(1, 0);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

					/* 設定顯示變數 */
					srDispObj.inMaxLen = 4;
					srDispObj.inY = _LINE_8_7_;
					srDispObj.inR_L = _DISP_RIGHT_;
					srDispObj.inMask = VS_TRUE;
					srDispObj.inColor = _COLOR_RED_;

					memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
					srDispObj.inOutputLen = 0;
	
					inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
					if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
						return (VS_ERROR);

					if (srDispObj.inOutputLen == 4)
						break;
					else 
						continue;
				}

				if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
				{
					/* 修改成功 圖片 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					/* 儲存管理號碼 */
					inSetRebootPwd(szPWDNew);
					inSavePWDRec(0);

					break;
				}
				else
				{
					/* 輸入錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _NO_KEY_MSG_;
					srDispMsgObj.inTimeout = 0;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 3;
					srDispMsgObj.inBeepInterval = 1000;
			
					inDISP_Msg_BMP(&srDispMsgObj);
					continue;
				}
			}
		} /* if (Edit == true) end */
	}
	return (VS_SUCCESS);
}

/*
Function        :inFunc_ReviewReport
Date&Time       :2016/2/24 上午 9:33
Describe        :交易查詢使用
*/
int inFunc_ReviewReport(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_ReviewReport START!");
        
	/* inACCUM_ReviewReport_Total */
	if (inFLOW_RunFunction(pobTran, _FUNCTION_TOTAL_REVIEW_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* inBATCH_ReviewReport_Detail_Flow_By_Sqlite */
	if (inFLOW_RunFunction(pobTran, _FUNCTION_DETAIL_REVIEW_) != VS_SUCCESS)
	{
		inBATCH_CheckReport_By_Sqlite(pobTran);
		
		return (VS_ERROR);
	}
	
	
	return	(VS_SUCCESS);
}

/*
Function        :inFunc_ReviewReport
Date&Time       :2016/2/24 上午 9:33
Describe        :交易查詢使用
*/
int inFunc_ReviewReport_NEWUI(TRANSACTION_OBJECT *pobTran)
{
	/* inACCUM_ReviewReport_Total */
	if (inFLOW_RunFunction(pobTran, _FUNCTION_TOTAL_REVIEW_) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	/* inBATCH_ReviewReport_Detail_Flow_By_Sqlite */
	if (inFLOW_RunFunction(pobTran, _FUNCTION_DETAIL_REVIEW_NEWUI_) != VS_SUCCESS)
	{
		inBATCH_CheckReport_By_Sqlite(pobTran);
		
		return (VS_ERROR);
	}
	
	
	return	(VS_SUCCESS);
}

/*
Function        :inFun_CheckPAN_EXP
Date&Time       :2016/3/1 上午 10:32
Describe        :確認卡號及有效期，相當於Verifone inNCCC_FuncDisplaySwipeCardInfo
*/
int inFunc_CheckPAN_EXP(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg [40 + 1] = {0};
	char		szFinalPAN[_PAN_UCARD_SIZE_ + 1] = {0};
	char		szFESMode[2 + 1] = {0};
	char		szTemplate[20 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
	unsigned char	uszKey = 0x00;
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckPAN_EXP START!");
	
	memset(szFESMode, 0x00, sizeof(szFESMode));
	inGetNCCCFESMode(szFESMode);
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 晶片卡不需要顯示有效期 */
        /* 感應卡不需要顯示有效期 */
        /* HGI卡不需要顯示有效期，跳過第二次讀取信用卡流程 */
        /* 一般HG交易不需要顯示有效期，跳過第二次讀取信用卡流程 */
        /* HG卡不需要顯示有效期 */
	/* MPAS刷卡不顯示有效期 */
	/* 票證無法顯示有效期 */
	/* 第二段收銀機連線不用顯示 */
	/* (需求單 - 107276)自助交易標準做法 使用磁條卡進行交易，不需提示核對卡號及有效期畫面。 by Russell 2019/3/6 上午 10:32 */
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
	/* 客製化005且用ECR傳送卡號的情況 */
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_					|| 
	    pobTran->srBRec.uszFiscTransBit == VS_TRUE					||
            pobTran->srBRec.uszContactlessBit == VS_TRUE				||
	    pobTran->srBRec.uszRefundCTLSBit == VS_TRUE					||
            pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_			||
            pobTran->srBRec.uszHappyGoSingle == VS_TRUE					||
            memcmp(pobTran->srBRec.szPAN, "9552", 4) == 0				||
	    memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) == 0	||
	    pobTran->srTRec.uszESVCTransBit == VS_TRUE					||
	    pobTran->uszCardInquirysSecondBit == VS_TRUE				||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
	    pobTran->srBRec.uszUnyTransBit == VS_TRUE					||
	    (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_005_FPG_, _CUSTOMER_INDICATOR_SIZE_) && pobTran->srBRec.uszManualBit == VS_TRUE)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_) ||
            pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckPAN_EXP END!");
                
		return (VS_SUCCESS);
	}
	/* 顯示檢查碼 */
	else if (pobTran->uszInputCheckNoBit == VS_TRUE)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_CHECK_PAN_EXP_3_, 0, _COORDINATE_Y_LINE_8_4_);
		/* Disp Card Number */
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);

		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		sprintf(szDispMsg, " %s",szFinalPAN);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

		/* Disp CheckNo */
		if (strlen(pobTran->srBRec.szExpDate) > 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			if (strlen(pobTran->srBRec.szCheckNO) > 0)
			{
				strcpy(szTemplate, pobTran->srBRec.szCheckNO);
			}
			else
			{
				inCARD_ExpDateEncryptAndDecrypt(pobTran, "", szTemplate, _EXP_ENCRYPT_);
			}
			
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " Check No.%s", szTemplate);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
		}
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_CHECK_PAN_EXP_, 0, _COORDINATE_Y_LINE_8_4_);
		/* Disp Card Number */
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
		strcpy(szFinalPAN, pobTran->srBRec.szPAN);

		/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
		sprintf(szDispMsg, " %s",szFinalPAN);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

		/* Disp Card Label */
		if (strlen(pobTran->srBRec.szExpDate) > 0)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, " MM/'YY = %.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
		}
	}
	
	uszKey = 0x00;
	while (1)
	{
		uszKey = uszKBD_GetKey(_ECR_RS232_GET_CARD_TIMEOUT_);

		if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
                        vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckPAN_EXP cancel or timeout");
                        
			return (VS_ERROR);
		}
		else if (uszKey == _KEY_ENTER_)
		{
			break;
		}

	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckPAN_EXP END!");
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckPAN_EXP_Loyalty_Redeem
Date&Time       :2017/2/3 下午 1:13
Describe        :確認卡號及有效期，優惠兌換用，請核對輸入資訊
*/
int inFunc_CheckPAN_EXP_Loyalty_Redeem(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg [40 + 1];
	char		szFinalPAN[_PAN_UCARD_SIZE_ + 1];
	unsigned char	uszKey;
	
	vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckPAN_EXP_Loyalty_Redeem START!");
	
	/* 晶片卡不需要顯示有效期 */
	/* 第二段收銀機連線不用顯示 */
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_	|| 
	    pobTran->srBRec.uszFiscTransBit == VS_TRUE	|| 
	    pobTran->srBRec.uszContactlessBit == VS_TRUE||
	    pobTran->uszCardInquirysSecondBit == VS_TRUE)
	{
		return (VS_SUCCESS);
	}
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_PAN_EXP_2_, 0, _COORDINATE_Y_LINE_8_4_);
	/* Disp Card Number */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	memset(szFinalPAN, 0x00, sizeof(szFinalPAN));
	strcpy(szFinalPAN, pobTran->srBRec.szPAN);
	
	/* (需求單 - 106349)移除原U CARD邏輯判斷 2018/10/31 下午 3:41 by Russell */
	sprintf(szDispMsg, " %s",szFinalPAN);
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

	/* Disp Card Label */
	if (strlen(pobTran->srBRec.szExpDate) > 0)
	{
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, " MM/'YY = %.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	}
	
	uszKey = 0x00;
	while (1)
	{
		uszKey = uszKBD_GetKey(_ECR_RS232_GET_CARD_TIMEOUT_);

		if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else if (uszKey == _KEY_ENTER_)
		{
			break;
		}

	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckBarcode_Loyalty_Redeem
Date&Time       :2017/2/20 上午 10:06
Describe        :確認優惠兌換條碼，優惠兌換用，請核對輸入資訊
*/
int inFunc_CheckBarcode_Loyalty_Redeem(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg [40 + 1];
	unsigned char	uszKey;
	
	vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckBarcode_Loyalty_Redeem START!");
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_PAN_EXP_2_, 0, _COORDINATE_Y_LINE_8_4_);
	/* Disp Barcode1 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%s",pobTran->szL3_Barcode1);
	inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_RED_, _DISP_LEFT_);

	/* Disp Barcode2 */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "%s", pobTran->szL3_Barcode2);
	inDISP_EnglishFont_Color(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _COLOR_RED_, _DISP_LEFT_);
	
	uszKey = 0x00;
	while (1)
	{
		uszKey = uszKBD_GetKey(_ECR_RS232_GET_CARD_TIMEOUT_);

		if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else if (uszKey == _KEY_ENTER_)
		{
			break;
		}

	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckHGPAN_EXP
Date&Time       :2017/2/21 下午 5:09
Describe        :確認HG卡號及有效期
*/
int inFunc_CheckHGPAN_EXP(TRANSACTION_OBJECT *pobTran)
{
	char		szDispMsg [26 + 1];
	unsigned char	uszKey;
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_CheckHGPAN_EXP START!");
	
	/* 晶片卡不需要顯示有效期 */
        /* 感應卡不需要顯示有效期 */
        /* HGI卡不需要顯示有效期，跳過第二次讀取信用卡流程 */
        /* HG卡不需要顯示有效期 */
	if (pobTran->srBRec.inChipStatus == _EMV_CARD_ || 
            !memcmp(&pobTran->srBRec.szPAN[0], "9552", 4))
	{
		return (VS_SUCCESS);
	}
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_PAN_EXP_, 0, _COORDINATE_Y_LINE_8_4_);
	/* Disp Card Number */
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, " %s",pobTran->srBRec.szPAN);
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);

	/* Disp Card Label */
	if (strlen(pobTran->srBRec.szExpDate) > 0)
	{
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, " MM/'YY = %.2s/'%.2s", pobTran->srBRec.szExpDate + 2, pobTran->srBRec.szExpDate);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	}
	
	uszKey = 0x00;
	while (1)
	{
		uszKey = uszKBD_GetKey(_ECR_RS232_GET_CARD_TIMEOUT_);

		if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else if (uszKey == _KEY_ENTER_)
		{
			break;
		}

	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Display_Initial
Date&Time       :2017/10/2 下午 5:48
Describe        :開機流程Display初始化
*/
int inFunc_Booting_Flow_Display_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Display_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Display_Initial() START !");
	}
	
	inRetVal = inDISP_Initial();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Display_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Print_Initial
Date&Time       :2017/10/2 下午 5:48
Describe        :開機流程Print初始化
*/
int inFunc_Booting_Flow_Print_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szSignPadMode[2 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Print_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Print_Initial() START !");
	}
	
	/* 是否有列印功能或有內建簽名板功能(文字圖片轉向要initial) */
	inLoadCFGTRec(0);
	memset(szSignPadMode, 0x00, sizeof(szSignPadMode));
	inGetSignPadMode(szSignPadMode);
	if (inFunc_Check_Print_Capability(ginMachineType) == VS_SUCCESS)
	{
		/* 列印參數初始化 */
		inRetVal = inPRINT_Initial();
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}

		inRetVal = inPRINT_TTF_SetFont(_PRT_CHINESE_1_);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}
		
		inRetVal = inPRINT_TTF_SetFont_Style(gusPrintFontStyleRegular);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}
	}
	/* for 不支援印表機，但需要文字轉圖片的V3P使用 */
	else if (memcmp(szSignPadMode, _SIGNPAD_MODE_1_INTERNAL_, strlen(_SIGNPAD_MODE_1_INTERNAL_)) == 0)
	{
		vdCTOSS_PrinterTTFSelect((unsigned char*)_PRT_CHINESE_FONE_1_, 0);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Print_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Print_Image_Initial
Date&Time       :2018/6/5 下午 6:27
Describe        :開機流程列印圖片初始化，為了避免開機更新導致抓不到圖片高度
*/
int inFunc_Booting_Flow_Print_Image_Initial(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Print_Image_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Print_Image_Initial() START !");
	}
	
	/* 若沒下TMS會沒圖片抓高度，所以不判斷成功或失敗 */
	inPRINT_Buffer_GetHeightFlow();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Print_Image_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Castle_library_Initial
Date&Time       :2017/10/2 下午 5:48
Describe        :開機流程虹堡library初始化，虹堡有一些library要開機初始化
*/
int inFunc_Booting_Flow_Castle_library_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Castle_library_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Castle_library_Initial() START !");
	}
	
	/* KMS library initial */
	inRetVal = inKMS_Initial();
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* TLS library initial*/
	inRetVal = inTLS_Init();
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Castle_library_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Update_Parameter
Date&Time       :2018/6/4 下午 8:45
Describe        :要更新就直接重開機，到這一步驟時就更新
*/
int inFunc_Booting_Flow_Update_Parameter(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	char		szCloseBatchBit[2 + 1] = {0};
	char		szTemplate[2 + 1] = {0};
	char		szCFESMode[2 + 1] = {0};
	unsigned char	uszUpdateBit = VS_FALSE;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Update_Parameter START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Update_Parameter() START !");
	}
	
	/* 這段程式碼沒用到，inNCCCTMS_Check_FileList_Flow裡面會重新選 Start */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSDownloadMode(szTemplate);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	if (memcmp(szTemplate, _TMS_DLMODE_FTPS_, strlen(_TMS_DLMODE_FTPS_)) == 0)
	{	
		if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
		{
			pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_SFTP_;
		}
		else
		{
			pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_FTPS_;
		}
	}
	else if (memcmp(szTemplate, _TMS_DLMODE_ISO8583_, strlen(_TMS_DLMODE_ISO8583_)) == 0)
	{
		pobTran->uszFTP_TMS_Download = _TMS_DOWNLOAD_SECURE_MODE_NONE_;
	}
	/* End */
	
	/* TMS參數生效 */
	/* 代表有參數列表，有機會更新，接著檢查時間 */
	if (inNCCCTMS_Check_FileList_Flow(pobTran) == VS_SUCCESS)
	{
		if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_SFTP_)
		{
			vdUtility_SYSFIN_LogMessage(AT, "_TMS_DOWNLOAD_SECURE_MODE_SFTP_");
		}
		else if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_FTPS_)
		{
			vdUtility_SYSFIN_LogMessage(AT, "_TMS_DOWNLOAD_SECURE_MODE_FTPS_");
		}
		else
		{
			vdUtility_SYSFIN_LogMessage(AT, "_TMS_DOWNLOAD_SECURE_MODE_NONE_");
		}
		
		/* ISO8583 */
		if (pobTran->uszFTP_TMS_Download == _TMS_DOWNLOAD_SECURE_MODE_NONE_)
		{
			/* 有File List檔案代表有參數要更新 */
			if (inNCCCTMS_Schedule_Effective_Date_Time_Check(pobTran) == VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "已到生效時間");
				
				/* 檢查時間到時，檢查FileList 及 下載檔案是否合法 */
				if (inNCCCTMS_CheckAllDownloadFile_Flow(pobTran) != VS_SUCCESS)
				{
					vdUtility_SYSFIN_LogMessage(AT, "下載檔案不合法");
					return (VS_SUCCESS);
				}
				
				/* 檢查是否有帳 */
				inLoadTMSCPTRec(0);
				inGetTMSEffectiveCloseBatch(szCloseBatchBit);
				if (memcmp(szCloseBatchBit, "Y", strlen("Y")) == 0)
				{
					if (inFLOW_RunFunction(pobTran, _FUNCTION_All_HOST_MUST_SETTLE_) != VS_SUCCESS)
					{
						vdUtility_SYSFIN_LogMessage(AT, "尚未結帳，無法更新");
						return (VS_SUCCESS);
					}
				}

				uszUpdateBit = VS_TRUE;
			}
		}
		/* FTPS */
		else
		{
			/* FTPS不檢查時間，有下就等到開機再更新 */
			/* 檢查時間到時，檢查FileList 及 下載檔案是否合法 */
			if (inNCCCTMS_CheckAllDownloadFile_Flow(pobTran) != VS_SUCCESS)
			{
				vdUtility_SYSFIN_LogMessage(AT, "下載檔案不合法");
				return (VS_SUCCESS);
			}
			
			/* 檢查是否有帳 */
			inLoadTMSFTPRec(0);
			inGetFTPEffectiveCloseBatch(szCloseBatchBit);
			if (memcmp(szCloseBatchBit, "Y", strlen("Y")) == 0)
			{
				if (inFLOW_RunFunction(pobTran, _FUNCTION_All_HOST_MUST_SETTLE_) != VS_SUCCESS)
				{	
					vdUtility_SYSFIN_LogMessage(AT, "尚未結帳，無法更新");
					return (VS_SUCCESS);
				}
			}
				
			/* 會走到這邊代表已經有檔案了 */
			uszUpdateBit = VS_TRUE;
		}
	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT, "無FileList需更新");
		return (VS_SUCCESS);
	}
	
	/* 進行更新 */
	if (uszUpdateBit == VS_TRUE)
	{
		/* 這邊比Load Table還前面，所以先讀 */
		inLoadEDCRec(0);
		inRetVal = inNCCCTMS_UpdateParam(pobTran);
		if (inRetVal == VS_SUCCESS)
		{
			inNCCCTMS_Deal_With_Things_After_Parameter_Update(pobTran);
			vdUtility_SYSFIN_LogMessage(AT, "更新成功");
		}
		else
		{
			/* 更新失敗 刪除File List */
			inNCCCTMS_Delete_FileList_Flow(pobTran);
			inSetTMSOK("N");
			inSaveEDCRec(0);
			
			/* 參數生效失敗 */
			inLoadTMSFTPRec(0);
			inSetFTPDownloadResponseCode(_FTP_DOWNLOAD_REPORT_EFFECT_FAILED_);
			inSaveTMSFTPRec(0);
			
			vdUtility_SYSFIN_LogMessage(AT, "更新失敗");
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Update_Parameter() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Load_Table
Date&Time       :2017/10/2 下午 5:48
Describe        :開機流程load Table內的參數，以便執行接下來的流程
*/
int inFunc_Booting_Flow_Load_Table(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Load_Table START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Load_Table() START !");
	}
	
	/* CFGT.dat */
	inRetVal = inLoadCFGTRec(0);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Load CFGT Failed");
		}
		return (inRetVal);
	}
	
	/* EDC.dat */
	inRetVal = inLoadEDCRec(0);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Load EDC Failed");
		}
		return (inRetVal);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Load_Table() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Sync_Debug
Date&Time       :2017/10/3 上午 9:13
Describe        :開機流程同步Table內的Debug Flag
*/
int inFunc_Booting_Flow_Sync_Debug(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	i = 0;
	char	szISODebug[20 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Sync_Debug START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Sync_Debug() START !");
	}
	
	inLoadEDCRec(0);
	/* 補足isodebug內的0x00 */
	memset(szISODebug, 0x00, sizeof(szISODebug));
	inGetISODebug(szISODebug);
	for (i = 0; i >= 20; i++)
	{
		if (szISODebug[i] == 0x00)
			szISODebug[i] = '0';
	}
	inSetISODebug(szISODebug);
	inSaveEDCRec(0);
	
	/* 將ISODebug和Table同步 */
	inRetVal = inFunc_Sync_Debug_Switch();
	inRetVal = inFunc_Sync_ISODebug_Switch();
	inRetVal = inFunc_Sync_DisplayDebug_Switch();
	inRetVal = inFunc_Sync_EngineerDebug_Switch();
	inRetVal = inFunc_Sync_ESCDebug_Switch();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Sync_Debug() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Communication_Initial
Date&Time       :2017/10/3 上午 9:42
Describe        :開機流程同步Table內的Debug Flag
*/
int inFunc_Booting_Flow_Communication_Initial(TRANSACTION_OBJECT *pobTran)
{
#ifdef _COMMUNICATION_CAPBILITY_	
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Communication_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Communication_Initial() START !");
	}
	
        /* 網路初始化，根據table選擇撥接或是Ethernet的初始化 */
	inRetVal = inCOMM_InitCommDevice();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Communication_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
#endif	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_ECR_Initial
Date&Time       :2017/10/3 上午 9:42
Describe        :開機流程初始化ECR用的port
*/
int inFunc_Booting_Flow_ECR_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_ECR_Initial START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_ECR_Initial() START !");
	}
	
	if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		/* 多接設備initial */
		inRetVal = inMultiFunc_First_Initial();
	}
	else
	{
		/* ECR initial */
		inRetVal = inECR_Initial();
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_ECR_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_EMV_Initial
Date&Time       :2017/10/3 上午 10:28
Describe        :開機流程初始化EMV
*/
int inFunc_Booting_Flow_EMV_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_EMV_Initial START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_EMV_Initial() START !");
	}
	
	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		/* EMV Initial */
		inRetVal = inEMV_Initial();
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_EMV_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_TMSOK_Check_Initial
Date&Time       :2017/10/3 上午 11:05
Describe        :開機流程確認是否有下TMS或鎖機，只有這個開機流程會回傳錯誤，只要在這流程後的都可以視為一定要下TMS才能執行
*/
int inFunc_Booting_Flow_TMSOK_Check_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inOrgIndex = 0;
	char	szTMSOK[2 + 1] = {0};
	char	szDemoMode[2 + 1] = {0};
	char	szHostEnable[2 + 1];
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_TMSOK_Check_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_TMSOK_Check_Initial() START !");
	}
	
	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		/* DEMO票證餘額 2018/12/4 上午 10:08 by Russell */
		memset(szDemoMode, 0x00, sizeof(szDemoMode));
		inGetDemoMode(szDemoMode);
		if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
		{
			gulDemoHappyGoPoint = 100000;
			gulDemoRedemptionPointsBalance = 100000;
			gulDemoTicketPoint = 499;
		
			/* DEMO要先抓DCC有沒有開 不然IDLE UI提示錯誤 */
			memset(szHostEnable, 0x00, sizeof(szHostEnable));
			inNCCC_DCC_GetDCC_Enable(inOrgIndex, szHostEnable);
		}
	}
	else
	{
		inRetVal = VS_ERROR;
		vdUtility_SYSFIN_LogMessage(AT, "TMSOK != Y");
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_TMSOK_Check_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_Booting_Flow_Eticket_Initial
Date&Time       :2017/10/3 上午 11:05
Describe        :開機流程票證初始化
*/
int inFunc_Booting_Flow_Eticket_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1] = {0};
	char	szCTLSEnable[2 + 1] = {0};		/* 觸控是否打開 */
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Eticket_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Eticket_Initial() START !");
	}
	
	inDISP_ClearAll();
	inFunc_Display_LOGO(0, _COORDINATE_Y_LINE_16_2_);
	inDISP_PutGraphic(_MENU_LOGON_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);
	
	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	/* 判斷是否有開感應 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		/* 目前裡面不設定網路，由端末機控制 */
		/* 若找不到該票證的SAM卡，則不能感應 */
		
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);
		if (!memcmp(&szCTLSEnable[0], "Y", 1) && (inMENU_Check_ETICKET_Enable(0) == VS_SUCCESS))
		{
			inRetVal = inNCCC_Ticket_Init_Logon_Flow(pobTran);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Eticket_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_CTLS_Initial
Date&Time       :2017/10/3 上午 11:05
Describe        :開機流程感應初始化
*/
int inFunc_Booting_Flow_CTLS_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1];
	char	szCTLSEnable[2 + 1];		/* 觸控是否打開 */
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_CTLS_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_CTLS_Initial() START !");
	}
	
	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	/* 判斷是否有開感應 */
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		memset(szCTLSEnable, 0x00, sizeof(szCTLSEnable));
		inGetContactlessEnable(szCTLSEnable);
		if (!memcmp(&szCTLSEnable[0], "Y", 1))
		{
			/* Contactless initial */
			inRetVal = inCTLS_InitReader_Flow();
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_CTLS_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Sqlite_Initial
Date&Time       :2017/10/3 上午 11:05
Describe        :開機流程Sqlite初始化
*/
int inFunc_Booting_Flow_SQLite_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_SQLite_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Sqlite_Initial() START !");
	}
	
	/* SQLite Initial */
	inRetVal = inSqlite_Initial();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Sqlite_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_TSAM_Initial
Date&Time       :2017/10/3 下午 1:55
Describe        :開機流程tSAM初始化
*/
int inFunc_Booting_Flow_TSAM_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1];
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_TSAM_Initial START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_TSAM_Initial() START !");
	}
	
	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		/* tSAM initial */
		inRetVal = inNCCC_tSAM_InitialSLOT(pobTran);
	}
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_TSAM_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_TMS_Parameter_Inquire
Date&Time       :2017/10/3 下午 2:38
Describe        :開機流程TMS詢問
*/
int inFunc_Booting_Flow_TMS_Parameter_Inquire(TRANSACTION_OBJECT *pobTran)
{
#ifdef	_COMMUNICATION_CAPBILITY_	
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1] = {0};
	char    szTemplate[16 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_TMS_Parameter_Inquire START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_TMS_Parameter_Inquire() START !");
	}

	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		/* 開機TMS參數詢問 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTMSOK(szTemplate);

		if (!memcmp(&szTemplate[0], "Y", 1))
		{
			inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_SCHEDULE_INQUIRE_);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_TMS_Parameter_Inquire() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
#endif	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_TMS_DCC_Schedule
Date&Time       :2017/10/3 下午 2:59
Describe        :TMS 連動DCC下載
*/
int inFunc_Booting_Flow_TMS_DCC_Schedule(TRANSACTION_OBJECT *pobTran)
{
#ifdef	_COMMUNICATION_CAPBILITY_
	int	inRetVal = VS_SUCCESS;
	char	szTMSOK[2 + 1];
	char    szTemplate[16 + 1];
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_TMS_DCC_Schedule START!");
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_TMS_DCC_Schedule() START !");
	}
	
	/* 確認是否鎖機 */
	/* TMS有下才跑下面*/
	memset(szTMSOK, 0x00, sizeof(szTMSOK));
	inGetTMSOK(szTMSOK);
	if (memcmp(szTMSOK, "Y", 1) == 0 && inFunc_Check_EDCLock() == VS_SUCCESS)
	{
		/* TMS連動DCC下載 */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetDCCDownloadMode(szTemplate);

		if (memcmp(szTemplate, _NCCC_DCC_DOWNLOAD_MODE_NOW_, 1) == 0)
		{
			inRetVal = inEVENT_Responder(_TMS_DCC_SCHEDULE_EVENT_);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_TMS_DCC_Schedule() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
#endif	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Enter_PowerOn_Password
Date&Time       :2017/10/3 下午 3:33
Describe        :輸入開機密碼
*/
int inFunc_Booting_Flow_Enter_PowerOn_Password(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Enter_PowerOn_Password START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Enter_PowerOn_Password() START !");
	}
	
	/* 輸入開機密碼 */
        while (1)
        {
		inRetVal = inFunc_CheckCustomizePassword(_ACCESS_WITH_CUSTOM_, _POWER_ON_);
                if (inRetVal != VS_SUCCESS)
                        continue;
                else
                        break;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Enter_PowerOn_Password() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_CUP_LOGON
Date&Time       :2017/10/3 下午 3:53
Describe        :安全認證
*/
int inFunc_Booting_Flow_CUP_LOGON(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_CUP_LOGON START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_CUP_LOGON() START !");
	}

#ifdef	_COMMUNICATION_CAPBILITY_
	int	inRetVal = VS_SUCCESS;

	if (0)
	{
		
	}
	else if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
	{
		inRetVal = inNCCC_Func_CUP_PowerOn_LogOn(pobTran);
	}
	else
	{
		
	}
#endif
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_CUP_LOGON() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Clear_AP_Dump
Date&Time       :2018/6/19 下午 1:57
Describe        :清空AP下載的暫存檔案
*/
int inFunc_Booting_Flow_Clear_AP_Dump(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Clear_AP_Dump START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Clear_AP_Dump() START !");
	}
	
	inRetVal = inFunc_Clear_AP_Dump();
	
	/* 刪完程式剩餘檔，就不應進入AP更新流程 */
	inLoadTMSCPTRec(0);
	inSetTMSAPPUpdateStatus("N");
	inSaveTMSCPTRec(0);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Clear_AP_Dump() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Process_Cradle
Date&Time       :2023/5/10 下午 7:03
Describe        :等待底座連接
*/
int inFunc_Booting_Flow_Process_Cradle(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	int	inTimeout = 0;
	int	inChoice = _DisTouch_No_Event_;
	int	inTouchSensorFunc = _Touch_CUP_LOGON_;
	char	szKey = 0x00;
	char	szEnable[1 + 1] = {0};
	char	szECRComPort[4 + 1] = {0};
	char	szCommMode[1 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Process_Cradle START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Process_Cradle() START !");
	}

	/* */
	if (inFunc_Is_Portable_Type() == VS_TRUE)
	{
		do
		{
			memset(szEnable, 0x00, sizeof(szEnable));
			inGetECREnable(szEnable);
			memset(szECRComPort, 0x00, sizeof(szECRComPort));
			inGetECRComPort(szECRComPort);
			memset(szCommMode, 0x00, sizeof(szCommMode));
			inGetCommMode(szCommMode);
			if ((!memcmp(szEnable, "Y", 1) && !memcmp(szECRComPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)))	||
			    (!memcmp(szEnable, "Y", 1) && !memcmp(szECRComPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_))))
			{
				/* 有底座ECR需求 */
			}
			else if (!memcmp(szCommMode, _COMM_ETHERNET_MODE_, 1)	||
				 !memcmp(szCommMode, _COMM_MODEM_MODE_, 1))
			{
				/* 有底座ethernet或撥接需求 */
			}
			else
			{
				/* 其餘可跳過 */
				break;
			}
			
			inDISP_ClearAll();
			inDISP_ChineseFont("等待連接底座中", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);

			/* 等待連接timeout 25秒 */
			inTimeout = 2500;
			inDISP_Timer_Start_MicroSecond(_TIMER_NEXSYS_1_, inTimeout);
			do
			{
				if (inFunc_Is_Cradle_Attached() == VS_SUCCESS)
				{
					break;
				}

				if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
				{
					inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					inDISP_ChineseFont("底座未連接", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
					inDISP_ChineseFont("是否重試", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
					inDISP_PutGraphic(_MSG_ENTER_OR_CANCEL_, 0, _COORDINATE_Y_LINE_8_8_);
					/* 執行下載請按確認，不執行請按清除 */
					/* 設定Timeout */
					inDISP_Timer_Start(_TIMER_NEXSYS_2_, _EDC_TIMEOUT_);
					while (1)
					{
						inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
						szKey = uszKBD_Key();

						/* TimeOut */
						if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
						{
							szKey = _KEY_TIMEOUT_;
						}

						if (szKey == _KEY_ENTER_			|| 
						    inChoice == _CUPLogOn_Touch_KEY_1_)
						{
							inRetVal = VS_SUCCESS;
							break;
						}
						else if (szKey == _KEY_CANCEL_			|| 
							 inChoice == _CUPLogOn_Touch_KEY_2_)
						{
							inRetVal = VS_USER_CANCEL;
							break;
						}
						else if (szKey == _KEY_TIMEOUT_)
						{
							inRetVal = VS_TIMEOUT;
							break;
						}
					}

					if (inRetVal == VS_SUCCESS)
					{
						inDISP_ClearAll();
						inDISP_ChineseFont("等待連接底座中", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
						/* 10秒 */
						inTimeout = 1000;
						inDISP_Timer_Start_MicroSecond(_TIMER_NEXSYS_1_, inTimeout);
						continue;
					}
					else if (inRetVal == VS_USER_CANCEL)
					{
						break;
					}
					else if (inRetVal == VS_TIMEOUT)
					{
						break;
					}
				}
			}while(1);

			inDISP_ClearAll();
			break;
		}while(1);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Process_Cradle() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_PowerManagement
Date&Time       :2018/3/14 上午 10:40
Describe        :經測試V3M沒辦法用auto power management，廢棄(2018/3/14 下午 5:32)
*/
int inFunc_Booting_Flow_PowerManagement(TRANSACTION_OBJECT *pobTran)
{
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_PowerManagement() START !");
	}

//#if	_MACHINE_TYPE_ == _CASTLE_TYPE_V3M_
//	
//	inPWM_PowerAutoModeEnable();
//	/* 避免進入Standby Mode */
//	inPWM_PowerAutoMode_Standby_Time_Set(0);
//	inPWM_PowerAutoMode_Sleep_Time_Set(60);
//	
//#endif
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_PowerManagement() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Reprint_Poweroff
Date&Time       :2018/3/14 上午 10:40
Describe        :檢查是否有未簽交易，要重印簽單
*/
int inFunc_Booting_Flow_Reprint_Poweroff(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Reprint_Poweroff START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Reprint_Poweroff() START !");
	}

	inNCCC_ESC_Process_PowerOff_When_Signing(pobTran);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Reprint_Poweroff() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Update_Success_Report
Date&Time       :2018/6/5 上午 9:26
Describe        :一定要放最後一個流程，生效回報
*/
int inFunc_Booting_Flow_Update_Success_Report(TRANSACTION_OBJECT *pobTran)
{
	char	szTMSReportBit[2 + 1] = {0};
	char	szTMS_FTPReportBit[2 + 1] = {0};
	char	szDEMOMode[2 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Update_Success_Report START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Update_Success_Report() START !");
	}

	memset(szDEMOMode, 0x00, sizeof(szDEMOMode));
	inGetDemoMode(szDEMOMode);
	if (memcmp(szDEMOMode, "Y", strlen("Y")) == 0)
	{
		inLoadTMSCPTRec(0);
		inSetTMSEffectiveReportBit("N");
		inSaveTMSCPTRec(0);
		
		inLoadTMSFTPRec(0);
		inSetTMSEffectiveReportBit("N");
		inSaveTMSCPTRec(0);
		
		vdUtility_SYSFIN_LogMessage(AT, "DEMO不執行回報");
		return (VS_SUCCESS);
	}
	
	inLoadTMSFTPRec(0);
	inLoadTMSCPTRec(0);
	memset(szTMSReportBit, 0x00, sizeof(szTMSReportBit));
	inGetTMSEffectiveReportBit(szTMSReportBit);
	memset(szTMS_FTPReportBit, 0x00, sizeof(szTMS_FTPReportBit));
	inGetFTPEffectiveReportBit(szTMS_FTPReportBit);
	if (memcmp(szTMSReportBit, "Y", strlen("Y")) == 0 ||
	    memcmp(szTMS_FTPReportBit, "Y", strlen("Y")) == 0)
	{
		/* 執行生效回報 */
		/* 參數生效狀態回報 */
		inNCCCTMS_TMS_Return_Report(pobTran);

		inSetTMSEffectiveReportBit("N");
		inSaveTMSCPTRec(0);
		inSetFTPEffectiveReportBit("N");
		inSaveTMSFTPRec(0);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Update_Success_Report() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_TMS_Download
Date&Time       :2018/6/25 上午 4:58
Describe        :開機執行，TMS下載
*/
int inFunc_Booting_Flow_TMS_Download(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_TMS_Download START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_TMS_Download() START !");
	}

	inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_TMS_DOWNLOAD_SETTLE_);
	
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_TMS_Download() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_TMS_AP_Update_ReCheck_OK
Date&Time       :2018/11/7 下午 1:15
Describe        :用來處理虹堡API更新非正常重開機，導致後續處理沒跑的問題
*/
int inFunc_Booting_Flow_TMS_AP_Update_ReCheck_OK(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inFd = 0;
	char	szFilePath[50 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_TMS_AP_Update_ReCheck_OK START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_TMS_AP_Update_ReCheck_OK() START !");
	}

	if (ginDebug == VS_TRUE)
	{
		inFunc_ls("", _APPL_UPDATE_CHECK_PATH_);
	}
	
	sprintf(szFilePath, "%s%s", _APPL_UPDATE_CHECK_PATH_, _APPL_UPDATE_CHECK_NAME_);
	inRetVal = inFile_Linux_Open(&inFd, szFilePath);
	if (inRetVal == VS_SUCCESS)
	{
		inFile_Linux_Close(inFd);
		/* AP更新成功後要處理的事項 */
		inNCCCTMS_Deal_With_Things_After_APPL_Update(pobTran);
		
		inFunc_Data_Delete("-r", "*", _APPL_UPDATE_CHECK_PATH_);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "補TMSOK成功");
		}
		
		vdUtility_SYSFIN_LogMessage(AT, "TMS_AP_Update_ReCheck_OK");
	}
	else
	{
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_TMS_AP_Update_ReCheck_OK() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_LoadTMK_ECR_Initial
Date&Time       :2019/1/10 下午 2:39
Describe        :開機流程初始化loadkey的ECR
*/
int inFunc_Booting_Flow_Load_TMK_ECR_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_LoadTMK_ECR_Initial() START !");
	}
	
	/* ECR initial */
	inRetVal = inECR_Load_TMK_Initial();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_LoadTMK_ECR_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Set_Castle_PWD
Date&Time       :2019/2/20 上午 11:42
Describe        :開機流程設定進入系統模式的密碼
*/
int inFunc_Booting_Flow_Set_Castle_PWD(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Set_Castle_PWD START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Set_Castle_PWD() START !");
	}
	
	/* */
	inRetVal = inNCCC_Func_Set_Terminal_Password();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Set_Castle_PWD() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Get_EDC_Boot_Time
Date&Time       :2019/2/27 上午 11:17
Describe        :取得EDC的開機時間，並將資訊紀錄在檔案裡。Added by Hachi
*/
int inFunc_Booting_Flow_Get_EDC_Boot_Time(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = 0;
	int		inHandle = 0;;
        char		szTempBuf[27+1] = "";
	char		szPathBuf[100+1] = "";
        RTC_NEXSYS	srRTC = {};
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Get_EDC_Boot_Time START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Get_EDC_Boot_Time() START !");
	}
	
        strcpy(szPathBuf, _FS_DATA_PATH_);
        strcat(szPathBuf, _TMS_TRACE_LOG_);
        
	
        inFunc_GetSystemDateAndTime(&srRTC);
	memset(szTempBuf,0x00,sizeof(szTempBuf));
        snprintf(szTempBuf,sizeof(szTempBuf),"%02u%02u%02u%02u%02u : EDC_BOOT_Time\n"
                                             ,srRTC.uszMonth
                                             ,srRTC.uszDay
                                             ,srRTC.uszHour
                                             ,srRTC.uszMinute
                                             ,srRTC.uszSecond
                                                                );
        inHandle = open(szPathBuf,O_RDWR | O_CREAT, S_IRWXU);/*增加權限，確保創檔完的檔案可以讀取*/
	if (inHandle > 0)
	{
		inFile_Open_File_Cnt_Increase();
	}else
	{
		/* [BUG] 移除檔案同步API SYNC()指令
		 * [20260506] 記錄LOG,修改檔案開啟及關閉失敗,要回傳成功不繼續使用 */
		vdUtility_SYSFIN_LogMessage(AT,"BootOpen_Fail Name[%s] HLD[%ld]", szPathBuf, inHandle);
		return VS_SUCCESS;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inHandle=%d fun=%s,line=%d",inHandle,__FUNCTION__,__LINE__);
	}
	
	
        write(inHandle, szTempBuf, 27);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "Time String is:%s, fun=%s,line=%d",szTempBuf,__FUNCTION__,__LINE__);
	}
	
        inRetVal = close(inHandle);
	if (inRetVal == 0)
	{
		inFile_Open_File_Cnt_Decrease();
	}else
	{
		/* [BUG] 移除檔案同步API SYNC()指令
		 * [20260506] 記錄LOG,修改檔案開啟及關閉失敗,要回傳成功不繼續使用 */
		vdUtility_SYSFIN_LogMessage(AT,"BootClose Close Fail Name[%s] HLD[%d] Rvl[%d]", szPathBuf, inHandle, inRetVal);
	}
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Get_EDC_Boot_Time() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Set_System_Power_Function
Date&Time       :2019/10/21 下午 2:02
Describe        :開機流程設定進入
*/
int inFunc_Booting_Flow_Set_System_Power_Function(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Set_System_Power_Function START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Set_System_Power_Function() START !");
	}
	
	/* */
	if (inFunc_Is_Portable_Type() == VS_TRUE)
	{
		inRetVal = inFunc_PowerKeyFunSet(d_POWERKEY_FUN_POWEROFF);
	}
	else
	{
		inRetVal = inFunc_PowerKeyFunSet(d_POWERKEY_FUN_DISABLE);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Set_Power_Function() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Set_Brightness
Date&Time       :2023/8/31 下午 2:21
Describe        :
*/
int inFunc_Booting_Flow_Set_Brightness(TRANSACTION_OBJECT *pobTran)
{
	char	szScreenBrightness[3 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_Set_Brightness START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Booting_Flow_Set_Brightness() START !");
	}
	
	/* 根據參數調整螢幕亮度 */
	memset(szScreenBrightness, 0x00, sizeof(szScreenBrightness));
	inGetScreenBrightness(szScreenBrightness);
	
	if (strlen(szScreenBrightness) > 0)
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(szScreenBrightness));
	}
	else
	{
		inDISP_BackLightSetBrightness(d_BKLIT_LCD_DEVICE_0, atoi(_SCREEN_BRIGHTNESS_MAX_));
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Booting_Flow_Set_Brightness() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

int inFunc_TWNAddDataToEMVPacket(TRANSACTION_OBJECT *pobTran, unsigned short usTag, unsigned char *pbtPackBuff)
{
	int	offset = 0;
	BYTE	btTmp[128 + 1] = {0};
	BYTE	btTVR[5 + 1] = {0};
	char	szDefaultTAC[10 + 1] = {0};
	char	szDenialTAC[10 + 1] = {0};
	char	szOnlineTAC[10 + 1] = {0};
        char    szNCCCFESMode[2 + 1] = {0};
        char	szDebugMsg[100 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	unsigned short usLen = 0;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inFunc_TWNAddDataToEMVPacket START!!");
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inFunc_TWNAddDataToEMVPacket(0x%02X)", usTag);
                inLogPrintf(AT, szDebugMsg);
        }
        
	if ((usTag & 0xDF00) == 0xDF00)
	{
		pbtPackBuff[offset++] = (BYTE) (usTag >> 8);
		pbtPackBuff[offset++] = (BYTE) (usTag & 0x00FF);

		if ((usTag & 0x00FF) == 0x0091)
		{
			memcpy(&pbtPackBuff[offset], &ISS_SCRIPT_RES, ISS_SCRIPT_RES[0] + 1);
			offset += (ISS_SCRIPT_RES[0] + 1);
			
			return (offset);
		}

		/* Tag DFEC */
		if ((usTag & 0x00FF) == 0x00EC)
		{
			pbtPackBuff[offset ++] = 0x01;
			pbtPackBuff[offset ++] = 0x01;
			
			return (offset);
		}

		/* Tag DFED */
		if ((usTag & 0x00FF) == 0x00ED)
		{
			pbtPackBuff[offset ++] = 0x01;
                        
                        memset(szNCCCFESMode, 0x00, sizeof(szNCCCFESMode));
                        inGetNCCCFESMode(szNCCCFESMode);
			
			memset(szCFESMode, 0x00, sizeof(szCFESMode));
			inGetCloud_MFES(szCFESMode);

			/* ATS規格 Fallback時，Tag DFED=0x01 */
			if ((!memcmp(szNCCCFESMode, _NCCC_05_ATS_MODE_, strlen(_NCCC_05_ATS_MODE_))		|| 
			     !memcmp(szNCCCFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_))		||
			    (!memcmp(szNCCCFESMode, _NCCC_03_MFES_MODE_, strlen(_NCCC_03_MFES_MODE_))		&&
			      memcmp(szCFESMode, "Y", 1) == 0))							&& 
			      pobTran->srBRec.uszEMVFallBackBit == VS_TRUE					&& 
			      pobTran->srBRec.uszDCCTransBit != VS_TRUE)
				pbtPackBuff[offset ++] = 0x01;
			else
				pbtPackBuff[offset ++] = 0x02;
			
			return (offset);
		}

		/* Tag DFEE */
		if ((usTag & 0x00FF) == 0x00EE)
		{
			pbtPackBuff[offset++] = 0x01;
			pbtPackBuff[offset++] = 0x05;
			
			return (offset);
		}

		/*  Tag DFEF */
		if ((usTag & 0x00FF) == 0x00EF)
		{
                        memcpy(btTVR, pobTran->srEMVRec.usz95_TVR, pobTran->srEMVRec.in95_TVRLen);
                        inGetDefaultTAC(szDefaultTAC);
                        inGetOnlineTAC(szOnlineTAC);
                        inGetDenialTAC(szDenialTAC);

			if (strlen(szDefaultTAC))
			{
                                inFunc_ASCII_to_BCD((BYTE *)TACDEFAULT, szDefaultTAC, 5);
                                inFunc_ASCII_to_BCD((BYTE *)TACDEFAULT, szDenialTAC, 5);
                                inFunc_ASCII_to_BCD((BYTE *)TACDEFAULT, szOnlineTAC, 5);
			}

			pbtPackBuff[offset ++] = 0x02;

			/*******************************************************************************
			1. 1504: fallback tx.
			2. 1511: merchant suspicious.  Merchant force online.
			3. 1503: random selection.  If both random selection bit of TVR and TAC are set.
			4. 1510: over floor limit.  If both floor limit bit of TVR and TAC are set.
			5. 1508: any other bits of TVR and TAC are set
			6. 1505: icc.  If ARQC return from card.
			********************************************************************************/

			if ((btTVR[3] & 0x10) && (TACONLINE[3] & 0x10))
			{
				/* Random select online */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x03;
			}
			else if (pobTran->srBRec.uszEMVFallBackBit == VS_TRUE)
			{
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x04;
			}
			else if ((btTVR[3] & 0x80) && (TACONLINE[3] & 0x80))
			{
				/* Over floor limit.  If both floor limit bit of TVR and TAC are set. */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x10;
			}
			else if (btTVR[3] & 0x01)
			{
				/* (Online force by terminal)Merchant force online */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x11;
			}
			else if ((btTVR[0] & TACONLINE[0]) &&
				 (btTVR[1] & TACONLINE[1]) &&
				 (btTVR[2] & TACONLINE[2]) &&
				 (btTVR[3] & TACONLINE[3]) &&
				 (btTVR[4] & TACONLINE[4]))
			{
				/* any other bits of TVR and TAC are set */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x08;
			}
			else
			{
				/* (Online force by card ARQC) */
				pbtPackBuff[offset ++] = 0x15;
				pbtPackBuff[offset ++] = 0x05;
			}
			
			return (offset);
		}
		
		return (0);
	}
	else
	{
		/* 9FXX */
		if ((usTag & 0xFFFF) == 0x9F5B)
		{
			memcpy( &pbtPackBuff[offset], &ISS_SCRIPT_RES, ISS_SCRIPT_RES[0] + 1);
			offset += (ISS_SCRIPT_RES[0] + 1);
			
			return (offset);
		}
		else
		{
                        if (EMV_DataGet(usTag, &usLen, btTmp) != d_EMVAPLIB_OK)
                        {
                                return (0);
                        }

			if ((usTag & 0xFFFF) == 0x4F00)
				pbtPackBuff[offset ++] = 0x84;
			else
			{
				if ((usTag & 0x1F00) == 0x1F00)
				{	/* 2-byte tag */
					pbtPackBuff[offset ++] = (BYTE) (usTag >> 8);
					pbtPackBuff[offset ++] = (BYTE) (usTag & 0x00FF);
				}
				else
				{
					pbtPackBuff[offset ++] = (BYTE) (usTag >> 8);
				}
			}

			pbtPackBuff[offset++] = (BYTE) usLen;
			memcpy(pbtPackBuff + offset, btTmp, usLen);
			
			return (offset + (int )usLen);
		}
	}
}

/*
Function        :inFunc_Set_Temp_VersionID
Date&Time       :2017/1/12 上午 10:25
Describe        :設定臨時的Terminal Version ID
*/
int inFunc_Set_Temp_VersionID()
{
	int		inRetVal;
	char		szTerminalVersionID[15 + 1];	/* 檔案名稱最多15字*/
        DISPLAY_OBJECT  srDispObj;

        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));

        srDispObj.inY = _LINE_8_7_;
        srDispObj.inR_L = _DISP_RIGHT_;
        srDispObj.inMaxLen = 16;
	srDispObj.inColor = _COLOR_RED_;
	srDispObj.inTimeout = 30;

        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	
	memset(szTerminalVersionID, 0x00, sizeof(szTerminalVersionID));
	if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szTerminalVersionID, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szTerminalVersionID);
	}
	inDISP_ChineseFont_Color("原VersonID:", _FONTSIZE_8X22_, _LINE_8_4_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color(szTerminalVersionID, _FONTSIZE_16X22_, _LINE_16_9_, _COLOR_RED_, _DISP_LEFT_);
	/* Set Version ID */
        inDISP_ChineseFont_Color("設定 Version ID:", _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	
	srDispObj.inOutputLen = strlen(szTerminalVersionID);
	memcpy(srDispObj.szOutput, szTerminalVersionID, srDispObj.inOutputLen);
	inDISP_EnglishFont_Color(srDispObj.szOutput, _FONTSIZE_8X22_, _LINE_8_7_, srDispObj.inColor, _DISP_RIGHT_);
	
        inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

        if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);
	
	memset(gszTermVersionID, 0x00, sizeof(gszTermVersionID));
	memcpy(gszTermVersionID, &srDispObj.szOutput[0], srDispObj.inMaxLen);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_Version_ID
Date&Time       :2017/1/18 上午 11:42
Describe        :可確認Version
*/
int inFunc_Check_Version_ID()
{
	char		szDispMsg[50 + 1];
	unsigned char	uszKey;
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CHECK_VERSION_, 0, _COORDINATE_Y_LINE_8_4_);

	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	if (strlen(gszTermVersionID) > 0)
	{
		memcpy(szDispMsg, gszTermVersionID, strlen(gszTermVersionID));
	}
	else
	{
		inGetTermVersionID(szDispMsg);
	}
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	if (strlen(gszTermVersionDate) > 0)
	{
		memcpy(szDispMsg, gszTermVersionDate, strlen(gszTermVersionDate));
	}
	else
	{
		inGetTermVersionDate(szDispMsg);
	}
	inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);

	while (1)
	{
		uszKey = uszKBD_GetKey(30);

		if (uszKey == _KEY_ENTER_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_ || uszKey == _KEY_TIMEOUT_)
		{
			return (VS_ERROR);
		}
		else
		{
			continue;
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Delete_Signature
Date&Time       :2017/3/22 上午 10:21
Describe        :砍簽名圖檔
*/
int inFunc_Delete_Signature(TRANSACTION_OBJECT *pobTran)
{
	char	szSignature[16 + 1];
	char	szDebugMsg[100 + 1];
	
	/* 商店聯印成功就砍簽名圖檔 */
	memset(szSignature, 0x00, sizeof(szSignature));
	/* 因為用invoice所以不用inFunc_ComposeFileName */
	inFunc_ComposeFileName_InvoiceNumber(pobTran, szSignature, _PICTURE_FILE_EXTENSION_, 6);
	/* 砍簽名圖檔 */
	inFILE_Delete((unsigned char*)szSignature);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Delete Signature %s", szSignature);
		inLogPrintf(AT, szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_DuplicateSave
Date&Time       :2017/3/30 下午 4:23
Describe        :檢核是否重複刷卡
*/
int inFunc_DuplicateSave(TRANSACTION_OBJECT* pobTran)
{
	char	szSplitTxnCheckEnable[2 + 1];
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_DuplicateSave START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_DuplicateSave() START !");
	}
	if (ginFindRunTime == VS_TRUE)
	{
		inFunc_RecordTime_Append("%d %s", __LINE__, __FUNCTION__);
	}
	
	memset(szSplitTxnCheckEnable, 0x00, sizeof(szSplitTxnCheckEnable));
	inGetSplitTransCheckEnable(szSplitTxnCheckEnable);
	if (memcmp(szSplitTxnCheckEnable, "Y", strlen("Y")) != 0)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_DuplicateSave Not_Enable END!");
   		return (VS_SUCCESS);
        }

	/* 我覺得應該是不會有人刷了之後馬上重開機再刷一次，所以直接存global */
	if (pobTran->inTransactionCode == _VOID_			||
	    pobTran->inTransactionCode == _REFUND_			||
	    pobTran->inTransactionCode == _INST_REFUND_			||
	    pobTran->inTransactionCode == _REDEEM_REFUND_		||
	    pobTran->inTransactionCode == _CUP_REFUND_			||
	    pobTran->inTransactionCode == _CUP_MAIL_ORDER_REFUND_	||
	    pobTran->inTransactionCode == _CUP_VOID_)
	{
		
	}
	else
	{
		memcpy(gszDuplicatePAN, pobTran->srBRec.szPAN, sizeof(pobTran->srBRec.szPAN));
	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_DuplicateSave() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_DuplicateSave END!");
        
	return (VS_SUCCESS);
}

/*
Function        :inFunc_DuplicateCheck
Date&Time       :2017/3/30 下午 4:23
Describe        :檢核是否重複刷卡
*/
int inFunc_DuplicateCheck(TRANSACTION_OBJECT* pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szSplitTxnCheckEnable[2 + 1];
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_DuplicateCheck START!");
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_DuplicateCheck() START !");
	}
	
	memset(szSplitTxnCheckEnable, 0x00, sizeof(szSplitTxnCheckEnable));
	inGetSplitTransCheckEnable(szSplitTxnCheckEnable);
	if (memcmp(szSplitTxnCheckEnable, "Y", strlen("Y")) != 0)
        {
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_DuplicateCheck szSplitTxnCheckEnable END!");
   		return (VS_SUCCESS);
        }
	
	/* 功能有開才檢核 */
	if (memcmp(szSplitTxnCheckEnable, "Y", strlen("Y")) == 0)
	{
		if (pobTran->inTransactionCode == _SALE_		||
		    pobTran->inTransactionCode == _SALE_OFFLINE_	||
		    pobTran->inTransactionCode == _FORCE_CASH_ADVANCE_	||
		    pobTran->inTransactionCode == _INST_SALE_		||
		    pobTran->inTransactionCode == _REDEEM_SALE_		||
		    pobTran->inTransactionCode == _CUP_SALE_		||
		    pobTran->inTransactionCode == _REDEEM_ADJUST_	||
		    pobTran->inTransactionCode == _INST_ADJUST_)
		{
			if (!memcmp(gszDuplicatePAN, pobTran->srBRec.szPAN, strlen(pobTran->srBRec.szPAN)))
			{
				DISPLAY_OBJECT	srDispMsgObj;
				memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
				strcpy(srDispMsgObj.szDispPic1Name, "");
				srDispMsgObj.inDispPic1YPosition = 0;
				srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
				srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
				strcpy(srDispMsgObj.szErrMsg1, "此卡號與前筆重覆");
				srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
				srDispMsgObj.inBeepTimes = 1;
				srDispMsgObj.inBeepInterval = 0;
				inDISP_Msg_BMP(&srDispMsgObj);	/* 127 = 此卡號與前筆重覆 */
				inRetVal = VS_ERROR;
			}
			
		}

	}

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_DuplicateCheck() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_DuplicateCheck END!");
        
	return (inRetVal);
}

/*
Function        :inFunc_SHA256
Date&Time       :2017/4/5 上午 9:19
Describe        :
*/
int inFunc_SHA256(unsigned char* uszInData, unsigned int uiInDataLen, unsigned char* uszOutData)
{
	CTOS_SHA256_CTX	srSHA256;
	
	CTOS_SHA256Init(&srSHA256);
	
	CTOS_SHA256Update(&srSHA256, uszInData, uiInDataLen);
	
	CTOS_SHA256Final(&srSHA256, uszOutData);

	return (VS_SUCCESS);
}

/*
Function        :inFunc_Base64_Encryption
Date&Time       :2017/4/5 上午 10:25
Describe        :(Verifone NCCCfunc.c:31114 base64_encode)
*/
int inFunc_Base64_Encryption(char* szInData, int inInDataLen, char* szOutData)
{
	char		*szInDatap = szInData;
	char		*top = szOutData;
	char		end[3];
	char		b64string[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned char	cbyte;
	unsigned char	obyte;

	for(; inInDataLen >= 3; inInDataLen -= 3)
	{
		cbyte = *szInDatap++;
		*top++ = b64string[(int)(cbyte >> 2)];
		obyte = (cbyte << 4) & 0x30;		/* 0011 0000 */

		cbyte = *szInDatap++;
		obyte |= (cbyte >> 4);			/* 0000 1111 */
		*top++ = b64string[(int)obyte];
		obyte = (cbyte << 2) & 0x3C;		/* 0011 1100 */

		cbyte = *szInDatap++;
		obyte |= (cbyte >> 6);			/* 0000 0011 */
		*top++ = b64string[(int)obyte];
		*top++ = b64string[(int)(cbyte & 0x3F)];/* 0011 1111 */
	}

	if(inInDataLen)
	{
		end[0] = *szInDatap++;
		if (--inInDataLen) end[1] = *szInDatap++; else end[1] = 0;
		end[2] = 0;

		cbyte = end[0];
		*top++ = b64string[(int)(cbyte >> 2)];
		obyte = (cbyte << 4) & 0x30;		/* 0011 0000 */

		cbyte = end[1];
		obyte |= (cbyte >> 4);
		*top++ = b64string[(int)obyte];
		obyte = (cbyte << 2) & 0x3C;		/* 0011 1100 */

		if (inInDataLen) *top++ = b64string[(int)obyte];
		else *top++ = '=';
		*top++ = '=';
	}

	*top = 0;
	
	return (VS_SUCCESS);
}

/*
 Function        :inFunc_Base64_Decryption
 Date&Time       :2024/8/23 上午 11:08
 Describe        :(Verifone NCCCfunc.c:28902 base64_decode)
 */
int inFunc_Base64_Decryption(char *to, const char *from, int len)
{ 
    int i;
    int padding = 0;
    char *top = to;
    const char *fromp = from;
    unsigned char cbyte = 0;
    unsigned char obyte = 0;
    unsigned char decode_table[256];
    const char *b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if (!to || !from || len <= 0 || (len % 4 != 0)) 
    {
        return -1; // 檢查無效輸入
    }

    memset(decode_table, 0x80, sizeof(decode_table)); // 初始化所有條目為非法字符

    // 定義合法的Base64字符
    for (i = 0; i < 64; i++) 
    {
        decode_table[(unsigned char)b64_chars[i]] = i;
    }
    decode_table['='] = 0; // '='為填充字符

    // 開始解碼
    while (len > 0) 
    {
        cbyte = decode_table[(unsigned char)*fromp++];
        if (cbyte & 0x80) 
        {
            return -1; // 非法字符
        }
        obyte = cbyte << 2;

        cbyte = decode_table[(unsigned char)*fromp++];
        if (cbyte & 0x80) 
        {
            return -1; // 非法字符
        }
        obyte |= cbyte >> 4;
        *top++ = obyte;

        if (*fromp == '=') 
        {
            padding++;
            fromp++;
        } 
        else 
        {
            obyte = (cbyte << 4);
            cbyte = decode_table[(unsigned char)*fromp++];
            if (cbyte & 0x80) 
            {
                return -1; // 非法字符
            }
            obyte |= (cbyte >> 2);
            *top++ = obyte;
        }

        if (*fromp == '=') 
        {
            padding++;
            fromp++;
        } 
        else 
        {
            obyte = (cbyte << 6);
            cbyte = decode_table[(unsigned char)*fromp++];
            if (cbyte & 0x80) 
            {
                return -1; // 非法字符
            }
            obyte |= cbyte;
            *top++ = obyte;
        }

        len -= 4;
    }

    *top = '\0'; // 終止字串

    return (top - to) - padding; // 返回解碼後的有效字節數
}

/*
Function        :inFunc_Update_AP
Date&Time       :2017/5/8 下午 2:54
Describe        :
*/
int inFunc_Update_AP(char* szPathName)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Update_AP() START !");
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Update_AP START!(%s)", szPathName);
	
	/* 保存資料 */
	inFunc_UpdateAP_Preserve();
	
	inRetVal = CTOS_UpdateFromMMCIEx((unsigned char *)szPathName, &callbackFun);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inRetVal = 0x%04X", inRetVal);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* OS1404版後會多回傳0x65,0x66，也要判定為成功 2021/10/12 下午 4:07 */
	if (inRetVal == d_OK			||
	    inRetVal == d_CAP_RESET_REQUIRED	||
	    inRetVal == d_CAP_REBOOT_REQUIRED)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inFunc_Update_AP CTOS_UpdateFromMMCIEx_success(0x%04X)", inRetVal);
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "CTOS_UpdateFromMMCIEx update success");
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRetVal = 0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		/* 更新成功 */
		/* 回覆資料 */
		inFunc_UpdateAP_Recover();
	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT, "inFunc_Update_AP CTOS_UpdateFromMMCIEx_failed(0x%04X)", inRetVal);
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRetVal = 0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Update_AP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Idle_CheckCustomPassword_Flow
Date&Time       :2017/6/8 下午 2:51
Describe        :
*/
int inFunc_Idle_CheckCustomPassword_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Idle_CheckCustomPassword_Flow START");
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Idle_CheckCustomPassword_Flow(incode = %d) START !", pobTran->srBRec.inCode);
	}
	
	/* 第一層輸入密碼 */
        if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE)
        {
                inRetVal = inFunc_CheckCustomizePassword(_ACCESS_WITH_CUSTOM_, pobTran->srTrustRec.inCode);
        }
        else
        {
                inRetVal = inFunc_CheckCustomizePassword(_ACCESS_WITH_CUSTOM_, pobTran->srBRec.inCode);
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Idle_CheckCustomPassword_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Idle_CheckCustomPassword_Flow END");
	
	return (inRetVal);
}

/*
Function        :inFunc_ECR_CheckCustomPassword_Flow
Date&Time       :2022/5/11 上午 10:06
Describe        :
*/
int inFunc_ECR_CheckCustomPassword_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	char	szCustomIndicator[3 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_ECR_CheckCustomPassword_Flow START!");
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_ECR_CheckCustomPassword_Flow(incode = %d) START !", pobTran->srBRec.inCode);
	}
	
	if (pobTran->uszECRBit == VS_TRUE)
	{
		memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
		inGetCustomIndicator(szCustomIndicator);
		
		/* 收銀機發動結帳，不用提示輸入管理號碼，只有單機才需要 */
		if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_) ||
		    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			
		}
		else
		{
			/* 兩段式第二段不用重跑 */
			if (pobTran->uszCardInquirysSecondBit == VS_TRUE)
			{

			}
			else
			{
				/* 紅利積點、點數扣抵、加價購且選用信用卡、HGI、銀聯卡要套用sale密碼 */
				if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_		||
				    pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_	||
				    pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
				{
					if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_		||
					    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_	||
					    pobTran->srBRec.lnHGPaymentType == _HG_PAY_CUP_)
					{
						inRetVal = inFunc_CheckCustomizePassword(_ACCESS_WITH_CUSTOM_, pobTran->srBRec.lnHGTransactionType);
					}
					else
					{
						/* 不輸入 */
					}
				}
				else if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_	||
					 pobTran->srBRec.lnHGTransactionType == _HG_INQUIRY_)
				{
					/* 不輸入 */
				}
				else
				{
					/* 第一層輸入密碼 */
					inRetVal = inFunc_CheckCustomizePassword(_ACCESS_WITH_CUSTOM_, pobTran->inTransactionCode);
				}
			}
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_ECR_CheckCustomPassword_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_ECR_CheckCustomPassword_Flow END!");
        
	return (inRetVal);
}

/*
Function        :inFunc_Change_EMVForceOnline
Date&Time       :2017/9/4 上午 10:58
Describe        :
*/
int inFunc_Change_EMVForceOnline()
{
	char	szEMVForceOnline[2 + 1];
	char	szDispMsg[50 + 1];
	
	memset(szEMVForceOnline, 0x00, sizeof(szEMVForceOnline));
	inGetEMVForceOnline(szEMVForceOnline);
	
	if (memcmp(szEMVForceOnline, "N", strlen("N")) == 0)
	{
		memcpy(szEMVForceOnline, "Y", strlen("Y"));
	}
	else
	{
		memcpy(szEMVForceOnline, "N", strlen("N"));
	}
	
	inSetEMVForceOnline(szEMVForceOnline);
	inSaveEDCRec(0);
	
	memset(szDispMsg, 0x00, sizeof(szDispMsg));
	sprintf(szDispMsg, "Force Online: %s", szEMVForceOnline);
	DISPLAY_OBJECT	srDispMsgObj;
	memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
	strcpy(srDispMsgObj.szDispPic1Name, "");
	srDispMsgObj.inDispPic1YPosition = 0;
	srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
	srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
	strcpy(srDispMsgObj.szErrMsg1, szDispMsg);
	srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
	srDispMsgObj.inBeepTimes = 1;
	srDispMsgObj.inBeepInterval = 0;
			
	inDISP_Msg_BMP(&srDispMsgObj);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_LRC
Date&Time       :2017/9/19 下午 4:05
Describe        :
 *		szbuff填陣列位置
 *		inLen填字串長度
 *		szOutLRC輸出的字元
*/
int inFunc_Check_LRC(char *szbuff, int inLen, char *szOutLRC)
{
	int	i = 0;
	
	/* 先清為0 */
	*szOutLRC = 0x00;
	
	for (i = 0; i < inLen; i++)
	{
		*szOutLRC = *szOutLRC ^ szbuff[i];
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Start_Display_Time_Thread
Date&Time       :2017/12/8 下午 4:57
Describe        :廢棄（2017/12/8 下午 5:42）
*/
int inFunc_Start_Display_Time_Thread()
{
//	pthread_t		uliId;
//	
//	pthread_create(&uliId, NULL, (void*)vdFunc_Display_Time, NULL);
	
	return (VS_SUCCESS);
}

/*
Function        :vdFunc_Display_All_Status
Date&Time       :2018/3/9 上午 11:47
Describe        :szFunEnable欄位表示功能開關
 *		 1:時間
 *		 2:連線狀態
*/
int inFunc_Display_All_Status(char* szFunEnable)
{
	int	inOffset = 0;
	int	inStatusNum = 5;
	char	szCommMode[2 + 1] = {0};
	
	if (strlen(szFunEnable) < inStatusNum)
	{
		return (VS_ERROR);
	}
	
	/* 檢查是否需更新時間 */
	/* 檢查TIMEOUT，至少一秒才顯示差別 */
	if (inTimerGet(_TIMER_NEXSYS_2_) == VS_SUCCESS)
	{
		/* 顯示時間 */
		if (strlen(szFunEnable) >= 1)
		{
			inOffset = 0;
			if (*(szFunEnable + inOffset) == '1')
			{
				inFunc_Display_Time();
			}
		}
		
		/* 顯示連線狀態 */
		if (strlen(szFunEnable) >= 2)
		{
			inOffset = 1;
			if (*(szFunEnable + inOffset) == '1')
			{
				vdFunc_Display_Ethernet_Status();
			}
		}
		
		/* 顯示電量 */
		if (strlen(szFunEnable) >= 3)
		{
			inOffset = 2;
			if (*(szFunEnable + inOffset) == '1')
			{
				vdFunc_Display_Battery_Status();
			}
		}
		
		/* 顯示WiFi收訊 260~ 290 */
		if (strlen(szFunEnable) >= 4)
		{
			inOffset = 3;
			if (*(szFunEnable + inOffset) == '1')
			{
				vdFunc_Display_WiFi_Quality();
			}
		}
		
		/* 顯示SIM卡收訊 290 ~ 320*/
		if (strlen(szFunEnable) >= 5)
		{
			inOffset = 4;
			if (*(szFunEnable + inOffset) == '1')
			{
				memset(szCommMode, 0x00, sizeof(szCommMode));
				inGetCommMode(szCommMode);
				
				if (memcmp(szCommMode, _COMM_GPRS_MODE_, 1) == 0 ||
				    memcmp(szCommMode, _COMM_3G_MODE_, 1) == 0 ||
				    memcmp(szCommMode, _COMM_4G_MODE_, 1) == 0)
				{
					vdFunc_Display_SIM_Quality();
				}
			}
		}
		
		inDISP_Timer_Start(_TIMER_NEXSYS_2_, 1);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Display_All_Status_By_Machine_Type
Date&Time       :2018/5/21 上午 10:51
Describe        :
*/
int inFunc_Display_All_Status_By_Machine_Type(void)
{
	int	inRetVal = VS_ERROR;
	
	if (ginMachineType == _CASTLE_TYPE_V3C_)
	{
		inRetVal = inFunc_Display_All_Status("11001");
	}
	else if (ginMachineType == _CASTLE_TYPE_V3P_)
	{
		inRetVal = inFunc_Display_All_Status("11001");
	}
	else if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		inRetVal = inFunc_Display_All_Status("00000");
	}
	else if (ginMachineType == _CASTLE_TYPE_MP200_)
	{
		inRetVal = inFunc_Display_All_Status("00110");
	}
	else
	{
		inRetVal = inFunc_Display_All_Status("11111");
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_Display_Time
Date&Time       :2017/10/25 下午 3:27
Describe        :
*/
int inFunc_Display_Time()
{
        char		szTemplate[42 + 1] = {0};
	RTC_NEXSYS	srRTC;

	memset(szTemplate, 0x00, sizeof(szTemplate));

	memset(&srRTC, 0x00, sizeof(srRTC));
	if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	sprintf(szTemplate, "20%02d/%02d/%02d %02d:%02d:%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
	inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_16X44_, _LINE_16_1_, _COLOR_BLACK_, _COLOR_WHITE_, 1);
	
	return (VS_SUCCESS);
}

/*
Function        :vdFunc_Display_Ethernet_Status
Date&Time       :2018/3/9 上午 11:18
Describe        :icon 30*30
*/
void vdFunc_Display_Ethernet_Status()
{
	unsigned int	uiXPos = 165;
	
	if (inETHERNET_IsPhysicalOnine() == VS_SUCCESS)
	{
		inDISP_PutGraphic(_ICON_ETHERNET_CONNECTED_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
	else
	{
		inDISP_PutGraphic(_ICON_ETHERNET_DISCONNECTED_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
}

/*
Function        :vdFunc_Display_Battery_Status
Date&Time       :2018/3/9 下午 1:32
Describe        :電池icon 20*30
*/
void vdFunc_Display_Battery_Status()
{
	int		inRetVal = VS_ERROR;
	char		szTemplate[42 + 1] = {0};
	char		szPercentage[4 + 1] = {0};
	unsigned int	uiXPos = 200;
	unsigned char	uszPercentage = 0;
	
	/* 顯示沒電池 */
	if (inFunc_Is_Battery_Exist() != VS_SUCCESS)
	{
//		inDISP_PutGraphic(_ICON_BATTERY_NOT_EXIST_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
	/* 有電池 */
	else
	{
		/* 是否充電中 */
		inRetVal = inFunc_Is_Battery_Charging();
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_PutGraphic(_ICON_BATTERY_CHARGING_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			inRetVal = inFunc_Get_Battery_Capacity(&uszPercentage);
		}
		else
		{
			inRetVal = inFunc_Get_Battery_Capacity(&uszPercentage);
			/* 0% */
			if (inRetVal != VS_SUCCESS	||
			    (uszPercentage >= 0		&&
			     uszPercentage < 10))
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_000_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 10	&&
				 uszPercentage < 20)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_020_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 20	&&
				 uszPercentage < 40)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_040_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 40	&&
				 uszPercentage < 60)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_060_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 60	&&
				 uszPercentage < 80)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_080_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			else if (uszPercentage >= 80	&&
				 uszPercentage <= 1000)
			{
				inDISP_PutGraphic(_ICON_BATTERY_CAPACITY_100_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
		}
		
		/* 顯示電量比 */
		/* 清空，不然數字可能殘留 */
		inDISP_ChineseFont_Point_Color("    ", _FONTSIZE_16X44_, _LINE_16_1_, _COLOR_BLACK_, _COLOR_WHITE_, 28);
		if (uszPercentage > 0)
		{
			memset(szPercentage, 0x00, sizeof(szPercentage));
			sprintf(szPercentage, "%d%%", uszPercentage);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			inFunc_PAD_ASCII(szTemplate, szPercentage, ' ', 4, _PADDING_LEFT_);
			inDISP_ChineseFont_Point_Color(szTemplate, _FONTSIZE_16X44_, _LINE_16_1_, _COLOR_BLACK_, _COLOR_WHITE_, 28);
		}
	}
}

/*
Function        :vdFunc_Display_WiFi_Quality
Date&Time       :2018/3/13 下午 4:18
Describe        :icon 30 * 30
*/
void vdFunc_Display_WiFi_Quality(void)
{
	int		inRetVal = VS_ERROR;
	unsigned int	uiXPos = 260;
	unsigned char	uszQuality = 0;
	
	/* 不支援 */
	if (ginWiFiSupport != VS_TRUE)
	{
//		inDISP_PutGraphic(_ICON_WIFI_NO_SIGNAL_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
	/* 有訊號 */
	else
	{
		inRetVal = inWiFi_Get_Quality(&uszQuality);
		
		if (inRetVal != VS_SUCCESS)
		{
			/* 抓失敗也不顯示*/
		}
		else
		{
			/* 待測試 */
			/* 無法測定 */
			if (uszQuality == 0)
			{
				inDISP_PutGraphic(_ICON_WIFI_NO_SIGNAL_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			/* 一格 */
			else if (uszQuality == 1)
			{
				inDISP_PutGraphic(_ICON_WIFI_SIGNAL_1_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			/* 兩格 */
			else if (uszQuality == 2)
			{
				inDISP_PutGraphic(_ICON_WIFI_SIGNAL_2_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			/* 三格 */
			else if (uszQuality == 3)
			{
				inDISP_PutGraphic(_ICON_WIFI_SIGNAL_3_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			/* 四格 */
			else if (uszQuality == 4)
			{
				inDISP_PutGraphic(_ICON_WIFI_SIGNAL_4_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
			/* 五格 */
			else if (uszQuality == 5)
			{
				inDISP_PutGraphic(_ICON_WIFI_SIGNAL_5_, uiXPos, _COORDINATE_Y_LINE_16_1_);
			}
		}
	}
}

/*
Function        :vdFunc_Display_SIM_Quality
Date&Time       :2018/3/13 下午 2:13
Describe        :icon 30 * 30
*/
void vdFunc_Display_SIM_Quality(void)
{
	unsigned char	uszQuality = 0;
	unsigned int	uiXPos = 290;
	
	/* 顯示沒訊號 */
	if (inGSM_GetSignalQuality(&uszQuality) != VS_SUCCESS ||
	    inGPRS_Check_Network_Status() != VS_SUCCESS)
	{
//		inDISP_PutGraphic(_ICON_SIM_NO_SIGNAL_, uiXPos, _COORDINATE_Y_LINE_16_1_);
	}
	/* 有訊號 */
	else
	{
		
		/* 無法測定 */
		if (uszQuality == 99)
		{
			inDISP_PutGraphic(_ICON_SIM_NO_SIGNAL_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 1格 -113 ~ -111 */
		else if (uszQuality >= 0	&&
			 uszQuality <= 1)
		{
			inDISP_PutGraphic(_ICON_SIM_SIGNAL_1_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 兩格 -110 ~ -91 */
		else if (uszQuality > 1		&&
			 uszQuality <= 11)
		{
			inDISP_PutGraphic(_ICON_SIM_SIGNAL_2_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 三格 -90 ~ -76*/
		else if (uszQuality > 11		&&
			 uszQuality <= 17)
		{
			inDISP_PutGraphic(_ICON_SIM_SIGNAL_3_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
		/* 四格 */
		else if (uszQuality > 17	&&
			 uszQuality <= 31)
		{
			inDISP_PutGraphic(_ICON_SIM_SIGNAL_4_, uiXPos, _COORDINATE_Y_LINE_16_1_);
		}
	}
}

/*
Function        :inFunc_Display_LOGO
Date&Time       :2017/11/21 下午 5:03
Describe        :
*/
int inFunc_Display_LOGO(int inX, int inY)
{
	int	inLOGONum = 0;
	char	szFileName[100 + 1];
	char	szLOGONum[4 + 1];
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Display_LOGO() START !");
	}
	
	memset(szLOGONum, 0x00, sizeof(szLOGONum));
	inGetLOGONum(szLOGONum);
	memset(szFileName, 0x00, sizeof(szFileName));
	
	if (strlen(szLOGONum) <= 0)
	{
		
	}
	else
	{
		memset(szFileName, 0x00, sizeof(szFileName));
		
		inLOGONum = atoi(szLOGONum);
		switch(inLOGONum)
		{
			case	1:
				sprintf(szFileName, "%s", _MENU_HOST_001_TAIWAN_COOPERATIVE_BANK_);
				break;
			case	2:
				sprintf(szFileName, "%s", _MENU_HOST_002_SHANGHAI_COMMERCIAL_SAVINGS_BANK_);
				break;
			case	3:
				sprintf(szFileName, "%s", _MENU_HOST_003_LAND_BANK_OF_TAIWAN_);
				break;
			case	4:
				sprintf(szFileName, "%s", _MENU_HOST_004_CHINATRUST_COMMERCIAL_BANK_);
				break;
			case	5:
				sprintf(szFileName, "%s", _MENU_HOST_005_JIH_SUN_INTERNATIONAL_BANK_);
				break;
			case	6:
				sprintf(szFileName, "%s", _MENU_HOST_006_TAIPEI_FUBON_COMMERCIAL_BANK_);
				break;
			case	7:
				sprintf(szFileName, "%s", _MENU_HOST_007_TAISHIN_INTERNATIONAL_BANK_);
				break;
			case	8:
				sprintf(szFileName, "%s", _MENU_HOST_008_Bank_SINO_PAC_);
				break;
			case	9:
				sprintf(szFileName, "%s", _MENU_HOST_009_ESUN_COMMERCIAL_BANK_);
				break;
				
				
			case	10:
				sprintf(szFileName, "%s", _MENU_HOST_010_MEGA_INTERNAIONAL_COMMERCIAL_BANK_);
				break;
			case	11:
				sprintf(szFileName, "%s", _MENU_HOST_011_ENTIE_COMMERCIAL_BANK_);
				break;
			case	12:
				sprintf(szFileName, "%s", _MENU_HOST_012_BANK_OF_KAOHSIUNG_);
				break;
			case	13:
				sprintf(szFileName, "%s", _MENU_HOST_013_CATHAY_UNITED_BANK_);
				break;
			case	14:
				sprintf(szFileName, "%s", _MENU_HOST_014_FIRST_COMMERCIAL_BANK_);
				break;
			case	15:
				sprintf(szFileName, "%s", _MENU_HOST_015_KGI_BANK_);
				break;
			case	16:
				sprintf(szFileName, "%s", _MENU_HOST_016_HUA_NAN_COMMERCIAL_BANK_);
				break;
			case	17:
				sprintf(szFileName, "%s", _MENU_HOST_017_SHIN_KONG_COMMERCIAL_BANK_);
				break;
			case	18:
				sprintf(szFileName, "%s", _MENU_HOST_018_CHANG_HWA_COMMERCIAL_BANK_);
				break;
			case	19:
				sprintf(szFileName, "%s", _MENU_HOST_019_TAIWAN_BUSINESS_BANK_);
				break;
				
				
			case	20:
				sprintf(szFileName, "%s", _MENU_HOST_020_FAR_EASTERN_INTERNATIONAL_BANK_);
				break;
			case	21:
				sprintf(szFileName, "%s", _MENU_HOST_021_UNION_BANK_);
				break;
			case	22:
				sprintf(szFileName, "%s", _MENU_HOST_022_COTA_COMMERCIAL_BANK_);
				break;
			case	23:
				sprintf(szFileName, "%s", _MENU_HOST_023_TA_CHONG_BANK_);
				break;
			case	24:
				sprintf(szFileName, "%s", _MENU_HOST_024_YUANTA_COMMERCIAL_BANK_);
				break;
			case	25:
				sprintf(szFileName, "%s", _MENU_HOST_025_TAICHUNG_COMMERCIAL_BANK_);
				break;
			case	26:
				sprintf(szFileName, "%s", _MENU_HOST_026_AEON_);
				break;
			case	27:
				sprintf(szFileName, "%s", _MENU_HOST_027_RAKUTEN_);
				break;
			case	28:
				sprintf(szFileName, "%s", _MENU_HOST_028_CITIBANK_);
				break;
			case	29:
				sprintf(szFileName, "%s", _MENU_HOST_029_DBS_BANK_);
				break;
				
				
			case	30:
				sprintf(szFileName, "%s", _MENU_HOST_030_CHARTERED_BANK_);
				break;
			case	31:
				sprintf(szFileName, "%s", _MENU_HOST_031_HWATAI_BANK_);
				break;
			case	32:
				sprintf(szFileName, "%s", _MENU_HOST_032_SUNNY_BANK_);
				break;
			case	33:
				sprintf(szFileName, "%s", _MENU_HOST_033_HSBC_BANK_);
				break;
			case	34:
				sprintf(szFileName, "%s", _MENU_HOST_034_ABN_AMRO_BANK_);
				break;
			
			case	95:
				sprintf(szFileName, "%s", _MENU_HOST_095_TAIPEI_FUBON_COMMERCIAL_BANK_);
				break;
			case	96:
				sprintf(szFileName, "%s", _MENU_HOST_096_CATHY_UNITED_BANK_);
				break;	
			case	97:
				sprintf(szFileName, "%s", _MENU_HOST_097_TAISHIN_);
				break;
			case	98:
				sprintf(szFileName, "%s", _MENU_HOST_098_NCCC_);
				break;
			case	99:
				sprintf(szFileName, "%s", _MENU_HOST_099_NEXSYS_);
				break;
			case	901:
				sprintf(szFileName, "%s", _MENU_HOST_901_WE_LINK_);
				break;
			case	902:
				sprintf(szFileName, "%s", _MENU_HOST_902_DIGITALENT_);
				break;
				
			default:
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
					sprintf(szDebugMsg, "No Such LOGONum");
					inLogPrintf(AT, szDebugMsg);
				}
				return (VS_ERROR);
				break;
		}
	}
	
	inDISP_PutGraphic(szFileName, inX,  inY);		/* 顯示 LOGO */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Display_LOGO() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Edit_LOGONum
Date&Time       :2017/11/22 上午 9:27
Describe        :修改logo編號
*/
int inFunc_Edit_LOGONum()
{
	int		inRetVal = VS_SUCCESS;
	char		szTemplate[22 + 1];
        DISPLAY_OBJECT  srDispObj;
	
	inDISP_ClearAll();

	memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	memset(szTemplate, 0x00, sizeof(szTemplate));
	srDispObj.inY = _LINE_8_8_;
	srDispObj.inR_L = _DISP_RIGHT_;
	srDispObj.inMaxLen = 4;
	srDispObj.inColor = _COLOR_BLACK_;
	inRetVal = inGetLOGONum(szTemplate);

	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont_Color("LOGO編號", _FONTSIZE_8X22_, _LINE_8_5_, _COLOR_BLACK_, _DISP_LEFT_);
	inDISP_ChineseFont_Color(szTemplate, _FONTSIZE_8X22_, _LINE_8_6_, _COLOR_BLACK_, _DISP_LEFT_);
	
	while (1)
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_ChineseFont_Color("輸入數字後按OK,放棄按X", _FONTSIZE_8X22_, _LINE_8_7_, _COLOR_BLACK_, _DISP_LEFT_);

		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
		inRetVal = inDISP_Enter8x16(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
		{
			break;
		}
		else if (srDispObj.inOutputLen >= 0)
		{
			inRetVal = inSetLOGONum(srDispObj.szOutput);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			inRetVal = inSaveEDCRec(0);
			if (inRetVal != VS_SUCCESS)
			{
				return (inRetVal);
			}
			
			break;
		}

	}		
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_IP_Transform
Date&Time       :2018/3/2 下午 5:52
Describe        :轉換IP表示形式
*/
int inFunc_IP_Transform(char* szAsciiIP, char* szDecimalIP)
{
	int	i = 0;
	int	inCnt = 0;
	int	inTemp = 0;
	
	for (i = 0; i < strlen(szAsciiIP); i++)
	{
		inTemp = 0;
		while (1)
		{
			if (szAsciiIP[inCnt] == '.'	||
			    szAsciiIP[inCnt] == 0x00)
			{
				inCnt++;
				break;
			}
			else
			{
				inTemp = inTemp * 10 + (szAsciiIP[inCnt] - '0');
				inCnt++;
			}
		}
		
		szDecimalIP[i] = inTemp;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Get_Battery_Status
Date&Time       :2018/3/9 下午 1:52
Describe        :
*/
int inFunc_Get_Battery_Status(unsigned int* uiStatus)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_BatteryStatus(uiStatus);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BatteryStatus() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (usRetVal ==d_BATTERY_NOT_SUPPORT)
	{
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BatteryStatus Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Is_Battery_Exist
Date&Time       :2018/3/9 下午 1:58
Describe        :
*/
int inFunc_Is_Battery_Exist()
{
	int		inRetVal = VS_ERROR;
	unsigned int	uiStatus = 0;

	inRetVal = inFunc_Get_Battery_Status(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		if ((uiStatus & d_MK_BATTERY_EXIST) == d_MK_BATTERY_EXIST)
		{
			return (VS_SUCCESS);
		}
		else
		{
			return (VS_ERROR);
		}
	}
}

/*
Function        :inFunc_Is_Battery_Charging
Date&Time       :2018/3/12 下午 5:37
Describe        :
*/
int inFunc_Is_Battery_Charging()
{
	int		inRetVal = VS_ERROR;
	unsigned int	uiStatus = 0;

	inRetVal = inFunc_Get_Battery_Status(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	else
	{
		if ((uiStatus & d_MK_BATTERY_CHARGE) == d_MK_BATTERY_CHARGE)
		{
			return (VS_SUCCESS);
		}
		else
		{
			return (VS_ERROR);
		}
	}
}

/*
Function        :inFunc_Get_Battery_Capacity
Date&Time       :2018/3/9 下午 2:10
Describe        :
*/
int inFunc_Get_Battery_Capacity(unsigned char* uszPercentage)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_BatteryGetCapacity(uszPercentage);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BatteryGetCapacity Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		*uszPercentage = 0;
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BatteryGetCapacity () OK, (%d%%)", *uszPercentage);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Device_Model_Get
Date&Time       :2018/5/8 下午 9:00
Describe        :
*/
int inFunc_Device_Model_Get(unsigned char* uszModel)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_DeviceModelGet(uszModel);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_DeviceModelGet() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Decide_Machine_Type
Date&Time       :2018/5/8 下午 9:09
Describe        :
*/
int inFunc_Decide_Machine_Type(int* inType)
{
	unsigned char	uszModel = 0;
	ULONG	ulUsedDiskSize = 0;
	ULONG	ulTotalDiskSize = 0;
	ULONG	ulUsedRamSize = 0;
	ULONG	ulTotalRamSize = 0;
	
	inFunc_Device_Model_Get(&uszModel);
	if (uszModel == d_MODEL_VEGA3000)
	{
		if (inFunc_Is_Portable_Type() == VS_TRUE)
		{
			*inType = _CASTLE_TYPE_V3M_;
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "_CASTLE_TYPE_V3M_");
			}
		}
		else
		{
			*inType = _CASTLE_TYPE_V3C_;
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "_CASTLE_TYPE_V3C_");
			}
		}
	}
	else if (uszModel == d_MODEL_VEGA3000P)
	{
		*inType = _CASTLE_TYPE_V3P_;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "_CASTLE_TYPE_V3P_");
		}
	}
	else if (uszModel == d_MODEL_V3UL)
	{
		*inType = _CASTLE_TYPE_V3UL_;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "_CASTLE_TYPE_V3UL_");
		}
	}
	else if (uszModel == d_MODEL_MP200)
	{
		*inType = _CASTLE_TYPE_MP200_;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "_CASTLE_TYPE_MP200_");
		}
	}
	else if (uszModel == d_MODEL_UPT1000)
	{
		*inType = _CASTLE_TYPE_UPT1000_;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "_CASTLE_TYPE_UPT1000_");
		}
	}
	else
	{
		*inType = _CASTLE_TYPE_V3C_;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "_CASTLE_TYPE_V3C_");
		}
	}
	
	inFunc_Is_CONTACTLESS_SUPPORT();
	inFunc_Is_WiFi_Support();
	inFunc_Check_Screen_Size();
	
	/* 分類記憶體類型，回傳單位為KB */
	CTOS_SystemMemoryStatus(&ulUsedDiskSize, &ulTotalDiskSize, &ulUsedRamSize, &ulTotalRamSize);
	gulTotalROMSize = ulTotalDiskSize / 1024;
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "Total(%lu)MB", gulTotalROMSize);
		inLogPrintf(AT, "Used ROM(%lu)Total ROM(%lu)Used RAM(%lu)Total RAM(%lu)", ulUsedDiskSize, ulTotalDiskSize, ulUsedRamSize, ulTotalRamSize);
	}
	vdUtility_SYSFIN_LogMessage(AT, "Used ROM(%lu)Total ROM(%lu)Used RAM(%lu)Total RAM(%lu)", ulUsedDiskSize, ulTotalDiskSize, ulUsedRamSize, ulTotalRamSize);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Get_Termial_Model_Name
Date&Time       :2019/11/15 上午 11:41
Describe        :
*/
int inFunc_Get_Termial_Model_Name(char *szModelName)
{
	int	inType = _CASTLE_TYPE_V3C_;
	
	inFunc_Decide_Machine_Type(&inType);
	
	if (inType == _CASTLE_TYPE_V3C_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_V3C_);
	}
	else if (inType == _CASTLE_TYPE_V3M_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_V3M_);
	}
	else if (inType == _CASTLE_TYPE_V3P_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_V3P_);
	}
	else if (inType == _CASTLE_TYPE_V3UL_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_V3UL_);
	}
	else if (inType == _CASTLE_TYPE_MP200_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_MP200_);
	}
	else if (inType == _CASTLE_TYPE_UPT1000_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_UPT1000_);
	}
	else if (inType == _CASTLE_TYPE_UPT1000F_)
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_UPT1000F_);
	}
	else
	{
		strcpy(szModelName, _CASTLE_TYPE_NAME_UNKNOWN_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_Print_Capability
Date&Time       :2018/5/8 下午 9:05
Describe        :
*/
int inFunc_Check_Print_Capability(int inType)
{
	if (inType == _CASTLE_TYPE_V3C_	||
	    inType == _CASTLE_TYPE_V3M_)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Have Print Capability");
		}
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "NO Print Capability");
		}
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_UpdateAP_Preserve
Date&Time       :2018/6/23 下午 10:07
Describe        :更新AP前先儲存會被清掉的東西，之後可以考慮用XML存
*/
int inFunc_UpdateAP_Preserve()
{
	char		szTemplate[50 + 1] = {0};
	xmlDocPtr       srDoc = NULL;
        xmlNodePtr      srRoot_Node = NULL;
	
	/* 刪除舊有xml */
	inFile_Unlink_File(_TMS_PRESERVE_XML_FILE_NAME_, _FS_DATA_PATH_);
	
	/* 建立一個新的檔案 */
        srDoc = xmlNewDoc(BAD_CAST "1.0");
        srRoot_Node = xmlNewNode(NULL, BAD_CAST "NEXSYS");
        xmlDocSetRootElement (srDoc, srRoot_Node);
	
	inLoadEDCRec(0);
	
	/* IP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTermIPAddress(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_EDC_IP_, BAD_CAST szTemplate);
	
	/* MASK */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTermMASKAddress(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_MASK_IP_, BAD_CAST szTemplate);
	
	/* GATEWAY */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTermGetewayAddress(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_GATEWAY_IP_, BAD_CAST szTemplate);
	
	inLoadTMSFTPRec(0);
	/* FTP IP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetTMSIPAddress(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_FTP_IP_, BAD_CAST szTemplate);
	
	/*FTP PORT */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetFTPPortNum(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_FTP_PORT_, BAD_CAST szTemplate);
	
	/* szSupECR_UDP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSupECR_UDP(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_SUP_ECR_UDP_, BAD_CAST szTemplate);
	
	/* szPOS_IP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetPOS_IP(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_POS_IP_, BAD_CAST szTemplate);
	
	/* szPOS_ID */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetPOS_ID(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_POS_ID_, BAD_CAST szTemplate);
	
	/* szGetGPRS_APN */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetGPRS_APN(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_GPRS_APN_, BAD_CAST szTemplate);
	
	/* szSignpadBeepInterval */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSignpadBeepInterval(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_SIGNPAD_BEEP_INTERVAL_, BAD_CAST szTemplate);
	
	/* PWMEnable */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetPWMEnable(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_PWM_ENABLE_, BAD_CAST szTemplate);
	
	/* PWMMode */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetPWMMode(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_PWM_MODE_, BAD_CAST szTemplate);
	
	/* PWMIdleTimeout */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetPWMIdleTimeout(szTemplate);
	xmlNewChild(srRoot_Node, NULL, BAD_CAST _TMS_PRESERVE_TAG_PWM_IDLE_TIMEOUT_, BAD_CAST szTemplate);
	
	/* 儲存xml檔 xml檔現在會自動換行了 */
        inXML_SaveFile(_TMS_PRESERVE_XML_FILE_NAME_, &srDoc, "utf-8", 1);
	
	/* 清空佔用記憶體 */
	inXML_End(&srDoc);
	
	inFile_Move_File(_TMS_PRESERVE_XML_FILE_NAME_, _AP_ROOT_PATH_, "", _FS_DATA_PATH_);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_UpdateAP_Recover
Date&Time       :2018/6/23 下午 10:07
Describe        :更新AP後回復會被清掉的東西，之後可以考慮用XML
*/
int inFunc_UpdateAP_Recover()
{
	int		inRetVal = VS_ERROR;
	char		szTagData[1536 + 1] = {0};
	char		szFileName_New[50 + 1] = {0};
	char		szDebugMsg[100 + 1] = {0};
	char		szTemplate[100 + 1] = {0};
	xmlChar*	szTagPtr = NULL;
	xmlDocPtr       srDoc = NULL;
	xmlNodePtr	srCur = NULL;		/* (第二層) */
	xmlNodePtr	srTagNode = NULL;	/* 用來移動Tag那一層(第三層) */
	xmlNodePtr	srTextNode = NULL;	/* 因為ElementNode的子節點TextNode才有內容(第四層) */
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_UpdateAP_Recover() START !");
	}
	
	if (inFILE_Check_Exist((unsigned char*)_TMS_PRESERVE_XML_FILE_NAME_) != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inFunc_UpdateAP_Recover file not exist");
		}
		return (VS_ERROR);
	}
	
	sprintf(szFileName_New, "%s%s", _FS_DATA_PATH_, _TMS_PRESERVE_XML_FILE_NAME_);
	inRetVal = inXML_ReadFile(szFileName_New, &srDoc, NULL, XML_PARSE_RECOVER | XML_PARSE_PEDANTIC);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	inRetVal = inXML_Get_RootElement(&srDoc, &srCur);
	/* 異常 */
	if (inRetVal != VS_SUCCESS)
	{
		inXML_End(&srDoc);
		
		return (VS_ERROR);
	}
	
	inLoadEDCRec(0);
	inLoadTMSFTPRec(0);
	
	/* 換下一個節點檢查，往下走到TRANS的子節點，接著開始往橫走 */
	while (srCur != NULL)
	{
		/* 每次都設為NULL，如果有要查看的話才設定位置 */
		srTagNode = NULL;
		if (memcmp(srCur->name, _TMS_PRESERVE_TAG_NEXSYS_, _TMS_PRESERVE_TAG_NEXSYS_LEN_) == 0	&&
	            srCur->children != NULL)
		{
			srTagNode = srCur ->children;
		}
		
		/* 推移第三層 */
		while (srTagNode != NULL)
		{
			memset(szTagData, 0x00, sizeof(szTagData));
			
			/* 有分XML_TEXT_NODE和XML_ELEMENT_NODE 才有content可抓 */
			if (srTagNode->type == XML_ELEMENT_NODE	&&
			   (srTagNode->children != NULL		&&
			    srTagNode->children->type == XML_TEXT_NODE))
			{
				srTextNode = srTagNode->children;
				memset(szTagData, 0x00, sizeof(szTagData));
				szTagPtr = xmlNodeListGetString(srDoc, srTextNode, srTextNode->line);
				if (szTagPtr != NULL	&&
				    xmlStrlen(szTagPtr) != 0)
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s : %s", srTagNode->name, szTagPtr);
						inLogPrintf(AT, szDebugMsg);
					}

					strcat(szTagData, (char*)szTagPtr);
					xmlFree(szTagPtr);
				}
			}

			/* IP */
			if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_EDC_IP_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetTermIPAddress(szTagData);
				}
			}
			/* MASK */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_MASK_IP_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetTermMASKAddress(szTagData);
				}
			}
			/* Gateway */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_GATEWAY_IP_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inGetTermGetewayAddress(szTagData);
				}
			}
			/* FTP IP */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_FTP_IP_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetFTPIPAddress(szTagData);
				}
			}
			/* FTP PORT */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_FTP_PORT_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetFTPPortNum(szTagData);
				}
			}
			/* szSupECR_UDP */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_SUP_ECR_UDP_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetSupECR_UDP(szTagData);
				}
			}
			/* szPOS_IP */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_POS_IP_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetPOS_IP(szTagData);
				}
			}
			/* szPOS_ID */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_POS_ID_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetPOS_ID(szTagData);
				}
			}
			/* szGPRS_APN */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_GPRS_APN_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetGPRS_APN(szTagData);
				}
			}
			/* szSignpadBeepInterval */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_SIGNPAD_BEEP_INTERVAL_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetSignpadBeepInterval(szTagData);
				}
			}
			/* PWM_ENABLE */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_PWM_ENABLE_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetPWMEnable(szTagData);
				}
			}
			/* PWM_MODE */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_PWM_MODE_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetPWMMode(szTagData);
				}
			}
			/* PWM_IDLE_TIMEOUT */
			else if (xmlStrcmp(srTagNode->name, (const xmlChar*)_TMS_PRESERVE_TAG_PWM_IDLE_TIMEOUT_) == 0)
			{
				if (strlen(szTagData) > 0)
				{
					inSetPWMIdleTimeout(szTagData);
				}
			}
			
			srTagNode = srTagNode->next;
		}
			
		/* 推移第二層 */
		srCur = srCur->next;
	}
	
	/* 給初始值 */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetUDP_Port(szTemplate);
	if (strlen(szTemplate) == 0)
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%d", _ECR_UDP_DEFAULT_PORT_);
		inSetUDP_Port(szTemplate);
	}
	
	inSaveEDCRec(0);
	inSaveTMSFTPRec(0);
	
	/* free */
	inXML_End(&srDoc);
	
	/* 回存完刪除檔案 */
	inFile_Unlink_File(_TMS_PRESERVE_XML_FILE_NAME_, _FS_DATA_PATH_);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_UpdateAP_Recover() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Table_Delete_Record
Date&Time       :2018/7/5 上午 9:15
Describe        :
*/
int inFunc_Table_Delete_Record(char *szFileName, int inRecordIndex)
{
	/* lnTotalLen : 該檔案全長度 ,		inRecLen : 一個Record的長度 */
	/* lnFinalToalLen :Resort完的長度，用來確認檔案完整性 */
        /* szFile : 該檔案全部內容 ,		szRecord : 該Record全部內容 */

	int			i;
	int			inOldRecLen = 0;		/* Old Record 長度 */
	int			inRecCnt = 0;
	int			inOldRecStartOffset = 0;	/* Old Record 起始位置 */
	int			inRetVal;
        char			*szFile, *szRecord;
	long			lnTotalLen = 0;
	long			lnLeftSize = 0;			/* 剩餘長度 */
	unsigned long		ulHandle;
	
	/* index不合法 */
	if (inRecordIndex == -1)
	{
		return (VS_ERROR);
	}

        inRetVal = inFILE_Open(&ulHandle, (unsigned char*)szFileName);
        if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
        
	/* 算總長度 */
        lnTotalLen = lnFILE_GetSize(&ulHandle, (unsigned char*)szFileName);
	
	/* 讀出HDT所有資料 */
        inFILE_Seek(ulHandle, 0L, _SEEK_BEGIN_);
	szFile = malloc((lnTotalLen + 1) * sizeof(char));
	memset(szFile, 0x00, lnTotalLen * sizeof(char));
	lnLeftSize = lnTotalLen;
	
	for (i = 0;; ++i)
	{
		/* 剩餘長度大於或等於1024 */
		if (lnLeftSize >= 1024)
		{
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], 1024) == VS_SUCCESS)
			{
				/* 一次讀1024 */
				lnLeftSize -= 1024;

				/* 當剩餘長度剛好為1024，會剛好讀完 */
				if (lnLeftSize == 0)
					break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
		}
		/* 剩餘長度小於1024 */
		else if (lnLeftSize < 1024)
		{
			/* 就只讀剩餘長度 */
			if (inFILE_Read(&ulHandle, (unsigned char*)&szFile[1024 * i], lnLeftSize) == VS_SUCCESS)
			{
				break;
			}
			/* 讀失敗時 */
			else
			{
				/* Close檔案 */
				inFILE_Close(&ulHandle);

				/* Free pointer */
				free(szFile);

				return (VS_ERROR);
			}
			
		}
		
	}
	
	/* 算出Old Record的長度 */
	for (i = 0, inRecCnt = 0; i < lnTotalLen; i++)
	{
		/* Record 長度 */
		if (inRecCnt == inRecordIndex)
		{
			inOldRecLen++;
		}
		/* 若大於，代表已經算完，要跳出 */
		else if (inRecCnt > inRecordIndex)
		{
			break;
		}
		
		/* Record 結尾 */
		if (szFile[i] == 0x0A && szFile[i - 1] == 0x0D)
		{
			inRecCnt++;
			
			/* 算出Record起始位置 若是第0 Record，永遠不會進去，則inOldRecStartOffset 為 0 */
			if (inRecCnt == inRecordIndex)
			{
				inOldRecStartOffset = i + 1;
			}
		}
		
	}
	
	/* 移到Record的起始位置 */
	if (inFILE_Seek(ulHandle, inOldRecStartOffset, _SEEK_BEGIN_) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 讀出Record的資料 */
	szRecord = malloc((inOldRecLen + 1) * sizeof(char));
	memset(szRecord, 0x00, inOldRecLen * sizeof(char));
	if (inFILE_Read(&ulHandle, (unsigned char*)szRecord, inOldRecLen) != VS_SUCCESS)
	{
		inFILE_Close(&ulHandle);
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 因這裡V3模仿Verifone的insert和delete，但用自己的方式實做，所以需要先關檔再由該function操作*/
	if (inFILE_Close(&ulHandle) != VS_SUCCESS)
	{
		free(szRecord);
		free(szFile);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
        free(szFile);
	
	if (inFILE_Data_Delete((unsigned char*)szFileName, inOldRecStartOffset, inOldRecLen) != VS_SUCCESS)	/* 刪除DCC那一段 */
	{
		/* 釋放記憶體 */
		free(szRecord);
		
		return (VS_ERROR);
	}
	
	/* 釋放記憶體 */
	free(szRecord);

        return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_SDCard_Mounted
Date&Time       :2018/7/24 下午 1:34
Describe        :參考條件:
 *		"ls -l /sys/block/ | grep 'mmc'"
*/
int inFunc_Check_SDCard_Mounted(void)
{
	int	inRetVal = VS_ERROR;
	char	szCommand[100 + 1] = {0};
	
	memset(szCommand, 0x00, sizeof(szCommand));
	sprintf(szCommand, "mount | grep '/media/mdisk'");
	
	inRetVal = inFunc_ShellCommand_System(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SD Not Mounted");
		}
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "SD Mounted");
		}
		
		return (VS_SUCCESS);
	}
}

/*
Function        :inFunc_Check_USB_Mounted
Date&Time       :2019/2/23 上午 11:50
Describe        :參考條件：
 *		system("ls /sys/block/ | grep 'sd'") == 0 &&
 *		system("find /dev/ -name 'sd*'") == 0
*/
int inFunc_Check_USB_Mounted(void)
{
	int	inRetVal = VS_ERROR;
	char	szCommand[100 + 1] = {0};
	
	/* 有掛載udisk */
	memset(szCommand, 0x00, sizeof(szCommand));
	sprintf(szCommand, "mount | grep '/media/udisk'");

	inRetVal = inFunc_ShellCommand_System(szCommand);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "USB Not Mounted");
		}

		inRetVal = VS_ERROR;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "USB Mounted");
		}

		inRetVal = VS_SUCCESS;
	}
	
	return (inRetVal);
}

/*
Function	:inFunc_ECR_Comport_Switch
Date&Time	:2018/7/27 上午 9:50
Describe        :
*/
int inFunc_ECR_Comport_Switch(void)
{
        char		szTemplate[64 + 1] = {0};
	char		szDispMsg[64 + 1] = {0};
	unsigned char	uszKey = 0x00;
        
        while (1)
        {
		/* Comport */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetECRComPort(szTemplate);

		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "ECR Comport: %s", szTemplate);

		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("0 = USB1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("1 = COM1", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		inDISP_ChineseFont("2 = COM2", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
                uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_0_)
		{
			inSetECRComPort(_COMPORT_USB1_);
			inSaveEDCRec(0);
		}
		else if (uszKey == _KEY_1_)
		{
			inSetECRComPort(_COMPORT_COM1_);
			inSaveEDCRec(0);
		}
		else if (uszKey == _KEY_2_)
		{
			inSetECRComPort(_COMPORT_COM2_);
			inSaveEDCRec(0);
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else
		{
			continue;
		}
        }
        
        
        return (VS_SUCCESS);
}

/*
Function        :inFunc_Decide_APVersion_Type
Date&Time       :2018/8/13 下午 2:23
Describe        :
*/
int inFunc_Decide_APVersion_Type(int* inType)
{
	char	szHostLabel[8 + 1] = {0};
	
	memset(szHostLabel, 0x00, sizeof(szHostLabel));
	inLoadHDTRec(0);
	inGetHostLabel(szHostLabel);
	
	if (memcmp(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
	{
		*inType = _APVERSION_TYPE_NCCC_;
	}
	else if (0)
	{
		
	}
	else if (0)
	{
		
	}
	else if (0)
	{
		
	}
	else
	{
		*inType = _APVERSION_TYPE_NCCC_;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Default: NCCC");
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_AuthCode_Validate
Date&Time       :2018/9/28 下午 2:34
Describe        :
*/
int inFunc_Check_AuthCode_Validate(char* szAuthCode)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	
	if (szAuthCode == NULL)
	{
		return (VS_SUCCESS);
	}
	
	for (i = 0; i < strlen(szAuthCode); i++)
	{
		if (((szAuthCode[i] >= '0') && (szAuthCode[i] <= '9')) ||
		    ((szAuthCode[i] >= 'A') && (szAuthCode[i] <= 'Z')) ||
		    ((szAuthCode[i] >= 'a') && (szAuthCode[i] <= 'z')) ||
		    (szAuthCode[i] == 0x20))
		{
			continue;
		}
		else
		{
			inRetVal = VS_ERROR;
			break;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_Log_All_Firmware_Version
Date&Time       :2018/10/1 下午 1:40
Describe        :
*/
int inFunc_Log_All_Firmware_Version(void)
{
	int	inRetVal = VS_SUCCESS;
	int	i = 0;
	int	inID = 0;
	char	szIDName[20 + 1] = {0};
	char	szBuffer[50 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	
	for (i = 0; i <= ID_MAXIMUM; i++)
	{
		inID = i;
		memset(szIDName, 0x00, sizeof(szIDName));
		switch (inID)
		{
			case ID_BOOTSULD:
				sprintf(szIDName, "%s", "BOOTSULD");
				break;
			case ID_CRYPTO_HAL:
				sprintf(szIDName, "%s", "CRYPTO_HAL");
				break;
			case ID_LINUX_KERNEL:
				sprintf(szIDName, "%s", "LINUX_KERNEL");
				break;
			case ID_SECURITY_KO:
				sprintf(szIDName, "%s", "SECURITY_KO");
				break;
			case ID_SYSUPD_KO:
				sprintf(szIDName, "%s", "SYSUPD_KO");
				break;
			case ID_KMS:
				sprintf(szIDName, "%s", "KMS");
				break;
			case ID_CADRV_KO:
				sprintf(szIDName, "%s", "CADRV_KO");
				break;
			case ID_CAUSB_KO:
				sprintf(szIDName, "%s", "CAUSB_KO");
				break;
			case ID_LIBCAUART_SO:
				sprintf(szIDName, "%s", "LIBCAUART_SO");
				break;
			case ID_LIBCAUSBH_SO:
				sprintf(szIDName, "%s", "LIBCAUSBH_SO");
				break;
			case ID_LIBCAMODEM_SO:
				sprintf(szIDName, "%s", "LIBCAMODEM_SO");
				break;
			case ID_LIBCAETHERNET_SO:
				sprintf(szIDName, "%s", "LIBCAETHERNET_SO");
				break;
			case ID_LIBCAFONT_SO:
				sprintf(szIDName, "%s", "LIBCAFONT_SO");
				break;
			case ID_LIBCALCD_SO:
				sprintf(szIDName, "%s", "LIBCALCD_SO");
				break;
			case ID_LIBCAPRT_SO:
				sprintf(szIDName, "%s", "LIBCAPRT_SO");
				break;
			case ID_LIBCARTC_SO:
				sprintf(szIDName, "%s", "LIBCARTC_SO");
				break;
			case ID_LIBCAULDPM_SO:
				sprintf(szIDName, "%s", "LIBCAULDPM_SO");
				break;
			case ID_LIBCAPMODEM_SO:
				sprintf(szIDName, "%s", "LIBCAPMODEM_SO");
				break;
			case ID_LIBCAGSM_SO:
				sprintf(szIDName, "%s", "LIBCAGSM_SO");
				break;
			case ID_LIBCAEMVL2_SO:
				sprintf(szIDName, "%s", "LIBCAEMVL2_SO");
				break;
			case ID_LIBCAKMS_SO:
				sprintf(szIDName, "%s", "LIBCAKMS_SO");
				break;
			case ID_LIBCAFS_SO:
				sprintf(szIDName, "%s", "LIBCAFS_SO");
				break;
			case ID_LIBCABARCODE_SO:
				sprintf(szIDName, "%s", "LIBCABARCODE_SO");
				break;
			case ID_CRADLE_MP:
				sprintf(szIDName, "%s", "CRADLE_MP");
				break;
			case ID_LIBTLS_SO:
				sprintf(szIDName, "%s", "LIBTLS_SO");
				break;
			case ID_LIBCLVW_SO:
				sprintf(szIDName, "%s", "LIBCLVW_SO");
				break;
			case ID_LIBCTOSAPI_SO:
				sprintf(szIDName, "%s", "LIBCTOSAPI_SO");
				break;
			case ID_SAM_KO:
				sprintf(szIDName, "%s", "SAM_KO");
				break;
			case ID_CLVWM_MP:
				sprintf(szIDName, "%s", "CLVWM_MP");
				break;
			case ID_ROOTFS:
				sprintf(szIDName, "%s", "ROOTFS");
				break;
			case ID_BIOS:
				sprintf(szIDName, "%s", "BIOS");
				break;
			case ID_CIF_KO:
				sprintf(szIDName, "%s", "CIF_KO");
				break;
			case ID_CLDRV_KO:
				sprintf(szIDName, "%s", "CLDRV_KO");
				break;
			case ID_TMS:
				sprintf(szIDName, "%s", "TMS");
				break;
			case ID_ULDPM:
				sprintf(szIDName, "%s", "ULDPM");
				break;
			case ID_SC_KO:
				sprintf(szIDName, "%s", "SC_KO");
				break;
			case ID_EMV_SO:
				sprintf(szIDName, "%s", "EMV_SO");
				break;
			case ID_EMVCL_SO:
				sprintf(szIDName, "%s", "EMVCL_SO");
				break;
			default	:
				break;
		}
		
		memset(szBuffer, 0x00, sizeof(szBuffer));
		inRetVal = inFunc_GetSystemInfo(inID, (unsigned char*)szBuffer);
		if (inRetVal == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%s:%s", szIDName, szBuffer);
				inLogPrintf(AT, szDebugMsg);
			}
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Display_Msg_Instead_For_Print_Receipt
Date&Time       :2018/10/5 下午 2:33
Describe        :無印表機時替代顯示
*/
int inFunc_Display_Msg_Instead_For_Print_Receipt(TRANSACTION_OBJECT *pobTran)
{
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		/* 節省時間，不顯示 */
//		if (pobTran->inTransactionCode == _PRE_AUTH_)
//		{
//			DISPLAY_OBJECT	srDispMsgObj;
//			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
//			strcpy(srDispMsgObj.szDispPic1Name, "");
//			srDispMsgObj.inDispPic1YPosition = 0;
//			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
//			srDispMsgObj.inTimeout = 2;
//			strcpy(srDispMsgObj.szErrMsg1, "預先授權成功");
//			srDispMsgObj.inErrMsgLine1 = _LINE_8_6_;
//			inDISP_Msg_BMP("", 0, _NO_KEY_MSG_, 2, "預先授權成功", _LINE_8_6_);
//		}
//		else if (pobTran->inTransactionCode == _PRE_COMP_)
//		{
//			DISPLAY_OBJECT	srDispMsgObj;
//			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
//			strcpy(srDispMsgObj.szDispPic1Name, "");
//			srDispMsgObj.inDispPic1YPosition = 0;
//			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
//			srDispMsgObj.inTimeout = 2;
//			strcpy(srDispMsgObj.szErrMsg1, "預先授權完成成功");
//			srDispMsgObj.inErrMsgLine1 = _LINE_8_6_;
//			inDISP_Msg_BMP("", 0, _NO_KEY_MSG_, 2, "預先授權完成成功", _LINE_8_6_);
//		}
//		else
//		{
//			DISPLAY_OBJECT	srDispMsgObj;
//			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
//			strcpy(srDispMsgObj.szDispPic1Name, _TRANSACTION_SUCCESS_);
//			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_4_;
//			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
//			srDispMsgObj.inTimeout = 2;
//			strcpy(srDispMsgObj.szErrMsg1, "");
//			srDispMsgObj.inErrMsgLine1 = 0;
//			inDISP_Msg_BMP(_TRANSACTION_SUCCESS_, _COORDINATE_Y_LINE_8_4_, _NO_KEY_MSG_, 2, "", 0);
//		}
	}
	else
	{
		/* (需求單 - 107276)自助交易標準做法 EDC於發動交易及完成時，需發出提示音 by Russell 2019/3/8 上午 10:46 */
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)		||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _TRANSACTION_SUCCESS_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_4_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
		else
		{
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _TRANSACTION_SUCCESS_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_4_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 2;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			inDISP_Msg_BMP(&srDispMsgObj);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function	:inFunc_KBDLock_Switch
Date&Time	:2018/12/8 下午 9:54
Describe        :
*/
int inFunc_KBDLock_Switch(void)
{
        char		szTemplate[64 + 1] = {0};
	char		szDispMsg[64 + 1] = {0};
	unsigned char	uszKey = 0x00;
        
        while (1)
        {
		/* KBDLock */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetKBDLock(szTemplate);

		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "KBDLock: %s", szTemplate);

		inDISP_ChineseFont(szDispMsg, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("0 = N", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("1 = Y", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	
                uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_0_)
		{
			inSetKBDLock("N");
			inSaveEDCRec(0);
		}
		else if (uszKey == _KEY_1_)
		{
			inSetKBDLock("Y");
			inSaveEDCRec(0);
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else
		{
			continue;
		}
        }
        
        return (VS_SUCCESS);
}
/*
Function        :inFunc_Check_HDPT_ERROR
Date&Time       :2018/12/14 上午 11:37
Describe        :
*/
int inFunc_Check_HDPT_ERROR(void)
{
	int	inRetVal = VS_ERROR;
	char	szTRTFName[12 + 1] = {0};
	char	szHostLabel[8 + 1] = {0};
	
	inLoadHDPTRec(0);
	memset(szTRTFName, 0x00, sizeof(szTRTFName));
	inGetTRTFileName(szTRTFName);
	memset(szHostLabel, 0x00, sizeof(szHostLabel));
	inGetHostLabel(szHostLabel);
	
	if (!memcmp(szTRTFName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
	{
		if (memcmp(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) == 0)
		{
			
		}
		else
		{
			inRetVal = VS_ERROR;
		}
	}
		
	return (VS_SUCCESS);
};

/*
Function        :inFunc_Reboot_Function_Table
Date&Time       :2019/3/8 上午 10:32
Describe        :為了放進OPT使用
*/
int inFunc_Reboot_Function_Table(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	
	inRetVal = inFunc_Reboot();
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Load_MVT_By_Index
Date&Time       :2019/6/25 下午 1:48
Describe        :
*/
int inFunc_Load_MVT_By_Index(char* szExpectApplicationIndex)
{
	int	inRetVal = VS_SUCCESS;
	int	inMVTIndex = 0;
	char	szApplicationIndex[2 + 1] = {0};
	
	if (strlen(szExpectApplicationIndex) <= 0)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "szExpectApplicationIndex null");
		}
		return (VS_ERROR);
	}
	
	for (inMVTIndex = 0; inMVTIndex < _MVT_MAX_INDEX_; inMVTIndex++)
	{
		inRetVal = inLoadMVTRec(inMVTIndex);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "MVT 中無此感應卡");
			}

			inRetVal = VS_WAVE_ERROR;
			break;
		}

		memset(szApplicationIndex, 0x00, sizeof(szApplicationIndex));
		inGetApplicationIndex(szApplicationIndex);
		if (memcmp(szApplicationIndex, szExpectApplicationIndex, strlen(szExpectApplicationIndex)) == 0)
		{
			break;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_Check_Host_Must_Settle
Date&Time       :2019/8/7 下午 4:15
Describe        :
*/
int inFunc_Check_Host_Must_Settle(TRANSACTION_OBJECT *pobTran)
{
	int inRetVal = VS_SUCCESS;
	
	
	
	
	
	
	
	return (inRetVal);
}

/*
Function        :inFunc_Save_Last_Txn_Host
Date&Time       :2019/8/15 下午 2:59
Describe        :在./fs_data/底下建立LAST_TXN_HOST.txt， 紀錄當筆pobTran->srBRec.inHDTIndex
*/
/* 【需求單 - 108128】	單機重印前筆簽單流程新增畫面 by Russell 2019/8/15 下午 2:59 */
int inFunc_Save_Last_Txn_Host(TRANSACTION_OBJECT *pobTran)
{
	int	inFd = 0;
	int	inRetVal = VS_ERROR;
	int	inBufferSize = 0;
	int	inNCCCIndex = -1;
	int	inDCCIndex = -1;
	int	inHGIndex = -1;
	char	szBuffer[20 + 1] = {0};
	char	szCustomerIndicator[3 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Save_Last_Txn_Host() START !");
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/30 下午 1:30
	 * 客製化075、103只紀錄NCCC和DCC和HG */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inNCCCIndex);
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inDCCIndex);
		inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHGIndex);
		
		/* 是這三個Host繼續做，否則跳過 */
		if (pobTran->srBRec.inHDTIndex == inNCCCIndex	||
		    pobTran->srBRec.inHDTIndex == inDCCIndex	||
		    pobTran->srBRec.inHDTIndex == inHGIndex)
		{
			
		}
		else
		{
			return (VS_SUCCESS);
		}
	}
	
	inFile_Linux_Delete_In_Fs_Data(_LAST_TXN_HOST_FILE_NAME_);
	
	inRetVal = inFile_Linux_Create_In_Fs_Data(&inFd, _LAST_TXN_HOST_FILE_NAME_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inFunc_Save_Last_Txn_Host create fail");
		}
		
		return (VS_ERROR);
	}

	/* rwxr--r--*/
	inFunc_Data_Chmod("744", _LAST_TXN_HOST_FILE_NAME_, _FS_DATA_PATH_);

	do
	{
		inRetVal = inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		memset(szBuffer, 0x00, sizeof(szBuffer));
		sprintf(szBuffer, "%02d", pobTran->srBRec.inHDTIndex);
		inBufferSize = strlen(szBuffer);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "lasthost: %s", szBuffer);
		}
		
		inRetVal = inFile_Linux_Write(inFd, szBuffer, &inBufferSize);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		break;
	}while(1);

	inFile_Linux_Close(inFd);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Save_Last_Txn_Host() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inFunc_Load_Last_Txn_Host
Date&Time       :2019/8/15 下午 2:59
Describe        :
*/
/* 【需求單 - 108128】	單機重印前筆簽單流程新增畫面 by Russell 2019/8/15 下午 2:59 */
int inFunc_Load_Last_Txn_Host(int* inHDTindex)
{
	int	inFd = 0;
	int	inRetVal = VS_ERROR;
	int	inBufferSize = 0;
	char	szBuffer[20 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Load_Last_Txn_Host() START !");
	}
	
	inRetVal = inFile_Linux_Open_In_Fs_Data(&inFd, _LAST_TXN_HOST_FILE_NAME_);
	if (inRetVal != VS_SUCCESS)
	{
		*inHDTindex = -1;
		
		return (inRetVal);
	}
	
	do
	{
		inRetVal = inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		memset(szBuffer, 0x00, sizeof(szBuffer));
		inBufferSize = sizeof(szBuffer);
		inRetVal = inFile_Linux_Read(inFd, szBuffer, &inBufferSize);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		if (inBufferSize > 0)
		{
			*inHDTindex = atoi(szBuffer);
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "LastHost:%d", *inHDTindex);
			}
		}
		else
		{
			inRetVal = VS_ERROR;
			break;
		}

		break;
	}while(1);

	inFile_Linux_Close(inFd);
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Load_Last_Txn_Host() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (inRetVal);
}

/*
Function        :inFunc_Menu_Sam_Slot_PowerOn
Date&Time       :2019/8/30 下午 3:28
Describe        :
*/
int inFunc_Menu_Sam_Slot_PowerOn(void)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszSlot = _SAM_SLOT_1_;
	unsigned char	uszKey = 0x00;
	
	inDISP_ClearAll();
	inDISP_ChineseFont("選擇Slot", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("1.Slot 1", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.Slot 2", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.Slot 3", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.Slot 4", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			uszSlot = _SAM_SLOT_1_;
			inRetVal = inNCCC_tSAM_PowerOn_Flow(&uszSlot);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ChineseFont("Slot Power On 成功", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			else
			{
				inDISP_ChineseFont("Slot Power On 失敗", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			break;
		}
		else if (uszKey == _KEY_2_)
		{
			uszSlot = _SAM_SLOT_2_;
			inRetVal = inNCCC_tSAM_PowerOn_Flow(&uszSlot);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ChineseFont("Slot Power On 成功", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			else
			{
				inDISP_ChineseFont("Slot Power On 失敗", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			break;
		}
		else if (uszKey == _KEY_3_)
		{
			uszSlot = _SAM_SLOT_3_;
			inRetVal = inNCCC_tSAM_PowerOn_Flow(&uszSlot);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ChineseFont("Slot Power On 成功", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			else
			{
				inDISP_ChineseFont("Slot Power On 失敗", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			break;
		}
		else if (uszKey == _KEY_4_)
		{
			uszSlot = _SAM_SLOT_4_;
			inRetVal = inNCCC_tSAM_PowerOn_Flow(&uszSlot);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ChineseFont("Slot Power On 成功", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			else
			{
				inDISP_ChineseFont("Slot Power On 失敗", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_Menu_Sam_Slot_Select_NCCC_AID
Date&Time       :2019/8/30 下午 4:20
Describe        :
*/
int inFunc_Menu_Sam_Slot_Select_NCCC_AID(void)
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszSlot = _SAM_SLOT_1_;
	unsigned char	uszKey = 0x00;
	
	inDISP_ClearAll();
	inDISP_ChineseFont("選擇Slot", _FONTSIZE_8X16_, _LINE_8_1_, _DISP_LEFT_);

	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("1.Slot 1", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("2.Slot 2", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("3.Slot 3", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("4.Slot 4", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		
		uszKey = uszKBD_GetKey(30);
		if (uszKey == _KEY_1_)
		{
			uszSlot = _SAM_SLOT_1_;
			inRetVal = inNCCC_tSAM_SelectAID_NCCC(uszSlot);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ChineseFont("Slot Select AID 成功", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			else
			{
				inDISP_ChineseFont("Slot Select AID 失敗", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			break;
		}
		else if (uszKey == _KEY_2_)
		{
			uszSlot = _SAM_SLOT_2_;
			inRetVal = inNCCC_tSAM_SelectAID_NCCC(uszSlot);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ChineseFont("Slot Select AID 成功", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			else
			{
				inDISP_ChineseFont("Slot Select AID 失敗", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			break;
		}
		else if (uszKey == _KEY_3_)
		{
			uszSlot = _SAM_SLOT_3_;
			inRetVal = inNCCC_tSAM_SelectAID_NCCC(uszSlot);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ChineseFont("Slot Select AID 成功", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			else
			{
				inDISP_ChineseFont("Slot Select AID 失敗", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			break;
		}
		else if (uszKey == _KEY_4_)
		{
			uszSlot = _SAM_SLOT_4_;
			inRetVal = inNCCC_tSAM_SelectAID_NCCC(uszSlot);
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ChineseFont("Slot Select AID 成功", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			else
			{
				inDISP_ChineseFont("Slot Select AID 失敗", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
				inDISP_Wait(2000);
			}
			break;
		}
		else if (uszKey == _KEY_TIMEOUT_)
		{
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			break;
		}
		else
		{
			
		}
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_Is_CONTACTLESS_SUPPORT
Date&Time       :2019/10/8 下午 4:22
Describe        :確認是否支援感應機型
*/
int inFunc_Is_CONTACTLESS_SUPPORT(void)
{
	char		szDebugMsg[100 + 1];
	unsigned char	uszPortable;
	unsigned char	uszPCI;
	unsigned short	usHWSupport;
	unsigned short	usRetVal;

	usRetVal = CTOS_HWSettingGet(&uszPortable, &uszPCI, &usHWSupport);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_HWSettingGet Err: 0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "usHWSupport : 0x%04X", usHWSupport);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if ((usHWSupport & d_MK_HW_CONTACTLESS) == d_MK_HW_CONTACTLESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			strcpy(szDebugMsg, "contactless support!");
			inLogPrintf(AT, szDebugMsg);
		}
		ginContactlessSupport = VS_TRUE;
		
		return (VS_TRUE);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			strcpy(szDebugMsg, "contactless not support!");
			inLogPrintf(AT, szDebugMsg);
		}
		ginContactlessSupport = VS_FALSE;
		
		return (VS_FALSE);
	}	
}

/*
Function        :inFunc_Is_WiFi_Support
Date&Time       :2020/3/6 下午 5:26
Describe        :確認是否支援WiFi
*/
int inFunc_Is_WiFi_Support(void)
{
	char		szDebugMsg[100 + 1];
	unsigned char	uszPortable;
	unsigned char	uszPCI;
	unsigned short	usHWSupport;
	unsigned short	usRetVal;

	usRetVal = CTOS_HWSettingGet(&uszPortable, &uszPCI, &usHWSupport);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_HWSettingGet Err: 0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "usHWSupport : 0x%04X", usHWSupport);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if ((usHWSupport & d_MK_HW_WIFI) == d_MK_HW_WIFI)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			strcpy(szDebugMsg, "WiFi support!");
			inLogPrintf(AT, szDebugMsg);
		}
		ginWiFiSupport = VS_TRUE;
		
		return (VS_TRUE);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			strcpy(szDebugMsg, "WiFi not support!");
			inLogPrintf(AT, szDebugMsg);
		}
		ginWiFiSupport = VS_FALSE;
		
		return (VS_FALSE);
	}	
}

/*
Function        :inFunc_PowerKeyFunGet
Date&Time       :2019/10/21 下午 1:39
Describe        :
*/
int inFunc_PowerKeyFunGet(unsigned char *uszFunc)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_PowerKeyFunGet(uszFunc);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerKeyFunGet Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerKeyFunGet(%s) OK", uszFunc);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_PowerKeyFunSet
Date&Time       :2019/10/21 下午 1:53
Describe        :
*/
int inFunc_PowerKeyFunSet(unsigned char uszFunc)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_PowerKeyFunSet(uszFunc);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerKeyFunSet Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerKeyFunSet(%c) OK", uszFunc);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Set_Auto_SignOn_Time_OneStep
Date&Time       :2019/12/19 下午 4:16
Describe        :偷懶不設定
*/
int inFunc_Set_Auto_SignOn_Time_OneStep(void)
{
	int		inRetVal = VS_ERROR;
	int		i = 0;
	char		szDate[8 + 1] = {0};
	char		szTime[6 + 1] = {0};
	RTC_NEXSYS	srRTC = {};
	RTC_NEXSYS	srExpectRTC = {};
	
	for (i = 0; i < _TDT_INDEX_MAX_; i++)
	{
		inLoadTDTRec(i);
		inSetTicket_LastAutoSignOnDate(" ");
		inSaveTDTRec(i);
	}
	
	memset(&srRTC, 0x00, sizeof(srRTC));
	inFunc_GetSystemDateAndTime(&srRTC);
	
	memset(&srExpectRTC, 0x00, sizeof(srExpectRTC));
	inRetVal = inNCCC_Ticket_Auto_SignOn_Time_Return(&srExpectRTC);
	
	memset(szDate, 0x00, sizeof(szDate));
	memset(szTime, 0x00, sizeof(szTime));
	
	sprintf(szDate, "20%02d%02d%02d", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
	if (srExpectRTC.uszSecond == 0)
	{
		sprintf(szTime, "%02d%02d%02d", srExpectRTC.uszHour, (srExpectRTC.uszMinute - 1), 0);
	}
	else
	{
		sprintf(szTime, "%02d%02d%02d", srExpectRTC.uszHour, srExpectRTC.uszMinute, 0);
	}
	
	inFunc_SetEDCDateTime(szDate, szTime);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_Screen_Size
Date&Time       :2019/12/26 下午 1:57
Describe        :確認機器螢幕區分是大螢幕還小螢幕 也可分辨觸控功能
*/
int inFunc_Check_Screen_Size(void)
{
	char		szDebugMsg[100 + 1];
	unsigned char	uszColor, uszTouch;
	unsigned short  usRetVal;
	unsigned long	ulResolution;
	
	usRetVal = CTOS_LCDAttributeGet(&ulResolution, &uszColor, &uszTouch);
	
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_LCDAttributeGet Err: 0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
	
	if (ulResolution == d_RESOLUTION_320x240)
	{
		ginHalfLCD = VS_TRUE;
	}
	else
	{
		ginHalfLCD = VS_FALSE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		
		if (ulResolution == d_RESOLUTION_480x320)
			strcpy(szDebugMsg, "Screen Size : 480 x 320");
		else if (ulResolution == d_RESOLUTION_320x480)
			strcpy(szDebugMsg, "Screen Size : 320 x 480");
		else if (ulResolution == d_RESOLUTION_320x240)
			strcpy(szDebugMsg, "Screen Size : 320 x 240");
		else if (ulResolution == d_RESOLUTION_128x64)
			strcpy(szDebugMsg, "Screen Size : 128 x 64");
		else if (ulResolution == d_RESOLUTION_128x32)
			strcpy(szDebugMsg, "Screen Size : 128 x 32");
		else
			sprintf(szDebugMsg, "unknown Size : %ld", ulResolution);
		
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (uszTouch == d_TOUCH_RESISTOR	||
	    uszTouch == d_TOUCH_CAPACITOR_1P)
	{
		ginTouchEnable = VS_TRUE;
	}
	else
	{
		ginTouchEnable = VS_FALSE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		
		if (uszTouch == d_TOUCH_RESISTOR)
			strcpy(szDebugMsg, "Touch Capability : TOUCH_RESISTOR");
		else if (uszTouch == d_TOUCH_CAPACITOR_1P)
			strcpy(szDebugMsg, "Touch Capability : TOUCH_CAPACITOR_1P");
		else if (uszTouch == d_TOUCH_NONE)
			strcpy(szDebugMsg, "Touch Capability : NONE");
		else
			sprintf(szDebugMsg, "unknown Touch Capability : %u", uszTouch);
		
		inLogPrintf(AT, szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_UI_Initial
Date&Time       :2019/10/25 上午 10:47
Describe        :確認機器類型後 解壓縮更新大螢幕或小螢幕圖檔
*/
int inFunc_Booting_Flow_UI_Initial(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;
	int	inUI_FULL = VS_ERROR;
	int	inUI_HALF = VS_ERROR;
	char	szPathName[200 + 1] = {0};
	char	szFileName[100 + 1] = {0};
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Booting_Flow_UI_Initial START!");
	
	inUI_FULL = inFILE_Check_Exist((unsigned char*)_UI_PICTURE_FULL_);
	inUI_HALF = inFILE_Check_Exist((unsigned char*)_UI_PICTURE_HALF_);
	
	if ((inUI_FULL == VS_SUCCESS) || (inUI_HALF == VS_SUCCESS))
	{
		/* 代表有圖檔要解壓縮 繼續往下執行 */
		/* 初始化 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("UI初始化", _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);
		
		inFunc_Dir_Make(_UI_UPDATE_DIR_NAME_, _FS_DATA_PATH_);

		if (inUI_FULL == VS_SUCCESS)
			inFile_Move_File(_UI_PICTURE_FULL_, _FS_DATA_PATH_, "", _UI_UPDATE_PATH_);

		if (inUI_HALF == VS_SUCCESS)
			inFile_Move_File(_UI_PICTURE_HALF_, _FS_DATA_PATH_, "", _UI_UPDATE_PATH_);

		/* 小螢幕機型和大螢幕機型分別解自己的zip檔 */
		if (ginHalfLCD == VS_TRUE && inUI_HALF == VS_SUCCESS)
		{
			inFunc_Unzip("-o", _UI_PICTURE_HALF_, _UI_UPDATE_PATH_, "-d", _UI_UPDATE_PATH_);
			inFile_Unlink_File(_UI_PICTURE_HALF_, _UI_UPDATE_PATH_);		
		}
		else if (ginHalfLCD != VS_TRUE && inUI_FULL == VS_SUCCESS)
		{
			inFunc_Unzip("-o", _UI_PICTURE_FULL_, _UI_UPDATE_PATH_, "-d", _UI_UPDATE_PATH_);
			inFile_Unlink_File(_UI_PICTURE_FULL_, _UI_UPDATE_PATH_);
		}
		else
		{
			/* load到不是該機型的圖檔壓縮檔 清空 */
			inFunc_Clear_UI_Dump();
			
			vdUtility_SYSFIN_LogMessage(AT, "更新失敗");
			return (VS_SUCCESS);
		}

		if (ginDebug == VS_TRUE)
		{
			inFunc_ls("-R -l", _UI_UPDATE_PATH_);
		}

		memset(szFileName, 0x00, sizeof(szFileName));

		if (ginHalfLCD == VS_TRUE)
			strcpy(szFileName, "UIList_V3_HALF.mmci");
		else
			strcpy(szFileName, "UIList_V3_FULL.mmci");

		memset(szPathName, 0x00, sizeof(szPathName));
		sprintf(szPathName, "%s%s", _UI_UPDATE_PATH_, szFileName);

		inRetVal = inFunc_Update_UI(szPathName);
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "更新UI失敗");
		}
		else
		{
			vdUtility_SYSFIN_LogMessage(AT, "更新UI成功");
		}
		inFunc_Reboot();

		return (VS_SUCCESS);
	}
	else
	{
		inFunc_Clear_UI_Dump();
		return (VS_SUCCESS);
	}
}

/*
Function        :inFunc_Clear_UI_Dump
Date&Time       :2019/10/25 上午 10:47
Describe        :移除路徑下檔案
*/
int inFunc_Clear_UI_Dump(void)
{
	int	inReVal = VS_ERROR;
	
	inReVal = inFunc_Data_Delete("-r", "*", _UI_UPDATE_PATH_);
	if (inReVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Update_UI
Date&Time       :2019/10/25 上午 10:47
Describe        :跑mmci更新UI
*/
int inFunc_Update_UI(char* szPathName)
{
	int	inRetVal = VS_ERROR;
	char	szDebugMsg[100 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Update_UI() START !");
	}
	
	inRetVal = CTOS_UpdateFromMMCIEx((unsigned char *)szPathName, &callbackFun);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inRetVal = 0x%04X", inRetVal);
		inLogPrintf(AT, szDebugMsg);
	}
	
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRetVal = 0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		vdUtility_SYSFIN_LogMessage(AT, "CTOS_UpdateFromMMCIEx failed inRetVal = 0x%04X", inRetVal);
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Update_UI() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckFile_From_PUB_To_SD_ALL
Date&Time       :2021/7/2 下午 5:19
Describe        :
*/
int inFunc_CheckFile_From_PUB_To_SD_ALL()
{
	char		szPath[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	RTC_NEXSYS	srRTC = {0};
	DISPLAY_OBJECT	srDispMsgObj = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

	if (inFunc_Check_SDCard_Mounted() != VS_SUCCESS)
	{
		memset(&srDispMsgObj, 0x00, sizeof(DISPLAY_OBJECT));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "未掛載SD");
		srDispMsgObj.inErrMsg1Line = _LINE_8_7_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		inDISP_ChineseFont("SD已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);
	
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%s_Check/", _SD_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	
	inFunc_Dir_Make(szDirName, szPath);

	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s_Check/%s/", _SD_PATH_, szModelName, szDirName);
	inFunc_Data_Copy("* -r", _AP_PUB_PATH_, "", szPath);
	
	/* 因為外接設備,仍需要使用同步 */
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_CheckFile_From_PUB_To_USB_ALL
Date&Time       :2021/7/2 下午 5:21
Describe        :
*/
int inFunc_CheckFile_From_PUB_To_USB_ALL(void)
{
	int		inOrgUSBMode = 0;
	int		inRetVal = VS_ERROR;
	int		inTimeout = 30;
	char		szPath[100 + 1] = {0};
	char		szDirName[100 + 1] = {0};
	char		szModelName[20 + 1] = {0};
	unsigned char	uszKey = 0x00;
	unsigned char	uszCheckUSBBit = VS_FALSE;
	unsigned char	uszCheckTimeoutBit = VS_FALSE;
	unsigned char	uszCheckKeyBit = VS_FALSE;
	RTC_NEXSYS	srRTC = {0};
	DISPLAY_OBJECT	srDispMsgObj = {0};
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("等待掛載中 Timeout:", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	
	inUSB_Get_Host_Device_Mode(&inOrgUSBMode);
	inUSB_SelectMode(_USB_MODE_HOST_);
	inDISP_TimeoutStart(inTimeout);
	
	while (1)
	{
		inRetVal = inDISP_TimeoutCheck(_FONTSIZE_8X22_, _LINE_8_7_, _DISP_RIGHT_);
		if (inRetVal == VS_TIMEOUT)
		{
			uszCheckTimeoutBit = VS_TRUE;
		}
		
		uszKey = uszKBD_Key();
		if (uszKey == _KEY_CANCEL_)
		{
			uszCheckKeyBit = VS_TRUE;
		}
		
		inRetVal = inFunc_Check_USB_Mounted();
		if (inRetVal == VS_SUCCESS)
		{
			uszCheckUSBBit = VS_TRUE;
		}
		
		if (uszCheckTimeoutBit == VS_TRUE	|| 
		    uszCheckUSBBit == VS_TRUE		||
		    uszCheckKeyBit == VS_TRUE)
		{
			break;
		}
	}
	
	if (uszCheckUSBBit != VS_TRUE)
	{
		memset(&srDispMsgObj, 0x00, sizeof(DISPLAY_OBJECT));
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		srDispMsgObj.inTimeout = 2;
		strcpy(srDispMsgObj.szErrMsg1, "未掛載USB");
		srDispMsgObj.inErrMsg1Line = _LINE_8_7_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);
		
		inUSB_SelectMode(inOrgUSBMode);
		
		return (VS_ERROR);
	}
	else
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("USB已掛載", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
	}
	
	memset(szModelName, 0x00, sizeof(szModelName));
	inFunc_Get_Termial_Model_Name(szModelName);
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案中，請稍後", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	inFunc_GetSystemDateAndTime(&srRTC);
	sprintf(szPath, "%s%s_Check/", _USB_PATH_, szModelName);
	sprintf(szDirName, "%02d%02d%02d_%02d%02d%02d_Nexsys%s", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, szModelName);
	
	inFunc_Dir_Make(szDirName, szPath);

	memset(szPath, 0x00, sizeof(szPath));
	sprintf(szPath, "%s%s_Check/%s/", _USB_PATH_, szModelName, szDirName);
	inFunc_Data_Copy("* -r", _AP_PUB_PATH_, "", szPath);
	
	/* 因為外接設備,仍需要使用同步 */
	sync();
	
	inDISP_Clear_Line(_LINE_8_8_, _LINE_8_8_);
	inDISP_ChineseFont("複製檔案完成", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
	inUSB_SelectMode(inOrgUSBMode);
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Save_Last_UniqueNo
Date&Time       :2019/8/15 下午 2:59
Describe        :
*/
int inFunc_Save_Last_UniqueNo(TRANSACTION_OBJECT *pobTran)
{
	int	inFd = 0;
	int	inRetVal = VS_ERROR;
	int	inBufferSize = 0;
	char	szBuffer[20 + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
        if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_) &&
            memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
	{
                return (VS_SUCCESS);
        }
        
        if (pobTran->uszECRBit == VS_FALSE)
        {
                return (VS_SUCCESS);
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Save_Last_UniqueNo() START !");
	}
	
	inFile_Linux_Delete_In_Fs_Data(_LAST_UNIQUE_NO_NAME_);
	
	inRetVal = inFile_Linux_Create_In_Fs_Data(&inFd, _LAST_UNIQUE_NO_NAME_);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inFunc_Save_Last_UniqueNo create fail");
		}
		
		return (VS_ERROR);
	}

	/* rwxr--r--*/
	inFunc_Data_Chmod("744", _LAST_UNIQUE_NO_NAME_, _FS_DATA_PATH_);

	do
	{
		inRetVal = inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		memset(szBuffer, 0x00, sizeof(szBuffer));
		sprintf(szBuffer, "%s", pobTran->szUniqueNo);
		inBufferSize = strlen(szBuffer);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "lastUniqueNo: %s", szBuffer);
		}
		
		inRetVal = inFile_Linux_Write(inFd, szBuffer, &inBufferSize);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		break;
	}while(1);

	inFile_Linux_Close(inFd);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Save_Last_UniqueNo() END !");
		inLogPrintf(AT, "----------------------------------------");
	}

	return (inRetVal);
}

/*
Function        :inFunc_Load_Last_UniqueNo
Date&Time       :2019/8/15 下午 2:59
Describe        :
*/
int inFunc_Load_Last_UniqueNo(char* szInData)
{
	int	inFd = 0;
	int	inRetVal = VS_ERROR;
	int	inBufferSize = 0;
	char	szBuffer[20 + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
        if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_098_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_) &&
            memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_105_MCDONALDS_, _CUSTOMER_INDICATOR_SIZE_))  
	{
                return (VS_SUCCESS);
        }
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_Load_Last_UniqueNo() START !");
	}
	
	inRetVal = inFile_Linux_Open_In_Fs_Data(&inFd, _LAST_UNIQUE_NO_NAME_);
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	do
	{
		inRetVal = inFile_Linux_Seek(inFd, 0, _SEEK_BEGIN_);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		memset(szBuffer, 0x00, sizeof(szBuffer));
		inBufferSize = sizeof(szBuffer);
		inRetVal = inFile_Linux_Read(inFd, szBuffer, &inBufferSize);
		if (inRetVal != VS_SUCCESS)
		{
			break;
		}

		if (inBufferSize > 0)
		{
			memcpy(&szInData[0], &szBuffer[0], 10);
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "lastUniqueNo:%s", szBuffer);
			}
		}
		else
		{
			inRetVal = VS_ERROR;
			break;
		}

		break;
	}while(1);

	inFile_Linux_Close(inFd);
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_Load_Last_UniqueNo() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (inRetVal);
}

/*
Function        :inFunc_Get_ResetBatchNum_Switch
Date&Time       :2022/1/3 上午 9:57
Describe        :
*/
int inFunc_Get_ResetBatchNum_Switch(char* szSwitch)
{
	unsigned long	ulHandle = 0;
	
	if (inFILE_OpenReadOnly(&ulHandle, (unsigned char*)_RESET_BATCH_FILE_NAME_) != VS_SUCCESS)
	{
		inFILE_Create(&ulHandle, (unsigned char*)_RESET_BATCH_FILE_NAME_);
		inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_);
		inFILE_Write(&ulHandle, (unsigned char*)_RESET_BATCH_NONE_, 1);
	}
	
	inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_);
	inFILE_Read(&ulHandle, (unsigned char*)szSwitch, 1);
	inFILE_Close(&ulHandle);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "ResetBatchNum_Switch Get \"%s\"", szSwitch);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Set_ResetBatchNum_Switch
Date&Time       :2022/1/3 上午 9:57
Describe        :
*/
int inFunc_Set_ResetBatchNum_Switch(char* szSwitch)
{
	unsigned long	ulHandle = 0;
	
	if (inFILE_OpenReadOnly(&ulHandle, (unsigned char*)_RESET_BATCH_FILE_NAME_) != VS_SUCCESS)
	{
		inFILE_Create(&ulHandle, (unsigned char*)_RESET_BATCH_FILE_NAME_);
	}
	
	inFILE_Seek(ulHandle, 0, _SEEK_BEGIN_);
	inFILE_Write(&ulHandle, (unsigned char*)szSwitch, 1);
	inFILE_Close(&ulHandle);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "ResetBatchNum_Switch Set \"%s\"", szSwitch);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Check_Digit
Date&Time       :2022/1/3 上午 9:57
Describe        :
*/
int inFunc_Check_Digit(void)
{
        char	szCustomerIndicator[3 + 1] = {0};

        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
            return 8;
        else
            return 7;
}

/*
Function        :inFunc_Check_Batch_limit
Date&Time       :2022/1/3 上午 9:57
Describe        :
*/
int inFunc_Check_Batch_limit(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal = VS_SUCCESS;
        char    szHostLabel[8 + 1];
        char	szCustomerIndicator[3 + 1] = {0};
	
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_Check_Batch_limit START!");
        // Tusin_076
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
        
        if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_076_8_DIGITS_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = VS_SUCCESS;
                vdUtility_SYSFIN_LogMessage(AT, "inFunc_Check_Batch_limit Not cus_076 END!");
                return (inRetVal);
	}
        
        // 單筆檢核
        inRetVal = inFunc_Check_Single_Trade(pobTran);
        
        if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inFunc_Check_Batch_limit inFunc_Check_Single_Trade(%d)", inRetVal);
		return (inRetVal); 
	}
        
        // 目前只檢核NCCC(HG) + DCC，大高、電票總額不可能爆表
        memset(szHostLabel, 0x00, sizeof(szHostLabel));
        inGetHostLabel(szHostLabel);
        
        if (!memcmp(szHostLabel, _HOST_NAME_CREDIT_NCCC_, strlen(_HOST_NAME_CREDIT_NCCC_)) ||
            !memcmp(szHostLabel, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_))  ||
            !memcmp(szHostLabel, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_))    ||
            !memcmp(szHostLabel, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_)))
        {
               inRetVal = inFunc_Check_Total_Trade(pobTran); 
        }
        
	if (inRetVal != VS_SUCCESS)
	{
		vdUtility_SYSFIN_LogMessage(AT, "inFunc_Check_Batch_limit inFunc_Check_Total_Trade(%d)", inRetVal);
	}
	vdUtility_SYSFIN_LogMessage(AT, "inFunc_Check_Batch_limit END!");
        
        return (inRetVal); 
}

/*
Function        :inFunc_Check_Total_Trade
Date&Time       :2022/1/3 上午 9:57
Describe        :
*/
int inFunc_Check_Total_Trade(TRANSACTION_OBJECT *pobTran)
{
        int		inHostIndex = -1;
        long long	llNSum = 0;
        long long	llDSum = 0;
        long long	llSum = 0;
        char            szHostLabel[8 + 1];
        TRANSACTION_OBJECT TempTran;
        ACCUM_TOTAL_REC	srAccumRec = {0};
	
        // 找NCCC
        inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inHostIndex);
        
        if (inHostIndex == -1)
        {
                llNSum = 0;
        }   
        else
        {    
                memset(&TempTran, 0x00, sizeof(TempTran));
                TempTran.srBRec.inHDTIndex = inHostIndex;

                // 為了HG，inACCUM_GetRecord() => inFunc_ComposeFileName() => inFunc_HostName_DecideByTRT()會擋，需修改NCCC再還原HG
                memset(szHostLabel, 0x00, sizeof(szHostLabel));
                inGetHostLabel(szHostLabel);
                inSetHostLabel(_HOST_NAME_CREDIT_NCCC_);
                
                memset(&srAccumRec, 0x00, sizeof(ACCUM_TOTAL_REC));
                inACCUM_GetRecord(&TempTran, &srAccumRec);
                
                inSetHostLabel(szHostLabel);

                llNSum = srAccumRec.llTotalAmount;
        }
        
        // 找DCC
        inHostIndex = -1;
        inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_DCC_, &inHostIndex);
        
        if (inHostIndex == -1)
        {
                llDSum = 0;
        }   
        else
        {
                memset(&TempTran, 0x00, sizeof(TempTran));
                TempTran.srBRec.inHDTIndex = inHostIndex;

                memset(&srAccumRec, 0x00, sizeof(ACCUM_TOTAL_REC));
                inACCUM_GetRecord(&TempTran, &srAccumRec);

                llDSum = srAccumRec.llTotalAmount;
        }

        llSum = llNSum + llDSum;
        
        /* inACCUM_GetRecord() 會Load到，要還原 */
        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
        
        if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "[Check_Batch_limit]");
                inLogPrintf(AT, "%01lld + %01lld = %01lld", llNSum, llDSum, llSum);
        }
        
        if (pobTran->inTransactionCode == _VOID_)
        {    
		if (pobTran->srBRec.inOrgCode == _REFUND_           || pobTran->srBRec.inOrgCode == _INST_REFUND_   || 
                    pobTran->srBRec.inOrgCode == _REDEEM_REFUND_    || pobTran->srBRec.inOrgCode == _CUP_REFUND_    || 
                    pobTran->srBRec.inOrgCode == _CUP_MAIL_ORDER_REFUND_)	
                {
                        llSum += pobTran->srBRec.lnTxnAmount;
                }
                else if (pobTran->srBRec.inOrgCode == _TIP_)
                {       
                        llSum -= pobTran->srBRec.lnTipTxnAmount;
                }
                else
                {
                        llSum -= pobTran->srBRec.lnTxnAmount;
                }
        }
        else
        {        
                if (pobTran->srBRec.inCode == _REFUND_              || pobTran->srBRec.inCode == _INST_REFUND_      || 
		    pobTran->srBRec.inCode == _REDEEM_REFUND_       || pobTran->srBRec.inCode == _CUP_REFUND_       || 
                    pobTran->srBRec.inCode == _CUP_MAIL_ORDER_REFUND_)
                {
                        llSum -= pobTran->srBRec.lnTxnAmount;
                }   
                else if (pobTran->srBRec.inCode == _TIP_)
                {       
                        llSum += pobTran->srBRec.lnTipTxnAmount;
                }
                else
                {
                        llSum += pobTran->srBRec.lnTxnAmount;
                }   
        }
        
        if (abs(llSum) > 999999999)
        {
                DISPLAY_OBJECT	srDispMsgObj;
                memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
                strcpy(srDispMsgObj.szDispPic1Name, "");
                srDispMsgObj.inDispPic1YPosition = 0;
                srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
                srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
                strcpy(srDispMsgObj.szErrMsg1, "超過批次最大限額");
                srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
                strcpy(srDispMsgObj.szErrMsg2, "請先結帳");
                srDispMsgObj.inErrMsg2Line = _LINE_8_6_;
                srDispMsgObj.inBeepTimes = 1;
                srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
                return (VS_ERROR);
        }
        else
        {       
                inLogPrintf(AT, "%01lld < 999,999,999 Pass", llSum);
                return (VS_SUCCESS); 
        }
}

/*
Function        :inFunc_Check_Single_Trade
Date&Time       :2022/1/3 上午 9:57
Describe        :
*/
int inFunc_Check_Single_Trade(TRANSACTION_OBJECT *pobTran)
{
        if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "[Check_Batch_limit]");
                inLogPrintf(AT, "Amount = %ld", pobTran->srBRec.lnTxnAmount);
                inLogPrintf(AT, "Tip = %ld", pobTran->srBRec.lnTipTxnAmount);
        }
        
        if (pobTran->srBRec.inCode == _TIP_)
        {       
                if (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnTipTxnAmount > 99999999)
                {
                        DISPLAY_OBJECT	srDispMsgObj;
                        memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
                        strcpy(srDispMsgObj.szDispPic1Name, "");
                        srDispMsgObj.inDispPic1YPosition = 0;
                        srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
                        srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
                        strcpy(srDispMsgObj.szErrMsg1, "交易金額超過上限");
                        srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
                        srDispMsgObj.inBeepTimes = 1;
                        srDispMsgObj.inBeepInterval = 0;
                        inDISP_Msg_BMP(&srDispMsgObj);
                        return (VS_ERROR);
                }
        }
        
        /* 擋ECR的溢位檢查 */
        if (pobTran->srBRec.lnTxnAmount > 99999999 ||
            pobTran->srBRec.lnTxnAmount < 0)
        {
                DISPLAY_OBJECT	srDispMsgObj;
                memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
                strcpy(srDispMsgObj.szDispPic1Name, "");
                srDispMsgObj.inDispPic1YPosition = 0;
                srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
                srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
                strcpy(srDispMsgObj.szErrMsg1, "交易金額超過上限");
                srDispMsgObj.inErrMsg1Line = _LINE_8_6_;
                srDispMsgObj.inBeepTimes = 1;
                srDispMsgObj.inBeepInterval = 0;
                inDISP_Msg_BMP(&srDispMsgObj);
                return (VS_ERROR);
        }
    
        return (VS_SUCCESS);
}

/*
Function        :inFunc_GetHostNum_Flow
Date&Time       :2022/5/24 上午 10:50
Describe        :根據客製化決定界面
*/
int inFunc_GetHostNum_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szCustomIndicator[3 + 1] = {0};
	
	guszNoChooseHostBit = VS_FALSE;
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
	if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
	    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_GET_HOST_NUM_NEWUI_CUS_075_);
	}
	else
	{
		inRetVal = inFLOW_RunFunction(pobTran, _FUNCTION_GET_HOST_NUM_NEWUI_);
	}
	
	return (inRetVal);
}

/*
Function        :inFunc_GetHostNum_NewUI_Cus_075
Date&Time       :2022/5/24 上午 10:41
Describe        :075專用UI
*/
int inFunc_GetHostNum_NewUI_Cus_075(TRANSACTION_OBJECT *pobTran)
{
	int		inOpenHostCnt = 0;      /* 記錄有幾個Host有開 */
	int		i, j = 0;               /* j是inHostIndex陣列索引 */
	int		inHostIndex[12 + 1];    /* 記錄HostEnable為Y的HostIndex */
	int		inKey = 0;
	int		inChoice = 0;
	int		inTouchSensorFunc = _Touch_NEWUI_CHOOSE_HOST_;
	int		inRetVal = VS_SUCCESS;
	int		inRetVal2 = VS_ERROR;	/* inFunc_Load_Last_Txn_Host使用 */
	int		inLastHDTIndex = -1;
        int             inFindHDTIndex = -1;
	char		szKey = 0x00;
	char		szHostEnable[2 + 1] = {0};
	char		szHostName[42 + 1] = {0};
	char		szTemp[48 + 1] = {0};
	char		szLine1[48 + 1] = {0};		/* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
	char		szLine2[48 + 1] = {0};		/* 存第二行要顯示的Host */
	char		szLine3[48 + 1] = {0};		/* 存第三行要顯示的Host */
	char		szLine4[48 + 1] = {0};
	char		szLine5[48 + 1] = {0};
	char		szLine6[48 + 1] = {0};
	char		szTemp2[48 + 1] = {0};
	char		szLine1_2[48 + 1] = {0};	/* 存第一行要顯示的Host */	/* linux系統中文字length一個字為3，小心爆掉 */
	char		szLine2_2[48 + 1] = {0};	/* 存第二行要顯示的Host */
	char		szLine3_2[48 + 1] = {0};	/* 存第三行要顯示的Host */
	char		szLine4_2[48 + 1] = {0};
	char		szLine5_2[48 + 1] = {0};
	char		szLine6_2[48 + 1] = {0};
	char		szBatchNum[6 + 1] = {0};
	char		szTimeout[4 + 1] = {0};
	char		szDebugMsg[42 + 1] = {0};
	DISPLAY_OBJECT  srDispObj;
        
        vdUtility_SYSFIN_LogMessage(AT, "inFunc_GetHostNum_NewUI_Cus_075 START!");
	
	/* 如果要連動結帳，跳過選Host流程 */
	/* ECR列印總額、明細，直接連動 */
	if (pobTran->uszAutoSettleBit == VS_TRUE							||
	   (pobTran->uszECRBit == VS_TRUE && pobTran->inRunOperationID == _OPERATION_TOTAL_REPORT_)	||
	   (pobTran->uszECRBit == VS_TRUE && pobTran->inRunOperationID == _OPERATION_DETAIL_REPORT_))
	{
		guszNoChooseHostBit = VS_TRUE;
		return (VS_SUCCESS);
	}
	
	if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
		/* 1.交易帳單 */
		/* 2.結帳帳單 */
		/* 請選擇？ */
		memset(szLine1, 0x00, sizeof(szLine1));
		memset(szLine1_2, 0x00, sizeof(szLine1_2));
		memset(szLine2, 0x00, sizeof(szLine2));
		memset(szLine2_2, 0x00, sizeof(szLine2_2));
		sprintf(szLine1, "    ");
		sprintf(szLine1_2, "是  1");
		sprintf(szLine2, "    ");
		sprintf(szLine2_2, "否  2");

		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_PutGraphic(_CHOOSE_HOST_2_, 0, _COORDINATE_Y_LINE_8_4_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);

		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
		inDISP_ChineseFont("是否列印上一筆?", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (1)
		{
			
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			/* 是 */
			if (szKey == _KEY_1_			||
			    inChoice == _NEWUI_CHOOSE_HOST_Touch_HOST_1_)
			{
				/* 上筆簽單Host */
				inLastHDTIndex = -1;
				inRetVal2 = inFunc_Load_Last_Txn_Host(&inLastHDTIndex);

				if (inRetVal2 != VS_SUCCESS || 
				    inLastHDTIndex < 0)
				{
					 inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
					/* 顯示無交易紀錄 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_RECORD_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
					srDispMsgObj.inTimeout = 2;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);
					inRetVal = VS_ERROR;
				}
				else
				{
					pobTran->srBRec.inHDTIndex = inLastHDTIndex;
					if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
					{
						return (VS_ERROR);
					}

					if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
					{
						return (VS_ERROR);
					}

					memset(szBatchNum, 0x00, sizeof(szBatchNum));
					inGetBatchNum(szBatchNum);
					pobTran->srBRec.lnBatchNum = atol(szBatchNum);
					/* 上筆簽單調閱編號 */
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
					inRetVal = VS_SUCCESS;
				}
				break;
			}
			/* 否 */
			else if (szKey == _KEY_2_			||
				 inChoice == _NEWUI_CHOOSE_HOST_Touch_HOST_2_)
			{
				/* 輸入密碼的層級 */
				/* 第一層輸入密碼 */
				inRetVal = inFunc_CheckCustomizePassword(_ACCESS_WITH_DUTY_FREE_REPRINT_PASSWORD_, pobTran->inTransactionCode);
				if (inRetVal != VS_SUCCESS)
				{
					inRetVal = VS_ERROR;
				}
				else
				{
					inRetVal = VS_SUCCESS;
				}
				break;
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}

		}
		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}
	else if (pobTran->inRunOperationID == _OPERATION_DETAIL_REPORT_)
	{
		/* 輸入密碼的層級 */
		/* 第一層輸入密碼 */
		inRetVal2 = inFunc_CheckCustomizePassword(_ACCESS_WITH_DUTY_FREE_DETAIL_PASSWORD_, pobTran->inTransactionCode);
		if (inRetVal2 != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	/* 075流程中止，直接退出 */
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
	/* 銀聯選NCCC */
	if (pobTran->srBRec.uszCUPTransBit == VS_TRUE)
	{
		pobTran->srBRec.inHDTIndex = 0;
		
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		 
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}
	
	/* SmartPay選NCCC */
	if (pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		pobTran->srBRec.inHDTIndex = 0;
		
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		 
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}
        
        /* 信託重印直接選 */
	if (pobTran->srTrustRec.uszTrustTransBit == VS_TRUE &&
            pobTran->inRunOperationID == _OPERATION_REPRINT_)
	{
                inFindHDTIndex = -1;
                inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_TRUST_, &inFindHDTIndex);
		pobTran->srBRec.inHDTIndex = inFindHDTIndex;
		
		if (inLoadHDTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}

		if (inLoadHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
			return (VS_ERROR);
		}
		 
		memset(szBatchNum, 0x00, sizeof(szBatchNum));
		inGetBatchNum(szBatchNum);
		pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	}
		
	
	/* 如果已經有HostNum，跳過選Host流程(ECR發動) */
	if (pobTran->srBRec.inHDTIndex != -1)
	{
		guszNoChooseHostBit = VS_TRUE;
		return (VS_SUCCESS);
	}
	
	/* 以上是特例，如果都沒有就手動選Host */

        memset(szLine1, 0x00, sizeof(szLine1));
        memset(szLine2, 0x00, sizeof(szLine2));
        memset(szLine3, 0x00, sizeof(szLine3));
        memset(szLine4, 0x00, sizeof(szLine4));
	memset(szLine5, 0x00, sizeof(szLine5));
	memset(szLine6, 0x00, sizeof(szLine6));
	memset(szLine1_2, 0x00, sizeof(szLine1_2));
        memset(szLine2_2, 0x00, sizeof(szLine2_2));
        memset(szLine3_2, 0x00, sizeof(szLine3_2));
        memset(szLine4_2, 0x00, sizeof(szLine4_2));
	memset(szLine5_2, 0x00, sizeof(szLine5_2));
	memset(szLine6_2, 0x00, sizeof(szLine6_2));
        memset(szTimeout, 0x00, sizeof(szTimeout));
        memset(inHostIndex, 0x00, sizeof(inHostIndex));
        memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
	
        for (i = 0;; ++i)
        {
                /* 先LoadHDT */
                if (inLoadHDTRec(i) == VS_ERROR)
                {
                        /* 當找不到第i筆資料會回傳VS_ERROR */
                        break;
                }
		
		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
		* 客製化075,103 NCCC和DCC要在取消和小費下視為合併 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_DCC_, strlen(_HOST_NAME_DCC_)) == 0)
		{
			if (pobTran->inRunOperationID == _OPERATION_VOID_	||
			    pobTran->inRunOperationID == _OPERATION_TIP_	||
			    pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				continue;
			}
		}

		/* ESC不該出現在選單上 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESC_, strlen(_HOST_NAME_ESC_)) == 0)
		{
			continue;
		}

		/* HG不該出現在選單上(結帳、小費、交易查詢) */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_HG_, strlen(_HOST_NAME_HG_)) == 0)
		{
			if (pobTran->inRunOperationID == _OPERATION_VOID_		||
			    pobTran->inRunOperationID == _OPERATION_TIP_		||
			    pobTran->inRunOperationID == _OPERATION_SETTLE_		||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_		||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_TOTAL_	||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_DETAIL_	||
			    pobTran->inRunOperationID == _OPERATION_DELETE_BATCH_	||
			    pobTran->inRunOperationID == _OPERATION_REPRINT_)
			{
				continue;
			}
		}

		/* ESVC不該出現在一般交易選單上`，目前只有結帳需要，採正向表列 */
		/* 客製化075、103，重印帳單不出現電票選項 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
		{
			if (pobTran->inRunOperationID == _OPERATION_SETTLE_		||
			    pobTran->inRunOperationID == _OPERATION_DELETE_BATCH_	||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_		||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_TOTAL_	||
			    pobTran->inRunOperationID == _OPERATION_REVIEW_DETAIL_	||
			    pobTran->inRunOperationID == _OPERATION_TOTAL_REPORT_	||
			    pobTran->inRunOperationID == _OPERATION_DETAIL_REPORT_)
			{
				
			}
			else
			{
				continue;
			}
		}
                
                /* TR不該出現在一般交易選單上`，目前只有結帳需要，採正向表列 */
		memset(szHostName, 0x00, sizeof(szHostName));
		inGetHostLabel(szHostName);
		if (memcmp(szHostName, _HOST_NAME_TRUST_, strlen(_HOST_NAME_TRUST_)) == 0)
		{
			if (pobTran->inRunOperationID != _OPERATION_SETTLE_		&&
			    pobTran->inRunOperationID != _OPERATION_DELETE_BATCH_)
			{
				continue;
			}
		}
		
                /* GET HOST Enable */
                memset(szHostEnable, 0x00, sizeof(szHostEnable));
                if (inGetHostEnable(szHostEnable) == VS_ERROR)
                {	
                        return (VS_ERROR);
                }
                else
                {
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "%d HostEnable: %s", i, szHostEnable);
				inLogPrintf(AT, szDebugMsg);
			}
			
                        if (memcmp(szHostEnable, "Y", 1) != 0)
                        {
                                /* 如果HostEnable != Y，就continue */
                                continue;
                        }

                        /* 如果主機有開，才loadHDPT */
                        if (inLoadHDPTRec(i) == VS_ERROR)
                        {
                                /* 當找不到第i筆資料會回傳VS_ERROR */
                                return (VS_ERROR);
                        }
			
                        inOpenHostCnt ++;       /* 記錄有幾個Host有開 */

                        /* 記錄HostEnable為Y的HostIndex，減1是因為HostIndex從01開始 */
                        inHostIndex[j] = i;
                        j++;
			
			memset(szHostName, 0x00, sizeof(szHostName));
			inGetHostLabel(szHostName);
			
                        /* 用szTRTFileName來決定要顯示的Host Name */
                        memset(szTemp, 0x00, sizeof(szTemp));
			inFunc_DiscardSpace(szHostName);
			/* ESVC要顯示電子票證 */
			if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
			{
				sprintf(szTemp, "%s", "電子票");
				memset(szTemp2, 0x00, sizeof(szTemp2));
				sprintf(szTemp2, "證   %d", inOpenHostCnt);
			}
			else
			{
				sprintf(szTemp, "%s", szHostName);
				memset(szTemp2, 0x00, sizeof(szTemp2));
				sprintf(szTemp2, "     %d", inOpenHostCnt);
			}
			
                        /* 每一行顯示的內容先存在陣列裡 */
                        switch (inOpenHostCnt)
                        {
                                case 1:
                                        memcpy(&szLine1[0], szTemp, strlen(szTemp));
					
					memcpy(szLine1_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 2:
                                        memcpy(&szLine2[0], szTemp, strlen(szTemp));
					
					memcpy(szLine2_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 3:
                                        memcpy(&szLine3[0], szTemp, strlen(szTemp));
					
					memcpy(szLine3_2, szTemp2, strlen(szTemp2));
                                        break;
                                case 4:
                                        memcpy(&szLine4[0], szTemp, strlen(szTemp));
					
					memcpy(szLine4_2, szTemp2, strlen(szTemp2));
                                        break;
				case 5:
                                        memcpy(&szLine5[0], szTemp, strlen(szTemp));
					
					memcpy(szLine5_2, szTemp2, strlen(szTemp2));
                                        break;
				case 6:
                                        memcpy(&szLine6[0], szTemp, strlen(szTemp));
					
					memcpy(szLine6_2, szTemp2, strlen(szTemp2));
                                        break;
                                default:
                                        break;
                        }
                }
		
        }/* End of For loop */

        /* 當inOpenHostCnt = 0，表示主機都沒開或者inLoadHDT有問題 */
        if (inOpenHostCnt == 0)
        {
                /* 主機選擇錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
		srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
		srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
		srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		strcpy(srDispMsgObj.szErrMsg1, "");
		srDispMsgObj.inErrMsg1Line = 0;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

                return (VS_ERROR);
        }

        if (inOpenHostCnt == 1)
        {
                /* 只有開一個Host */
                if (inLoadHDTRec(inHostIndex[0]) == VS_ERROR)
                {
                        /* 主機選擇錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

                        return (VS_ERROR);
                }

                if (inLoadHDPTRec(inHostIndex[0]) == VS_ERROR)
                {
                        /* 主機選擇錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 1;
			srDispMsgObj.inBeepInterval = 0;
			inDISP_Msg_BMP(&srDispMsgObj);

                        return (VS_ERROR);
                }
                else
                {
                        pobTran->srBRec.inHDTIndex = inHostIndex[0];
                        memset(szBatchNum, 0x00, sizeof(szBatchNum));
                        inGetBatchNum(szBatchNum);
                        pobTran->srBRec.lnBatchNum = atol(szBatchNum);
                }

		guszNoChooseHostBit = VS_TRUE;
                inRetVal = VS_SUCCESS;
        }
        else
        {
                inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		
		switch (inOpenHostCnt)
		{
			case	2:
				inDISP_PutGraphic(_CHOOSE_HOST_2_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			case	3:
				inDISP_PutGraphic(_CHOOSE_HOST_3_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			case	4:
				inDISP_PutGraphic(_CHOOSE_HOST_4_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			case	5:
				inDISP_PutGraphic(_CHOOSE_HOST_5_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			case	6:
				inDISP_PutGraphic(_CHOOSE_HOST_6_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
				
			default:
				inDISP_PutGraphic(_CHOOSE_HOST_6_, 0,_COORDINATE_Y_LINE_8_4_);
				break;
		}
		
                /*有開多個Host */
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine1_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
		
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine2_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
		
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine3_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
		
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine4_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_1_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);
		
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine5_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_2_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);
		
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_13_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szLine6_2, _FONTSIZE_16X22_, _COLOR_WHITE_, _COLOR_BUTTON_, _COORDINATE_X_CHOOSE_HOST_3_, _COORDINATE_Y_LINE_16_14_, VS_FALSE);
		
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, szLine1);
			inLogPrintf(AT, szLine2);
			inLogPrintf(AT, szLine3);
			inLogPrintf(AT, szLine4);
			inLogPrintf(AT, szLine5);
			inLogPrintf(AT, szLine6);
		}
	
		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(inTouchSensorFunc);
			szKey = uszKBD_Key();

			/* 轉成數字判斷是否在inOpenHostCnt的範圍內 */
			inKey = 0;
			/* 有觸摸*/
			if (inChoice != _DisTouch_No_Event_)
			{
				switch (inChoice)
				{
					case _NEWUI_CHOOSE_HOST_Touch_HOST_1_:
						inKey = 1;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_2_:
						inKey = 2;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_3_:
						inKey = 3;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_4_:
						inKey = 4;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_5_:
						inKey = 5;
						break;
					case _NEWUI_CHOOSE_HOST_Touch_HOST_6_:
						inKey = 6;
						break;
					default:
						inKey = 0;
						break;
				}
			}
			/* 有按按鍵 */
			else if (szKey != 0)
			{
				switch (szKey)
				{
					case _KEY_1_:
						inKey = 1;
						break;
					case _KEY_2_:
						inKey = 2;
						break;
					case _KEY_3_:
						inKey = 3;
						break;
					case _KEY_4_:
						inKey = 4;
						break;
					case _KEY_5_:
						inKey = 5;
						break;
					case _KEY_6_:
						inKey = 6;
						break;
					default:
						inKey = 0;
						break;
				}
			}

			/* Timeout */
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				szKey = _KEY_TIMEOUT_;
			}

			if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			else if (inKey >= 1 && inKey <= inOpenHostCnt)
			{
				if (pobTran->inRunOperationID == _OPERATION_REPRINT_ &&
				    inKey == inOpenHostCnt)
				{
					/* 上筆簽單Host */
					/* 取得上筆交易Host失敗*/
					if (inRetVal2 != VS_SUCCESS || 
					    inLastHDTIndex < 0)
					{
						 inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
						/* 顯示無交易紀錄 */
						DISPLAY_OBJECT	srDispMsgObj;
						memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
						strcpy(srDispMsgObj.szDispPic1Name, _ERR_RECORD_);
						srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
						srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
						srDispMsgObj.inTimeout = 2;
						strcpy(srDispMsgObj.szErrMsg1, "");
						srDispMsgObj.inErrMsg1Line = 0;
						srDispMsgObj.inBeepTimes = 1;
						srDispMsgObj.inBeepInterval = 0;

						inDISP_Msg_BMP(&srDispMsgObj);
						return (VS_ERROR);
					}
					else
					{
						
					}
					/* 上筆簽單調閱編號 */
					pobTran->srBRec.lnOrgInvNum = _BATCH_LAST_RECORD_;
				}
				
				if (inLoadHDTRec(inHostIndex[inKey - 1]) == VS_ERROR)
				{
					/* 主機選擇錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);

					inRetVal = VS_ERROR;
					break;
				}

				if (inLoadHDPTRec(inHostIndex[inKey - 1]) == VS_ERROR)
				{
					/* 主機選擇錯誤 */
					DISPLAY_OBJECT	srDispMsgObj;
					memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
					strcpy(srDispMsgObj.szDispPic1Name, _ERR_CHOOSE_HOST_);
					srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
					srDispMsgObj.inMsgType = _CLEAR_KEY_MSG_;
					srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
					strcpy(srDispMsgObj.szErrMsg1, "");
					srDispMsgObj.inErrMsg1Line = 0;
					srDispMsgObj.inBeepTimes = 1;
					srDispMsgObj.inBeepInterval = 0;
					inDISP_Msg_BMP(&srDispMsgObj);

					inRetVal = VS_ERROR;
					break;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szTemp, 0x00, sizeof(szTemp));
						sprintf(szTemp, "%d", inHostIndex[inKey - 1]);
						inLogPrintf(AT, szTemp);
					}
					pobTran->srBRec.inHDTIndex = inHostIndex[inKey - 1];
					memset(szBatchNum, 0x00, sizeof(szBatchNum));
					inGetBatchNum(szBatchNum);
					pobTran->srBRec.lnBatchNum = atol(szBatchNum);
					
					/* 票證 */
					memset(szHostName, 0x00, sizeof(szHostName));
					inGetHostLabel(szHostName);
					if (memcmp(szHostName, _HOST_NAME_ESVC_, strlen(_HOST_NAME_ESVC_)) == 0)
					{
						pobTran->srTRec.uszESVCTransBit = VS_TRUE;
					}

					inRetVal = VS_SUCCESS;
					break;
				}
			}
		}

		/* 清空Touch資料 */
		inDisTouch_Flush_TouchFile();
	}

	return (inRetVal);
}

/*
Function        :inFunc_Edit_Duty_Free_Reprint_Pwd
Date&Time       :2022/5/25 上午 9:56
Describe        :
*/
int inFunc_Edit_Duty_Free_Reprint_Pwd()
{
	int		inRetVal = VS_SUCCESS;
	int		inTableID = 0;
	int		inReprintPwdLen = 0;
	char		szPWDNew[4 + 1] = {0};
	char		szReprintPwd[4 + 1] = {0};		/* 預設密碼8940 */
	char		szDetailPwd[4 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
	SQLITE_ALL_TABLE	srAll;
	
	/* 請輸入管理者號碼 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("請輸入重印管理碼", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	/* 取得現有密碼 */
	memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
	inTableID = 0;
	memset(&szReprintPwd, 0x00, sizeof(szReprintPwd));
	memset(&szDetailPwd, 0x00, sizeof(szDetailPwd));
	inNCCC_Func_Table_Link_Duty_Free_Pwd(&srAll, _LS_READ_, &inTableID, szReprintPwd, szDetailPwd);
	inSqlite_Get_Table_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_DUTY_FREE_PWD_, 0, &srAll);
	inReprintPwdLen = strlen(szReprintPwd);
	
	while (1)
	{
		inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

		/* 設定顯示變數 */
		srDispObj.inMaxLen = 4;
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (srDispObj.inOutputLen == inReprintPwdLen)
			break;
		else 
			continue;
	}

	if (!memcmp(&srDispObj.szOutput[0], &szReprintPwd[0], inReprintPwdLen))
	{
		 /* 輸入正確 */
	}
	else
	{
		/* 輸入錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
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


	while (1)
	{
		/* 請輸入
		 * 新重印管理碼 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("請輸入", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("新重印管理碼", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

		while (1)
		{
			inDISP_BEEP(1, 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

			/* 設定顯示變數 */
			srDispObj.inMaxLen = 4;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inMask = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (srDispObj.inOutputLen == 4)
				break;
			else 
				continue;
		}

		memset(szPWDNew, 0x00, sizeof(szPWDNew));
		memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

		/* 請再次輸入新管理號碼 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("請再次輸入", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("新重印管理碼", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

		while (1)
		{
			inDISP_BEEP(1, 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

			/* 設定顯示變數 */
			srDispObj.inMaxLen = 4;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inMask = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (srDispObj.inOutputLen == 4)
				break;
			else 
				continue;
		}

		if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
		{
			/* 修改成功 圖片 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;

			inDISP_Msg_BMP(&srDispMsgObj);
			
			/* 儲存重印管理碼 */
			memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
			inNCCC_Func_Table_Link_Duty_Free_Pwd(&srAll, _LS_UPDATE_, &inTableID, szPWDNew, szDetailPwd);
			inSqlite_Insert_Or_Replace_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_DUTY_FREE_PWD_, 0, &srAll);

			break;
		}
		else
		{
			/* 輸入錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;

			inDISP_Msg_BMP(&srDispMsgObj);
			continue;
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_Edit_Duty_Free_Detail_Pwd
Date&Time       :2022/5/25 上午 11:02
Describe        :
*/
int inFunc_Edit_Duty_Free_Detail_Pwd()
{
	int		inRetVal = VS_SUCCESS;
	int		inTableID = 0;
	int		inDetailPwdLen = 0;
	char		szPWDNew[4 + 1] = {0};
	char		szReprintPwd[4 + 1] = {0};		/* 預設密碼8940 */
	char		szDetailPwd[4 + 1] = {0};
        DISPLAY_OBJECT  srDispObj;
	SQLITE_ALL_TABLE	srAll;
	
	/* 請輸入明細管理碼 */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_ChineseFont("請輸入明細管理碼", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);
	/* 取得現有密碼 */
	memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
	inTableID = 0;
	memset(&szReprintPwd, 0x00, sizeof(szReprintPwd));
	memset(&szDetailPwd, 0x00, sizeof(szDetailPwd));
	inNCCC_Func_Table_Link_Duty_Free_Pwd(&srAll, _LS_READ_, &inTableID, szReprintPwd, szDetailPwd);
	inSqlite_Get_Table_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_DUTY_FREE_PWD_, 0, &srAll);
	inDetailPwdLen = strlen(szDetailPwd);
	
	while (1)
	{
		inDISP_BEEP(1, 0);
		inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
		memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

		/* 設定顯示變數 */
		srDispObj.inMaxLen = 4;
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMask = VS_TRUE;
		srDispObj.inColor = _COLOR_RED_;
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;

		inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
		if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			return (VS_ERROR);

		if (srDispObj.inOutputLen == inDetailPwdLen)
			break;
		else 
			continue;
	}

	if (!memcmp(&srDispObj.szOutput[0], &szDetailPwd[0], inDetailPwdLen))
	{
		 /* 輸入正確 */
	}
	else
	{
		/* 輸入錯誤 */
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
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


	while (1)
	{
		/* 請輸入
		 * 新明細管理碼 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("請輸入", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("新明細管理碼", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

		while (1)
		{
			inDISP_BEEP(1, 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

			/* 設定顯示變數 */
			srDispObj.inMaxLen = 4;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inMask = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (srDispObj.inOutputLen == 4)
				break;
			else 
				continue;
		}

		memset(szPWDNew, 0x00, sizeof(szPWDNew));
		memcpy(&szPWDNew[0], &srDispObj.szOutput[0], 4);

		/* 請再次輸入新管理號碼 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("請再次輸入", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("新明細管理碼", _FONTSIZE_8X16_, _LINE_8_5_, _DISP_LEFT_);

		while (1)
		{
			inDISP_BEEP(1, 0);
			inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
			memset(&srDispObj, 0x00, DISPLAY_OBJECT_SIZE);

			/* 設定顯示變數 */
			srDispObj.inMaxLen = 4;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inMask = VS_TRUE;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;

			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);
			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				return (VS_ERROR);

			if (srDispObj.inOutputLen == 4)
				break;
			else 
				continue;
		}

		if (!memcmp(&srDispObj.szOutput[0], &szPWDNew[0], 4))
		{
			/* 修改成功 圖片 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_SET_PWD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;

			inDISP_Msg_BMP(&srDispMsgObj);
			
			/* 儲存重印管理碼 */
			memset(&srAll, 0x00, sizeof(SQLITE_ALL_TABLE));
			inNCCC_Func_Table_Link_Duty_Free_Pwd(&srAll, _LS_UPDATE_, &inTableID, szReprintPwd, szPWDNew);
			inSqlite_Insert_Or_Replace_ByRecordID_All(gszReprintDBPath, _TABLE_NAME_DUTY_FREE_PWD_, 0, &srAll);

			break;
		}
		else
		{
			/* 輸入錯誤 */
			DISPLAY_OBJECT	srDispMsgObj;
			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PWD_);
			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
			srDispMsgObj.inMsgType = _NO_KEY_MSG_;
			srDispMsgObj.inTimeout = 0;
			strcpy(srDispMsgObj.szErrMsg1, "");
			srDispMsgObj.inErrMsg1Line = 0;
			srDispMsgObj.inBeepTimes = 3;
			srDispMsgObj.inBeepInterval = 1000;

			inDISP_Msg_BMP(&srDispMsgObj);
			continue;
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inFunc_GetSytemInfo
Date&Time       :2022/10/28 上午 11:21
Describe        :
*/
int inFunc_GetSytemInfo(BYTE bID, BYTE* pbBuf)
{
	USHORT	usRetVal = d_OK;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inFunc_GetSytemInfo(%X) START !", bID);
	}
	
	usRetVal = CTOS_GetSystemInfo(bID, pbBuf);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inFunc_GetSytemInfo(%s) END !", pbBuf);
		inLogPrintf(AT, "----------------------------------------");
	}
	
	if (usRetVal == d_OK)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "usRetVal = %04X", usRetVal);
		}
		return (VS_ERROR);
	}
}

/*
Function        :inFunc_Get_UpperCase_Char
Date&Time       :2023/4/13 下午 6:07
Describe        :
*/
int inFunc_Get_UpperCase_Char(char* szString)
{
	int inAmount = 0;
	int i = 0;
	
	for (i = 0; *(szString + i) != 0x00; i++)
	{	
		if (*(szString + i) - 'A' >= 0 && *(szString + i) - 'A' <= 25)
		{
			inAmount++;
		}
	}
	
	return (inAmount);
}