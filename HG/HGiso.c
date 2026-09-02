#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sqlite3.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../CREDIT/Creditfunc.h"
#include "../NCCC/NCCCats.h"
#include "../NCCC/NCCCmfes.h"
#include "../DINERS/DINERSiso.h"
#include "../NCCC/NCCCsrc.h"
#include "HGiso.h"
#include "HGsrc.h"

extern  int		ginDebug;  /* Debug使用 extern */
extern	int		ginISODebug;
extern	int		ginMachineType;
extern	int		ginAPVersionType;
extern	char		gszTermVersionID[16 + 1];
extern	unsigned long	gulDemoRedemptionPointsBalance;
extern  unsigned long	gulDemoHappyGoPoint;	/* DEMO用HG點數 */
extern	unsigned char	guszEnormousNoNeedResetBit;	/* 資料庫是否須從頭找 */

unsigned char	guszHG_ISO_Field03[_HG_PCODE_SIZE_ + 1];
int		ginHGHostIndex = -1;

ISO_FIELD_HG_TABLE srHG_ISOFieldPack[] =
{
        {2,        inHG_Pack02},
        {3,        inHG_Pack03},
        {4,        inHG_Pack04},
        {11,       inHG_Pack11},
        {12,       inHG_Pack12},
        {13,       inHG_Pack13},
        {14,       inHG_Pack14},
        {22,       inHG_Pack22},
        {24,       inHG_Pack24},
        {37,       inHG_Pack37},
        {38,       inHG_Pack38},
        {41,       inHG_Pack41},
        {42,       inHG_Pack42},
        {48,       inHG_Pack48},
        {58,       inHG_Pack58},
        {60,       inHG_Pack60},
        {62,       inHG_Pack62},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_HG_TABLE srHG_ISOFieldUnPack[] =
{
        {12,       inHG_UnPack12},
        {13,       inHG_UnPack13},
        {37,       inHG_UnPack37},
        {38,       inHG_UnPack38},
        {39,       inHG_UnPack39},
        {58,       inHG_UnPack58},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_CHECK_HG_TABLE srHG_ISOFieldCheck[] =
{
        {3,       inHG_Check03},
        {41,      inHG_Check41},
        {0,        NULL},  /* 最後一組一定要放 0 */
};

ISO_FIELD_TYPE_HG_TABLE srHG_ISOFieldType[] =
{
        {2,     _HG_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {3,     _HG_ISO_BCD_,         VS_FALSE,       _HG_PCODE_SIZE_ * 2},
        {4,     _HG_ISO_BCD_,         VS_FALSE,       12},
        {11,    _HG_ISO_BCD_,         VS_FALSE,       6},
        {12,    _HG_ISO_BCD_,         VS_FALSE,       _TIME_SIZE_},
        {13,    _HG_ISO_BCD_,         VS_FALSE,       4},
        {14,    _HG_ISO_BCD_,         VS_FALSE,       4},
        {22,    _HG_ISO_BCD_,         VS_FALSE,       4},
        {24,    _HG_ISO_BCD_,         VS_FALSE,       4},
        {25,    _HG_ISO_BCD_,         VS_FALSE,       2},
        {27,    _HG_ISO_BCD_,         VS_FALSE,       2},
        {35,    _HG_ISO_NIBBLE_2_,    VS_FALSE,       0},
        {37,    _HG_ISO_ASC_,         VS_FALSE,       _HG_RRN_SIZE_},
        {38,    _HG_ISO_ASC_,         VS_FALSE,       _AUTH_CODE_SIZE_},
        {39,    _HG_ISO_ASC_,         VS_FALSE,       2},
        {41,    _HG_ISO_ASC_,         VS_FALSE,       8},
        {42,    _HG_ISO_ASC_,         VS_FALSE,       15},
        {48,    _HG_ISO_BYTE_3_,      VS_TRUE,        0},
        {54,    _HG_ISO_BYTE_3_,      VS_TRUE,        0},
        {55,    _HG_ISO_BYTE_3_,      VS_FALSE,       0},
        {56,    _HG_ISO_BYTE_3_,      VS_FALSE,       0},
        {57,    _HG_ISO_BYTE_3_,      VS_FALSE,       0},
        {58,    _HG_ISO_BYTE_3_,      VS_FALSE,       0},
        {59,    _HG_ISO_BYTE_3_,      VS_TRUE,        0},
        {60,    _HG_ISO_BYTE_3_,      VS_TRUE,        0},
        {61,    _HG_ISO_BYTE_3_,      VS_TRUE,        0},
        {62,    _HG_ISO_BYTE_3_,      VS_TRUE,        0},
        {63,    _HG_ISO_BYTE_3_,      VS_FALSE,       0},
        {0,     _HG_ISO_BCD_,         VS_FALSE,       0},  /* 最後一組一定要放 0 */
};

int inHG_REVERSAL[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_INQUIRY[] = {2, 3, 11, 22, 24, 37, 41, 42, 0}; /* 最後一組一定要放 0!! */
int inHG_FULL_REDEMPTION[] = {2, 3, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_POINT_CERTAIN_CASH[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_POINT_CERTAIN_GIFT_PAPER[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_ONLINE_REDEEM_CASH[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_ONLINE_REDEEM_GIFT_PAPER[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_CASH[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_GIFT_PAPERR[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_SEND_ADVICE[] = {2, 3, 4, 11, 12, 13, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_CREDIT[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_CREDIT_INSIDE[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_CUP[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_ONLINE_REDEEM_CREDIT[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_ONLINE_REDEEM_CREDIT_INSIDE[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_ONLINE_REDEEM_CUP[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_POINT_CERTAIN_CREDIT[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_POINT_CERTAIN_CREDIT_INSIDE[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_POINT_CERTAIN_CUP[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_REDEMPTION_CREDIT[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_REDEMPTION_CREDIT_INSIDE[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_INSTALLMENT_CREDIT[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_INSTALLMENT_CREDIT_INSIDE[] = {2, 3, 4, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REWARD_REFUND[] = {2, 3, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_REDEEM_REFUND[] = {2, 3, 11, 22, 24, 37, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */
int inHG_VOID[] = {2, 3, 4, 11, 22, 24, 37, 38, 41, 42, 58, 60, 62, 0}; /* 最後一組一定要放 0!! */

BIT_MAP_HG_TABLE srHG_ISOBitMap[] =
{
	{_REVERSAL_,				inHG_REVERSAL,				"0400",		"000000"}, /* 沖銷 */
	{_ADVICE_,				inHG_SEND_ADVICE,			"0220",		"019999"}, /* 紅利積點 + SEND_ADVICE */
	{_HG_INQUIRY_,				inHG_INQUIRY,				"0200",		"000010"}, /* 點數查詢 */
	{_HG_FULL_REDEMPTION_,			inHG_FULL_REDEMPTION,			"0200",		"039999"}, /* 點數兌換 */
	{_HG_POINT_CERTAIN_CASH_,		inHG_POINT_CERTAIN_CASH,		"0200",		"039998"}, /* 加價購 + 現金 */
	{_HG_POINT_CERTAIN_GIFT_PAPER_,		inHG_POINT_CERTAIN_GIFT_PAPER,		"0200",		"039998"}, /* 加價購 + 禮券 */
	{_HG_POINT_CERTAIN_CREDIT_,		inHG_POINT_CERTAIN_CREDIT,		"0200",		"039998"}, /* 加價購 + 信用卡 */
	{_HG_POINT_CERTAIN_CREDIT_INSIDE_,	inHG_POINT_CERTAIN_CREDIT_INSIDE,	"0200",		"039998"}, /* 加價購 + HGI */
	{_HG_POINT_CERTAIN_CUP_,		inHG_POINT_CERTAIN_CUP,			"0200",		"039998"}, /* 加價購 + 銀聯卡 */
	{_HG_ONLINE_REDEEM_CASH_,		inHG_ONLINE_REDEEM_CASH,		"0200",		"039997"}, /* 點數扣抵 + 現金 */
	{_HG_ONLINE_REDEEM_GIFT_PAPER_,		inHG_ONLINE_REDEEM_GIFT_PAPER,		"0200",		"039997"}, /* 點數扣抵 + 禮券 */
	{_HG_ONLINE_REDEEM_CREDIT_,		inHG_ONLINE_REDEEM_CREDIT,		"0200",		"039997"}, /* 點數扣抵 + 信用卡 */
	{_HG_ONLINE_REDEEM_CREDIT_INSIDE_,	inHG_ONLINE_REDEEM_CREDIT_INSIDE,	"0200",		"039997"}, /* 點數扣抵 + HGI */
	{_HG_ONLINE_REDEEM_CUP_,		inHG_ONLINE_REDEEM_CUP,			"0200",		"039997"}, /* 點數扣抵 + 銀聯卡 */
	{_HG_REWARD_CASH_,			inHG_REWARD_CASH,			"0200",		"019999"}, /* 紅利積點 + 現金 */
	{_HG_REWARD_GIFT_PAPER_,		inHG_REWARD_GIFT_PAPERR,		"0200",		"019999"}, /* 紅利積點 + 禮券 */
	{_HG_REWARD_CREDIT_,			inHG_REWARD_CREDIT,			"0200",		"019999"}, /* 紅利積點 + 信用卡 */
	{_HG_REWARD_CREDIT_INSIDE_,		inHG_REWARD_CREDIT_INSIDE,		"0200",		"019999"}, /* 紅利積點 + HGI */
	{_HG_REWARD_CUP_,			inHG_REWARD_CUP,			"0200",		"019999"}, /* 紅利積點 + 銀聯卡 */
	{_HG_REWARD_REDEMPTION_CREDIT_,		inHG_REWARD_REDEMPTION_CREDIT,		"0200",		"019999"}, /* 紅利積點 + 紅利扣抵 */
	{_HG_REWARD_REDEMPTION_CREDIT_INSIDE_,	inHG_REWARD_REDEMPTION_CREDIT_INSIDE,	"0200",		"019999"}, /* 紅利積點 + 紅利扣抵 */
	{_HG_REWARD_INSTALLMENT_CREDIT_,	inHG_REWARD_INSTALLMENT_CREDIT,		"0200",		"019999"}, /* 紅利積點 + 分期付款 */
	{_HG_REWARD_INSTALLMENT_CREDIT_INSIDE_,	inHG_REWARD_INSTALLMENT_CREDIT_INSIDE,	"0200",		"019999"}, /* 紅利積點 + 分期付款 */
	{_HG_REWARD_REFUND_,			inHG_REWARD_REFUND,			"0200",		"219999"}, /* 回饋退貨 */
	{_HG_REDEEM_REFUND_,			inHG_REDEEM_REFUND,			"0200",		"239999"}, /* 扣抵退貨 */
	{_VOID_,				inHG_VOID,				"0200",		"110000"}, /* 取消 */
	{_TRANS_TYPE_NULL_,			NULL,					"0000",		"000000"}, /* 最後一組一定要放 _TRANS_TYPE_NULL_!! */
};

ISO_TYPE_HG_TABLE srHG_ISOFunc[] =
{
	{
		srHG_ISOFieldPack,
		srHG_ISOFieldUnPack,
		srHG_ISOFieldCheck,
		srHG_ISOFieldType,
		srHG_ISOBitMap,
		inHG_ISOGetBitMapCode,
		inHG_ISOPackMTI,
		inHG_ISOModifyBitMap,
		inHG_ISOModifyPackData,
		inHG_ISOCheckHeader,
		inHG_ISOOnlineAnalyse,
		inHG_ISOAdviceAnalyse
	},
};

int inHG_Pack02(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0, inPANLen;
	char		szPAN[40 + 1];
	unsigned char	uszBCD[20 + 1];

	memset(szPAN, 0x00, sizeof(szPAN));
	inPANLen = strlen(pobTran->srBRec.szHGPAN);
	uszPackBuf[inCnt ++] = (inPANLen / 10 * 16) + (inPANLen % 10);
	memcpy(szPAN, pobTran->srBRec.szHGPAN, inPANLen);
	if (inPANLen % 2)
		szPAN[inPANLen ++] = '0';

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szPAN[0], inPANLen/2);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], (inPANLen / 2));

	inCnt += (inPANLen / 2);

	return (inCnt);
}

int inHG_Pack03(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;

	memcpy(&uszPackBuf[inCnt], &guszHG_ISO_Field03[0], _HG_PCODE_SIZE_);
	inCnt += _HG_PCODE_SIZE_;

        return (inCnt);
}

int inHG_Pack04(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	char		szTemplate[12 + 1];
	unsigned char	uszBCD[20 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE && 
	    (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_	||
	     pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_))
                sprintf(szTemplate, "%010ld00", (pobTran->srBRec.lnTxnAmount + pobTran->srBRec.lnHGRedeemAmount));
        else
                sprintf(szTemplate, "%010ld00", pobTran->srBRec.lnTxnAmount);

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 6);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 6);
	inCnt += 6;

	return (inCnt);
}

int inHG_Pack11(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	char		szSTAN[6 + 1];
	unsigned char	uszBCD[20 + 1];

	memset(szSTAN, 0x00, sizeof(szSTAN));
	sprintf(szSTAN, "%06ld", pobTran->srBRec.lnHGSTAN);

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szSTAN[0], 3);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 3);
	inCnt += 3;

	return (inCnt);
}

int inHG_Pack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szTime[0], 3);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 3);
	inCnt += 3;

	return (inCnt);
}

int inHG_Pack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szDate[4], 2);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inHG_Pack14(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
        unsigned char	uszBCD[20 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &pobTran->srBRec.szExpDate[0], 2);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inHG_Pack22(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	unsigned char	uszBCD[20 + 1];

	inFunc_ASCII_to_BCD(&uszBCD[0], "0022", 2);
	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inHG_Pack24(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int		inCnt = 0;
	char		szTemplate[10 + 1];
	unsigned char	uszBCD[10 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));
        if (inGetNII(&szTemplate[1]) == VS_ERROR)
        	return (VS_ERROR);
        szTemplate[0] = '0' ;

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 2);

	memcpy((char *)&uszPackBuf[inCnt], (char *)&uszBCD[0], 2);
	inCnt += 2;

	return (inCnt);
}

int inHG_Pack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0;

	memcpy((char *)&uszPackBuf[inCnt], &pobTran->srBRec.szHGRefNo[0], 12);
	inCnt += 12;

	return (inCnt);
}

int inHG_Pack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int 	inCnt = 0, i;
	char	szHGAuthCode[_AUTH_CODE_SIZE_ + 1];


        /* 確認是否授權碼是合法字元 */
	for (i = 0; i < strlen(pobTran->srBRec.szHGAuthCode); i ++)
	{
                if (((pobTran->srBRec.szHGAuthCode[i] >= '0') && (pobTran->srBRec.szHGAuthCode[i] <= '9')) || ((pobTran->srBRec.szHGAuthCode[i] >= 'A') && (pobTran->srBRec.szHGAuthCode[i] <= 'Z')) || (pobTran->srBRec.szHGAuthCode[i] == 0x20))
		{
        		continue;
        	}
        	else
        	{
			return (VS_ERROR);
		}
	}

	if (!memcmp(pobTran->srBRec.szHGAuthCode, "000000", 6))
	{
		return (VS_ERROR);
	}

	memset(szHGAuthCode, 0x00, sizeof(szHGAuthCode));
        strcpy(szHGAuthCode, pobTran->srBRec.szHGAuthCode);
        inFunc_PAD_ASCII(szHGAuthCode, szHGAuthCode, ' ', 6, _PADDING_RIGHT_);

	memcpy((char *)&uszPackBuf[inCnt], &szHGAuthCode[0], _AUTH_CODE_SIZE_);
	inCnt += _AUTH_CODE_SIZE_;

	return (inCnt);
}

int inHG_Pack41(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szTemplate[8 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

	if (inGetTerminalID(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	memcpy((char *)&uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);

	return (inCnt);
}

int inHG_Pack42(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
	char	szTemplate[15 + 1];

	memset(szTemplate, 0x00, sizeof(szTemplate));

        if (inGetMerchantID(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_RIGHT_);
	memcpy((char *)&uszPackBuf[inCnt], szTemplate, strlen(szTemplate));
	inCnt += strlen(szTemplate);

	return (inCnt);
}

int inHG_Pack48(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int     inCnt = 0;
        char	szTemplate[42 + 1];
        
        memset(szTemplate, 0x00, sizeof(szTemplate));
        
        if (inGetCustomIndicator(szTemplate) == VS_ERROR)
                return (VS_ERROR);
        
        uszPackBuf[inCnt ++] = 0x00;
        uszPackBuf[inCnt ++] = 0x19;
        uszPackBuf[inCnt ++] = 0x12;
        memcpy((char *)&uszPackBuf[inCnt], pobTran->srBRec.szStoreID, 18);
        inCnt += 18;
        
        return (inCnt);
}

int inHG_Pack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
	int             inCnt = 0, inPacketCnt = 0, inF58Bit;
	char            szTemplate[50];
        char            szDebugMsg[100 + 1];
        char		szASCII[4 + 1];
	unsigned char	uszBCD[2 + 1];
        unsigned char   uszPacket[50 + 1];
        unsigned char   uszBitMap[8 + 1];
     
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
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inHG_Pack58() START!!");
        }
        
        /* 組【BitMap】*/
	memset(uszPacket, 0x00, sizeof(uszPacket));
        
        switch (pobTran->srBRec.inHGCode)
	{
		case _HG_REWARD_CREDIT_:
		case _HG_REWARD_CUP_:
		case _HG_REWARD_CASH_:
		case _HG_REWARD_GIFT_PAPER_:
		case _HG_REWARD_CREDIT_INSIDE_:
		case _HG_REWARD_INSTALLMENT_CREDIT_:
		case _HG_REWARD_INSTALLMENT_CREDIT_INSIDE_:
		case _HG_REWARD_REDEMPTION_CREDIT_:
		case _HG_REWARD_REDEMPTION_CREDIT_INSIDE_:
			memcpy(&uszPacket[0], "\x00\x00\x00\x00\x00\x00\x20\x00", 8);
			break;
		case _HG_ONLINE_REDEEM_CREDIT_:
		case _HG_ONLINE_REDEEM_CUP_:
		case _HG_ONLINE_REDEEM_CASH_:
		case _HG_ONLINE_REDEEM_GIFT_PAPER_:
		case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
			if (pobTran->inHGTransactionCode == _VOID_)
				memcpy(&uszPacket[0], "\x00\x00\x00\x00\x00\x80\x22\x00", 8);
			else
				memcpy(&uszPacket[0], "\x00\x00\x00\x00\x00\x00\x20\x00", 8);

			break;
		case _HG_POINT_CERTAIN_CREDIT_:
		case _HG_POINT_CERTAIN_CUP_:
		case _HG_POINT_CERTAIN_CASH_:
		case _HG_POINT_CERTAIN_GIFT_PAPER_:
		case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
			memcpy(&uszPacket[0], "\x00\x00\x00\x00\x00\x80\x20\x00", 8);
			break;
		case _HG_REDEEM_REFUND_:
		case _HG_REWARD_REFUND_:
			memcpy(&uszPacket[0], "\x00\x00\x00\x00\x00\x80\x00\x00", 8);
			break;
		case _HG_FULL_REDEMPTION_:
			memcpy(&uszPacket[0], "\x00\x00\x00\x00\x00\x80\x00\x00", 8);
			break;
		default :
			return (VS_ERROR);
	}
        
        if (ginDebug == VS_TRUE)
        {
                memset(szDebugMsg, 0, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "uszPacket: %02X%02X%02X%02X%02X%02X%02X%02X", uszPacket[0], uszPacket[1], uszPacket[2], uszPacket[3], uszPacket[4], uszPacket[5], uszPacket[6], uszPacket[7]);
                inLogPrintf(AT, szDebugMsg);
        }
        
        inPacketCnt += 8;
	memset(uszBitMap, 0x00, sizeof(uszBitMap));
	memcpy(&uszBitMap[0], &uszPacket[0], 8);
        
        for (inF58Bit = 1; inF58Bit <= 64; inF58Bit ++)
	{
		if (inHG_BitMapCheck(uszBitMap, inF58Bit))
		{
                        if (ginDebug == VS_TRUE)
                        {
                                memset(szDebugMsg, 0, sizeof(szDebugMsg));
                                sprintf(szDebugMsg, "inF58Bit: %d", inF58Bit);
                                inLogPrintf(AT, szDebugMsg);
                        }
                        
			switch (inF58Bit)
			{
				case _F58_40_ServerBalPoints_:
					/* 聯合_HAPPY_GO_剩餘點數 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%08ld", pobTran->srBRec.lnHGBalancePoint);
                                        inFunc_ASCII_to_BCD(&uszPacket[inPacketCnt], szTemplate, 4);
					inPacketCnt += 4;
					break;
				case _F58_41_LoyaltyTransPoints_:
					/* 聯合_HAPPY_GO_交易點數  合計 */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%08ld", pobTran->srBRec.lnHGTransactionPoint);
					inFunc_ASCII_to_BCD(&uszPacket[inPacketCnt], szTemplate, 4);
					inPacketCnt += 4;
					break;
				case _F58_51_PaymentTerm_:
					/* 聯合_HAPPY_GO_支付工具_主機回_*/
					if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_ || pobTran->srBRec.lnHGPaymentType == _HG_PAY_CUP_)
						uszPacket[inPacketCnt] = 0x01;
					else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CASH_)
						uszPacket[inPacketCnt] = 0x02;
					else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_GIFT_PAPER_)
						uszPacket[inPacketCnt] = 0x03;
					else if (pobTran->srBRec.lnHGPaymentType == _HG_PAY_CREDIT_INSIDE_)
						uszPacket[inPacketCnt] = 0x01;
					else
						return (VS_ERROR);

					inPacketCnt ++;
					break;
				case _F58_54_LackPointForRefund_:
					break;
				case _F58_55_PayAmount_:
					/* 聯合_HAPPY_GO_扣抵後金額  (商品金額 = lnHGAmount + lnHGRedeemAmt) */
					memset(szTemplate, 0x00, sizeof(szTemplate));
					sprintf(szTemplate, "%08ld", pobTran->srBRec.lnHGAmount);
					inFunc_ASCII_to_BCD(&uszPacket[inPacketCnt], szTemplate, 4);
					inPacketCnt += 4;
					break;
				default:
					break;

			}
		}
	}
        
        /* Packet Data Length */
	memset(szASCII, 0x00, sizeof(szASCII));
	sprintf(szASCII, "%04d", inPacketCnt);
	memset(uszBCD, 0x00, sizeof(uszBCD));
	inFunc_ASCII_to_BCD(uszBCD, szASCII, 2);
	memcpy((char *)&uszPackBuf[inCnt], &uszBCD[0], 2);
	inCnt += 2;
	/* Packet Data */
	memcpy((char *)&uszPackBuf[inCnt], &uszPacket[0], inPacketCnt);
	inCnt += inPacketCnt;
        

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inHG_Pack58() END!!");
        }
        
	return	(inCnt);
}

int inHG_Pack60(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
        long	lnBatchNum;
	char    szTemplate[30 + 1];

	if (pobTran->inISOTxnCode == _TIP_ || 
           (pobTran->inISOTxnCode == _BATCH_UPLOAD_ && pobTran->srBRec.inOrgCode == _TIP_))
	{
		sprintf((char *)&uszPackBuf[inCnt], "%010ld00", pobTran->srBRec.lnTxnAmount);
		inCnt += 12;
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (inGetBatchNum(szTemplate) == VS_ERROR)
			return (VS_ERROR);

		lnBatchNum = atol(szTemplate);
		uszPackBuf[inCnt ++] = 0x00;
		uszPackBuf[inCnt ++] = 0x06;
		sprintf((char *)&uszPackBuf[inCnt], "%06ld", lnBatchNum);
		inCnt += 6;
	}

	return (inCnt);
}

int inHG_Pack62(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackBuf)
{
        int	inCnt = 0;
        char    szTemplate[30 + 1];

        uszPackBuf[inCnt ++] = 0x00;
	uszPackBuf[inCnt ++] = 0x06;
        
        if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
        {
                if (pobTran->inISOTxnCode == _ADVICE_ || pobTran->srBRec.uszVOIDBit == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        if (inGetInvoiceNum(szTemplate) == VS_ERROR)
                                return (VS_ERROR);
                        
                        sprintf((char *)&uszPackBuf[inCnt], "%06ld", atol(szTemplate));
                }
                else 
                {
                        sprintf((char *)&uszPackBuf[inCnt], "%06ld", pobTran->srBRec.lnOrgInvNum);
                }
        }
	else 
        {
                sprintf((char *)&uszPackBuf[inCnt], "%06ld", pobTran->srBRec.lnOrgInvNum);
        }
	
    	inCnt += 6;

	return (inCnt);
}

int inHG_Check03(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
        if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 3))
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_Check03() ERROR!!");
                
                return (VS_ERROR);
        }
        
        return (VS_SUCCESS);
}

int inHG_Check41(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, unsigned char *uszRecePacket)
{
	if (memcmp((char *)uszSendPacket, (char *)uszRecePacket, 8))
	{
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_Check41() ERROR!!");
                
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

int inHG_UnPack12(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode == _ADVICE_) /* 送 Advice 不要 Update 時間 */
		return (VS_SUCCESS);
    
	memset(pobTran->srBRec.szTime, 0x00, sizeof(pobTran->srBRec.szTime));
        inFunc_BCD_to_ASCII(&pobTran->srBRec.szTime[0], &uszUnPackBuf[0], 3);

        return (VS_SUCCESS);
}

int inHG_UnPack13(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        if (pobTran->inISOTxnCode == _REVERSAL_ || pobTran->inISOTxnCode == _ADVICE_) /* 送 Advice 不要 Update 時間 */
		return (VS_SUCCESS);
        
        /* 只改變月份和日期 */
        memset(&pobTran->srBRec.szDate[4], 0x00, 4);
        inFunc_BCD_to_ASCII(&pobTran->srBRec.szDate[4], &uszUnPackBuf[0], 2);

        return (VS_SUCCESS);
}

int inHG_UnPack37(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	memset(pobTran->srBRec.szHGRefNo, 0x00, sizeof(pobTran->srBRec.szHGRefNo));
	memcpy(&pobTran->srBRec.szHGRefNo[0], (char *)&uszUnPackBuf[0], 12);

        return (VS_SUCCESS);
}

int inHG_UnPack38(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	memset(pobTran->srBRec.szHGAuthCode, 0x00, sizeof(pobTran->srBRec.szHGAuthCode));
	memcpy(&pobTran->srBRec.szHGAuthCode[0], (char *)&uszUnPackBuf[0], _AUTH_CODE_SIZE_);

        return (VS_SUCCESS);
}

int inHG_UnPack39(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
	memcpy(&pobTran->srBRec.szHGRespCode[0], (char *)&uszUnPackBuf[0], 2);
	pobTran->srBRec.szHGRespCode[2] = 0x00;

        return (VS_SUCCESS);
}

int inHG_UnPack58(TRANSACTION_OBJECT *pobTran, unsigned char *uszUnPackBuf)
{
        int             inCnt = 0, inF58Bit, inF58TotalLen;
	char            szTemplate[50];
        char            szDebugMsg[100 + 1];
        unsigned char   uszBitMap[8 + 1];
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_UnPack58() START!");
        
        inF58TotalLen = ((uszUnPackBuf[0] / 16 * 10 + uszUnPackBuf[0] % 16) * 100) +
	                 (uszUnPackBuf[1] / 16 * 10 + uszUnPackBuf[1] % 16);
        
        if (ginDebug == VS_TRUE)
        {
                memset(szDebugMsg, 0, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "inF58TotalLen: %d", inF58TotalLen);
                inLogPrintf(AT, szDebugMsg);
        }
        
        if (inF58TotalLen > 8)
	{
		inCnt = 2;
		memset(uszBitMap, 0x00, sizeof(uszBitMap));
		memcpy(&uszBitMap[0], &uszUnPackBuf[2], 8);
		inCnt += 8;
                
                for (inF58Bit = 1; inF58Bit <= 64; inF58Bit ++)
		{
                        if (inHG_BitMapCheck(uszBitMap, inF58Bit))
                        {
                                switch (inF58Bit)
                                {
                                        case _F58_40_ServerBalPoints_:
                                                /* 聯合_HAPPY_GO_剩餘點數 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inFunc_BCD_to_ASCII(szTemplate, (BYTE *)&uszUnPackBuf[inCnt], 4);
						pobTran->srBRec.lnHGBalancePoint = atol((char *)szTemplate);
						inCnt += 4;
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "lnHGBalancePoint: %ld", pobTran->srBRec.lnHGBalancePoint);
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                break;
                                        case _F58_41_LoyaltyTransPoints_:
                                                /* 聯合_HAPPY_GO_交易點數  合計 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inFunc_BCD_to_ASCII(szTemplate, (BYTE *)&uszUnPackBuf[inCnt], 4);
						pobTran->srBRec.lnHGTransactionPoint = atol((char *)szTemplate);
						inCnt += 4;
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "lnHGTransactionPoint: %ld", pobTran->srBRec.lnHGTransactionPoint);
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                break;
                                        case _F58_51_PaymentTerm_:
                                                /* 聯合_HAPPY_GO_支付工具_主機回_*/
						pobTran->srBRec.lnHGPaymentTeam = uszUnPackBuf[inCnt];
						inCnt ++;
                                                break;
                                        case _F58_54_LackPointForRefund_:
                                                /* 聯合_HAPPY_GO_不足點數 */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inFunc_BCD_to_ASCII(szTemplate, (BYTE *)&uszUnPackBuf[inCnt], 4);
						pobTran->srBRec.lnHGRefundLackPoint = atol((char *)szTemplate);
						inCnt += 4;
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szDebugMsg, 0, sizeof(szDebugMsg));
                                                        sprintf(szDebugMsg, "lnHGRefundLackPoint: %ld", pobTran->srBRec.lnHGRefundLackPoint);
                                                        inLogPrintf(AT, szDebugMsg);
                                                }
                                                break;
                                        case _F58_55_PayAmount_:
                                                /* 聯合_HAPPY_GO_扣抵後金額  (商品金額 = lnHGAmount + lnHGRedeemAmt) */
						memset(szTemplate, 0x00, sizeof(szTemplate));
						inFunc_BCD_to_ASCII(szTemplate, (BYTE *)&uszUnPackBuf[inCnt], 4);
						pobTran->srBRec.lnHGAmount = atol((char *)szTemplate);

						if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_	||
						    pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_)
							pobTran->srBRec.lnHGRedeemAmount = pobTran->srBRec.lnOrgTxnAmount - pobTran->srBRec.lnHGAmount;

						inCnt += 4;
                                                break;
                                        default:
                                                break;
                                }
                        }
                }
        }
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_UnPack58() END!");
        
	return (VS_SUCCESS);
}

/*
App Name	: inHG_ISOGetBitMapCode
App Builder	: Michelin
App Date&Time	: 2017-01-23 PM 08:00:11
App Function	: 由 inTxnType 跟 pobTran 來決定欲組成封包的 Bit Map Code
Input Param	: *pobTran --> 交易結構
Input Para2	: inTxnCode --> 交易類別 1. REVERSAL 時會等於 pobTran->srBRec.inCod or pobTran->srBRec.fVoided
                                         2. Advice 交易時會等於 SEND_ADVICE
                                         3. 其餘交易都會等於 pobTran->srBRec.inCode
Output Param	: inBitMapCode --> 欲組成封包的Bit Map Code
*/
int inHG_ISOGetBitMapCode(TRANSACTION_OBJECT *pobTran, int inTxnType)
{
	int	inBitMapCode = -1;
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_ISOGetBitMapCode START");

	switch (inTxnType)
	{
		case _ADVICE_ :
		case _REVERSAL_ :
		case _HG_INQUIRY_ :
		case _HG_FULL_REDEMPTION_ :
		case _HG_REWARD_CREDIT_ :
		case _HG_REWARD_CUP_ :
		case _HG_REWARD_CASH_ :
		case _HG_REWARD_GIFT_PAPER_ :
		case _HG_REWARD_CREDIT_INSIDE_ :
		case _HG_ONLINE_REDEEM_CREDIT_ :
		case _HG_ONLINE_REDEEM_CUP_ :
		case _HG_ONLINE_REDEEM_CASH_ :
		case _HG_ONLINE_REDEEM_GIFT_PAPER_ :
		case _HG_ONLINE_REDEEM_CREDIT_INSIDE_ :
		case _HG_POINT_CERTAIN_CREDIT_ :
		case _HG_POINT_CERTAIN_CUP_ :
		case _HG_POINT_CERTAIN_CASH_ :
		case _HG_POINT_CERTAIN_GIFT_PAPER_ :
		case _HG_POINT_CERTAIN_CREDIT_INSIDE_ :
		case _HG_REWARD_REDEMPTION_CREDIT_ :
		case _HG_REWARD_REDEMPTION_CREDIT_INSIDE_ :
		case _HG_REWARD_INSTALLMENT_CREDIT_ :
		case _HG_REWARD_INSTALLMENT_CREDIT_INSIDE_ :
		case _HG_REDEEM_REFUND_ :
		case _HG_REWARD_REFUND_ :
		case _VOID_ :
			inBitMapCode = inTxnType;
			break;
		default :
			break;
	}
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_ISOGetBitMapCode END");
        
	return (inBitMapCode);
}

/*
App Name	: inHG_ISOPackMTI
App Builder	: Michelin
App Date&Time	: 2017-01-23 PM 07:22:20
App Function	: 將 szMTI 組到 bPackData 裡
Input Param	: *pobTran --> 交易結構
Input Para2	: inTxnCode --> 交易類別 REVERSAL 時，會等於 REVERSAL ，
                                送 Advice 交易時會等於 SEND_ADVICE，
                                其餘交易都會等於 inTxnCode
Input Para3	: *bPackData --> ISO 交易封包
Input Para4	: *szMTI --> 從 BIT_MAP_TABLE 裡面藉著交易類別抓到的 Message Type
		 【格式為 ASCII 需轉為 Hex 格式】
Output Param	: inCnt --> 封包長度
*/
int inHG_ISOPackMTI(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *bPackData, char *szMTI)
{
	int	inCnt = 0;
        unsigned char	uszBCD[20 + 1];

	if (inTxnCode == _REVERSAL_)
        {
		memcpy((char *)&bPackData[inCnt], "\x04\x00", _HG_MTI_SIZE_);
        }
	else if (inTxnCode == _ADVICE_)
        {
		memcpy((char *)&bPackData[inCnt], "\x02\x20", _HG_MTI_SIZE_);
        }
	else
        {
                memset(uszBCD, 0x00, sizeof(uszBCD));
                inFunc_ASCII_to_BCD(&uszBCD[0], &szMTI[0], _HG_MTI_SIZE_);
                memcpy((char *)&bPackData[inCnt], (char *)&uszBCD[0], _HG_MTI_SIZE_);
        }

	inCnt += _HG_MTI_SIZE_;
	return (inCnt);
}

int inHG_ISOPackMessageType(TRANSACTION_OBJECT *pobTran, int inTxnCode, unsigned char *uszPackData, char *szMTI)
{
	int		inCnt = 0;
	unsigned char	uszBCD[10 + 1];

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szMTI[0], _HG_MTI_SIZE_);
	memcpy((char *)&uszPackData[inCnt], (char *)&uszBCD[0], _HG_MTI_SIZE_);
	inCnt += _HG_MTI_SIZE_;

        return (inCnt);
}

/*
App Name	: inHG_ISOModifyBitMap
App Builder	: Michelin
App Date&Time	: 2017-01-23 PM 08:03:22
App Function	: 新增或移除Bit Map到inBitMap
Input Param	: *TransRec --> 交易結構
Input Para2	: inTxnType --> 交易類別/模式,REVERSAL時,會等於REVERSAL,送Advice交易時會等於SEND_ADVICE,其餘交易都會等於TransRec->srBRec.inTXCode
Input Para3	: *inBitMap --> 欲新增或移除的inBitMap
*/
int inHG_ISOModifyBitMap(TRANSACTION_OBJECT *pobTran, int inTxnType, int *inBitMap)
{
        switch (pobTran->srBRec.inHGCode)
	{
		case _HG_FULL_REDEMPTION_ :
		case _HG_REDEEM_REFUND_ :
		case _HG_REWARD_REFUND_ :
			inHG_BitMapReset(inBitMap, 4);
			break;
		default :
			break;
	}
        
        if (strlen(pobTran->srBRec.szHGAuthCode) > 0)
		inHG_BitMapSet(inBitMap, 38);

	if (strlen(pobTran->srBRec.szStoreID) > 0	&&
	    inTxnType != _TIP_)
		inHG_BitMapSet(inBitMap, 48);
        
        return (VS_SUCCESS);
}

int inHG_ISOModifyPackData(TRANSACTION_OBJECT *pobTran, unsigned char *uszPackData, int *inPackLen)
{
        return (VS_SUCCESS);
}

/*
App Name	: inHG_ISOCheckHeader
App Builder	: Michelin
App Date&Time	: 2017-01-23 PM 08:11:33
App Function	: 檢查送出去的 szSendISOHeader 跟收回來的 szReceISOHeader 是否相同
Input Param	: *TransRec --> 交易結構
Input Para2	: *szSendISOHeader --> 送出去的 TPDU+MTI+Bit Map, 此欄位可更改
Input Para3	: *szReceISOHeader --> 收回來的 TPDU+MTI+Bit Map, 此欄位不可更改
Output Param	: 相同 --> VS_SUCCESS
                  不相同 --> VS_ERROR
*/
int inHG_ISOCheckHeader(TRANSACTION_OBJECT *pobTran, char *szSendISOHeader, char *szReceISOHeader)
{
	int	inCnt = 0;

	/* 檢查TPDU */
	inCnt += _HG_TPDU_SIZE_;
	/* 檢查MTI */
	szSendISOHeader[inCnt + 1] += 0x10;
	if (memcmp(&szSendISOHeader[inCnt], &szReceISOHeader[inCnt], _HG_MTI_SIZE_))
		return (VS_ERROR);

	return (VS_SUCCESS);
}

/*
App Name      : inHG_ISOOnlineAnalyse
App Builder   : Michelin
App Date&Time : 2017-01-23 PM 08:13:33
App Function  : Online 交易在收到主機正確的回覆資料後，在此做分析的動作
Input Param   : *pobTran --> 交易結構
Output Param  : 成功 --> VS_SUCCESS
		失敗 --> VS_ERROR
*/
int inHG_ISOOnlineAnalyse(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal;

	inRetVal = inHG_OnlineAnalyseMagneticManual(pobTran);

	return (inRetVal);
}

/*
App Name	: inHG_ISOAdviceAnalyse
App Builder	: Michelin
App Date&Time	: 2017-01-23 PM 08:15:11
App Function	: 在送收完 Advice 後在此做分析的動作
Input Param	: *TransRec --> 交易結構
Input Para2	: blTcUpload --> 是否為當筆的 Tc Upload
Output Param	: 成功 --> VS_SUCCESS
	          失敗 --> VS_ERROR
*/
int inHG_ISOAdviceAnalyse(TRANSACTION_OBJECT *pobTran, unsigned char *uszTcUpload)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_ISOAdviceAnalyse START");

	if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "00", 2))
	{
		if (inHG_CheckAuthCode(pobTran) != VS_SUCCESS)
		{
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "inHG_ISOAdviceAnalyse CHECK_AUTH_ERROR");
			/* 拒絕交易 */
			pobTran->inErrorMsg = _ERROR_CODE_V3_AUTH_CODE_NOT_VALID_;
			pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
				
			return (VS_ERROR);
		}

		/* pobTran->srTRTRec.fUpdate 表示 【fUpdateBatch / TRANS_BATCH_KEY】是要更新記錄 */
		//pobTran->srTRTRec.fUpdate = VS_TRUE; // 待確認

		/* 存檔 */
		if (inFLOW_RunFunction(pobTran, _UPDATE_BATCH_) != VS_SUCCESS)
		{
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "inHG_ISOAdviceAnalyse() PDATE_BATCH_ERROR");
			return (VS_ERROR);
		}

		/* 如果【ADVICE】刪除失敗會鎖機 */
		if (inADVICE_Update(pobTran) != VS_SUCCESS)
                {
			/* 鎖機 */
			inFunc_EDCLock(AT);
			
			return (VS_ERROR);
                }
                
                 if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_ISOAdviceAnalyse() ADV_DELETE_END");
                
		return (VS_SUCCESS);
	}
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_ISOAdviceAnalyse _RESP_ER");
        
	return (VS_ERROR);
}

/*
App Name	: inHG_BitMapSet
App Builder	: Michelin
App Date&Time	: 2017-01-23 PM 20:30:11
App Function	: 在 inBitMap 裡面將 inFeild 新增
Input Param	: *inBitMap --> 欲檢查的 Bit Map
Input Para2	: inFeild -->欲新增的 inFeild
*/
int inHG_BitMapSet(int *inBitMap, int inFeild)
{
	int 	i, inBMapCnt, inBitMapCnt;
	int 	inBMap[_HG_MAX_BIT_MAP_CNT_];

	memset((char *)inBMap, 0x00, sizeof(inBMap));
	inBMapCnt = inBitMapCnt = 0;

	for (i = 0; i < 64; i ++)
	{
		if (inBitMap[inBitMapCnt] > inFeild)
		{
			if (i == 0)
			{
				/* 第一個BitMap */
				inBMap[inBMapCnt ++] = inFeild;
				break;
			}
		}
		else if (inBitMap[inBitMapCnt] < inFeild)
		{
			inBMap[inBMapCnt ++] = inBitMap[inBitMapCnt ++];
			if (inBitMap[inBitMapCnt] == 0 || inBitMap[inBitMapCnt] > inFeild) /* 最後一個 BitMap */
			{
				inBMap[inBMapCnt ++] = inFeild;
				break;
			}
		}
		else
			break; /* if (*inBitMap == inFeild) */
	}

	for (i = 0 ;; i ++)
	{
		if (inBitMap[inBitMapCnt] != 0)
			inBMap[inBMapCnt ++] = inBitMap[inBitMapCnt ++];
		else
			break;
	}

	memcpy((char *)inBitMap, (char *)inBMap, sizeof(inBMap));

	return (VS_SUCCESS);
}

/*
App Name	: inHG_BitMapReset
App Builder	: Michelin
App Date&Time	: 2017-01-23 PM 20:36:19
App Function	: 在 inBitMap 裡面將 inFeild 移除
Input Param	: *inBitMap --> 欲檢查的 Bit Map
Input Para2	: inFeild -->欲移除的 inFeild
*/
int inHG_BitMapReset(int *inBitMap, int inFeild)
{
	int 	i, inBMapCnt;
	int 	inBMap[_HG_MAX_BIT_MAP_CNT_];

	memset((char *)inBMap, 0x00, sizeof(inBMap));
	inBMapCnt = 0;

	for (i = 0; i < 64; i ++)
	{
		if (inBitMap[i] == 0)
			break;
		else if (inBitMap[i] != inFeild)
			inBMap[inBMapCnt ++] = inBitMap[i];
	}

	memcpy((char *)inBitMap, (char *)inBMap, sizeof(inBMap));

	return (VS_SUCCESS);
}

int inHG_BitMapCheck(unsigned char *inBitMap, int inFeild)
{
	int	inByteIndex, inBitIndex;

	inFeild --;
	inByteIndex = inFeild / 8;
	inBitIndex  = 7 - (inFeild - inByteIndex * 8);

	if (_HG_CHECK_BIT_MAP_(inBitMap[inByteIndex], inBitIndex))
	{
		return (VS_TRUE);
	}

	return (VS_FALSE);
}

/*
App Name	: inHG_CopyBitMap
App Builder	: Michelin
App Date&Time	: 2017-01-23 PM 20:42:45
App Function	: 複製結構 inSourceBitMap 裡的 Bit Map 到結構 inBitMap 裡
Input Param	: *inBitMap --> 欲複製的結構 inBitMap
Input Para2	: *inSourceBitMap --> 複製的結構 inSourceBitMap
*/
int inHG_CopyBitMap(int *inBitMap, int *inSearchBitMap)
{
	int 	i;

	for (i = 0; i < _HG_MAX_BIT_MAP_CNT_; i ++)
	{
		if (inSearchBitMap[i] == 0)
			break;
		else
			inBitMap[i] = inSearchBitMap[i];
	}

	return (VS_SUCCESS);
}

int inHG_GetBitMapTableIndex(ISO_TYPE_HG_TABLE *srISOFunc, int inBitMapTxnCode)
{
	int	inBitMapIndex;

	for (inBitMapIndex = 0 ;; inBitMapIndex ++)
	{
		if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == _HG_NULL_TX_)
		{
			return (VS_ERROR); /* 找不到相對應交易別的Bit Map */
		}

		if (srISOFunc->srBitMap[inBitMapIndex].inTxnID == inBitMapTxnCode)
			break; /* 找到一樣的交易類別 */
	}

	return (inBitMapIndex);
}

int inHG_GetBitMapMessagegTypeField03(TRANSACTION_OBJECT *pobTran, ISO_TYPE_HG_TABLE *srISOFunc, int inTxnType, int *inTxnBitMap,
                unsigned char *uszSendBuf)
{
	int		i, j, k, inBitMapIndex, *inBitMap, inCnt, inBitMapTxnCode = -1;
        char		szErrorMessage[256 + 1];
        unsigned char 	uszBuf;

	/* 設定交易別 */
	inBitMapTxnCode = srISOFunc->inGetBitMapCode(pobTran, inTxnType);
	if (inBitMapTxnCode == -1)
		return (VS_ERROR);

	/* 要搜尋 BIT_MAP_HG_TABLE srHG_ISOBitMap 相對應的 inTxnID */
	if ((inBitMapIndex = inHG_GetBitMapTableIndex(srISOFunc, inBitMapTxnCode)) == VS_ERROR)
		return (VS_ERROR);

	/* Pack Message Type */
	inCnt = 0;
	inCnt += srISOFunc->inPackMTI(pobTran, inTxnType, &uszSendBuf[inCnt], srISOFunc->srBitMap[inBitMapIndex].szMTI);

        if (ginDebug == VS_TRUE)
        {
                for(i = 0 ; i < 20 ; i ++)
                {
                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                        sprintf(szErrorMessage, "1.inBitMap = %d", srISOFunc->srBitMap[inBitMapIndex].inBitMap[i]);
                        inLogPrintf(AT, szErrorMessage);
                }
        }

	/* 要搜尋 BIT_MAP_HG_TABLE srHG_ISOBitMap 相對應的 inBitMap */
	inHG_CopyBitMap(inTxnBitMap, srISOFunc->srBitMap[inBitMapIndex].inBitMap);

        if (ginDebug == VS_TRUE)
        {
                for(i = 0 ; i < 20 ; i ++)
                {
                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                        sprintf(szErrorMessage, "2.inBitMap = %d", srISOFunc->srBitMap[inBitMapIndex].inBitMap[i]);
                        inLogPrintf(AT, szErrorMessage);
                }
        }

	/* 修改 Bit Map */
	srISOFunc->inModifyBitMap(pobTran, inTxnType, inTxnBitMap);
	/* Pack Bit Map */
	inBitMap = inTxnBitMap;
	for (i = 0 ;; i ++)
	{
		uszBuf = 0x80;
		if (*inBitMap == 0)
			break;

		j = *inBitMap / 8;
		k = *inBitMap % 8;

		if (k == 0)
		{
			j --;
			k = 8;
		}

		k --;

		while (k)
		{
			uszBuf = uszBuf >> 1;
			k --;
		}

		uszSendBuf[inCnt + j] += uszBuf;
		inBitMap ++;
	}

	inCnt += _HG_BIT_MAP_SIZE_;

	/* Process Code */
	memset(guszHG_ISO_Field03, 0x00, sizeof(guszHG_ISO_Field03));
        inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, 3);

	if (inBitMapTxnCode == _REVERSAL_)
	{
                switch(pobTran->srBRec.inHGCode)
                {
                        case _HG_FULL_REDEMPTION_:
                                inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "039999", _HG_PCODE_SIZE_);
                                break;
                        case _HG_POINT_CERTAIN_CASH_:
                        case _HG_POINT_CERTAIN_GIFT_PAPER_:
                        case _HG_POINT_CERTAIN_CREDIT_:
                        case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
                        case _HG_POINT_CERTAIN_CUP_:
                                inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "039998", _HG_PCODE_SIZE_);
                                break;
                        case _HG_ONLINE_REDEEM_CASH_:
                        case _HG_ONLINE_REDEEM_GIFT_PAPER_:
                        case _HG_ONLINE_REDEEM_CREDIT_:
                        case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
                        case _HG_ONLINE_REDEEM_CUP_:
                                inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "039997", _HG_PCODE_SIZE_);
                                break;
                        case _HG_REWARD_REFUND_:
                                inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "219999", _HG_PCODE_SIZE_);
                                break;
                        case _HG_REDEEM_REFUND_:
                                inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "239999", _HG_PCODE_SIZE_);
                                break;
                        case _HG_REWARD_CASH_:
                        case _HG_REWARD_GIFT_PAPER_:
                        case _HG_REWARD_CREDIT_:
                        case _HG_REWARD_CREDIT_INSIDE_:
			case _HG_REWARD_REDEMPTION_CREDIT_:
			case _HG_REWARD_REDEMPTION_CREDIT_INSIDE_:
			case _HG_REWARD_INSTALLMENT_CREDIT_:
			case _HG_REWARD_INSTALLMENT_CREDIT_INSIDE_:
                        case _HG_REWARD_CUP_:
                                if (pobTran->inHGTransactionCode != _VOID_)
                                {
                                    return (VS_ERROR);
                                }
                                else
                                {
                                        inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "019999", _HG_PCODE_SIZE_);
                                }                                        
                                break;
                        default:
                                return (VS_ERROR);
                }	
                
                if (pobTran->inHGTransactionCode == _VOID_)
                {
                        if (pobTran->srBRec.lnHGPaymentType == _REFUND_)
                        {
                                if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_REFUND_)
                                {
                                        guszHG_ISO_Field03[0] = 0x12;
                                }
                                else if (pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_)
                                {
                                        guszHG_ISO_Field03[0] = 0x14;
                                }
                                else
                                {
                                        return (VS_ERROR);
                                }
                        }
                        else
                        {
                                if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_)
                                {
                                        guszHG_ISO_Field03[0] = 0x13;
                                }
                                else
                                {
                                        guszHG_ISO_Field03[0] = 0x11;
                                }
                        }
			
                }
		
	}	
        else
        {
                inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], srISOFunc->srBitMap[inBitMapIndex].szPCode, _HG_PCODE_SIZE_);
                
                if (pobTran->inHGTransactionCode == _VOID_ && pobTran->inISOTxnCode != _ADVICE_)
                {
                        switch(pobTran->srBRec.inHGCode)
                        {
                                case _HG_FULL_REDEMPTION_:
                                        inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "119999", _HG_PCODE_SIZE_);
                                        break;
                                case _HG_POINT_CERTAIN_CASH_:
                                case _HG_POINT_CERTAIN_GIFT_PAPER_:
                                case _HG_POINT_CERTAIN_CREDIT_:
                                case _HG_POINT_CERTAIN_CREDIT_INSIDE_:
                                case _HG_POINT_CERTAIN_CUP_:
                                        inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "119998", _HG_PCODE_SIZE_);
                                        break;
                                case _HG_ONLINE_REDEEM_CASH_:
                                case _HG_ONLINE_REDEEM_GIFT_PAPER_:
                                case _HG_ONLINE_REDEEM_CREDIT_:
                                case _HG_ONLINE_REDEEM_CREDIT_INSIDE_:
                                case _HG_ONLINE_REDEEM_CUP_:
                                        inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "119997", _HG_PCODE_SIZE_);
                                        break;
                                case _HG_REWARD_CASH_:
                                case _HG_REWARD_GIFT_PAPER_:
                                case _HG_REWARD_CREDIT_:
                                case _HG_REWARD_CREDIT_INSIDE_:
                                case _HG_REWARD_CUP_:
                                case _HG_REWARD_REDEMPTION_CREDIT_:
				case _HG_REWARD_REDEMPTION_CREDIT_INSIDE_:
                                case _HG_REWARD_INSTALLMENT_CREDIT_:
				case _HG_REWARD_INSTALLMENT_CREDIT_INSIDE_:
                                        inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "139999", _HG_PCODE_SIZE_);                                        
                                        break;
                                case _HG_REWARD_REFUND_:
                                        inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "129999", _HG_PCODE_SIZE_);
                                        break;
                                case _HG_REDEEM_REFUND_:
                                        inFunc_ASCII_to_BCD(&guszHG_ISO_Field03[0], "149999", _HG_PCODE_SIZE_);
                                        break;                                
                                default:
                                        return (VS_ERROR);
                        }	
                }
        }
	return (inCnt);
}

int inHG_PackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, int inTxnCode)
{
	int			i, inSendCnt, inField, inCnt;
	int			inBitMap[_HG_MAX_BIT_MAP_CNT_ + 1];
	int			inRetVal, inISOFuncIndex = -1;
	char			szTemplate[40 + 1];
        char			szLogMessage[40 + 1];
	unsigned char		uszBCD[20 + 1];
	ISO_TYPE_HG_TABLE	srISOFunc;

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_PackISO() START!");

	inSendCnt = 0;
	inField = 0;

	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
	inISOFuncIndex = 0; /* 不加密 */
	/* 決定要執行第幾個 Function Index */
	memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *)&srISOFunc, (char *)&srHG_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));
	memset((char *)inBitMap, 0x00, sizeof(inBitMap));

	/* 開始組 ISO 電文 */
	/* Pack TPDU */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetTPDU(szTemplate) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetTPDU() ERROR!");

		return (VS_ERROR);
        }

	memset(uszBCD, 0x00, sizeof(uszBCD));
        inFunc_ASCII_to_BCD(&uszBCD[0], &szTemplate[0], 5);
	memcpy((char *)&uszSendBuf[inSendCnt], (char *)&uszBCD[0], _HG_TPDU_SIZE_);
	inSendCnt += _HG_TPDU_SIZE_;
	/* Get Bit Map / Mesage Type / Processing Code */
	inRetVal = inHG_GetBitMapMessagegTypeField03(pobTran, &srISOFunc, inTxnCode, inBitMap, &uszSendBuf[inSendCnt]);
	if (inRetVal == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_GetBitMapMessagegTypeField03() ERROR!");

		return (VS_ERROR);
        }
	else
		inSendCnt += inRetVal; /* Bit Map 長度 */

	for (i = 0 ;; i ++)
	{
                if (ginDebug == VS_TRUE)
                {
                        memset(szLogMessage, 0x00, sizeof(szLogMessage));
                        sprintf(szLogMessage, "inBitMap[%d] = %d BEGIN", i, inBitMap[i]);
                        inLogPrintf(AT, szLogMessage);
                }
                
		if (inBitMap[i] == 0)
			break;

		while (inBitMap[i] > srISOFunc.srPackISO[inField].inFieldNum)
		{
			inField ++;
		}

		if (inBitMap[i] == srISOFunc.srPackISO[inField].inFieldNum)
		{
			inCnt = srISOFunc.srPackISO[inField].inISOLoad(pobTran, &uszSendBuf[inSendCnt]);
			if (inCnt <= 0)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szLogMessage, 0x00, sizeof(szLogMessage));
                                        sprintf(szLogMessage, "inField[%d] = %d Error", inField, srISOFunc.srPackISO[inField].inFieldNum);
                                        inLogPrintf(AT, szLogMessage);
				}

                                return (VS_ERROR);
                        }
			else
				inSendCnt += inCnt;
		}
                
                if (ginDebug == VS_TRUE)
                {
                        memset(szLogMessage, 0x00, sizeof(szLogMessage));
                        sprintf(szLogMessage, "inBitMap[%d] END", i);
                        inLogPrintf(AT, szLogMessage);
                }
	}

	if (srISOFunc.inModifyPackData != _HG_NULL_TX_)
		srISOFunc.inModifyPackData(pobTran, uszSendBuf, &inSendCnt);

	return (inSendCnt);
}

int inHG_CheckUnPackField(int inField, ISO_FIELD_HG_TABLE *srCheckUnPackField)
{
	int 	i;

	for (i = 0; i < 64; i ++)
	{
		if (srCheckUnPackField[i].inFieldNum == 0)
			break;
		else if (srCheckUnPackField[i].inFieldNum == inField)
		{
			return (VS_SUCCESS); /* i 是 ISO_FIELD_TABLE 裡的第幾個 */
		}
	}

	return (VS_ERROR);
}

int inHG_GetCheckField(int inField, ISO_CHECK_HG_TABLE *ISOFieldCheck)
{
	int 	i;

	for (i = 0; i < 64; i ++)
	{
		if (ISOFieldCheck[i].inFieldNum == 0)
			break;
		else if (ISOFieldCheck[i].inFieldNum == inField)
		{
			return (i); /* i 是 ISO_CHECK_HG_TABLE 裡的第幾個 */
		}
	}

	return (VS_ERROR);
}

int inHG_GetFieldLen(int inField, unsigned char *uszSendData, ISO_FIELD_TYPE_HG_TABLE *srFieldType)
{
	int 	inCnt = 0, i, inLen;

	for (i = 0 ;; i ++)
	{
		if (srFieldType[i].inFieldNum == 0)
			break;

		if (srFieldType[i].inFieldNum != inField)
			continue;

		switch (srFieldType[i].inFieldType)
		{
			case _HG_ISO_ASC_ :
				inCnt += srFieldType[i].inFieldLen;
				break;
			case _HG_ISO_BCD_ :
				inCnt += srFieldType[i].inFieldLen / 2;
				break;
			case _HG_ISO_NIBBLE_2_ :
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += ((inLen + 1) / 2) + 1;
				break;
			case _HG_ISO_NIBBLE_3_ :
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += ((inLen + 1) / 2) + 2;
				break;
			case _HG_ISO_BYTE_2_ :
				inLen = (uszSendData[0] / 16 * 10) + (uszSendData[0] % 16);
				inCnt += inLen + 1;
				break;
			case _HG_ISO_BYTE_3_ :
				inLen = (uszSendData[0] / 16 * 1000) + (uszSendData[0] % 16 * 100) + (uszSendData[1] / 16 * 10) + (uszSendData[1] % 16);
				inCnt += inLen + 2;
				break;
			case _HG_ISO_BYTE_2_H_ :
				inLen = (int)uszSendData[0];
				inCnt += inLen + 1;
				break;
			case _HG_ISO_BYTE_3_H_ :
				inLen = ((int)uszSendData[0] * 0xFF) + (int)uszSendData[1];
				inCnt += inLen + 1;
				break;
			default :
				break;
		}

		break;
	}

	return (inCnt);
}

int inHG_GetFieldIndex(int inField, ISO_FIELD_TYPE_HG_TABLE *srFieldType)
{
        int 	i;

	for (i = 0 ;; i ++)
	{
		if (srFieldType[i].inFieldNum == 0)
			break;

		if (srFieldType[i].inFieldNum == inField)
			return (i);
	}

	return (VS_ERROR);
}

int inHG_UnPackISO(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendBuf, unsigned char *uszRecvBuf)
{
	int			i, inRecvCnt, inCnt, inSendCnt, inSendField, inRecvField, inISOFuncIndex = -1;
	char			szBuf[_HG_TPDU_SIZE_ + _HG_MTI_SIZE_ + _HG_BIT_MAP_SIZE_ + 1];
        char 			szErrorMessage[40 + 1];
	unsigned char		uszSendMap[_HG_BIT_MAP_SIZE_ + 1], uszReceMap[_HG_BIT_MAP_SIZE_ + 1];
	ISO_TYPE_HG_TABLE	srISOFunc;


        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_UnPackISO() START!");

	inSendField = inRecvField = 0;
	inSendCnt = inRecvCnt = 0;

	memset((char *)uszSendMap, 0x00, sizeof(uszSendMap));
	memset((char *)uszReceMap, 0x00, sizeof(uszReceMap));
	memset((char *)szBuf, 0x00, sizeof(szBuf));
	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
	inISOFuncIndex = 0; /* 不加密 */
	memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *)&srISOFunc, (char *)&srHG_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

	if (srISOFunc.inCheckISOHeader != NULL)
	{
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "srISOFunc.inCheckISOHeader != NULL");

		if (srISOFunc.inCheckISOHeader(pobTran, (char *)&uszSendBuf[inRecvCnt], (char *)&uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                {
                        if (ginDebug == VS_TRUE)
                                inLogPrintf(AT, "srISOFunc.inCheckISOHeader ERROR");

                        return (_TRAN_RESULT_UNPACK_ERR_);
                }
	}

	inSendCnt += _HG_TPDU_SIZE_;
	inRecvCnt += _HG_TPDU_SIZE_;
	inSendCnt += _HG_MTI_SIZE_;
	inRecvCnt += _HG_MTI_SIZE_;

	memcpy((char *)uszSendMap, (char *)&uszSendBuf[inSendCnt], _HG_BIT_MAP_SIZE_);
	memcpy((char *)uszReceMap, (char *)&uszRecvBuf[inRecvCnt], _HG_BIT_MAP_SIZE_);

	inSendCnt += _HG_BIT_MAP_SIZE_;
	inRecvCnt += _HG_BIT_MAP_SIZE_;

	/* 先檢查 ISO Field_39 */
	if (inHG_BitMapCheck(uszReceMap, 39) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_BitMapCheck(39) ERROR");

		return (VS_ERROR);
        }

	if (inHG_BitMapCheck(uszReceMap, 41) == VS_FALSE)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_BitMapCheck(41) ERROR");

		return (VS_ERROR);
        }

	for (i = 1; i <= 64; i ++)
	{
		/* 有送出去的 Field 但沒有收回來的 Field */
		if (inHG_BitMapCheck(uszSendMap, i) && !inHG_BitMapCheck(uszReceMap, i))
		{
			inSendCnt += inHG_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
		}
		else if (inHG_BitMapCheck(uszReceMap, i)) /* 收的 BitMap */
		{
			if (inHG_BitMapCheck(uszSendMap, i)) /* 送的 BitMap 都有 */
			{
				/* 是否要進行檢查封包資料包含【送】【收】 */
				if ((inSendField = inHG_GetCheckField(i, srISOFunc.srCheckISO)) != VS_ERROR)
				{
					if (srISOFunc.srCheckISO[inSendField].inISOCheck(pobTran, &uszSendBuf[inSendCnt], &uszRecvBuf[inRecvCnt]) != VS_SUCCESS)
                                        {
                                                if (ginDebug == VS_TRUE)
                                                {
                                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                                        sprintf(szErrorMessage, "inSendField = %d Error!", inSendField);
                                                        inLogPrintf(AT, szErrorMessage);
                                                }

						return (_TRAN_RESULT_UNPACK_ERR_);
                                        }
				}

				inSendCnt += inHG_GetFieldLen(i, &uszSendBuf[inSendCnt], srISOFunc.srISOFieldType);
			}

			while (i > srISOFunc.srUnPackISO[inRecvField].inFieldNum) /* 只有收 */
			{
				inRecvField ++;
			}

			if (i == srISOFunc.srUnPackISO[inRecvField].inFieldNum)
			{
				/* 要先檢查 UnpackISO 是否存在 */
				if (inHG_CheckUnPackField(i, srISOFunc.srUnPackISO) == VS_SUCCESS)
					srISOFunc.srUnPackISO[inRecvField].inISOLoad(pobTran, &uszRecvBuf[inRecvCnt]);
			}

			inCnt = inHG_GetFieldLen(i, &uszRecvBuf[inRecvCnt], srISOFunc.srISOFieldType);
			if (inCnt == VS_ERROR)
			{
                                if (ginDebug == VS_TRUE)
                                {
                                        memset(szErrorMessage, 0x00, sizeof(szErrorMessage));
                                        sprintf(szErrorMessage, "inRecvField = %d Error!", inRecvField);
                                        inLogPrintf(AT, szErrorMessage);
                                }

				return (_TRAN_RESULT_UNPACK_ERR_);
			}

			inRecvCnt += inCnt;
		}
	}

	/* 這裡表示已經解完電文要檢查是否有回 ISO Field_38 */
	if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "00", 2))
	{
		switch (pobTran->inISOTxnCode)
		{
			case _SALE_ :
			case _REFUND_ :
			case _VOID_ :
			case _PRE_AUTH_ :
				if (inHG_BitMapCheck(uszReceMap, 38) == VS_FALSE)
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inHG_BitMapCheck 38 Error!");
                                        }

					return (VS_ERROR);
                                }

				break;
			default :
				break;
		}
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_UnPackISO() END!");

	return (VS_SUCCESS);
}

int inHG_GetSTAN(TRANSACTION_OBJECT *pobTran)
{
        char szSTANNum[12 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_GetSTAN() START!");

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        pobTran->srBRec.lnHGSTAN = atol(szSTANNum);

        return (VS_SUCCESS);
}

int inHG_SetSTAN(TRANSACTION_OBJECT *pobTran)
{
        long    lnSTAN;
        char    szSTANNum[12 + 1];

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        if (inGetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        lnSTAN = atol(szSTANNum);
        if (ginAPVersionType == _APVERSION_TYPE_NCCC_)
	{
		lnSTAN += 1;
		/* SPDH通訊編號只有兩碼，遇到100倍數要多跳一號 */
		if ((lnSTAN % 100) == 0)
		{
			lnSTAN += 1;
		}
			
		if (lnSTAN > 999999)
			lnSTAN = 1;
	}
        else
	{
		lnSTAN += 1;
		if (lnSTAN > 999999)
			lnSTAN = 1;
	}

        memset(szSTANNum, 0x00, sizeof(szSTANNum));
        sprintf(szSTANNum, "%06ld", lnSTAN);
        if (inSetSTANNum(szSTANNum) == VS_ERROR)
                return (VS_ERROR);

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

int inHG_ProcessReversal(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_SUCCESS;
        char    szSendReversalBit[2 + 1];
        char    szOrgHGRefNo[_HG_RRN_SIZE_ + 1];
        

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_ProcessReversal() START!");

        memset(szSendReversalBit, 0x00, sizeof(szSendReversalBit));
        if (inGetSendReversalBit(szSendReversalBit) == VS_ERROR)
                return (VS_ERROR);

        /* 檢查是否要先送 Reversal */
        if (!memcmp(szSendReversalBit, "Y", 1))
        {
                if (pobTran->inTransactionCode == _VOID_)
                {
                        memset(szOrgHGRefNo, 0, sizeof(szOrgHGRefNo));
                        memcpy(&szOrgHGRefNo[0], &pobTran->srBRec.szHGRefNo[0], _HG_RRN_SIZE_);
                }
            
                if ((inRetVal = inHG_ReversalSendRecvPacket(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }

                if (inHG_SetReversalCnt(pobTran, _ADD_) == VS_ERROR)
                {
                        return (VS_ERROR);
                }
                
                if (pobTran->inTransactionCode == _VOID_)
                {
                        memcpy(&pobTran->srBRec.szHGRefNo[0], &szOrgHGRefNo[0], _HG_RRN_SIZE_);
                }
        }

        /* 檢查是否要產生 Reversal */
        if (pobTran->uszReversalBit == VS_TRUE)
        {
                if ((inRetVal = inHG_ReversalSave_Flow(pobTran)) != VS_SUCCESS)
                {
                        return (VS_ERROR);
                }
        }
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_ProcessReversal() END!");

        return (VS_SUCCESS);
}

int inHG_AdviceSendRecvPacket(TRANSACTION_OBJECT *pobTran, int inAdvCnt)
{
	int			inCnt, inRetVal = VS_ERROR, inISOFuncIndex = -1;
        unsigned char		uszTCUpload = 0;
	TRANSACTION_OBJECT	ADVpobTran;
	ISO_TYPE_HG_TABLE	srISOFunc;

	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
	inISOFuncIndex = 0; /* 不加密 */
	memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *)&srISOFunc, (char *)&srHG_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

	memset((char *)&ADVpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
	memcpy((char *)&ADVpobTran, (char *)pobTran, sizeof(TRANSACTION_OBJECT));

        /* 開啟【.bat】【.bkey】【.adv】三個檔 */
	if (inBATCH_AdviceHandleReadOnly_By_Sqlite(pobTran) != VS_SUCCESS)
	{
		return (VS_NO_RECORD);
	}

	/* 這裡的 for () 不可以在裡面直接call Return () ....  */
	for (inCnt = 0; inCnt < inAdvCnt; inCnt ++)
	{
		ADVpobTran.srBRec.lnOrgInvNum = -1;

		if (ADVpobTran.uszTCUploadBit != VS_TRUE)
		{
			/* 這裡要開始逐一將【0220】交易上傳 */
			if (inBATCH_GetAdviceDetailRecord_By_Sqlite(&ADVpobTran, inCnt) == VS_SUCCESS)
			{
				inRetVal = VS_SUCCESS;
			}
			else
			{
				inRetVal = VS_ERROR;
				break;
			}
		}
		
                ADVpobTran.inISOTxnCode = _ADVICE_;

		if (inRetVal == VS_SUCCESS)
			inRetVal = inHG_SendPackRecvUnPack(&ADVpobTran);

		if (inRetVal == VS_SUCCESS)
			if (srISOFunc.inAdviceAnalyse != NULL)
				inRetVal = srISOFunc.inAdviceAnalyse(&ADVpobTran, &uszTCUpload);

		if (inRetVal != VS_SUCCESS)
			break;
	}

	/* 關閉【.bat】【.bket】【.adv】三個檔 */
	inBATCH_GlobalAdviceHandleClose_By_Sqlite();

	return (inRetVal);
}

int inHG_ProcessOnline(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal;
	char    szTemplate[512 + 1];
	char	szCustomIndicator[3 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_ProcessOnline() START!");
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);

	/* 開始組交易封包，送、收、組、解 */
	pobTran->inISOTxnCode = pobTran->inHGTransactionCode; /* 以 TRT Index */
	inRetVal = inHG_SendPackRecvUnPack(pobTran);
	if (inRetVal == VS_COMM_ERROR)
	{
		/* 這裡不用再產生reversal */
		/* (需求單-109327)-vx520客製化需求(昇恆昌客製化) by Russell 2022/5/20 下午 2:26
		 * 客製化075、103要送當筆reversal */
		if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_075_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_103_DUTY_FREE_, _CUSTOMER_INDICATOR_SIZE_))
		{
			inCOMM_End(pobTran);
			inFLOW_RunFunction(pobTran, _COMM_START_);
			pobTran->uszReversalBit = VS_FALSE;
			inHG_ProcessReversal(pobTran);
		}
		
		/* 這裡是通訊失敗，所以不管是不是晶片卡先定義為通訊失敗 */
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
		
		if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", _TRAN_RESULT_COMM_ERROR_);
                        inLogPrintf(AT, szTemplate);
                }
                
                return (inRetVal);
	}
	else if (inRetVal == VS_ISO_PACK_ERR || inRetVal == VS_ISO_UNPACK_ERROR)
        {
		if (inRetVal == VS_ISO_PACK_ERR)
		{
			pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		}
		else if (inRetVal == VS_ISO_UNPACK_ERROR)
		{
			pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		}
		
                if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", inRetVal);
                        inLogPrintf(AT, szTemplate);
                }
                
                return (inRetVal);
        }
	else if (inRetVal == VS_HG_REWARD_COMM_ERR)
	{
		if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "紅利積點失敗轉advice");
                        inLogPrintf(AT, szTemplate);
                }
		
		pobTran->inTransactionResult = _TRAN_RESULT_HG_REWARD_COMM_ERR_;
		return (VS_SUCCESS); /* 紅利積點失敗要當做【ADVICE】處理 */
	}
	else
	{
		/* 決定後續交易的流程先看【Field_39】再看【Field_38】 */
		pobTran->inTransactionResult = inHG_CheckRespCode(pobTran); /* 【Field_39】 */
		
		if (ginDebug == VS_TRUE)
                {
                        memset(szTemplate, 0x00, sizeof(szTemplate));
                        sprintf(szTemplate, "pobTran->inTransactionResult = %d", pobTran->inTransactionResult);
                        inLogPrintf(AT, szTemplate);
                }
		if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_)
		{
			if (inHG_CheckAuthCode(pobTran) != VS_SUCCESS)
                        {
				/* 拒絕交易 */
				pobTran->inErrorMsg = _ERROR_CODE_V3_AUTH_CODE_NOT_VALID_;
				pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
				
				return (VS_ISO_UNPACK_ERROR); /* 主機沒有回覆授權碼 */
                        }
		}
		else if (pobTran->inTransactionResult == _TRAN_RESULT_UNPACK_ERR_)
                {
			return (VS_ISO_UNPACK_ERROR); /* 主機沒有回回覆碼 */
                }
                else if (pobTran->inTransactionResult == _TRAN_RESULT_UNPACK_ERR_)
                {
                        return (VS_ISO_UNPACK_ERROR);
                }
                else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
                {
                        return (VS_SUCCESS);
                }
                else if (pobTran->inTransactionResult == _TRAN_RESULT_HG_REWARD_COMM_ERR_ || pobTran->inTransactionResult == _TRAN_RESULT_HG_REWARD_CANCELLED_ERR_)
                {
                        return (VS_SUCCESS);
                }
                else
                {
                        return (VS_ERROR);
                }
	}

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_ProcessOnline() END!");

        return (VS_SUCCESS);
}

int inHG_ProcessOffline(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	/* Save Advice */
	if (inADVICE_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
	{
		inRetVal = VS_ERROR;
	}
	else
	{
		pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;

		memset(pobTran->srBRec.szHGRespCode, 0x00, sizeof(pobTran->srBRec.szHGRespCode));
		strcpy(pobTran->srBRec.szHGRespCode, "00");
		pobTran->srBRec.szHGRespCode[2] = 0x00;
	}

	return (inRetVal);
}

int inHG_AnalysePacket(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal = VS_SUCCESS, inISOFuncIndex = -1;
	ISO_TYPE_HG_TABLE	srISOFunc;

	if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
	{
                if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_ || 
                    pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_ ||
                    pobTran->inTransactionResult == _TRAN_RESULT_COMM_ERROR_ ||
                    pobTran->inTransactionResult == _TRAN_RESULT_HG_REWARD_COMM_ERR_ || 
                    pobTran->inTransactionResult == _TRAN_RESULT_HG_REWARD_CANCELLED_ERR_)
		{
			/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
			inISOFuncIndex = 0; /* 不加密 */
			memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
			memcpy((char *)&srISOFunc, (char *)&srHG_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

			if (srISOFunc.inOnAnalyse != NULL)
				inRetVal = srISOFunc.inOnAnalyse(pobTran);
		}
		else
		{
			inRetVal = VS_ERROR;
		}
	}

	return (inRetVal);
}

/*
Function        :inHG_ReversalSendRecvPacket
Date&Time       :2017/3/30 上午 11:48
Describe        :
*/
int inHG_ReversalSendRecvPacket(TRANSACTION_OBJECT *pobTran)
{
	int             inRetVal, inSendCnt;
	long            lnREVCnt;
	char		szDialBackupEnable[2 + 1];
	char		szCommMode[2 + 1];
        unsigned char   uszFileName[20 + 1];
	unsigned char   uszSendPacket[_HG_ISO_SEND_ + 1], uszRecvPacket[_HG_ISO_RECV_ + 1];
        unsigned long 	ulREVFHandle;

	memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
	memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));
	/* 如果有DialBackup */
	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);
	memset(szCommMode, 0x00, sizeof(szCommMode));
	inGetCommMode(szCommMode);
	
	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0 && memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
	{
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
			return (VS_ERROR);
	}
	else
	{
		memset(uszFileName, 0x00, sizeof(uszFileName));
		if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
			return (VS_ERROR);
	}

	/* 開檔案 */
	if ((inRetVal = inFILE_OpenReadOnly(&ulREVFHandle, uszFileName)) == VS_ERROR)
	{
		return (VS_ERROR);
	}

	/* 取得 Reversal 檔案大小 */
        lnREVCnt = lnFILE_GetSize(&ulREVFHandle, uszFileName);
	if (lnREVCnt == 0)
	{
		inFILE_Close(&ulREVFHandle);
		return (VS_ERROR);
	}

	/* 指到第一個 BYTE */
	if ((inRetVal = inFILE_Seek(ulREVFHandle, 0, _SEEK_BEGIN_)) == VS_ERROR)
        {
                inFILE_Close(&ulREVFHandle);
                return (VS_ERROR);
	}

	/* 讀檔案 */
	if ((inRetVal = inFILE_Read(&ulREVFHandle, &uszSendPacket[0], lnREVCnt)) == VS_ERROR)
        {
                inFILE_Close(&ulREVFHandle);
                return (VS_ERROR);
	}

	/* 關檔案 */
	inFILE_Close(&ulREVFHandle);

	/* 傳送及接收 ISO 電文 */
        inSendCnt = lnREVCnt % 65536; /* Add by li for ln2int */

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_CommSendRecvToHost() Before");

	if ((inRetVal = inHG_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_CommSendRecvToHost() Error");

		return (VS_ERROR);
	}

	/* 解 ISO 電文 */
	if ((inRetVal = inHG_UnPackISO(pobTran, uszSendPacket, uszRecvPacket)) != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
		
		return (VS_ERROR);
	}

        /* memcmp兩字串相同回傳0 */
	if (memcmp(pobTran->srBRec.szHGRespCode, "00", 2))
	{
		pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
		
		return (VS_ERROR);
	}
	else
	{
		inSetSendReversalBit("N");
		if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
		{
	                return (VS_ERROR);
		}

		/* 如果有DialBackup，要砍兩個Reversal */
		memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
		inGetDialBackupEnable(szDialBackupEnable);
		memset(szCommMode, 0x00, sizeof(szCommMode));
		inGetCommMode(szCommMode);

		if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0 && memcmp(szCommMode, _COMM_MODEM_MODE_, strlen(_COMM_MODEM_MODE_)) == 0)
		{
			/* 刪除撥接reersal */
			memset(uszFileName, 0x00, sizeof(uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
				return (VS_ERROR);
			
			inFILE_Delete(uszFileName);
			
			/* 刪除非撥接Reversal */
			memset(uszFileName, 0x00, sizeof(uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
				return (VS_ERROR);
			
			inFILE_Delete(uszFileName);
		}
		else
		{
			memset(uszFileName, 0x00, sizeof(uszFileName));
			if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_FILE_EXTENSION_, 6) != VS_SUCCESS)
				return (VS_ERROR);
			
			inFILE_Delete(uszFileName);
		}
		
	}

        return (VS_SUCCESS);
}

/*
Function        :inHG_ReversalSave_Flow
Date&Time       :2017/3/30 上午 11:06
Describe        :
*/
int inHG_ReversalSave_Flow(TRANSACTION_OBJECT *pobTran)
{
	int 	inRetVal;
	char	szDialBackupEnable[2 + 1];
	
	inRetVal = inHG_ReversalSave(pobTran);
	if (inRetVal != VS_SUCCESS)
	{
		return (VS_ERROR);
	}

	memset(szDialBackupEnable, 0x00, sizeof(szDialBackupEnable));
	inGetDialBackupEnable(szDialBackupEnable);

	if (memcmp(szDialBackupEnable, "Y", strlen("Y")) == 0)
	{

		inRetVal = inHG_ReversalSave_For_DialBeckUp(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}

	}
	
	return (VS_SUCCESS);
}

int inHG_ReversalSave(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal, inPacketCnt;
	long		lnBatchNum;
	char		szTemplate[20 + 1];
	unsigned char	uszReversalPacket[_HG_ISO_SEND_ + 1];
        unsigned char	uszFileName[20 + 1];
	unsigned long	ulFHandle;

	memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
	pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

	/* 組 REVERSAL 封包 */
	inPacketCnt = inHG_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
	if (inPacketCnt <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
		return (VS_ERROR);
	}

	memset(szTemplate, 0x00, sizeof(szTemplate));
	if (inGetBatchNum(szTemplate) == VS_ERROR)
		return (VS_ERROR);

	lnBatchNum = atol(szTemplate);
        memset(uszFileName, 0x00, sizeof(uszFileName));
        sprintf((char *)uszFileName, "%s%06lu%s", _FILE_NAME_HG_, lnBatchNum, _REVERSAL_FILE_EXTENSION_);

	/* 先刪除上一筆 Reversal，確保是當筆的 Reversal */
        inFILE_Delete(uszFileName);

	inRetVal = inFILE_Create(&ulFHandle, uszFileName);
	if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
	}

	inRetVal = inFILE_Seek(ulFHandle, 0, _SEEK_BEGIN_); /* 從頭開始 */
	if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
	}

	inRetVal = inFILE_Write(&ulFHandle, &uszReversalPacket[0], inPacketCnt);
	if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&ulFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
	}

	inFILE_Close(&ulFHandle);
	inSetSendReversalBit("Y");
	if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
	{
		inFILE_Close(&ulFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
	}

        return (VS_SUCCESS);
}

/*
Function        :inHG_ReversalSave_For_DialBeckUp
Date&Time       :2017/3/30 上午 10:24
Describe        :
*/
int inHG_ReversalSave_For_DialBeckUp(TRANSACTION_OBJECT *pobTran)
{
        int 		inRetVal, inPacketCnt;
        unsigned char 	uszReversalPacket[_HG_ISO_SEND_ + 1];
        unsigned char 	uszFileName[20 + 1];
        unsigned long 	srFHandle;


        memset(uszReversalPacket, 0x00, sizeof(uszReversalPacket));
        pobTran->inISOTxnCode = _REVERSAL_; /* 沖銷 */

        /* 組_REVERSAL_ 封包 */
        inPacketCnt = inHG_PackISO(pobTran, &uszReversalPacket[0], pobTran->inISOTxnCode);
        if (inPacketCnt <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                return (VS_ERROR);
	}

        memset(uszFileName, 0x00, sizeof(uszFileName));
	if (inFunc_ComposeFileName(pobTran, (char*)uszFileName, _REVERSAL_DIALBACKUP_FILE_EXTENSION_, 6) != VS_SUCCESS)
		return (VS_ERROR);
	
        /* 先刪除上一筆 Reversal，確保是當筆的 Reversal */
        inFILE_Delete(uszFileName);

        inRetVal = inFILE_Create(&srFHandle, uszFileName);
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
        }

        inRetVal = inFILE_Seek(srFHandle, 0, _SEEK_BEGIN_); /* 從頭開始 */
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
        }

        inRetVal = inFILE_Write(&srFHandle, &uszReversalPacket[0], inPacketCnt);
        if (inRetVal == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
        }

        inFILE_Close(&srFHandle);
        inSetSendReversalBit("Y");
        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) == VS_ERROR)
        {
                inFILE_Close(&srFHandle);
                inFILE_Delete(uszFileName);

                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

int inHG_CommSendRecvToHost(TRANSACTION_OBJECT *pobTran, unsigned char *uszSendPacket, int inSendLen, unsigned char *uszRecvPacket)
{
        int		inRetVal;
        int		inSendTimeout = 10;
        int		inReceiveTimeout = 10;
        int		inReceiveSize = _COMM_RECEIVE_MAX_LENGTH_;		/* 配合Ingenico */
	char		szTimeOut[2 + 1] = {0};
        char		szHostResponseTimeOut[2 + 1] = {0};
	unsigned char	uszDispBit = VS_TRUE;
        DISPLAY_OBJECT	srDisplay;
        
        /* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
	{
                inLogPrintf(AT, "inHG_CommSendRecvToHost() START");
        }

        memset(&srDisplay, 0x00, sizeof(DISPLAY_OBJECT));
        srDisplay.inY = _LINE_8_4_;
        srDisplay.inR_L = _DISP_LEFT_;
        strcpy(srDisplay.szPromptMsg, "傳送中...");       
	
	/* 如果沒設定TimeOut，就用EDC.dat內的TimeOut */
	if (inSendTimeout <= 0)
	{
		memset(szTimeOut, 0x00, sizeof(szTimeOut));
		inGetIPSendTimeout(szTimeOut);
		inSendTimeout = atoi(szTimeOut);
	}
	
	/* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
	{
		vdHG_ISO_FormatDebug_DISP(pobTran, &uszSendPacket[0], inSendLen);
	}
	/* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
	{
                vdHG_ISO_FormatDebug_PRINT(pobTran, &uszSendPacket[0], inSendLen);
	}
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		/* 不顯示訊息 */
		uszDispBit = VS_FALSE;
	}
	else
	{
		/* 不顯示訊息 */
		uszDispBit = VS_TRUE;
	}
	
        if ((inRetVal = inCOMM_Send(uszSendPacket, inSendLen, inSendTimeout , uszDispBit)) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inCOMM_Send() ERROR");
                return (VS_ERROR);
        }
        
        /* 這裡是用CPT.DAT的東西，所以一定要用 */
	memset(szHostResponseTimeOut, 0x00, sizeof(szHostResponseTimeOut));
        inRetVal = inGetHostResponseTimeOut(szHostResponseTimeOut);

        if (inRetVal != VS_SUCCESS)
                return (VS_ERROR);

        inReceiveTimeout = atoi(szHostResponseTimeOut);

        memset(&srDisplay, 0x00, sizeof(DISPLAY_OBJECT));
        srDisplay.inY = _LINE_8_4_;
        srDisplay.inR_L = _DISP_LEFT_;
        strcpy(srDisplay.szPromptMsg, "接收中...");
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		/* 不顯示訊息 */
		uszDispBit = VS_FALSE;
	}
	else
	{
		/* 不顯示訊息 */
		uszDispBit = VS_TRUE;
	}
	
        if ((inRetVal = inCOMM_Receive(uszRecvPacket, inReceiveSize, inReceiveTimeout, uszDispBit)) != VS_SUCCESS)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inCOMM_Receive() ERROR");

                return (VS_ERROR);
        }
        
        /* 列印ISO Debug */
        if (ginISODebug == VS_TRUE)
        {
                vdHG_ISO_FormatDebug_PRINT(pobTran, &uszRecvPacket[0], inRetVal);
        }
        
        /* 顯示ISO Debug */
	if (ginDebug == VS_TRUE)
	{
                vdHG_ISO_FormatDebug_DISP(pobTran, &uszRecvPacket[0], inRetVal);
                inLogPrintf(AT, "inHG_CommSendRecvToHost() END");
        }

	return (VS_SUCCESS);
}

int inHG_SendPackRecvUnPack(TRANSACTION_OBJECT *pobTran)
{
	int		inSendCnt = 0, inRecvCnt = 0, inRetVal;
	unsigned char	uszSendPacket[_HG_ISO_SEND_ + 1], uszRecvPacket[_HG_ISO_RECV_ + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_SendPackRecvUnPack() START!");

	memset(uszSendPacket, 0x00, sizeof(uszSendPacket));
	memset(uszRecvPacket, 0x00, sizeof(uszRecvPacket));

	/* 組 ISO 電文 */
	if ((inSendCnt = inHG_PackISO(pobTran, uszSendPacket, pobTran->inISOTxnCode)) <= 0)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_PACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_PACK_;
		
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_PackISO() Error!");

		return (VS_ISO_PACK_ERR); /* 組交易電文錯誤，不用組 REVERSAL */
	}

	/* 傳送及接收 ISO 電文 */
	if ((inRecvCnt = inHG_CommSendRecvToHost(pobTran, uszSendPacket, inSendCnt, uszRecvPacket)) != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;

                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_CommSendRecvToHost() Error");

		if (pobTran->inISOTxnCode != _ADVICE_)
		{
			/* 紅利積點失敗要當做【ADVICE】處理 */
                        if (pobTran->inHGTransactionCode == _HG_REWARD_CASH_			||
                            pobTran->inHGTransactionCode == _HG_REWARD_GIFT_PAPER_		||
                            pobTran->inHGTransactionCode == _HG_REWARD_CREDIT_			||
                            pobTran->inHGTransactionCode == _HG_REWARD_CUP_			||
                            pobTran->inHGTransactionCode == _HG_REWARD_CREDIT_INSIDE_		||
                            pobTran->inHGTransactionCode == _HG_REWARD_REDEMPTION_CREDIT_	||
			    pobTran->inHGTransactionCode == _HG_REWARD_REDEMPTION_CREDIT_INSIDE_||
                            pobTran->inHGTransactionCode == _HG_REWARD_INSTALLMENT_CREDIT_	||
			    pobTran->inHGTransactionCode == _HG_REWARD_INSTALLMENT_CREDIT_INSIDE_)
			{
				pobTran->inTransactionResult = _TRAN_RESULT_HG_REWARD_COMM_ERR_;
				
				return (VS_HG_REWARD_COMM_ERR);
			}
			else
				memset(pobTran->srBRec.szHGRespCode, 0x00, sizeof(pobTran->srBRec.szHGRespCode));
		}

		return (VS_COMM_ERROR);
	}

	/* 解 ISO 電文 */
	inRetVal = inHG_UnPackISO(pobTran, uszSendPacket, uszRecvPacket);
	if (inRetVal != VS_SUCCESS)
	{
		pobTran->inTransactionResult = _TRAN_RESULT_UNPACK_ERR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
				
		inRetVal = VS_ISO_UNPACK_ERROR;
	}
        
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_SendPackRecvUnPack() END!");

	return (inRetVal);
}

int inHG_CheckRespCode(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = _TRAN_RESULT_COMM_ERROR_;

	if (!memcmp(pobTran->srBRec.szHGRespCode, "00", 2))
	{
		inRetVal = _TRAN_RESULT_AUTHORIZED_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_NONE_;
	}
	else if (!memcmp(pobTran->srBRec.szHGRespCode, "  ", 2))
        {
		inRetVal = _TRAN_RESULT_UNPACK_ERR_; /* 不是定義的 Response Code */
		pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
        }
	else if (pobTran->srBRec.szHGRespCode[0] == 0x00)
        {
		inRetVal = _TRAN_RESULT_COMM_ERROR_;
		pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
        }
	else if (!memcmp(pobTran->srBRec.szHGRespCode, "D0", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "D1", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "D2", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "D3", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "DA", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "DB", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "DC", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "DD", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "DF", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "F4", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "F5", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "F6", 2) ||
                 !memcmp(pobTran->srBRec.szHGRespCode, "FF", 2))
        {
                inRetVal = _TRAN_RESULT_CANCELLED_;
        }
        else
	{
                if ((pobTran->srBRec.szHGRespCode[0] >= '0' && pobTran->srBRec.szHGRespCode[0] <= '9') && (pobTran->srBRec.szHGRespCode[1] >= '0' && pobTran->srBRec.szHGRespCode[1] <= '9'))
		{
                        inRetVal = _TRAN_RESULT_CANCELLED_;
		}
		else
                {
			inRetVal = _TRAN_RESULT_UNPACK_ERR_; /* 不是定義的 Response Code */
			pobTran->inErrorMsg = _ERROR_CODE_V3_ISO_UNPACK_;
                }
	}
        
	/* 紅利積點混信用卡 只要信用卡過讓HG過 */
        if (memcmp(pobTran->srBRec.szHGRespCode, "00", 2)			&&
           (pobTran->srBRec.uszHappyGoMulti == VS_TRUE				&&
	    pobTran->srBRec.lnHGTransactionType == _HG_REWARD_			&& 
	   (pobTran->srBRec.lnHGPaymentType == _HG_REWARD_CUP_			||
            pobTran->srBRec.lnHGPaymentType == _HG_REWARD_CREDIT_INSIDE_	||
            pobTran->srBRec.lnHGPaymentType == _HG_REWARD_CREDIT_)))
        {
                if (inRetVal == _TRAN_RESULT_CANCELLED_)
			inRetVal = _TRAN_RESULT_HG_REWARD_CANCELLED_ERR_;
		else
                        inRetVal = _TRAN_RESULT_HG_REWARD_COMM_ERR_;
        }       

	return (inRetVal);
}

int inHG_CheckAuthCode(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;

	switch (pobTran->inISOTxnCode)
	{
		case _SETTLE_ :
		case _BATCH_UPLOAD_ :
		case _CLS_BATCH_ :
			break; /* 不檢核 */
		default :
			if (!memcmp(&pobTran->srBRec.szHGAuthCode[0], "000000", 6) || !memcmp(&pobTran->srBRec.szHGAuthCode[0], "      ", 6))
				inRetVal = VS_ERROR;

			break;
	}

	return (inRetVal);
}

int inHG_OnlineAnalyseMagneticManual(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
        int     inHDTIndexHG = -1;
        RTC_NEXSYS	srRTC;				/* Date & Time */
        
        if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHDTIndexHG) != VS_SUCCESS) 
        {
                return (VS_ERROR);
        }

        if (inHDTIndexHG < 0)
        {
                return (VS_ERROR);
        }

	if (pobTran->inTransactionResult == _TRAN_RESULT_AUTHORIZED_) 
        {
                if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                {
                        inSetSendReversalBit("N");                                                
                        if (inSaveHDPTRec(inHDTIndexHG) == VS_ERROR)
                        {
                                return (VS_ERROR);
                        }
                        
                        /* 因為是【Online】交易在這裡送【Advice】 */
                        inHG_AdviceSendRecvPacket(pobTran, 1);
                }                
        }
        else if (pobTran->inTransactionResult == _TRAN_RESULT_HG_REWARD_COMM_ERR_)
        {
                /* 紅利積點不會有【REVERSAL】 */
                if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                {
                        inSetSendReversalBit("N");                        
                        if (inSaveHDPTRec(inHDTIndexHG) == VS_ERROR)
                        {
                                return (VS_ERROR);
                        }
                        
                        /* 要變成一筆【ADVICE】交易 */
                        pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;

                        /* 取得EDC時間日期 */
                        memset(&srRTC, 0x00, sizeof(RTC_NEXSYS));
                        if (inFunc_GetSystemDateAndTime(&srRTC) != VS_SUCCESS)
                        {
                                return (VS_ERROR);
                        }

                        /* 更新pobTran內日期時間 */
                        if (inFunc_Sync_BRec_Date_Time(pobTran, &srRTC) != VS_SUCCESS)
                        {
                                return (VS_ERROR);
                        }
                        
                        /* Save Advice */
                        if (inADVICE_SaveAppend(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
                        {
                                inRetVal = VS_ERROR;
                        }
                }
        }
        else if (pobTran->inTransactionResult == _TRAN_RESULT_HG_REWARD_CANCELLED_ERR_)
        {
		/*  【HAPPY_GO】主機拒絕不送【REVERSAL】【ADVICE】 */
                if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                {
                        inSetSendReversalBit("N");  
                        if (inSaveHDPTRec(inHDTIndexHG) == VS_ERROR)
                        {
                                return (VS_ERROR);
                        }
                        
                        pobTran->inTransactionResult = _TRAN_RESULT_AUTHORIZED_;
                }
        }
        else if (pobTran->inTransactionResult == _TRAN_RESULT_CANCELLED_)
        {
                /* 主機拒絕交易 */
                if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
                {
                        inSetSendReversalBit("N");  
                        if (inSaveHDPTRec(inHDTIndexHG) == VS_ERROR)
                        {
                                return (VS_ERROR);
                        }
                        inHG_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
                        
                        inRetVal = VS_ERROR;
                }
        }
        else
        {
		inHG_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
		
                inRetVal = VS_ERROR;
        }
        

	return (inRetVal);
}

int inHG_ProcessSettleBatchUpload(TRANSACTION_OBJECT *pobTran)
{
	int			inRetVal;
	int			inBatchTotalCnt, i;
	int			inRunCLS_BATCH = VS_TRUE;
	TRANSACTION_OBJECT	OrgpobTran;

	/* 先算出來有多少筆交易資料，如果有會回傳交易總筆數 */
	inBatchTotalCnt = inBATCH_GetTotalCountFromBakFile_By_Sqlite(pobTran);
	if (inBatchTotalCnt == VS_ERROR)
		return (VS_ERROR);
	else if (inBatchTotalCnt == 0)
	{
		/* 主機回《95》但是沒有帳，還要做一次結帳 */
		inRunCLS_BATCH = VS_TRUE;
	}
	else if (inBatchTotalCnt > 0)
	{
		/* 有記錄要開始執行 BATCH UPLOAD */
		pobTran->uszLastBatchUploadBit = VS_FALSE;

		memset((char *)&OrgpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
		memcpy((char *)&OrgpobTran, (char *)pobTran, sizeof(TRANSACTION_OBJECT));

		/* 開始讀取 */
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_START(pobTran);
		/* 預設為無須重找 */
		guszEnormousNoNeedResetBit = VS_TRUE;
		
		for (i = 0; i < inBatchTotalCnt; i ++)
		{
			/* 這裡要顯示【批次上傳】訊息 */

			pobTran->inISOTxnCode = _BATCH_UPLOAD_;
			/* 開始讀批次檔記錄*/
			if (inBATCH_GetDetailRecords_By_Sqlite_Enormous_Read(pobTran, i) != VS_SUCCESS)
			{
				inRunCLS_BATCH = VS_FALSE;
				break;
			}

			/* 抓最新的STAN */
			inHG_GetSTAN(pobTran);

                        /* AE 取消(void)會上傳 batch upload */
                        if (pobTran->srBRec.inCode == _PRE_AUTH_)
				continue;

			if ((i + 1) == inBatchTotalCnt)
				pobTran->uszLastBatchUploadBit = VS_TRUE; /* 最後一筆交易 */

			inRetVal = inHG_SendPackRecvUnPack(pobTran);
			/* 成功或失敗 System Trace Number 都要加一 */
			inHG_SetSTAN(pobTran);
			
			if (inRetVal != VS_SUCCESS)
			{
				inRunCLS_BATCH = VS_FALSE;
				break;
			}
			else
			{
				if (memcmp(pobTran->srBRec.szHGRespCode, "00", 2))
				{
					inHG_DispHostResponseCode(pobTran);		/* 顯示主機回的錯誤訊息 */
					inRunCLS_BATCH = VS_FALSE;
					break;
				}
			}

		}
		/* 結束讀取 */
		inBATCH_GetDetailRecords_By_Sqlite_Enormous_END(pobTran);
		
		/* 還原成原來的【 pobTran 】 */
		memcpy((char *)pobTran, (char *)&OrgpobTran, sizeof(TRANSACTION_OBJECT));
	}

	/* 抓最新的STAN */
	inHG_GetSTAN(pobTran);

	if (inRunCLS_BATCH == VS_TRUE)
	{
		/* 暫時保留
		pobTran->srBRec.uszManualBit = VS_FALSE;
		pobTran->srBRec.uszVOIDBit = VS_FALSE;
		pobTran->srBRec.uszSignatureBit = VS_FALSE;
		pobTran->srBRec.uszOfflineBit = VS_FALSE;
		*/

		pobTran->inISOTxnCode = _CLS_BATCH_;

		inRetVal = inHG_SendPackRecvUnPack(pobTran);
		/* 成功或失敗 System Trace Number 都要加一 */
		inHG_SetSTAN(pobTran);
			
		if (inRetVal != VS_SUCCESS)
			return (VS_ERROR);
		else
		{
			if (memcmp(pobTran->srBRec.szHGRespCode, "00", 2))
				return (VS_ERROR); /* 一定要回【00】 */
		}
	}
	else
	{
		return (VS_ERROR);
	}

	return (VS_SUCCESS);
}

/*
Function        :inHG_DispHostResponseCode
Date&Time       :2016/11/15 下午 6:12
Describe        :顯示錯誤代碼
*/
int inHG_DispHostResponseCode(TRANSACTION_OBJECT *pobTran)
{
	int		inChoice = _DisTouch_No_Event_;
	int		inRetVal = VS_SUCCESS;
	char		szHGLine1[42 + 1] = {0};
	char		szHGLine2[42 + 1] = {0};
	char		szCustomerIndicator[3 + 1] = {0};
        unsigned char   uszKey = 0x00;
	
	if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_	&&
	   (pobTran->srBRec.lnHGPaymentType != _HG_PAY_CASH_	&&
	    pobTran->srBRec.lnHGPaymentType != _HG_PAY_GIFT_PAPER_))
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "紅利積點且不為現金或禮券");
			inLogPrintf(AT, "不顯示錯誤碼");
		}
		return (VS_SUCCESS);
	}
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(szHGLine1, 0x00, sizeof(szHGLine1));
	if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "D0", 2))
		sprintf(szHGLine1, "%s", "HG點數不足D0");		/* HG點數不足D0 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "D1", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易D1");		/* HG拒絕交易D1 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "D2", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易D2");		/* HG拒絕交易D2 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "D3", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易D3");		/* HG拒絕交易D3 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "DA", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易DA");		/* HG拒絕交易DA */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "DB", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易DB");		/* HG拒絕交易DB */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "DC", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易DC");		/* HG拒絕交易DC */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "DD", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易DD");		/* HG拒絕交易DD */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "DE", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易DE");		/* HG拒絕交易DE */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "DF", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易DF");		/* HG拒絕交易DF */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "F4", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易F4");		/* HG拒絕交易F4 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "F5", 2))
		sprintf(szHGLine1, "%s", "卡號尚未開卡F5");		/* 卡號尚未開卡F5 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "F6", 2))
		sprintf(szHGLine1, "%s", "HG拒絕交易F6");		/* HG拒絕交易F6 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "19", 2))
		sprintf(szHGLine1, "%s", "請重試交易19");		/* 請重試交易19 */
	else
		sprintf(szHGLine1, "%s%s", "拒絕交易", pobTran->srBRec.szHGRespCode);	/* 拒絕交易 */
	
	
	memset(szHGLine2, 0x00, sizeof(szHGLine2));
	if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "D0", 2)	|| 
	    !memcmp(&pobTran->srBRec.szHGRespCode[0], "F5", 2))
		sprintf(szHGLine2, "%s %lu %s", "剩餘", pobTran->srBRec.lnHGBalancePoint, "點");		/* 剩餘          點 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "D1", 2) ||
		 !memcmp(&pobTran->srBRec.szHGRespCode[0], "D2", 2) ||
		 !memcmp(&pobTran->srBRec.szHGRespCode[0], "D3", 2))
		sprintf(szHGLine2, "%s", "請用退貨處理");						/* 請用退貨處理 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "DA", 2))
		sprintf(szHGLine2, "%s", "己停卡");							/*      己停卡    */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "DC", 2) || 
		 !memcmp(&pobTran->srBRec.szHGRespCode[0], "DE", 2))
		sprintf(szHGLine2, "%s", "REJECT");							/* REJECT */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "DF", 2))
		sprintf(szHGLine2, "%s", "卡號無效");							/* 卡號無效 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "F4", 2))
		sprintf(szHGLine2, "%s", "此卡號尚未開卡");						/* 此卡號尚未開卡 */
	else if (!memcmp(&pobTran->srBRec.szHGRespCode[0], "F5", 2))
		sprintf(szHGLine2, "%s", "卡號無效");							/* 卡號無效 */
		
	/* 如果當SDK，不用顯示 */
	if (memcmp(gszTermVersionID, "MD731UAGAS001", strlen("MD731UAGAS001")) == 0)
	{
		
	}
	/* 在客製化107.111的時候將請按確認鍵跟請按清除鍵這兩個字樣移除 */
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_107_BUMPER_, _CUSTOMER_INDICATOR_SIZE_)		||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_, _CUSTOMER_INDICATOR_SIZE_)	||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)			||
		 !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
	{
		DISPLAY_OBJECT	srDispMsgObj;
		memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
		
		if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_)	||
		    !memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_124_EVER_RICH_, _CUSTOMER_INDICATOR_SIZE_))
		{
			srDispMsgObj.inTimeout = _CUSTOMER_111_KIOSK_STANDARD_DISPLAY_TIMEOUT_;
		}
		else
		{
			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
		}
		
		strcpy(srDispMsgObj.szDispPic1Name, "");
		srDispMsgObj.inDispPic1YPosition = 0;
		srDispMsgObj.inMsgType = _NO_KEY_MSG_;
		strcpy(srDispMsgObj.szErrMsg1, szHGLine1);
		srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
		strcpy(srDispMsgObj.szErrMsg2, szHGLine2);
		srDispMsgObj.inErrMsg2Line = _LINE_8_6_;
		srDispMsgObj.inBeepTimes = 1;
		srDispMsgObj.inBeepInterval = 0;
		inDISP_Msg_BMP(&srDispMsgObj);

		/* 為了強調Timeout時間 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	}
	else
	{
		/* 清下排 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);

		inDISP_ChineseFont(szHGLine1, _FONTSIZE_8X16_, _LINE_8_5_, _DISP_CENTER_);
		inDISP_ChineseFont(szHGLine2, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_CENTER_);

		inDISP_PutGraphic(_ERR_CLEAR_, 0, _COORDINATE_Y_LINE_8_7_);
		inDISP_BEEP(1, 0);

		inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
		while (1)
		{
			inChoice = inDisTouch_TouchSensor_Click_Slide(_Touch_BATCH_END_);
			uszKey = uszKBD_Key();
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				uszKey = _KEY_TIMEOUT_;
			}

			if (uszKey == _KEY_CANCEL_	||
			    inChoice == _BATCH_END_Touch_ENTER_BUTTON_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else if (uszKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_SUCCESS;
				break;
			}
			else
			{
				continue;
			}
		}
		/* 為了強調Timeout時間 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDisTouch_Flush_TouchFile();
	}
	
        return (inRetVal);
}

int inHG_SyncHostTerminalDateTime(TRANSACTION_OBJECT *pobTran)
{
	return (VS_SUCCESS);
}

int inHG_ProcessReferral(TRANSACTION_OBJECT *pobTran)
{
	/* 檢查是否有支援 REFERRAL */

	/* 輸入授權碼 */
	if (inFunc_REFERRAL_GetManualApproval(pobTran) != VS_SUCCESS)
		return (VS_ERROR);

	return (VS_SUCCESS);
}

int inHG_GetReversalCnt(TRANSACTION_OBJECT *pobTran)
{
        int	inCnt;
        char	szReversalCnt[6 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_GetReversalCnt() START!");

        memset(szReversalCnt, 0x00, sizeof(szReversalCnt));
        if (inGetReversalCnt(szReversalCnt) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetReversalCnt ERROR!");
                return (VS_ERROR);
        }
        inCnt = atoi(szReversalCnt);

        return (inCnt);
}

int inHG_SetReversalCnt(TRANSACTION_OBJECT *pobTran, long lnSetMode)
{
        long	lnCnt;
        char	szReversalCnt[6 + 1];

        memset(szReversalCnt, 0x00, sizeof(szReversalCnt));
        if (inGetReversalCnt(szReversalCnt) == VS_ERROR)
                return (VS_ERROR);

        if (lnSetMode == _ADD_)
        {
                lnCnt = atol(szReversalCnt);
                if (lnCnt++ > 999)
                        lnCnt = 1;
        }
        else if (lnSetMode == _RESET_)
        {
                lnCnt = 0;
        }
        else
        {
                return (VS_ERROR);
        }

        memset(szReversalCnt, 0x00, sizeof(szReversalCnt));
        sprintf(szReversalCnt, "%06ld", lnCnt);
        if (inSetReversalCnt(szReversalCnt) == VS_ERROR)
                return (VS_ERROR);

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

int inHG_GetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inHG_GetMustSettleBit() START!");

	/* 刷完卡就LoadHDPT，不另在Load */
        memset(szMustSettleBit, 0x00, sizeof(szMustSettleBit));
        if (inGetMustSettleBit(szMustSettleBit) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inHG_GetMustSettleBit ERROR!");
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

int inHG_SetMustSettleBit(TRANSACTION_OBJECT *pobTran, char *szMustSettleBit)
{
        if (inSetMustSettleBit(&szMustSettleBit[0]) == VS_ERROR)
                return (VS_ERROR);

        if (inSaveHDPTRec(pobTran->srBRec.inHDTIndex) < 0)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inHG_Func_BuildAndSendPacket_CREDIT
Date&Time       :2017/3/9 下午 5:07
Describe        :HappyGo可以混合大來交易
*/
int inHG_Func_BuildAndSendPacket_CREDIT(TRANSACTION_OBJECT *pobTran)
{
	int	inHGIndex = -1;
        int	inRetVal = VS_ERROR;
	char	szDebugMsg[100 + 1] = {0};
	char	szTRTFileName[12 + 1] = {0};
	char	szFesMode[2 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	memset(szFesMode, 0x00, sizeof(szFesMode));
	inGetNCCCFESMode(szFesMode);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);

	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
	{
		if (memcmp(szFesMode, _NCCC_03_MFES_MODE_, 2) == 0)
		{
			if (memcmp(szCFESMode, "Y", strlen("Y")) == 0)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_ATS_FUNCTION_BUILD_AND_SEND_PACKET_);
			}
			else
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_MFES_FUNCTION_BUILD_AND_SEND_PACKET_);
			}
		}
		else if (memcmp(szFesMode, _NCCC_05_ATS_MODE_, 2) == 0	||
			 memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0)
		{
			inRetVal = inFLOW_RunFunction(pobTran, _NCCC_ATS_FUNCTION_BUILD_AND_SEND_PACKET_);
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inHG_Func_BuildAndSendPacket_CREDIT :無此FES 流程");
				inLogPrintf(AT, szDebugMsg);
			}
			inRetVal = VS_ERROR;
		}
		
	}
	else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_TAKA_, strlen(_TRT_FILE_NAME_TAKA_)))
	{
		inRetVal = inFLOW_RunFunction(pobTran, _TAKA_FUNCTION_BUILD_AND_SEND_PACKET_);
	}
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inHG_Func_BuildAndSendPacket_CREDIT :無此TRT流程");
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = VS_ERROR;
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		/* HappyGo混合交易，若已以授權HG交易（點數扣抵或加價購）而信用卡交易失敗時，要將HG主機的紀錄Reversal掉 */
		if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
		{
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_	||
				    pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_	||
				    pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
				{
					inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHGIndex);
					if (inHGIndex == -1)
					{
						return (VS_ERROR);
					}
					inLoadHDPTRec(inHGIndex);
					inSetSendReversalBit("Y");
					inSaveHDPTRec(inHGIndex);

					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				}

			}
			else
			{
				/* 取消的交易順序又不同 */
				if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_	||
				    pobTran->srBRec.lnHGTransactionType == _HG_REWARD_REFUND_)
				{
					inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inHGIndex);
					if (inHGIndex == -1)
					{
						return (VS_ERROR);
					}
					inLoadHDPTRec(inHGIndex);
					inSetSendReversalBit("Y");
					inSaveHDPTRec(inHGIndex);

					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
				}

			}

		}
		
	}
	
	return (inRetVal);
}

/*
Function        :inHG_Func_BuildAndSendPacket_HG
Date&Time       :2017/3/10 上午 11:16
Describe        :
*/
int inHG_Func_BuildAndSendPacket_HG(TRANSACTION_OBJECT *pobTran)
{
        int     inRetVal = VS_SUCCESS;
        char	szDebugMsg[100 + 1] = {0};
        char	szDemoMode[2 + 1] = {0};
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inHG_BuildAndSendPacket_HG() START!");
                
                memset(szDebugMsg, 0, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "HDT Index: %d", pobTran->srBRec.inHDTIndex);
                inLogPrintf(AT, szDebugMsg);
                
                memset(szDebugMsg, 0, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "uszConnectionBit: %d", pobTran->uszConnectionBit);
                inLogPrintf(AT, szDebugMsg);
        }

	inFunc_ResetTitle(pobTran);
	
	/* 教育訓練模式 */
	memset(szDemoMode, 0x00, sizeof(szDemoMode));
	inGetDemoMode(szDemoMode);
	if (memcmp(szDemoMode, "Y", strlen("Y")) == 0)
	{
		int	i;
		char	szTemplate[100 + 1], szTemplate1[100 + 1], szTemplate2[42 + 1];	

		/* 步驟 1 Get System Trans Number */
		inHG_GetSTAN(pobTran);
		pobTran->srBRec.lnRedemptionPointsBalance = gulDemoRedemptionPointsBalance;
		pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint;

		/* 先計算HappyGO點數 */
		if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
		{
			/* 點數兌換 */
			if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_)
			{
				/* 剩餘點數大於兌換點數 */
				if (gulDemoHappyGoPoint > pobTran->srBRec.lnHGTransactionPoint)
				{
					/* 聯合_HAPPY_GO_剩餘點數 */
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint - pobTran->srBRec.lnHGTransactionPoint);
				}
				else
				{
					/* 點數不足 */
					strcpy(pobTran->srBRec.szHGRespCode, "D0");
					pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
					inHG_DispHostResponseCode(pobTran);
					return (VS_ERROR);
				}
			}
			else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
			{
				/* 加價購 */
				/* 剩餘點數大於要扣的點數 */
				if (gulDemoHappyGoPoint > pobTran->srBRec.lnHGTransactionPoint)
				{
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint - pobTran->srBRec.lnHGTransactionPoint);
				}
				else
				{
					/* 最多只能扣原本的剩餘點數 */
					pobTran->srBRec.lnHGTransactionPoint = gulDemoHappyGoPoint;
					/* HappyGO剩餘點數 */
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = 0;
				}
			}
			else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_)
			{
				/* 點數扣抵 */
				/* 聯合_HAPPY_GO_預計扣抵點數 */
				pobTran->srBRec.lnHGTransactionPoint = ((pobTran->srBRec.lnTxnAmount / 100) * 100);
				
				/* 剩餘點數大於預計扣抵點數 */
				if (gulDemoHappyGoPoint > pobTran->srBRec.lnHGTransactionPoint)
				{
					/* 聯合_HAPPY_GO_點數扣抵金額 */
					pobTran->srBRec.lnHGRedeemAmount = (pobTran->srBRec.lnHGTransactionPoint / 10);
					/* 聯合_HAPPY_GO_實際支付金額 */
					pobTran->srBRec.lnHGAmount = (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnHGRedeemAmount);
					/* 聯合_HAPPY_GO_實際刷卡金額 */
					if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
						pobTran->srBRec.lnTxnAmount = (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnHGRedeemAmount);
					/* 聯合_HAPPY_GO_剩餘點數 */
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint - pobTran->srBRec.lnHGTransactionPoint);
				}
				else
				{
					/* 聯合_HAPPY_GO_點數可扣抵金額 (最多只能扣原本的剩餘點數/10) */
					pobTran->srBRec.lnHGRedeemAmount = (gulDemoHappyGoPoint / 10);
					/* 聯合_HAPPY_GO_扣抵點數 (最多只可扣原本剩餘點數) */
					pobTran->srBRec.lnHGTransactionPoint = gulDemoHappyGoPoint;
					/* 聯合_HAPPY_GO_剩餘點數 */
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = 0;
					/* 聯合_HAPPY_GO_實際刷卡金額 */
					if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
						pobTran->srBRec.lnTxnAmount = (pobTran->srBRec.lnTxnAmount - pobTran->srBRec.lnHGRedeemAmount);
				}

			}
			else if (pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_)
			{
				/* 扣抵退貨 */
				/* 聯合_HAPPY_GO_剩餘點數 */
				pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint + pobTran->srBRec.lnHGTransactionPoint);

				if (gulDemoHappyGoPoint > 100000)
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = 100000;
			}
			else if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_REFUND_)
			{
				/* 回饋退貨 */
				/* 剩餘點數大於回饋點數 */
				if (gulDemoHappyGoPoint > pobTran->srBRec.lnHGTransactionPoint)
				{
					/* 聯合_HAPPY_GO_剩餘點數 */
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint - pobTran->srBRec.lnHGTransactionPoint);
				}
				else
				{
					/* 點數不足 */
					memcpy(&pobTran->srBRec.szHGRespCode[0], "D0", 2);
					pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
					inHG_DispHostResponseCode(pobTran);
					return (VS_ERROR);
				}
			}

		}
		else	/* 負向交易 */
		{
			if (pobTran->srBRec.lnHGTransactionType == _HG_FULL_REDEMPTION_)
			{
				/* HG_FULL_REDEMPTION = 點數兌換 */
				/* 聯合_HAPPY_GO_剩餘點數 */
				pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint + pobTran->srBRec.lnHGTransactionPoint);

				if (gulDemoHappyGoPoint > 100000)
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = 100000;				
			}
			else if (pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
			{
				/* 加價購 */
				/* 聯合_HAPPY_GO_剩餘點數 */
				pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint + pobTran->srBRec.lnHGTransactionPoint);

				if (gulDemoHappyGoPoint > 100000)
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = 100000;				
			}
			else if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_)
			{
				/* 點數扣抵 */
				/* 聯合_HAPPY_GO_剩餘點數 */
				pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint + pobTran->srBRec.lnHGTransactionPoint);

				if (gulDemoHappyGoPoint > 100000)
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = 100000;
			}
			else if (pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_)
			{
				/* 扣抵退貨 */
				/* 剩餘點數大於扣抵點數 */
				if (gulDemoHappyGoPoint > pobTran->srBRec.lnHGTransactionPoint)
				{
					/* 聯合_HAPPY_GO_剩餘點數 */
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint - pobTran->srBRec.lnHGTransactionPoint);
				}
				else
				{
					/* 點數不足 */
					memcpy(&pobTran->srBRec.szHGRespCode[0], "D0", 2);
					pobTran->inTransactionResult = _TRAN_RESULT_CANCELLED_;
					inHG_DispHostResponseCode(pobTran);
					return (VS_ERROR);
				}
			}
			else if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_REFUND_)
			{
				/* 回饋退貨 */
				/* 聯合_HAPPY_GO_剩餘點數 */
				pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = (gulDemoHappyGoPoint + pobTran->srBRec.lnHGTransactionPoint);

				if (gulDemoHappyGoPoint > 100000)
					pobTran->srBRec.lnHGBalancePoint = gulDemoHappyGoPoint = 100000;
			}

		}

		/* 授權碼 = Batch Number + Invoice Number */
		if (pobTran->inTransactionCode == _SALE_)
		{
			if ((!memcmp(pobTran->srBRec.szAuthCode, "VLP", 3) && pobTran->srBRec.uszContactlessBit == VS_TRUE) ||
			    (!memcmp(pobTran->srBRec.szAuthCode, "JCB", 3) && pobTran->srBRec.uszContactlessBit == VS_TRUE) ||
			    (!memcmp(pobTran->srBRec.szAuthCode, "Y1", 2) && pobTran->srBRec.inChipStatus == _EMV_CARD_))
			{

			}
			else if ((pobTran->srBRec.uszCUPTransBit == VS_TRUE || pobTran->srBRec.uszFiscTransBit == VS_TRUE) &&
				 (pobTran->srBRec.lnTxnAmount <= 50))
			{
				/* 教育訓練版需求要點_V1_10 新增空白授權碼 add by LingHsiung 2018/9/21 下午 05:51:57 */
				/* 新增就銀聯卡及SMART PAY之相關交易，於交易金額$50(含)以下，成功交易、但無授權碼之交易 */
				strcpy(pobTran->srBRec.szAuthCode, "      ");
			}
			else
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				memset(szTemplate2, 0x00, sizeof(szTemplate2));

				inGetBatchNum(szTemplate1);
				inGetInvoiceNum(szTemplate2);

				sprintf(szTemplate, "%03d%03d", atoi(szTemplate1), atoi(szTemplate2));
				strcpy(pobTran->srBRec.szAuthCode, szTemplate);
			}
		}
		else
		{
			if (strlen(pobTran->srBRec.szAuthCode) <= 0)
			{
				memset(szTemplate, 0x00, sizeof(szTemplate));
				memset(szTemplate1, 0x00, sizeof(szTemplate1));
				memset(szTemplate2, 0x00, sizeof(szTemplate2));

				inGetBatchNum(szTemplate1);
				inGetInvoiceNum(szTemplate2);

				sprintf(szTemplate, "%03d%03d", atoi(szTemplate1), atoi(szTemplate2));
				strcpy(pobTran->srBRec.szAuthCode, szTemplate);
			}

			if ((pobTran->srBRec.uszCUPTransBit == VS_TRUE || pobTran->srBRec.uszFiscTransBit == VS_TRUE) &&
			    (pobTran->srBRec.lnTxnAmount <= 50))
			{
				/* 教育訓練版需求要點_V1_10 新增空白授權碼 add by LingHsiung 2018/9/21 下午 05:51:57 */
				/* 新增就銀聯卡及SMART PAY之相關交易，於交易金額$50(含)以下，成功交易、但無授權碼之交易 */
				strcpy(pobTran->srBRec.szAuthCode, "      ");
			}
		}

		/* 主機回應碼 */
		strcpy(pobTran->srBRec.szRespCode, "00");
		strcpy(pobTran->srBRec.szHGRespCode, "00");
		
		/* 信用卡交易回再跑去inNCCC_Func_BuildAndSendPacket_Demo_Flow 
		 * 這邊只要處理HappyGO點數 */

		/* Set STAN */
		inHG_SetSTAN(pobTran);

		for (i = 0; i < 2; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_SEND_, 0, _COORDINATE_Y_LINE_8_7_);/* 傳送中... */
		}

		for (i = 0; i < 1; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */
		}

		for (i = 0; i < 1; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_RECEIVE_, 0, _COORDINATE_Y_LINE_8_7_);/* 接收中... */
		}	

		for (i = 0; i < 1; i++)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_);/* 處理中... */
		}
		
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inHG_BuildAndSendPacket_HG() END!");
		
		return (VS_SUCCESS);
	}
	else
	{
		/* 步驟 1 Get System Trans Number */
		if (inHG_GetSTAN(pobTran) == VS_ERROR)
			return (VS_ERROR);   

		/* 確保主機連線資料會重新設定，先斷線 */
		inCOMM_End(pobTran);

		/* 步驟 2.1 開始連線 */
		if (pobTran->uszConnectionBit != VS_TRUE)
		{
			inRetVal = inFLOW_RunFunction(pobTran, _COMM_START_);
			if (inRetVal != VS_SUCCESS)
			{
				/* 如果是紅利積點，如果通訊失敗要送【ADVICE】 */
				if (pobTran->inHGTransactionCode == _HG_REWARD_CASH_			||
				    pobTran->inHGTransactionCode == _HG_REWARD_GIFT_PAPER_		||
				    pobTran->inHGTransactionCode == _HG_REWARD_CREDIT_			||
				    pobTran->inHGTransactionCode == _HG_REWARD_CUP_			||
				    pobTran->inHGTransactionCode == _HG_REWARD_CREDIT_INSIDE_		||
				    pobTran->inHGTransactionCode == _HG_REWARD_REDEMPTION_CREDIT_	||
				    pobTran->inHGTransactionCode == _HG_REWARD_REDEMPTION_CREDIT_INSIDE_||
				    pobTran->inHGTransactionCode == _HG_REWARD_INSTALLMENT_CREDIT_	||
				    pobTran->inHGTransactionCode == _HG_REWARD_INSTALLMENT_CREDIT_INSIDE_)
				{
					inHG_ProcessOffline(pobTran);
					inHG_SetSTAN(pobTran);
					pobTran->uszConnectionBit = VS_TRUE;

					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, "Step 2.1");
					}

					return (VS_SUCCESS);
				}
				else
				{
					/* 通訊失敗‧‧‧‧ */
					pobTran->inTransactionResult = _TRAN_RESULT_COMM_ERROR_;
					pobTran->inErrorMsg = _ERROR_CODE_V3_COMM_;
					inFunc_Display_Error(pobTran);			/* 通訊失敗 */
				}

				return (VS_ERROR);
			}
		}
		/* 步驟 2.2 先送上筆交易失敗的 Reversal 及產生當筆交易 Reversal */
		if ((inRetVal = inHG_ProcessReversal(pobTran)) != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Step 2.2");
			}

			return (VS_ERROR);
		}
		/* 步驟 2.3 如果是取消交易要在這裡檢查是否為紅利積點【ADVICE】 */
		if (pobTran->inHGTransactionCode == _VOID_)
		{
			switch (pobTran->srBRec.inHGCode)
			{
				case _HG_REWARD_CREDIT_:
				case _HG_REWARD_CUP_:
				case _HG_REWARD_CASH_:
				case _HG_REWARD_GIFT_PAPER_:
				case _HG_REWARD_CREDIT_INSIDE_:
				case _HG_REWARD_REDEMPTION_CREDIT_:
				case _HG_REWARD_REDEMPTION_CREDIT_INSIDE_:
				case _HG_REWARD_INSTALLMENT_CREDIT_:
				case _HG_REWARD_INSTALLMENT_CREDIT_INSIDE_:
					if (inHG_SendAdvice_VOID(pobTran, pobTran->srBRec.lnOrgInvNum) != VS_SUCCESS)
					{
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "Step 2.3");
						}
						return (VS_ERROR);
					}

					break;
				default :
					break;
			}
		}         

		/* 步驟 3 處理當筆交易 */
		if (pobTran->srBRec.uszOfflineBit == VS_FALSE)
		{
			if ((inRetVal = inHG_ProcessOnline(pobTran)) != VS_SUCCESS)
			{
				inHG_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Step 3");
				}

				return (VS_ERROR); /* Return 【VS_ERROR】【VS_SUCCESS】 */
			}
		}       

		inHG_SetSTAN(pobTran); /* 成功或失敗 System_Trace_Number 都要加一 */

		/* 1. 表示有收到完整的資料後的分析
		   2. 處理 Online & Offline 交易
		 */
		inRetVal = inHG_AnalysePacket(pobTran);
		if (inRetVal != VS_SUCCESS)
		{
			return (inRetVal);
		}

		/* 避免撥接太久，收送完就斷線 */
		inCOMM_End(pobTran);

		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inHG_BuildAndSendPacket_HG() END!");

		return (VS_SUCCESS);
	}
}

/*
Function        :inHG_Func_BuildAndSendPacket_HG_Flow
Date&Time       :2017/3/10 上午 11:15
Describe        :
*/
int inHG_Func_BuildAndSendPacket_HG_Flow(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inCREDITIndex = -1;
	
	inRetVal = inHG_Func_BuildAndSendPacket_HG(pobTran);
	
	if (inRetVal != VS_SUCCESS)
	{
		/* HappyGo混合交易，若已以授權CREDIT交易（紅利積點）而HG交易失敗時，不用將CREDIT主機的紀錄Reversal掉 */
		if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE)
		{
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				/* 紅利積點 信用卡成功 HG失敗 不Reversal 信用卡交易 */
				if (pobTran->srBRec.lnHGTransactionType == _HG_REWARD_	||
				    pobTran->srBRec.lnHGTransactionType == _HG_REDEEM_REFUND_)
				{
					return (VS_SUCCESS);
				}
				else
				{
					return (inRetVal);
				}

			}
			else
			{
				/* 取消的交易順序又不同 */
				if (pobTran->srBRec.lnHGTransactionType == _HG_ONLINE_REDEEM_	||
				    pobTran->srBRec.lnHGTransactionType == _HG_POINT_CERTAIN_)
				{
					inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_CREDIT_NCCC_, &inCREDITIndex);
					if (inCREDITIndex == -1)
					{
						return (VS_ERROR);
					}
					inLoadHDPTRec(inCREDITIndex);
					inSetSendReversalBit("Y");
					inSaveHDPTRec(inCREDITIndex);

					inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
					return (VS_SUCCESS);
				}
				else
				{
					return (inRetVal);
				}
				
			}

		}
		else
		{
			return (inRetVal);
		}
		
	}
	else
	{
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inHG_Func_SetHost_CREDIT
Date&Time       :2017/02/13 下午 04:56
Describe        :選擇信用卡Host
*/
int inHG_Func_SetHost_CREDIT(TRANSACTION_OBJECT *pobTran)
{
        char	szBatchNum [6 + 1], szInvoiceNum [6 + 1], szSTANNum [6 + 1];
        
        if (pobTran->srBRec.inHGCreditHostIndex < 0)
        {
                /* 如果HG Credit Index == -1直接跳出，回傳ERROR */
                return (VS_ERROR);
        }
        
        pobTran->srBRec.inHDTIndex = pobTran->srBRec.inHGCreditHostIndex;

        inLoadHDTRec(pobTran->srBRec.inHDTIndex);
        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
        
	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	inGetBatchNum(szBatchNum);
	pobTran->srBRec.lnBatchNum = atol(szBatchNum);
	
	memset(szSTANNum, 0x00, sizeof(szSTANNum));
        inGetSTANNum(szSTANNum);
        pobTran->srBRec.lnSTANNum = atol(szSTANNum);
	
	/* 取消混合交易不用抓批號和調閱編號 */
	if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE && pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		
	}
	else
	{
		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetInvoiceNum(szInvoiceNum);
		pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
	}
        
        return (VS_SUCCESS);
}

/*
Function        :inHG_Func_SetHost_HG
Date&Time       :2017/02/13 下午 04:56
Describe        :選擇HG Host
*/
int inHG_Func_SetHost_HG(TRANSACTION_OBJECT *pobTran)
{
        int     inHDTIndexHG = -1;
        char	szBatchNum [6 + 1], szInvoiceNum [6 + 1], szSTANNum [6 + 1];
        
        if (inFunc_Find_Specific_HDTindex(pobTran->srBRec.inHDTIndex, _HOST_NAME_HG_, &inHDTIndexHG) != VS_SUCCESS)
        {
                /* 如果沒有HG HOST 直接跳出，回傳ERROR */
                return (VS_ERROR);
        }

        if (inHDTIndexHG < 0)
        {
                /* 如果HG HOST Index == -1直接跳出，回傳ERROR */
                return (VS_ERROR);
        }
        
        pobTran->srBRec.inHDTIndex = inHDTIndexHG;
        
        inLoadHDTRec(pobTran->srBRec.inHDTIndex);
        inLoadHDPTRec(pobTran->srBRec.inHDTIndex);
        
	memset(szBatchNum, 0x00, sizeof(szBatchNum));
	inGetBatchNum(szBatchNum);
	pobTran->srBRec.lnBatchNum = atol(szBatchNum);
		
	memset(szSTANNum, 0x00, sizeof(szSTANNum));
        inGetSTANNum(szSTANNum);
        pobTran->srBRec.lnSTANNum = atol(szSTANNum);
	
	/* 取消混合交易不用抓批號和調閱編號 */
	if (pobTran->srBRec.uszHappyGoMulti == VS_TRUE && pobTran->srBRec.uszVOIDBit == VS_TRUE)
	{
		
	}
	else
	{
		memset(szInvoiceNum, 0x00, sizeof(szInvoiceNum));
		inGetInvoiceNum(szInvoiceNum);
		pobTran->srBRec.lnOrgInvNum = atol(szInvoiceNum);
	}

        return (VS_SUCCESS);
}

/*
Function        :inHG_Func_SetTxnOnlineOffline
Date&Time       :2017/02/15 上午 11:15
Describe        :根據交易別決定是否Online，HappyGo可以混合大來交易
*/
int inHG_Func_SetTxnOnlineOffline_CREDIT(TRANSACTION_OBJECT *pobTran)
{
        int	inRetVal;
	char	szDebugMsg[100 + 1];
	char	szTRTFileName[12 + 1];
	char	szFesMode[2 + 1];
	
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	
	memset(szFesMode, 0x00, sizeof(szFesMode));
	inGetNCCCFESMode(szFesMode);
	
	if (!memcmp(szTRTFileName, _TRT_FILE_NAME_CREDIT_, strlen(_TRT_FILE_NAME_CREDIT_)))
	{
		if (memcmp(szFesMode, _NCCC_03_MFES_MODE_, 2) == 0)
		{
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_DCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_);
			}
			else
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_MFES_FUNCTION_SET_TXN_ONLINE_OFFLINE_);
			}
		}
		else if (memcmp(szFesMode, _NCCC_05_ATS_MODE_, 2) == 0	||
			 memcmp(szFesMode, _NCCC_04_MPAS_MODE_, 2) == 0)
		{
			if (pobTran->srBRec.uszDCCTransBit == VS_TRUE)
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_DCC_FUNCTION_SET_TXN_ONLINE_OFFLINE_);
			}
			else
			{
				inRetVal = inFLOW_RunFunction(pobTran, _NCCC_ATS_FUNCTION_SET_TXN_ONLINE_OFFLINE_);
			}
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "inHG_Func_SetTxnOnlineOffline :EMV無此FES 流程");
				inLogPrintf(AT, szDebugMsg);
			}
			inRetVal = VS_ERROR;
		}
		
		return (inRetVal);
	}
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	else if (!memcmp(szTRTFileName, _TRT_FILE_NAME_TAKA_, strlen(_TRT_FILE_NAME_TAKA_)))
	{
		inRetVal = inFLOW_RunFunction(pobTran, _TAKA_FUNCTION_SET_TXN_ONLINE_OFFLINE_);
		
		return (inRetVal);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inHG_Func_SetTxnOnlineOffline :EMV無此TRT流程");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
}

int inHG_Func_SetTxnOnlineOffline_HG(TRANSACTION_OBJECT *pobTran)
{
        /* HG都是Online交易 */
        pobTran->srBRec.uszOfflineBit = VS_FALSE;
	/* offline交易及settle不需要做reversal */
        pobTran->uszReversalBit = VS_TRUE;

	/* 紅利積點混合交易除了取消其他不產生Reversal */
	/* 點數查詢不Reversal */
        switch (pobTran->srBRec.lnHGTransactionType)
        {
		case _HG_REWARD_:
			if (pobTran->srBRec.uszVOIDBit != VS_TRUE)
			{
				pobTran->uszReversalBit = VS_FALSE;
			}
			break;
		case _HG_INQUIRY_:
			pobTran->uszReversalBit = VS_FALSE;
			break;
		default:
			break;
        }

        return (VS_SUCCESS);
}

int inHG_SendAdvice_VOID(TRANSACTION_OBJECT *pobTran, long lnAdvInvoice)
{
        int                     inRetVal = VS_SUCCESS;
        int                     inISOFuncIndex = -1;
        int                     inADVIndex = -1;
        TRANSACTION_OBJECT	ADVpobTran;
	ISO_TYPE_HG_TABLE	srISOFunc;
	
	/* 先確認是否有ADVICE要送 */
	if (inADVICE_GetTotalCount(pobTran) == VS_NO_RECORD)
		return (VS_SUCCESS);

	/* 加密模式，預設值 = 0。【0 = 不加密，1 = tSAM 加密，2 = 軟體加密】 */
	inISOFuncIndex = 0; /* 不加密 */
	memset((char *)&srISOFunc, 0x00, sizeof(srISOFunc));
	memcpy((char *)&srISOFunc, (char *)&srHG_ISOFunc[inISOFuncIndex], sizeof(srISOFunc));

	memset((char *)&ADVpobTran, 0x00, sizeof(TRANSACTION_OBJECT));
	memcpy((char *)&ADVpobTran, (char *)pobTran, sizeof(TRANSACTION_OBJECT));
        
        /* 開啟【.bat】【.bkey】【.adv】三個檔 */
	if (inBATCH_AdviceHandleReadOnly_By_Sqlite(pobTran) != VS_SUCCESS)
		return (VS_NO_RECORD);
        
        inRetVal = inADVICE_SearchRecord_Index(&ADVpobTran, lnAdvInvoice, &inADVIndex);
        
        if (inADVIndex >= 0)
        {
                ADVpobTran.srBRec.lnOrgInvNum = lnAdvInvoice;
                inRetVal = inBATCH_GetTransRecord(&ADVpobTran);
        }
        else
        {
                inRetVal = VS_SUCCESS;
        }
        
        ADVpobTran.inISOTxnCode = _ADVICE_; /* 表示做結帳前要先送【Advice】所以交易日期、時間不應該被更新 */
        
        if (inRetVal == VS_SUCCESS)
        {
                inRetVal = inHG_SendPackRecvUnPack(&ADVpobTran);
        }
        
        if (inRetVal == VS_SUCCESS)
        {
                if (srISOFunc.inAdviceAnalyse != NULL)
                {
                    inRetVal = srISOFunc.inAdviceAnalyse(&ADVpobTran, VS_FALSE);
                }
        }
        
        if (inRetVal == VS_SUCCESS)
        {
                memset(pobTran->srBRec.szHGAuthCode, 0x00, sizeof(pobTran->srBRec.szHGAuthCode));
                strcpy(pobTran->srBRec.szHGAuthCode, ADVpobTran.srBRec.szHGAuthCode);
        }
        
        /* 關閉【.bat】【.bket】【.adv】三個檔 */
	inBATCH_GlobalAdviceHandleClose_By_Sqlite();
        
        return (inRetVal);
}

/*
Function        :inHG_GetHG_Enable
Date&Time       :2017/9/13 上午 10:13
Describe        :傳HostEnable進來，會得到'Y' OR 'N'
*/
int inHG_GetHG_Enable(int inOrgHDTIndex, char *szHostEnable)
{
	/* 此function只用來查詢HG是否開，不應該切換Host */
	/* 若之前已查詢到HG的index為何，就可以直接使用 */
	if (ginHGHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_HG_, &ginHGHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}
	}
	

	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginHGHostIndex) < 0)
	{
		if (inOrgHDTIndex >= 0)
		{
			/* 回覆原本的Host */
			inLoadHDTRec(inOrgHDTIndex);
		}
		else
		{
			/* load回信用卡主機 */
			inLoadHDTRec(0);
		}

		return (VS_ERROR);
	}
		
	/* 判斷HOST是否開啟  */
	memset(szHostEnable, 0x00, 1);
	inGetHostEnable(szHostEnable);
	if (szHostEnable[0] != 'Y')
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "HG Enable not open.");
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "HG Enable open.");
		}
	}

	if (inOrgHDTIndex >= 0)
	{
		/* 回覆原本的Host */
		inLoadHDTRec(inOrgHDTIndex);
	}
	else
	{
		/* load回信用卡主機 */
		inLoadHDTRec(0);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inHG_SwitchToHG_Host
Date&Time       :22017/9/13 上午 10:13
Describe        :切換到HDT中HG的REC上，之後要回覆就靠inOrgHDTIndex（在此function中回傳錯誤也會回覆原host）
*/
int inHG_SwitchToHG_Host(int inOrgHDTIndex)
{
	/* 開機後只找一次HG */
	if (ginHGHostIndex != -1)
	{
		/* 直接使用 */
	}
	else
	{
		if (inFunc_Find_Specific_HDTindex(inOrgHDTIndex, _HOST_NAME_HG_, &ginHGHostIndex) != VS_SUCCESS)
		{
			/* 找不到直接return VS_ERROR */
			return (VS_ERROR);
		}

	}
	
	/* 如果Load失敗，回復原Host並回傳VS_ERROR(理論上不會有此情況) */
	if (inLoadHDTRec(ginHGHostIndex) < 0)
	{
		if (inOrgHDTIndex >= 0)
		{
			/* 回覆原本的Host */
			inLoadHDTRec(inOrgHDTIndex);
		}
		else
		{
			/* load回信用卡主機 */
			inLoadHDTRec(0);
		}

		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :vdHG_ISO_FormatDebug_DISP
Date&Time       :2017/1/24 下午 1:24
Describe        :顯示ISO Debug 
*/
void vdHG_ISO_FormatDebug_DISP(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34;
	int			i, inPrintLineCnt = 0, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	ISO_TYPE_HG_TABLE       srISOTypeTable;

	memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
	memcpy((char *)&srISOTypeTable, (char *)&srHG_ISOFunc[0], sizeof(srISOTypeTable));

        inLogPrintf(AT, "==========================================");

        if (uszDebugBuf[6] == 0x00 || uszDebugBuf[6] == 0x20 || uszDebugBuf[6] == 0x40)
                inLogPrintf(AT, "ISO8583 Format  <<Send Data>>");
	else
                inLogPrintf(AT, "ISO8583 Format  <<Recvive Data>>");

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, "TPDU = [");
        inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 5);
	strcat(szBuf,"]");
        inLogPrintf(AT, szBuf);
	inCnt += 5;

	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf,"MTI  = [");
        inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 2);
	strcat(szBuf,"]");
        inLogPrintf(AT, szBuf);
	inCnt += 2;

	memset(szBuf, 0x00, sizeof(szBuf));
	memset(szBitMap, 0x00, sizeof(szBitMap));
	memcpy(szBitMap, (char *)&uszDebugBuf[inCnt], 8);
	sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
						uszDebugBuf[inCnt + 0],
						uszDebugBuf[inCnt + 1],
						uszDebugBuf[inCnt + 2],
						uszDebugBuf[inCnt + 3],
						uszDebugBuf[inCnt + 4],
						uszDebugBuf[inCnt + 5],
						uszDebugBuf[inCnt + 6],
						uszDebugBuf[inCnt + 7]);
        inLogPrintf(AT, szBuf);
	memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
	memset(uszBitMap, 0x00, sizeof(uszBitMap));
	memcpy(uszBitMap, (char *)&uszDebugBuf[inCnt], 8);

	for (inBitMapCnt1 = 0; inBitMapCnt1 < 8; inBitMapCnt1 ++)
        {
                for (inBitMapCnt2 = 0; inBitMapCnt2 < 8; inBitMapCnt2 ++)
                {
                        if (uszBitMap[inBitMapCnt1] & 0x80)
                                uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '1';
                        else
                                uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '0';

                        uszBitMap[inBitMapCnt1] = uszBitMap[inBitMapCnt1] << 1;
            }
        }

        inLogPrintf(AT, "  1234567890   1234567890   1234567890");
	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, "0>%10.10s 1>%10.10s 2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
        inLogPrintf(AT, szBuf);
        sprintf(szBuf, "3>%10.10s 4>%10.10s 5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
        inLogPrintf(AT, szBuf);
        sprintf(szBuf, "6>%4.4s", &uszBitMapDisp[61]);
        inLogPrintf(AT, szBuf);

	inCnt += 8;

	for (i = 1; i <= 64; i ++)
	{
		if (!inHG_BitMapCheck((unsigned char *)szBitMap, i))
			continue;

		inLen = 0;
		inFieldLen = 0;
		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "F_%02d ", i);
		inLen = 5;
                strcat(&szBuf[inLen ++], "[");
                
		switch (i)
		{
			case 3 :
				strcat(szBuf, "P_CODE ");
				inLen += 7;
				break;
			case 4 :
				strcat(szBuf, "AMT ");
				inLen += 4;
				break;
			case 11 :
				strcat(szBuf, "STAN ");
				inLen += 5;
				break;
			case 12 :
				strcat(szBuf, "TIME ");
				inLen += 5;
				break;
			case 13 :
				strcat(szBuf, "DATE ");
				inLen += 5;
				break;
			case 24 :
				strcat(szBuf, "NII ");
				inLen += 4;
				break;
			case 32 :
				strcat(szBuf, "AIIC ");
				inLen += 5;
				break;
			case 35 :
				break;
			case 37 :
				strcat(szBuf, "RRN ");
				inLen += 4;
				break;
			case 38 :
				strcat(szBuf, "AUTH_ID ");
				inLen += 8;
				break;
			case 39 :
				strcat(szBuf, "RESP_CODE ");
				inLen += 10;
				break;
			case 41 :
				strcat(szBuf, "TID ");
				inLen += 4;
				break;
			case 42 :
				strcat(szBuf, "MID ");
				inLen += 4;
				break;
			case 48 :
				strcat(szBuf, "STORE_ID ");
				inLen += 9;
				break;
			case 52 :
				strcat(szBuf, "PIN_BLOCK ");
				inLen += 10;
				break;
			case 54 :
				strcat(szBuf, "TIP_AMT ");
				inLen += 8;
				break;
			case 55 :
				strcat(szBuf, "EMV ");
				inLen += 4;
				break;
			case 56 :
				strcat(szBuf, "EMV ");
				inLen += 4;
				break;
			case 57 :
				strcat(szBuf, "CSUM ");
				inLen += 5;
				break;
			case 58 :
				strcat(szBuf, "TABLE ");
				inLen += 6;
				break;
			case 59 :
				strcat(szBuf, "TABLE ");
				inLen += 6;
				break;
			case 64 :
				strcat(szBuf, "MAC ");
				inLen += 4;
				break;
			default :
				break;
		}       
		
		inField = inHG_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
		inFieldLen = inHG_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

		switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
		{
			case _HG_ISO_ASC_  :
				memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				break;
                        case _HG_ISO_BYTE_3_  :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], 2);
				inCnt += 2;
				inLen += 4;
				strcat(szBuf, " ");
				inLen ++;
				inFieldLen --;
				inFieldLen --;

				if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
				{
					if (i != 58 && i != 59)
						memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
				}
				else
				{
					if (i != 55 && i != 56)
                                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
				}

				break;
			case _HG_ISO_NIBBLE_2_  :
			case _HG_ISO_BYTE_2_  :
                                inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
				inLen += 2;
				strcat(szBuf, " ");
				inLen ++;

				inFieldLen --;
				if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
				else
                                        inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);

				break;
			case _HG_ISO_BCD_  :
				if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
				else
                                        inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);

				break;
			default :
				break;
		}

                strcat(&szBuf[inLen ++], "]");
                
		inPrintLineCnt = 0;
		while ((inPrintLineCnt * inOneLineLen) < strlen(szBuf))
		{
			memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
			memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
			if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szBuf))
			{
				strcat(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen]);
			}
			else
			{
				memcpy(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen], inOneLineLen);
			}
			
			sprintf(szPrtBuf, "  %s", szPrintLineData);
			
			inLogPrintf(AT, szPrtBuf);
			inPrintLineCnt ++;
		};

		if (i == 58)
			vdHG_ISO_FormatDebug_DISP_58(&uszDebugBuf[inCnt], inFieldLen);
                
		inCnt += inFieldLen;
	}

        inLogPrintf(AT, "==========================================");
}

void vdHG_ISO_FormatDebug_DISP_58(unsigned char *uszDebugBuf, int inFieldLen)
{
        
}

/*
Function        :vdHG_ISO_FormatDebug_PRINT
Date&Time       :2017/1/24 下午 1:24
Describe        :列印ISO Debug
*/
void vdHG_ISO_FormatDebug_PRINT(TRANSACTION_OBJECT *pobTran, unsigned char *uszDebugBuf, int inSendLen)
{
	int			inOneLineLen = 34, inPrintLineCnt = 0;
	int			i, inCnt = 0, inField, inLen, inFieldLen;
	int			inBitMapCnt1, inBitMapCnt2;
	char			szPrtBuf[50 + 1], szBuf[1536 + 1], szBitMap[8 + 1];
	char			szPrintLineData[36 + 1];
	unsigned char		uszBitMap[9], uszBitMapDisp[66];
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
	ISO_TYPE_HG_TABLE       srISOTypeTable;

	/* 是否有列印功能 */
	if (inFunc_Check_Print_Capability(ginMachineType) != VS_SUCCESS)
	{
		
	}
	else
	{
		/* 初始化 */
		inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

		memset((char *)&srISOTypeTable, 0x00, sizeof(srISOTypeTable));
		memcpy((char *)&srISOTypeTable, (char *)&srHG_ISOFunc[0], sizeof(srISOTypeTable));

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		if (uszDebugBuf[6] == 0x00 || uszDebugBuf[6] == 0x20 || uszDebugBuf[6] == 0x40)
			inPRINT_Buffer_PutIn("ISO8583 Format  <<Send Data>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		else
			inPRINT_Buffer_PutIn("ISO8583 Format  <<Recvive Data>>", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "TPDU = [");
		inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 5);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += 5;

		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf,"MTI  = [");
		inFunc_BCD_to_ASCII(&szBuf[8], &uszDebugBuf[inCnt], 2);
		strcat(szBuf,"]");
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		inCnt += 2;

		memset(szBuf, 0x00, sizeof(szBuf));
		memset(szBitMap, 0x00, sizeof(szBitMap));
		memcpy(szBitMap, (char *)&uszDebugBuf[inCnt], 8);
		sprintf(szBuf, "BMap = [%02X %02X %02X %02X %02X %02X %02X %02X]",
							uszDebugBuf[inCnt + 0],
							uszDebugBuf[inCnt + 1],
							uszDebugBuf[inCnt + 2],
							uszDebugBuf[inCnt + 3],
							uszDebugBuf[inCnt + 4],
							uszDebugBuf[inCnt + 5],
							uszDebugBuf[inCnt + 6],
							uszDebugBuf[inCnt + 7]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(uszBitMapDisp, 0x00, sizeof(uszBitMapDisp));
		memset(uszBitMap, 0x00, sizeof(uszBitMap));
		memcpy(uszBitMap, (char *)&uszDebugBuf[inCnt], 8);

		for (inBitMapCnt1 = 0; inBitMapCnt1 < 8; inBitMapCnt1 ++)
		{
			for (inBitMapCnt2 = 0; inBitMapCnt2 < 8; inBitMapCnt2 ++)
			{
				if (uszBitMap[inBitMapCnt1] & 0x80)
					uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '1';
				else
					uszBitMapDisp[(inBitMapCnt1*8) + inBitMapCnt2 + 1] = '0';

				uszBitMap[inBitMapCnt1] = uszBitMap[inBitMapCnt1] << 1;
		    }
		}

		inPRINT_Buffer_PutIn("  1234567890   1234567890   1234567890", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		memset(szBuf, 0x00, sizeof(szBuf));
		sprintf(szBuf, "0>%10.10s 1>%10.10s 2>%10.10s", &uszBitMapDisp[1], &uszBitMapDisp[11], &uszBitMapDisp[21]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, "3>%10.10s 4>%10.10s 5>%10.10s", &uszBitMapDisp[31], &uszBitMapDisp[41], &uszBitMapDisp[51]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
		sprintf(szBuf, "6>%4.4s", &uszBitMapDisp[61]);
		inPRINT_Buffer_PutIn(szBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inCnt += 8;

		for (i = 1; i <= 64; i ++)
		{
			if (!inHG_BitMapCheck((unsigned char *)szBitMap, i))
				continue;

			inLen = 0;
			inFieldLen = 0;
			memset(szBuf, 0x00, sizeof(szBuf));
			sprintf(szBuf, "F_%02d ", i);
			inLen = 5;
			strcat(&szBuf[inLen ++], "[");

			switch (i)
			{
				case 3 :
					strcat(szBuf, "P_CODE ");
					inLen += 7;
					break;
				case 4 :
					strcat(szBuf, "AMT ");
					inLen += 4;
					break;
				case 11 :
					strcat(szBuf, "STAN ");
					inLen += 5;
					break;
				case 12 :
					strcat(szBuf, "TIME ");
					inLen += 5;
					break;
				case 13 :
					strcat(szBuf, "DATE ");
					inLen += 5;
					break;
				case 24 :
					strcat(szBuf, "NII ");
					inLen += 4;
					break;
				case 32 :
					strcat(szBuf, "AIIC ");
					inLen += 5;
					break;
				case 35 :
					break;
				case 37 :
					strcat(szBuf, "RRN ");
					inLen += 4;
					break;
				case 38 :
					strcat(szBuf, "AUTH_ID ");
					inLen += 8;
					break;
				case 39 :
					strcat(szBuf, "RESP_CODE ");
					inLen += 10;
					break;
				case 41 :
					strcat(szBuf, "TID ");
					inLen += 4;
					break;
				case 42 :
					strcat(szBuf, "MID ");
					inLen += 4;
					break;
				case 48 :
					strcat(szBuf, "STORE_ID ");
					inLen += 9;
					break;
				case 52 :
					strcat(szBuf, "PIN_BLOCK ");
					inLen += 10;
					break;
				case 54 :
					strcat(szBuf, "TIP_AMT ");
					inLen += 8;
					break;
				case 55 :
					strcat(szBuf, "EMV ");
					inLen += 4;
					break;
				case 56 :
					strcat(szBuf, "EMV ");
					inLen += 4;
					break;
				case 57 :
					strcat(szBuf, "CSUM ");
					inLen += 5;
					break;
				case 58 :
					strcat(szBuf, "TABLE ");
					inLen += 6;
					break;
				case 59 :
					strcat(szBuf, "TABLE ");
					inLen += 6;
					break;
				case 64 :
					strcat(szBuf, "MAC ");
					inLen += 4;
					break;
				default :
					break;
			}       

			inField = inHG_GetFieldIndex(i, srISOTypeTable.srISOFieldType);
			inFieldLen = inHG_GetFieldLen(i, &uszDebugBuf[inCnt], srISOTypeTable.srISOFieldType);

			switch (srISOTypeTable.srISOFieldType[inField].inFieldType)
			{
				case _HG_ISO_ASC_  :
					memcpy(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					break;
				case _HG_ISO_BYTE_3_  :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], 2);
					inCnt += 2;
					inLen += 4;
					strcat(szBuf, " ");
					inLen ++;
					inFieldLen --;
					inFieldLen --;

					if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
					{
						if (i != 58 && i != 59)
							memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
					}
					else
					{
						if (i != 55 && i != 56)
							inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);
					}

					break;
				case _HG_ISO_NIBBLE_2_  :
				case _HG_ISO_BYTE_2_  :
					inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt ++], 1);
					inLen += 2;
					strcat(szBuf, " ");
					inLen ++;

					inFieldLen --;
					if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
						memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
					else
						inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);

					break;
				case _HG_ISO_BCD_  :
					if (srISOTypeTable.srISOFieldType[inField].uszDispAscii == VS_TRUE)
						memcpy(&szBuf[inLen], (char *)&uszDebugBuf[inCnt], inFieldLen);
					else
						inFunc_BCD_to_ASCII(&szBuf[inLen], &uszDebugBuf[inCnt], inFieldLen);

					break;
				default :
					break;
			}

			strcat(&szBuf[inLen ++], "]");

			inPrintLineCnt = 0;
			while ((inPrintLineCnt * inOneLineLen) < strlen(szBuf))
			{
				memset(szPrintLineData, 0x00, sizeof(szPrintLineData));
				memset(szPrtBuf, 0x00, sizeof(szPrtBuf));
				if (((inPrintLineCnt + 1) * inOneLineLen) > strlen(szBuf))
				{
					strcat(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen]);
				}
				else
				{
					memcpy(szPrintLineData, &szBuf[inPrintLineCnt * inOneLineLen], inOneLineLen);
				}

				sprintf(szPrtBuf, "  %s", szPrintLineData);

				inPRINT_Buffer_PutIn(szPrtBuf, _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
				inPrintLineCnt ++;
			};

			if (i == 58)
				vdHG_ISO_FormatDebug_PRINT_58(&uszDebugBuf[inCnt], inFieldLen, uszBuffer, &srFont_Attrib, &srBhandle);		

			inCnt += inFieldLen;
		}

		inPRINT_Buffer_PutIn("==========================================", _PRT_ISO_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

		inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
	}
}

void vdHG_ISO_FormatDebug_PRINT_58(unsigned char *uszDebugBuf, int inFieldLen, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
        
}
