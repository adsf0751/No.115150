#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../EVENT/MenuMsg.h"
#include "../COMM/Comm.h"
#include "../../CTLS/CTLS.h"
#include "Sqlite.h"
#include "Function.h"
#include "CFGT.h"
#include "HDT.h"
#include "CPT.h"
#include "EDC.h"
#include "HDPT.h"
#include "Utility.h"
#include "PowerManagement.h"

extern int	ginDebug;

/*
Function        :inPWM_PowerAutoModeEnable
Date&Time       :2018/3/14 上午 10:17
Describe        :
*/
int inPWM_PowerAutoModeEnable()
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_PowerAutoModeEnable();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerAutoModeEnable Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerAutoModeEnable() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_PowerAutoModeDisable
Date&Time       :2018/3/14 上午 10:17
Describe        :
*/
int inPWM_PowerAutoModeDisable()
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_PowerAutoModeDisable();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerAutoModeDisable Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerAutoModeDisable() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_PowerAutoMode_Standby_Time_Set
Date&Time       :2018/3/14 上午 10:22
Describe        :unit: second
*/
int inPWM_PowerAutoMode_Standby_Time_Set(unsigned long ulSecond)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_PowerAutoModeTimeToStandby(ulSecond);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerAutoModeTimeToStandby Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerAutoModeTimeToStandby() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_PowerAutoMode_Sleep_Time_Set
Date&Time       :2018/3/14 上午 10:22
Describe        :unit: second
*/
int inPWM_PowerAutoMode_Sleep_Time_Set(unsigned long ulSecond)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_PowerAutoModeTimeToSleep(ulSecond);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerAutoModeTimeToSleep Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerAutoModeTimeToSleep() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_Get_PowerSource
Date&Time       :2018/3/14 下午 5:48
Describe        :
*/
int inPWM_Get_PowerSource(unsigned char* uszSource)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned char	uszTempSource = 0;
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_PowerSource(&uszTempSource);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerSource Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerSource() OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
		*uszSource = uszTempSource;
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_Power_Mode
Date&Time       :2018/3/14 下午 5:51
Describe        :
*/
int inPWM_Power_Mode(unsigned char uszMode)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_PowerMode(uszMode);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerMode Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_PowerMode() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_Sleep_Mode_Timer_Start
Date&Time       :2018/3/15 上午 9:19
Describe        :計時器for進入睡眠模式
*/
int inPWM_Sleep_Mode_Timer_Start(void)
{
	int	inTimeOut = 0;
	char	szTimeOut[4 + 1] = {0};
	
	memset(szTimeOut, 0x00, sizeof(szTimeOut));
	inGetPWMIdleTimeout(szTimeOut);
	inTimeOut = atoi(szTimeOut);
	
	inDISP_Timer_Start(_TIMER_POWER_MANAGEMENT_, inTimeOut);
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_StandBy_Mode_Timer_Start
Date&Time       :2018/3/21 上午 9:33
Describe        :計時器for進入待機模式
*/
int inPWM_StandBy_Mode_Timer_Start(void)
{
	int	inTimeOut = 0;
	char	szTimeOut[4 + 1] = {0};
	
	memset(szTimeOut, 0x00, sizeof(szTimeOut));
	inGetPWMIdleTimeout(szTimeOut);
	inTimeOut = atoi(szTimeOut);
	
	inDISP_Timer_Start(_TIMER_POWER_MANAGEMENT_, inTimeOut);
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_Enter_Sleep_Mode
Date&Time       :2018/3/14 下午 5:34
Describe        :進入睡眠模式前、後要做的事
*/
int inPWM_Enter_Sleep_Mode(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inPWM_Enter_Sleep_Mode START!");
        
	inPWM_Power_Mode(_POWER_MODE_SLEEP_);
	
	/* 電量管理用 */
	inPWM_Sleep_Mode_Timer_Start();
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_Enter_StandBy_Mode
Date&Time       :22018/3/21 上午 9:20
Describe        :進入待機模式前、後要做的事
*/
int inPWM_Enter_StandBy_Mode(TRANSACTION_OBJECT *pobTran)
{
        vdUtility_SYSFIN_LogMessage(AT, "inPWM_Enter_StandBy_Mode START!");
        
	inPWM_Power_Mode(_POWER_MODE_STANDBY_);
	
	/* 電量管理用 */
	inPWM_StandBy_Mode_Timer_Start();
	
	return (VS_SUCCESS);
}

/*
Function        :inPWM_IS_PWM_Enable
Date&Time       :2018/4/12 下午 3:08
Describe        :
*/
int inPWM_IS_PWM_Enable()
{
	char	szFuncEnable[2 + 1] = {0};
	
	inGetPWMEnable(szFuncEnable);
	
	if (memcmp(szFuncEnable, "Y", strlen("Y")) == 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}
