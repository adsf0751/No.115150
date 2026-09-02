#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <ctos_clapi.h>
#include <emv_cl.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/File.h"
#include "../PRINT/Print.h"
#include "APDU.h"
#include "Utility.h"

extern	int	ginDebug;
extern	int	ginDisplayDebug;

/*
Function        :inAPDU_APDUTransmit
Date&Time       :2018/6/13 上午 10:01
Describe        :下APDU Command
*/
int inAPDU_APDUTransmit(unsigned char uszSlotID, unsigned char* uszSendBuffer, unsigned short usSendLen, unsigned char* uszReceiveBuffer, unsigned short* usReceiveLen)
{
	char		szDebugMsg[1024 + 1] = {0};
	char		szAscii[1024 + 1] = {0};
	unsigned short	usRetVal = 0x00;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inAPDU_APDUTransmit()_START");
        }
        
	/* 記得要先把Buffer的最長長度放到usReceiveLen中，這邊只傳pointer所以要在外層做 */
        
        if (ginDebug == VS_TRUE)
        {
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                inFunc_BCD_to_ASCII(szDebugMsg, uszSendBuffer, usSendLen);
                inLogPrintf(AT, szDebugMsg);
        }
        
        usRetVal = CTOS_SCSendAPDU(uszSlotID, uszSendBuffer, usSendLen, uszReceiveBuffer, usReceiveLen);
        
        if (usRetVal != d_OK)
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "CTOS_SCSendAPDU_ERR : %04X", usRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		
		inUtility_StoreTraceLog_OneStep("CTOS_SCSendAPDU_ERR : %04X", usRetVal);
		
		if (usRetVal == d_SC_NOT_PRESENT)
		{
			return (VS_EMV_CARD_OUT);
		}
                else
		{
			return (usRetVal);
		}
        }
        else
        {
        	if (ginDebug == VS_TRUE)
	        {
                        inLogPrintf(AT, "CTOS_SCSendAPDU_OK");
			
	                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	                sprintf(szDebugMsg, "%02x %02x Length = %d", uszReceiveBuffer[*usReceiveLen - 2], uszReceiveBuffer[*usReceiveLen - 1], *usReceiveLen);
	                inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inFunc_BCD_to_ASCII(szAscii, uszReceiveBuffer, *usReceiveLen);
	                sprintf(szDebugMsg, "Data: %s", szAscii);
	                inLogPrintf(AT, szDebugMsg);
	        }
		
        }
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inAPDU_APDUTransmit()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inAPDU_Send_APDU_User_Slot_Process
Date&Time       :2018/6/13 上午 10:24
Describe        :對大卡槽下命令
*/
int inAPDU_Send_APDU_User_Slot_Process(APDU_COMMAND* srAPDU)
{
	int		inRetVal = VS_ERROR;
	unsigned char	uszSlot = d_SC_USER;
	
	/* 記得要先把Buffer的最長長度放到usReceiveLen中， */
	srAPDU->inRecevLen = sizeof(srAPDU->uszRecevData);
	
	inRetVal = inAPDU_APDUTransmit(uszSlot, srAPDU->uszSendData, (unsigned short)srAPDU->inSendLen, srAPDU->uszRecevData, (unsigned short*)&srAPDU->inRecevLen);
	
	return (inRetVal);
}

/*
Function        :inAPDU_APDUTransmit_CTLS
Date&Time       :2018/6/13 上午 10:48
Describe        :感應使用的APDU Command
*/
int inAPDU_APDUTransmit_CTLS(unsigned char* uszSendBuffer, unsigned short usSendLen, unsigned char* uszReceiveBuffer, unsigned short* usReceiveLen)
{
        int     inRetVal = 0x00;
        char    szDebugMsg[1024 + 1] = {0};
	char	szAscii[1024 + 1] = {0};

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inAPDU_APDUTransmit_CTLS()_START");
        }
	
	/* 記得要先把Buffer的最長長度放到usReceiveLen中，這邊只傳pointer所以要在外層做 */
        
        if (ginDebug == VS_TRUE)
        {
                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                inFunc_BCD_to_ASCII(&szDebugMsg[0], uszSendBuffer, usSendLen);
                inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                sprintf(szDebugMsg, "Len: %d", usSendLen);
                inLogPrintf(AT, szDebugMsg);
        }
        
        inRetVal = CTOS_CLAPDU(uszSendBuffer, usSendLen, uszReceiveBuffer, usReceiveLen);
        
        if (inRetVal != d_OK)
        {
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "CTOS_CLAPDU_ERR : %04X", inRetVal);
                        inLogPrintf(AT, szDebugMsg);
                }
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_CLAPDU_ERR : %04X", inRetVal);
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_16_, VS_TRUE);
		}
		inUtility_StoreTraceLog_OneStep("CTOS_CLAPDU_ERR : %04X", inRetVal);
                
                return (VS_ERROR);
        }
        else
        {
        	if (ginDebug == VS_TRUE)
	        {
                        inLogPrintf(AT, "CTOS_SCSendAPDU_OK");
			
	                memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
	                sprintf(szDebugMsg, "%02x %02x Length = %d", uszReceiveBuffer[*usReceiveLen - 2], uszReceiveBuffer[*usReceiveLen - 1], *usReceiveLen);
	                inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			inFunc_BCD_to_ASCII(szAscii, uszReceiveBuffer, *usReceiveLen);
	                sprintf(szDebugMsg, "Data: %s", szAscii);
	                inLogPrintf(AT, szDebugMsg);
	        }
        }
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inAPDU_APDUTransmit_CTLS()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inAPDU_User_Slot_Send_APDU
Date&Time       :2018/6/13 上午 10:24
Describe        :對大卡槽下命令
*/
int inAPDU_Send_APDU_CTLS_Process(APDU_COMMAND* srAPDU)
{
	int	inRetVal = VS_ERROR;
	
	/* 記得要先把Buffer的最長長度放到usReceiveLen中， */
	srAPDU->inRecevLen = sizeof(srAPDU->uszRecevData);
	
	inRetVal = inAPDU_APDUTransmit_CTLS(srAPDU->uszSendData, (unsigned short)srAPDU->inSendLen, srAPDU->uszRecevData, (unsigned short*)&srAPDU->inRecevLen);
	
	return (inRetVal);
}

/*
Function        :inAPDU_APDUTransmit_Flow
Date&Time       :2018/6/13 下午 1:37
Describe        :下APDU Command
*/
int inAPDU_APDUTransmit_Flow(TRANSACTION_OBJECT *pobTran, APDU_COMMAND *srAPDU_COMMAND)
{
        int     inRetVal = VS_ERROR;

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inAPDU_APDUTransmit_Flow()_START");
        }
        
        /* 感應下APDU不同 另外寫 */
        if (pobTran->srBRec.uszContactlessBit == VS_TRUE)
        {
                inRetVal = inAPDU_Send_APDU_CTLS_Process(srAPDU_COMMAND);
        }
	else
	{
		inRetVal = inAPDU_Send_APDU_User_Slot_Process(srAPDU_COMMAND);
	}
        
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inAPDU_APDUTransmit_Flow()_END");
        }

        return (inRetVal);
}

/*
App Name	: inAPDU_BuildAPDU
App Date&Time	: 2018/6/13 下午 2:03
App Function	: Build APDU Command
Input Param	: *srAPDUData --> APDU Command 結構
Output Param	: 成功 : VS_SUCCESS
		  失敗 : VS_ERROR
*/
int inAPDU_BuildAPDU(APDU_COMMAND *srAPDUData)
{
	int	inCnt = 0;
        char    szDebugMSG[100 + 1] = {0};

        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "inAPDU_BuildAPDU()_START");
        }

	srAPDUData->uszSendData[inCnt ++] = srAPDUData->uszCommandCLAData[0];
	srAPDUData->uszSendData[inCnt ++] = srAPDUData->uszCommandINSData[0];
	srAPDUData->uszSendData[inCnt ++] = srAPDUData->uszCommandP1Data[0];
	srAPDUData->uszSendData[inCnt ++] = srAPDUData->uszCommandP2Data[0];
	if (srAPDUData->inCommandDataLen > 0)
	{
		srAPDUData->uszSendData[inCnt ++] = srAPDUData->inCommandDataLen;
		memcpy(&srAPDUData->uszSendData[inCnt], &srAPDUData->uszCommandData[0], srAPDUData->inCommandDataLen);
		inCnt += srAPDUData->inCommandDataLen;
	}

	/* LE(有的要塞0x00) */
	/* Select AID要塞LE 0x00 */
	if (srAPDUData->uszCommandCLAData[0] == _FISC_SELECT_AID_CLA_COMMAND_	&&
	    srAPDUData->uszCommandINSData[0] == _FISC_SELECT_AID_INS_COMMAND_	&& 
	    srAPDUData->uszCommandP1Data[0] == _FISC_SELECT_AID_P1_COMMAND_	&& 
	    srAPDUData->uszCommandP2Data[0] == _FISC_SELECT_AID_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _FISC_SELECT_AID_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _TICKET_SELECT_AID_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _TICKET_SELECT_AID_INS_COMMAND_	&& 
		 srAPDUData->uszCommandP1Data[0] == _TICKET_SELECT_AID_P1_COMMAND_	&& 
		 srAPDUData->uszCommandP2Data[0] == _TICKET_SELECT_AID_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _TICKET_SELECT_AID_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _FISC_WRITE_RECORD_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _FISC_WRITE_RECORD_INS_COMMAND_	&& 
		 srAPDUData->uszCommandP1Data[0] == _FISC_WRITE_RECORD_P1_COMMAND_	&& 
		 srAPDUData->uszCommandP2Data[0] == _FISC_WRITE_RECORD_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _FISC_WRITE_RECORD_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _FISC_WRITE_RECORD_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _FISC_WRITE_RECORD_INS_COMMAND_	&& 
		 srAPDUData->uszCommandP1Data[0] == _FISC_WRITE_RECORD_P1_COMMAND_	&& 
		 srAPDUData->uszCommandP2Data[0] == _FISC_WRITE_RECORD_P2_COMMAND_CTLS_)
	{
		srAPDUData->uszSendData[inCnt ++] = _FISC_WRITE_RECORD_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _FISC_READ_RECORDS_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _FISC_READ_RECORDS_INS_COMMAND_	&& 
		 srAPDUData->uszCommandP1Data[0] == _FISC_READ_RECORDS_P1_COMMAND_	&& 
		 srAPDUData->uszCommandP2Data[0] == _FISC_READ_RECORDS_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _FISC_READ_RECORDS_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _FISC_WRITE_AUTH_RECORD_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _FISC_WRITE_AUTH_RECORD_INS_COMMAND_	&& 
		 srAPDUData->uszCommandP1Data[0] == _FISC_WRITE_AUTH_RECORD_P1_COMMAND_		&& 
		 srAPDUData->uszCommandP2Data[0] == _FISC_WRITE_AUTH_RECORD_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _FISC_WRITE_AUTH_RECORD_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _FISC_READ_AUTH_RECORD_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _FISC_READ_AUTH_RECORD_INS_COMMAND_	&& 
		 srAPDUData->uszCommandP1Data[0] == _FISC_READ_AUTH_RECORD_P1_COMMAND_		&& 
		 srAPDUData->uszCommandP2Data[0] == _FISC_READ_AUTH_RECORD_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _FISC_READ_AUTH_RECORD_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _NCCC_TMK_KEY_CARD_GET_APPLET_INFO_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _NCCC_TMK_KEY_CARD_GET_APPLET_INFO_INS_COMMAND_	&& 
	         srAPDUData->uszCommandP1Data[0] == _NCCC_TMK_KEY_CARD_GET_APPLET_INFO_P1_COMMAND_	&&
		 srAPDUData->uszCommandP2Data[0] == _NCCC_TMK_KEY_CARD_GET_APPLET_INFO_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _NCCC_TMK_KEY_CARD_GET_APPLET_INFO_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _NCCC_TMK_KEY_CARD_GET_KEY_INFO_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _NCCC_TMK_KEY_CARD_GET_KEY_INFO_INS_COMMAND_	&& 
	         srAPDUData->uszCommandP1Data[0] == _NCCC_TMK_KEY_CARD_GET_KEY_INFO_P1_COMMAND_		&&
		 srAPDUData->uszCommandP2Data[0] == _NCCC_TMK_KEY_CARD_GET_KEY_INFO_SINGLE_KEY_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _NCCC_TMK_KEY_CARD_GET_KEY_INFO_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _NCCC_TMK_KEY_CARD_GET_KEY_INFO_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _NCCC_TMK_KEY_CARD_GET_KEY_INFO_INS_COMMAND_	&& 
	         srAPDUData->uszCommandP1Data[0] == _NCCC_TMK_KEY_CARD_GET_KEY_INFO_P1_COMMAND_		&&
		 srAPDUData->uszCommandP2Data[0] == _NCCC_TMK_KEY_CARD_GET_KEY_INFO_MULTI_KEY_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _NCCC_TMK_KEY_CARD_GET_KEY_INFO_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _NCCC_TMK_KEY_CARD_GEN_RANDOM_NUM_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _NCCC_TMK_KEY_CARD_GEN_RANDOM_NUM_INS_COMMAND_	&& 
	         srAPDUData->uszCommandP1Data[0] == _NCCC_TMK_KEY_CARD_GEN_RANDOM_NUM_P1_COMMAND_	&&
		 srAPDUData->uszCommandP2Data[0] == _NCCC_TMK_KEY_CARD_GEN_RANDOM_NUM_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _NCCC_TMK_KEY_CARD_GEN_RANDOM_NUM_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _NCCC_TMK_KEY_CARD_GET_TMK_CLA_COMMAND_		&&
		 srAPDUData->uszCommandINSData[0] == _NCCC_TMK_KEY_CARD_GET_TMK_INS_COMMAND_		&& 
	         srAPDUData->uszCommandP1Data[0] == _NCCC_TMK_KEY_CARD_GET_TMK_P1_COMMAND_		&&
		 srAPDUData->uszCommandP2Data[0] == _NCCC_TMK_KEY_CARD_GET_TMK_SINGLE_KEY_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _NCCC_TMK_KEY_CARD_GET_TMK_LE_COMMAND_;
	}
	else if (srAPDUData->uszCommandCLAData[0] == _NCCC_TMK_KEY_CARD_GET_TMK_CLA_COMMAND_		&&
		 srAPDUData->uszCommandINSData[0] == _NCCC_TMK_KEY_CARD_GET_TMK_INS_COMMAND_		&& 
	         srAPDUData->uszCommandP1Data[0] == _NCCC_TMK_KEY_CARD_GET_TMK_P1_COMMAND_		&&
		 srAPDUData->uszCommandP2Data[0] == _NCCC_TMK_KEY_CARD_GET_TMK_MULTI_KEY_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _NCCC_TMK_KEY_CARD_GET_TMK_LE_COMMAND_;
		srAPDUData->uszSendData[inCnt ++] = _NCCC_TMK_KEY_CARD_GET_TMK_LE2_COMMAND_;
		srAPDUData->uszSendData[inCnt ++] = _NCCC_TMK_KEY_CARD_GET_TMK_LE3_COMMAND_;
	}
        else if (srAPDUData->uszCommandCLAData[0] == _EMV_READ_RECORDS_CLA_COMMAND_	&&
		 srAPDUData->uszCommandINSData[0] == _EMV_READ_RECORDS_INS_COMMAND_	&& 
		 srAPDUData->uszCommandP1Data[0] == _EMV_READ_RECORDS_01_P1_COMMAND_	&& 
		 srAPDUData->uszCommandP2Data[0] == _EMV_READ_RECORDS_P2_COMMAND_)
	{
		srAPDUData->uszSendData[inCnt ++] = _EMV_READ_RECORDS_LE_COMMAND_;
	}

	srAPDUData->inSendLen = inCnt;

        if (ginDebug == VS_TRUE)
        {
                memset(szDebugMSG, 0x00, sizeof(szDebugMSG));
                sprintf(szDebugMSG, "srAPDUData->inSendLen : [%d]", srAPDUData->inSendLen);
                inLogPrintf(AT, szDebugMSG);
                inLogPrintf(AT, "inAPDU_BuildAPDU()_END");
        }
        
	return (VS_SUCCESS);
}
