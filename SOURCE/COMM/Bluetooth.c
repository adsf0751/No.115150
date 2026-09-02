#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../EVENT/MenuMsg.h"
#include "../FUNCTION/Function.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/ECR.h"
#include "Bluetooth.h"


extern  int		ginDebug;		/* Debug使用 extern */
extern  int		ginISODebug;

/*
Function        :inBluetooth_Open
Date&Time       :2018/3/8 下午 2:29
Describe        :
*/
int inBluetooth_Open(void)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_BluetoothOpen();
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothOpen Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothOpen() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBluetooth_Connect
Date&Time       :2018/3/8 上午 10:18
Describe        :
*/
int inBluetooth_Connect(stDeviceInfo *spBTDeviceInfo, stServiceInfo *spBTServiceInfo)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_BluetoothConnect(spBTDeviceInfo, spBTServiceInfo);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothConnect Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothConnect() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	return (VS_SUCCESS);
}

/*
Function        :inBluetooth_Get_LastDevice
Date&Time       :2018/3/8 上午 10:25
Describe        :
*/
int inBluetooth_Get_LastDevice(stDeviceInfo *spBTDeviceInfo)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_BluetoothLastConnectedDeviceInfoGet(spBTDeviceInfo);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothLastConnectedDeviceInfoGet Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothLastConnectedDeviceInfoGet() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBluetooth_Get_Service_Info
Date&Time       :2018/3/8 上午 10:45
Describe        :
*/
int inBluetooth_Get_Service_Info(unsigned char *uszInfoNum, stServiceInfo* spBTServiceInfo)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;
	stServiceInfo*	spBTServiceInfoTemp;

	usRetVal = CTOS_BluetoothServiceInfoGet(uszInfoNum, &spBTServiceInfoTemp);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothServiceInfoGet Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothServiceInfoGet() OK");
			inLogPrintf(AT, szDebugMsg);
		}
		
		if (*uszInfoNum > _BLUETOOTH_SERVICE_INFO_MAXNUM_)
		{
			*uszInfoNum = _BLUETOOTH_SERVICE_INFO_MAXNUM_;
		}
		else
		{
			
		}
		
		memcpy(spBTServiceInfo, spBTServiceInfoTemp, sizeof(stServiceInfo) * (*uszInfoNum));
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBluetooth_Get_PairedDeviced_Info
Date&Time       :2018/3/8 下午 1:59
Describe        :
*/
int inBluetooth_Get_PairedDeviced_Info(unsigned char *uszInfoNum, stDeviceInfo* spBTDeviceInfo)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_BluetoothPairedListGet(uszInfoNum, &spBTDeviceInfo);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothPairedListGet Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_BluetoothPairedListGet() OK");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inBluetooth_Default_Cradle_Connect
Date&Time       :2018/3/8 上午 10:47
Describe        :
*/
int inBluetooth_Default_Cradle_Connect(void)
{
	unsigned char	uszDeviceNum = 0;
	unsigned char	uszServiceNum = 0;
	stDeviceInfo	stBTDeviceInfo[_BLUETOOTH_DEVICE_INFO_MAXNUM_] = {{0}};
	stServiceInfo	stBTServiceInfo[_BLUETOOTH_SERVICE_INFO_MAXNUM_] = {{0}};
	
	inBluetooth_Open();
	
	inBluetooth_Get_PairedDeviced_Info(&uszDeviceNum, stBTDeviceInfo);
	inBluetooth_Get_Service_Info(&uszServiceNum, stBTServiceInfo);
	
	if (stBTDeviceInfo[0].pbDeviceName != NULL	&&
	    stBTServiceInfo[0].pbSeviceName != NULL)
	{
		inBluetooth_Connect(&stBTDeviceInfo[0], &stBTServiceInfo[0]);
	}
	
	return (VS_SUCCESS);
}
