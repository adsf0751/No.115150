#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Define_2.h"
#include "../INCLUDES/TransType.h"
#include "../INCLUDES/Transaction.h"
#include "../INCLUDES/AllStruct.h"
#include "../DISPLAY/Display.h"
#include "../DISPLAY/DispMsg.h"
#include "../FUNCTION/ECR.h"
#include "../FUNCTION/EDC.h"
#include "../FUNCTION/Utility.h"
#include "../FUNCTION/CFGT.h"
#include "../FUNCTION/File.h"
#include "../../NCCC/NCCCsrc.h"
#include "UDP.h"

extern	int		ginDebug;
extern	int		ginDisplayDebug;
extern	int		ginEngineerDebug;
extern	int		ginMachineType;
extern	ECR_TABLE	gsrECROb;
extern	int		ginECR_ServerFd;		/* File Descripter */
extern	int		ginECR_ResponseFd;
extern	BYTE		gbECR_UDP_Buffer[_ECR_UDP_BUFF_SIZE_];
extern	UINT		guiECR_UDP_BufferLen;
extern	BYTE		gbBarCodeECRBit;		/* 是否為掃碼交易規格 */
extern	BYTE		gbECR_UDP_TransBit;
struct sockaddr_in	gsrUDP_SockHost_SendRece;
socklen_t  gsocketSize = sizeof (gsrUDP_SockHost_SendRece);

ECR_TRANS_TABLE stUDP_ECRTable[] =
{	
	/* 第零組標準【400】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Standard_Receive_Packet,
		inUDP_ECR_8N1_Standard_Send_Packet,
                inUDP_ECR_8N1_Standard_Mirror_Packet,
		inUDP_ECR_8N1_Standard_Send_Error,
                NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第一組標準【144】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,                        
		NULL,
		NULL,
		NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第二組(107)邦柏客製化需求【400】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Customer_107_Bumper_Receive_Packet,
		inUDP_ECR_8N1_Customer_107_Bumper_Send_Packet,
                NULL,
		inUDP_ECR_8N1_Customer_107_Bumper_Send_Error,
		NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第三組(111)KIOSK標準客製化需求【400】規格*/
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet,
		inUDP_ECR_8N1_Customer_107_Bumper_Send_Packet,
                NULL,
		inUDP_ECR_8N1_Customer_107_Bumper_Send_Error,
		NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第四組(039)【400】規格 */
	/* 卡號遮眼前8後4 */
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Standard_Receive_Packet,
		inUDP_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet,
                NULL,     
		inUDP_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error,
		NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第五組(022)德先【144】規格*/
	/* 卡號改遮掩前8後4 */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第六組(002)耐斯廣場 & 王子飯店【247】規格*/
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
        
	/* 第七組(098)麥當勞客製化需求【400】規格，不支援*/
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
		NULL,        
                NULL,
                NULL,
		NULL,
		inUDP_ECR_400_Standard_Close,
	},

	/* 第八組(034)燦坤3C電子發票客製化需求【144】規格，不支援*/
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,
                NULL,
                NULL,
		NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第九組(003)惠康超市客製化需求【144】規格 */
	/* 不支援UDP */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,                        
		NULL,
                NULL,
                NULL,        
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十組(027)中華電信客製化需求【144】規格 */
	/* 不支援UDP */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,                        
		NULL,
                NULL,
                NULL,        
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十一組(033)立榮航空客製化需求【400】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Standard_Receive_Packet,
		inUDP_ECR_8N1_Customer_033_UNIAIR_Send_Packet,
                NULL,
		inUDP_ECR_8N1_Standard_Send_Error,
                NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十二組(053)台中SOGO客製化需求【144】規格 */
	/* 不支援UDP */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,                        
		NULL,
                NULL,
                NULL,        
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十三組(035)美麗華影城【144】規格 */
	/* 不支援UDP */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,                        
		NULL,
                NULL,
                NULL,        
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十四組標準【144】義大 baudrate 115200 */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,                        
		NULL,
		NULL,
		NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十五組標準【400】義大 baudrate 115200 */
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Standard_Receive_Packet,
		inUDP_ECR_8N1_Standard_Send_Packet,
                NULL,
		inUDP_ECR_8N1_Standard_Send_Error,
                NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十六組喜滿客影城【144】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,                        
		NULL,
		NULL,
		NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十七組IKEA 【144】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,                        
		NULL,
                NULL,
                NULL,        
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十八組bellavita【400】【8N1】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Standard_Receive_Packet,
		inUDP_ECR_8N1_Standard_Send_Packet,
                NULL,
		inUDP_ECR_8N1_Standard_Send_Error,
                NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第十九組台塑生醫【480】【8N1】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		NULL,
		NULL,
                NULL,                        
		NULL,
                NULL,
                NULL,        
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第二十組 大高【144】【7ES1】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Standard_Receive_Packet,
		inUDP_ECR_8N1_Standard_Send_Packet,
                NULL,
		inUDP_ECR_8N1_Standard_Send_Error,
                NULL,
                NULL,    
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第二十一組 大高惠康 (Jasons)【144】【7ES1】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Standard_Receive_Packet,
		inUDP_ECR_8N1_Standard_Send_Packet,
                NULL,
		inUDP_ECR_8N1_Standard_Send_Error,
                NULL,
                NULL,    
		inUDP_ECR_400_Standard_Close,
	},
	
	/* 第二十二組 台北101【400】規格 */
	{
		inUDP_ECR_400_Standard_Initial,
		inUDP_ECR_8N1_Standard_Receive_Packet,
		inUDP_ECR_8N1_Customer_090_TAIPEI_101_Send_Packet,
                NULL,
		inUDP_ECR_8N1_Standard_Send_Error,
                NULL,
                NULL,
		inUDP_ECR_400_Standard_Close,
	},
};

/*
Function        :inUDP_Server_Open
Date&Time       :2021/12/1 下午 4:29
Describe        :開一個Server端的Socket
*/
int inUDP_Server_Open()
{
	int			inHostPort = 0;
	int			inOption = 0;
	int			inFlags = 0;
	char			szECR_UDP_Port[5 + 1] = {0};
	struct  sockaddr_in	srServer_addr = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inUDP_Server_Open() START !");
	}
	
	/*	AF = Address Family
	 * 	PF = Protocol Family 
	 * 	AF_INET = PF_INET
	 * 	所以，理論上建立socket時是指定協議，應該用PF_xxxx，設置地址時應該用AF_xxxx。當然AF_INET和 PF_INET的值是相同的，混用也不會有太大的問題。
	 *	INET 表示 Internet
	 * 
	*/
	memset(szECR_UDP_Port, 0x00, sizeof(szECR_UDP_Port));
	inGetUDP_Port(szECR_UDP_Port);
	inHostPort = atoi(szECR_UDP_Port);
	
	ginECR_ServerFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (ginECR_ServerFd == -1)
	{
		/* [BUG] 移除檔案同步API SYNC()指令
		* [20260506] 新增關閉失敗LOG */
		vdUtility_SYSFIN_LogMessage(AT,"UDP Create Socket Fail errno[%d]", errno);
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
	
	/* 開關 ON */
	inOption = 1;
	/* 允許重用本地位址和埠 */
	setsockopt(ginECR_ServerFd, SOL_SOCKET, SO_REUSEADDR, &inOption, sizeof(inOption));

	/* Linux内核中对connect的超时时间限制是75s， Soliris 9是几分钟，因此通常认为是75s到几分钟不等*/
	/* 師爺給我翻譯翻譯：不用非阻塞Timeut就是75秒，所以一定要用非阻塞 */
	/* 先獲得現在Handle的開關狀態 */
	inFlags = fcntl(ginECR_ServerFd, F_GETFL, 0);
	/* 把非阻塞開關On起來(做or運算)，並設定回去 */
	fcntl(ginECR_ServerFd, F_SETFL, inFlags | O_NONBLOCK);
	
	/* Give the socket FD the local address ADDR (which is LEN bytes long).  */
	/* 網路上的Sample Code都說要轉(struct sockaddr*) */
	bind(ginECR_ServerFd, (struct sockaddr*)&srServer_addr, sizeof(srServer_addr));
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inUDP_Server_Open() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUDP_Server_Close
Date&Time       :2021/12/1 下午 4:29
Describe        :
*/
int inUDP_Server_Close()
{	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inUDP_Server_Close() START !");
	}
	
	int inRetVal = VS_SUCCESS;
	
	/* Close Server 不關 */
	if (ginECR_ServerFd > 0)
	{
		inRetVal = close(ginECR_ServerFd);
		if (inRetVal == 0)
		{
			ginECR_ServerFd = -1;
			inFile_Open_File_Cnt_Decrease();
		}else
		{
			/* [BUG] 移除檔案同步API SYNC()指令
			 * [20260506] 新增關閉失敗LOG */
			vdUtility_SYSFIN_LogMessage(AT,"UDP Close Fail ServerFd[%d] inRetVal[%d] error[%d]", ginECR_ServerFd, inRetVal, errno);
		}
	}
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inUDP_Server_Close() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUDP_FlushRxBuffer
Date&Time       :2021/12/6 下午 4:52
Describe        :清空buffer中的資料
*/
int inUDP_FlushRxBuffer(void)
{
	unsigned short	usReceLen = 0;
	unsigned char	uszData[_ECR_UDP_BUFF_SIZE_] = {0};
	
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	inDISP_Timer_Start(_TIMER_NEXSYS_1_, 10);
	do
	{
		/* 清空接收的buffer */
		usReceLen = 0;
		inUDP_Data_Receive_Check(&usReceLen);
		if (usReceLen == 0)
		{
			break;
		}
		/* 保險起見，最多十秒跳出 */
		else if (inTimerGet(_TIMER_NEXSYS_1_) == VS_SUCCESS)
		{
			break;
		}
		
		usReceLen = sizeof(uszData);
		inUDP_Data_Receive(uszData, &usReceLen);
	}while (1);
	
	return (VS_SUCCESS);   
}

/*
Function        :inUDP_FlushTxBuffer
Date&Time       :2017/6/26 下午 5:15
Describe        :清空Txbuffer中的資料
*/
int inUDP_FlushTxBuffer(void)
{
	/* 沒設定完成，不用檢查 */
	if (gsrECROb.srSetting.uszSettingOK != VS_TRUE)
	{
		return (VS_ERROR);
	}
	
	return (VS_SUCCESS);
	       
}

/*
Function        :inUDP_Data_Send_Check
Date&Time       :2021/12/7 上午 10:08
Describe        :檢查是否IO忙碌，可以傳送東西
*/
int inUDP_Data_Send_Check(void)
{
	return (VS_SUCCESS);
}

/*
Function        :inUDP_Data_Send
Date&Time       :2021/12/7 上午 10:10
Describe        :UDP送東西API
*/
int inUDP_Data_Send(unsigned char *uszSendBuff, unsigned short usSendSize)
{
	int	inTempLen = 0;
	char	szDebugMsg[100 + 1];
	
	inUDP_FlushTxBuffer();
	
	/* Send message */  
	inTempLen = sendto(ginECR_ServerFd, uszSendBuff, usSendSize, 0, (struct sockaddr *)&gsrUDP_SockHost_SendRece, gsocketSize);
	
	if (inTempLen >= 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			memset(szDebugMsg, 0x00, sizeof(szDebugMsg));
			sprintf(szDebugMsg, "send Errno: %d", errno);
			inLogPrintf(AT, szDebugMsg);
		}
		inUtility_StoreTraceLog_OneStep("inUDP_Data_Send Fail, Errno: %d", errno);
		
		return (VS_ERROR);
	}
}

/*
Function        :inUDP_Data_Receive_Check
Date&Time       :2021/12/6 下午 3:27
Describe        :
*/
int inUDP_Data_Receive_Check(unsigned short *usReceiveLen)
{
	int	inTempLen = 0;
	
	/* 清空才繼續收 */
	if (guiECR_UDP_BufferLen == 0)
	{
		inTempLen = recvfrom(ginECR_ServerFd, gbECR_UDP_Buffer, sizeof(gbECR_UDP_Buffer), 0, (struct sockaddr *)&gsrUDP_SockHost_SendRece, &gsocketSize);
		
		/* 如果回傳-1會變成65535，所以要做安全檢核 */
		if (inTempLen > 0)
		{
			guiECR_UDP_BufferLen = (unsigned short)inTempLen;

		}
		else
		{
			guiECR_UDP_BufferLen = 0;
		}
	}
	
	/* 收完之後 */
	if (guiECR_UDP_BufferLen > 0)
	{
		*usReceiveLen = guiECR_UDP_BufferLen;
		return (VS_SUCCESS);
	}
	else
	{
		*usReceiveLen = 0;
		return (VS_ERROR);
	}
}

/*
Function        :inUDP_Data_Receive
Date&Time       :2021/12/6 下午 3:32
Describe        :
*/
int inUDP_Data_Receive(unsigned char* uszData, unsigned short *usLen)
{
	int	inRemainLen = 0;
	BYTE	bTempBuffer[_ECR_UDP_BUFF_SIZE_] = {0};
	
	if (*usLen >= guiECR_UDP_BufferLen)
	{
		/* 抓要的資料 */
		*usLen = guiECR_UDP_BufferLen;
		memcpy(uszData, gbECR_UDP_Buffer, *usLen);
		/* 清空原buffer */
		guiECR_UDP_BufferLen = 0;
		memset(gbECR_UDP_Buffer, 0x00, sizeof(gbECR_UDP_Buffer));
	}
	else
	{
		/* 備份原資料 */
		memcpy(bTempBuffer, gbECR_UDP_Buffer, guiECR_UDP_BufferLen);
		/* 抓要的資料 */
//		*usLen = *usLen;
		memcpy(uszData, gbECR_UDP_Buffer, *usLen);
		inRemainLen = guiECR_UDP_BufferLen - *usLen;
		/* 清空原buffer */
		guiECR_UDP_BufferLen = 0;
		memset(gbECR_UDP_Buffer, 0x00, sizeof(gbECR_UDP_Buffer));
		/* 放回剩餘資料 */
		guiECR_UDP_BufferLen = inRemainLen;
		memcpy(gbECR_UDP_Buffer, &bTempBuffer[*usLen], inRemainLen);
	}
	
	return (VS_SUCCESS);
}

/*
Function        :inUDP_ECR_Receive_Transaction
Date&Time       :2021/12/6 下午 4:40
Describe        :從ECR接收金額及交易別資料
*/
int inUDP_ECR_Receive_Transaction(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;
	int	inSize = 0;
        char	szOrgData[_ECR_UDP_BUFF_SIZE_] = {0};     /* 用來存原始資料做備份 */

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
	if (stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrRece == NULL)
	{
		inRetVal = VS_ERROR;
	}
	else
	{
		inRetVal = stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrRece(pobTran, &gsrECROb);
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_Receive_Transaction ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_Receive_Transaction Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_Receive_Transaction Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_Receive_Transaction Error");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inUDP_ECR_Send_Transaction_Result
Date&Time       :2021/12/7 下午 4:13
Describe        :印帳單前要送給ECR
*/
int inUDP_ECR_Send_Transaction_Result(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal = VS_ERROR;

	/* 送資料 */
	if (stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrSend == NULL)
	{
		inRetVal = VS_ERROR;
	}
	else
	{
		inRetVal = stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrSend(pobTran, &gsrECROb);
	}
	
	if (inRetVal != VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_Send_Transaction_Result Error");
		}
		
		
		return (inRetVal);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_Send_Transaction_Result ok");
		}
		
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUDP_ECR_SendError
Date&Time       :2021/12/7 上午 9:49
Describe        :送錯誤訊息給ECR
*/
int inUDP_ECR_SendError(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
	int	inRetVal = VS_SUCCESS;
	
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	gsrECROb.srTransData.inErrorType = inErrorType;
	
	if (stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrSendError == NULL)
	{
		inRetVal = VS_ERROR;
	}
	else
	{
		inRetVal = stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrSendError(pobTran, &gsrECROb);
		if (inRetVal != VS_SUCCESS)
		{
			inRetVal = VS_ERROR;
		}
		else
		{
			inRetVal = VS_SUCCESS;
		}
	}
	
	return (inRetVal);
}

/*
Function        :inUDP_ECR_SendMirror
Date&Time       :2021/12/7 下午 4:00
Describe        :送映射訊息給ECR
*/
int inUDP_ECR_SendMirror(TRANSACTION_OBJECT * pobTran)
{
	if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
        if (stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrMirror == NULL)
		return (VS_SUCCESS);
        
	if (stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrMirror(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inUDP_ECR_ReceiveEI
Date&Time       :2021/12/6 下午 4:40
Describe        :從ECR接收金額及交易別資料
*/
int inUDP_ECR_ReceiveEI(TRANSACTION_OBJECT *pobTran)
{
	int	inRetVal;

        /* 清空上次交易的資料 */
        memset(&gsrECROb.srTransData, 0x00, sizeof(ECR_TRANSACTION_DATA)); 
                
	/* 收資料 */
	if (stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrRece == NULL)
	{
		inRetVal = VS_ERROR;
	}
	else
	{
		inRetVal = stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrEIRece(pobTran, &gsrECROb);
	}
	
	if (inRetVal == VS_SUCCESS)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_ReceiveEI ok");
		}
		return (VS_SUCCESS);
	}
	else if (inRetVal == VS_TIMEOUT)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_ReceiveEI Timeout");
		}
		
		return (VS_TIMEOUT);
	}
	else if (inRetVal == VS_USER_CANCEL)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_ReceiveEI Cancel");
		}
		
		return (VS_USER_CANCEL);
	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "inUDP_ECR_ReceiveEI Error");
		}
		
		return (VS_ERROR);
	}
}

/*
Function        :inUDP_ECR_SendEI
Date&Time       :2021/12/7 下午 4:00
Describe        :送映射訊息給ECR
*/
int inUDP_ECR_SendEI(TRANSACTION_OBJECT * pobTran, int inErrorType)
{
        if (pobTran->uszECRBit != VS_TRUE)
		return (VS_SUCCESS);
	
	gsrECROb.srTransData.inErrorType = inErrorType;
	
	if (stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrEISend(pobTran, &gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inUDP_ECR_DeInitial
Date&Time       :2021/12/7 下午 4:39
Describe        :反初始化
*/
int inUDP_ECR_DeInitial(void)
{	
	if (stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrEnd(&gsrECROb) != VS_SUCCESS)
		return (VS_ERROR);
	
	return (VS_SUCCESS);
}

/*
Function        :inUDP_ECR_Initial
Date&Time       :2021/12/6 下午 4:46
Describe        :
*/
int inUDP_ECR_Initial(void)
{
	int	inRetVal = VS_ERROR;
	char	szUDPECRVersion[2 + 1] = {0};
	
	/* 目前只接受標準400 */
	memset(szUDPECRVersion, 0x00, sizeof(szUDPECRVersion));
	inGetUDP_ECRVersion(szUDPECRVersion);
	gsrECROb.srSetting.inUDPVersion = atoi(szUDPECRVersion);
        vdUtility_SYSFIN_LogMessage(AT, "ECR Init UDP ECRVerson :%d", gsrECROb.srSetting.inUDPVersion);
	
	if (stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrInitial != NULL)
	{
		inRetVal = stUDP_ECRTable[gsrECROb.srSetting.inUDPVersion].inEcrInitial(&gsrECROb);
	}
	
	return (inRetVal);
}

/*
Function        :inUDP_ECR_NCCC_144_To_400
Date&Time       :2021/12/6 下午 5:55
Describe        :
*/
int inUDP_ECR_NCCC_144_To_400(ECR_TABLE * srECROb, unsigned char *uszReceiveBuffer)
{
	
	if (srECROb->srSetting.inUDPVersion == atoi(_ECR_UDP_VERSION_STANDARD_144_))
	{
		if (uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'I'	||
		    uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'E'	||
		    uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'Q')
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Receive 'I' or 'E' or 'Q'!");
			}
			
			inECR_DeInitial();
			
			inSetUDP_ECRVersion(_ECR_UDP_VERSION_STANDARD_400_);
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("144轉400", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
			
			inECR_Initial();

			return (VS_SUCCESS);
		}
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else if (srECROb->srSetting.inUDPVersion == atoi(_ECR_UDP_VERSION_EDA_144_))
	{
		if (uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'I'	||
		    uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'E'	||
		    uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'Q')
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Receive 'I' or 'E' or 'Q'!");
			}
			
			inECR_DeInitial();
			
			inSetUDP_ECRVersion(_ECR_UDP_VERSION_EDA_400_);
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("144轉400", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
			
			inECR_Initial();

			return (VS_SUCCESS);
		}
		/* 不需要轉換協定 */
		return (VS_ERROR);
	}
	else if (srECROb->srSetting.inUDPVersion == atoi(_ECR_UDP_VERSION_082_IKEA_))
	{
		if (uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'I'	||
		    uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'E'	||
		    uszReceiveBuffer[_ECR_UDP_SOH_SIZE_ + _ECR_UDP_HEADER_SIZE_ + _ECR_RS232_STX_SIZE_] == 'Q')
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Receive 'I' or 'E' or 'Q'!");
			}
			
			inECR_DeInitial();
			
			inSetUDP_ECRVersion(_ECR_UDP_VERSION_STANDARD_KIOSK_);
			inSaveEDCRec(0);

			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
			inDISP_ChineseFont("144轉400", _FONTSIZE_8X22_, _LINE_8_5_, _DISP_CENTER_);
			
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
Function        :inUDP_CheckFromPOSIP
Date&Time       :2021/12/15 下午 1:47
Describe        :
*/
int inUDP_CheckFromPOSIP(void)
{
	char		szIP[16 + 1] = {0};
	char		szPOSIP[16 + 1] = {0};
	BYTE		bWrongIP_Bit = VS_FALSE;
	socklen_t	IPLen = 0;
	
	/* (需求單-110106)-收銀機連線通訊新增UDP模式需求 檢核是否從POS IP傳來 by Russell 2021/12/8 下午 2:56 */
	memset(szIP, 0x00, sizeof(szIP));
	IPLen = sizeof(szIP);
	inet_ntop(AF_INET, &gsrUDP_SockHost_SendRece.sin_addr, szIP, IPLen);
	memset(szPOSIP, 0x00, sizeof(szPOSIP));
	inGetPOS_IP(szPOSIP);
	if (IPLen > 0)
	{
		if (memcmp(szPOSIP, szIP, IPLen) == 0)
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "POS IP Check OK");
				inLogPrintf(AT, "POS IP:'%s'", szPOSIP);
			}
			bWrongIP_Bit = VS_FALSE;
		}
		else
		{
			if (ginDebug == VS_TRUE)
			{
				inLogPrintf(AT, "Not POS IP:'%s'", szIP);
				inLogPrintf(AT, "POS IP:'%s'", szPOSIP);
			}
			bWrongIP_Bit = VS_TRUE;
		}

	}
	else
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Not POS IP:'%s'", szIP);
			inLogPrintf(AT, "POS IP:'%s'", szPOSIP);
		}
		bWrongIP_Bit = VS_TRUE;
	}
	
	if (bWrongIP_Bit == VS_TRUE)
	{
		return (VS_ERROR);
	}
	else
	{
		return (VS_SUCCESS);
	}
}

/*
Function        :inUDP_Check_Not_Same_PosTxUniqueNO
Date&Time       :2021/12/23 上午 9:46
Describe        :
*/
int inUDP_Check_Not_Same_PosTxUniqueNO(void)
{
	char	szPosTxUniqueNO[_ECR_UDP_HEADER_SIZE_ + 1];
	
	memset(szPosTxUniqueNO, 0x00, sizeof(szPosTxUniqueNO));
	inGetPOSTxUniqueNo(szPosTxUniqueNO);
	if (strlen(szPosTxUniqueNO) == 0)
	{
		inSetPOSTxUniqueNo(gsrECROb.srTransData.szPOSTxUniqueNo);
		inSaveEDCRec(0);
		
		return (VS_SUCCESS);
	}
	else
	{
		if (memcmp(szPosTxUniqueNO, gsrECROb.srTransData.szPOSTxUniqueNo, strlen(gsrECROb.srTransData.szPOSTxUniqueNo)) == 0)
		{
			return (VS_ERROR);
		}
		else
		{
			inSetPOSTxUniqueNo(gsrECROb.srTransData.szPOSTxUniqueNo);
			inSaveEDCRec(0);
			
			return (VS_SUCCESS);
		}
	}
}

/*
Function        :inUDP_ECR_400_Standard_Initial
Date&Time       :2021/12/6 下午 4:49
Describe        :
*/
int inUDP_ECR_400_Standard_Initial(ECR_TABLE *srECROb)
{
	char	szTemplate[20 + 1] = {0};
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "----------------------------------------");
		inLogPrintf(AT, "inUDP_ECR_8N1_Standard_Initial() START !");
	}
	
	memset(szTemplate, 0x00, sizeof(szTemplate));
	inGetSupECR_UDP(szTemplate);
	if (memcmp(szTemplate, "Y", 1) != 0)
	{
		return (VS_SUCCESS);
	}
	else
	{
		inUDP_Server_Close();
		inUDP_Server_Open();
	}
	
	/* 清空接收的buffer */
        inUDP_FlushRxBuffer();
	
	if (ginDebug == VS_TRUE)
	{
		inLogPrintf(AT, "inUDP_ECR_8N1_Standard_Initial() END !");
		inLogPrintf(AT, "----------------------------------------");
	}
	
        return (VS_SUCCESS);
}

/*
Function        :inUDP_ECR_8N1_Standard_Receive_Packet
Date&Time       :2021/12/6 下午 5:18
Describe        :處理收銀機傳來的資料
*/
int inUDP_ECR_8N1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_UDP_BUFF_SIZE_];	/* 電文不包含STX和LRC */

	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive_UDP(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
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
Function        :inUDP_ECR_8N1_Standard_Send_Packet
Date&Time       :2021/12/7 上午 10:17
Describe        :處理要送給收銀機的資料
*/
int inUDP_ECR_8N1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
Function        :inUDP_ECR_8N1_Standard_Send_Error
Date&Time       :2016/7/18 上午 10:21
Describe        :傳送錯誤訊息ECR
*/
int inUDP_ECR_8N1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_];	/* 封包資料 */
	
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
Function        :inUDP_ECR_8N1_Standard_Close
Date&Time       :2016/7/11 下午 3:34
Describe        :關閉Comport
*/
int inUDP_ECR_400_Standard_Close(ECR_TABLE* srECRob)
{
        /*關閉port*/
        if (inUDP_Server_Close() != VS_SUCCESS)
        {
                return (VS_ERROR);
        }

        return (VS_SUCCESS);
}




/*
Function        :inUDP_ECR_7E1_Standard_Receive_Packet
Date&Time       :2021/12/7 上午 10:37
Describe        :處理收銀機傳來的資料
*/
int inUDP_ECR_7E1_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_UDP_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive_UDP(pobTran, srECROb, szDataBuffer, _ECR_7E1_Standard_Data_Size_);
	
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
Function        :inUDP_ECR_7E1_Standard_Send_Packet
Date&Time       :2021/12/7 上午 10:37
Describe        :處理要送給收銀機的資料
*/
int inUDP_ECR_7E1_Standard_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
Function        :inUDP_ECR_7E1_Standard_Send_Error
Date&Time       :2021/12/7 上午 10:37
Describe        :傳送錯誤訊息ECR
*/
int inUDP_ECR_7E1_Standard_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_];	/* 封包資料 */
	
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
Function        :inUDP_ECR_8N1_Customer_107_Bumper_Receive_Packet
Date&Time       :2021/12/7 下午 2:52
Describe        :處理收銀機傳來的資料
*/
int inUDP_ECR_8N1_Customer_107_Bumper_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_UDP_BUFF_SIZE_];	/* 電文不包含STX和LRC */
	
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive_UDP(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
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
Function        :inUDP_ECR_8N1_Customer_107_Bumper_Send_Packet
Date&Time       :2021/12/7 下午 2:52
Describe        :處理要送給收銀機的資料
*/
int inUDP_ECR_8N1_Customer_107_Bumper_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_] = {0};	/* 封包資料 */
	
	/* 如果已經回過ECR就不再回 */
	if (srECROb->srTransData.uszIsResponse == VS_TRUE)
		return (VS_SUCCESS);
	
	/* 防停車場ECR遺失問題，判斷是否票證遺失檔案 */
	if (srECROb->srSetting.inUDPVersion == atoi(_ECR_UDP_VERSION_STANDARD_KIOSK_))
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
Function        :inUDP_ECR_8N1_Customer_107_Bumper_Send_Error
Date&Time       :2021/12/7 下午 2:52
Describe        :傳送錯誤訊息ECR
*/
int inUDP_ECR_8N1_Customer_107_Bumper_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_];	/* 封包資料 */
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
	/* 客製化123，不回傳除回應碼外的資料 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = VS_SUCCESS;
	}
	else
	{
		if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_SETTLEMENT_, strlen(_ECR_8N1_SETTLEMENT_)) == 0)
		{
			inRetVal = inECR_8N1_Customer_Self_Trans_Settle_Pack(pobTran, srECROb, szDataBuf);
		}
		else
		{
			inRetVal = inECR_8N1_Standard_Pack_Error(pobTran, srECROb, szDataBuf);
		}
	}
	
	if (inRetVal == VS_ERROR)
	{
		return (VS_ERROR);
	}
	
/* ---------------------塞進錯誤訊息--------------------------------------------- */
	/* 客製化123，不回傳除回應碼外的資料 */
	if (!memcmp(szCustomerIndicator, _CUSTOMER_INDICATOR_123_IKEA_, _CUSTOMER_INDICATOR_SIZE_))
	{
		inRetVal = VS_SUCCESS;
	}
	else
	{
		inECR_8N1_Customer_107_Bumper_Pack_ResponseCode(pobTran, srECROb, szDataBuf);
	}
	
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
Function        :inUDP_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet
Date&Time       :2021/12/7 下午 2:52
Describe        :處理收銀機傳來的資料
*/
int inUDP_ECR_8N1_Customer_111_Kiosk_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuffer[_ECR_UDP_BUFF_SIZE_] = {0};	/* 電文不包含STX和LRC */
	
	/* 防停車場ECR遺失問題，開始ECR前刪暫存檔 */
	if (srECROb->srSetting.inUDPVersion == atoi(_ECR_UDP_VERSION_STANDARD_KIOSK_))
	{
		inNCCC_Func_Delete_Temp_PobTran_For_ECR_Missing_Data(pobTran);
	}
	
	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive_UDP(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
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
Function        :inUDP_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet
Date&Time       :2021/12/7 下午 2:52
Describe        :處理要送給收銀機的資料
*/
int inUDP_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
Function        :inUDP_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error
Date&Time       :2021/12/7 下午 2:52
Describe        :傳送錯誤訊息ECR
*/
int inUDP_ECR_8N1_Customer_039_CARD_NO_HIDE_F8_B4_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_];	/* 封包資料 */
	
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
Function        :inUDP_ECR_8N1_Customer_098_Mcdonalds_Standard_Receive_Packet
Date&Time       :2021/12/7 下午 2:53
Describe        :處理收銀機傳來的資料
*/
int inUDP_ECR_8N1_Customer_098_Mcdonalds_Standard_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSize = 0;
	char	szDataBuffer[_ECR_UDP_BUFF_SIZE_] = {0};	/* 電文不包含STX和LRC */
	
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
                inRetVal = inECR_Receive_UDP(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);

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
Function        :inUDP_ECR_8N1_Customer_098_Mcdonalds_Send_Packet
Date&Time       :2021/12/7 下午 2:53
Describe        :處理要送給收銀機的資料
*/
int inUDP_ECR_8N1_Customer_098_Mcdonalds_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
	
	/* (需求單 - 107227)邦柏科技自助作業客製化 存起ECR Out Data by Russell 2018/12/7 下午 2:36 */
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_START_CARD_NO_INQUIRY_, strlen(_ECR_8N1_START_CARD_NO_INQUIRY_)) != 0	&&
	    memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0   &&
            memcmp(srECROb->srTransData.szTransType, _ECR_8N1_REPRINT_RECEIPT_, strlen(_ECR_8N1_REPRINT_RECEIPT_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
	}
	
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
Function        :inUDP_ECR_8N1_Customer_098_Mcdonalds_Mirror_Packet
Date&Time       :2021/12/7 下午 2:53
Describe        :送給收銀機目前端末機行為
*/
int inUDP_ECR_8N1_Customer_098_Mcdonalds_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
Function        :inUDP_ECR_8N1_Customer_098_Mcdonalds_Send_Error
Date&Time       :2021/12/7 下午 2:53
Describe        :傳送錯誤訊息ECR
*/
int inUDP_ECR_8N1_Customer_098_Mcdonalds_Send_Error(TRANSACTION_OBJECT *pobTran, ECR_TABLE *srECROb)
{
	int	inRetVal;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_];	/* 封包資料 */
	
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
//	inECR_Save_Response(szDataBuf, _ECR_8N1_Standard_Data_Size_);
        
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
Function        :inUDP_ECR_8N1_Customer_033_UNIAIR_Send_Packet
Date&Time       :2022/7/12 下午 2:41
Describe        :033客製化，兩段式第二段時不接收對方ACK
*/
int inUDP_ECR_8N1_Customer_033_UNIAIR_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
		inRetVal = inECR_8N1_Standard_Pack(pobTran, srECROb, szDataBuf);
	}
	
	if (memcmp(srECROb->srTransData.szTransType, _ECR_8N1_INQUIRY_LAST_TRANSACTION_, strlen(_ECR_8N1_INQUIRY_LAST_TRANSACTION_)) != 0)
	{
		inECR_Save_Response(szDataBuf, inSendSize);
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
Function        :inUDP_ECR_8N1_Customer_038_Bellavita_Receive_Packet
Date&Time       :2022/9/13 下午 6:32
Describe        :處理收銀機傳來的資料
*/
int inUDP_ECR_8N1_Customer_038_Bellavita_Receive_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal;
	char	szDataBuffer[_ECR_UDP_BUFF_SIZE_];	/* 電文不包含STX和LRC */

	memset(&szDataBuffer, 0x00, sizeof(szDataBuffer));
/* -----------------------開始接收資料------------------------------------------ */
	srECROb->srSetting.inTimeout = _ECR_RECEIVE_REQUEST_TIMEOUT_;
	inRetVal = inECR_Receive_UDP(pobTran, srECROb, szDataBuffer, _ECR_8N1_Standard_Data_Size_);
	
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
Function        :inUDP_ECR_8N1_Customer_090_TAIPEI_101_Send_Packet
Date&Time       :2022/12/6 下午 6:45
Describe        :處理要送給收銀機的資料
*/
int inUDP_ECR_8N1_Customer_090_TAIPEI_101_Send_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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
Function        :inUDP_ECR_8N1_Standard_Mirror_Packet
Date&Time       :2025/10/17 下午 5:14
Describe        :送給收銀機目前端末機行為
*/
int inUDP_ECR_8N1_Standard_Mirror_Packet(TRANSACTION_OBJECT *pobTran, ECR_TABLE * srECROb)
{
	int	inRetVal = VS_ERROR;
	int	inSendSize = 0;
	char	szDataBuf[_ECR_UDP_BUFF_SIZE_] = {0};	/* 封包資料 */
	
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