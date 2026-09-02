#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../PRINT/Print.h"
#include "../FUNCTION/Function.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Flow.h"
#include "../FUNCTION/Sqlite.h"
#include "../FUNCTION/CDT.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/CPT.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/ECR.h"
#include "../FUNCTION/HDT.h"
#include "../FUNCTION/HDPT.h"
#include "../FUNCTION/MultiFunc.h"
#include "../FUNCTION/File.h"
#include "../FUNCTION/Utility.h"
#include "../../NCCC/NCCCsrc.h"
#include "Comm.h"
#include "Ethernet.h"
#include "WiFi.h"

extern	int		ginDebug;
extern	int		ginDisplayDebug;
extern	int		ginEngineerDebug;
extern	int		ginMachineType;
extern ECR_TABLE	gsrECROb;
extern unsigned char	guszCTLSInitiOK;

extern int	ginECR_ServerFd;		/* File Descripter */
extern int	ginECR_ResponseFd;
extern int	ginTrans_ClientFd;
extern int	ginEthernetFlow;
extern BYTE	gbBarCodeECRBit;
int		ginWiFiSwitch = VS_FALSE;	/* 看WiFi是開還是關 */
unsigned char	guszNum = 0;
CTOS_stWifiInfo	gstWiFiInfo[_WiFi_Info_MaxNum_];

ECR_TRANS_TABLE stWiFi_ECRTable[] =
{
	/* 第零組標準【400】【8N1】規格 */
	{
		NULL,
		NULL,
		NULL,
		NULL,
                NULL,
                NULL,
		NULL,
                NULL,        
	},
	/* 第一組標準【144】【7E1】規格 */
	{
		NULL,
		NULL,
		NULL,
		NULL,
                NULL,
                NULL,
		NULL,
                NULL,        
	},
	/* 第二組邦柏(107)客製化需求【400】【8N1】規格 */
	{
		NULL,
		NULL,
		NULL,
		NULL,
                NULL,
                NULL,
		NULL,
                NULL,        
	},
	
};

/* Castle沒提供WiFi連Socket方法，要自行實做 */
/*
Function        :inWiFi_Open
Date&Time       :2017/5/3 上午 9:37
Describe        :
*/
int inWiFi_Open()
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_Open() START !");
	}
	
	inRetVal = CTOS_WifiOpen();
	/* 如果錯誤是IO_Processing，代表已開WiFi */
	if (inRetVal == d_OK	||
	    inRetVal == d_WIFI_IO_PROCESSING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Open OK");
			inLogPrintf(AT, szDebugMsg);
		}
		ginWiFiSwitch = VS_TRUE;
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Open Err :0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		vdUtility_SYSFIN_LogMessage(AT, "WiFi Open Err :0x%04X", inRetVal);
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_Open() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Close
Date&Time       :2017/5/3 上午 9:37
Describe        :
*/
int inWiFi_Close()
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_Close() START !");
	}
	
	inRetVal = CTOS_WifiClose();
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Close Err :0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Close OK");
			inLogPrintf(AT, szDebugMsg);
		}
		ginWiFiSwitch = VS_FALSE;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_Close() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Connect_Flow
Date&Time       :2017/9/29 下午 1:57
Describe        :WiFi只能用原生的，所以另寫一隻
*/
int inWiFi_Connect_Flow(char *szHostIP, char *szPort)
{
	int	inRetVal = VS_ERROR;
	char	szI_FES_Mode[2 + 1] = {0};
	char	szTRTFileName[12 + 1] = {0};
	char	szDebugMsg[100 + 1] = {0};
	char	szCFESMode[2 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_Connect_Flow() START !");

		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "IP:%s Port:%s", szHostIP, szPort);
		inLogPrintf(AT, szDebugMsg);

	}
	
	/* 一律先重置Handle */
	ginTrans_ClientFd = -1;
	
	/* 用原生C Socket */
	memset(szI_FES_Mode, 0x00, sizeof(szI_FES_Mode));
	inGetI_FES_Mode(szI_FES_Mode);
	memset(szTRTFileName, 0x00, sizeof(szTRTFileName));
	inGetTRTFileName(szTRTFileName);
	memset(szCFESMode, 0x00, sizeof(szCFESMode));
	inGetCloud_MFES(szCFESMode);
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "IFES: %s, %s", szI_FES_Mode, szTRTFileName);
		inLogPrintf(AT, szDebugMsg);
		
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "CFES: %s", szCFESMode);
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 判斷是IFES及非大來，其他全跑IFES */
	/* 2018/5/8 下午 2:10 DFS需求不再使用大來主機 */
	if (memcmp(szI_FES_Mode, "Y", strlen("Y")) == 0	||
	    memcmp(szCFESMode, "Y", strlen("Y")) ==0)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: TLS");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = inETHERNET_Connect_TLS_Flow(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_IFES_;
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	/* WiFi沒有虹堡API */
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Connect Flow: Native");
			inLogPrintf(AT, szDebugMsg);
		}
		
		inRetVal = inETHERNET_Connect_By_Native(szHostIP, szPort);
		/* 標示現在連線方式，斷線後重新恢復成_ETHERNET_FLOW_CTOS_ */
		ginEthernetFlow = _ETHERNET_FLOW_NATIVE_;

	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_Connect_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (inRetVal);
}

/*
Function        :inWiFi_DisConnect_Flow
Date&Time       :2017/9/29 下午 2:24
Describe        :分流
*/
int inWiFi_DisConnect_Flow()
{
	int	inRetVal = VS_ERROR;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_DisConnect_Flow() START !");
	}
	
	if (ginEthernetFlow == _ETHERNET_FLOW_IFES_)
	{
		inRetVal = inETHERNET_DisConnect_TLS_Flow();
		if (inRetVal == VS_SUCCESS)
		{
			ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
		}
		else
		{
			return (VS_ERROR);
		}
	}
	/* 用native方式建Socket，之後要優化再考慮使用(優點自己決定Timeout等等細微設定) */
	else
	{
		inRetVal = inETHERNET_DisConnect_By_Native();
		if (inRetVal == VS_SUCCESS)
		{
			ginEthernetFlow = _ETHERNET_FLOW_CTOS_;
		}
	}
		
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_DisConnect_Flow() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Initial
Date&Time       :2017/7/18 下午 3:13
Describe        :
*/
int inWiFi_Initial()
{
	int	inRetVal = 0x00;
	int	inRetryNow = 0;
	int	inRetryMax = 3;
	char	szEDCIPAddress[16 + 1];
	char	szSubMask[16 + 1];
	char	szGateWay[16 + 1];
	char	szDHCPMode[2 + 1];
	char	szDebugMsg[100 + 1];
	char	szAutoConnect[10 + 1];
	
	/* inWiFi_Initial() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inWiFi_Initial() START！");
        }
	
	memset(szEDCIPAddress, 0x00, sizeof(szEDCIPAddress));
        memset(szSubMask, 0x00, sizeof(szSubMask));
        memset(szGateWay, 0x00, sizeof(szGateWay));
	
	inDISP_ClearAll();
	inFunc_Display_LOGO(0,  _COORDINATE_Y_LINE_16_2_);					/* 第一層顯示 LOGO */
	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <通訊設定> */
	
	memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
	inGetAutoConnect(szAutoConnect);
	if (szAutoConnect[1] == '1')
	{
		if (inWiFi_IsConected() == VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "Already Connected");
				inLogPrintf(AT, szDebugMsg);
			}
		}

	}
	else
	{
		inWiFi_WifiDisconnectAP();
		
		inRetryNow = 0;
		/* 若上次有設定AutoConnect，則WiFi Open時會自動抓上一次的AP */
		do
		{
			/* 停一秒，不然API會一直跳d_WIFI_IO_PROCESSING 0x6001 */
			inDISP_Wait(1000);
			
			inRetVal = inWiFi_Open();
			inRetryNow++;

		} while ((inRetryNow < inRetryMax) && (inRetVal != VS_SUCCESS));
		
		/* 若重試三次還是失敗就跳出 */
		if (inRetVal != VS_SUCCESS)
		{
			vdUtility_SYSFIN_LogMessage(AT, "Wifi retry failed inRetryNow:%d", inRetryNow);
			return (VS_ERROR);
		}
	}
	
        if (inGetTermIPAddress(szEDCIPAddress) == VS_ERROR)
        {
                /* inGetEDCIP ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetEDCIP() ERROR!!");
                return (VS_ERROR);
        }
        
        if (inGetTermGetewayAddress(szGateWay) == VS_ERROR)
        {
                /* inGetGateWay ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetGateWay() ERROR!!");
                return (VS_ERROR);
        }
        
        if (inGetTermMASKAddress(szSubMask) == VS_ERROR)
        {
                /* inGetSubMask ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetSubMask() ERROR!!");
                return (VS_ERROR);
        }

	memset(szDHCPMode, 0x00, sizeof(szDHCPMode));
	inGetDHCP_Mode(szDHCPMode);
	if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
	{
		/* DHCP有開不用設定這三項 */
	}
	else
	{
		/* IP */
		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_IP, (unsigned char*)szEDCIPAddress, strlen(szEDCIPAddress));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFI_CONFIG_IP Error");
			}
		}

		/* MASK */
		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_MASK, (unsigned char*)szGateWay, strlen(szGateWay));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFI_CONFIG_MASK Error");
			}
		}

		/* GATEWAY */
		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_GATEWAY, (unsigned char*)szSubMask, strlen(szSubMask));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFI_CONFIG_GATEWAY Error");
			}
		}
	}
	
	/* inWiFi_Initial()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inWiFi_Initial()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_Begin
Date&Time       :2017/7/18 下午 3:13
Describe        :
*/
int inWiFi_Begin(TRANSACTION_OBJECT *pobtran)
{
        char    szCommIndex[2 + 1];
        char    szDebugBuf[64 + 1];

        /* inWiFi_Begin() START */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inWiFi_Begin() START！");
        }

        memset(szCommIndex, 0x00, sizeof(szCommIndex));
        /* 從HDT內get CPT的index */
        if (inGetCommunicationIndex(szCommIndex) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetCommunicationIndex Error!");
                
                return (VS_ERROR);
        }
        else
        {
                /* CPT index從0開始，但是CPTRec index從1開始，所以要減一  */
                pobtran->srBRec.inCPTIndex = atoi(szCommIndex) - 1;
        }

        if (inLoadCPTRec(pobtran->srBRec.inCPTIndex) == VS_ERROR)
        {
                if (ginDebug == VS_TRUE)
                {
                        sprintf(szDebugBuf, "inLoadCPTRec(%d) Error!", pobtran->srBRec.inCPTIndex);
                        inLogPrintf(AT, szDebugBuf);
                }
                /* Load CPT失敗 */
                return (VS_ERROR);
        }

        /* inWiFi_Begin()_END */
        if (ginDebug == VS_TRUE)
        {
                inLogPrintf(AT, "----------------------------------------");
                inLogPrintf(AT, "inWiFi_Begin()_END");
        }

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_SetConfig
Date&Time       :2017/7/18 下午 3:14
Describe        :
*/
int inWiFi_SetConfig(void)
{
	int	inRetVal = 0;
	int	inConnectMaxCnt = 1;		/* 連線最大重試次數(含第一次) */
	int	inConnectNowCnt = 0;		/* 連線目前重試次數 */
        char	szHostIPPrimary[16 + 1];
	char	szHostIPSecond[16 + 1];
	char	szHostPortNoPrimary[6 + 1];
	char	szHostPortNoSecond[6 + 1];
	char	szConfig[16 + 1];
	char	szDebugMsg[100 + 1];
	char	szDHCPMode[2 + 1];
	char	szAutoConnect[10 + 1];

        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inWiFi_SetConfig() START!");

	/* Set Host IP */
	memset(szHostIPPrimary,0x00,sizeof(szHostIPPrimary));
	if (inGetHostIPPrimary(szHostIPPrimary) == VS_ERROR)
	{
                /* inGetHostIPPrimary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetHostIPPrimary() ERROR!!");

                return (VS_ERROR);
	}
	     
	/* Set Host Port */
	memset(szHostPortNoPrimary,0x00,sizeof(szHostPortNoPrimary));
	if (inGetHostPortNoPrimary(szHostPortNoPrimary) == VS_ERROR)
	{
		/* Get HostPortNumber Primary ERROR */
		/* debug */
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inGetHostPortNoPrimary() ERROR!!");
                
		 return (VS_ERROR);
	}
	
	/* Auto Connect */
	memset(szConfig, 0x00, sizeof(szConfig));
	memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
	inGetAutoConnect(szAutoConnect);
	if (szAutoConnect[1] == '1')
	{
		szConfig[0] = '1';
	}
	else
	{
		szConfig[0] = '0';
	}
	
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_AUTOCON_AP, (unsigned char*)szConfig, 1);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_AUTOCON_AP Error");
		}
	}

        /*
                d_WiFi_CONFIG_DHCP
                Set IP configuration.
                = 0 : Static. Use the static IP set in the Ethernet configuration.
                = 1 : DHCP. Retrieve the dynamic IP from the DHCP server.
        */
        /* DHCP預設0 */
	memset(szConfig, 0x00, sizeof(szConfig));
	memset(szDHCPMode, 0x00, sizeof(szDHCPMode));
	inGetDHCP_Mode(szDHCPMode);
	if (memcmp(szDHCPMode, "Y", strlen("Y")) == 0)
	{
		szConfig[0] = 0x31;
	}
	else
	{
		szConfig[0] = 0x30;
	}
        
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_DHCP, (unsigned char*)szConfig, 1);
	if (inRetVal != VS_SUCCESS)
	{
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "d_ETHERNET_CONFIG_DHCP Error");
	
                return (VS_ERROR);
	}

	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do 
	{	
		inRetVal = inWiFi_Connect_Flow(szHostIPPrimary, szHostPortNoPrimary);
		inConnectNowCnt ++;	
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);
        
	
	/* 第一授權主機連線成功 */
        if (inRetVal == VS_SUCCESS)
        {
                return (VS_SUCCESS);
        }
        else
	{
               /* 若第一授權主機連線不成功，改連第二授權主機 */ 
	}
	
	/* 第一授權IP失敗 嘗試第二IP */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "First IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Try Second IP...");
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 嘗試連線第二授權主機 */
	/* 更改HostIP */
	/* Set Host IP */
        memset(szHostIPSecond,0x00,sizeof(szHostIPSecond));
        if (inGetHostIPSecond(szHostIPSecond) == VS_ERROR)
        {
                /* inGetHostIPSecond ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetHostIPSecond() ERROR!!");

                return (VS_ERROR);
        }
        
	/* Set Host Port */
        memset(szHostPortNoSecond, 0x00, sizeof(szHostPortNoSecond));
        if (inGetHostPortNoSecond(szHostPortNoSecond) == VS_ERROR)
        {
                /* Get HostPortNumber Primary ERROR */
                /* debug */
                if (ginDebug == VS_TRUE)
                        inLogPrintf(AT, "inGetHostPortNoSecond() ERROR!!");
                
                return (VS_ERROR);
        }
        
	/* 重置目前重試次數 */
	inConnectNowCnt = 0;
	/* 若連線失敗看是否要重試 */
	do 
	{	
		inRetVal = inWiFi_Connect_Flow(szHostIPSecond, szHostPortNoSecond);
		inConnectNowCnt ++;
	}while (inRetVal != VS_SUCCESS && inConnectNowCnt < inConnectMaxCnt);

	/* 第二授權主機連線成功 */
        if (inRetVal == VS_SUCCESS)
        {
                return (VS_SUCCESS);
        }
        else
	{
               /* 若第二授權主機連線不成功，直接斷線 */ 
	}
	
	/* 第二授權IP失敗 */
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Second IP Conenct Fail");
		inLogPrintf(AT, szDebugMsg);
	}
	
	/* 到最後還失敗，斷線 */
	inWiFi_END();
	
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inWiFi_SetConfig() END!");
        
	/* 若連線成功，中間就會Return出去，若一直失敗，則最後一定回傳失敗 */
        return (VS_ERROR);
}

/*
Function        :inWiFi_END
Date&Time       :2017/7/19 上午 11:09
Describe        :斷線
*/
int inWiFi_END(void)
{
        /* inWiFi_END() START! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inWiFi_END() START!");
	
	if (inWiFi_DisConnect_Flow() != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
			inLogPrintf(AT, "inWiFi_Disconnect_Flow() == VS_ERROR");
		return (VS_ERROR);
	}

        /* inWiFi_END() END! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inWiFi_END() END!");

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_Flush
Date&Time       :2017/9/29 下午 2:35
Describe        :清空Comport的資料，WiFi無此API可使用
*/
int inWiFi_Flush(void)
{
        /* inWiFi_Flush() START! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inWiFi_Flush() START!");
	
	
        /* inWiFi_Flush() END! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inWiFi_Flush() END!");

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_DeInitial
Date&Time       :2017/9/29 下午 2:36
Describe        :關閉Comport
*/
int inWiFi_DeInitial(void)
{
	int	inRetVal = 0;
	char	szDebugMsg[100 + 1];   /* debug message */
        
        /* inWiFi_DeInitial() START! */
        if (ginDebug == VS_TRUE)
                inLogPrintf(AT, "inWiFi_DeInitial() START!");

	inRetVal = inWiFi_Close();
	if (inRetVal != VS_SUCCESS)
	{
                /* debug */
                if (ginDebug == VS_TRUE)
                {
                        memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
                        sprintf(szDebugMsg, "inWiFi_Close ERR");
                        inLogPrintf(AT, szDebugMsg);
                }
	}
	
        /* inWiFi_DeInitial() END! */
	if (ginDebug == VS_TRUE)
		inLogPrintf(AT, "inWiFi_DeInitial() END!");

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_Switch
Date&Time       :2017/5/5 上午 11:48
Describe        :
*/
int inWiFi_Switch()
{
	if (ginWiFiSwitch == VS_FALSE)
	{
		inWiFi_Open();
	}
	else
	{
		inWiFi_Close();
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Status
Date&Time       :2017/5/3 上午 9:52
Describe        :
*/
int inWiFi_Status()
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned int	uiStatus = 0;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_Status() START !");
	}
	
	inRetVal = inWiFi_GetStatus(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_GetStatus Err");
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	if ((uiStatus & d_WIFI_STATE_AP_CONNECTED) == d_WIFI_STATE_AP_CONNECTED)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_AP_CONNECTED");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_SCANNING) == d_WIFI_STATE_SCANNING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_SCANNING");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if ((uiStatus && d_WIFI_STATE_AP_CONNECTING) == d_WIFI_STATE_AP_CONNECTING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_AP_CONNECTING");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_CONNECTING) == d_WIFI_STATE_CONNECTING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_CONNECTING");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_SENDING) == d_WIFI_STATE_SENDING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_SENDING");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_RECEIVING) == d_WIFI_STATE_RECEIVING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_RECEIVING");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_DISCONNECTING) == d_WIFI_STATE_DISCONNECTING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_DISCONNECTING");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if ((uiStatus & d_WIFI_STATE_AP_DISCONNECTING) == d_WIFI_STATE_AP_DISCONNECTING)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "d_WIFI_STATE_AP_DISCONNECTING");
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_Status() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_GetStatus
Date&Time       :2017/6/2 上午 10:06
Describe        :
*/
int inWiFi_GetStatus(unsigned int *uiStatus)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_GetStatus() START !");
	}
	
	inRetVal = CTOS_WifiStatus(uiStatus);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_WifiStatus Err :0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		inRetVal = VS_ERROR;
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_GetStatus() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_Scan
Date&Time       :2017/5/3 下午 5:42
Describe        :Scan是一段過程，所以需等到CTOS_WifiStatus吐d_ok或error才可做下一步，若吐d_WIFI_STATE_SCANNING則等到他完成
*/
int inWiFi_Scan()
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_Scan() START !");
	}
	
	inRetVal = CTOS_WifiScan();
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFi Scan Err :0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_Scan() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
		
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ScanProcess
Date&Time       :2017/6/2 上午 10:16
Describe        :Scan是一段過程，所以需等到CTOS_WifiStatus吐d_ok或error才可做下一步，若吐d_WIFI_STATE_SCANNING則等到他完成
*/
int inWiFi_ScanProcess()
{
	int		inRetVal = VS_SUCCESS;
	unsigned int	uiStatus = 0;
	
	inWiFi_Scan();
	
	while(1)
	{
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("掃描中", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
		
		inRetVal = inWiFi_GetStatus(&uiStatus);
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("掃描完成", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			break;
		}
		else
		{
			/* 繼續掃等完成 */
			if (uiStatus & d_WIFI_STATE_SCANNING)
			{
				continue;
			}
			else
			{
				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("掃描失敗", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
				break;
			}
		}
		
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_IsConected
Date&Time       :2017/5/31 下午 3:51
Describe        :是否已連上
*/
int inWiFi_IsConected()
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned int	uiStatus = 0;
	
	inRetVal = inWiFi_GetStatus(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_GetStatus Err");
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	if ((uiStatus & d_WIFI_STATE_AP_CONNECTED) == d_WIFI_STATE_AP_CONNECTED)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inWiFi_IsScanning
Date&Time       :2017/6/2 上午 9:56
Describe        :是否掃描中
*/
int inWiFi_IsScanning()
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
	unsigned int	uiStatus = 0;
	
	inRetVal = inWiFi_GetStatus(&uiStatus);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_GetStatus Err");
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	if ((uiStatus & d_WIFI_STATE_SCANNING) == d_WIFI_STATE_SCANNING)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inWiFi_Info_Get
Date&Time       :2017/5/3 下午 5:50
Describe        :
*/
int inWiFi_Info_Get(unsigned char* uszNum, CTOS_stWifiInfo *srInfo)
{
	int		inRetVal = VS_ERROR;
	char		szDebugMsg[100 + 1];
	CTOS_stWifiInfo *srInfoTemp;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_Info_Get() START !");
	}
	
	inRetVal = CTOS_WifiInfoGet(uszNum, &srInfoTemp);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_Info_Get Err :0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	else
	{
		if (*uszNum > _WiFi_Info_MaxNum_)
		{
			*uszNum = _WiFi_Info_MaxNum_;
		}
		else
		{
			
		}
			
		memcpy(srInfo, srInfoTemp, sizeof(CTOS_stWifiInfo) * (*uszNum));
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_Info_Get() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_WifiConnectAP
Date&Time       :2017/5/3 下午 5:50
Describe        :
*/
int inWiFi_WifiConnectAP(CTOS_stWifiInfo* srInfo, char*	szPassword, int inPasswordLen)
{
	int	inRetVal = -1;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_WifiConnectAP() START !");
	}
	
	inRetVal = CTOS_WifiConnectAP(srInfo, (unsigned char*)szPassword, inPasswordLen);
	inWiFi_Status();
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_WifiConnectAP Err :0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_WifiConnectAP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_WifiDisconnectAP
Date&Time       :2017/5/3 下午 5:50
Describe        :
*/
int inWiFi_WifiDisconnectAP()
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_WifiDisconnectAP() START !");
	}
	
	inRetVal = CTOS_WifiDisconnectAP();
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_WifiDisconnectAP Err :0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_WifiDisconnectAP() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_DispAP_Enter_Pin
Date&Time       :2017/5/5 下午 4:39
Describe        :
*/
int inWiFi_DispAP_Enter_Pin(CTOS_stWifiInfo* srInfo)
{
	int		inRetVal = VS_ERROR;
	int		inPasswordLen = 0;
	char		szPassword[20 + 1];
	unsigned int	uiStatus = 0;
	DISPLAY_OBJECT	srDispObj;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_DispAP_Enter_Pin() START !");
	}
	
	inDISP_ClearAll();
	inDISP_ChineseFont((char*)srInfo->ESSID, _FONTSIZE_8X16_, _LINE_8_1_, _DISP_CENTER_);
	
		
	/* 沒加密，不用輸入密碼*/
	if (memcmp(srInfo->EncryptionKey, "off", strlen("off")) == 0)
	{
		memset(szPassword, 0x00, sizeof(szPassword));
		inPasswordLen = 0;
	}
	/* 有加密 */
	else
	{
		inDISP_ChineseFont("請輸入密碼", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
		memset(&srDispObj, 0x00, sizeof(srDispObj));
		srDispObj.inY = _LINE_8_7_;
		srDispObj.inR_L = _DISP_RIGHT_;
		srDispObj.inMaxLen = 20;			/* 不可超過9，long變數最多放9位 */
		srDispObj.inColor = _COLOR_RED_;
		while (1)
		{
			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16_Character(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				return (inRetVal);
			}
			else
			{
				memset(szPassword, 0x00, sizeof(szPassword));
				memcpy(szPassword, srDispObj.szOutput, srDispObj.inOutputLen);
				inPasswordLen = srDispObj.inOutputLen;
				break;
			}
		}
	}
	
	inWiFi_WifiConnectAP(srInfo, szPassword, inPasswordLen);
	
	inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
	inDISP_PutGraphic(_CONNECTING_, 0, _COORDINATE_Y_LINE_8_7_);
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);
	
	do
	{
		inRetVal = inWiFi_GetStatus(&uiStatus);
		if (inRetVal == VS_SUCCESS)
		{
			break;
		}
		else
		{
			if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
			{
				inRetVal = VS_TIMEOUT;
				break;
			}
			else if (uiStatus & d_WIFI_STATE_AP_CONNECTING)
			{
				continue;
			}
			else
			{
				break;
			}
		}
		
	} while (1);

	if (inRetVal != VS_SUCCESS)
	{
		inDISP_ClearAll();
		inDISP_ChineseFont("連接失敗", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
	}
	else
	{
		inDISP_ClearAll();
		inDISP_ChineseFont("連接成功", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
	}
	uszKBD_GetKey(_EDC_TIMEOUT_);

	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_DispAP_Enter_Pin() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_Disp_Info
Date&Time       :2017/5/5 下午 4:00
Describe        :
*/
int inWiFi_Disp_All_WiFi_Name(int inInfoNum, CTOS_stWifiInfo *stWiFiInfo)
{
	int	i = 0;
	int	inRetVal = VS_SUCCESS;
	int	inPage = 0;
	int	inWiFi_InOnePage = 6;
	int	inRemainCount = 0;
	int	inOffset = 0;
	char	szKey = 0;
	char	szDispMsg[100 + 1];
	
	/* 沒有WiFi device資訊 */
	if (inInfoNum == 0)
	{
		inDISP_ClearAll();
		inDISP_ChineseFont("沒有WiFi資訊", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		uszKBD_GetKey(_EDC_TIMEOUT_);
		return (VS_ERROR);
	}
		
	while (1)
	{
		inDISP_ClearAll();
		
		inOffset = inPage * inWiFi_InOnePage;
		
		inRemainCount = inInfoNum - inOffset;
		if (inRemainCount > inWiFi_InOnePage)
		{
			inRemainCount = inWiFi_InOnePage;
		}
		
		for (i = 0; i < inRemainCount; i++)
		{
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			sprintf(szDispMsg, "%d. %s", i + 1, stWiFiInfo[inOffset + i].ESSID);
			inDISP_ChineseFont(szDispMsg, _FONTSIZE_16X44_, _LINE_16_1_ + i, _DISP_LEFT_);
		}
		
		while (1)
		{
			szKey = uszKBD_GetKey(_EDC_TIMEOUT_);
			
			if (szKey >= _KEY_1_ && szKey <= _KEY_6_ )
			{
				switch (szKey)
				{
					case _KEY_1_ :
						if (strlen((char*)stWiFiInfo[inOffset + 0].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 0]);
						break;
					case _KEY_2_ :
						if (strlen((char*)stWiFiInfo[inOffset + 1].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 1]);
						break;
					case _KEY_3_ :
						if (strlen((char*)stWiFiInfo[inOffset + 2].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 2]);
						break;
					case _KEY_4_ :
						if (strlen((char*)stWiFiInfo[inOffset + 3].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 3]);
						break;
					case _KEY_5_ :
						if (strlen((char*)stWiFiInfo[inOffset + 4].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 4]);
						break;
					case _KEY_6_ :
						if (strlen((char*)stWiFiInfo[inOffset + 5].ESSID) < 0)
						{
							return (VS_ERROR);
						}
						inRetVal = inWiFi_DispAP_Enter_Pin(&stWiFiInfo[inOffset + 5]);
						break;
					default:
						break;
				}
				
				if (inRetVal == VS_SUCCESS)
				{
					return (VS_SUCCESS);
				}
				else if (inRetVal == VS_USER_CANCEL)
				{
					return (VS_USER_CANCEL);
				}
				else if (inRetVal == VS_TIMEOUT)
				{
					return (VS_TIMEOUT);
				}
				else
				{
					return (VS_ERROR);
				}
				
			}
			else if (szKey == _KEY_UP_)
			{
				inPage--;
				if (inPage < 0)
				{
					inPage = 0;
				}
				break;
			}
			else if (szKey == _KEY_DOWN_)
			{
				if (inOffset + inWiFi_InOnePage >= inInfoNum)
				{
					/* 不變 */
				}
				else
				{
					inPage++;
				}
				break;
			}
			else if (szKey == _KEY_TIMEOUT_)
			{
				inRetVal = VS_TIMEOUT;
				return (inRetVal);
			}
			else if (szKey == _KEY_CANCEL_)
			{
				inRetVal = VS_USER_CANCEL;
				return (inRetVal);
			}
		}
	}
	
	return (inRetVal);
}

/*
Function        :inWiFi_ConfigGet
Date&Time       :2017/5/31 下午 1:46
Describe        :
*/
int inWiFi_ConfigGet(unsigned char uszTag, unsigned char* uszValue, unsigned char *uszLen)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
	inRetVal = CTOS_WifiConfigGet(uszTag, uszValue, uszLen);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_ConfigGet Err :0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Tag:%d, Value:%s", uszTag, uszValue);
		inLogPrintf(AT, szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ConfigSet
Date&Time       :2017/5/31 下午 1:46
Describe        :
*/
int inWiFi_ConfigSet(unsigned char uszTag, unsigned char* uszValue, unsigned char uszLen)
{
	int	inRetVal = VS_SUCCESS;
	char	szDebugMsg[100 + 1];
	
	inRetVal = CTOS_WifiConfigSet(uszTag, uszValue, uszLen);
	if (inRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inWiFi_ConfigSet Err :0x%04X", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		vdUtility_SYSFIN_LogMessage(AT, "inWiFi_ConfigSet Err :0x%04X", inRetVal);
		vdUtility_SYSFIN_LogMessage(AT, "Tag:%d, Value:(%s) Len(%d)", uszTag, uszValue, uszLen);
		return (VS_ERROR);
	}
	
	if (ginDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Tag:%d, Value:(%s) Len(%d)", uszTag, uszValue, uszLen);
		inLogPrintf(AT, szDebugMsg);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_SetConfig_Flow
Date&Time       :2017/5/31 下午 1:53
Describe        :
*/
int inWiFi_SetConfig_Flow()
{
	int		inRetVal = VS_ERROR;
	int		j = 0;
	int		inDot = 0;
	char		szDispMsg[16 + 1] = {0};
	char		szAutoConnect[10 + 1] = {};
	unsigned char	uszChange = VS_FALSE;	/* 預設沒改變，若改變EDC IP相關，bit on起來並在結尾重新initial裝置 */
	unsigned char	uszCancel = VS_FALSE;	/* 跳出Ethernet設定迴圈 */
	DISPLAY_OBJECT  srDispObj;
	
	/* Load EDC Record */
        inLoadEDCRec(0);
	
	inDISP_ClearAll();
	while(1)
	{
		/* 輸入DHCP */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("DHCP(輸入Y/N)", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetDHCP_Mode(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 1;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}

			if (strlen(srDispObj.szOutput) > 0)
			{
				if (srDispObj.szOutput[0] == 'Y'	||
				    srDispObj.szOutput[0] == 'N')
				{
					inSetDHCP_Mode(srDispObj.szOutput);
					inSaveCFGTRec(0);
					/* 有做更改 */
					uszChange = VS_TRUE;
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!!  DHCP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					continue;
				}
			}

			break;
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
		/* Auto Connect */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("Auto Connect", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("(輸入1/0)", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
		inGetAutoConnect(szAutoConnect);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		memcpy(szDispMsg, &szAutoConnect[1], 1);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 1;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16_Character_Mask(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}

			if (strlen(srDispObj.szOutput) > 0)
			{
				if (srDispObj.szOutput[0] == '1'	||
				    srDispObj.szOutput[0] == '0')
				{
					memcpy(&szAutoConnect[1], srDispObj.szOutput, 1);
					
					inSetAutoConnect(szAutoConnect);
					inSaveEDCRec(0);
					/* 有做更改 */
					uszChange = VS_TRUE;
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!! ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					continue;
				}
			}

			break;
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
		
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetDHCP_Mode(szDispMsg);
		if (memcmp(szDispMsg, "Y", strlen("Y")) == 0)
		{
			
		}
		/* 非DHCP才要設定 */
		else
		{
			/* 輸入EDC IP */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_EDIT_EDC_IP_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTermIPAddress(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}

				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTermIPAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  IP ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}

				break;
			}

			/* 跳出最大的迴圈*/
			if (uszCancel == VS_TRUE)
			{
				break;
			}

			/* 輸入EDC SUBMASK */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_EDIT_EDC_SUB_MASK_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTermMASKAddress(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTermMASKAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  Submask ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}

				break;
			}

			/* 跳出最大的迴圈*/
			if (uszCancel == VS_TRUE)
			{
				break;
			}

			/* 輸入EDC Geteway */
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_PutGraphic(_EDIT_EDC_GATEWAY_, 0, _COORDINATE_Y_LINE_8_4_);
			memset(szDispMsg, 0x00, sizeof(szDispMsg));
			inGetTermGetewayAddress(szDispMsg);
			inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

			while (1)
			{
				memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
				srDispObj.inMaxLen = 15;
				srDispObj.inY = _LINE_8_7_;
				srDispObj.inR_L = _DISP_RIGHT_;
				srDispObj.inColor = _COLOR_RED_;

				memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
				srDispObj.inOutputLen = 0;

				inRetVal = inDISP_Enter8x16_NumDot(&srDispObj);

				if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
				{
					uszCancel = VS_TRUE;
					break;
				}


				if (strlen(srDispObj.szOutput) > 0)
				{
					inDot = 0;

					for (j = 0 ;; j ++)
					{
						if (srDispObj.szOutput[j] == '.')
							inDot ++;
						else if (srDispObj.szOutput[j] == 0x00)
							break;
					}

					if (inDot == 3 && (strlen(srDispObj.szOutput) > 6))
					{
						inSetTermGetewayAddress(srDispObj.szOutput);
						inSaveEDCRec(0);
						/* 有做更改 */
						uszChange = VS_TRUE;
						break;
					}
					else
					{
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						inDISP_EnglishFont("!!  Geteway ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
						inDISP_BEEP(2, 500);
						inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
						continue;
					}
				}

				break;
			}

			/* 跳出最大的迴圈*/
			if (uszCancel == VS_TRUE)
			{
				break;
			}
		}
		
		/* 輸入EDC port */
		inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
		inDISP_ChineseFont("請輸入port", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		memset(szDispMsg, 0x00, sizeof(szDispMsg));
		inGetTermECRPort(szDispMsg);
		inDISP_EnglishFont(szDispMsg, _FONTSIZE_8X16_, _LINE_8_6_, _DISP_LEFT_);

		while (1)
		{
			memset(&srDispObj, 0x00, sizeof(DISPLAY_OBJECT));
			srDispObj.inMaxLen = 5;
			srDispObj.inY = _LINE_8_7_;
			srDispObj.inR_L = _DISP_RIGHT_;
			srDispObj.inColor = _COLOR_RED_;

			memset(srDispObj.szOutput, 0x00, sizeof(srDispObj.szOutput));
			srDispObj.inOutputLen = 0;
	
			inRetVal = inDISP_Enter8x16(&srDispObj);

			if (inRetVal == VS_TIMEOUT || inRetVal == VS_USER_CANCEL)
			{
				uszCancel = VS_TRUE;
				break;
			}


			if (strlen(srDispObj.szOutput) > 0)
			{
				if (strlen(srDispObj.szOutput) <= srDispObj.inMaxLen)
				{
					inSetTermECRPort(srDispObj.szOutput);
					inSaveEDCRec(0);
					/* 有做更改 */
					uszChange = VS_TRUE;
					break;
				}
				else
				{
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					inDISP_EnglishFont("!!  PORT ERROR  !!", _FONTSIZE_8X16_, _LINE_8_7_, _DISP_LEFT_);
					inDISP_BEEP(2, 500);
					inDISP_Clear_Line(_LINE_8_7_, _LINE_8_8_);
					continue;
				}
			}

			break;
		}
		
		/* 跳出最大的迴圈*/
		if (uszCancel == VS_TRUE)
		{
			break;
		}
	}
	
	inWiFi_AutoSetCofig();
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_AutoSetCofig
Date&Time       :2017/10/18 下午 6:05
Describe        :
*/
int inWiFi_AutoSetCofig()
{
	int	inRetVal = VS_ERROR;
	char	szAutoConnect[10 + 1] = {};
	char	szTemplate[20 + 1];
	char	szConfig[30 + 1];
	
	/* DHCP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetDHCP_Mode(szTemplate);
	memset(szConfig, 0x00, sizeof(szConfig));
	if (szTemplate[0] == 'Y')
	{
		szConfig[0] = '1';
	}
	else
	{
		szConfig[0] = '0';
	}
	
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_DHCP, (unsigned char*)szConfig, 1);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_DHCP Error");
		}
	}
	
	/* Auto Connect */
	memset(szConfig, 0x00, sizeof(szConfig));
	memset(szAutoConnect, 0x00, sizeof(szAutoConnect));
	inGetAutoConnect(szAutoConnect);
	if (memcmp(&szAutoConnect[1], "1", strlen("1")) == 0)
	{
		szConfig[0] = '1';
	}
	else
	{
		szConfig[0] = '0';
	}
	
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_AUTOCON_AP, (unsigned char*)szConfig, 1);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_AUTOCON_AP Error");
		}
	}
	
	
	/* 啟用DHCP，不用設定IP */
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetDHCP_Mode(szTemplate);
	memset(szConfig, 0x00, sizeof(szConfig));
	if (szTemplate[0] == 'Y')
	{
		/* IP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "0.0.0.0", strlen("0.0.0.0"));
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_IP, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFI_CONFIG_IP Error");
			}
		}

		/* MASK */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "0.0.0.0", strlen("0.0.0.0"));
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_MASK, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFI_CONFIG_MASK Error");
			}
		}

		/* GATEWAY */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		memcpy(szTemplate, "0.0.0.0", strlen("0.0.0.0"));
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_GATEWAY, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFI_CONFIG_GATEWAY Error");
			}
		}
	}
	else
	{
		/* IP */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTermIPAddress(szTemplate);
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_IP, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFI_CONFIG_IP Error");
			}
		}

		/* MASK */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTermMASKAddress(szTemplate);
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_MASK, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFI_CONFIG_MASK Error");
			}
		}

		/* GATEWAY */
		memset(szTemplate, 0x00, sizeof(szTemplate));
		inGetTermGetewayAddress(szTemplate);
		memset(szConfig, 0x00, sizeof(szConfig));
		strcpy(szConfig, szTemplate);

		inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_GATEWAY, (unsigned char*)szConfig, strlen(szConfig));
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFI_CONFIG_GATEWAY Error");
			}
		}
	}
	
	/* DNSIP */
	memset(szConfig, 0x00, sizeof(szConfig));
	strcpy(szConfig, "8.8.8.8");
	
	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_DNSIP, (unsigned char*)szConfig, strlen(szConfig));
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_DNSIP Error");
		}
	}
	
//	/* MAC */
//	memset(szConfig, 0x00, sizeof(szConfig));
//	strcpy(szConfig, "");
//	
//	inRetVal = inWiFi_ConfigSet(d_WIFI_CONFIG_MAC, (unsigned char*)szConfig, strlen(szConfig));
//	if (inRetVal != VS_SUCCESS)
//	{
//		if (ginDebug == VS_TRUE)
//		{
//			inLogPrintf(AT, "d_WIFI_CONFIG_MAC Error");
//		}
//	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_GetConfig_Flow
Date&Time       :2017/6/1 上午 11:19
Describe        :
*/
int inWiFi_GetConfig_Flow()
{
	int		inRetVal;
	char		szTemplate[20 + 1];
	char		szConfig[30 + 1];
	unsigned char	uszLen = 0;
	
	inDISP_ClearAll();
	
	/* DHCP */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_DHCP, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_DHCP Error");
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (szConfig[0] == '1')
		{
			sprintf(szTemplate, "DHCP: ON");
		}
		else
		{
			sprintf(szTemplate, "DHCP: OFF");
		}
			
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
	}

	/* Auto Connect */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_AUTOCON_AP, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_AUTOCON_AP Error");
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		if (szConfig[0] == '1')
		{
			sprintf(szTemplate, "AUTOCON_AP: ON");
		}
		else
		{
			sprintf(szTemplate, "AUTOCON_AP: OFF");
		}
			
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
	}
	
	/* IP */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_IP, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_IP Error");
		}
	}
	else
	{
		inDISP_ChineseFont("IP:", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	}
	
	/* MASK */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_MASK, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_MASK Error");
		}
	}
	else
	{
		inDISP_ChineseFont("MASK: ", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
	}
	
	/* GATEWAY */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_GATEWAY, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_GATEWAY Error");
		}
	}
	else
	{
		inDISP_ChineseFont("GATEWAY: ", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	}
	
	uszKBD_GetKey(_EDC_TIMEOUT_);
	inDISP_ClearAll();
	
	/* DNSIP */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_DNSIP, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_DNSIP Error");
		}
	}
	else
	{
		inDISP_ChineseFont("DNSIP: ", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
	}
	
	/* MAC */
	memset(szConfig, 0x00, sizeof(szConfig));
	uszLen = sizeof(szConfig);
	inRetVal = inWiFi_ConfigGet(d_WIFI_CONFIG_MAC, (unsigned char*)szConfig, &uszLen);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "d_WIFI_CONFIG_MAC Error");
		}
	}
	else
	{
		inDISP_ChineseFont("MAC: ", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "%s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
	}
	
	/* port */
	memset(szConfig, 0x00, sizeof(szConfig));
	inRetVal = inGetTermECRPort(szConfig);
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "TermECRPort Error");
		}
	}
	else
	{
		memset(szTemplate, 0x00, sizeof(szTemplate));
		sprintf(szTemplate, "port: %s", szConfig);
		inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
	}
	
	uszKBD_GetKey(_EDC_TIMEOUT_);
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Server_Open
Date&Time       :2017/5/3 下午 3:31
Describe        :開一個Server端的Socket
*/
int inWiFi_Server_Open()
{
	int			inHostPort = 0;
	int			inRetVal = 0;
	char			szDebugMsg[500 + 1];
	char			szECRTCPPort[4 + 1];
	struct  sockaddr_in	srServer_addr;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inWiFi_Server_Open() START !");
	}
	
	/*	AF = Address Family
	 * 	PF = Protocol Family 
	 * 	AF_INET = PF_INET
	 * 	所以，理論上建立socket時是指定協議，應該用PF_xxxx，設置地址時應該用AF_xxxx。當然AF_INET和 PF_INET的值是相同的，混用也不會有太大的問題。
	 *	INET 表示 Internet
	 * 
	*/
	memset(szECRTCPPort, 0x00, sizeof(szECRTCPPort));
	inGetTermECRPort(szECRTCPPort);
	inHostPort = atoi(szECRTCPPort);
	
	ginECR_ServerFd = socket(PF_INET, SOCK_STREAM, 0);
	if (ginECR_ServerFd == -1)
	{
		return (VS_ERROR);
	}
	else
	{
		inFile_Open_File_Cnt_Increase();
	}
	
	/* init struct dest */
	memset(&srServer_addr, 0x00, sizeof(srServer_addr));
	srServer_addr.sin_family = AF_INET;
	srServer_addr.sin_port = htons(inHostPort);		/* htons(host to network short)用來將主機的無符號短整形數轉換成網絡字節順序，因為主機是從高字節到低字節的 原因參考:http://beej-zhtw.netdpi.net/09-man-manual/9-12-htons-htonl-ntohs-ntohl */
	srServer_addr.sin_addr.s_addr = INADDR_ANY;		/* 因為是server端，位置設為空，也可寫成inet_addr("0.0.0.0) */
	
	/* Give the socket FD the local address ADDR (which is LEN bytes long).  */
	/* 網路上的Sample Code都說要轉(struct sockaddr*) */
	bind(ginECR_ServerFd, (struct sockaddr*)&srServer_addr, sizeof(srServer_addr));
	
	/* Prepare to accept connections on socket FD.
	 * N connection requests will be queued before further requests are refused.
	 * Returns 0 on success, -1 for errors.  */
	inRetVal = listen(ginECR_ServerFd, _Socket_Connections_);
	if (inRetVal == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Listen Fail");
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	inWiFi_Create_Accept_Thread();
	
//	/* Close Server 不關 */
//	close(ginECR_ServerFd);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inWiFi_Server_Open() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Create_Accept_Thread
Date&Time       :2017/6/1 下午 2:21
Describe        :
*/
int inWiFi_Create_Accept_Thread()
{
	pthread_t		uliId;
	
	pthread_create(&uliId, NULL, (void*)vdWiFi_Accept_Thread, NULL);
	
	return (VS_SUCCESS);
}

/*
Function        :vdWiFi_Accept_Thread
Date&Time       :2017/5/31 下午 6:07
Describe        :
*/
void vdWiFi_Accept_Thread()
{
	int			inAddrLen = 0;
	struct	sockaddr_in	srClient_addr;
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "vdWiFi_Accept_Thread() START !");
	}
	
	inAddrLen = sizeof(srClient_addr);
	/* Wait and Accept connection */
	ginECR_ResponseFd = accept(ginECR_ServerFd, (struct sockaddr*)&srClient_addr, (socklen_t*)&inAddrLen);
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "vdWiFi_Accept_Thread() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
}

/*
Function        :inWiFi_IsAccept
Date&Time       :2017/6/2 下午 3:08
Describe        :
*/
int inWiFi_IsAccept(void)
{
	if (ginECR_ResponseFd >= 0)
	{

		if (ginDisplayDebug == VS_TRUE)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("Debug: 接受連線", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
		}

		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}

/*
Function        :inWiFi_Close_Client
Date&Time       :2017/6/1 下午 3:09
Describe        :
*/
int inWiFi_Close_Client(void)
{
	int inRetVal = VS_SUCCESS;
	
	/* close(client) */
	inRetVal = close(ginECR_ResponseFd);
	if (inRetVal == 0)
	{
		ginECR_ResponseFd = -1;
		inFile_Open_File_Cnt_Decrease();
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ConnectedAP_InfoGet
Date&Time       :2017/6/2 下午 5:27
Describe        :
*/
int inWiFi_ConnectedAP_InfoGet(unsigned char uszTag, unsigned char* uszValue, unsigned char *uszLen)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = 0x00;
	
	if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		
	}
	else
	{
		usRetVal = CTOS_WifiConnectedAPInfoGet(uszTag, uszValue, uszLen);

		if (usRetVal != d_OK)
		{
			if (ginDebug == VS_TRUE)
			{
				memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
				sprintf(szDebugMsg, "CTOS_WifiConnectedAPInfoGet Err :0x%04X", usRetVal);
				inLogPrintf(AT, szDebugMsg);
			}
			return (VS_ERROR);
		}

		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Tag:%d, Value:%s", uszTag, uszValue);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ConnectedAP_InfoGet_Flow
Date&Time       :2017/6/2 下午 5:27
Describe        :
 *		signal和quality的差別
 *		Signal level: 
 *			raw signal level either in dBm or RSSI from the card.
 *		Link Quality: 
 *			a subjective measure of how good the link is, including signal strength, 
 *			speed the link is operating at vs. capable speed, packet loss, retries, etc.
 * 
 *		Signal level is just the raw measure of RF energy received by the radio on the card and is one component of link quality.
 * 
 *		Link Quality in correctly-written drivers should be a value in the range [0, 100] inclusive representing a percentage between “worst” link and 
 *		“best” link.
 * 
 *		Link quality in Linux wireless drivers has never been extremely reliable, though this is getting much better as time goes on. The ipw 
 *		drivers are good examples of how to do link quality correctly
*/
int inWiFi_ConnectedAP_InfoGet_Flow()
{
	int		inRetVal = VS_ERROR;
	char		szTemplate[100 + 1] = {0};
	char		szConfig[100 + 1] = {0};
	unsigned char	uszLen = 0;
	
	if (ginMachineType == _CASTLE_TYPE_V3UL_)
	{
		
	}
	else
	{
		

		inDISP_ClearAll();

		inRetVal = inWiFi_IsConected();
		if (inRetVal == VS_SUCCESS)
		{
			inDISP_ChineseFont("已連接", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_CENTER_);
		}
		else
		{
			inDISP_ChineseFont("未連接", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_CENTER_);

		}

		/* SSID */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_SSID, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFIAP_SSID Error");
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "SSID: %s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		}

		/* Quality */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_Quality, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFIAP_Quality Error");
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "Quality: %s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		}

		/* SignalLv */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_SignalLv, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFIAP_SignalLv Error");
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "SignalLv: %s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		}

		/* NoiseLv */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_NoiseLv, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFIAP_NoiseLv Error");
			}
		}
		else
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "NoiseLv: %s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		}

		/* MAC */
		memset(szConfig, 0x00, sizeof(szConfig));
		uszLen = sizeof(szConfig);
		inRetVal = inWiFi_ConnectedAP_InfoGet(d_WIFIAP_MAC, (unsigned char*)szConfig, &uszLen);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "d_WIFIAP_MAC Error");
			}
		}
		else
		{
			inDISP_ChineseFont("MAC:", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
			memset(szTemplate, 0x00, sizeof(szTemplate));
			sprintf(szTemplate, "%s", szConfig);

			inDISP_ChineseFont(szTemplate, _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		}

		uszKBD_GetKey(_EDC_TIMEOUT_);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Get_Quality
Date&Time       :2018/3/13 下午 4:44
Describe        :
*/
int inWiFi_Get_Quality(unsigned char* uszQuality)
{
	char		szDebugMsg[100 + 1] = {0};
	unsigned short	usRetVal = VS_ERROR;

	usRetVal = CTOS_WifiQualityGet(uszQuality);
	if (usRetVal == d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_WifiQualityGet() OK");
			inLogPrintf(AT, szDebugMsg);
			
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "WiFiQuality :%u", *uszQuality);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	else if (usRetVal == d_WIFI_NOT_OPEN	||
		 usRetVal == d_WIFI_NOT_SUPPORTED)
	{
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_WifiQualityGet Err :0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_Test
Date&Time       :2017/5/5 下午 12:00
Describe        :
*/
int inWiFi_Test_Menu()
{
	int		inRetVal = VS_SUCCESS;
	unsigned char	uszKey;
	
	while (1)
	{
		inDISP_ClearAll();
		inDISP_ChineseFont("1.開WiFi", _FONTSIZE_8X22_, _LINE_8_1_, _DISP_LEFT_);
		inDISP_ChineseFont("2.關WiFi", _FONTSIZE_8X22_, _LINE_8_2_, _DISP_LEFT_);
		inDISP_ChineseFont("3.Scan WiFi", _FONTSIZE_8X22_, _LINE_8_3_, _DISP_LEFT_);
		inDISP_ChineseFont("4.Choose AP", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
		inDISP_ChineseFont("5.Disconnect AP", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_LEFT_);
		inDISP_ChineseFont("6.Set Config", _FONTSIZE_8X22_, _LINE_8_6_, _DISP_LEFT_);
		inDISP_ChineseFont("7.Status", _FONTSIZE_8X22_, _LINE_8_7_, _DISP_LEFT_);
		inDISP_ChineseFont("8.Get Config", _FONTSIZE_8X22_, _LINE_8_8_, _DISP_LEFT_);
	
		uszKey = uszKBD_GetKey(_EDC_TIMEOUT_);
		
		if (uszKey == _KEY_TIMEOUT_)
		{
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (uszKey == _KEY_CANCEL_)
		{
			inRetVal = VS_USER_CANCEL;
			break;
		}
		else if (uszKey == _KEY_1_)
		{
			inRetVal = inWiFi_Open();
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("WiFi Open OK", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			else
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("WiFi Open Failed", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			uszKBD_GetKey(_EDC_TIMEOUT_);
		}
		else if (uszKey == _KEY_2_)
		{
			inRetVal = inWiFi_Close();
			if (inRetVal == VS_SUCCESS)
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("WiFi Close OK", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			else
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("WiFi Close Failed", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			uszKBD_GetKey(_EDC_TIMEOUT_);
		}
		else if (uszKey == _KEY_3_)
		{
			inDISP_ClearAll();
			inWiFi_ScanProcess();
			uszKBD_GetKey(_EDC_TIMEOUT_);
		}
		else if (uszKey == _KEY_4_)
		{
			inWiFi_AutoSetCofig();
				
			memset(&gstWiFiInfo, 0x00, sizeof(gstWiFiInfo));
			guszNum = 0;
			inWiFi_Info_Get(&guszNum, gstWiFiInfo);
			inWiFi_Disp_All_WiFi_Name((int)guszNum, gstWiFiInfo);
		}
		else if (uszKey == _KEY_5_)
		{
			inWiFi_WifiDisconnectAP();
			inRetVal = inDISP_ClearAll();
			if (inRetVal != VS_SUCCESS)
			{
				inDISP_ChineseFont("Disconnect 失敗", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			else
			{
				inDISP_ChineseFont("Disconnect 成功", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_LEFT_);
			}
			uszKBD_GetKey(_EDC_TIMEOUT_);
		}
		else if (uszKey == _KEY_6_)
		{
			inRetVal = inWiFi_SetConfig_Flow();
			if (inRetVal != VS_SUCCESS)
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("設定失敗", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			}
			else
			{
				inDISP_ClearAll();
				inDISP_ChineseFont("設定成功", _FONTSIZE_8X22_, _LINE_8_4_, _DISP_CENTER_);
			}
			
			uszKBD_GetKey(_EDC_TIMEOUT_);
			
		}
		else if (uszKey == _KEY_7_)
		{
			inWiFi_ConnectedAP_InfoGet_Flow();
		}
		else if (uszKey == _KEY_8_)
		{
			inWiFi_GetConfig_Flow();
		}
		
	}
	
	inDISP_ClearAll();
	
	
	return (inRetVal);
}

/*
Function        :inWiFi_Initial
Date&Time       :2017/6/2 上午 9:43
Describe        :WiFi開機initial
*/
int inWiFi_ECR_Initial()
{
	int	inRetVal = VS_ERROR;
	
	inRetVal = stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial(&gsrECROb);
	
	return (inRetVal);
}

/*
Function        :inWiFi_ECR_Receive_Transaction
Date&Time       :2017/5/31 下午 5:08
Describe        :
*/
int inWiFi_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 清空上次交易的資料 */
	memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
	memset(&gsrECROb.srTransDataTSB, 0x00, sizeof(ECR_TRANSACTION_DATA_TSB));
	
	/* 收資料 */	
	inRetVal = stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Receive_Transaction ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Receive_Transaction Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Receive_Transaction Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Receive_Transaction Error");
		}
		
		return (inRetVal);
	}
}
/*
Function        :inWiFi_ECR_Send_Result
Date&Time       :2017/6/1 下午 3:16
Describe        :回傳ECR
*/
int inWiFi_ECR_Send_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	/* 送資料 */	
	inRetVal = stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrSend(pobTran, &gsrECROb);
	
	if (inRetVal == VS_ERROR)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Send_Result Error");
		}
		
		
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Send_Result ok");
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Send_Error
Date&Time       :2017/6/1 下午 3:20
Describe        :
*/
int inWiFi_ECR_Send_Error(TRANSACTION_OBJECT *pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	switch (gsrECROb.srSetting.inVersion)
	{
		case	1:
			if (gsrECROb.srTransData.inErrorType  == 0)
			{
				gsrECROb.srTransData.inErrorType = inErrorType;
			}
			if (gsrECROb.srTransDataTSB.inErrorType  == 0)
			{
				gsrECROb.srTransDataTSB.inErrorType = inErrorType;
			}
			
			break;
		default:
			gsrECROb.srTransData.inErrorType = inErrorType;
			gsrECROb.srTransDataTSB.inErrorType = inErrorType;
			break;
	}
	
	
	if (stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrSendError(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Send_Mirror
Date&Time       :2017/6/1 下午 3:20
Describe        :
*/
int inWiFi_ECR_Send_Mirror(TRANSACTION_OBJECT *pobTran)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
        if (stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrMirror == NULL)
		return (VS_SUCCESS);
	
	if (stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrMirror(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Receive_EI
Date&Time       :2017/5/31 下午 5:08
Describe        :
*/
int inWiFi_ECR_Receive_EI(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal;

	/* 清空上次交易的資料 */
	memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
	memset(&gsrECROb.srTransDataTSB, 0x00, sizeof(ECR_TRANSACTION_DATA_TSB));
	
	/* 收資料 */	
	inRetVal = stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrEIRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Receive_EI ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Receive_EI Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Receive_EI Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inWiFi_ECR_Receive_EI Error");
		}
		
		return (inRetVal);
	}
}

/*
Function        :inWiFi_ECR_Send_EI
Date&Time       :2017/6/1 下午 3:20
Describe        :
*/
int inWiFi_ECR_Send_EI(TRANSACTION_OBJECT *pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	switch (gsrECROb.srSetting.inVersion)
	{
		case	1:
			if (gsrECROb.srTransData.inErrorType  == 0)
			{
				gsrECROb.srTransData.inErrorType = inErrorType;
			}
			if (gsrECROb.srTransDataTSB.inErrorType  == 0)
			{
				gsrECROb.srTransDataTSB.inErrorType = inErrorType;
			}
			
			break;
		default:
			gsrECROb.srTransData.inErrorType = inErrorType;
			gsrECROb.srTransDataTSB.inErrorType = inErrorType;
			break;
	}
	
	
	if (stWiFi_ECRTable[gsrECROb.srSetting.inVersion].inEcrEISend(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);

        return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Receive
Date&Time       :2017/5/31 下午 5:08
Describe        :
*/
int inWiFi_ECR_Receive(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		inRetVal;
	int		inTempReceiveSize = 0;
	int		inRetry = 0;				/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_WIFI_RETRYTIMES_;	/* 最大重試次數 */
	char		szReceiveBuffer[1024 + 1];
	unsigned short	usReceiveBufferSize = 0;		/* uszReceiveBuffer的長度，*/
	unsigned char	uszTempBuffer[1024 + 1];		/* 從 Buffer收到的片段 */
	
	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_WIFI_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inDISP_Timer_Start(_ECR_WIFI_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
	/* 初始化放收到資料的陣列 */
	memset(szReceiveBuffer, 0x00, sizeof(szReceiveBuffer));
	usReceiveBufferSize = 0;

	while (1)
	{
		/* 超過重試次數 */
		if (inRetry > inRecMaxRetry)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Retry too many times!");
			}
			
			return (VS_ERROR);
		}
		
		memset(uszTempBuffer, 0x00, sizeof(uszTempBuffer));
		inTempReceiveSize = 0;
		
		do
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_WIFI_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inTempReceiveSize = sizeof(uszTempBuffer);
			inETHERNET_Receive_Data_By_Native(ginECR_ResponseFd, uszTempBuffer, (unsigned short*)&inTempReceiveSize);
		} while(inTempReceiveSize <= 0);
		
		memcpy(&szReceiveBuffer[usReceiveBufferSize], uszTempBuffer, inTempReceiveSize);
		usReceiveBufferSize += inTempReceiveSize;
		
		/* 長度相符 */
		if (usReceiveBufferSize == inDataSize)
		{
			break;
		}
	}
	
	/* 把資料放到szrRealReceBuffer */
	memcpy(szDataBuffer, &szReceiveBuffer[0], usReceiveBufferSize);
	
	/* 列印紙本電文和顯示電文訊息 */
	inECR_Print_Receive_ISODeBug(szDataBuffer, usReceiveBufferSize, inDataSize);
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Receive_01_TSB
Date&Time       :2017/5/31 下午 5:08
Describe        :因為台新ECR為非固定長度，故另寫一隻
*/
int inWiFi_ECR_Receive_01_TSB(TRANSACTION_OBJECT *pobTran, ECR_TABLE* srECROb, char *szDataBuffer, int inDataSize)
{
	int		inRetVal = VS_ERROR;
	unsigned short	usReceiveBufferSize = 0;		/* uszReceiveBuffer的長度，*/
	
	/* 因台新連線規格為變動長度，故邊收邊判斷 */
	inDataSize = 0;
	
	int		inTempReceiveSize = 0;
	int		inRetry = 0;				/* 目前已重試次數 */
	int		inRecMaxRetry = _ECR_WIFI_RETRYTIMES_;	/* 最大重試次數 */
	char		szReceiveBuffer[1024 + 1];
	char		szTemplate[100 + 1];
	unsigned char	uszTempBuffer[1024 + 1];		/* 從 Buffer收到的片段 */
	
	/* 設定Timeout */
	srECROb->srSetting.inTimeout = _ECR_WIFI_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inDISP_Timer_Start(_ECR_WIFI_RECEIVE_TIMER_, srECROb->srSetting.inTimeout);
	
	/* 初始化放收到資料的陣列 */
	memset(szReceiveBuffer, 0x00, sizeof(szReceiveBuffer));
	usReceiveBufferSize = 0;

	while (1)
	{
		/* 超過重試次數 */
		if (inRetry > inRecMaxRetry)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Retry too many times!");
			}
			
			return (VS_ERROR);
		}
		
		memset(uszTempBuffer, 0x00, sizeof(uszTempBuffer));
		inTempReceiveSize = 0;
		
		do
		{
			/* 如果timeout就跳出去 */
			if (inTimerGet(_ECR_WIFI_RECEIVE_TIMER_) == VS_SUCCESS)
			{
				return (VS_TIMEOUT);
			}
			
			inTempReceiveSize = sizeof(uszTempBuffer);
			inETHERNET_Receive_Data_By_Native(ginECR_ResponseFd, uszTempBuffer, (unsigned short*)&inTempReceiveSize);
		} while(inTempReceiveSize <= 0);
		
		memcpy(&szReceiveBuffer[usReceiveBufferSize], uszTempBuffer, inTempReceiveSize);
		usReceiveBufferSize += inTempReceiveSize;
		
		/* 可以開始判斷長度 */
		if (usReceiveBufferSize >= 8	&& 
		    inDataSize == 0)
		{
			memset(szTemplate, 0x00, sizeof(szTemplate));
			memcpy(szTemplate, &szReceiveBuffer[4], 4);
			inDataSize = 8 + atoi(szTemplate) + 5;
			
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "inDataSize: %d", inDataSize);
			}
		}
		
		/* 長度符合後跳出 */
		if (inDataSize > 0	&& 
		    usReceiveBufferSize == inDataSize)
		{
			break;
		}
	}
	
	/* 把資料放到szrRealReceBuffer */
	/* 根據台新ICMP 連線規格 */
	memcpy(szDataBuffer, &szReceiveBuffer[0], usReceiveBufferSize);
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_Send
Date&Time       :2017/6/2 下午 2:14
Describe        :傳送要給收銀機的資料
*/
int inWiFi_ECR_Send(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb, char *szDataBuffer, int inDataSize)
{
	int	i = 0;
	int	inRetVal;
	int	inRetry = 0;
	int	inRecMaxRetry = _ECR_WIFI_RETRYTIMES_;					/* 最大重試次數 */
	int	inSendLen = 0;
	char	szSendBuf[_ECR_WIFI_BUFF_SIZE_];					/* 包含STX、ETX、LRC的電文 */
	
	/* 把Data Buffer(純資料)裡所有0x00值改成0x20 */
	for (i = 0; i < inDataSize; i++)
	{
		if (szDataBuffer[i] == 0x00)
		{
			szDataBuffer[i] = 0x20;
		}
	}
	
	/* 在要傳送Buffer裡放要傳送的資料 */
	memcpy(&szSendBuf[inSendLen], szDataBuffer, inDataSize);
	inSendLen += inDataSize;

	while (1)
	{

		/* 測試Flag */
		if (ginEngineerDebug == VS_TRUE)
		{
			inRetVal = VS_SUCCESS;
		}
		else
		{
			/* 經由port傳送資料 */
			inRetVal = inETHERNET_Send_Data_By_Native(ginECR_ResponseFd, (unsigned char*)szSendBuf, (unsigned short)inSendLen);
		}
		
		if (inRetVal != VS_SUCCESS)
		{
			return (VS_ERROR);
		}
		else
		{
			
			/* 傳送Retry */
			if (inRetry < inRecMaxRetry)
			{
				/* 成功 */
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "ECR ACK OK!");
				}
				return (VS_SUCCESS);
			}
			/* 超過最大重試次數，仍要完成交易，收銀機提示補登畫面 */
			else
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Exceed max retry times!");
				}
				return (VS_SUCCESS);
			}

		}/* inRS232_Data_Send */
	
	}/* while(1) */
	
}


#include <ctos_tls2.h>
int inWiFi_TLS_Test()
{
	unsigned int	uiCTX_ID;
	unsigned int	uiSSL_ID;		/* SSL Object ID */
	CTOS_TLS2_CTX_New(d_TLS2_METHOD_CLIENT_TLS_V1_2, &uiCTX_ID);
	CTOS_TLS2_CTX_LoadCACertificationFromFile(uiCTX_ID, (unsigned char*)"nccc.com.tw.pem");
	CTOS_TLS2_CTX_SetVerificationMode(uiCTX_ID, d_TLS2_VERIFY_PEER);
	CTOS_TLS2_New(uiCTX_ID, &uiSSL_ID);
	/* 設定驗證方式 */
	CTOS_TLS2_SetVerificationMode(uiSSL_ID, d_TLS2_VERIFY_NONE);
	/* 設定加密列表 */
	CTOS_TLS2_SetCipherList(uiSSL_ID, (unsigned char*)"ALL");	
	/* 設定socket */
	CTOS_TLS2_SetSocket(uiSSL_ID, ginTrans_ClientFd);	
	/* 設定協議標準 */
	CTOS_TLS2_SetProtocolVersion(uiSSL_ID, d_TLS2_VERSION_TLSV1_2);
//	CTOS_TLS2_Connect(uiSSL_ID);
//	
//	CTOS_TLS2_Disconnect(uiSSL_ID, 0);
	CTOS_TLS2_Free(uiSSL_ID);
	CTOS_TLS2_CTX_Free(uiCTX_ID);
	
	TRANSACTION_OBJECT	pobTran;
	memset(&pobTran, 0x00, sizeof(pobTran));
	
//	inCOMM_ConnectStart(&pobTran);
//	
//	inCOMM_End(&pobTran);
	
	
	return (VS_SUCCESS);
}

/*
Function        :inWiFi_ECR_8N1_Standard_Mirror_Packet
Date&Time       :2025/10/17 下午 5:14
Describe        :送給收銀機目前端末機行為
*/
int inWiFi_ECR_8N1_Standard_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_WIFI_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
	/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/9/10 上午 10:32 */
	if (gbBarCodeECRBit == VS_TRUE)
	{
		inSendSize = _ECR_8N1_1000_CUPQRCODE_Size_;
	}
	else
	{
		inSendSize = _ECR_8N1_Standard_Data_Size_;
	}
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Standard_Mirror_Pack(pobTran, srECROb, szDataBuf);

	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send_NotACK(pobTran, srECROb, szDataBuf, inSendSize);

        return (VS_SUCCESS);
}