#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../DISPLAY/Display.h"

extern int	ginDebug;

/*
Function        :inCastlePWD_Set_PMEnterPassword
Date&Time       :2019/2/19 下午 4:44
Describe        :The length of password should be between 4 to 8 bytes, 
 *		 and format should be in ASCII digit, range from '0' to '9'.
*/
int inCastlePWD_Set_PMEnterPassword(unsigned char* uszPassword, unsigned char uszEnable)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_SetPMEnterPassword(uszPassword, uszEnable);
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
			sprintf(szDebugMsg, "() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inCastlePWD_Set_FunKeyPassword
Date&Time       :2019/2/20 上午 10:28
Describe        :
*/
int inCastlePWD_Set_FunKeyPassword(unsigned char* uszPassword, unsigned char uszEnable)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_SetFunKeyPassword(uszPassword, uszEnable);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_SetFunKeyPassword Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_SetFunKeyPassword () OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}