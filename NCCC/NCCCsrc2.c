#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctosapi.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <libxml/tree.h>
#include "../SOURCE/INCLUDES/Define_1.h"
#include "../SOURCE/INCLUDES/Define_2.h"
#include "../SOURCE/INCLUDES/TransType.h"
#include "../SOURCE/INCLUDES/Transaction.h"
#include "../SOURCE/INCLUDES/AllStruct.h"
#include "../SOURCE/DISPLAY/Display.h"
#include "../SOURCE/DISPLAY/DispMsg.h"
#include "../SOURCE/DISPLAY/DisTouch.h"
#include "../SOURCE/PRINT/Print.h"
#include "../SOURCE/COMM/Comm.h"
#include "../SOURCE/COMM/Modem.h"
#include "../SOURCE/COMM/Ftps.h"
#include "../SOURCE/COMM/TLS.h"
#include "../SOURCE/FUNCTION/Sqlite.h"
#include "../SOURCE/FUNCTION/ASMC.h"
#include "../SOURCE/FUNCTION/Accum.h"
#include "../SOURCE/FUNCTION/Batch.h"
#include "../SOURCE/FUNCTION/File.h"
#include "../SOURCE/FUNCTION/Function.h"
#include "../SOURCE/FUNCTION/FuncTable.h"
#include "../SOURCE/FUNCTION/CDT.h"
#include "../SOURCE/FUNCTION/CDTX.h"
#include "../SOURCE/FUNCTION/Card.h"
#include "../SOURCE/FUNCTION/CPT.h"
#include "../SOURCE/FUNCTION/HDT.h"
#include "../SOURCE/FUNCTION/CFGT.h"
#include "../SOURCE/FUNCTION/EDC.h"
#include "../SOURCE/FUNCTION/HDPT.h"
#include "../SOURCE/FUNCTION/ECR.h"
#include "../SOURCE/FUNCTION/RS232.h"
#include "../SOURCE/FUNCTION/SCDT.h"
#include "../SOURCE/FUNCTION/VWT.h"
#include "../SOURCE/FUNCTION/TDT.h"
#include "../SOURCE/FUNCTION/PIT.h"
#include "../SOURCE/FUNCTION/NexsysSDK.h"
#include "../SOURCE/FUNCTION/CastlePWD.h"
#include "../SOURCE/FUNCTION/Utility.h"
#include "../SOURCE/FUNCTION/APDU.h"
#include "../SOURCE/FUNCTION/TMSIPDT.h"
#include "../SOURCE/FUNCTION/XML.h"
#include "../SOURCE/EVENT/MenuMsg.h"
#include "../SOURCE/EVENT/Flow.h"
#include "../SOURCE/EVENT/Menu.h"
#include "../HG/HGsrc.h"
#include "../HG/HGiso.h"
#include "../CREDIT/Creditfunc.h"
#include "../CREDIT/CreditprtByBuffer.h"
#include "../FISC/NCCCfisc.h"
#include "../EMVSRC/EMVsrc.h"
#include "../CTLS/CTLS.h"
#include "NCCCsrc.h"
#include "NCCCats.h"
#include "NCCCmfes.h"
#include "NCCCtmk.h"
#include "NCCCdcc.h"
#include "NCCCesc.h"
#include "NCCCloyalty.h"
#include "NCCCTicketSrc.h"
#include "NCCCtmsCPT.h"
#include "NCCCtmsFTPFLT.h"
#include "NCCCtmsFTP.h"
#include "NCCCtms.h"
#include "NCCCtSAM.h"
#include "TAKAsrc.h"
#include "TAKAiso.h"
#include "NCCCEWsrc.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern  int		ginDebug;		/* Debug使用 extern */
extern  int		ginISODebug;		/* Debug使用 extern */
extern	int		ginDCCHostIndex;
extern	int		ginMachineType;
extern	int		ginAPVersionType;
extern	int		ginContactlessSupport;
extern	int		ginTouchEnable;
extern	char		gszTermVersionID[16 + 1];
extern	long		glnESC_PokaYoke_TestAmt;		/* 測試ESC防呆機制使用金額 */
extern	unsigned long	gulDemoRedemptionPointsBalance;
extern	unsigned char	gusztSAMKeyIndex;
extern	unsigned char	gusztSAMCheckSum_59[4 + 1];
extern	int		ginSpecialSituation;
extern  int		ginTouch_Handle;
extern	char		gszReprintDBPath[100 + 1];
extern	SQLITE_TAG_TABLE TABLE_REPRINT_TITLE_TAG[];
extern	int		ginHGHostIndex;
extern	FPG_FTC_REC	gsrFPG_FTC_Rec;
extern	unsigned char	guszCus123UnlockBit;
extern	unsigned char	guszNoChooseHostBit;
extern	int		ginHalfLCD;
extern	int		ginTAKAHostIndex;
extern	int		ginTAKACommIndex;
extern	DEMO_EW_REC	gsrDEMO_EWRec[6];
extern	char		gszTranDBPath[100 + 1];
extern	char		gszParamDBPath[100 + 1];
extern	ECR_TABLE	gsrECROb;
extern	unsigned long	gulPCIRebootTime;

/*
Function        :inNCCC_Func_Check_Is_Ticket_Purchase
Date&Time       :2025/2/20 下午 4:06
Describe        :
*/
int inNCCC_Func_Check_Is_Ticket_Purchase(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
	char	szKey = 0;
        char    szCustomerIndicator[3 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Func_Check_Ticket_Purchase() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_Ticket_Purchase START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
        if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_126_MASTERCARD_FLIGHT_TICKET_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inNCCC_Func_Check_Ticket_Purchase(Not Cus-126) END !");
                        inLogPrintf(AT, "----------------------------------------");
                }
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_Ticket_Purchase Not Cus-126 END!");
                return (VS_SUCCESS);
        }
        
        if (pobTran->uszECRBit == VS_TRUE)
	{
                if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
                {
                        if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Func_Check_Is_Ticket_Purchase(Inquiry1st) END !");
				inLogPrintf(AT, "----------------------------------------");
			}
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_Is_Ticket_Purchase (Inquiry1st) END!");
                        
                        return (VS_SUCCESS);
                }
                else
                {
                        if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_ ||
                            pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        inLogPrintf(AT, "inNCCC_Func_Check_Is_Ticket_Purchase(%c) END !", pobTran->srBRec.szFlightTicketTransBit[0]);
                                        inLogPrintf(AT, "----------------------------------------");
                                }
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_Is_Ticket_Purchase (%c) END!");

                                return (VS_SUCCESS);
                        }
                        
                        if (inNCCC_Func_Flight_Ticket_ECR_Not_Allow(pobTran, &gsrECROb) == VS_TRUE)
                        {
                                /* 這些交易別跳過機票輸入 */
                                if (ginDebug == VS_TRUE)
                                {
                                        inLogPrintf(AT, "inNCCC_Func_Check_Is_Ticket_Purchase(skip input) END !");
                                        inLogPrintf(AT, "----------------------------------------");
                                }
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_Is_Ticket_Purchase (skip input) END!");

                                return (VS_SUCCESS);
                        }
                }
	}
        
        /* 不接受分期、紅利、smartpay、銀聯、電票 */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE	||
	    pobTran->srBRec.uszRedeemBit == VS_TRUE		||
	    pobTran->srBRec.uszCUPTransBit == VS_TRUE		||
	    pobTran->srBRec.uszFiscTransBit == VS_TRUE          ||
            pobTran->srTRec.uszESVCTransBit == VS_TRUE          ||
            pobTran->srBRec.uszUnyTransBit == VS_TRUE           ||
            pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Check_Is_Ticket_Purchase(Not support inCode) END !", pobTran->srBRec.inCode);
			inLogPrintf(AT, "----------------------------------------");
		}
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_Is_Ticket_Purchase (Not support inCode(%d)) END!", pobTran->srBRec.inCode);
		return (VS_SUCCESS);
	}
        
        /* Is this a ticket purchase? */
	/* yes, press 1. */
        /* No, press 2. */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_ChineseFont("Is this a ticket purchase?", _FONTSIZE_8X33_, _LINE_8_4_, _DISP_LEFT_);
        inDISP_ChineseFont("yes, press 1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
        inDISP_ChineseFont("No, press 2.", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
        inDISP_ChineseFont("是否為機票交易?", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
        inDISP_ChineseFont("是按[1]，否按[2]", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
        
        do
        {
                szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
                if (szKey == _KEY_1_)
                {
                        pobTran->srBRec.szFlightTicketTransBit[0] = _FLIGHT_TICKET_TRANS_YES_;
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_2_)
                {
                        pobTran->srBRec.szFlightTicketTransBit[0] = _FLIGHT_TICKET_TRANS_NOT_;
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
        }while(1);
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Func_Check_Ticket_Purchase() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Check_Ticket_Purchase END!");
        
	return (inRetVal);
}

/*
Function        :inNCCC_Func_Get_PDS0523
Date&Time       :2025/2/21 下午 3:42
Describe        :取得出發地機場代碼
*/
int inNCCC_Func_Get_PDS0523(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
	char	szKey = 0;
        char    szCustomerIndicator[3 + 1] = {0};
        char    szDispMsg[100 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0523() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523 START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
        if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_126_MASTERCARD_FLIGHT_TICKET_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inNCCC_Func_Get_PDS0523(Not Cus-126) END !");
                        inLogPrintf(AT, "----------------------------------------");
                }
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523 Not Cus-126 END!");
                return (VS_SUCCESS);
        }
	
	if (pobTran->uszECRBit == VS_TRUE)
	{
                if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
                {
                        if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Func_Get_PDS0523(Inquiry1st) END !");
				inLogPrintf(AT, "----------------------------------------");
			}
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523 (Inquiry1st) END!");
                        
                        return (VS_SUCCESS);
                }
                else
                {
                        if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_ ||
                            pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
                        {
                                if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_PDS0523_SOUVENIR_, 3))
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_Func_Get_PDS0523(SOUVENIR 777) END !");
                                                inLogPrintf(AT, "----------------------------------------");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523 (SOUVENIR) END!");

                                        return (VS_SUCCESS);
                                }
                                else if ((inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0523, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0523) == 3) ||
                                         (inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0523, 4, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0523) == 4) ||
                                         (inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0523, 5, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0523) == 5))
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_Func_Get_PDS0523(ECR filled) END !");
                                                inLogPrintf(AT, "----------------------------------------");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523 (ECR filled) END!");
                                        return (VS_SUCCESS);
                                }
                                else
                                {

                                }
                        }
                        else
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        inLogPrintf(AT, "inNCCC_Func_Get_PDS0523(Not Flight-ticket-trans) END !");
                                        inLogPrintf(AT, "----------------------------------------");
                                }
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523 (Not Flight-ticket-trans) END!");
                                return (VS_SUCCESS);
                        }
                }
	}
	else
	{
		/* 單機非機票交易要跳走 */
		if (pobTran->srBRec.szFlightTicketTransBit[0] != _FLIGHT_TICKET_TRANS_YES_)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Func_Get_PDS0523(Not Flight-ticket-trans) END !");
				inLogPrintf(AT, "----------------------------------------");
			}
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523 (Not Flight-ticket-trans) END!");
			return (VS_SUCCESS);
		}
	}
	
	/* 不接受分期、紅利、smartpay、銀聯 */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE	||
	    pobTran->srBRec.uszRedeemBit == VS_TRUE		||
	    pobTran->srBRec.uszCUPTransBit == VS_TRUE		||
	    pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Get_PDS0523(Not support inCode) END !", pobTran->srBRec.inCode);
			inLogPrintf(AT, "----------------------------------------");
		}
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523 (Not support inCode(%d)) END!", pobTran->srBRec.inCode);
		return (VS_SUCCESS);
	}
        
        /* Enter departure? */
	/* 請輸入出發地機場代碼 */
        /* 1.TPE 2.TSA 3.KHH */
	/* 4.RMQ 5.TNN 6.HUN */
	/* 7.TTT 8.MZG 9.KNH */
	/* 0.Others */
	/* None above, press 0 */
	/* 以上皆非，請按[0] */
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_ChineseFont("Enter departure?", _FONTSIZE_16X22_, _LINE_16_7_, _DISP_LEFT_);
	inDISP_ChineseFont("請輸入出發地機場代碼", _FONTSIZE_16X33_, _LINE_16_8_, _DISP_LEFT_);
        memset(szDispMsg, 0x00, sizeof(szDispMsg));
        sprintf(szDispMsg, "%d.%s %d.%s %d.%s", 1, _FLIGHT_TICKET_PDS0523_1_, 2, _FLIGHT_TICKET_PDS0523_2_, 3, _FLIGHT_TICKET_PDS0523_3_);
        inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_9_, _DISP_LEFT_);
        memset(szDispMsg, 0x00, sizeof(szDispMsg));
        sprintf(szDispMsg, "%d.%s %d.%s %d.%s", 4, _FLIGHT_TICKET_PDS0523_4_, 5, _FLIGHT_TICKET_PDS0523_5_, 6, _FLIGHT_TICKET_PDS0523_6_);
        inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_10_, _DISP_LEFT_);
        memset(szDispMsg, 0x00, sizeof(szDispMsg));
        sprintf(szDispMsg, "%d.%s %d.%s %d.%s", 7, _FLIGHT_TICKET_PDS0523_7_, 8, _FLIGHT_TICKET_PDS0523_8_, 9, _FLIGHT_TICKET_PDS0523_9_);
        inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_11_, _DISP_LEFT_);
        inDISP_ChineseFont("0.Others", _FONTSIZE_16X22_, _LINE_16_12_, _DISP_LEFT_);
	inDISP_ChineseFont("None above, press 0.", _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
	inDISP_ChineseFont("以上皆非，請按[0]", _FONTSIZE_16X22_, _LINE_16_15_, _DISP_LEFT_);
        
        do
        {
                szKey = uszKBD_GetKey(60);
                if (szKey == _KEY_1_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_1_);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_2_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_2_);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_3_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_3_);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_4_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_4_);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_5_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_5_);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_6_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_6_);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_7_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_7_);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_8_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_8_);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                 else if (szKey == _KEY_9_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_9_);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_0_)
                {
                        /* 這邊塞key-in UI */
			inRetVal = inNCCC_Func_Get_PDS0523_Other(pobTran);
			inFunc_ResetTitle(pobTran);
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
        }while(1);
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0523() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523 END!");
        
	return (inRetVal);
}

/*
Function        :inNCCC_Func_Get_PDS0523_Other
Date&Time       :2025/2/21 下午 3:42
Describe        :取得出發地機場代碼
*/
int inNCCC_Func_Get_PDS0523_Other(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
	char	szDebugMsg[100 + 1] = {0};
	DISPLAY_OBJECT	srDispObj = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0523_Other() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523_Other START!");
        
        while (1)
        {
                inDISP_ClearAll();
		if (pobTran->srBRec.inCode == _REFUND_		||
		    pobTran->srBRec.inCode == _INST_REFUND_	||
		    pobTran->srBRec.inCode == _REDEEM_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_REFUND_)
		{
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_X_16_1_);
		}
		else
		{
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_X_16_1_);
		}
		inDISP_ChineseFont_Color("Enter departure?", _FONTSIZE_8X33_, _LINE_8_2_, _COLOR_BLACK_, _DISP_RIGHT_);
		inDISP_ChineseFont_Color("請輸入出發地機場代碼(IATA編碼)", _FONTSIZE_8X34_, _LINE_8_3_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("A", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("B", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("C", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("D", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("E", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("F", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_6_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_6_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("G", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_7_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_7_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("H", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_8_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_8_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("I", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("J", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_10_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_10_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("K", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_11_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_11_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("L", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_12_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_12_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("M", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("N", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_14_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_14_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("O", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_15_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_15_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("P", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_16_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_16_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("Q", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("R", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_18_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_18_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("S", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_19_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_19_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("T", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_20_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_20_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("U", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("V", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_22_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_22_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("W", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_23_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_23_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("X", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_24_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_24_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("倒退", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_Xm_ - 20, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("Y", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_26_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_26_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("Z", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_27_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_27_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("OK!", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_28_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_28_BUTTON_Y1_, VS_FALSE);
                
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Y1_, 1, (_LCD_YSIZE_ - _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Y1_));
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Y1_, 1, (_LCD_YSIZE_ - _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Y1_));
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Y1_, 1, (_LCD_YSIZE_ - _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Y1_));
		
                srDispObj.inY = _LINE_8_2_;
                srDispObj.inR_L = _DISP_LEFT_;
                srDispObj.inMaxLen = 5;
                srDispObj.inMask = VS_FALSE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTouchSensorFunc = _Touch_KEY_IN_MENU_LINE_3_TO_8_4X7_;
                srDispObj.inTimeout = 60;       /* 經業務單位回覆，請調整Time out設置60秒 */
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask_And_DisTouch_For_Flight_ticket(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                {
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_;
                        return (VS_ERROR);
                }
                else if (inRetVal > 0)
                {
			if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_)
			{
				if ((inFunc_Check_Validation(srDispObj.szOutput, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 3)	||
				    (inFunc_Check_Validation(srDispObj.szOutput, 4, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 4)	||
				    (inFunc_Check_Validation(srDispObj.szOutput, 5, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 5))
				{
					memcpy(pobTran->srBRec.szFlightTicketPDS0523, srDispObj.szOutput, 5);
                                        inRetVal = VS_SUCCESS;
                                        break;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0523", "No illegal", srDispObj.szOutput);
						inLogPrintf(AT, szDebugMsg);
					}
					continue;
				}
			}
			else if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
			{
				if ((inFunc_Check_Validation(srDispObj.szOutput, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 3)	 ||
				    (inFunc_Check_Validation(srDispObj.szOutput, 4, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 4)	 ||
				    (inFunc_Check_Validation(srDispObj.szOutput, 5, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 5))
				{
					memcpy(pobTran->srBRec.szFlightTicketPDS0523, srDispObj.szOutput, 5);
                                        inRetVal = VS_SUCCESS;
                                        break;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0523", "No illegal", srDispObj.szOutput);
						inLogPrintf(AT, szDebugMsg);
					}
					continue;
				}
			}
                }
		else if (inRetVal == 0)
                {
			continue;
		}

                break;
        }
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0523_Other() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0523_Other END!");
        
	return (inRetVal);
}

/*
Function        :inNCCC_Func_Get_PDS0524
Date&Time       :2025/2/26 下午 3:55
Describe        :取得出發地機場代碼
*/
int inNCCC_Func_Get_PDS0524(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
        char    szCustomerIndicator[3 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0524() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
        if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_126_MASTERCARD_FLIGHT_TICKET_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inNCCC_Func_Get_PDS0524(Not Cus-126) END !");
                        inLogPrintf(AT, "----------------------------------------");
                }
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 Not Cus-126 END!");
                return (VS_SUCCESS);
        }
        
	if (pobTran->uszECRBit == VS_TRUE)
	{
                if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
                {
                        if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Func_Get_PDS0524(Inquiry1st) END !");
				inLogPrintf(AT, "----------------------------------------");
			}
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 (Inquiry1st) END!");
                        
                        return (VS_SUCCESS);
                }
                else
                {
                        if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_ ||
                            pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
                        {
                                if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_PDS0523_SOUVENIR_, 3))
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_Func_Get_PDS0524(SOUVENIR 777) END !");
                                                inLogPrintf(AT, "----------------------------------------");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 (SOUVENIR) END!");

                                        return (VS_SUCCESS);
                                }
                                else if ((inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0524, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0524) == 3) ||
                                         (inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0524, 4, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0524) == 4) ||
                                         (inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0524, 5, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0524) == 5))
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_Func_Get_PDS0524(ECR filled) END !");
                                                inLogPrintf(AT, "----------------------------------------");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 (ECR filled) END!");
                                        return (VS_SUCCESS);
                                }
                                else
                                {

                                }
                        }
                        else
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        inLogPrintf(AT, "inNCCC_Func_Get_PDS0524(Not Flight-ticket-trans) END !");
                                        inLogPrintf(AT, "----------------------------------------");
                                }
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 (Not Flight-ticket-trans) END!");
                                return (VS_SUCCESS);
                        }
                }
	}
	else
	{
		/* 單機非機票交易要跳走 */
		if (pobTran->srBRec.szFlightTicketTransBit[0] != _FLIGHT_TICKET_TRANS_YES_)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Func_Get_PDS0524(Not Flight-ticket-trans) END !");
				inLogPrintf(AT, "----------------------------------------");
			}
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 (Not Flight-ticket-trans) END!");
			return (VS_SUCCESS);
		}
	}
	
	/* 不接受分期、紅利、smartpay、銀聯 */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE	||
	    pobTran->srBRec.uszRedeemBit == VS_TRUE		||
	    pobTran->srBRec.uszCUPTransBit == VS_TRUE		||
	    pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Get_PDS0524(Not support inCode) END !", pobTran->srBRec.inCode);
			inLogPrintf(AT, "----------------------------------------");
		}
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 (Not support inCode(%d)) END!", pobTran->srBRec.inCode);
		return (VS_SUCCESS);
	}

	/* 這邊塞key-in UI */
	inRetVal = inNCCC_Func_Get_PDS0524_Other(pobTran);
	inFunc_ResetTitle(pobTran);

        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0524() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 END!");
        
	return (inRetVal);
}

/*
Function        :inNCCC_Func_Get_PDS0524_Other
Date&Time       :2025/2/21 下午 3:42
Describe        :取得出發地機場代碼
*/
int inNCCC_Func_Get_PDS0524_Other(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
	char	szDebugMsg[100 + 1] = {0};
	DISPLAY_OBJECT	srDispObj = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0524_Other() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524_Other START!");
        
        while (1)
        {
                inDISP_ClearAll();
		if (pobTran->srBRec.inCode == _REFUND_		||
		    pobTran->srBRec.inCode == _INST_REFUND_	||
		    pobTran->srBRec.inCode == _REDEEM_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_REFUND_)
		{
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_X_16_1_);
		}
		else
		{
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_X_16_1_);
		}
		inDISP_ChineseFont_Color("Enter arrival?", _FONTSIZE_8X33_, _LINE_8_2_, _COLOR_BLACK_, _DISP_RIGHT_);
		inDISP_ChineseFont_Color("請輸入目的地機場代碼(IATA編碼)", _FONTSIZE_8X34_, _LINE_8_3_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("A", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("B", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("C", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("D", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("E", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("F", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_6_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_6_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("G", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_7_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_7_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("H", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_8_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_8_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("I", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("J", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_10_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_10_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("K", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_11_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_11_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("L", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_12_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_12_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("M", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("N", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_14_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_14_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("O", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_15_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_15_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("P", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_16_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_16_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("Q", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("R", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_18_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_18_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("S", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_19_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_19_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("T", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_20_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_20_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("U", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("V", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_22_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_22_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("W", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_23_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_23_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("X", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_24_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_24_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("倒退", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_Xm_ - 20, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("Y", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_26_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_26_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("Z", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_27_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_27_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("OK!", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_28_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_28_BUTTON_Y1_, VS_FALSE);
                
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Y1_, 1, (_LCD_YSIZE_ - _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Y1_));
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Y1_, 1, (_LCD_YSIZE_ - _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Y1_));
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Y1_, 1, (_LCD_YSIZE_ - _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Y1_));
		
                srDispObj.inY = _LINE_8_2_;
                srDispObj.inR_L = _DISP_LEFT_;
                srDispObj.inMaxLen = 5;
                srDispObj.inMask = VS_FALSE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTouchSensorFunc = _Touch_KEY_IN_MENU_LINE_3_TO_8_4X7_;
                srDispObj.inTimeout = 60;       /* 經業務單位回覆，請調整Time out設置60秒 */
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask_And_DisTouch_For_Flight_ticket(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                {
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_;
                        return (VS_ERROR);
                }
                else if (inRetVal > 0)
                {
			if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_)
			{
				if ((inFunc_Check_Validation(srDispObj.szOutput, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 3) ||
				    (inFunc_Check_Validation(srDispObj.szOutput, 4, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 4) ||
				    (inFunc_Check_Validation(srDispObj.szOutput, 5, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 5))
				{
					memcpy(pobTran->srBRec.szFlightTicketPDS0524, srDispObj.szOutput, 5);
                                        inRetVal = VS_SUCCESS;
                                        break;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0524", "No illegal", srDispObj.szOutput);
						inLogPrintf(AT, szDebugMsg);
					}
					continue;
				}
			}
			else if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
			{
				if ((inFunc_Check_Validation(srDispObj.szOutput, 3, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 3)  ||
				    (inFunc_Check_Validation(srDispObj.szOutput, 4, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 4)  ||
				    (inFunc_Check_Validation(srDispObj.szOutput, 5, _CHECK_VALIDATION_MODE_1_) == VS_TRUE && strlen(srDispObj.szOutput) == 5))
				{
					memcpy(pobTran->srBRec.szFlightTicketPDS0524, srDispObj.szOutput, 5);
                                        inRetVal = VS_SUCCESS;
                                        break;
				}
				else
				{
					if (ginDebug == VS_TRUE)
					{
						memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
						sprintf(szDebugMsg, "%s: %s  %s", "szFlightTicketPDS0524", "No illegal", srDispObj.szOutput);
						inLogPrintf(AT, szDebugMsg);
					}
					continue;
				}
			}
                }
		else if (inRetVal == 0)
                {
			continue;
		}

                break;
        }
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0524_Other() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524_Other END!");
        
	return (inRetVal);
}

/*
Function        :inNCCC_Func_Get_Txn_Category
Date&Time       :2025/2/26 下午 5:13
Describe        :請輸入交易類型
*/
int inNCCC_Func_Get_Txn_Category(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
	char	szKey = 0;
        char    szCustomerIndicator[3 + 1] = {0};
	char    szDispMsg[100 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Func_Get_Txn_Category() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_Txn_Category START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
        if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_126_MASTERCARD_FLIGHT_TICKET_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inNCCC_Func_Get_Txn_Category(Not Cus-126) END !");
                        inLogPrintf(AT, "----------------------------------------");
                }
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_Txn_Category Not Cus-126 END!");
                return (VS_SUCCESS);
        }
        
        if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inNCCC_Func_Get_Txn_Category(Inquiry1st) END !");
                        inLogPrintf(AT, "----------------------------------------");
                }
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_Txn_Category (Inquiry1st) END!");

                return (VS_SUCCESS);
        }
        
        if (pobTran->srBRec.szFlightTicketTransBit[0] != _FLIGHT_TICKET_TRANS_NOT_)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inNCCC_Func_Get_Txn_Category(Is Flight-ticket-trans) END !");
                        inLogPrintf(AT, "----------------------------------------");
                }
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_Txn_Category (Is Flight-ticket-trans) END!");
                return (VS_SUCCESS);
        }
        else
        {
                if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_PDS0523_SOUVENIR_, 3)		||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_UPGRADE_, 3)	||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_CLUB_FEE_, 3)	||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_CARGO_, 3)		||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_DUTY_FREE_, 3)	||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_BAGGAGE_CHARGE_, 3)	||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_PREPAID_TICKET_, 3)	||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_DELIVERY_, 3)	||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_PET_CARRIER_, 3)	||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_TICKET_REISSUE_, 3)	||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_TOUR_ORDER_, 3)	||
                    !memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_OTHERS_, 3))
                {
                        /* 已選定交易類型 */
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "inNCCC_Func_Get_Txn_Category(ECR filled) END !");
                                inLogPrintf(AT, "----------------------------------------");
                        }
                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_Txn_Category (ECR filled) END!");
                        return (VS_SUCCESS);
                }
        }

        do
        {
		/* Enter transaction category? */
		/* 請輸入交易類型 */
		/* 1.Upgrade 2.Club fee */
		/* 3.Cargo 4.Duty free */
		/* 5.Baggage charge */
		/* 6.Prepaid ticket */
		/* 7.Delivery */
		/* 8.Pet carrier */
		/* 請按確認鍵到下一頁 */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Enter transaction category?", _FONTSIZE_16X33_, _LINE_16_7_, _DISP_LEFT_);
		inDISP_ChineseFont("請輸入交易類型", _FONTSIZE_16X22_, _LINE_16_8_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 1, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_UPGRADE_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_9_, _DISP_LEFT_);
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 2, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_CLUB_FEE_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_9_, VS_FALSE);
                /* 因r字距過寬需獨立處理 */
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 3, "Car");
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, 0, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%s", "go");
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, 66, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s",4, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_DUTY_FREE_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _COORDINATE_X_RECEPT_CENTER_, _COORDINATE_Y_LINE_16_10_, VS_FALSE);
                
                /* 因r字距過寬需獨立處理 */
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 5, "Baggage char");
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, 0, _COORDINATE_Y_LINE_16_11_, VS_FALSE);
                memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%s", "ge");
                inDISP_ChineseFont_Point_Color_By_Graphic_Mode(szDispMsg, _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, 192, _COORDINATE_Y_LINE_16_11_, VS_FALSE);
                
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 6, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_PREPAID_TICKET_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_12_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 7, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_DELIVERY_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 8, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_PET_CARRIER_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_14_, _DISP_LEFT_);
		inDISP_ChineseFont("請按確認鍵到下一頁", _FONTSIZE_16X22_, _LINE_16_15_, _DISP_LEFT_);
	
                szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
                if (szKey == _KEY_1_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_UPGRADE_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_2_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_CLUB_FEE_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_3_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_CARGO_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_4_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_DUTY_FREE_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_5_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_BAGGAGE_CHARGE_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_6_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_PREPAID_TICKET_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_7_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_DELIVERY_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_8_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_PET_CARRIER_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_ENTER_)
                {
                        /* 這邊塞另一頁 UI */
			inRetVal = inNCCC_Func_Get_TXN_CATEGORY_Other_Page(pobTran);
			if (inRetVal == VS_NEXT_PAGE)
			{
				continue;
			}
			else if (inRetVal == VS_SUCCESS)
			{
				break;
			}
			else
			{
				break;
			}
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
        }while(1);

        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Func_Get_Txn_Category() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_Txn_Category END!");
        
	return (inRetVal);
}

/*
Function        :inNCCC_Func_Get_TXN_CATEGORY_Other_Page
Date&Time       :2025/2/26 下午 5:13
Describe        :請輸入交易類型
*/
int inNCCC_Func_Get_TXN_CATEGORY_Other_Page(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
	char	szKey = 0;
        char    szCustomerIndicator[3 + 1] = {0};
	char    szDispMsg[100 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Func_Get_TXN_CATEGORY_Other_Page() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0524 START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
        if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_126_MASTERCARD_FLIGHT_TICKET_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inNCCC_Func_Get_TXN_CATEGORY_Other_Page(Not Cus-126) END !");
                        inLogPrintf(AT, "----------------------------------------");
                }
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_TXN_CATEGORY_Other_Page Not Cus-126 END!");
                return (VS_SUCCESS);
        }
        else if (pobTran->srBRec.szFlightTicketTransBit[0] != _FLIGHT_TICKET_TRANS_NOT_)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "inNCCC_Func_Get_TXN_CATEGORY_Other_Page(Is Flight-ticket-trans) END !");
                        inLogPrintf(AT, "----------------------------------------");
                }
                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_TXN_CATEGORY_Other_Page (Is Flight-ticket-trans) END!");
                return (VS_SUCCESS);
        }
        
        do
        {
		/* Enter transaction category? */
		/* 請輸入交易類型 */
		/* 1.Ticket reissue */
		/* 2.Tour order */
		/* 3.Others */
		/* 4.Souvenir */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Enter transaction category?", _FONTSIZE_16X33_, _LINE_16_7_, _DISP_LEFT_);
		inDISP_ChineseFont("請輸入交易類型", _FONTSIZE_16X22_, _LINE_16_8_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 1, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_TICKET_REISSUE_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_9_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 2, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_TOUR_ORDER_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_10_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 3, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_MISCELLANEOUS_CHARGE_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_11_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		sprintf(szDispMsg, "%d.%s", 4, _FLIGHT_TICKET_TXN_CATEGORY_DISPLAY_SOUVENIR_);
		inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X22_, _LINE_16_12_, _DISP_LEFT_);
		inDISP_ChineseFont("請按確認鍵到上一頁", _FONTSIZE_16X22_, _LINE_16_13_, _DISP_LEFT_);
	
                szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
                if (szKey == _KEY_1_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_TICKET_REISSUE_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_2_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_TOUR_ORDER_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_3_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_TXN_CATEGORY_OTHERS_);
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_4_)
                {
                        sprintf(&pobTran->srBRec.szFlightTicketPDS0523[0], "%s", _FLIGHT_TICKET_PDS0523_SOUVENIR_);
			sprintf(&pobTran->srBRec.szFlightTicketPDS0524[0], "%s", pobTran->srBRec.szFlightTicketPDS0523);
                        inRetVal = VS_SUCCESS;
                        break;
                }
                else if (szKey == _KEY_ENTER_)
                {
                        /* 這邊塞另一頁 UI */
			inRetVal = VS_NEXT_PAGE;
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
        }while(1);

        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Func_Get_TXN_CATEGORY_Other_Page() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_TXN_CATEGORY_Other_Page END!");
        
	return (inRetVal);
}

/*
Function        :inNCCC_Func_Get_PDS0530
Date&Time       :2025/2/27 下午 3:02
Describe        :取得航班號碼
*/
int inNCCC_Func_Get_PDS0530(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
        char    szCustomerIndicator[3 + 1] = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "3X4inNCCC_Func_Get_PDS0530() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "3X4inNCCC_Func_Get_PDS0530 START!");
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
        inGetCustomIndicator(szCustomerIndicator);
        if (memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_126_MASTERCARD_FLIGHT_TICKET_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "3X4inNCCC_Func_Get_PDS0530(Not Cus-126) END !");
                        inLogPrintf(AT, "----------------------------------------");
                }
                vdUtility_SYSFIN_LogMessage(AT, "3X4inNCCC_Func_Get_PDS0530 Not Cus-126 END!");
                return (VS_SUCCESS);
        }
	
	if (pobTran->uszECRBit == VS_TRUE)
	{
                if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "inNCCC_Func_Get_PDS0530(Inquiry1st) END !");
                                inLogPrintf(AT, "----------------------------------------");
                        }
                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0530 (Inquiry1st) END!");

                        return (VS_SUCCESS);
                }
                else
                {
                        if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_ ||
                            pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)
                        {
                                if ((inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0530, 1, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0530) == 1) ||
				    (inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0530, 2, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0530) == 2) ||
				    (inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0530, 3, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0530) == 3) ||
                                    (inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0530, 4, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0530) == 4) ||
                                    (inFunc_Check_Validation(pobTran->srBRec.szFlightTicketPDS0530, 5, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(pobTran->srBRec.szFlightTicketPDS0530) == 5))
                                {
                                        if (ginDebug == VS_TRUE)
                                        {
                                                inLogPrintf(AT, "inNCCC_Func_Get_PDS0530(ECR filled) END !");
                                                inLogPrintf(AT, "----------------------------------------");
                                        }
                                        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0530 (ECR filled) END!");
                                        return (VS_SUCCESS);
                                }
                                else
                                {

                                }
                        }
                        else
                        {
                                if (ginDebug == VS_TRUE)
                                {
                                        inLogPrintf(AT, "inNCCC_Func_Get_PDS0530(Not Flight-ticket-trans) END !");
                                        inLogPrintf(AT, "----------------------------------------");
                                }
                                vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0530 (Not Flight-ticket-trans) END!");
                                return (VS_SUCCESS);
                        }
                }
	}
	else
	{
		/* 單機非機票交易要跳走 */
		if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NONE_)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inNCCC_Func_Get_PDS0530(Not Flight-ticket-trans) END !");
				inLogPrintf(AT, "----------------------------------------");
			}
			vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0530 (Not Flight-ticket-trans) END!");
			return (VS_SUCCESS);
		}
	}
	
	/* 不接受分期、紅利、smartpay、銀聯 */
	if (pobTran->srBRec.uszInstallmentBit == VS_TRUE	||
	    pobTran->srBRec.uszRedeemBit == VS_TRUE		||
	    pobTran->srBRec.uszCUPTransBit == VS_TRUE		||
	    pobTran->srBRec.uszFiscTransBit == VS_TRUE)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Get_PDS0530(Not support inCode) END !", pobTran->srBRec.inCode);
			inLogPrintf(AT, "----------------------------------------");
		}
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0530 (Not support inCode(%d)) END!", pobTran->srBRec.inCode);
		return (VS_SUCCESS);
	}
	else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_PDS0523_SOUVENIR_, 3))
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inNCCC_Func_Get_PDS0530(SOUVENIR 777) END !");
			inLogPrintf(AT, "----------------------------------------");
		}
		vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0530 (SOUVENIR) END!");
		return (VS_SUCCESS);
	}

	/* 這邊塞key-in UI */
	inRetVal = inNCCC_Func_Get_PDS0530_Other(pobTran);
	inFunc_ResetTitle(pobTran);

        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "3X4inNCCC_Func_Get_PDS0530() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        vdUtility_SYSFIN_LogMessage(AT, "3X4inNCCC_Func_Get_PDS0530 END!");
        
	return (inRetVal);
}

/*
Function        :inNCCC_Func_Get_PDS0530_Other
Date&Time       :2025/2/21 下午 3:42
Describe        :取得航班號碼
*/
int inNCCC_Func_Get_PDS0530_Other(TRANSACTION_OBJECT *pobTran)
{
	int     inRetVal = VS_ERROR;
	DISPLAY_OBJECT	srDispObj = {0};
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0530_Other() START !");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0530_Other START!");
        
        while (1)
        {
                inDISP_ClearAll();
		if (pobTran->srBRec.inCode == _REFUND_		||
		    pobTran->srBRec.inCode == _INST_REFUND_	||
		    pobTran->srBRec.inCode == _REDEEM_REFUND_	||
		    pobTran->srBRec.inCode == _CUP_REFUND_)
		{
			inDISP_PutGraphic(_MENU_REFUND_TITLE_, 0, _COORDINATE_X_16_1_);
		}
		else
		{
			inDISP_PutGraphic(_MENU_SALE_TITLE_, 0, _COORDINATE_X_16_1_);
		}
		inDISP_ChineseFont_Color("Enter flight number? (Numbers only)", _FONTSIZE_16X44_, _LINE_16_5_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Color("請輸入航班號碼(僅須輸入數字)", _FONTSIZE_16X33_, _LINE_16_6_, _COLOR_BLACK_, _DISP_LEFT_);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("1", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_1_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_1_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("2", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("3", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("4", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_4_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_4_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("5", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_5_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_5_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("6", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_6_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_6_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("7", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_7_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_7_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("8", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_8_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_8_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("9", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_9_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_9_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("倒退", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_10_BUTTON_Xm_ - 20, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_10_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("0", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_11_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_11_BUTTON_Y1_, VS_FALSE);
		inDISP_ChineseFont_Point_Color_By_Graphic_Mode("OK!", _FONTSIZE_16X22_, _COLOR_BLACK_, _COLOR_WHITE_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_12_BUTTON_Xm_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_12_BUTTON_Y1_, VS_FALSE);
                
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_1_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_1_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_4_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_4_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_7_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_7_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_10_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_10_BUTTON_Y1_, _LCD_XSIZE_, 1);
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_Y1_, 1, (_LCD_YSIZE_ - _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_Y1_));
		inDISP_Display_Black_Back(_Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_X1_, _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_Y1_, 1, (_LCD_YSIZE_ - _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_Y1_));
		
                srDispObj.inY = _LINE_8_2_;
                srDispObj.inR_L = _DISP_LEFT_;
                srDispObj.inMaxLen = 5;
                srDispObj.inMask = VS_FALSE;
		srDispObj.inColor = _COLOR_RED_;
		srDispObj.inTouchSensorFunc = _Touch_KEY_IN_MENU_LINE_3_TO_8_3X4_;
		
		memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
		srDispObj.inOutputLen = 0;
	
                inRetVal = inDISP_Enter8x16_Character_Mask_And_DisTouch_For_Flight_ticket(&srDispObj);

                if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
                {
			pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_USER_TERMINATE_ERROR_;
                        return (VS_ERROR);
                }
                else if (inRetVal > 0)
                {
			if ((inFunc_Check_Validation(srDispObj.szOutput, 1, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(srDispObj.szOutput) == 1)  ||
			    (inFunc_Check_Validation(srDispObj.szOutput, 2, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(srDispObj.szOutput) == 2)  ||
			    (inFunc_Check_Validation(srDispObj.szOutput, 3, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(srDispObj.szOutput) == 3)  ||
			    (inFunc_Check_Validation(srDispObj.szOutput, 4, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(srDispObj.szOutput) == 4)  ||
			    (inFunc_Check_Validation(srDispObj.szOutput, 5, _CHECK_VALIDATION_MODE_2_) == VS_TRUE && strlen(srDispObj.szOutput) == 5))
			{
				memset(pobTran->srBRec.szFlightTicketPDS0530, 0x00, sizeof(pobTran->srBRec.szFlightTicketPDS0530));
				memcpy(pobTran->srBRec.szFlightTicketPDS0530, srDispObj.szOutput, srDispObj.inOutputLen);
				inRetVal = VS_SUCCESS;
				break;
			}
			else
			{
				continue;
			}
                }
		else if (inRetVal == 0)
                {
			continue;
		}

                break;
        }
	
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inNCCC_Func_Get_PDS0530_Other() END !");
                inLogPrintf(AT, "----------------------------------------");
        }
        vdUtility_SYSFIN_LogMessage(AT, "inNCCC_Func_Get_PDS0530_Other END!");
        
	return (inRetVal);
}

/*
 Function        :inNCCC_Func_Get_Flight_Ticket_Txn_Category_DisplayName
 Date&Time       :2025/3/3 下午 4:36
 Describe        :
 */
int inNCCC_Func_Get_Flight_Ticket_Txn_Category_PrintName(TRANSACTION_OBJECT *pobTran, char* szBuffer)
{
        if (pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_)
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_PURCHASE_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_UPGRADE_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_UPGRADE_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_UPGRADE_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_CLUB_FEE_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_CLUB_FEE_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_CLUB_FEE_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_CARGO_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_CARGO_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_CARGO_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_DUTY_FREE_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_DUTY_FREE_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_DUTY_FREE_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_BAGGAGE_CHARGE_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_BAGGAGE_CHARGE_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_BAGGAGE_CHARGE_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_PREPAID_TICKET_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_PREPAID_TICKET_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PREPAID_TICKET_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_DELIVERY_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_DELIVERY_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_DELIVERY_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_PET_CARRIER_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_PET_CARRIER_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_PET_CARRIER_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_TICKET_REISSUE_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_TICKET_REISSUE_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TICKET_REISSUE_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_TOUR_ORDER_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_TOUR_ORDER_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_TOUR_ORDER_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_TXN_CATEGORY_OTHERS_, strlen(_FLIGHT_TICKET_TXN_CATEGORY_OTHERS_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_MISCELLANEOUS_CHARGE_);
        }
        else if (!memcmp(pobTran->srBRec.szFlightTicketPDS0523, _FLIGHT_TICKET_PDS0523_SOUVENIR_, strlen(_FLIGHT_TICKET_PDS0523_SOUVENIR_)))
        {
                sprintf(szBuffer, "%s", _FLIGHT_TICKET_TXN_CATEGORY_PRINT_SOUVENIR_);
        }
        else
        {
                memset(szBuffer, 0x00, 1);
        }
	
	return (VS_SUCCESS);
}

/*
 Function        :inNCCC_Func_Flight_Ticket_Allow_Print
 Date&Time       :2025/3/28 下午 4:51
 Describe        :
 */
int inNCCC_Func_Flight_Ticket_Allow_Print(TRANSACTION_OBJECT *pobTran)
{
        if ((pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_YES_ ||
             pobTran->srBRec.szFlightTicketTransBit[0] == _FLIGHT_TICKET_TRANS_NOT_)	&&
             pobTran->srBRec.uszVOIDBit != VS_TRUE					&&
             pobTran->srBRec.uszCUPTransBit != VS_TRUE                                  &&
             pobTran->srBRec.uszDCCTransBit != VS_TRUE)
        {
                return (VS_TRUE);
        }
        else
        {
                return (VS_FALSE);
        }
}

/*
 Function        :inNCCC_Func_Flight_Ticket_ECR_Not_Allow
 Date&Time       :2025/3/28 下午 5:52
 Describe        :
 */
int inNCCC_Func_Flight_Ticket_ECR_Not_Allow(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
        /* 查詢金融代碼不檢核 */
        if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
        {
                return (VS_TRUE);
        }
        /* 手動選擇時卡別時不吃機票交易資訊 */
        /* 可接受手動按hotkey不吃機票交易資訊 */
        else if (gsrECROb.srTransData.szTransTypeIndicator[0] == 'S' ||
                 gsrECROb.srTransData.szField_05[0] == '0')
        {
                return (VS_TRUE);
        }
        /* 掃碼格式相關交易*/
        else if (gsrECROb.srTransData.szECRIndicator[0] == 'Q')
        {
                return (VS_TRUE);
        }
        else
        {
                return (VS_FALSE);
        }
}

/*
 Function        :inNCCC_Func_Flight_Ticket_Not_Allow_Txn_Type
 Date&Time       :2025/3/28 下午 6:14
 Describe        :
 */
int inNCCC_Func_Flight_Ticket_Not_Allow_Txn_Type(TRANSACTION_OBJECT *pobTran)
{
        if (pobTran->srBRec.uszInstallmentBit == VS_TRUE	||
	    pobTran->srBRec.uszRedeemBit == VS_TRUE		||
	    pobTran->srBRec.uszCUPTransBit == VS_TRUE		||
	    pobTran->srBRec.uszFiscTransBit == VS_TRUE          ||
            pobTran->srTRec.uszESVCTransBit == VS_TRUE          ||
            pobTran->srBRec.uszUnyTransBit == VS_TRUE           ||
            pobTran->srBRec.uszCUPEMVQRCodeBit == VS_TRUE)
	{
                return (VS_TRUE);
        }
        else
        {
                return (VS_FALSE);
        }
}

/*
Function        :inNCCC_Func_Didp_Reboot_Time
Date&Time       :2025/4/15 下午 7:03
Describe        :
*/
int inNCCC_Func_Didp_Reboot_Time(void)
{
	char		szMessage[100 + 1] = {0};
	char		szUnixTime[10 + 1] = {0};
	unsigned char	uszKey = 0x00;
	RTC_NEXSYS	srRTC = {};
	
	memset(szUnixTime, 0x00, sizeof(szUnixTime));
	sprintf(szUnixTime, "%lu", gulPCIRebootTime);
	inFuncGetUnixTimeToLocalTime(&srRTC, szUnixTime, 8, 8);
	
	inDISP_ClearAll();
	do
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		memset(szMessage, 0x00, sizeof(szMessage));
		sprintf(szMessage, "20%02u.%02u.%02u %02u:%02u:%02u", srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay, srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond);
		inDISP_ChineseFont(szMessage, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
		uszKey = uszKBD_GetKey(30);
		break;
	}while(1);
	
	return (VS_SUCCESS);
}

/*
 Function        :inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn
 Date&Time       :2025/5/12 下午 2:06
 Describe        :【需求單-113172】TMS系統新增「TMS參數Print_Mode 4=兩聯免簽不印商店聯」，免簽名交易不列印商店存根聯
 *                交易使用，會偵測是否免簽名。

 */
int inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Txn(TRANSACTION_OBJECT *pobTran)
{
        char	szPrtMode[2 + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
        char    szFESMode[2 + 1] = {0};
        
        memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
        memset(szFESMode, 0x00, sizeof(szFESMode));
        inGetNCCCFESMode(szFESMode);
        
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_000_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (!memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, 1) &&
                    (pobTran->srBRec.uszNoSignatureBit == VS_TRUE           ||
		     pobTran->inRunOperationID == _OPERATION_SETTLE_        ||
		     pobTran->inRunOperationID == _OPERATION_TOTAL_REPORT_))
                {
                        /* 重印簽單商店聯比照現行模式列印商店聯，重印不觸發 */
                        if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
                        {
                                return (VS_FALSE);
                        }
                        
                        /* 端末機開啟ECR連線時維持既有規則，現行小額特店主機取完授權後，若回傳收銀機成功時不印商店存根聯；若回傳收銀機失敗時列印出商店存根聯 */
                        if (!memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) && 
                             pobTran->uszECRBit == VS_TRUE)
                        {
                                return (VS_FALSE);
                        }
                  
                        return (VS_TRUE);
                }
        }
        else
        {
                return (VS_FALSE);
        }
	
	 return (VS_FALSE);
}

/*
 Function        :inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Accum
 Date&Time       :2025/5/12 下午 2:06
 Describe        :【需求單-113172】TMS系統新增「TMS參數Print_Mode 4=兩聯免簽不印商店聯」，免簽名交易不列印商店存根聯
 *                用於是否顯示銷售免。

 */
int inNCCC_Func_Check_Print_Mode_4_No_Merchant_For_Accum(TRANSACTION_OBJECT *pobTran)
{
        char	szPrtMode[2 + 1] = {0};
        char	szCustomerIndicator[3 + 1] = {0};
        char    szFESMode[2 + 1] = {0};
        
        memset(szPrtMode, 0x00, sizeof(szPrtMode));
	inGetPrtMode(szPrtMode);
        
        memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
        
        memset(szFESMode, 0x00, sizeof(szFESMode));
        inGetNCCCFESMode(szFESMode);
        
        if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_000_, _CUSTOMER_INDICATOR_SIZE_))
        {
                if (!memcmp(szPrtMode, _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_, 1) &&
                    (pobTran->inRunOperationID == _OPERATION_SETTLE_        ||
		     pobTran->inRunOperationID == _OPERATION_TOTAL_REPORT_  ||
                     pobTran->inRunOperationID == _OPERATION_DETAIL_REPORT_))
                {
                        /* 重印簽單商店聯比照現行模式列印商店聯，重印不觸發 */
                        if (pobTran->inRunOperationID == _OPERATION_REPRINT_)
                        {
                                return (VS_FALSE);
                        }
                        
                        /* 端末機開啟ECR連線時維持既有規則，現行小額特店主機取完授權後，若回傳收銀機成功時不印商店存根聯；若回傳收銀機失敗時列印出商店存根聯 */
                        if (!memcmp(szFESMode, _NCCC_04_MPAS_MODE_, strlen(_NCCC_04_MPAS_MODE_)) && 
                             pobTran->uszECRBit == VS_TRUE)
                        {
                                return (VS_FALSE);
                        }
                  
                        return (VS_TRUE);
                }
        }
        else
        {
                return (VS_FALSE);
        }
	
	 return (VS_FALSE);
}