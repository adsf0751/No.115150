#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../EVENT/Menu.h"
#include "../PRINT/Print.h"
#include "../PRINT/PrtMsg.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DisTouch.h"
#include "../EVENT/MenuMsg.h"
#include "../EVENT/Flow.h"
#include "../../NCCC/NCCCsrc.h"
#include "../../NCCC/NCCCtmk.h"
#include "../../NCCC/NCCCtSAM.h"
#include "../../IPASS/IPASSFunc.h"
#include "Sqlite.h"
#include "File.h"
#include "Function.h"
#include "FuncTable.h"
#include "CDT.h"
#include "CFGT.h"
#include "EDC.h"
#include "HDT.h"
#include "HDPT.h"
#include "SCDT.h"
#include "VWT.h"
#include "ECR.h"
#include "RS232.h"
#include "MultiFunc.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

extern int		ginDebug;
extern int		ginISODebug;
extern int		ginDisplayDebug;
extern unsigned char	guszCTLSInitiOK;
extern ECR_TABLE	gsrECROb;
extern EI_TABLE         gsrEIOb;
extern BYTE		gbBarCodeECRBit;
extern BYTE		gbEIECRBit;

ECR_TRANS_TABLE stRS232_ECRTable[] =
{
	/* 第零組標準【400】【8N1】規格 */
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Standard_Receive_Packet,
		inRS232_ECR_8N1_Standard_Send_Packet,
                inRS232_ECR_8N1_Standard_Mirror_Packet,
		inRS232_ECR_8N1_Standard_Send_Error,
                inRS232_ECR_8N1_EInvoice_Receive_Packet,
                inRS232_ECR_8N1_EInvoice_Send_Packet,    
		inRS232_ECR_8N1_Standard_Close,
	},
	
	/* 第一組標準【144】【7E1】規格 */
	{
		inRS232_ECR_7E1_Standard_Initial,
		inRS232_ECR_7E1_Standard_Receive_Packet,
		inRS232_ECR_7E1_Standard_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Standard_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第二組(107)邦柏客製化需求【400】【8N1】規格 */
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Customer_107_Bumper_Receive_Packet,
		inRS232_ECR_8N1_Customer_107_Bumper_Send_Packet,
                NULL,
		inRS232_ECR_8N1_Customer_107_Bumper_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_8N1_Standard_Close,
	},
	
	/* 第三組(111)KIOSK標準客製化需求【400】【8N1】規格*/
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet,
		inRS232_ECR_8N1_Customer_107_Bumper_Send_Packet,
                NULL,
		inRS232_ECR_8N1_Customer_107_Bumper_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_8N1_Standard_Close,
	},
	
	/* 第四組(039)【400】【8N1】規格 */
	/* 卡號遮眼前8後4 */
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Standard_Receive_Packet,
		inRS232_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet,
                NULL,     
		inRS232_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_8N1_Standard_Close,
	},
        
	/* 第五組(022)德先【144】【7E1】規格*/
	/* 卡號遮眼前8後4 */
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
        
        /* 第六組(002)耐斯廣場/王子大飯店客製化需求【247】【7E1】規格 */
	{
		inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Initial,
		inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Receive_Packet,
		inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
        
        /* 第七組(098)麥當勞客製化需求【400】【8N1】規格*/
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Customer_098_Mcdonalds_Standard_Receive_Packet,
		inRS232_ECR_8N1_Customer_098_Mcdonalds_Send_Packet,
                inRS232_ECR_8N1_Customer_098_Mcdonalds_Mirror_Packet,        
		inRS232_ECR_8N1_Customer_098_Mcdonalds_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_8N1_Standard_Close,
	},
        
        /* 第八組(034)燦坤3C電子發票客製化需求【144】【8N1】規格*/
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Receive_Packet,
		inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Send_Packet,
                NULL,        
		inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Send_Error,
                inRS232_ECR_8N1_EInvoice_Receive_Packet,
                inRS232_ECR_8N1_EInvoice_Send_Packet,     
		inRS232_ECR_8N1_Standard_Close,
	},
	
	/* 第九組(003)惠康超市客製化需求【144】【7E1】規格 */
	{
		inRS232_ECR_7E1_Standard_Initial,
		inRS232_ECR_7E1_Customer_003_WELLCOME_Receive_Packet,
		inRS232_ECR_7E1_Customer_003_WELLCOME_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Customer_003_WELLCOME_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第十組(027)中華電信客製化需求【144】【7E1】規格 */
	{
		inRS232_ECR_7E1_Standard_Initial,
		inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Receive_Packet,
		inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第十一組(033)立榮航空客製化需求【400】【8N1】規格 */
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Customer_033_UNIAIR_Receive_Packet,
		inRS232_ECR_8N1_Customer_033_UNIAIR_Send_Packet,
                NULL,
		inRS232_ECR_8N1_Customer_033_UNIAIR_Send_Error,
                inRS232_ECR_8N1_EInvoice_Receive_Packet,
                inRS232_ECR_8N1_EInvoice_Send_Packet,    
		inRS232_ECR_8N1_Standard_Close,
	},
	
	/* 第十二組(053)台中SOGO需求【144】規格 */
	{
		inRS232_ECR_7E1_Standard_Initial,
		inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Receive_Packet,
		inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第十三組(035)美麗華影城【144】【7ES1】規格 */
	{
		inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Initial,
		inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Receive_Packet,
		inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第十四組標準【144】【7E1】義大 baudrate 115200 */
	{
		inRS232_ECR_7E1_CUS_096_097_EDA_Initial,
		inRS232_ECR_7E1_CUS_096_097_EDA_Receive_Packet,
		inRS232_ECR_7E1_Standard_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Standard_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第十五組標準【400】【8N1】義大 baudrate 115200 */
	{
		inRS232_ECR_8N1_CUS_096_097_EDA_Initial,
		inRS232_ECR_8N1_CUS_096_097_EDA_Receive_Packet,
		inRS232_ECR_8N1_Standard_Send_Packet,
                NULL,
		inRS232_ECR_8N1_Standard_Send_Error,
                inRS232_ECR_8N1_EInvoice_Receive_Packet,
                inRS232_ECR_8N1_EInvoice_Send_Packet,    
		inRS232_ECR_8N1_Standard_Close,
	},
	
	/* 第十六組(079)喜滿客影城【144】【7E1】規格 */
	{
		inRS232_ECR_7E1_Standard_Initial,
		inRS232_ECR_7E1_Customer_079_CINEMARK_Receive_Packet,
		inRS232_ECR_7E1_Customer_079_CINEMARK_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Customer_079_CINEMARK_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第十七組IKEA 【144】【7E1】規格 */
	{
		inRS232_ECR_7E1_Standard_Initial,
		inRS232_ECR_7E1_Customer_082_IKEA_Receive_Packet,
		inRS232_ECR_7E1_Customer_082_IKEA_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Customer_082_IKEA_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第十八組bellavita【400】【8N1】規格 */
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Customer_038_Bellavita_Receive_Packet,
		inRS232_ECR_8N1_Standard_Send_Packet,
                NULL,
		inRS232_ECR_8N1_Standard_Send_Error,
                inRS232_ECR_8N1_EInvoice_Receive_Packet,
                inRS232_ECR_8N1_EInvoice_Send_Packet,    
		inRS232_ECR_8N1_Standard_Close,
	},
	
	/* 第十九組台塑生醫【480】【8N1】規格 */
	{
		inRS232_ECR_8N1_Customer_005_FPG_Initial,
		inRS232_ECR_8N1_Customer_005_FPG_Receive_Packet,
		inRS232_ECR_8N1_Customer_005_FPG_Send_Packet,
                NULL,
		inRS232_ECR_8N1_Customer_005_FPG_Send_Error,
                NULL,
                NULL,    
		inRS232_ECR_8N1_Standard_Close,
	},
	
	/* 第二十組 大高【144】【7ES1】規格 */
	{
		inRS232_ECR_7E1_Standard_Initial,
		inRS232_ECR_7E1_Customer_026_TAKA_Receive_Packet,
		inRS232_ECR_7E1_Customer_026_TAKA_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Customer_026_TAKA_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第二十一組 大高惠康 (Jasons)【144】【7ES1】規格 */
	{
		inRS232_ECR_7E1_Standard_Initial,
		inRS232_ECR_7E1_Customer_021_TAKAWEL_Receive_Packet,
		inRS232_ECR_7E1_Customer_021_TAKAWEL_Send_Packet,
                NULL,                        
		inRS232_ECR_7E1_Customer_021_TAKAWEL_Send_Error,
                NULL,
                NULL,        
		inRS232_ECR_7E1_Standard_Close,
	},
	
	/* 第二十二組台北101【400】【8N1】規格 */
	{
		inRS232_ECR_8N1_Standard_Initial,
		inRS232_ECR_8N1_Standard_Receive_Packet,
		inRS232_ECR_8N1_Customer_090_TAIPEI_101_Send_Packet,
                NULL,
		inRS232_ECR_8N1_Standard_Send_Error,
                inRS232_ECR_8N1_EInvoice_Receive_Packet,
                inRS232_ECR_8N1_EInvoice_Send_Packet,    
		inRS232_ECR_8N1_Standard_Close,
	},
};
ECR_TRANS_TABLE stRS232_LoadKey_ECRTable[] =
{
	/* 第零組子機接收 */
	{
		inRS232_ECR_LOAD_TMK_FROM_520_Standard_Initial,
		inRS232_ECR_LOAD_TMK_FROM_520_Standard_Receive_Packet,
		inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Packet,
                NULL,        
		inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Error,
                NULL,
                NULL,
		inRS232_ECR_LOAD_TMK_FROM_520_Standard_Close,
	},
	
};

/*
Function        :inRS232_Open
Date&Time       :2017/7/13 上午 11:54
Describe        :
*/
int inRS232_Open(unsigned char uszComport, unsigned long ulBaudRate, unsigned char uszParity, unsigned char uszDataBits, unsigned char uszStopBits)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_RS232Open(uszComport, ulBaudRate, uszParity, uszDataBits, uszStopBits);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", uszComport + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
		    inLogPrintf(AT, "CTOS_RS232Open OK");
		    memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		    sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", uszComport, ulBaudRate, uszDataBits, uszParity, uszStopBits);
		    inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inRS232_Close
Date&Time       :2017/7/13 下午 1:50
Describe        :
*/
int inRS232_Close(unsigned char uszComport)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	usRetVal = CTOS_RS232Close(uszComport);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232Close Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "COM%d", uszComport + 1);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
		    inLogPrintf(AT, "CTOS_RS232Close OK");
		    memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
		    sprintf(szDebugMsg, "COM%d", uszComport + 1);
		    inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	
}

/*
Function        :inRS232_FlushRxBuffer
Date&Time       :2016/6/21 上午 10:54
Describe        :清空buffer中的資料
*/
int inRS232_FlushRxBuffer(unsigned char uszComPort)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	/* 清空接收的buffer */
	usRetVal = CTOS_RS232FlushRxBuffer(uszComPort);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232FlushRxBuffer Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232FlushRxBuffer Successs Com%d", uszComPort + 1);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inRS232_FlushTxBuffer
Date&Time       :2017/6/26 下午 5:15
Describe        :清空Txbuffer中的資料
*/
int inRS232_FlushTxBuffer(unsigned char uszComPort)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	/* 清空接收的buffer */
	usRetVal = CTOS_RS232FlushTxBuffer(uszComPort);
	if (usRetVal != d_OK)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232FlushTxBuffer Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);
	}
	else
	{
		/* 收到的資料長度歸0 */
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232FlushTxBuffer Successs Com%d", uszComPort + 1);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_SUCCESS);
	}
	       
}

/*
Function        :inRS232_Data_Send_Check
Date&Time       :2016/6/20 下午 6:02
Describe        :檢查是否IO忙碌，可以傳送東西
*/
int inRS232_Data_Send_Check(unsigned char uszComPort)
{
	unsigned short	usRetVal;
	
	/* 檢查是否IO忙碌，可以傳送東西 */
        usRetVal = CTOS_RS232TxReady(uszComPort);
	if (usRetVal != d_OK)
	{
		/* Rx not ready 有可能沒開comport或其他原因 */
                return (VS_ERROR);         
        }
	else
		return (VS_SUCCESS);
}

/*
Function        :inRS232_Data_Send
Date&Time       :2016/7/11 下午 2:37
Describe        :RS232送東西API
*/
int inRS232_Data_Send(unsigned char uszComPort, unsigned char *uszSendBuff, unsigned short usSendSize)
{
	int	inRetVal;
	char	szDebugMsg[100 + 1];
	
	inRS232_FlushTxBuffer(uszComPort);
	
	inRetVal = CTOS_RS232TxData(uszComPort, uszSendBuff, usSendSize);
	if (inRetVal == d_OK)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232TxData Error: 0x%04x", inRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inRS232_Data_Receive_Check
Date&Time       :2016/6/20 下午 6:02
Describe        :RS232有收到東西，就觸發事件
*/
int inRS232_Data_Receive_Check(unsigned char uszComPort, unsigned short *usReceiveLen)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal;
	
	/* 檢查資料是否可以從port被取得，看是否有收到資料長度，若有資料，*usReceiveLen會被放入資料長度 */
        usRetVal = CTOS_RS232RxReady(uszComPort, usReceiveLen);
	if (usRetVal == d_OK)
	{
		
	}
	/* 接底座若離開會跑這一個 */
	else if (usRetVal == d_ETHERNET_HARDWARE_ERROR)
	{
		
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof (szDebugMsg));
			sprintf(szDebugMsg, "CTOS_RS232RxReady : 0x%04X", usRetVal);
			inLogPrintf(AT, szDebugMsg);
		}
		/* Rx not ready 有可能沒開comport或其他原因 */
                return (VS_ERROR);         
        }
	
	/* 有收到資料就回傳成功 */
	if (*usReceiveLen > 0)	
		return (VS_SUCCESS);
	else
		return (VS_ERROR);
	
}

/*
Function        :inRS232_Data_Receive
Date&Time       :2016/7/6 下午 3:28
Describe        :RS232收東西API
*/
int inRS232_Data_Receive(unsigned char uszComPort, unsigned char *uszReceBuff, unsigned short *usReceSize)
{
	int	inRetVal;
	
	inRetVal = CTOS_RS232RxData(uszComPort, uszReceBuff, usReceSize);
	if (inRetVal == d_OK)
	{
		return (VS_SUCCESS);
	}
	else
	{
		return (VS_ERROR);
	}
}



/*
Function        :inRS232_ECR_Initial
Date&Time       :2017/6/1 下午 5:46
Describe        :
*/
int inRS232_ECR_Initial(void)
{
	int	inRetVal = VS_ERROR;
	
	if (stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial != NULL)
	{
		inRetVal = stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial(&gsrECROb);
	}
	
	return (inRetVal);
}

/*
Function        :inRS232_ECR_Receive_Transaction
Date&Time       :2016/6/21 上午 11:42
Describe        :從ECR接收金額及交易別資料
*/
int inRS232_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inSize = 0;
        char	szOrgData[_ECR_BUFF_SIZE_ + 1];     /* 用來存原始資料做備份 */

        if (gsrECROb.srTransData.uszUseOrgData == VS_TRUE)
	{
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/10/12 下午 5:55 */
		if (gbBarCodeECRBit == VS_TRUE)
		{
			inSize = _ECR_8N1_1000_CUPQRCODE_Size_;
		}
		else
		{
			inSize = _ECR_8N1_Standard_Data_Size_;
		}
		
                /* 客製化098，Remove Care時，收到的ECR資料繼續使用 */
                memcpy(szOrgData, gsrECROb.srTransData.szOrgData, inSize);
                
                /* 清空上次交易的資料 */
                memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA)); 
                
                memcpy(gsrECROb.srTransData.szOrgData, szOrgData, inSize);
                gsrECROb.srTransData.uszUseOrgData = VS_TRUE;
        }
        else
        {
                /* 清空上次交易的資料 */
                memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA)); 
        }
                
	/* 收資料 */	
	inRetVal = stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Receive_Transaction ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Receive_Transaction Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Receive_Transaction Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Receive_Transaction Error");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inRS232_ECR_Send_Transaction_Result
Date&Time       :2016/7/11 下午 3:44
Describe        :印帳單前要送給ECR
*/
int inRS232_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	/* 送資料 */	
	inRetVal = stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrSend(pobTran, &gsrECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Send_Transaction_Result Error");
		}
		
		
		return (inRetVal);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Send_Transaction_Result ok");
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_SendError
Date&Time       :2016/7/18 上午 10:34
Describe        :送錯誤訊息給ECR
*/
int inRS232_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	gsrECROb.srTransData.inErrorType = inErrorType;
	
	if (stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrSendError(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_SendMirror
Date&Time       :2016/7/18 上午 10:34
Describe        :送映射訊息給ECR
*/
int inRS232_ECR_SendMirror(TRANSACTION_OBJECT * pobTran)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
        if (stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrMirror == NULL)
		return (VS_SUCCESS);
        
	if (stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrMirror(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_ReceiveEI
Date&Time       :2016/6/21 上午 11:42
Describe        :從ECR接收金額及交易別資料
*/
int inRS232_ECR_ReceiveEI(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

        /* 清空上次交易的資料 */
        memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA)); 
                
	/* 收資料 */	
	inRetVal = stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrEIRece(pobTran, &gsrECROb);
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_ReceiveEI ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_ReceiveEI Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_ReceiveEI Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_ReceiveEI Error");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inRS232_ECR_SendEI
Date&Time       :2016/7/18 上午 10:34
Describe        :送EI訊息給ECR
*/
int inRS232_ECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	gsrECROb.srTransData.inErrorType = inErrorType;
	
	if (stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrEISend(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_DeInitial
Date&Time       :2018/5/22 上午 10:14
Describe        :反初始化
*/
int inRS232_ECR_DeInitial(void)
{	
	if (stRS232_ECRTable[gsrECROb.srSetting.inVersion].inEcrEnd(&gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_NCCC_7E1_To_8N1
Date&Time       :2017/11/15 下午 4:29
Describe        :回傳VS_ERROR代表不用轉換
*/
int inRS232_ECR_NCCC_7E1_To_8N1(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer)
{
	char	szCOMPort[4 + 1];
	
	memset(szCOMPort, 0x00, sizeof(szCOMPort));
	inGetECRComPort(szCOMPort);
	/* 非RS232，不用繼續判斷 */
	if (memcmp(szCOMPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) != 0	&&
	    memcmp(szCOMPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) != 0	&&
	    memcmp(szCOMPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) != 0	&&
	    memcmp(szCOMPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) != 0)
	{
		
		return (VS_ERROR);
	}
	
	
	if (srECROb->srSetting.inVersion == 1)
	{
		/* 【需求單 - 108046】電子發票BIN大於6碼需求 標準400收銀機欄位ECR Indicator新增”E”值 by Russell 2019/7/5 下午 5:55 */
		if (uszReceiveBuffer[1] == 'I'	||
		    uszReceiveBuffer[1] == 'E')
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Receive 'I' or 'E'!");
			}
			
			inECR_DeInitial();
			
			inSetECRVersion("00");
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("7E1轉8N1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);

			inECR_Initial();

			return (VS_SUCCESS);
		}
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else
	{
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
}

/*
Function        :inRS232_ECR_NCCC_7E1_To_8N1_Cheat
Date&Time       :2018/5/22 上午 9:57
Describe        :我已經沒招了，直接作弊，parity Bit不同根本沒辦法判別，還轉個毛，直接判斷收到是0x00就轉8N1
*/
int inRS232_ECR_NCCC_7E1_To_8N1_Cheat(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer)
{
	char	szCOMPort[4 + 1];
	char	szCustomIndicator[3 + 1] = {0};
	
	memset(szCOMPort, 0x00, sizeof(szCOMPort));
	inGetECRComPort(szCOMPort);
	/* 非RS232，不用繼續判斷 */
	if (memcmp(szCOMPort, _COMPORT_COM1_, strlen(_COMPORT_COM1_)) != 0	&&
	    memcmp(szCOMPort, _COMPORT_COM2_, strlen(_COMPORT_COM2_)) != 0	&&
	    memcmp(szCOMPort, _COMPORT_COM3_, strlen(_COMPORT_COM3_)) != 0	&&
	    memcmp(szCOMPort, _COMPORT_COM4_, strlen(_COMPORT_COM4_)) != 0)
	{
		
		return (VS_ERROR);
	}
	
	memset(szCustomIndicator, 0x00, sizeof(szCustomIndicator));
	inGetCustomIndicator(szCustomIndicator);
	
	if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_STANDARD_144_))
	{
		if (uszReceiveBuffer[0] == 0x00)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Receive 'I' or 'E'!");
			}
			
			inECR_DeInitial();
			
			inSetECRVersion(_ECR_RS232_VERSION_STANDARD_400_);
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("7E1轉8N1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
			
			inECR_Initial();

			return (VS_SUCCESS);
		}
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_EDA_144_))
	{
		if (uszReceiveBuffer[0] == 0x00)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Receive 'I' or 'E' or 'Q'!");
			}
			
			inECR_DeInitial();
			
			inSetECRVersion(_ECR_RS232_VERSION_EDA_400_);
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("7E1轉8N1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
			
			inECR_Initial();

			return (VS_SUCCESS);
		}
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_026_TAKA_) ||
		 srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_021_TAKAWEL_))
	{
		if (uszReceiveBuffer[0] == 0x00)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Receive 'I' or 'E' or 'Q'!");
			}
			
			inECR_DeInitial();
			
			inSetECRVersion(_ECR_RS232_VERSION_STANDARD_400_);
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("7E1轉8N1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
			
			inECR_Initial();

			return (VS_SUCCESS);
		}
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_082_IKEA_))
	{
		/* 只有客製化123才套用 */
		if (!memcmp(szCustomIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
		{
			if (uszReceiveBuffer[0] == 0x00)
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "Receive 'I' or 'E'!");
				}

				inECR_DeInitial();

				inSetECRVersion(_ECR_RS232_VERSION_STANDARD_KIOSK_);
				inSaveEDCRec(0);

				inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
				inDISP_ChineseFont("7E1轉8N1", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);

				inECR_Initial();

				return (VS_SUCCESS);
			}
		}
		
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else
	{
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
}

/*
Function        :inRS232_ECR_Load_TMK_Initial
Date&Time       :2019/1/10 下午 2:42
Describe        :
*/
int inRS232_ECR_Load_TMK_Initial(void)
{
	int	inRetVal = VS_ERROR;
	
	if (stRS232_LoadKey_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial != NULL)
	{
		inRetVal = stRS232_LoadKey_ECRTable[gsrECROb.srSetting.inVersion].inEcrInitial(&gsrECROb);
	}

	return (inRetVal);
}

/*
Function        :inRS232_ECR_Load_TMK_From_Master
Date&Time       :2019/1/10 下午 2:43
Describe        :
*/
int inRS232_ECR_Load_TMK_From_Master(TRANSACTION_OBJECT *pobTran)
{
	int		inRetVal = VS_SUCCESS;
	unsigned short	usReceiveLen = 0;
	
	inDISP_TimeoutStart(10);
	
	while (1)
	{
		inRetVal = inRS232_Data_Receive_Check(gsrECROb.srSetting.uszComPort, &usReceiveLen);
		if (inTimerGet(_TIMER_NEXSYS_3_) == VS_SUCCESS)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_4_);
			inDISP_ChineseFont("Waiting", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
			inDISP_Timer_Start(_TIMER_NEXSYS_3_, 1);
		}
		
		if (inDISP_TimeoutCheck(_FONTSIZE_8X16_, _LINE_8_6_, _DISP_RIGHT_) == VS_TIMEOUT)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_4_);
			inDISP_ChineseFont("Timeout", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
			inRetVal = VS_TIMEOUT;
			break;
		}
		else if (usReceiveLen > 0)
		{
			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_4_);
			inDISP_ChineseFont("Processing", _FONTSIZE_8X16_, _LINE_8_4_, _DISP_CENTER_);
			inRetVal = VS_SUCCESS;
			break;
		}
		
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		/* 清空上次交易的資料 */
		memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA));
		/* 收資料 */	
		inRetVal = stRS232_LoadKey_ECRTable[gsrECROb.srSetting.inVersion].inEcrRece(pobTran, &gsrECROb);
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Load_TMK_From_Master ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Load_TMK_From_Master Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Load_TMK_From_Master Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Load_TMK_From_Master Error");
		}
		
		return (VS_ERROR);
	}  
}


/*
Function        :inRS232_ECR_Send_TMK
Date&Time       :2019/1/8 下午 5:49
Describe        :
*/
int inRS232_ECR_Send_TMK(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	/* 送資料 */	
	inRetVal = stRS232_LoadKey_ECRTable[gsrECROb.srSetting.inVersion].inEcrSend(pobTran, &gsrECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Send_TMK Error");
		}
		
		
		return (inRetVal);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_ECR_Send_TMK ok");
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_Load_TMK_DeInitial
Date&Time       :2019/1/10 下午 2:49
Describe        :反初始化
*/
int inRS232_ECR_Load_TMK_DeInitial(void)
{	
	if (stRS232_LoadKey_ECRTable[gsrECROb.srSetting.inVersion].inEcrEnd(&gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}
/*
Function        :inRS232_ECR_8N1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 8N1 COM PORT
*/
int inRS232_ECR_8N1_Standard_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1];
	char		szECRComPort[4 + 1];
	unsigned char	uszParity;
	unsigned char	uszDataBits;
	unsigned char	uszStopBits;
	unsigned long	ulBaudRate;
	unsigned short	usRetVal;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, _COMPORT_COM1_, 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, _COMPORT_COM2_, 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, _COMPORT_COM3_, 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, _COMPORT_COM4_, 4))
		srECROb->srSetting.uszComPort = d_COM4;
		
	/* BaudRate = 9600 */
	ulBaudRate = 9600;
	
	/* Parity */
	uszParity = 'N';
	
	/* Data Bits */
	uszDataBits = 8;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_Open OK");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
	}
        
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
	srECROb->srSetting.inCustomerLen = _ECR_8N1_Standard_Data_Size_;
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
        if (pobTran->uszIsTradeBit)
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_EI_TIMEOUT_;
        else
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
        
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Standard_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) == 0)
	{
		inRetVal = inECR_8N1_Inquiry_Last_Transaction_Pack(pobTran, srECROb, szDataBuf);
		
		/* (需求單-111155)-電子錢包業者收付訊息整合平台需求 by Russell 2023/8/24 下午 5:40 */
		/* "註8.	當收銀機發動’查詢前筆交易別狀態’予EDC，EDC查詢前筆交易為電子錢包(不論成功或失敗交易)，EDC須回覆’0010’予收銀機。" */
		if (szDataBuf[10] == 'W')
		{
			memcpy(&szDataBuf[76], "0010", 4);
		}
	}
	else
	{
		inRetVal = inECR_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal = VS_SUCCESS;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
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
	inRetVal = inECR_8N1_Standard_Pack_Error(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
		
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inRS232_ECR_8N1_Standard_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inRS232_Close(srECRob->srSetting.uszComPort) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 7E1 COM PORT
*/
int inRS232_ECR_7E1_Standard_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	char		szECRComPort[4 + 1] = {0};
	unsigned char	uszParity = 0;
	unsigned char	uszDataBits = 0;
	unsigned char	uszStopBits = 0;
	unsigned long	ulBaudRate = 0;
	unsigned short	usRetVal = 0;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, _COMPORT_COM1_, 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, _COMPORT_COM2_, 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, _COMPORT_COM3_, 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, _COMPORT_COM4_, 4))
		srECROb->srSetting.uszComPort = d_COM4;
	
	/* BaudRate = 9600 */
	ulBaudRate = 9600;
	
	/* Parity */
	uszParity = 'E';
	
	/* Data Bits */
	uszDataBits = 7;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_Open OK");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
	}
		
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
	srECROb->srSetting.inCustomerLen = _ECR_7E1_Standard_Data_Size_;
	
        return (VS_SUCCESS);
}


/*
Function        :inRS232_ECR_7E1_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Standard_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Standard_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Standard_Pack_Error(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_7E1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inRS232_ECR_7E1_Standard_Close(ECR_TABLE* srECROb)
{
        /*關閉port*/
        if (inRS232_Close(srECROb->srSetting.uszComPort) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Initial
Date&Time       :2016/6/20 下午 2:55
Describe        :initial 8N1 COM PORT
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	char		szECRComPort[4 + 1] = {0};
	unsigned char	uszParity = 0;
	unsigned char	uszDataBits = 0;
	unsigned char	uszStopBits = 0;
	unsigned long	ulBaudRate = 0;
	unsigned short	usRetVal = 0;
	
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, _COMPORT_COM1_, 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, _COMPORT_COM2_, 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, _COMPORT_COM3_, 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, _COMPORT_COM4_, 4))
		srECROb->srSetting.uszComPort = d_COM4;
		
	
	/* BaudRate = 19200 */
	ulBaudRate = 19200;
	
	/* Parity */
	uszParity = 'N';
	
	/* Data Bits */
	uszDataBits = 8;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_Open OK");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
	}
        
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = 20;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_LOAD_TMK_FROM_520_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_LOAD_TMK_FROM_520_Standard_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
		
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_LOAD_TMK_FROM_520_Standard_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_LOAD_TMK_FROM_520_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	/* 標示已送給ECR回覆電文 */
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_LOAD_TMK_FROM_520_Standard_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_LOAD_TMK_FROM_520_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_LOAD_TMK_FROM_520_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inRS232_ECR_LOAD_TMK_FROM_520_Standard_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inRS232_Close(srECRob->srSetting.uszComPort) != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_TSB_KIOSK_Initial
Date&Time       :2018/2/21 下午 4:38
Describe        :initial 8N1 COM PORT
*/
int inRS232_ECR_8N1_TSB_KIOSK_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	char		szECRComPort[4 + 1] = {0};
	unsigned char	uszParity = 0;
	unsigned char	uszDataBits = 0;
	unsigned char	uszStopBits = 0;
	unsigned long	ulBaudRate = 0;
	unsigned short	usRetVal = 0;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, _COMPORT_COM1_, 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, _COMPORT_COM2_, 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, _COMPORT_COM3_, 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, _COMPORT_COM4_, 4))
		srECROb->srSetting.uszComPort = d_COM4;
		
	
	/* BaudRate = 115200 */
	ulBaudRate = 115200;
	
	/* Parity */
	uszParity = 'N';
	
	/* Data Bits */
	uszDataBits = 8;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_Open OK");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
	}
        
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_TSB_KIOSK_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_TSB_KIOSK_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	char	szDebugMsg[100 + 1] = {0};
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "開始接收資料");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
	}
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			uszKBD_GetKey(30);
		}
		
		return (inRetVal);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "開始分析資料");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
	}
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_TSB_KIOSK_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		
		if (ginDisplayDebug == VS_TRUE)
		{
			uszKBD_GetKey(30);
		}
		
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_TSB_KIOSK_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_8N1_TSB_KIOSK_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_TSB_KIOSK_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	/* 標示已送給ECR回覆電文 */
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_TSB_KIOSK_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_8N1_TSB_KIOSK_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	char	szDebugMsg[100 + 1] = {0};
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Errorr 開始");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
	}
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Error Pack");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
	}
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_TSB_KIOSK_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Send Error Pack Fail");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
		}
		return (VS_ERROR);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Error PackRe");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
	}
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_TSB_KIOSK_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Send Error PackRe Fail");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
		}
		
		return (VS_ERROR);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Error Send");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_2_, VS_FALSE);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDisplayDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "Send Error Send Fail");
			inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_3_, VS_TRUE);
		}
		
		return (inRetVal);
	}
	
	if (ginDisplayDebug == VS_TRUE)
	{
		memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
		sprintf(szDebugMsg, "Send Errorr 結束");
		inDISP_LOGDisplay(szDebugMsg, _FONTSIZE_16X22_, _LINE_16_1_, VS_FALSE);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_TSB_KIOSK_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inRS232_ECR_8N1_TSB_KIOSK_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inRS232_Close(srECRob->srSetting.uszComPort) != VS_TRUE)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_107_Bumper_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Customer_107_Bumper_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_107_Bumper_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 防停車場ECR遺失問題，判斷是否票證遺失檔案 */
	if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_STANDARD_KIOSK_))
	{
		TRANSACTION_OBJECT	pobDataTran = {0};
		memset(&pobDataTran, 0x00, sizeof(pobDataTran));
		inNCCC_Func_Get_Temp_PobTran_For_ECR_Missing_Data(&pobDataTran);
		
		if (pobDataTran.srTRec.uszESVCTransBit == VS_TRUE)
		{
			memcpy(&pobTran->srTRec, &pobDataTran.srTRec, sizeof(TICKET_REC));
		}
	}
	
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
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, strlen(_ECR_8N1_SETTLEMENT_)) == 0)
	{
		inRetVal = inECR_8N1_Customer_Self_Trans_Settle_Pack(pobTran, srECROb, szDataBuf);
	}
	else if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) == 0)
	{
		inRetVal = inECR_8N1_Inquiry_Last_Transaction_Pack(pobTran, srECROb, szDataBuf);
	}
	else
	{
		inRetVal = inECR_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 存起ECR Out Data by Russell 2018/12/7 下午 2:36 */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, strlen(_ECR_8N1_START_CARD_NO_INQUIRY_)) != 0	&&
	    memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_107_Bumper_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
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
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, strlen(_ECR_8N1_SETTLEMENT_)) == 0)
	{
		inRetVal = inECR_8N1_Customer_Self_Trans_Settle_Pack(pobTran, srECROb, szDataBuf);
	}
	else
	{
		inRetVal = inECR_8N1_Standard_Pack_Error(pobTran, srECROb, szDataBuf);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Customer_107_Bumper_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 存起ECR Out Data by Russell 2018/12/7 下午 2:36 */
	inECR_Save_Response(szDataBuf, inSendSize);
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet
Date&Time       :2019/2/11 下午 6:19
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_] = {0};	/* 電文不包含STX和LRC */
	
	/* 防停車場ECR遺失問題，開始ECR前刪暫存檔 */
	if (srECROb->srSetting.inVersion == atoi(_ECR_RS232_VERSION_STANDARD_KIOSK_))
	{
		inNCCC_Func_Delete_Temp_PobTran_For_ECR_Missing_Data(pobTran);
	}
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Customer_111_Kiosk_Standard_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet
Date&Time       :2021/2/1 下午 4:26
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) == 0)
	{
		inRetVal = inECR_8N1_Inquiry_Last_Transaction_Pack(pobTran, srECROb, szDataBuf);
	}
	else
	{
		inRetVal = inECR_8N1_Customer_039_SKYKAND_Pack(pobTran, srECROb, szDataBuf);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, _ECR_8N1_Standard_Data_Size_);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error
Date&Time       :2021/2/1 下午 4:26
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));	
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Customer_039_SKYKAND_Pack_Error(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, _ECR_8N1_Standard_Data_Size_);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_098_Mcdonalds_Standard_Receive_Packet
Date&Time       :2021/6/29 下午 6:19
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_Customer_098_Mcdonalds_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSize = 0;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_] = {0};	/* 電文不包含STX和LRC */
	
	if (srECROb->srTransData.uszUseOrgData == VS_TRUE)
        {
		/* (需求單-109455)-Üny實體掃碼需求 by Russell 2021/10/12 下午 5:55 */
		if (gbBarCodeECRBit == VS_TRUE)
		{
			inSize = _ECR_8N1_1000_CUPQRCODE_Size_;
		}
		else
		{
			inSize = _ECR_8N1_Standard_Data_Size_;
		}
		
                /* 客製化098，使用暫存下來的DATA發動 */
                memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
                memcpy(szDataBuffer, srECROb->srTransData.szOrgData, inSize);
        }
        else
        {    
                memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
                inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);

                if (inRetVal != VS_SUCCESS)
                {
                        return (inRetVal);
                }
	}
        
/* -----------------------開始分析資料------------------------------------------ */
        if (pobTran->uszRemoveChecECRkBit == VS_TRUE)
        {
                /* 客製化098，Remove Card時，收到資料要判斷是否為取消交易或查詢最後一筆 */
                inRetVal = inECR_8N1_Customer_098_Mcdonalds_Remove_Card_Unpack(pobTran, srECROb, szDataBuffer);
                
                if (inRetVal != VS_SUCCESS)
                {
                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TRANS_FLOW_ERROR_;
                        inECR_SendError(pobTran, pobTran->inECRErrorMsg);
                }
                
                return (inRetVal);
        }
        else
        {
                inRetVal = inECR_8N1_Customer_098_Mcdonalds_Unpack(pobTran, srECROb, szDataBuffer);
      
                if (inRetVal != VS_SUCCESS)
                {
                        if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
                        {
                                inECR_SendError(pobTran, pobTran->inECRErrorMsg);
                        }
                        else
                        {
                                inECR_SendError(pobTran, inRetVal);
                        }
                        return (inRetVal);
                }
        }
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_098_Mcdonalds_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_8N1_Customer_098_Mcdonalds_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) == 0)
	{
		inRetVal = inECR_8N1_Inquiry_Last_Transaction_Pack(pobTran, srECROb, szDataBuf);
	}
	else
	{
		inRetVal = inECR_8N1_Customer_098_Mcdonalds_Pack(pobTran, srECROb, szDataBuf);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 存起ECR Out Data by Russell 2018/12/7 下午 2:36 */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, strlen(_ECR_8N1_START_CARD_NO_INQUIRY_)) != 0	&&
	    memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0   &&
            memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, strlen(_ECR_8N1_REPRINT_RECEIPT_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_098_Mcdonalds_Mirror_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :送給收銀機目前端末機行為
*/
int inRS232_ECR_8N1_Customer_098_Mcdonalds_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
	inRetVal = inECR_8N1_Customer_098_Mcdonalds_Mirror_Pack(pobTran, srECROb, szDataBuf);

	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send_NotACK(pobTran, srECROb, szDataBuf, inSendSize);

        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_098_Mcdonalds_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_8N1_Customer_098_Mcdonalds_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
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
	inRetVal = inECR_8N1_Customer_098_Mcdonalds_Pack_Error(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Customer_098_Mcdonalds_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 存起ECR Out Data by Russell 2018/12/7 下午 2:36 */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, strlen(_ECR_8N1_START_CARD_NO_INQUIRY_)) != 0	&&
	    memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0   &&
            memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, strlen(_ECR_8N1_REPRINT_RECEIPT_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
	}
        
        /* 只存不送0018 */
        if (pobTran->uszECCRetryBit == VS_TRUE)
                return (VS_SUCCESS);
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}


/*
Function        :inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
        if (pobTran->uszIsTradeBit)
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_EI_TIMEOUT_;
        else
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
                
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Customer_034_TK3C_EInvoice_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Customer_034_TK3C_EInvoice_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Customer_034_TK3C_EInvoice_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Customer_034_TK3C_EInvoice_Data_Size_);	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_8N1_Customer_034_TK3C_EInvoice_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Customer_034_TK3C_EInvoice_Pack_Error(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Customer_034_TK3C_EInvoice_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
/* ---------------------傳送電文--------------------------------------------- */
        if (gbEIECRBit)
                inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_100_EI_Send_Size_);
        else
                inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_Customer_034_TK3C_EInvoice_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_EInvoice_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_EInvoice_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
        char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_ECR_RECEIVE_, 0,  _COORDINATE_Y_LINE_8_6_);
        
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_EI_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Customer_034_TK3C_EInvoice_Data_Size_);
        
        if (inRetVal != VS_SUCCESS)
	{
            if (inRetVal == VS_TIMEOUT)
            {
                    inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
                    inDISP_ChineseFont_Color("接受資料逾時", _FONTSIZE_8X16_, _LINE_8_6_, _COLOR_BLACK_, _DISP_CENTER_);
                    inDISP_Wait(2000);
            }
		
            return (VS_ERROR);
	}
        
/* -----------------------開始分析資料------------------------------------------ */
        srECROb->srTransData.uszIsResponse = VS_FALSE;
	inRetVal = inECR_EI_Unpack(pobTran, szDataBuffer);
	
	inECR_SendEI(pobTran, pobTran->inECRErrorMsg);
        
        if (inRetVal == VS_ERROR)
        {
                inDISP_EI_Msg_BMP(pobTran->inECRErrorMsg);
                gsrEIOb.inCurrentPacket = 0;
        }
        
        return (inRetVal);
}

/*
Function        :inRS232_ECR_8N1_EInvoice_Send_Packet
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送EI訊息ECR
*/
int inRS232_ECR_8N1_EInvoice_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */

	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inECR_8N1_EInvoice_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
        
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
        inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_8N1_100_EI_Send_Size_);
        
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

int inECR_EI_Unpack(TRANSACTION_OBJECT *pobTran, char *szDataBuffer)
{
        int	inRetVal = VS_ESCAPE;
        int     inTotalPactket = 0, inCurrentPactket = 0;
        long    lnTotalSize = 0, lnCurrentSize = 0;
        char    szTemp[50 + 1];
        
        /* Indicator */
        if (szDataBuffer[0] != 'I')
        {
                /* 0001 */
                pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_INDICATOR_ERROR_;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " Header is not I");
		}
                return (VS_ERROR);
        }
        
        /* 回覆用 */
        memset(gsrEIOb.szHeader, 0x00, sizeof(gsrEIOb.szHeader));
        memcpy(&gsrEIOb.szHeader[0], &szDataBuffer[0], 23);
        
        /* 時間 */
        memset(szTemp, 0x00, sizeof(szTemp));
        memcpy(&szTemp[0], &szDataBuffer[10], 9);
        
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "EDC Time = %s", gsrEIOb.szPrintTime);
		inLogPrintf(AT, "ECR Time = %s", szTemp);
	}
        
        if (!memcmp(gsrEIOb.szPrintTime, szTemp, 9))
        {
                /* 0007 */
                pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_PACKET_SAME_ERROR_;
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " Time is same");
		}
                
                return (VS_ERROR);
        }
        else
        {
                memset(gsrEIOb.szPrintTime, 0x00, sizeof(gsrEIOb.szPrintTime));
                memcpy(&gsrEIOb.szPrintTime[0], &szTemp[0], 9);
        }    
        
        /* 封包數 */
        memset(szTemp, 0x00, sizeof(szTemp));
        memcpy(&szTemp[0], &szDataBuffer[35], 5);
        lnTotalSize = atol(szTemp);
        memset(szTemp, 0x00, sizeof(szTemp));
        memcpy(&szTemp[0], &szDataBuffer[40], 4);
        lnCurrentSize = atol(szTemp);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "EI Size  [%ld / %ld]", lnTotalSize, lnCurrentSize);
	}
        
        /* 封包Index */
        memset(szTemp, 0x00, sizeof(szTemp));
        memcpy(&szTemp[0], &szDataBuffer[19], 2);
        inTotalPactket = atoi(szTemp);
        memset(szTemp, 0x00, sizeof(szTemp));
        memcpy(&szTemp[0], &szDataBuffer[21], 2);
        inCurrentPactket = atoi(szTemp);
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "EI Count [%02d / %02d]", inTotalPactket, inCurrentPactket);
	}
       
        /* 起頭取得總封包數 */
        if (inCurrentPactket == 1)
        {
                /* 改寫，以封包數來malloc，不驗長度直接跑分析 */
                if (inTotalPactket >= inCurrentPactket)
                {
                        /* 連續收到inCurrentPactket == 1 的封包 */
                        if (gsrEIOb.uszCreatReadData == VS_FALSE)
                        {
                                gsrEIOb.szReadData = malloc(inTotalPactket * 2000 + 1);
                                gsrEIOb.uszCreatReadData = VS_TRUE;
                        }
                        else
                        {
                                /* 0010 */
                                pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_CURRENT_PACKET_ERROR_;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, " Double First Pactket");
				}
                                return (VS_ERROR);
                        }
                        
                        gsrEIOb.inTotalPacket = inTotalPactket;
                        gsrEIOb.lnTotalSize = lnTotalSize;
                        gsrEIOb.inCurrentPacket = inCurrentPactket;
                        gsrEIOb.lnDataSize = 0;
                        
                        /* 直接抓2000 */
                        memset(gsrEIOb.szReadData, 0x00, sizeof(gsrEIOb.szReadData));
                        memcpy(&gsrEIOb.szReadData[0], &szDataBuffer[44], 2000);
                        gsrEIOb.lnDataSize += 2000;
                }
                else
                {
                        /* 0001 */
                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_TOTAL_PACKET_ERROR_;
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, " Total Pactket is error");
			}
                        return (VS_ERROR);
                }    
        }
        else
        {
                if (gsrEIOb.uszCreatReadData == VS_FALSE)
                {
                        /* 0010 */
                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_CURRENT_PACKET_ERROR_;
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, " No Creat Buffer");
			}
                        return (VS_ERROR);
                }
                
                if (inCurrentPactket != gsrEIOb.inCurrentPacket + 1)
                {
                        /* 0010 */
                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_CURRENT_PACKET_ERROR_; 
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, " Pactket is miss");
			}
                        return (VS_ERROR);
                }    
                else
                {
                        gsrEIOb.inCurrentPacket = inCurrentPactket;
                        
                        /* 防止溢位 */
                        if (gsrEIOb.lnDataSize + lnCurrentSize > inTotalPactket * 2000)
                        {
                                /* 0011 */
                                pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_PACKET_SIZE_ERROR_;
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, " Pactket size is not match");
				}
                                return (VS_ERROR);
                        }   
                        else
                        {    
                                /* 直接抓2000 */
                                memcpy(&gsrEIOb.szReadData[gsrEIOb.lnDataSize], &szDataBuffer[44], 2000);
                                gsrEIOb.lnDataSize += 2000;
                        }
                }
        }
                
        if (gsrEIOb.inTotalPacket == gsrEIOb.inCurrentPacket)
        {
                /* 不驗長度，毛太多 */
//                if (gsrEIOb.lnDataSize != gsrEIOb.lnTotalSize)
//                {
//                        /* 0011 */
//                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_PACKET_SIZE_ERROR_;    
//                        inLogPrintf(AT, " Pactket size is not match2");
//                        return (VS_ERROR);
//                }
//            
//                memset(szTemp, 0x00, sizeof(szTemp));
//                memcpy(&szTemp[0], &gsrEIOb.szReadData[gsrEIOb.lnDataSize - 4], 4);
//                
//                if (memcmp(szTemp, "[$$]", 4) && memcmp(szTemp, "[##]", 4))
//                {
//                        /* 0001 */
//                        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_END_ERROR_;    
//                        inLogPrintf(AT, " [$$] Miss");
//                        return (VS_ERROR);
//                }
            
                inRetVal = inECR_EI_Unpack_Data(gsrEIOb.inTotalPacket * 2000, gsrEIOb.szReadData);
                
                if (inRetVal != VS_SUCCESS)
                {
                        pobTran->inECRErrorMsg = inRetVal;   
                        return (VS_ERROR);
                }
                
                return (VS_ESCAPE);
        }
        
        pobTran->inECRErrorMsg = _ECR_RESPONSE_CODE_NOT_SET_ERROR_;
        return (VS_SUCCESS);
}

int inECR_EI_Unpack_Data(long lnSize, char *szData)
{
        int     i = 0, j = 0, inCnt = 0, index = 0, inLen = 0, inLine = 0, inFont = _PRT_SH_;
        int     inX = 0, inY = 0, inZ = 0;
	int	inStart = 0, inEnd = 0;		/* 用來標示沒有換行字元的開頭和結尾 */
        char    szTemp[50 + 1], szType[20 + 1], szSub[2048 + 1], szBig5[1024 + 1], szPrint[1024 + 1];
        char    szQR1[256 + 1], szQR2[256 + 1];
        char	szPrtMerchantLogo[1 + 1];
	char	szBig5Temp[1024 + 1] = {0};	/* 用來暫放去除換行字元的data */
        unsigned char	uszBig5[10];
        unsigned char	uszEndBit = VS_FALSE;
        unsigned char	uszStringBit = VS_FALSE;
        unsigned char	uszCheckBit = VS_FALSE;
        unsigned char	uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
        BufferHandle	srBhandle;
	FONT_ATTRIB	srFont_Attrib;
        
        /* 先檢核 */
        for (i = 0; i < lnSize; i++) 
        {
                memset(szTemp, 0x00, sizeof(szTemp));
                memcpy(&szTemp[0], &szData[i], 4);
                
                if (!memcmp(szTemp, "[@@]", 4))
                {
                        inCnt++;
                        i+=3;
                }
                
                if (!memcmp(szTemp, "[##]", 4))
                {
                        inCnt--;
                        i+=3;
                }
                
                if (!memcmp(szTemp, "[$$]", 4))
                {
                        uszEndBit = VS_TRUE;
                }
        }
        
        if (!uszEndBit)
        {
                /* 0001 */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " No [$$]");
		}
                return (_ECR_RESPONSE_CODE_END_ERROR_);
        }
        
        if (inCnt != 0)
        {
                /* 0011 */
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, " [@@] [##] Not Match");
		}
                return (_ECR_RESPONSE_CODE_TEXT_SIZE_ERROR_);
        }
        
        inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);
               
        inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
        inDISP_PutGraphic(_PRINTING_, 0,  _COORDINATE_Y_LINE_8_6_);
        
        for (i = 0; i < lnSize; i++) 
        {
                memset(szTemp, 0x00, sizeof(szTemp));
                memcpy(&szTemp[0], &szData[i], 4);
                   
                if (!memcmp(szTemp, "[@@]", 4))
                {
                        memset(szSub, 0x00, sizeof(szSub));
                        index = 0;
                        uszStringBit = VS_TRUE;
                        i+=3;
                }
                else if (!memcmp(szTemp, "[##]", 4))
                {
                        /* 檢核用 */
                        memcpy(&szSub[index], &szData[i], 4);
                        
                        /* MLOGO沒長度 */
                        memset(szType, 0x00, sizeof(szType));
                        memcpy(&szType[0], &szSub[0], 9);
                                                
                        if (!memcmp(szType, _PRT_EI_ML_, 9))
                        {
                                memset(szPrtMerchantLogo, 0x00, sizeof(szPrtMerchantLogo));
                                inGetPrtMerchantLogo(szPrtMerchantLogo);
                                if(memcmp(szPrtMerchantLogo, "N", 1) == 0)
                                {

                                }
                                else
                                {
                                        inPRINT_PutGraphic((unsigned char *)_MERCHANT_LOGO_);
                                }
                        }
                        else  
                        {
                                memset(szTemp, 0x00, sizeof(szTemp));
                                memcpy(&szTemp[0], &szSub[10], 4);
                                inLen = atoi(szTemp);

                                if (szSub[index] != '[')    /* [XXXX-XX][XXXX] */
                                {
                                        /* 0011 */
					if (ginDebug == VS_TRUE)
					{
						inLogPrintf(AT, " Text size error");
					}
                                        
                                        if (!memcmp(szType, _PRT_EI_SH_, 9) || !memcmp(szType, _PRT_EI_H_, 9) || !memcmp(szType, _PRT_EI_B_, 9) || !memcmp(szType, _PRT_EI_S_, 9))
                                        {
                                                return (_ECR_RESPONSE_CODE_TEXT_SIZE_ERROR_);
                                        }
                                        else if (!memcmp(szType, _PRT_EI_QR_, 9))
                                        {
                                                return (_ECR_RESPONSE_CODE_QR_SIZE_ERROR_);
                                        }
                                        else if (!memcmp(szType, _PRT_EI_39C_, 9))
                                        {
                                                return (_ECR_RESPONSE_CODE_39_SIZE_ERROR_);
                                        }
                                }
                                else
                                {
                                        memset(szBig5, 0x00, sizeof(szBig5));
                                        memcpy(&szBig5[0], &szSub[15], inLen);
                                }
                                
                                if (!memcmp(szType, _PRT_EI_SH_, 9) || !memcmp(szType, _PRT_EI_H_, 9) || !memcmp(szType, _PRT_EI_B_, 9) || !memcmp(szType, _PRT_EI_S_, 9))
                                {
                                        if (!memcmp(szType, _PRT_EI_SH_, 9))
                                        {
                                                inLine = 16;
                                                inFont = _PRT_SH_;
                                        }        
                                        else if (!memcmp(szType, _PRT_EI_H_, 9))
                                        {
                                                inLine = 24;
                                                inFont = _PRT_H_;
                                        }
                                        else if (!memcmp(szType, _PRT_EI_B_, 9))
                                        {
                                                inLine = 42;
                                                inFont = _PRT_B_;
                                        }
                                        else if (!memcmp(szType, _PRT_EI_S_, 9))
                                        {
                                                inLine = 42;
                                                inFont = _PRT_S_;
                                        }
                                        
                                        /* 開頭結尾要去掉換行字元 */
					/* 循環去開頭換行字元 */
					inStart = 0;
					inEnd = inLen -1;;
					do
					{
						if (szBig5[inStart] == 0x0D && szBig5[inStart + 1] == 0x0A)
						{
							inStart += 2;
						}
						else
						{
							break;
						}
					}while(inStart < inEnd);
					
					
					/* 循環去結尾換行字元 */
					do
					{
						if (szBig5[inEnd - 1] == 0x0D && szBig5[inEnd] == 0x0A)
						{
							inEnd -= 2;
						}
						else
						{
							break;
						}
					}while(inStart < inEnd);
					
					inLen = inEnd - inStart + 1;
					memset(szBig5Temp, 0x00, sizeof(szBig5Temp));
					memcpy(szBig5Temp, &szBig5[inStart], inLen);
					memset(szBig5, 0x00, sizeof(szBig5));
					memcpy(szBig5, &szBig5Temp[0], inLen);
                                        
                                        inX = 0;
                                        inY = 0;
                                        memset(szSub, 0x00, sizeof(szSub));

                                        while(1)
                                        {
                                                uszCheckBit = VS_FALSE;

                                                /* 是否中文字 */
                                                memset(uszBig5, 0x00, sizeof(uszBig5));
                                                memcpy(&uszBig5[0], (unsigned char*)&szBig5[inY], 2);

						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, "[%02x][%02x]", uszBig5[0], uszBig5[1]);
						}

                                                if (uszBig5[0] == 0x0D && uszBig5[1] == 0x0A)
                                                {
                                                        memset(szPrint, 0x00, sizeof(szPrint));
                                                        inFunc_Big5toUTF8(szPrint, szSub);
                                                        inPRINT_Buffer_PutIn(szPrint, inFont, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        inX = 0;
                                                        memset(szSub, 0x00, sizeof(szSub));
                                                        inY += 2;
                                                        continue;
                                                }

						/* 確認是否為Big5 */
                                                if ((uszBig5[0] >= 0x81) && (uszBig5[0] <= 0xFE))
                                                {
                                                        if (uszBig5[1] >= 0x40 && uszBig5[1] <= 0x7E)
                                                        {
                                                                uszCheckBit = VS_TRUE;
                                                        }    

                                                        if (uszBig5[1] >= 0xA1 && uszBig5[1] <= 0xFE)
                                                        {
                                                                uszCheckBit = VS_TRUE;
                                                        }  
                                                } 

						/* 是big5抓兩個 */
                                                if (uszCheckBit)
                                                {
                                                        inZ = 2;
                                                }
                                                else
                                                {
                                                        inZ = 1;
                                                }    

						/* inX:此行已累積長度 inZ:此次增加長度 inY:已讀取長度 */
                                                if (inX + inZ <= inLine)
                                                {
                                                        /* 塞值 */
                                                        memcpy(&szSub[inX], &szBig5[inY], inZ);
                                                        inX += inZ;
                                                        inY += inZ;
                                                }
                                                else
                                                {
                                                        /* 塞不下，列印 */
                                                        memset(szPrint, 0x00, sizeof(szPrint));
                                                        inFunc_Big5toUTF8(szPrint, szSub);
                                                        inPRINT_Buffer_PutIn(szPrint, inFont, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        inX = 0;
                                                        memset(szSub, 0x00, sizeof(szSub));
                                                }    

                                                if (inY >= inLen)
                                                {
                                                        /* 到底，列印 */
                                                        memset(szPrint, 0x00, sizeof(szPrint));
                                                        inFunc_Big5toUTF8(szPrint, szSub);
                                                        inPRINT_Buffer_PutIn(szPrint, inFont, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                                        break;
                                                }
                                        }  
                                }
                                else if (!memcmp(szType, _PRT_EI_QR_, 9))
                                {
                                        uszCheckBit = VS_FALSE;
                                        
                                        for (j = 0; j < inLen - 1; j++) 
                                        {
                                                 memset(szTemp, 0x00, sizeof(szTemp));
                                                 memcpy(&szTemp[0], &szBig5[j], 2);
                                                 
                                                 if (!memcmp(szTemp, "^^", 2))
                                                 {
                                                        if (j > 128) 
                                                        {
                                                                /* 0001 */
								if (ginDebug == VS_TRUE)
								{
									inLogPrintf(AT, " QR1 > 128");
								}
                                                                return (_ECR_RESPONSE_CODE_QR_DATA_ERROR_);
                                                        }
                                                        
                                                        memset(szSub, 0x00, sizeof(szSub));
                                                        memcpy(&szSub[0], &szBig5[0], j);
                                                        memset(szQR1, 0x00, sizeof(szQR1));
                                                        strcpy(szQR1, szSub);
                                                        
                                                        if (inLen - j - 2 > 128) 
                                                        {
                                                                /* 0001 */
								if (ginDebug == VS_TRUE)
								{
									inLogPrintf(AT, " QR2 > 128");
								}
                                                                return (_ECR_RESPONSE_CODE_QR_DATA_ERROR_);
                                                        }
                                                        
                                                        memset(szSub, 0x00, sizeof(szSub));
                                                        memcpy(&szSub[0], &szBig5[j + 2], inLen - j - 2);
                                                        memset(szQR2, 0x00, sizeof(szQR2));
                                                        strcpy(szQR2, szSub);
                                                        inPRINT_Buffer_TwoQR(szQR1, szQR2, uszBuffer, &srBhandle);
                                                        uszCheckBit = VS_TRUE;
                                                        break;     
                                                 }
                                        }
                                        
                                        if (!uszCheckBit)
                                        {
                                                /* 0001 */
						if (ginDebug == VS_TRUE)
						{
							inLogPrintf(AT, " QRCode Not ^^");
						}
                                                return (_ECR_RESPONSE_CODE_QR_DATA_ERROR_);
                                        }
                                }
                                else if (!memcmp(szType, _PRT_EI_39C_, 9))
                                {
                                        for (j = 0; j < inLen; j++) 
                                        {
                                                memset(uszBig5, 0x00, sizeof(uszBig5));
                                                memcpy(&uszBig5[0], (unsigned char*)&szBig5[j], 1);

                                                if( (uszBig5[0] >= '0') && (uszBig5[0] <= '9') )
                                                        continue;
                                                else if( (uszBig5[0] >= 'A') && (uszBig5[0] <= 'Z') )
                                                        continue;
                                                else if (uszBig5[0] == '-' || uszBig5[0] == '+' || uszBig5[0] == ' ' || uszBig5[0] == '$' ||
                                                         uszBig5[0] == '/' || uszBig5[0] == '.' || uszBig5[0] == '%' || uszBig5[0] == '*')
                                                        continue;
                                                else
                                                {
                                                        /* 0001 */
							if (ginDebug == VS_TRUE)
							{
								inLogPrintf(AT, " 39Code not allow");
							}
                                                        return (_ECR_RESPONSE_CODE_39_DATA_ERROR_);
                                                }
                                        }
                                        
                                        inPRINT_Buffer_Barcode(szBig5, uszBuffer, &srBhandle, 0, VS_FALSE, _PRINT_BARCODE_TYPE_DEFAULT_, _PRINT_BARCODE_X_EXTEND_DEFAULT_, _PRINT_BARCODE_Y_EXTEND_EI_);
                                        inPRINT_Buffer_PutIn("", _PRT_S_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                                }
                        }
                        
                        uszStringBit = VS_FALSE;
                        i+=3;
                }
                else if (!memcmp(szTemp, "[$$]", 4))
                {     
                        for (j = 0; j < 8; j++)
                        {
                                inPRINT_Buffer_PutIn("", _PRT_HEIGHT_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
                        }
                        
                        inPRINT_Buffer_OutPut(uszBuffer, &srBhandle);
                        return (VS_SUCCESS);  
                }
                else
                {
                        if (!memcmp(szTemp, "\x0d\x0a", 2))
                        {
                                /* [##]與[@@]中間換行為無意義資料 */
                                if (uszStringBit == VS_FALSE)
                                {    
                                        i++;
                                        continue;
                                }
                        }    

                        memcpy(&szSub[index], &szData[i], 1);
                        index++;
                }
        }
        
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Initial
Date&Time       :2022/7/15 下午 12:03
Describe        :initial 7E1 COM PORT
*/
int inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	char		szECRComPort[4 + 1] = {0};
	unsigned char	uszParity = 0;
	unsigned char	uszDataBits = 0;
	unsigned char	uszStopBits = 0;
	unsigned long	ulBaudRate = 0;
	unsigned short	usRetVal = 0;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, _COMPORT_COM1_, 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, _COMPORT_COM2_, 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, _COMPORT_COM3_, 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, _COMPORT_COM4_, 4))
		srECROb->srSetting.uszComPort = d_COM4;
		
	
	/* BaudRate = 9600 */
	ulBaudRate = 9600;
	
	/* Parity */
	uszParity = 'E';
	
	/* Data Bits */
	uszDataBits = 7;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_Open OK");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
	}
		
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
	srECROb->srSetting.inCustomerLen = _ECR_7E1_CUSTOMER_002_NICE_PLAZA_Data_Size_;
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Receive_Packet
Date&Time       :2021/1/11 下午 5:31
Describe        :處理收銀機傳來的資料(長度247)
*/
int inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_CUSTOMER_002_NICE_PLAZA_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Customer_002_NICE_PLAZA_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Send_Packet
Date&Time       :2021/1/12 下午 4:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_002_NICE_PLAZA_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_CUSTOMER_002_NICE_PLAZA_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Send_Error
Date&Time       :2021/1/12 下午 4:31
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Customer_002_NICE_PLAZA_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_002_NICE_PLAZA_Pack_Error(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_7E1_Customer_002_NICE_PLAZA_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_CUSTOMER_002_NICE_PLAZA_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_003_WELLCOME_Receive_Packet
Date&Time       :2016/7/6 下午 4:04
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Customer_003_WELLCOME_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Customer_003_WELLCOME_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_003_WELLCOME_Send_Packet
Date&Time       :2021/1/12 下午 4:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Customer_003_WELLCOME_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_003_WELLCOME_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SETTLEMENT_, strlen(_ECR_7E1_SETTLEMENT_)))
	{
                srECROb->srTransData.uszIsResponse = VS_TRUE;
                return (VS_SUCCESS);
        }
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_003_WELLCOME_Send_Error
Date&Time       :2021/1/12 下午 4:31
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Customer_003_WELLCOME_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	/* 客製化003，結束卡號查詢，只回傳回應碼，不回傳其他結果 */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_END_CARD_NO_INQUIRY_, 2) == 0)
	{
		
	}
	else
	{
		inRetVal = inECR_7E1_Customer_003_WELLCOME_Pack_Error(pobTran, szDataBuf, srECROb);
		
	
		if (inRetVal == VS_ERROR)
		{
			return (VS_ERROR);
		}
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_003_WELLCOME_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_END_CARD_NO_INQUIRY_, strlen(_ECR_7E1_END_CARD_NO_INQUIRY_)))
	{
                srECROb->srTransData.uszIsResponse = VS_TRUE;
                return (VS_SUCCESS);
        }
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Receive_Packet
Date&Time       :2022/7/11 下午 2:38
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Customer_027_CHUNGHWA_TELECOM_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Send_Packet
Date&Time       :2022/7/11 下午 2:39
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_027_CHUNGHWA_TELECOM_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SETTLEMENT_, strlen(_ECR_7E1_SETTLEMENT_)))
	{
                srECROb->srTransData.uszIsResponse = VS_TRUE;
                return (VS_SUCCESS);
        }
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Send_Error
Date&Time       :2022/7/11 下午 2:39
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Customer_027_CHUNGHWA_TELECOM_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	/* 客製化003，結束卡號查詢，只回傳回應碼，不回傳其他結果 */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_7E1_END_CARD_NO_INQUIRY_, 2) == 0)
	{
		
	}
	else
	{
		inRetVal = inECR_7E1_Customer_027_CHUNGHWA_TELECOM_Pack_Error(pobTran, szDataBuf, srECROb);
		
	
		if (inRetVal == VS_ERROR)
		{
			return (VS_ERROR);
		}
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_027_CHUNGHWA_TELECOM_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_END_CARD_NO_INQUIRY_, strlen(_ECR_7E1_END_CARD_NO_INQUIRY_)))
	{
                srECROb->srTransData.uszIsResponse = VS_TRUE;
                return (VS_SUCCESS);
        }
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_033_UNIAIR_Receive_Packet
Date&Time       :2022/7/22 下午 7:18
Describe        :033客製化，不支援部份較新功能
*/
int inRS232_ECR_8N1_Customer_033_UNIAIR_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
        if (pobTran->uszIsTradeBit)
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_EI_TIMEOUT_;
        else
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
        
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Customer_033_UNIAIR_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_033_UNIAIR_Send_Packet
Date&Time       :2022/7/11 下午 4:48
Describe        :033客製化，兩段式第二段時不接收對方ACK
*/
int inRS232_ECR_8N1_Customer_033_UNIAIR_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) == 0)
	{
		inRetVal = inECR_8N1_Inquiry_Last_Transaction_Pack(pobTran, srECROb, szDataBuf);
	}
	else
	{
		inRetVal = inECR_8N1_Customer_033_UNIAIR_Pack(pobTran, srECROb, szDataBuf);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	/* 立榮航空第二段ECR連線做法：當EDC回傳卡號給ECR後，EDC忽略ACK判斷 */
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		srECROb->srOptionalSetting.uszNotReceiveAck = VS_TRUE;
	}
	
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	if (pobTran->uszCardInquiryFirstBit == VS_TRUE)
	{
		srECROb->srOptionalSetting.uszNotReceiveAck = VS_FALSE;
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_033_UNIAIR_Send_Error
Date&Time       :2022/7/22 下午 7:52
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_8N1_Customer_033_UNIAIR_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
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
	inRetVal = inECR_8N1_Standard_Pack_Error(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Customer_033_UNIAIR_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
		
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Receive_Packet
Date&Time       :2022/7/28 下午 4:22
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Customer_053_TAICHUNG_SOGO_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Send_Packet
Date&Time       :2022/7/12 下午 2:08
Describe        :
*/
int inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_053_TAICHUNG_SOGO_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Send_Error
Date&Time       :2022/8/17 下午 5:46
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Customer_053_TAICHUNG_SOGO_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_053_TAICHUNG_SOGO_Pack_Error(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_7E1_Standard_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Initial
Date&Time       :2022/7/15 上午 11:15
Describe        :retry 由3次改為4次
*/
int inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	char		szECRComPort[4 + 1] = {0};
	unsigned char	uszParity = 0;
	unsigned char	uszDataBits = 0;
	unsigned char	uszStopBits = 0;
	unsigned long	ulBaudRate = 0;
	unsigned short	usRetVal = 0;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, _COMPORT_COM1_, 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, _COMPORT_COM2_, 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, _COMPORT_COM3_, 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, _COMPORT_COM4_, 4))
		srECROb->srSetting.uszComPort = d_COM4;
		
	
	/* BaudRate = 9600 */
	ulBaudRate = 9600;
	
	/* Parity */
	uszParity = 'E';
	
	/* Data Bits */
	uszDataBits = 7;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_Open OK");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
	}
		
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
	srECROb->srSetting.inCustomerLen = _ECR_7E1_Standard_Data_Size_;
	
	/* 美麗華客製化重試由3次改為4次 */
	srECROb->srSetting.uszNonDefaultRetry = VS_TRUE;
	srECROb->srSetting.inNonDefaultRetryTimes = 4;
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Receive_Packet
Date&Time       :2022/7/14 下午 4:45
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Send_Packet
Date&Time       :2016/7/11 下午 3:29
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Customer_035_MIRAMAR_CINEMAS_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack_Error(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_7E1_Customer_035_MIRAMAR_CINEMAS_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_CUS_096_097_EDA_Initial
Date&Time       :2022/7/21 上午 10:28
Describe        :initial 7E1 COM PORT 115200
*/
int inRS232_ECR_7E1_CUS_096_097_EDA_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1] = {0};
	char		szECRComPort[4 + 1] = {0};
	unsigned char	uszParity = 0;
	unsigned char	uszDataBits = 0;
	unsigned char	uszStopBits = 0;
	unsigned long	ulBaudRate = 0;
	unsigned short	usRetVal = 0;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, _COMPORT_COM1_, 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, _COMPORT_COM2_, 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, _COMPORT_COM3_, 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, _COMPORT_COM4_, 4))
		srECROb->srSetting.uszComPort = d_COM4;
	
	/* BaudRate = 115200 */
	ulBaudRate = 115200;
	
	/* Parity */
	uszParity = 'E';
	
	/* Data Bits */
	uszDataBits = 7;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_Open OK");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
	}
		
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
	srECROb->srSetting.inCustomerLen = _ECR_7E1_Standard_Data_Size_;
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_CUS_096_097_EDA_Initial
Date&Time       :2022/7/21 上午 10:31
Describe        :initial 8N1 COM PORT 115200
*/
int inRS232_ECR_8N1_CUS_096_097_EDA_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1];
	char		szECRComPort[4 + 1];
	unsigned char	uszParity;
	unsigned char	uszDataBits;
	unsigned char	uszStopBits;
	unsigned long	ulBaudRate;
	unsigned short	usRetVal;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, _COMPORT_COM1_, 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, _COMPORT_COM2_, 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, _COMPORT_COM3_, 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, _COMPORT_COM4_, 4))
		srECROb->srSetting.uszComPort = d_COM4;
		
	/* BaudRate = 115200 */
	ulBaudRate = 115200;
	
	/* Parity */
	uszParity = 'N';
	
	/* Data Bits */
	uszDataBits = 8;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_Open OK");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
	}
	
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
	srECROb->srSetting.inCustomerLen = _ECR_8N1_Standard_Data_Size_;
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_CUS_096_097_EDA_Receive_Packet
Date&Time       :2022/8/1 下午 5:16
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_CUS_096_097_EDA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
        if (pobTran->uszIsTradeBit)
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_EI_TIMEOUT_;
	else if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_096_EDA_, _CUSTOMER_INDICATOR_SIZE_))
		srECROb->srSetting.inTimeout = _ECR_RECEIVE_CUS_096_IPASS_DIRECT_TIMEOUT_;
        else
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
        
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_096_EDA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = inECR_Receive_Cus_096(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	}
	else
	{
		inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	if (pobTran->uszCus096_iPASS_Direct_Bit == VS_TRUE)
	{
		inRetVal = inECR_Cus_096_Ipass_Parse_Data(pobTran, srECROb, szDataBuffer);
	}
	else
	{
		inRetVal = inECR_8N1_Standard_Unpack(pobTran, srECROb, szDataBuffer);
		
		if (inRetVal != VS_SUCCESS)
		{
			if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
			{
				inECR_SendError(pobTran, pobTran->inECRErrorMsg);
			}
			else
			{
				inECR_SendError(pobTran, inRetVal);
			}
			return (inRetVal);
		}
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_CUS_096_097_EDA_Receive_Packet
Date&Time       :2022/8/9 下午 4:01
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_CUS_096_097_EDA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_096_EDA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = inECR_Receive_Cus_096(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	}
	else
	{
		inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	if (pobTran->uszCus096_iPASS_Direct_Bit == VS_TRUE)
	{
		inRetVal = inECR_Cus_096_Ipass_Parse_Data(pobTran, srECROb, szDataBuffer);
	}
	else
	{
		inRetVal = inECR_7E1_Standard_Unpack(pobTran, szDataBuffer, srECROb);

		if (inRetVal != VS_SUCCESS)
		{
			if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
			{
				inECR_SendError(pobTran, pobTran->inECRErrorMsg);
			}
			else
			{
				inECR_SendError(pobTran, inRetVal);
			}
			return (inRetVal);
		}
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_Cus_096_Ipass_Trasaction_Transaction
Date&Time       :2022/8/12 下午 2:32
Describe        :處理ipass命令流程
*/
int inRS232_ECR_Cus_096_Ipass_Trasaction_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_SUCCESS;
	char	szInDataBuf[_ECR_IPASS_Receive_Size_ + 1] = {0};
	char	szOutDataBuf[_ECR_IPASS_Receive_Size_ + 1] = {0};
	
	inNCCC_tSAM_SelectAID_IPASS_Flow();

	/* 處理第一次的命令和回傳 */
	memset(szOutDataBuf, 0x00, sizeof(szOutDataBuf));
	inIPASS_Cus_096_Do_Cmd(pobTran, &gsrECROb);
	iniPASS_PackResult_Cus096_Internal(szOutDataBuf, &gsrECROb);
	
	/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, &gsrECROb, szOutDataBuf, _ECR_IPASS_Receive_Size_);	
	/* 標示已送給ECR回覆電文 */
	gsrECROb.srTransData.uszIsResponse = VS_TRUE;
	
	/* 處理2...n次的命令和回傳，直到收到"52"結束交易別 */
	do
	{
		memset(szInDataBuf, 0x00, sizeof(szInDataBuf));
		inRetVal = inECR_Receive_Cus_096(pobTran, &gsrECROb, szInDataBuf, _ECR_IPASS_Receive_Size_);
		if (inRetVal != VS_SUCCESS)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "ECR_IPASS Receive fail");
			}
			
			memset(szOutDataBuf, 0x00, sizeof(szOutDataBuf));
			iniPASS_PackResult_Cus096_Internal(szOutDataBuf, &gsrECROb);
			/* ---------------------傳送電文--------------------------------------------- */
			inECR_Send(pobTran, &gsrECROb, szOutDataBuf, _ECR_IPASS_Receive_Size_);
			
			
			break;
		}
		else
		{
			/* 先處理命令 */
			memset(szOutDataBuf, 0x00, sizeof(szOutDataBuf));
			inIPASS_Cus_096_Do_Cmd(pobTran, &gsrECROb);
			iniPASS_PackResult_Cus096_Internal(szOutDataBuf, &gsrECROb);
			/* ---------------------傳送電文--------------------------------------------- */
			inRetVal = inECR_Send(pobTran, &gsrECROb, szOutDataBuf, _ECR_IPASS_Receive_Size_);
			
			/* 如果結束就跳出 */
			if (!memcmp(gsrECROb.srTransData.srCus096_Data.szTransType, _S_END_, 2))
			{
				if (ginDebug == VS_TRUE)
				{
					inLogPrintf(AT, "ECR_IPASS Receive End Transaction");
				}
				break;
			}
		}
	}while(1);
	
	/* 切回NCCC */
	inNCCC_tSAM_SelectAID_NCCC_Flow();
	
	return (inRetVal);
}

/*
Function        :inRS232_ECR_7E1_Customer_079_CINEMARK_Receive_Packet
Date&Time       :2022/8/23 下午 6:18
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Customer_079_CINEMARK_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_SUCCESS;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_] = {0};	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_CUSTOMER_079_CINEMARK_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Customer_079_CINEMARK_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_079_CINEMARK_Send_Packet
Date&Time       :2022/8/23 下午 6:21
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Customer_079_CINEMARK_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_SUCCESS;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_079_CINEMARK_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, srECROb->srTransData.inSendPacketSize);	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_079_CINEMARK_Send_Error
Date&Time       :2022/8/29 下午 1:57
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Customer_079_CINEMARK_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal = VS_SUCCESS;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_079_CINEMARK_Pack_Error(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_7E1_Customer_079_CINEMARK_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, srECROb->srTransData.inSendPacketSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_082_IKEA_Receive_Packet
Date&Time       :2022/9/8 上午 11:55
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Customer_082_IKEA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Customer_082_IKEA_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_082_IKEA_Send_Packet
Date&Time       :2022/9/8 下午 5:59
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Customer_082_IKEA_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_082_IKEA_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_082_IKEA_Send_Error
Date&Time       :2022/9/8 下午 6:42
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Customer_082_IKEA_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal = VS_SUCCESS;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	char	szCustomerIndicator[3 + 1] = {0};
	
	memset(szCustomerIndicator, 0x00, sizeof(szCustomerIndicator));
	inGetCustomIndicator(szCustomerIndicator);
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	/* 客製化123，不回傳除回應碼外的資料 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = VS_SUCCESS;
	}
	else
	{
		inRetVal = inECR_7E1_Customer_082_IKEA_Pack_Error(pobTran, szDataBuf, srECROb);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_7E1_Customer_082_IKEA_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_038_Bellavita_Receive_Packet
Date&Time       :2022/9/13 下午 4:27
Describe        :和標準400一樣，但是不支援兩段式
*/
int inRS232_ECR_8N1_Customer_038_Bellavita_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
        if (pobTran->uszIsTradeBit)
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_EI_TIMEOUT_;
        else
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
        
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Customer_038_Bellavita_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_005_FPG_Initial
Date&Time       :2022/9/22 下午 4:17
Describe        :
*/
int inRS232_ECR_8N1_Customer_005_FPG_Initial(ECR_TABLE *srECROb)
{
	char		szDebugMsg[100 + 1];
	char		szECRComPort[4 + 1];
	unsigned char	uszParity;
	unsigned char	uszDataBits;
	unsigned char	uszStopBits;
	unsigned long	ulBaudRate;
	unsigned short	usRetVal;
	
	memset(&uszParity, 0x00, sizeof(uszParity));
	memset(&uszDataBits, 0x00, sizeof(uszDataBits));
	memset(&uszStopBits, 0x00, sizeof(uszStopBits));
	memset(&ulBaudRate, 0x00, sizeof(ulBaudRate));
	
	/* 從EDC.Dat抓出哪一個Comport */
	/* inGetECRComPort */
	memset(&szECRComPort, 0x00, sizeof(szECRComPort));
	inGetECRComPort(szECRComPort);
	/* Verifone用handle紀錄，Castle用Port紀錄 */
	if (!memcmp(szECRComPort, _COMPORT_COM1_, 4))
		srECROb->srSetting.uszComPort = d_COM1;
	else if (!memcmp(szECRComPort, _COMPORT_COM2_, 4))
		srECROb->srSetting.uszComPort = d_COM2;
	if (!memcmp(szECRComPort, _COMPORT_COM3_, 4))
		srECROb->srSetting.uszComPort = d_COM3;
	if (!memcmp(szECRComPort, _COMPORT_COM4_, 4))
		srECROb->srSetting.uszComPort = d_COM4;
		
	/* BaudRate = 9600 */
	ulBaudRate = 9600;
	
	/* Parity */
	uszParity = 'N';
	
	/* Data Bits */
	uszDataBits = 8;
	
	/* Stop Bits */
	uszStopBits = 1;
	
	
        /* 開port */
	/* Portable 機型若沒接上底座再開Ethernet會失敗 */
	usRetVal = inRS232_Open(srECROb->srSetting.uszComPort, ulBaudRate, uszParity, uszDataBits, uszStopBits);

	if (usRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "inRS232_Open Error: 0x%04x", usRetVal);
			inLogPrintf(AT, szDebugMsg);
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort + 1, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
		return (VS_ERROR);         
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inRS232_Open OK");
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "COM%d BaudRate:%lu %d%c%d", srECROb->srSetting.uszComPort, ulBaudRate, uszDataBits, uszParity, uszStopBits);
			inLogPrintf(AT, szDebugMsg);
		}
	}
        
	/* 清空接收的buffer */
        inRS232_FlushRxBuffer(srECROb->srSetting.uszComPort);
	
	srECROb->srSetting.inCustomerLen = _ECR_8N1_CUSTOMER_005_FPG_Data_Size_;
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_005_FPG_Receive_Packet
Date&Time       :2022/9/22 下午 4:22
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_8N1_Customer_005_FPG_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
        if (pobTran->uszIsTradeBit)
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_EI_TIMEOUT_;
        else
                srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
        
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_8N1_CUSTOMER_005_FPG_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_8N1_Customer_005_FPG_Unpack(pobTran, srECROb, szDataBuffer);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_005_FPG_Send_Packet
Date&Time       :2022/10/3 上午 10:46
Describe        :
*/
int inRS232_ECR_8N1_Customer_005_FPG_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
	inSendSize = _ECR_8N1_CUSTOMER_005_FPG_Data_Size_;
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Customer_005_FPG_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_Customer_005_FPG_Send_Error
Date&Time       :2022/10/4 下午 5:03
Describe        :
*/
int inRS232_ECR_8N1_Customer_005_FPG_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
	inSendSize = _ECR_8N1_CUSTOMER_005_FPG_Data_Size_;
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_8N1_Customer_005_FPG_Pack(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_8N1_Customer_005_FPG_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
		
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_026_TAKA_Receive_Packet
Date&Time       :2022/10/11 下午 8:13
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Customer_026_TAKA_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Customer_026_TAKA_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_026_TAKA_Send_Packet
Date&Time       :2022/10/12 上午 9:58
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Customer_026_TAKA_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_026_TAKA_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);	
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_026_TAKA_Send_Error
Date&Time       :2022/10/12 上午 11:55
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Customer_026_TAKA_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal = VS_SUCCESS;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_026_TAKA_Pack_Error(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inECR_7E1_Customer_026_TAKA_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_021_TAKAWEL_Receive_Packet
Date&Time       :2022/10/18 下午 5:30
Describe        :處理收銀機傳來的資料
*/
int inRS232_ECR_7E1_Customer_021_TAKAWEL_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_RS232_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
/* -----------------------開始分析資料------------------------------------------ */
	inRetVal = inECR_7E1_Customer_021_TAKAWEL_Unpack(pobTran, szDataBuffer, srECROb);
	
	if (inRetVal != VS_SUCCESS)
	{
		if (pobTran->inECRErrorMsg != _ECR_RESPONSE_CODE_NOT_SET_ERROR_)
		{
			inECR_SendError(pobTran, pobTran->inECRErrorMsg);
		}
		else
		{
			inECR_SendError(pobTran, inRetVal);
		}
		return (inRetVal);
	}
	
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_021_TAKAWEL_Send_Packet
Date&Time       :2022/10/18 下午 5:30
Describe        :處理要送給收銀機的資料
*/
int inRS232_ECR_7E1_Customer_021_TAKAWEL_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_021_TAKAWEL_Pack(pobTran, szDataBuf, srECROb);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        if (!memcmp(srECROb->srTransData.szTransType, _ECR_7E1_SETTLEMENT_, strlen(_ECR_7E1_SETTLEMENT_)))
	{
                srECROb->srTransData.uszIsResponse = VS_TRUE;
                return (VS_SUCCESS);
        }
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_7E1_Customer_021_TAKAWEL_Send_Error
Date&Time       :2021/1/12 下午 4:31
Describe        :傳送錯誤訊息ECR
*/
int inRS232_ECR_7E1_Customer_021_TAKAWEL_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_];	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 初始化 */
	memset(szDataBuf, 0x00, sizeof(szDataBuf));
/* ---------------------包裝電文--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_021_TAKAWEL_Pack_Error(pobTran, szDataBuf, srECROb);


	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	inRetVal = inECR_7E1_Customer_021_TAKAWEL_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, _ECR_7E1_Standard_Data_Size_);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Customer_090_TAIPEI_101_Send_Packet
Date&Time       :2022/12/6 下午 6:14
Describe        :
 * 針對客製化【090】台北101收銀機調整如下:
 * ECR Indicator =‘E’與該交易為信用卡交易(非電子票證交易)時，
 * 調整電文欄位格式第37欄位為「Card No. Hash Value」，
 * 預設值為’B00999’+ 44碼HASH值，
 * 以電文回覆內容金融機構代碼’B00xxx’+ 44碼HASH值為主。
*/
int inRS232_ECR_8N1_Customer_090_TAIPEI_101_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) == 0)
	{
		inRetVal = inECR_8N1_Inquiry_Last_Transaction_Pack(pobTran, srECROb, szDataBuf);
	}
	else
	{
		inRetVal = inECR_8N1_Customer_090_TAIPEI_101_Pack(pobTran, srECROb, szDataBuf);
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
	}
/* ---------------------傳送電文--------------------------------------------- */
	inRetVal = inECR_Send(pobTran, srECROb, szDataBuf, inSendSize);
	/* 標示已送給ECR回覆電文 */
	srECROb->srTransData.uszIsResponse = VS_TRUE;
	
	if (inRetVal != VS_SUCCESS)
	{
		return (inRetVal);
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inRS232_ECR_8N1_Standard_Mirror_Packet
Date&Time       :2025/10/17 下午 5:14
Describe        :送給收銀機目前端末機行為
*/
int inRS232_ECR_8N1_Standard_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_RS232_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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