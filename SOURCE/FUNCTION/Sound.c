#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
/** These two files are necessary for calling CTOS API **/
#include <ctosapi.h>
#include <soundplayer.h> 
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../PRINT/Print.h"
#include "../FUNCTION/Function.h"
#include "../EVENT/Event.h"
#include "../EVENT/MenuMsg.h"
#include "File.h"
#include "Sound.h"

extern int	ginDebug;
extern int	ginISODebug;

/*
Function        :inSoundplayer_Play
Date&Time       :2017/7/7 下午 3:20
Describe        :
*/
int inSoundplayer_Play(char *szPath)
{
#ifndef	_LOAD_KEY_AP_
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_SoundPlayerPlay((unsigned char*)szPath);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_SoundPlayerPlay Success");
			inDISP_LogPrintf(szDebugMsg);
		}
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_SoundPlayerPlay Error: 0x%04x", usRetVal);
			inDISP_LogPrintf(szDebugMsg);
		}
	}
#endif
	
	return (VS_SUCCESS);
}

/*
Function        :inSoundPlaer_test
Date&Time       :2017/7/7 下午 3:20
Describe        :
*/
int inSoundPlaer_test(void)
{
#ifndef	_LOAD_KEY_AP_
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned char	uszVolume = 0;
	
	inRetVal = CTOS_SoundPlayerGetVolume((unsigned char*)&uszVolume);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inRetVal : 0x%04x", inRetVal);
		inDISP_LogPrintf(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Volume : %d", uszVolume);
		inDISP_LogPrintf(szDebugMsg);
	}
	
	uszVolume = 5;
	inRetVal = CTOS_SoundPlayerSetVolume(uszVolume);
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "inRetVal : 0x%04x", inRetVal);
		inDISP_LogPrintf(szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Volume : %d", uszVolume);
		inDISP_LogPrintf(szDebugMsg);
	}
	
	inSoundplayer_Play(_TEST_GIRIGIRI_SOUND_);
#endif
	
	return (VS_SUCCESS);
}
