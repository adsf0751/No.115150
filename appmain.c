/**
**	A Template for developing new terminal application
**/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
/** These two files are necessary for calling CTOS API **/
#include <ctosapi.h>
#include <sqlite3.h>
#include "SOURCE/INCLUDES/Define_1.h"
#include "SOURCE/INCLUDES/Define_2.h"
#include "SOURCE/INCLUDES/Transaction.h"
#include "SOURCE/INCLUDES/TransType.h"
#include "SOURCE/INCLUDES/AllStruct.h"
#include "SOURCE/DISPLAY/Display.h"
#include "SOURCE/DISPLAY/DisTouch.h"
#include "SOURCE/PRINT/Print.h"
#include "SOURCE/FUNCTION/Function.h"
#include "SOURCE/EVENT/Event.h"
#include "SOURCE/EVENT/Menu.h"
#include "SOURCE/EVENT/MenuMsg.h"
#include "SOURCE/FUNCTION/Accum.h"
#include "SOURCE/FUNCTION/File.h"
#include "SOURCE/FUNCTION/HDT.h"
#include "SOURCE/FUNCTION/EDC.h"
#include "SOURCE/FUNCTION/CFGT.h"
#include "SOURCE/FUNCTION/PWD.h"
#include "SOURCE/FUNCTION/Card.h"
#include "SOURCE/FUNCTION/ECR.h"
#include "SOURCE/FUNCTION/RS232.h"
#include "SOURCE/FUNCTION/NexsysSDK.h"
#include "SOURCE/FUNCTION/Sqlite.h"
#include "SOURCE/FUNCTION/KMS.h"
#include "SOURCE/FUNCTION/Utility.h"
#include "SOURCE/COMM/Comm.h"
#include "SOURCE/COMM/TLS.h"
#include "FISC/NCCCfisc.h"
#include "EMVSRC/EMVsrc.h"
#include "CTLS/CTLS.h"
#include "NCCC/NCCCats.h"
#include "NCCC/NCCCtmsSCT.h"
#include "NCCC/NCCCtms.h"
#include "NCCC/NCCCtSAM.h"
#include "NCCC/NCCCesc.h"
#include "SOURCE/EVENT/Flow.h"
#include "NCCC/NCCCdcc.h"
#include "ETicket/Ticket.h"
#include "NCCC/NCCCsrc.h"
#include "NCCC/NCCCmfes.h"
#include "SOURCE/FUNCTION/MultiFunc.h"
#include "SOURCE/FUNCTION/FuncTable.h"

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

/* 這裡放library include的注意事項 */
/* MP200不能include 專案不能Link cyassl會和虹堡現有library衝突 */
/* V3UL一定要include bluetooth，不然會出現 look up error */
/* 虹堡的系統用little endian */
/* linux unzip指令 不支援以LZMA方式的zip，建議Deflate方式壓縮 */

/* Debug使用 extern */
/* 以Comport輸出Log */
int     ginDebug = VS_TRUE;
/* 以列印方式列印Log */
int	ginISODebug = VS_FALSE;
/* 以顯示方式顯示Log */
int	ginDisplayDebug = VS_FALSE;
/* 工程師測試功能使用 */
int	ginEngineerDebug = VS_FALSE;
/* TraceLog紀錄功能用 */

/* 紀錄時間使用 */
int	ginFindRunTime = VS_FALSE;
/* 驗測時使用 */
int	ginExamBit = VS_FALSE;
/* 製造特殊情況使用 */
int	ginSpecialSituation = 0;

int			ginRuntimeCnt = 0;
RUNTIME_ALL_RECORD	gsrRuntimeRecord = {};		/* 用來紀錄哪邊花比較多時間用的 */

/* 用來紀錄是哪種機器類型 */
int	ginMachineType = 0;
/* 用來分別是那一個銀行的版本 */
int	ginAPVersionType = 0;
/* 用來分別機器硬體是否支援感應 */
int	ginContactlessSupport = 0;
/* 用來確認是不是小螢幕機型 */
int	ginHalfLCD = VS_FALSE;
/* 用來確認是否支援觸控 */
int	ginTouchEnable = VS_FALSE;
/* 用以確認是否支援WiFi */
int	ginWiFiSupport = VS_FALSE;
unsigned long	gulPCIRebootTime = 946742400;   /* 預設值為2000年1月2日 */
unsigned long	gulTotalROMSize = 0;

extern  char gszTermVersionID[16 + 1];
extern  char gszTermVersionDate[16 + 1];
/* [260318_BUG] 新增控制參數 */
int ginToggleCardRetention = VS_TRUE;

/**
** The main entry of the terminal application
**/
int main(int argc,char *argv[])
{	
#if	_NEXSYS_APP_MODE_ == _NEXSYS_APP_MODE_SDK_
	inNexsysSDK_SelfTest();
	
	return EXIT_SUCCESS;
#else
	/* 查看檔案分佈 */
	inFunc_ls("-R -l", _AP_ROOT_PATH_);
        
        vdUtility_SYSFIN_LogMessage(AT, "AP VersionID:%s", gszTermVersionID);
        vdUtility_SYSFIN_LogMessage(AT, "AP VersionDate:%s", gszTermVersionDate);
	
	/* 紀錄機器類型 */
	inFunc_Decide_Machine_Type(&ginMachineType);
	/* 確認銀行版本 */
	inFunc_Decide_APVersion_Type(&ginAPVersionType);
	
	/* 紀錄機器狀態 */
	if (ginMachineType == _CASTLE_TYPE_V3C_)
	{
		vdUtility_SYSFIN_LogMessage(AT, "_CASTLE_TYPE_V3C_");
	}
	else if (ginMachineType == _CASTLE_TYPE_V3M_)
	{
		vdUtility_SYSFIN_LogMessage(AT, "_CASTLE_TYPE_V3M_");
	}
	else if (ginMachineType == _CASTLE_TYPE_V3P_)
	{
		vdUtility_SYSFIN_LogMessage(AT, "_CASTLE_TYPE_V3P_");
	}
	else if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		vdUtility_SYSFIN_LogMessage(AT, "_CASTLE_TYPE_V3UL_");
	}
	else if (ginMachineType == _CASTLE_TYPE_MP200_)
	{
		vdUtility_SYSFIN_LogMessage(AT, "_CASTLE_TYPE_MP200_");
	}
	else if (ginMachineType == _CASTLE_TYPE_UPT1000_)
	{
		vdUtility_SYSFIN_LogMessage(AT, "_CASTLE_TYPE_UPT1000_");
	}
	else if (ginMachineType == _CASTLE_TYPE_UPT1000F_)
	{
		vdUtility_SYSFIN_LogMessage(AT, "_CASTLE_TYPE_UPT1000F_");
	}
	
	if (ginContactlessSupport == VS_TRUE)
	{
		vdUtility_SYSFIN_LogMessage(AT, "ginContactlessSupport = True");
	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT, "ginContactlessSupport = False");
	}
	
	if (ginWiFiSupport == VS_TRUE)
	{
		vdUtility_SYSFIN_LogMessage(AT, "ginWiFiSupport = True");
	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT, "ginWiFiSupport = False");
	}
	
	if (ginHalfLCD == VS_TRUE)
	{
		vdUtility_SYSFIN_LogMessage(AT, "ginHalfLCD = True");
	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT, "ginHalfLCD = False");
	}
	
	if (ginTouchEnable == VS_TRUE)
	{
		vdUtility_SYSFIN_LogMessage(AT, "ginTouchEnable = True");
	}
	else
	{
		vdUtility_SYSFIN_LogMessage(AT, "ginTouchEnable = False");
	}
	
	/* 開機流程(開機流程不能中途跳出，所以都強制return success) */
	inEVENT_Responder(_BOOTING_EVENT_);
        TRANSACTION_OBJECT  pobTran = {};
	inFLOW_RunFunction(&pobTran, _EDC_BOOTING_RENEW_AUTO_REBOOT_TIME_);	/* 紀錄PCI重新開機時間 */
        
	/* Idle流程 */
	while (1)
	{
		inMENU_Decide_Idle_Menu();
	}

	return (VS_SUCCESS);
#endif
}
