#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/File.h"
#include "../EVENT/MenuMsg.h"
#include "../PRINT/Print.h"
#include "../FUNCTION/Signpad.h"
#include "../../NCCC/NCCCdcc.h"
#include "../FUNCTION/Utility.h"
#include "Display.h"
#include "DisTouch.h"
#include "errno.h"

extern int		ginDebug;				/* debug使用 */
int			ginTouch_Handle = -1;			/* 觸控文件的Handle 在idle下不希望開開關關，但若用pointer傳就幾乎每個function都要傳，所以決定使用glboal，同時也利用這個handle是否有值來得知是否已開觸控檔案 */
int			ginEventNum = 0;
extern	int		ginHalfLCD;
extern	int		ginTouchEnable;
extern SIGNPAD_OBJECT	gsrSignpad;				/* SignedPad中的Global變數，用來表示現在簽名方向是向左還是向右 */
DISTOUCH_EVENT		gsrEvent[_DisTouch_Event_Num_];		/* 用來放讀出的觸控事件 DISTOUCH_EVENT是仿照linux/input.h中的input_event結構 */
DISTOUCH_OBJECT		gsrDisTouchObj;				/* 因為要判斷拖曳的事件，需要記憶起始座標，所以放global */


/* 之前將ginIdleDispFlag移除，會有觸控失靈的狀況，目前研判是抓idle畫面觸控時，按到不是接受部份會close觸控事件檔案，會重複開關觸控事件檔案，此時只要Idle迴圈延遲一大，
 * 就會感覺觸控失靈。
 * 
 * 因此加入幾項改變，只有在確定進入下一畫面時，才close觸控事件檔案，加大gsrEvent空間到30個(原先為5)，儲存多個事件，
 * 到EV_SYN發生時，判斷是否在區域內，若在區域內就跳出，否則直到讀完觸控檔案。(以往只判斷一個EV_SYN座標就跳出，現在判斷多個座標增加容錯率)
 * 
 * 雖然做了這樣的修正，但整體還是建議while迴圈不要跑太多東西。
 */

/* 2016/6/24 下午 2:56 更新：
   目前有一個副作用，在可觸控畫面下利用非觸控方式到下個畫面，(例如:idle按數字鍵)，在回觸控畫面時會紀錄之前觸控的殘值
  （原因:只有在會發生觸控事件的地方才close，導致觸控事件檔案沒關繼續感應）
   ()作法一:離開inDisTouch_Sensor時候關，缺點while迴圈中，其他function執行過久會有明顯延遲 
   ()作法二:離開inDisTouch_Sensor不關閉，插入inDisTouch_Flush_TouchFile，方法Skip掉下一次讀取，缺點正常觸控畫面與正常感觸控畫面之間會有一次失靈。
   (v)作法三:離開while迴圈時關，插入inDisTouch_Flush_TouchFile，方法clode觸控檔案，缺點只要觸發離開inDisTouch_Sensor的地方都要插入inDisTouch_Flush_TouchFile
 */

/* 2016/7/5 下午 5:38
   目前無法判斷是用點擊還是拖曳到那一個區域，如果要做到準確的判斷是點擊，則要抓下筆時間和起筆時間的間隔做控制，並以起筆的狀態做判斷，或者只有偵測到下筆之後第一次座標才做
   是否在區域內的判斷。
 */

/*
Function	:inDisTouch_OpenTouchFile
Date&Time	:2016/6/24 上午 11:51
Describe	:開起觸控文件
*/
int inDisTouch_Open_TouchFile(int *inTouch_Handle)
{
	char	szDebugMsg[100 + 1];
	
	if (*inTouch_Handle < 0)
	{
		*inTouch_Handle = open (_TOUCH_FILE_ , O_RDONLY | O_NONBLOCK);
		if (*inTouch_Handle < 0)
		{
			/* Open fail */
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "TouchFile Open Fail handle:%d", *inTouch_Handle);
				inLogPrintf(AT, szDebugMsg);
			}
			/* 開失敗不支援觸控 */
			ginTouchEnable = VS_FALSE;
			/* [BUG] 移除檔案同步API SYNC()指令
			 * [20260506] 新增Touch 開啟及關閉失敗LOG */
			vdUtility_SYSFIN_LogMessage(AT,"DisTouch_Open Fail Handle[%d] error[%d]", *inTouch_Handle, errno);
			return (VS_ERROR);
		}
		else
		{
			/* Open success */
//			if (ginDebug == VS_TRUE)
//			{
//				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//				sprintf(szDebugMsg, "TouchFile Open Success handle:%d", *inTouch_Handle);
//				inLogPrintf(AT, szDebugMsg);
//			}
			/* 清空儲存的座標 */
			memset(&gsrDisTouchObj, 0x00, sizeof(DISTOUCH_OBJECT));
			inFile_Open_File_Cnt_Increase();
		}
		
	}
        
	return (VS_SUCCESS);
}

/*
Function	:inDIST_CloseTouchFile
Date&Time	:2016/6/24 下午 2:09
Describe	:關閉觸控文件
*/
int inDisTouch_Close_TouchFile(int *inTouch_Handle)
{
	char	szDebugMsg[100 + 1];
	
	if (*inTouch_Handle == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "No Need Close");
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_SUCCESS);
	}
	else if(*inTouch_Handle > 0)
	{
		/* 關閉錯誤 */
		if (close(*inTouch_Handle) < 0)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "TouchFile Close Fail handle:%d", *inTouch_Handle);
				inLogPrintf(AT, szDebugMsg);
			}
			/* [BUG] 移除檔案同步API SYNC()指令
			 * [20260506] 新增Touch 開啟及關閉失敗LOG */
			vdUtility_SYSFIN_LogMessage(AT,"DisTouch Close Fail Handle[%d]", *inTouch_Handle);
			return (VS_ERROR);
		}
		else
		{
//			if (ginDebug == VS_TRUE)
//			{
//				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
//				sprintf(szDebugMsg, "TouchFile Close Success handle:%d", *inTouch_Handle);
//				inLogPrintf(AT, szDebugMsg);
//			}
			
			/* close成功後自己要把handle設為-1 */
			*inTouch_Handle = -1;
			inFile_Open_File_Cnt_Decrease();
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function	:inDisTouch_Read_TouchFile
Date&Time	:2016/6/24 下午 6:27
Describe	:讀取觸控事件檔案，並回傳讀出長度
*/
int inDisTouch_Read_TouchFile(int *inTouch_Handle)
{
	int	inRead_Temp_Length = 0;	/* 每次讀的長度 */
        int	inRead_Length = 0;	/* 讀出的總長度 */
	char	szDebugMsg[100 + 1];
	
	memset(gsrEvent, 0x00, sizeof(DISTOUCH_EVENT) * ginEventNum);
	ginEventNum = 0;

	while (inRead_Temp_Length != -1 && ginEventNum < _DisTouch_Event_Num_)
	{
		inRead_Temp_Length = read(*inTouch_Handle, &gsrEvent[ginEventNum], sizeof(DISTOUCH_EVENT));

		if (inRead_Temp_Length != -1)
		{
			/* 讀出的長度不足一個事件，理論上不應該發生 */
			if (inRead_Temp_Length < sizeof (DISTOUCH_EVENT))
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "Read Length 小於一個event %d", inRead_Temp_Length);
					inLogPrintf(AT, szDebugMsg);
				}
				
				inDisTouch_Close_TouchFile(inTouch_Handle);
				
				return (VS_ERROR);
			}
			else
			{
				inRead_Length += inRead_Temp_Length;
				ginEventNum += inRead_Temp_Length / sizeof(DISTOUCH_EVENT);
			}
			
		}
		else
		{
			/* 讀取錯誤，或沒東西讀了 */
			break;
		}

	}
	
	if (ginEventNum > 0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "Event Num: %d", ginEventNum);
			inLogPrintf(AT, szDebugMsg);
		}
	}
        
        return (inRead_Length);
}

/*
Function	:inDisTouch_Flush_TouchFile
Date&Time	:2016/6/24 下午 6:34
Describe	:清掉殘值，"/dev/input/event0"不能寫、不能砍、不能seek也無法read到最後面(似乎會留一定的event)，目前作法關檔在重新開檔讓handle自動移至最後方，
		另一作法為用flag控制，若flag ON起來則下一次只偵測觸控不做回應
*/
int inDisTouch_Flush_TouchFile()
{
	if (ginTouchEnable != VS_TRUE)
		return (VS_SUCCESS);
	
	inDisTouch_Close_TouchFile(&ginTouch_Handle);
	
	/* 清空儲存的座標 */
	memset(&gsrDisTouchObj, 0x00, sizeof(DISTOUCH_OBJECT));
	
        return (VS_SUCCESS);
}

/*
Function	:inDisTouch_Sensor
Date&Time	:2016/6/27 下午 2:38
Describe	:分析感應事件，並看有沒有在要做回應的區域內，此function只分析點擊
*/
int inDisTouch_Sensor(int inTouchSensorFunc)
{
	int		inEventCount = 0;			/* 總共偵測到幾個事件 */
	int		inTouchEvent = _DisTouch_No_Event_;	/* 接回傳在哪個區域內的值 */
	long		lnRead_Length = 0;			/* 讀出的byte數 */
	char		szDebugMsg[100 + 1];
	
	if (ginTouchEnable != VS_TRUE)
		return (_DisTouch_No_Event_);
	
	/* 初始化儲存事件gsrEvent */
        memset(&gsrEvent, 0x00, sizeof(DISTOUCH_EVENT) * _DisTouch_Event_Num_);

	/* 開檔，裡面會偵測是否已經開檔 */
	inDisTouch_Open_TouchFile(&ginTouch_Handle);
	/* 將事件從檔案中讀出來，並放在結構中 */
        /* 讀取觸控檔案 */
	lnRead_Length = inDisTouch_Read_TouchFile(&ginTouch_Handle);
	
	/* 讀取長度為負數 回傳錯誤 */
	if (lnRead_Length <= 0)
		return (_DisTouch_No_Event_);
	
	/* 分析讀出的事件，是否下筆並有正確座標
	 * inEventCount：事件數 
	 * 
	 * 若 type是EV_KEY code是BTN_TOUCH 則Value == 1表示下筆 若是0表示起筆
	 * 若 type是EV_ABS code是0	   則Value表示X的絕對值 若該軸座標沒改變則不會觸發這個事件
	 *		   code是1	   則Value表示y的絕對值 若該軸座標沒改變則不會觸發這個事件
	 *		   code是24（0x18）則Value表示壓力值，但此系統只回傳0或1，和EV_KEY意義類似
	 * 若 type是EV_SYN		   同步為新座標
	 */
	for (inEventCount = 0; inEventCount < ginEventNum; inEventCount++)
	{
		if (EV_KEY == gsrEvent[inEventCount].ushType)
		{
			if (gsrEvent[inEventCount].ushCode == BTN_TOUCH)
			{
				/* 有按壓的事件表示下筆 */
				if (gsrEvent[inEventCount].uinValue == 1)
				{
					gsrDisTouchObj.inPenStatus = _DisTouch_PenStatus_Down_;
					gsrDisTouchObj.uszClicking = VS_TRUE;
				}
				/* 有起筆的事件 */
				else
				{
					gsrDisTouchObj.inPenStatus = _DisTouch_PenStatus_Up_;
				}
			}
		}
		else if (EV_ABS == gsrEvent[inEventCount].ushType)
		{
			/* X軸的絕對座標 */
			if (gsrEvent[inEventCount].ushCode == 0)
			{
				gsrDisTouchObj.inX_Temp = gsrEvent[inEventCount].uinValue;
			}
			/* Y軸的絕對座標 */
			else if (gsrEvent[inEventCount].ushCode == 1)
			{
				gsrDisTouchObj.inY_Temp = gsrEvent[inEventCount].uinValue;
			}
			/* 比較精細的ABS_PRESSURE會回傳壓力值，這裡只會回傳1或0，且會接在EV_KEY後面，所以沒有什麼用處 */
			else if (gsrEvent[inEventCount].ushCode == ABS_PRESSURE)
			{
				if (gsrEvent[inEventCount].uinValue == 1)
				{
					gsrDisTouchObj.inPenStatus = _DisTouch_PenStatus_Down_;
				}
				else if (gsrEvent[inEventCount].uinValue == 0)
				{
					gsrDisTouchObj.inPenStatus = _DisTouch_PenStatus_Up_;
				}
			}
		}
		else if (EV_SYN == gsrEvent[inEventCount].ushType)
		{
			/* x軸或y軸沒有值，不能確定為座標，continue直到收集到完整座標 */
			if (gsrDisTouchObj.inX_Temp == 0 || gsrDisTouchObj.inY_Temp == 0)
			{
				continue;
			}

			/* 座標確定後，檢察座標是否在區域內，inTouchSensorFunc:表示偵測哪個頁面的感應區域 */
			if (gsrDisTouchObj.uszClicking == VS_TRUE)
			{
				inTouchEvent = inDisTouch_InArea(&gsrDisTouchObj, inTouchSensorFunc);
				gsrDisTouchObj.uszClicking = VS_FALSE;
			}
			
			/* 回傳在哪個區域內 */
			if (inTouchEvent != _DisTouch_No_Event_)
			{
				if (ginDebug == VS_TRUE)
				{
					memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
					sprintf(szDebugMsg, "EV_SYN:[%d] X:%d y:%d", inEventCount, gsrDisTouchObj.inX_Temp, gsrDisTouchObj.inY_Temp);
					inLogPrintf(AT, szDebugMsg);
					inLogPrintf(AT, "-----------------------------------------------");
				}
				
				return (inTouchEvent);
			}
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			
			switch (gsrEvent[inEventCount].ushType)
			{
				case EV_KEY:
					sprintf(szDebugMsg, "EV_KEY:[%d] %d", inEventCount, gsrEvent[inEventCount].uinValue);
					break;
				case EV_ABS:
					if (gsrEvent[inEventCount].ushCode == ABS_X)
						sprintf(szDebugMsg, "EV_ABS:[%d] X:%d", inEventCount, gsrEvent[inEventCount].uinValue);
					else if (gsrEvent[inEventCount].ushCode == ABS_Y)
						sprintf(szDebugMsg, "EV_ABS:[%d] y:%d", inEventCount, gsrEvent[inEventCount].uinValue);
					else if (gsrEvent[inEventCount].ushCode == ABS_PRESSURE)
						sprintf(szDebugMsg, "EV_ABS:[%d] Pressure:%d", inEventCount, gsrEvent[inEventCount].uinValue);
					else
						sprintf(szDebugMsg, "EV_ABS:[%d] \"%d\":%d", inEventCount, gsrEvent[inEventCount].ushCode, gsrEvent[inEventCount].uinValue);
					break;
				case EV_SYN:
					sprintf(szDebugMsg, "EV_SYN:[%d] X:%d y:%d", inEventCount, gsrDisTouchObj.inX_Temp, gsrDisTouchObj.inY_Temp);
					break;
				default:
					sprintf(szDebugMsg, "\"%X\":[%d] Value:%d", gsrEvent[inEventCount].ushType, inEventCount, gsrEvent[inEventCount].uinValue);
					break;
			}

			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "time interval :%ld.%ld", gsrEvent[inEventCount].srTime.lnTime, gsrEvent[inEventCount].srTime.lnMicroSecond);
			inLogPrintf(AT, szDebugMsg);
			
			if (inEventCount == (lnRead_Length / sizeof (DISTOUCH_EVENT) - 1))
			{
				inLogPrintf(AT, "-----------------------------------------------");
			}

		}

	}

	return (VS_SUCCESS);
}

/*
Function	:inDisTouch_InArea
Date&Time	:2016/6/27 下午 3:20
Describe	:看座標是否在要做反應的區域內
 *		 inTouchSensorFunc:哪個畫面要偵測的觸控
 *		 srDisTouchObj:座標結構
*/
int inDisTouch_InArea(DISTOUCH_OBJECT *srDisTouchObj, int inTouchSensorFunc)
{
	int inFlag = 0; /* 回傳按到哪個區域，在DisTouch.h中用整數定義 */

	/* 不偵測畫面的觸控點擊 */
	if (inTouchSensorFunc == _Touch_NONE_)
	{

	}

	/* idle畫面的觸控 */
	if (inTouchSensorFunc == _Touch_IDLE_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			/* Idle Key1 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_IdleTouch_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_IdleTouch_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_IdleTouch_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_IdleTouch_KEY_1_Y2_))
			{
				inFlag = _IdleTouch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Idle Key2 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_IdleTouch_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_IdleTouch_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_IdleTouch_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_IdleTouch_KEY_2_Y2_))
			{
				inFlag = _IdleTouch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Idle Key3 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_IdleTouch_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_IdleTouch_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_IdleTouch_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_IdleTouch_KEY_3_Y2_))
			{
				inFlag = _IdleTouch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}

	}

	/* 選擇畫面的觸控 */
	if (inTouchSensorFunc == _Touch_MENU_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			/* _MenuTouch_KEY_1_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_1_Y2_))
			{
				inFlag = _MenuTouch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _MenuTouch_KEY_2_*/
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_2_Y2_))
			{
				inFlag = _MenuTouch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key3 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_3_Y2_))
			{
				inFlag = _MenuTouch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key4 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_4_Y2_))
			{
				inFlag = _MenuTouch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key5 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_5_Y2_))
			{
				inFlag = _MenuTouch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key6 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_6_Y2_))
			{
				inFlag = _MenuTouch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key7 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_7_Y2_))
			{
				inFlag = _MenuTouch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key8 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_8_Y2_))
			{
				inFlag = _MenuTouch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key9 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_9_Y2_))
			{
				inFlag = _MenuTouch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key10 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_10_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_10_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_10_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_10_Y2_))
			{
				inFlag = _MenuTouch_KEY_10_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key11 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_11_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_11_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_11_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_11_Y2_))
			{
				inFlag = _MenuTouch_KEY_11_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key12 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_12_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_12_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_12_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_12_Y2_))
			{
				inFlag = _MenuTouch_KEY_12_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key13 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_13_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_13_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_13_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_13_Y2_))
			{
				inFlag = _MenuTouch_KEY_13_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* Menu Key16 */
			if ((srDisTouchObj->inX_Temp > _DisTouch_MenuTouch_KEY_16_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_MenuTouch_KEY_16_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_MenuTouch_KEY_16_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_MenuTouch_KEY_16_Y2_))
			{
				inFlag = _MenuTouch_KEY_16_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}

	}


	/* 銀聯交易畫面的觸控 */
	if (inTouchSensorFunc == _Touch_UNIONPAY_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			/* _UnionpayTouch_KEY_1_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Unionpay_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Unionpay_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Unionpay_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Unionpay_KEY_1_Y2_))
			{
				inFlag = _UnionpayTouch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _UnionpayTouch_KEY_2_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Unionpay_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Unionpay_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Unionpay_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Unionpay_KEY_2_Y2_))
			{
				inFlag = _UnionpayTouch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			/* _UnionpayTouch_KEY_3_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Unionpay_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Unionpay_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Unionpay_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Unionpay_KEY_3_Y2_))
			{
				inFlag = _UnionpayTouch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _UnionpayTouch_KEY_4_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Unionpay_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Unionpay_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Unionpay_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Unionpay_KEY_4_Y2_))
			{
				inFlag = _UnionpayTouch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _UnionpayTouch_KEY_5_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Unionpay_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Unionpay_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Unionpay_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Unionpay_KEY_5_Y2_))
			{
				inFlag = _UnionpayTouch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _UnionpayTouch_KEY_6_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Unionpay_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Unionpay_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Unionpay_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Unionpay_KEY_6_Y2_))
			{
				inFlag = _UnionpayTouch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _UnionpayTouch_KEY_7_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Unionpay_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Unionpay_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Unionpay_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Unionpay_KEY_7_Y2_))
			{
				inFlag = _UnionpayTouch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _UnionpayTouch_KEY_8_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Unionpay_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Unionpay_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Unionpay_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Unionpay_KEY_8_Y2_))
			{
				inFlag = _UnionpayTouch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _UnionpayTouch_KEY_9_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Unionpay_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Unionpay_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Unionpay_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Unionpay_KEY_9_Y2_))
			{
				inFlag = _UnionpayTouch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}

	}

	/* 其他交易畫面的觸控 */
	if (inTouchSensorFunc == _Touch_OTHER_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			/* _OtherTouch_KEY_1_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Other_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Other_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Other_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Other_KEY_1_Y2_))
			{
				inFlag = _OtherTouch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _OtherTouch_KEY_2_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Other_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Other_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Other_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Other_KEY_2_Y2_))
			{
				inFlag = _OtherTouch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}


			/* _OtherTouch_KEY_3_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Other_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Other_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Other_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Other_KEY_3_Y2_))
			{
				inFlag = _OtherTouch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _OtherTouch_KEY_4_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Other_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Other_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Other_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Other_KEY_4_Y2_))
			{
				inFlag = _OtherTouch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			/* _OtherTouch_KEY_5_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Other_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Other_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Other_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Other_KEY_5_Y2_))
			{
				inFlag = _OtherTouch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _OtherTouch_KEY_6_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Other_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Other_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Other_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Other_KEY_6_Y2_))
			{
				inFlag = _OtherTouch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _OtherTouch_KEY_7_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Other_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Other_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Other_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Other_KEY_7_Y2_))
			{
				inFlag = _OtherTouch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _OtherTouch_KEY_8_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Other_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Other_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Other_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Other_KEY_8_Y2_))
			{
				inFlag = _OtherTouch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _OtherTouch_KEY_9_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Other_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Other_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Other_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Other_KEY_9_Y2_))
			{
				inFlag = _OtherTouch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

	}

	/* 功能畫面的觸控 */
	if (inTouchSensorFunc == _Touch_FUNCTION_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			/* _FunctionTouch_KEY_1_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_1_Y2_))
			{
				inFlag = _FunctionTouch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_2_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_2_Y2_))
			{
				inFlag = _FunctionTouch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_3_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_3_Y2_))
			{
				inFlag = _FunctionTouch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_4_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_4_Y2_))
			{
				inFlag = _FunctionTouch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_5_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_5_Y2_))
			{
				inFlag = _FunctionTouch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_6_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_6_Y2_))
			{
				inFlag = _FunctionTouch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_7_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_7_Y2_))
			{
				inFlag = _FunctionTouch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_8_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_8_Y2_))
			{
				inFlag = _FunctionTouch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_9_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_9_Y2_))
			{
				inFlag = _FunctionTouch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_10_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_10_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_10_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_10_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_10_Y2_))
			{
				inFlag = _FunctionTouch_KEY_10_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_11_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_11_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_11_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_11_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_11_Y2_))
			{
				inFlag = _FunctionTouch_KEY_11_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_12_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_12_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_12_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_12_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_12_Y2_))
			{
				inFlag = _FunctionTouch_KEY_12_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_13_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_13_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_13_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_13_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_13_Y2_))
			{
				inFlag = _FunctionTouch_KEY_13_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_14_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_14_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_14_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_14_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_14_Y2_))
			{
				inFlag = _FunctionTouch_KEY_14_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FunctionTouch_KEY_15_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Function_KEY_15_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Function_KEY_15_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Function_KEY_15_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Function_KEY_15_Y2_))
			{
				inFlag = _FunctionTouch_KEY_15_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}

	}

	/* 功能其他畫面的觸控 */
	if (inTouchSensorFunc == _Touch_FUNCOTHER_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			/* _FuncOtherTouch_KEY_1_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_1_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_2_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_2_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_3_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_3_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_4_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_4_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_5_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_5_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_6_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_6_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_7_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_7_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_8_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_8_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_9_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_9_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_10_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_10_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_10_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_10_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_10_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_10_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_11_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_11_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_11_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_11_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_11_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_11_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_12_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_12_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_12_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_12_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_12_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_12_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_13_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_13_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_13_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_13_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_13_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_13_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_14_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_14_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_14_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_14_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_14_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_14_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _FuncOtherTouch_KEY_15_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_FuncOther_KEY_15_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_FuncOther_KEY_15_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_FuncOther_KEY_15_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_FuncOther_KEY_15_Y2_))
			{
				inFlag = _FuncOtherTouch_KEY_15_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}

	}

	/* 電票交易畫面的觸控 */
	if (inTouchSensorFunc == _Touch_TICKET_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			/* _TicketTouch_KEY_1_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Ticket_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Ticket_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Ticket_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Ticket_KEY_1_Y2_))
			{
				inFlag = _TicketTouch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _TicketTouch_KEY_2_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Ticket_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Ticket_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Ticket_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Ticket_KEY_2_Y2_))
			{
				inFlag = _TicketTouch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}


			/* _TicketTouch_KEY_3_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Ticket_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Ticket_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Ticket_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Ticket_KEY_3_Y2_))
			{
				inFlag = _TicketTouch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _TicketTouch_KEY_4_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Ticket_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Ticket_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Ticket_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Ticket_KEY_4_Y2_))
			{
				inFlag = _TicketTouch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			/* _Ticket_KEY_5_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Ticket_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Ticket_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Ticket_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Ticket_KEY_5_Y2_))
			{
				inFlag = _TicketTouch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _Ticket_KEY_6_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Ticket_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Ticket_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Ticket_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Ticket_KEY_6_Y2_))
			{
				inFlag = _TicketTouch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _Ticket_KEY_7_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Ticket_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Ticket_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Ticket_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Ticket_KEY_7_Y2_))
			{
				inFlag = _TicketTouch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _Ticket_KEY_8_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Ticket_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Ticket_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Ticket_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Ticket_KEY_8_Y2_))
			{
				inFlag = _TicketTouch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			/* _Ticket_KEY_9_ */
			if ((srDisTouchObj->inX_Temp > _DisTouch_Ticket_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _DisTouch_Ticket_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _DisTouch_Ticket_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _DisTouch_Ticket_KEY_9_Y2_))
			{
				inFlag = _TicketTouch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}

	}

	/* 簽名畫面的觸控 */
	if (inTouchSensorFunc == _Touch_SIGNATURE_)
	{

		if (gsrSignpad.inPosition == _SIGNPAD_LEFT_)
		{
			/* 初步檢核，XY座標不為零，且為下筆狀態 */
			if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
			{
				/* _SignTouch_Clear_ */
				if ((srDisTouchObj->inX_Temp > _Distouch_Sign_Clear_X1_) &&
					(srDisTouchObj->inX_Temp < _Distouch_Sign_Clear_X2_) &&
					(srDisTouchObj->inY_Temp > _Distouch_Sign_Clear_Y1_) &&
					(srDisTouchObj->inY_Temp < _Distouch_Sign_Clear_Y2_))
				{
					inFlag = _SignTouch_Clear_;
					srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
					inDisTouch_Close_TouchFile(&ginTouch_Handle);

					return (inFlag);
				}

				/* _SignTouch_Ok_ */
				if ((srDisTouchObj->inX_Temp > _Distouch_Sign_Ok_X1_) &&
					(srDisTouchObj->inX_Temp < _Distouch_Sign_Ok_X2_) &&
					(srDisTouchObj->inY_Temp > _Distouch_Sign_Ok_Y1_) &&
					(srDisTouchObj->inY_Temp < _Distouch_Sign_Ok_Y2_))
				{
					inFlag = _SignTouch_Ok_;
					srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
					inDisTouch_Close_TouchFile(&ginTouch_Handle);

					return (inFlag);
				}

				/* _SignTouch_Rotate_ */
				if ((srDisTouchObj->inX_Temp > _Distouch_Sign_Rotate_X1_) &&
					(srDisTouchObj->inX_Temp < _Distouch_Sign_Rotate_X2_) &&
					(srDisTouchObj->inY_Temp > _Distouch_Sign_Rotate_Y1_) &&
					(srDisTouchObj->inY_Temp < _Distouch_Sign_Rotate_Y2_))
				{
					inFlag = _SignTouch_Rotate_;
					srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
					inDisTouch_Close_TouchFile(&ginTouch_Handle);

					return (inFlag);
				}

			}

		} else if (gsrSignpad.inPosition == _SIGNPAD_RIGHT_)
		{
			/* 初步檢核，XY座標不為零，且為下筆狀態 */
			if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
			{
				/* _SignTouch_Clear_ */
				if ((srDisTouchObj->inX_Temp > _Distouch_Sign_Right_Clear_X1_) &&
					(srDisTouchObj->inX_Temp < _Distouch_Sign_Right_Clear_X2_) &&
					(srDisTouchObj->inY_Temp > _Distouch_Sign_Right_Clear_Y1_) &&
					(srDisTouchObj->inY_Temp < _Distouch_Sign_Right_Clear_Y2_))
				{
					inFlag = _SignTouch_Clear_;
					srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
					inDisTouch_Close_TouchFile(&ginTouch_Handle);

					return (inFlag);
				}

				/* _SignTouch_Ok_ */
				if ((srDisTouchObj->inX_Temp > _Distouch_Sign_Right_Ok_X1_) &&
					(srDisTouchObj->inX_Temp < _Distouch_Sign_Right_Ok_X2_) &&
					(srDisTouchObj->inY_Temp > _Distouch_Sign_Right_Ok_Y1_) &&
					(srDisTouchObj->inY_Temp < _Distouch_Sign_Right_Ok_Y2_))
				{
					inFlag = _SignTouch_Ok_;
					srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
					inDisTouch_Close_TouchFile(&ginTouch_Handle);

					return (inFlag);
				}

				/* _SignTouch_Rotate_ */
				if ((srDisTouchObj->inX_Temp > _Distouch_Sign_Right_Rotate_X1_) &&
					(srDisTouchObj->inX_Temp < _Distouch_Sign_Right_Rotate_X2_) &&
					(srDisTouchObj->inY_Temp > _Distouch_Sign_Right_Rotate_Y1_) &&
					(srDisTouchObj->inY_Temp < _Distouch_Sign_Right_Rotate_Y2_))
				{
					inFlag = _SignTouch_Rotate_;
					srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
					inDisTouch_Close_TouchFile(&ginTouch_Handle);

					return (inFlag);
				}

			}

		}

	}

	if (inTouchSensorFunc == _Touch_DCC_CURENCY_CHECK_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurCheck_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurCheck_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurCheck_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurCheck_KEY_1_Y2_))
			{
				inFlag = _DccCurCheckTouch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurCheck_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurCheck_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurCheck_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurCheck_KEY_2_Y2_))
			{
				inFlag = _DccCurCheckTouch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_DCC_CURENCY_OPT_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_1_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_2_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_3_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_4_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_5_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_6_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_7_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_8_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_9_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_10_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_10_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_10_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_10_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_10_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_DccCurOption_KEY_Asterisk_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_DccCurOption_KEY_Asterisk_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_DccCurOption_KEY_Asterisk_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_DccCurOption_KEY_Asterisk_Y2_))
			{
				inFlag = _DccCurOptionTouch_KEY_Asterisk_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);


				return (inFlag);
			}

		}
	}

	if (inTouchSensorFunc == _Touch_CUP_LOGON_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_CUPLogOn_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_CUPLogOn_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_CUPLogOn_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_CUPLogOn_KEY_1_Y2_))
			{
				inFlag = _CUPLogOn_Touch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_CUPLogOn_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_CUPLogOn_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_CUPLogOn_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_CUPLogOn_KEY_2_Y2_))
			{
				inFlag = _CUPLogOn_Touch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_TICKET_CHECK_AMOUNT_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_TICKET_AMT_CHECK_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_TICKET_AMT_CHECK_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_TICKET_AMT_CHECK_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_TICKET_AMT_CHECK_KEY_1_Y2_))
			{
				inFlag = _TICKET_AMT_CHECK_Touch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_TICKET_AMT_CHECK_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_TICKET_AMT_CHECK_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_TICKET_AMT_CHECK_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_TICKET_AMT_CHECK_KEY_2_Y2_))
			{
				inFlag = _TICKET_AMT_CHECK_Touch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_TICKET_CHECK_RESULT_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_TICKET_AMT_CHECK_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_TICKET_AMT_CHECK_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_TICKET_AMT_CHECK_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_TICKET_AMT_CHECK_KEY_1_Y2_))
			{
				inFlag = _TICKET_RESULT_CHECK_Touch_ENTER_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_12X19_OPT_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_1_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_2_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_3_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_3_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_3_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_4_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_4_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_4_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_5_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_5_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_5_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_6_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_6_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_6_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_7_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_7_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_7_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_8_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_8_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_8_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_9_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_9_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_9_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_10_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_10_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_10_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_10_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_10_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_11_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_11_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_11_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_11_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_11_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT12X19_LINE_12_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT12X19_LINE_12_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT12X19_LINE_12_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT12X19_LINE_12_Y2_))
			{
				inFlag = _OPTTouch12X19_LINE_12_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_8X16_OPT_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_OPT8X16_LINE_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT8X16_LINE_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT8X16_LINE_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT8X16_LINE_1_Y2_))
			{
				inFlag = _OPTTouch8X16_LINE_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT8X16_LINE_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT8X16_LINE_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT8X16_LINE_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT8X16_LINE_2_Y2_))
			{
				inFlag = _OPTTouch8X16_LINE_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT8X16_LINE_3_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT8X16_LINE_3_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT8X16_LINE_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT8X16_LINE_3_Y2_))
			{
				inFlag = _OPTTouch8X16_LINE_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT8X16_LINE_4_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT8X16_LINE_4_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT8X16_LINE_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT8X16_LINE_4_Y2_))
			{
				inFlag = _OPTTouch8X16_LINE_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT8X16_LINE_5_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT8X16_LINE_5_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT8X16_LINE_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT8X16_LINE_5_Y2_))
			{
				inFlag = _OPTTouch8X16_LINE_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT8X16_LINE_6_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT8X16_LINE_6_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT8X16_LINE_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT8X16_LINE_6_Y2_))
			{
				inFlag = _OPTTouch8X16_LINE_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT8X16_LINE_7_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT8X16_LINE_7_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT8X16_LINE_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT8X16_LINE_7_Y2_))
			{
				inFlag = _OPTTouch8X16_LINE_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT8X16_LINE_8_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT8X16_LINE_8_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT8X16_LINE_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT8X16_LINE_8_Y2_))
			{
				inFlag = _OPTTouch8X16_LINE_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_16X22_OPT_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_1_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_2_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_3_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_3_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_3_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_4_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_4_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_4_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_5_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_5_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_5_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_6_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_6_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_6_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_7_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_7_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_7_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_8_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_8_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_8_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_9_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_9_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_9_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_10_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_10_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_10_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_10_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_10_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_11_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_11_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_11_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_11_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_11_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_12_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_12_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_12_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_12_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_12_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_13_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_13_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_13_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_13_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_13_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_14_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_14_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_14_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_14_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_14_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_15_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_15_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_15_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_15_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_15_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_OPT16X22_LINE_16_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_OPT16X22_LINE_16_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_OPT16X22_LINE_16_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_OPT16X22_LINE_16_Y2_))
			{
				inFlag = _OPTTouch16X22_LINE_16_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_CUST_RECEIPT_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_CUSTReceipt_ENTER_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_CUSTReceipt_ENTER_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_CUSTReceipt_ENTER_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_CUSTReceipt_ENTER_Y2_))
			{
				inFlag = _CUSTReceipt_Touch_ENTER_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_CUSTReceipt_CANCEL_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_CUSTReceipt_CANCEL_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_CUSTReceipt_CANCEL_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_CUSTReceipt_CANCEL_Y2_))
			{
				inFlag = _CUSTReceipt_Touch_CANCEL_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_CHECK_PWD_EDIT_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_CHECKPWDEdit_ENTER_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_CHECKPWDEdit_ENTER_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_CHECKPWDEdit_ENTER_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_CHECKPWDEdit_ENTER_Y2_))
			{
				inFlag = _CHECKEditPWD_Touch_ENTER_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}
	}

	if (inTouchSensorFunc == _Touch_HG_SELECT_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_HG_SELECT_Touch_YES_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_HG_SELECT_Touch_YES_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_HG_SELECT_Touch_YES_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_HG_SELECT_Touch_YES_Y2_))
			{
				inFlag = _HG_SELECT_Touch_YES_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_HG_SELECT_Touch_NO_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_HG_SELECT_Touch_NO_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_HG_SELECT_Touch_NO_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_HG_SELECT_Touch_NO_Y2_))
			{
				inFlag = _HG_SELECT_Touch_NO_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_SIGN_CHECK_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_SIGN_CHECK_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_SIGN_CHECK_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_SIGN_CHECK_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_SIGN_CHECK_KEY_1_Y2_))
			{
				inFlag = _SIGN_CHECK_Touch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_SIGN_CHECK_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_SIGN_CHECK_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_SIGN_CHECK_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_SIGN_CHECK_KEY_2_Y2_))
			{
				inFlag = _SIGN_CHECK_Touch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_NOSIGN_CHECK_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NOSIGN_CHECK_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NOSIGN_CHECK_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NOSIGN_CHECK_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NOSIGN_CHECK_KEY_1_Y2_))
			{
				inFlag = _NOSIGN_CHECK_Touch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

			if ((srDisTouchObj->inX_Temp > _Distouch_NOSIGN_CHECK_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NOSIGN_CHECK_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NOSIGN_CHECK_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NOSIGN_CHECK_KEY_2_Y2_))
			{
				inFlag = _NOSIGN_CHECK_Touch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_NEWUI_IDLE_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			/* 縮小偵測範圍，看是否能解決螢幕亂跳問題 */
			if ((srDisTouchObj->inX_Temp > (_Distouch_NEWUI_IDLE_KEY_FUNCTIONKEY_X1_ + 10)) &&
				(srDisTouchObj->inX_Temp < (_Distouch_NEWUI_IDLE_KEY_FUNCTIONKEY_X2_ - 10)) &&
				(srDisTouchObj->inY_Temp > (_Distouch_NEWUI_IDLE_KEY_FUNCTIONKEY_Y1_ + 10)) &&
				(srDisTouchObj->inY_Temp < (_Distouch_NEWUI_IDLE_KEY_FUNCTIONKEY_Y2_ - 10)))
			{
				inFlag = _NEWUI_IDLE_Touch_KEY_FUNCTIONKEY_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_NEWUI_FUNC_LINE_3_TO_8_3X3_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_1_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_2_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_3_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_4_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_5_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_6_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_7_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_8_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_8_3X3_KEY_9_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_8_3X3_Touch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_NEWUI_FUNC_LINE_3_TO_7_3X3_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_1_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_2_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_3_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_4_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_5_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_6_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_7_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_8_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_3_TO_7_3X3_KEY_9_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_3_TO_7_3X3_Touch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_LAST_PAGE_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_LAST_PAGE_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_LAST_PAGE_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_LAST_PAGE_BUTTON_Y2_))
			{
				inFlag = _NEWUI_LAST_PAGE_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_RETURN_IDLE_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_RETURN_IDLE_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_RETURN_IDLE_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_RETURN_IDLE_BUTTON_Y2_))
			{
				inFlag = _NEWUI_RETURN_IDLE_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_NEWUI_FUNC_LINE_2_TO_7_3X4_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_1_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_2_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_3_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_4_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_5_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_6_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_6_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_6_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_7_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_7_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_8_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_8_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_9_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_9_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_9_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_9_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_9_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_10_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_10_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_10_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_10_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_10_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_11_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_11_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_12_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_12_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_12_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_FUNC_LINE_2_TO_7_3X4_KEY_12_Y2_))
			{
				inFlag = _NEWUI_FUNC_LINE_2_TO_7_3X4_Touch_KEY_12_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_LAST_PAGE_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_LAST_PAGE_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_LAST_PAGE_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_LAST_PAGE_BUTTON_Y2_))
			{
				inFlag = _NEWUI_LAST_PAGE_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_RETURN_IDLE_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_RETURN_IDLE_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_RETURN_IDLE_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_RETURN_IDLE_BUTTON_Y2_))
			{
				inFlag = _NEWUI_RETURN_IDLE_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_NEWUI_CHOOSE_HOST_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_1_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_1_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_1_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_1_Y2_))
			{
				inFlag = _NEWUI_CHOOSE_HOST_Touch_HOST_1_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_2_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_2_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_2_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_2_Y2_))
			{
				inFlag = _NEWUI_CHOOSE_HOST_Touch_HOST_2_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_3_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_3_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_3_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_3_Y2_))
			{
				inFlag = _NEWUI_CHOOSE_HOST_Touch_HOST_3_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_4_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_4_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_4_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_4_Y2_))
			{
				inFlag = _NEWUI_CHOOSE_HOST_Touch_HOST_4_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_5_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_5_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_5_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_5_Y2_))
			{
				inFlag = _NEWUI_CHOOSE_HOST_Touch_HOST_5_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_6_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_6_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_CHOOSE_HOST_HOST_6_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_CHOOSE_HOST_HOST_6_Y2_))
			{
				inFlag = _NEWUI_CHOOSE_HOST_Touch_HOST_6_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_NEWUI_REVIEW_TOTAL_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_Y2_))
			{
				inFlag = _NEWUI_REVIEW_TOTAL_Touch_RETURN_IDLE_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_Y2_))
			{
				inFlag = _NEWUI_REVIEW_TOTAL_Touch_LAST_PAGE_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_NEWUI_REVIEW_BATCH_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_Y2_))
			{
				inFlag = _NEWUI_REVIEW_DETAIL_Touch_LAST_DATA_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_Y2_))
			{
				inFlag = _NEWUI_REVIEW_DETAIL_Touch_NEXT_DATA_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_BATCH_END_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_BATCH_END_Touch_ENTER_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_BATCH_END_Touch_ENTER_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_BATCH_END_Touch_ENTER_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_BATCH_END_Touch_ENTER_BUTTON_Y2_))
			{
				inFlag = _BATCH_END_Touch_ENTER_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}
	}

	if (inTouchSensorFunc == _Touch_OX_LINE8_8_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_TOUCH_OX_LINE8_8_ENTER_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_TOUCH_OX_LINE8_8_ENTER_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_TOUCH_OX_LINE8_8_ENTER_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_TOUCH_OX_LINE8_8_ENTER_BUTTON_Y2_))
			{
				inFlag = _Touch_OX_LINE8_8_ENTER_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_TOUCH_OX_LINE8_8_CANCEL_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_TOUCH_OX_LINE8_8_CANCEL_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_TOUCH_OX_LINE8_8_CANCEL_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_TOUCH_OX_LINE8_8_CANCEL_BUTTON_Y2_))
			{
				inFlag = _Touch_OX_LINE8_8_CANCEL_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}
	}

	if (inTouchSensorFunc == _Touch_O_LINE8_8_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_TOUCH_O_LINE8_8_ENTER_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_TOUCH_O_LINE8_8_ENTER_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_TOUCH_O_LINE8_8_ENTER_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_TOUCH_O_LINE8_8_ENTER_BUTTON_Y2_))
			{
				inFlag = _Touch_O_LINE8_8_ENTER_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}

		}
	}

	if (inTouchSensorFunc == _Touch_X_LINE8_8_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_TOUCH_X_LINE8_8_CANCEL_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_TOUCH_X_LINE8_8_CANCEL_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_TOUCH_X_LINE8_8_CANCEL_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_TOUCH_X_LINE8_8_CANCEL_BUTTON_Y2_))
			{
				inFlag = _Touch_X_LINE8_8_CANCEL_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_KEY_IN_MENU_LINE_3_TO_8_4X7_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_1_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_A_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_2_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_B_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_3_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_C_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_4_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_D_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_5_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_E_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_6_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_6_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_6_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_6_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_F_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_7_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_7_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_7_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_7_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_G_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_8_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_8_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_8_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_8_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_H_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_9_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_I_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_10_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_10_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_10_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_10_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_J_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_11_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_11_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_11_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_11_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_K_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_12_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_12_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_12_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_12_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_L_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_13_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_M_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_14_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_14_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_14_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_14_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_N_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_15_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_15_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_15_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_15_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_O_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_16_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_16_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_16_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_16_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_P_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_17_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_Q_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_18_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_18_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_18_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_18_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_R_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_19_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_19_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_19_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_19_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_S_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_20_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_20_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_20_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_20_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_T_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_21_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_U_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_22_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_22_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_22_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_22_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_V_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_23_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_23_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_23_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_23_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_W_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_24_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_24_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_24_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_24_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_X_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_25_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_CL_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_26_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_26_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_26_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_26_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_Y_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_27_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_27_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_27_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_27_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_Z_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_28_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_28_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_28_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_28_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_4X7_Touch_KEY_OK_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	if (inTouchSensorFunc == _Touch_KEY_IN_MENU_LINE_3_TO_8_3X4_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_1_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_1_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_1_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_1_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_1_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_2_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_3_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_4_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_4_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_4_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_4_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_4_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_5_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_5_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_5_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_5_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_5_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_6_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_6_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_6_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_6_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_6_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_7_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_7_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_7_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_7_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_7_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_8_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_8_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_8_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_8_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_8_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_9_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_9_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_9_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_9_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_9_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_10_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_10_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_10_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_10_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_CL_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_11_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_11_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_11_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_11_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_0_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_12_BUTTON_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_12_BUTTON_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_12_BUTTON_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_12_BUTTON_Y2_))
			{
				inFlag = _KEY_IN_MENU_LINE_3_TO_8_3X4_Touch_KEY_OK_BUTTON_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);

				return (inFlag);
			}
		}
	}

	/* [20251215_BUG_MDF][UI] 新增分期交易用警示畫面
	 * 新增接觸式感應區
	 */
	if (inTouchSensorFunc == _Touch_INST_INFO_CHECK_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
			if ((srDisTouchObj->inX_Temp > _Distouch_INST_INFO_ENTER_X1_) &&
				(srDisTouchObj->inX_Temp < _Distouch_INST_INFO_ENTER_X2_) &&
				(srDisTouchObj->inY_Temp > _Distouch_INST_INFO_ENTER_Y1_) &&
				(srDisTouchObj->inY_Temp < _Distouch_INST_INFO_ENTER_Y2_))
			{
				inFlag = _Touch_INST_INFO_CHECK_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);
				return (inFlag);
			}
		}
	}

	/* [114197] 中心收單作業ESG永續發展優化  
	 * [20260515] 新增 _APPROVAL_CHECK_Touch_MENU_ 的接觸參數 */
	if (inTouchSensorFunc == _APPROVAL_CHECK_Touch_MENU_)
	{
		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{

//			if ((srDisTouchObj->inX_Temp >= 45) &&
//				(srDisTouchObj->inX_Temp <= 170) &&
//				(srDisTouchObj->inY_Temp >= 255) &&
//				(srDisTouchObj->inY_Temp <= 285))
			if ((srDisTouchObj->inX_Temp >= 20) &&
				(srDisTouchObj->inX_Temp <= 150) &&
				(srDisTouchObj->inY_Temp >= 300) &&
				(srDisTouchObj->inY_Temp <= 360))
			{
				inFlag = _AGREE_TOUCH_YES_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);
				return (inFlag);
			}

		}

		/* 初步檢核，XY座標不為零，且為下筆狀態 */
		if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
		{
//			if ((srDisTouchObj->inX_Temp >= 45) &&
//				(srDisTouchObj->inX_Temp <= 200) &&
//				(srDisTouchObj->inY_Temp >= 315) &&
//				(srDisTouchObj->inY_Temp <= 340))
			if ((srDisTouchObj->inX_Temp >= 20) &&
				(srDisTouchObj->inX_Temp <= 170) &&
				(srDisTouchObj->inY_Temp >= 420) &&
				(srDisTouchObj->inY_Temp <= 480))
			{
				inFlag = _AGREE_TOUCH_NO_;
				srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
				inDisTouch_Close_TouchFile(&ginTouch_Handle);
				return (inFlag);
			}

		}
	}

	return (_DisTouch_No_Event_);
}

/*
Function        :inDisTouch_IsSigned
Date&Time       :2017/7/25 上午 11:08
Describe        :目前只用來偵測是否簽過簽名板用，若有判斷拖曳可能會誤判，所以目前只用在簽名板上
*/
int inDisTouch_IsSigned(DISTOUCH_OBJECT *srDisTouchObj, int inTouchSensorFunc)
{
	int inFlag; /* 回傳按到哪個區域，在DisTouch.h中用整數定義 */

	/* 簽名畫面的觸控 */
	if (inTouchSensorFunc == _Touch_SIGNATURE_)
	{

		if (gsrSignpad.inPosition == _SIGNPAD_LEFT_)
		{
			/* 初步檢核，XY座標不為零，且為下筆狀態 */
			if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
			{
				/* _SignTouch_Signpad_ */
				if ((srDisTouchObj->inX_Temp > _SIGNEDPAD_LEFT_X1_) && (srDisTouchObj->inX_Temp < _SIGNEDPAD_LEFT_X2_) && (srDisTouchObj->inY_Temp > _SIGNEDPAD_LEFT_Y1_) && (srDisTouchObj->inY_Temp < _SIGNEDPAD_LEFT_Y2_))
				{
					inFlag = _SignTouch_Signpad_;
					srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
					inDisTouch_Close_TouchFile(&ginTouch_Handle);

					return (inFlag);
				}

			}

		} else if (gsrSignpad.inPosition == _SIGNPAD_RIGHT_)
		{
			/* 初步檢核，XY座標不為零，且為下筆狀態 */
			if ((srDisTouchObj->inX_Temp != 0) && (srDisTouchObj->inY_Temp != 0))
			{
				/* _SignTouch_Signpad_ */
				if ((srDisTouchObj->inX_Temp > _SIGNEDPAD_RIGHT_X1_) && (srDisTouchObj->inX_Temp < _SIGNEDPAD_RIGHT_X2_) && (srDisTouchObj->inY_Temp > _SIGNEDPAD_RIGHT_Y1_) && (srDisTouchObj->inY_Temp < _SIGNEDPAD_RIGHT_Y2_))
				{
					inFlag = _SignTouch_Signpad_;
					srDisTouchObj->inPenStatus = _DisTouch_PenStatus_Up_;
					inDisTouch_Close_TouchFile(&ginTouch_Handle);

					return (inFlag);
				}

			}

		}

	}

	return (_DisTouch_No_Event_);
}
/*
Function	:inDisTouch_TouchSensor_Click_Slide
Date&Time	:2016/6/29 下午 4:06
Describe	:此function可以偵測滑動頁面和偵測點擊
*/
int inDisTouch_TouchSensor_Click_Slide(int inTouchSensorFunc)
{
	int		inReadLength;					/* 讀出byte數 */
	int		inEventCount = 0;				/* 讀出的事件數 */
	int		inTouchEvent = _DisTouch_No_Event_;		/* 需要做出反應的觸控事件 */
	char		szDebugMsg[100 + 1];
	
	if (ginTouchEnable != VS_TRUE)
		return (_DisTouch_No_Event_);
	
	/* 開啟觸控檔 */
	if (inDisTouch_Open_TouchFile(&ginTouch_Handle) != VS_SUCCESS)
		return (_DisTouch_No_Event_);
	
	/* 讀取觸控檔案 */
	inReadLength = inDisTouch_Read_TouchFile(&ginTouch_Handle);
	
	/* 讀取長度為負數 回傳錯誤 */
	if (inReadLength <= 0)
	{
		return (_DisTouch_No_Event_);
	}
//	/* 有感應事件，開啟背光 */
//	/* 畫面會一直閃，先不加 */
//	else
//	{
//		inDISP_BackLight_Set(_BACK_LIGHT_LCD_, 1);
//	}
	
	/* 開始分析讀出的事件，判斷方法:看到PenDown事件紀錄pen_down座標(座標不為0)，看到PenUp事件分析X軸距離，絕對值大於距離判定成功 */
	for (inEventCount = 0; inEventCount < ginEventNum; inEventCount++)
	{
		/* 判斷是否經過某個點 */
		if (inTouchEvent == _DisTouch_No_Event_)
		{
			inTouchEvent = inDisTouch_IsSigned(&gsrDisTouchObj, inTouchSensorFunc);
			if (inTouchEvent != _DisTouch_No_Event_)
			{
				break;
			}
		}
		
		/* PenDown or PenUp 事件 */
		if(EV_KEY == gsrEvent[inEventCount].ushType)
		{
			if(gsrEvent[inEventCount].ushCode == BTN_TOUCH)
			{
				/* PenDown 這時還沒有座標 要有同步事件才讀入*/
				if (gsrEvent[inEventCount].uinValue == 1 )
				{
					gsrDisTouchObj.inPenStatus = _DisTouch_PenStatus_Down_;
					gsrDisTouchObj.uszClicking = VS_TRUE;
				}
				/* PenUp */
				else
				{
					gsrDisTouchObj.inXpen_up = gsrDisTouchObj.inX_Temp;
					gsrDisTouchObj.inYpen_up = gsrDisTouchObj.inY_Temp;
					gsrDisTouchObj.inPenStatus = _DisTouch_PenStatus_Up_;
				}
			}
		}
		/* 座標值或壓力值事件 */
		else if(EV_ABS == gsrEvent[inEventCount].ushType)
		{
			/* X軸的絕對座標 */
			if(gsrEvent[inEventCount].ushCode == ABS_X)
			{
				gsrDisTouchObj.inX_Temp = gsrEvent[inEventCount].uinValue;
			}
			/* Y軸的絕對座標 */
			else if(gsrEvent[inEventCount].ushCode == ABS_Y)
			{
				gsrDisTouchObj.inY_Temp = gsrEvent[inEventCount].uinValue;
			}
			/* 比較精細的ABS_PRESSURE會回傳壓力值，這裡只會回傳1或0，且會接在EV_KEY後面，所以沒有什麼用處 */
			else if (gsrEvent[inEventCount].ushCode == ABS_PRESSURE)
			{
				if (gsrEvent[inEventCount].uinValue == 1)
				{
					gsrDisTouchObj.inPenStatus = _DisTouch_PenStatus_Down_;
				}
				else if (gsrEvent[inEventCount].uinValue == 0)
				{
					gsrDisTouchObj.inPenStatus = _DisTouch_PenStatus_Up_;
				}
			}
		}
		/* 同步事件 */
		else if(EV_SYN == gsrEvent[inEventCount].ushType)
		{
			/* X、Y都有值，紀錄起始座標 */
			if ((gsrDisTouchObj.inX_Temp >= 0 ) && (gsrDisTouchObj.inY_Temp >= 0 ))
			{
				/* 起始座標只紀錄一次，所以要看值是0，才紀錄 */
				if (gsrDisTouchObj.inXpen_down == 0)
				{
					gsrDisTouchObj.inXpen_down = gsrDisTouchObj.inX_Temp;
				}
				
				if (gsrDisTouchObj.inYpen_down == 0)
				{
					gsrDisTouchObj.inYpen_down = gsrDisTouchObj.inY_Temp;
				}
			}
			
			/* X、Y都有值且PenUp，判斷是否翻頁及點擊 */
			if ((gsrDisTouchObj.inX_Temp >= 0 ) && (gsrDisTouchObj.inY_Temp >= 0 ))
			{
				/* 下筆的判斷 */
				if (gsrDisTouchObj.inPenStatus == _DisTouch_PenStatus_Down_)
				{
					
				}
				
				/* 起筆的判斷 */
				if (gsrDisTouchObj.inPenStatus == _DisTouch_PenStatus_Up_)
				{
					/* 判斷是否翻頁 */
					if (inTouchEvent == _DisTouch_No_Event_)
					{
						inTouchEvent = inDisTouch_IsSlidePage(&gsrDisTouchObj, inTouchSensorFunc);
						if (inTouchEvent != _DisTouch_No_Event_)
						{
							break;
						}
					}

					/* 判斷是否點擊 */
					if (inTouchEvent == _DisTouch_No_Event_ && gsrDisTouchObj.uszClicking == VS_TRUE)
					{
						inTouchEvent = inDisTouch_InArea(&gsrDisTouchObj, inTouchSensorFunc);
						gsrDisTouchObj.uszClicking = VS_FALSE;
						if (inTouchEvent != _DisTouch_No_Event_)
						{
							break;
						}
					}
					
					/* 起筆且判斷完後無事件，初始化下筆座標值(用以解決快速點擊兩下判斷成滑動的問題) */
					if (inTouchEvent == _DisTouch_No_Event_)
					{
						gsrDisTouchObj.inXpen_down = 0;
						gsrDisTouchObj.inYpen_down = 0;
					}
					
				}
				
			}

		}

		/* Debug資訊 Start */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			switch (gsrEvent[inEventCount].ushType)
			{
				case EV_KEY:
					sprintf(szDebugMsg, "EV_KEY:[%d] %d", inEventCount, gsrEvent[inEventCount].uinValue);
					break;
				case EV_ABS:
					if (gsrEvent[inEventCount].ushCode == ABS_X)
						sprintf(szDebugMsg, "EV_ABS:[%d] X:%d", inEventCount, gsrEvent[inEventCount].uinValue);
					else if (gsrEvent[inEventCount].ushCode == ABS_Y)
						sprintf(szDebugMsg, "EV_ABS:[%d] y:%d", inEventCount, gsrEvent[inEventCount].uinValue);
					else if (gsrEvent[inEventCount].ushCode == ABS_PRESSURE)
						sprintf(szDebugMsg, "EV_ABS:[%d] Pressure:%d", inEventCount, gsrEvent[inEventCount].uinValue);
					else
						sprintf(szDebugMsg, "EV_ABS:[%d] \"%d\":%d", inEventCount, gsrEvent[inEventCount].ushCode, gsrEvent[inEventCount].uinValue);
					break;
				case EV_SYN:
					sprintf(szDebugMsg, "EV_SYN:[%d] X:%d y:%d", inEventCount, gsrDisTouchObj.inX_Temp, gsrDisTouchObj.inY_Temp);
					break;
				default:
					sprintf(szDebugMsg, "\"%X\":[%d] Value:%d", gsrEvent[inEventCount].ushType, inEventCount, gsrEvent[inEventCount].uinValue);
					break;
			}

			inLogPrintf(AT, szDebugMsg);

			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "time interval :%ld.%ld", gsrEvent[inEventCount].srTime.lnTime, gsrEvent[inEventCount].srTime.lnMicroSecond);
			inLogPrintf(AT, szDebugMsg);

			if (inEventCount == (inReadLength / sizeof (DISTOUCH_EVENT) - 1))
			{
				inLogPrintf(AT, "-----------------------------------------------");
			}

		}
		/* Debug資訊 End */
		
	}


	/* 有發生事件就關閉觸控檔案並回傳(即使沒發生事件也會回傳_DisTouch_No_Event_) */
	if (inTouchEvent != _DisTouch_No_Event_)
	{
		inDisTouch_Close_TouchFile(&ginTouch_Handle);
	}
	
	return (inTouchEvent);
}

/*
Function	:inDisTouch_IsSlidePage
Date&Time	:2016/6/29 下午 5:45
Describe	:判定是否翻頁
*/
int inDisTouch_IsSlidePage(DISTOUCH_OBJECT *srDisTouchObj, int inTouchSensorFunc)
{
	char	szDebugMsg[100 + 1];
	
	/* 若水平絕對值，大於垂直絕對值，以水平絕對值判斷(或存成結構，但改起來較複雜，有空再做)*/
	if (abs(srDisTouchObj->inXpen_down - srDisTouchObj->inX_Temp) > abs(srDisTouchObj->inYpen_down - srDisTouchObj->inY_Temp))
	{
		/* 超過判定距離，判定成功 */
		if (abs(srDisTouchObj->inXpen_down - srDisTouchObj->inX_Temp) > _DisTouch_Judgement_Horizontal_Distance_)
		{
			/* 判斷是拖曳就不判斷點擊 */
			gsrDisTouchObj.uszClicking = VS_FALSE;
			
			if (srDisTouchObj->inXpen_down > srDisTouchObj->inX_Temp)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Slide Next");
				}

				return (_DisTouch_Slide_Right_To_Left_);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Slide Last");
				}

				return (_DisTouch_Slide_Left_To_Right_);
			}

		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Pen distance:%d ", abs(srDisTouchObj->inXpen_down - srDisTouchObj->inX_Temp));
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Judgement distance:%d", _DisTouch_Judgement_Horizontal_Distance_);
				inLogPrintf(AT, szDebugMsg);
			}
			
		}
		
	}
	else
	{
		if (abs(srDisTouchObj->inYpen_down - srDisTouchObj->inY_Temp) > _DisTouch_Judgement_Vertical_Distance_)
		{
			/* 判斷是拖曳就不判斷點擊 */
			gsrDisTouchObj.uszClicking = VS_FALSE;
			
			if (srDisTouchObj->inXpen_down > srDisTouchObj->inX_Temp)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Slide Down");
				}

				return (_DisTouch_Slide_Up_To_Down_);
			}
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Slide Up");
				}

				return (_DisTouch_Slide_Down_To_Up_);
			}
			
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Pen distance:%d", abs(srDisTouchObj->inYpen_down - srDisTouchObj->inY_Temp));
				inLogPrintf(AT, szDebugMsg);
				
				memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
				sprintf(szDebugMsg, "Judgement distance:%d", _DisTouch_Judgement_Vertical_Distance_);
				inLogPrintf(AT, szDebugMsg);
			}
			
		}

	}
	/* 否則失敗 */
	
	
	/* 判斷拖曳失敗，初始化，準備做下一次判斷 */
	memset(&srDisTouchObj, 0x00, sizeof(DISTOUCH_OBJECT));

	return (_DisTouch_No_Event_);
}

/*
Function        :inDisTouch_Reverse_Back_Area
Date&Time       :2017/8/23 下午 4:06
Describe        :反白背景，目前只對文字有效
*/
int inDisTouch_Reverse_Back_Area(unsigned short usX, unsigned short usY, unsigned short usXSize, unsigned short usYSize)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0;

	usRetVal = CTOS_LCDGSetBox(usX, usY, usXSize, usYSize, d_LCD_FILL_XOR);
	if (usRetVal == d_OK)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, " Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
};

/*
Function        :inDisTouch_Reverse_Back_Key
Date&Time       :2017/8/23 下午 4:06
Describe        :反白背景，目前只對文字有效
*/
int inDisTouch_Reverse_Back_Key(int inTouchSensorFunc, int inEvent)
{
	int	inRetVal = VS_ERROR;
	int	inX1 = 0;
	int	inY1 = 0;
	int	inXSize = 0;
	int	inYSize = 0;
	
	if (inTouchSensorFunc == _Touch_12X19_OPT_)
	{
		if (inEvent == _OPTTouch12X19_LINE_1_)
		{
			inX1 = _Distouch_OPT12X19_LINE_1_X1_;
			inY1 = _Distouch_OPT12X19_LINE_1_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_1_X2_ - _Distouch_OPT12X19_LINE_1_X1_;
			inYSize = _Distouch_OPT12X19_LINE_1_Y2_ - _Distouch_OPT12X19_LINE_1_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_2_)
		{
			inX1 = _Distouch_OPT12X19_LINE_2_X1_;
			inY1 = _Distouch_OPT12X19_LINE_2_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_2_X2_ - _Distouch_OPT12X19_LINE_2_X1_;
			inYSize = _Distouch_OPT12X19_LINE_2_Y2_ - _Distouch_OPT12X19_LINE_2_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_3_)
		{
			inX1 = _Distouch_OPT12X19_LINE_3_X1_;
			inY1 = _Distouch_OPT12X19_LINE_3_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_3_X2_ - _Distouch_OPT12X19_LINE_3_X1_;
			inYSize = _Distouch_OPT12X19_LINE_3_Y2_ - _Distouch_OPT12X19_LINE_3_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_4_)
		{
			inX1 = _Distouch_OPT12X19_LINE_4_X1_;
			inY1 = _Distouch_OPT12X19_LINE_4_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_4_X2_ - _Distouch_OPT12X19_LINE_4_X1_;
			inYSize = _Distouch_OPT12X19_LINE_4_Y2_ - _Distouch_OPT12X19_LINE_4_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_5_)
		{
			inX1 = _Distouch_OPT12X19_LINE_5_X1_;
			inY1 = _Distouch_OPT12X19_LINE_5_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_5_X2_ - _Distouch_OPT12X19_LINE_5_X1_;
			inYSize = _Distouch_OPT12X19_LINE_5_Y2_ - _Distouch_OPT12X19_LINE_5_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_6_)
		{
			inX1 = _Distouch_OPT12X19_LINE_6_X1_;
			inY1 = _Distouch_OPT12X19_LINE_6_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_6_X2_ - _Distouch_OPT12X19_LINE_6_X1_;
			inYSize = _Distouch_OPT12X19_LINE_6_Y2_ - _Distouch_OPT12X19_LINE_6_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_7_)
		{
			inX1 = _Distouch_OPT12X19_LINE_7_X1_;
			inY1 = _Distouch_OPT12X19_LINE_7_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_7_X2_ - _Distouch_OPT12X19_LINE_7_X1_;
			inYSize = _Distouch_OPT12X19_LINE_7_Y2_ - _Distouch_OPT12X19_LINE_7_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_8_)
		{
			inX1 = _Distouch_OPT12X19_LINE_8_X1_;
			inY1 = _Distouch_OPT12X19_LINE_8_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_8_X2_ - _Distouch_OPT12X19_LINE_8_X1_;
			inYSize = _Distouch_OPT12X19_LINE_8_Y2_ - _Distouch_OPT12X19_LINE_8_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_9_)
		{
			inX1 = _Distouch_OPT12X19_LINE_9_X1_;
			inY1 = _Distouch_OPT12X19_LINE_9_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_9_X2_ - _Distouch_OPT12X19_LINE_9_X1_;
			inYSize = _Distouch_OPT12X19_LINE_9_Y2_ - _Distouch_OPT12X19_LINE_9_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_10_)
		{
			inX1 = _Distouch_OPT12X19_LINE_10_X1_;
			inY1 = _Distouch_OPT12X19_LINE_10_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_10_X2_ - _Distouch_OPT12X19_LINE_10_X1_;
			inYSize = _Distouch_OPT12X19_LINE_10_Y2_ - _Distouch_OPT12X19_LINE_10_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_11_)
		{
			inX1 = _Distouch_OPT12X19_LINE_11_X1_;
			inY1 = _Distouch_OPT12X19_LINE_11_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_11_X2_ - _Distouch_OPT12X19_LINE_11_X1_;
			inYSize = _Distouch_OPT12X19_LINE_11_Y2_ - _Distouch_OPT12X19_LINE_11_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch12X19_LINE_12_)
		{
			inX1 = _Distouch_OPT12X19_LINE_12_X1_;
			inY1 = _Distouch_OPT12X19_LINE_12_Y1_;
			inXSize = _Distouch_OPT12X19_LINE_12_X2_ - _Distouch_OPT12X19_LINE_12_X1_;
			inYSize = _Distouch_OPT12X19_LINE_12_Y2_ - _Distouch_OPT12X19_LINE_12_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		
	}
	
	if (inTouchSensorFunc == _Touch_8X16_OPT_)
	{
		if (inEvent == _OPTTouch8X16_LINE_1_)
		{
			inX1 = _Distouch_OPT8X16_LINE_1_X1_;
			inY1 = _Distouch_OPT8X16_LINE_1_Y1_;
			inXSize = _Distouch_OPT8X16_LINE_1_X2_ - _Distouch_OPT8X16_LINE_1_X1_;
			inYSize = _Distouch_OPT8X16_LINE_1_Y2_ - _Distouch_OPT8X16_LINE_1_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch8X16_LINE_2_)
		{
			inX1 = _Distouch_OPT8X16_LINE_2_X1_;
			inY1 = _Distouch_OPT8X16_LINE_2_Y1_;
			inXSize = _Distouch_OPT8X16_LINE_2_X2_ - _Distouch_OPT8X16_LINE_2_X1_;
			inYSize = _Distouch_OPT8X16_LINE_2_Y2_ - _Distouch_OPT8X16_LINE_2_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch8X16_LINE_3_)
		{
			inX1 = _Distouch_OPT8X16_LINE_3_X1_;
			inY1 = _Distouch_OPT8X16_LINE_3_Y1_;
			inXSize = _Distouch_OPT8X16_LINE_3_X2_ - _Distouch_OPT8X16_LINE_3_X1_;
			inYSize = _Distouch_OPT8X16_LINE_3_Y2_ - _Distouch_OPT8X16_LINE_3_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch8X16_LINE_4_)
		{
			inX1 = _Distouch_OPT8X16_LINE_4_X1_;
			inY1 = _Distouch_OPT8X16_LINE_4_Y1_;
			inXSize = _Distouch_OPT8X16_LINE_4_X2_ - _Distouch_OPT8X16_LINE_4_X1_;
			inYSize = _Distouch_OPT8X16_LINE_4_Y2_ - _Distouch_OPT8X16_LINE_4_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch8X16_LINE_5_)
		{
			inX1 = _Distouch_OPT8X16_LINE_5_X1_;
			inY1 = _Distouch_OPT8X16_LINE_5_Y1_;
			inXSize = _Distouch_OPT8X16_LINE_5_X2_ - _Distouch_OPT8X16_LINE_5_X1_;
			inYSize = _Distouch_OPT8X16_LINE_5_Y2_ - _Distouch_OPT8X16_LINE_5_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch8X16_LINE_6_)
		{
			inX1 = _Distouch_OPT8X16_LINE_6_X1_;
			inY1 = _Distouch_OPT8X16_LINE_6_Y1_;
			inXSize = _Distouch_OPT8X16_LINE_6_X2_ - _Distouch_OPT8X16_LINE_6_X1_;
			inYSize = _Distouch_OPT8X16_LINE_6_Y2_ - _Distouch_OPT8X16_LINE_6_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch8X16_LINE_7_)
		{
			inX1 = _Distouch_OPT8X16_LINE_7_X1_;
			inY1 = _Distouch_OPT8X16_LINE_7_Y1_;
			inXSize = _Distouch_OPT8X16_LINE_7_X2_ - _Distouch_OPT8X16_LINE_7_X1_;
			inYSize = _Distouch_OPT8X16_LINE_7_Y2_ - _Distouch_OPT8X16_LINE_7_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
		else if (inEvent == _OPTTouch8X16_LINE_8_)
		{
			inX1 = _Distouch_OPT8X16_LINE_8_X1_;
			inY1 = _Distouch_OPT8X16_LINE_8_Y1_;
			inXSize = _Distouch_OPT8X16_LINE_8_X2_ - _Distouch_OPT8X16_LINE_8_X1_;
			inYSize = _Distouch_OPT8X16_LINE_8_Y2_ - _Distouch_OPT8X16_LINE_8_Y1_;
			
			inRetVal = inDisTouch_Reverse_Back_Area(inX1, inY1, inXSize, inYSize);
		}
	}
	
	return (inRetVal);
};
