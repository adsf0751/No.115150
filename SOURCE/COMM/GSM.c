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
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/Function.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/HDPT.h"
#include "../COMM/TLS.h"
#include "Comm.h"

extern  int     ginDebug;  /* Debug使用 extern */
extern  int     ginISODebug;

/*
Function        :inGSM_GetSignalQuality
Date&Time       :2018/3/13 下午 2:05
Describe        :
 *		0: -113 dBm or less
		1: -111 dBm
		2~30: -109 dBm ~ -53 dBm, 2 dBm per step
		31: -51 dBm or greater
		99: not known or not detectable
*/
int inGSM_GetSignalQuality(unsigned char* uszQuality)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_GSMSignalQuality(uszQuality);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_GSMSignalQuality() OK");
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Quality: %u", *uszQuality);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (usRetVal == d_GSM_UNKNOWN_MODULE	||
		 usRetVal == d_GSM_NOT_SUPPORT)
	{
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_GSMSignalQuality Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}
