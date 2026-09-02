
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
#include "Global.h"

/*
//#include <winsock.h>
#include <winsock2.h>
#include <stdio.h>

#include "Error.h"
#include "TCPIP.h"
#include "Function.h"
*/

#if	RDEVN == NE_NEXSYS_V3
	/* For QA Debug使用 Start */
	extern	int	ginDebug;
	extern	int	ginISODebug;
	#include	<sqlite3.h>
	#include	"../../SOURCE/INCLUDES/Define_1.h"
	#include	"../../SOURCE/PRINT/Print.h"
	#include	"../../SOURCE/INCLUDES/Transaction.h"
	#include	"../../SOURCE/INCLUDES/AllStruct.h"
	#include	"../../SOURCE/FUNCTION/Sqlite.h"
	#include	"../../SOURCE/FUNCTION/Function.h"
	#include	"../../NCCC/NCCCTicketSrc.h"
	#include	"../../NCCC/NCCCTicketIso.h"
	/* For QA Debug使用 End */
#endif

STRUCT_TCPIP_PAR srTCPIPParL1[DLL_TOTAL_CNT] = {
	/*DLL_OPENCOM,*/				{0, 	0,		0,		0},
	/*DLL_SIGNONQUERY,*/			{0, 	0,		0,		0},
	/*DLL_RESET,*/					{172,	108,	79, 	0},
	/*DLL_RESET_OFF,*/				{0,		0,		0, 		0},
	/*DLL_SIGNON,*/ 				{0, 	0,		0,		0},
	/*DLL_READ_CODE_VERSION,*/		{0, 	0,		0,		0},
	/*DLL_READ_CARD_BASIC_DATA,*/	{0, 	0,		0,		0},
	/*DLL_DEDUCT_VALUE,*/			{150,	89, 	83, 	35},
	/*DLL_DEDUCT_VALUE_AUTH,*/		{0, 	0,		0,		0},
	/*DLL_ADD_VALUE,*/				{188,	85, 	83, 	35},
	/*DLL_ADD_VALUE_AUTH,*/ 		{63,	0,		0,		0},
	/*DLL_VOID_TXN,*/				{188,	85, 	83, 	35},
	/*DLL_AUTOLOAD,*/				{188,	85, 	83, 	35},
	/*DLL_AUTOLOAD_AUTH,*/			{63,	0,		0,		0},
	/*DLL_READ_DONGLE_LOG,*/		{0, 	0,		0,		0},
	/*DLL_READ_CARD_LOG,*/			{0, 	0,		0,		0},
	/*DLL_TX_REFUND,*/				{190,	85, 	83, 	35},
	/*DLL_TX_REFUND_AUTH,*/ 		{63,	0,		0,		0},
	/*DLL_TX_ADD_VALUE,*/			{188,	85, 	83, 	35},
	/*DLL_TX_ADD_VALUE_AUTH,*/		{63,	0,		0,		0},
	/*DLL_SET_VALUE,*/				{121,	85, 	83, 	35},
	/*DLL_SET_VALUE_AUTH,*/ 		{0, 	0,		0,		0},
	/*DLL_AUTOLOAD_ENABLE,*/		{191,	85, 	83, 	35},
	/*DLL_AUTOLOAD_ENABLE_AUTH,*/	{66,	0,		0,		0},
	/*DLL_ACCUNT_LINK,*/			{191,	85, 	83, 	35},
	/*DLL_ACCUNT_LINK_AUTH,*/		{66,	0,		0,		0},
	/*DLL_LOCK_CARD,*/				{0,		0,		0,		0},
	/*DLL_READ_CARD_NUMBER,*/		{0, 	0,		0,		0},
	/*DLL_ADD_VALUE2,*/				{188,	85, 	83, 	35},
	/*DLL_MULTI_SELECT,*/			{0, 	0,		0,		0},
	/*DLL_ANTENN_CONTROL,*/			{0, 	0,		0,		0},
	/*DLL_TAXI_READ,*/ 				{0, 	0,		0,		0},
	/*DLL_TAXI_DEDUCT,*/ 			{0, 	0,		0,		0},
	/*DLL_ADD_OFFLINE,*/ 			{0, 	0,		0,		0},
	/*DLL_MATCH,*/					{172,	108,	79, 	0},
	/*DLL_MATCH_AUTH,*/ 			{0, 	0,		0,		0},
	/*DLL_FAST_READ_CARD,*/			{0, 	0,		0,		0},
	/*DLL_RESET2,*/					{172,	108,	79, 	0},
	/*DLL_MATCH2,*/					{172,	108,	79, 	0},
	/*DLL_CLOSECOM,*/				{0, 	0,		0,		0},
	};

STRUCT_TCPIP_PAR srTCPIPParL2[DLL_TOTAL_CNT] = {
	/*DLL_OPENCOM,*/				{0, 	0,		0,		0},
	/*DLL_SIGNONQUERY,*/			{0, 	0,		0,		0},
	/*DLL_RESET,*/					{362,	203,	67, 	0},
	/*DLL_RESET_OFF,*/				{0, 	0,		0,		0},
	/*DLL_SIGNON,*/ 				{69,	0,		0,		0},
	/*DLL_READ_CODE_VERSION,*/		{0, 	0,		0,		0},
	/*DLL_READ_CARD_BASIC_DATA,*/	{0, 	0,		0,		0},
	/*DLL_DEDUCT_VALUE,*/			{241,	118,	67, 	69},
	/*DLL_DEDUCT_VALUE_AUTH,*/		{0, 	0,		0,		0},
	/*DLL_ADD_VALUE,*/				{327,	126,	71, 	73},
	/*DLL_ADD_VALUE_AUTH,*/ 		{114,	0,		0,		0},
	/*DLL_VOID_TXN,*/				{327,	126,	71, 	73},
	/*DLL_AUTOLOAD,*/				{330,	129,	71, 	73},
	/*DLL_AUTOLOAD_AUTH,*/			{117,	0,		0,		0},
	/*DLL_READ_DONGLE_LOG,*/		{0, 	0,		0,		0},
	/*DLL_READ_CARD_LOG,*/			{0, 	0,		0,		0},
	/*DLL_TX_REFUND,*/				{327,	126,	71, 	73},
	/*DLL_TX_REFUND_AUTH,*/ 		{114,	0,		0,		0},
	/*DLL_TX_ADD_VALUE,*/			{327,	126,	71, 	73},
	/*DLL_TX_ADD_VALUE_AUTH,*/		{114, 	0,		0,		0},
	/*DLL_SET_VALUE,*/				{236,	126,	71, 	73},
	/*DLL_SET_VALUE_AUTH,*/ 		{0, 	0,		0,		0},
	/*DLL_AUTOLOAD_ENABLE,*/		{330,	126,	71, 	73},
	/*DLL_AUTOLOAD_ENABLE_AUTH,*/	{117,	0,		0,		0},
	/*DLL_ACCUNT_LINK,*/			{330,	146,	72, 	77},
	/*DLL_ACCUNT_LINK_AUTH,*/		{118,	0,		0,		0},
	/*DLL_LOCK_CARD,*/				{0, 	0,		0,		0},
	/*DLL_READ_CARD_NUMBER,*/		{0, 	0,		0,		0},
	/*DLL_ADD_VALUE2,*/				{327,	126,	71, 	73},
	/*DLL_MULTI_SELECT,*/			{0, 	0,		0,		0},
	/*DLL_ANTENN_CONTROL,*/			{0, 	0,		0,		0},
	/*DLL_TAXI_READ,*/				{0, 	0,		0,		0},
	/*DLL_TAXI_DEDUCT,*/			{0, 	0,		0,		0},
	/*DLL_ADD_OFFLINE,*/			{0, 	0,		0,		0},
	/*DLL_MATCH,*/					{362,	203,	67, 	0},
	/*DLL_MATCH_AUTH,*/ 			{69,	0,		0,		0},
	/*DLL_FAST_READ_CARD,*/			{0, 	0,		0,		0},
	/*DLL_FAST_READ_CARD2,*/		{0, 	0,		0,		0},
	/*DLL_RESET2,*/					{362,	203,	67, 	0},
	/*DLL_MATCH2,*/					{362,	203,	67, 	0},
	/*DLL_S_SET_VALUE,*/			{328,	162,	72, 	74},
	/*DLL_S_SET_VALUE_AUTH,*/ 		{114, 	0,		0,		0},
	/*DLL_THSRC_READ,*/ 			{0,		0,		0,		0},
	/*DLL_E_ADD_VALUE,*/ 			{0,		0,		0,		0},
	/*DLL_VERIFY_HOST_CRYPT,*/ 		{0,		0,		0,		0},
	/*DLL_E_QR_TXN,*/ 				{0,		0,		0,		0},
	/*DLL_QR_VERIFY_HOST_CRYPT,*/ 	{0,		0,		0,		0},
	/*DLL_MATCH_OFF,*/ 				{0,		0,		0,		0},
	/*DLL_CBIKE_READ,*/ 			{0,		0,		0,		0},
	/*DLL_CBIKE_DEDUCT,*/ 			{0,		0,		0,		0},
	/*DLL_EDCA_READ,*/ 				{0,		0,		0,		0},
	/*DLL_EDCA_DEDUCT,*/ 			{0,		0,		0,		0},
	/*DLL_TLRT_AUTH,*/ 				{0,		0,		0,		0},
	/*DLL_ADD_VALUE1,*/				{351,	150,	71, 	73},
	/*DLL_ADD_VALUE_AUTH1,*/ 		{114,	0,		0,		0},
	/*DLL_REFUND_CARD_AUTH,*/		{114, 	0,		0,		0},
	/*DLL_DEDUCT_VALUE1,*/			{241,	121,	67,		69},
	/*DLL_DEDUCT_VALUE2,*/			{241,	118,	67, 	69},
	/*DLL_GET_CMASMODE,*/ 			{0,		0,		0,		0},
	/*DLL_CLOSECOM,*/				{0, 	0,		0,		0},
	};

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
SOCKET ConnectSocket = INVALID_SOCKET;
#else// READER_MANUFACTURERS==LINUX_API
#define MAX_SOCK_FD		1
int fd;
int sockfd_server,is_connected[MAX_SOCK_FD];
#endif

STRUCT_ADD_DATA srAdditionalData;

/*
inTCPIPSendData:送資料
IP:主機IP
Port:主機Port
bSendData:欲傳送之資料
inSendLen:欲傳送資料之長度
回傳值:成功->SUCCESS
	   失敗->小於0
*/
int inTCPIPSendData(unsigned char *IP,unsigned short Port,BYTE *bSendData,int inSendLen)
{
#if READER_MANUFACTURERS==WINDOWS_API// || READER_MANUFACTURERS==WINDOWS_CE_API
int inRetVal;
WSADATA wsaData;
struct sockaddr_in clientService;
//BYTE bSendBuf[512];
int buflen = 0,recelen = 0,inRespCodeAdr = 0;
int TimeOut=srTxnData.srParameter.gTCPIPTimeOut * 1000; //設置發送超過10秒
struct hostent *lphost;

	//初始化
	inRetVal = WSAStartup(MAKEWORD(2,2),&wsaData);
	if(inRetVal != NO_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"WSAStartup failed with error: %d",inRetVal);
		return ICER_ERROR;
		//return TCP_SOCKET_FAIL;
	}

	//----------------------
	//建Socket
	ConnectSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(ConnectSocket == INVALID_SOCKET)
	{
		log_msg(LOG_LEVEL_ERROR,"socket failed with error: %ld",WSAGetLastError());
		WSACleanup();
		return ICER_ERROR;
		//return TCP_SOCKET_FAIL;
	}

	#if READER_MANUFACTURERS==WINDOWS_API
	if((inRetVal = setsockopt(ConnectSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&TimeOut,sizeof(TimeOut)))==SOCKET_ERROR)
	{

		log_msg(LOG_LEVEL_ERROR,"inTCPIPSendData 3 : %d",inRetVal);
		return ICER_ERROR;
	}

	if((inRetVal = setsockopt(ConnectSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&TimeOut,sizeof(TimeOut)))==SOCKET_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPSendData 4 : %d",inRetVal);
		return ICER_ERROR;
	}
	#endif

	//設參數
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr((const char *)IP);

	if(clientService.sin_addr.s_addr==INADDR_NONE)
	{
		lphost=gethostbyname((char *)IP);
		if(lphost!=NULL)
			clientService.sin_addr = *((struct in_addr *)lphost->h_addr_list[0]);
		else
		{
			log_msg(LOG_LEVEL_ERROR,"gethostbyname(%s)(%d) failed with error: %d",IP,Port,WSAGetLastError());
			WSACleanup();
			return ICER_ERROR;
		}
        //clientService.sin_addr.s_addr = inet_ntoa(serv_addr.sin_addr);
	}

	clientService.sin_port = htons(Port);
	//log_msg(LOG_LEVEL_ERROR,"connect(%s)(%d) ",IP,Port);

	//連線
	inRetVal = connect(ConnectSocket,(SOCKADDR*)&clientService,sizeof(clientService));
	if(inRetVal == SOCKET_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"connect(%s)(%d) failed with error: %d",IP,Port,WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return ICER_ERROR;
		//return TCP_CONNECT_FAIL;
	}

	//送資料
	inRetVal = send(ConnectSocket,(const char *)bSendData,inSendLen,0);
	if(inRetVal == SOCKET_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"send failed with error: %d",WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return ICER_ERROR;
		//return TCP_SEND_FAIL;
	}
#elif READER_MANUFACTURERS==WINDOWS_CE_API
int inRetVal;
WSADATA wsaData;
struct sockaddr_in clientService;
//BYTE bSendBuf[512];
int buflen = 0,recelen = 0,inRespCodeAdr = 0;
int TimeOut=srTxnData.srParameter.gTCPIPTimeOut * 1000; //設置發送超過10秒
struct hostent *lphost;

	//初始化
	inRetVal = WSAStartup(MAKEWORD(2,2),&wsaData);
	if(inRetVal != NO_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"WSAStartup failed with error: %d",inRetVal);
		return ICER_ERROR;
		//return TCP_SOCKET_FAIL;
	}

	//----------------------
	//建Socket
	ConnectSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(ConnectSocket == INVALID_SOCKET)
	{
		log_msg(LOG_LEVEL_ERROR,"socket failed with error: %ld",WSAGetLastError());
		WSACleanup();
		return ICER_ERROR;
		//return TCP_SOCKET_FAIL;
	}

	//設參數
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr((const char *)IP);

	if(clientService.sin_addr.s_addr==INADDR_NONE)
	{
		lphost=gethostbyname((char *)IP);
		if(lphost!=NULL)
			clientService.sin_addr = *((struct in_addr *)lphost->h_addr_list[0]);
		else
		{
			log_msg(LOG_LEVEL_ERROR,"gethostbyname(%s)(%d) failed with error: %d",IP,Port,WSAGetLastError());
			WSACleanup();
			return ICER_ERROR;
		}
		//clientService.sin_addr.s_addr = inet_ntoa(serv_addr.sin_addr);
	}

	clientService.sin_port = htons(Port);
	//log_msg(LOG_LEVEL_ERROR,"connect(%s)(%d) ",IP,Port);

	//連線
	inRetVal = connect(ConnectSocket,(SOCKADDR*)&clientService,sizeof(clientService));
	if(inRetVal == SOCKET_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"connect(%s)(%d) failed with error: %d",IP,Port,WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return ICER_ERROR;
		//return TCP_CONNECT_FAIL;
	}

	//送資料
	inRetVal = send(ConnectSocket,(const char *)bSendData,inSendLen,0);
	if(inRetVal == SOCKET_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"send failed with error: %d",WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return ICER_ERROR;
		//return TCP_SEND_FAIL;
	}

#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
//int tcp_port, i;
int ret;
struct sockaddr_in dest;
char	tmpbuf[1024];
unsigned long ul = 1L;
fd_set rset,wset;
int error=-1, len;

	log_msg(LOG_LEVEL_FLOW,"inTCPIPSendData 1");
	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0)
	{
		log_msg(LOG_LEVEL_ERROR,"socket failed");
		return ICER_ERROR;
	}
	bzero(&dest, sizeof(dest));
	bzero(&tmpbuf, sizeof(tmpbuf));

	dest.sin_family = AF_INET;

	struct timeval tv;
	tv.tv_sec =10;
	tv.tv_usec=0;

	if (setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv, sizeof(tv)) == -1)
	{
		log_msg(LOG_LEVEL_ERROR,"setsockopt failed 1");
		close(fd);
		return ICER_ERROR;
	}
	tv.tv_sec =10;
	tv.tv_usec=0;

	if (setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&tv, sizeof(tv)) == -1)
	{
		log_msg(LOG_LEVEL_ERROR,"setsockopt failed 2");
		close(fd);
		return ICER_ERROR;
	}

	dest.sin_port = htons(Port);
	inet_aton((char *)IP, &dest.sin_addr); /*get server ip*/
	
	/* 設定MSS(maximum segment size) 預設為536，和520同步解決宏遠電信問題，含header設為1400，扣掉header(54)應該設為1346 */
	int	inOption = 0;
	inOption = 1346;
	setsockopt(fd, SOL_TCP, TCP_MAXSEG, &inOption, sizeof(inOption));

	ul = 1L;
	ioctl(fd,FIONBIO,&ul);

	ret = connect(fd, (struct sockaddr*)&dest, sizeof(dest));
	if (ret == 0)
		log_msg(LOG_LEVEL_FLOW,"connect ok!!");
	else
	{
		FD_ZERO(&rset);
        FD_SET(fd,&rset);
		wset = rset;
		tv.tv_sec =10;
		tv.tv_usec=0;
    	if(select(fd+1,&rset,&wset,NULL,&tv) > 0)
    	{
			if(FD_ISSET(fd,&rset) || FD_ISSET(fd,&wset))
			{
				len = sizeof(error);
    			getsockopt(fd,SOL_SOCKET,SO_ERROR,&error,(socklen_t *)&len);
    			if(error == 0)
    			{
    				ret = 1;
    			}
    			else
    			{
    				log_msg(LOG_LEVEL_ERROR,"getsockopt select failed error = %d",error);
    				ret = 0;
    			}
			}
    	}
    	else
    	{
    		log_msg(LOG_LEVEL_ERROR,"socket select failed error = %d",error);
    		ret = 0;
    	}
	}

	ul = 0L;
	ioctl(fd,FIONBIO,&ul);

	if(!ret)
	{
		close(fd);
		log_msg(LOG_LEVEL_ERROR,"socket failed ret = %d",ret);
		return ICER_ERROR;
	}

	if (send(fd, bSendData, inSendLen, 0) < 0) /*send data to ethernet*/
	{
		log_msg(LOG_LEVEL_ERROR,"send to ethernet fail");
		return ICER_ERROR;
	}

#else
int inRetVal;

	inRetVal = ECC_TCPIPConnect(IP,Port,10);
	if(inRetVal != 0)
	{
		log_msg(LOG_LEVEL_ERROR,"ECC_TCPIPConnect failed(%s)(%d)",IP,Port);
		return ICER_ERROR;
	}

	inRetVal = ECC_TCPIPSend(bSendData,inSendLen,srTxnData.srParameter.gTCPIPTimeOut);
	if(inRetVal != 0)
	{
		ECC_TCPIPClose();
		log_msg(LOG_LEVEL_ERROR,"ECC_TCPIPSend failed(%d)",inRetVal);
		return ICER_ERROR;
	}

#endif

	return SUCCESS;
}

/*
bRecvData:接收到之資料
回傳值:成功->接收到的資料長度
	   失敗->小於0
*/
int inTCPIPRecvData(BYTE *bRecvData,int inTCPHeaderLen)
{
#if READER_MANUFACTURERS==WINDOWS_API// || READER_MANUFACTURERS==WINDOWS_CE_API
int inRetVal,inRecvLen = 0,inHeaderLen = 0,inTotalLen = 0;
int timeout = srTxnData.srParameter.gTCPIPTimeOut * 1000/*,inTCPHeaderLen = 2*/;

	#if READER_MANUFACTURERS==WINDOWS_API
	//設接收時間
	inRetVal = setsockopt(ConnectSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout));
	if(inRetVal == SOCKET_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"setsockopt failed with error: %d",WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return ICER_ERROR;
		//return TCP_RECV_FAIL;
	}
	#endif

	/*if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
		inTCPHeaderLen = 2;
	else if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE || srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)
		inTCPHeaderLen = 3;
	else
		inTCPHeaderLen = 3;*/

	do
	{
		//收資料
		inRecvLen = recv(ConnectSocket,(char *)&bRecvData[inTotalLen],MAX_XML_FILE_SIZE,0);
		if(inRecvLen <= 0)
		{
			log_msg(LOG_LEVEL_ERROR,"recv failed with error: %d(%d),(%d),(%d)!=(%d)",WSAGetLastError(),inRecvLen,inTCPHeaderLen,inHeaderLen,inTotalLen);
			closesocket(ConnectSocket);
			WSACleanup();
			return ICER_ERROR;
			//return TCP_RECV_FAIL;
		}
		else if(inTCPHeaderLen == 0)
		{
			inTotalLen += inRecvLen;
			break;
		}

		inTotalLen += inRecvLen;
		if(inRecvLen > 2 && inHeaderLen == 0)
		{
			inHeaderLen = bRecvData[0] * 256 + bRecvData[1];
			//log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData 1 inRecvLen = %d,inHeaderLen = %d,inTotalLen = %d",inRecvLen,inHeaderLen,inTotalLen);
		}
		//else
		//	log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData 2 inRecvLen = %d,inHeaderLen = %d,inTotalLen = %d",inRecvLen,inHeaderLen,inTotalLen);
	}
	while(inTotalLen - inTCPHeaderLen < inHeaderLen);

	inRecvLen = inTotalLen;

	//關socket
	inRetVal = closesocket(ConnectSocket);
	if(inRetVal == SOCKET_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"close failed with error: %d",WSAGetLastError());
		WSACleanup();
		return ICER_ERROR;
		//return TCP_CLOSESOCKET_FAIL;
	}

	WSACleanup();

#elif READER_MANUFACTURERS==WINDOWS_CE_API

int inRetVal,inRecvLen = 0,inHeaderLen = 0,inTotalLen = 0;

	fd_set read;

	FD_ZERO(&read);
	FD_SET(ConnectSocket, &read);
	timeval timeout = {srTxnData.srParameter.gTCPIPTimeOut, 0};
	int status = select(ConnectSocket + 1, &read, 0, 0, &timeout);

#if 1
	if(status > 0)
	{// Success
		if (FD_ISSET(ConnectSocket, &read))
			inRecvLen = recv(ConnectSocket,(char *)&bRecvData[inTotalLen],MAX_XML_FILE_SIZE,0);
	}
	else if(status == 0)
	{// Timeout
		log_msg(LOG_LEVEL_ERROR,"Receive timeout.");
		closesocket(ConnectSocket);
		WSACleanup();
		return 0;
	}
	else if(status < 0)
	{// Error
		log_msg(LOG_LEVEL_ERROR,"Receive error %d", GetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 0;
	}
#else
	do
	{
		//收資料
		inRecvLen = recv(ConnectSocket,(char *)&bRecvData[inTotalLen],MAX_XML_FILE_SIZE,0);
		if(inRecvLen <= 0)
		{
			log_msg(LOG_LEVEL_ERROR,"recv failed with error: %d(%d),(%d),(%d)!=(%d)",WSAGetLastError(),inRecvLen,inTCPHeaderLen,inHeaderLen,inTotalLen);
			closesocket(ConnectSocket);
			WSACleanup();
			return ICER_ERROR;
			//return TCP_RECV_FAIL;
		}
		else if(inTCPHeaderLen == 0)
		{
			inTotalLen += inRecvLen;
			break;
		}

		inTotalLen += inRecvLen;
		if(inRecvLen > 2 && inHeaderLen == 0)
		{
			inHeaderLen = bRecvData[0] * 256 + bRecvData[1];
			//log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData 1 inRecvLen = %d,inHeaderLen = %d,inTotalLen = %d",inRecvLen,inHeaderLen,inTotalLen);
		}
		//else
		//	log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData 2 inRecvLen = %d,inHeaderLen = %d,inTotalLen = %d",inRecvLen,inHeaderLen,inTotalLen);
	}
	while(inTotalLen - inTCPHeaderLen < inHeaderLen);
#endif

	//inRecvLen = inTotalLen;

	//關socket
	inRetVal = closesocket(ConnectSocket);
	if(inRetVal == SOCKET_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR,"close failed with error: %d",WSAGetLastError());
		WSACleanup();
		return ICER_ERROR;
		//return TCP_CLOSESOCKET_FAIL;
	}

	WSACleanup();

#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
//unsigned char	buf[1024];
//char	tmpbuf[1024];
//int inRetVal;
int inRecvLen = 0,inHeaderLen = 0,inTotalLen = 0;

	//log_msg(inTCPIPRecvData,"inTCPIPSendData 1");
	do
	{
		//收資料
		inRecvLen = recv(fd,(char *)&bRecvData[inTotalLen],MAX_XML_FILE_SIZE,0);
		log_msg(LOG_LEVEL_FLOW,"inTCPIPRecvData 1 %d,%d,%d,%d",inRecvLen,inTotalLen,inTCPHeaderLen,inHeaderLen);
		if(inRecvLen <= 0)
		{
			log_msg(LOG_LEVEL_ERROR,"recv failed with error: (%d),(%d),(%d)!=(%d)",inRecvLen,inTCPHeaderLen,inHeaderLen,inTotalLen);
			close(fd);
			return ICER_ERROR;
			//return TCP_RECV_FAIL;
		}
		else if(inTCPHeaderLen == 0)
		{
			inTotalLen += inRecvLen;
			break;
		}

		inTotalLen += inRecvLen;
		if(inRecvLen > 2 && inHeaderLen == 0)
		{
			if(srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
				inHeaderLen = ((bRecvData[0] / 16) * 1000) + ((bRecvData[0] % 16) * 100) + ((bRecvData[1] / 16) * 10) + (bRecvData[1] % 16);
			else
				inHeaderLen = bRecvData[0] * 256 + bRecvData[1];
			//log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData 1 inRecvLen = %d,inHeaderLen = %d,inTotalLen = %d",inRecvLen,inHeaderLen,inTotalLen);
		}
		//else
		//	log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData 2 inRecvLen = %d,inHeaderLen = %d,inTotalLen = %d",inRecvLen,inHeaderLen,inTotalLen);
	}
	while(inTotalLen - inTCPHeaderLen < inHeaderLen);

	inRecvLen = inTotalLen;
	close(fd);

#else
int inRetVal,inRecvLen;
//int dwRead;
int dwTotalRead;
unsigned long ulHeaderLen = 0L;

  #if 1

	//while(1)
	//{
		ulHeaderLen = 0L;
		dwTotalRead = 0;

		inRetVal = inTCPIPRecvByLen(&bRecvData[0],inTCPHeaderLen,srTxnData.srParameter.gTCPIPTimeOut);
		if(inRetVal < SUCCESS)
		{
			ECC_TCPIPClose();
			log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData error 1: %d,inRecvLen = %d",inRetVal,dwTotalRead);
			return ICER_ERROR;
		}

		if(srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
			ulHeaderLen = ((bRecvData[0] / 16) * 1000) + ((bRecvData[0] % 16) * 100) + ((bRecvData[1] / 16) * 10) + (bRecvData[1] % 16);
		else
			ulHeaderLen = bRecvData[0] * 256 + bRecvData[1];
		dwTotalRead += inRetVal;
		log_msg(LOG_LEVEL_FLOW,"inTCPIPRecvData 1 inRetVal = %d,dwTotalRead = %d,ulHeaderLen = %d",inRetVal,dwTotalRead,ulHeaderLen);

		inRecvLen = ulHeaderLen - inRetVal + inTCPHeaderLen;
		//inRecvLen = ulHeaderLen - inRetVal + 2;
		if(inRecvLen > 0)
		{
			inRetVal = inTCPIPRecvByLen(&bRecvData[dwTotalRead],inRecvLen,srTxnData.srParameter.gTCPIPTimeOut);
			if(inRetVal < SUCCESS)
			{
				ECC_TCPIPClose();
				log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData error 2: %d,inRecvLen = %d",inRetVal,dwTotalRead);
				return ICER_ERROR;
			}

			dwTotalRead += inRetVal;
			log_msg(LOG_LEVEL_FLOW,"inTCPIPRecvData 2 inRetVal = %d,dwTotalRead = %d,ulHeaderLen = %d",inRetVal,dwTotalRead,ulHeaderLen);
		}
	//}

	inRecvLen = dwTotalRead;
	ECC_TCPIPClose();

	#if 0
	do
	{
		inRetVal = ECC_TCPIPRecv(&bRecvData[inStartLen],&inRecvLen,srTxnData.srParameter.gTCPIPTimeOut);
		if(inRetVal != SUCCESS || (int)inRecvLen < 0)
		{
			ECC_TCPIPClose();
			log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData error 1: %d,inRecvLen = %d",inRetVal,inRecvLen);
			return ICER_ERROR;
		}

		if(inTCPHeaderLen == 0)
		{
			ECC_TCPIPClose();
			return inRecvLen;
		}
		log_msg(LOG_LEVEL_FLOW,"inTCPIPRecvData 0 inRetVal = %d,inRecvLen = %d,%d,%d,%d,%02x%02x",inRetVal,inRecvLen,inTmpLen,inStartLen,inTCPHeaderLen,bRecvData[0],bRecvData[1]);

		inStartLen += inRecvLen;
		if(inStartLen >= inTCPHeaderLen)
		{
			if(srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
				inRecvLen = inTmpLen = ((bRecvData[0] / 16) * 1000) + ((bRecvData[0] % 16) * 100) + ((bRecvData[1] / 16) * 10) + (bRecvData[1] % 16);
			else
				inRecvLen = inTmpLen = bRecvData[0] * 256 + bRecvData[1] + inTCPHeaderLen;

			break;
		}
		else
			inRecvLen = inTCPHeaderLen - inStartLen;
	}
	while(1);

	log_msg(LOG_LEVEL_FLOW,"inTCPIPRecvData 1 inRetVal = %d,inRecvLen = %d,%d,%d,%d,%02x%02x",inRetVal,inRecvLen,inTmpLen,inStartLen,inTCPHeaderLen,bRecvData[0],bRecvData[1]);

	while(inTmpLen > 0 && inTmpLen > inStartLen)
	{
		/*if(inTmpLen == 0 && inStartLen >= inTCPHeaderLen)
		{
			if(srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
				inRecvLen = inTmpLen = ((bRecvData[0] / 16) * 1000) + ((bRecvData[0] % 16) * 100) + ((bRecvData[1] / 16) * 10) + (bRecvData[1] % 16);
			else
				inRecvLen = inTmpLen = bRecvData[0] * 256 + bRecvData[1] + inTCPHeaderLen;
		}*/

		log_msg(LOG_LEVEL_FLOW,"inTCPIPRecvData 2 inRetVal = %d,inRecvLen = %d,%d,%d,%d",inRetVal,inRecvLen,inTmpLen,inStartLen,inTCPHeaderLen);
		inRetVal = ECC_TCPIPRecv(&bRecvData[inStartLen],&inRecvLen,srTxnData.srParameter.gTCPIPTimeOut);
		if(inRetVal != SUCCESS || (int)inRecvLen < 0)
		{
			ECC_TCPIPClose();
			log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData error 2: %d,inRecvLen = %d,%d,%d,%d,",inRetVal,inRecvLen,inTmpLen,inTCPHeaderLen,inStartLen);
			return ICER_ERROR;
		}

		inStartLen += inRecvLen;
		inTmpLen -= inRecvLen;
	}

	ECC_TCPIPClose();
	inRecvLen = inStartLen;
	#endif
  #else
	inRetVal = ECC_TCPIPRecv(&bRecvData[inStartLen],&inRecvLen,srTxnData.srParameter.gTCPIPTimeOut);
	if(inRetVal != SUCCESS)
	{
		ECC_TCPIPClose();
		log_msg(LOG_LEVEL_ERROR,"ECC_TCPIPRecv error 1: %d",inRetVal);
		return ICER_ERROR;
	}

	if(inTCPHeaderLen == 0)
	{
		ECC_TCPIPClose();
		return inRecvLen;
	}

	inStartLen = inRecvLen;
	if(inRecvLen == inTCPHeaderLen)
	{
		inRecvLen = bRecvData[0] * 256 + bRecvData[1];

		inRetVal = ECC_TCPIPRecv(&bRecvData[inStartLen],&inRecvLen,srTxnData.srParameter.gTCPIPTimeOut);
		if(inRetVal != SUCCESS)
		{
			ECC_TCPIPClose();
			log_msg(LOG_LEVEL_ERROR,"ECC_TCPIPRecv error 2: %d",inRetVal);
			return ICER_ERROR;
		}

		inRecvLen += inTCPHeaderLen;
	}
	else
	{
		inTmpLen = bRecvData[0] * 256 + bRecvData[1];
		//log_msg(LOG_LEVEL_ERROR,"ECC_TCPIPRecv inTmpLen = %d,inTmpLen = %d,inTmpLen = %d",inTmpLen,inTCPHeaderLen,inRecvLen);
		if((inTmpLen + inTCPHeaderLen) < inRecvLen)
		{
			inRecvLen = inTmpLen + inTCPHeaderLen - inRecvLen;
			inRetVal = ECC_TCPIPRecv(&bRecvData[inStartLen],&inRecvLen,srTxnData.srParameter.gTCPIPTimeOut);
			if(inRetVal != SUCCESS)
			{
				ECC_TCPIPClose();
				log_msg(LOG_LEVEL_ERROR,"ECC_TCPIPRecv error 3: %d",inRetVal);
				return ICER_ERROR;
			}
		}
	}

	ECC_TCPIPClose();
  #endif

	//log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData inRecvLen = %d",inRecvLen);

#endif

	return inRecvLen;
}

int inTCPIPSendRecv(BYTE *bPCode,unsigned char *IP,unsigned short Port,BYTE *bSendBuf,int inSendLen,BYTE *bRecvBuf,char fConfirm)
{
	int	inRetVal,inTotalSendLen = 0,inRecvLen = 0;
//	int	inECCAdress = 0,inF11Adress = 0;
BYTE bSendData[MAX_XML_FILE_SIZE],bRecvData[MAX_XML_FILE_SIZE];
char chPacketLenFlag;

	memset(bSendData,0x00,sizeof(bSendData));
	memset(bRecvData,0x00,sizeof(bRecvData));
	memcpy(bSendData,bSendBuf,inSendLen);
	inTotalSendLen = inSendLen;
	chPacketLenFlag = chGetchPacketLenFlag();
	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA /*&& fConfirm == TRUE*/)
	{
		if(fConfirm == TRUE)
		{
			memset((char *)&srAdditionalData,0x00,STRUCT_ADD_DATA_LEN);
			memcpy(srAdditionalData.bPCode,bPCode,6);
			srAdditionalData.inMTI = MTI_NORMAL_TXN;
			inRetVal = inAddTcpipDataSend(&srBankXMLData,&srAdditionalData,bSendData,(chPacketLenFlag == TRUE)?bSendBuf:&bSendBuf[2],(chPacketLenFlag == TRUE)?inSendLen:inSendLen - 2);
			if(inRetVal > SUCCESS)
				inTotalSendLen += inRetVal;
		}
		chPacketLenFlag = TRUE;//此參數用來控制CMAS的電文長度,不控制銀行共用規格的電文長度
	}

	vdIcnBankData(1);//1:送失敗也要+1

	//送資料
	if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
		inRetVal = inSSLSendData(IP,Port,(chPacketLenFlag == TRUE)?bSendData:&bSendData[2],(chPacketLenFlag == TRUE)?inTotalSendLen:inTotalSendLen - 2);
	else //if(srTxnData.srParameter.chTCPIP_SSL == TCPIP_PROTOCOL)
		inRetVal = inTCPIPSendData(IP,Port,(chPacketLenFlag == TRUE)?bSendData:&bSendData[2],(chPacketLenFlag == TRUE)?inTotalSendLen:inTotalSendLen - 2);
	
#if	RDEVN == NE_NEXSYS_V3
	/* For QA Debug使用 Start */
	{
		int			inLen = 0;
		TRANSACTION_OBJECT	pobTran;
		
		memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
		pobTran.srTRec.inTicketType = _TICKET_TYPE_ECC_;
		inLen = bSendData[0] * 256 + bSendData[1] - 2;
		if (inLen > 0)
		{
			/* 顯示ISO Debug */
			if (ginDebug == VS_TRUE)
			{
				vdNCCC_TICKET_ISO_FormatDebug_DISP(&pobTran, &bSendData[2], inLen);
			}
			/* 列印ISO Debug */
			if (ginISODebug == VS_TRUE)
			{
				vdNCCC_TICKET_ISO_FormatDebug_PRINT(&pobTran, &bSendData[2], inLen);
			}
		}
	}
	/* For QA Debug使用 End */
#endif
		
	if(inRetVal != SUCCESS)
    {
        log_msg(LOG_LEVEL_ERROR,"inTCPIPSendRecv failed 1 with error: %d",inRetVal);
        return R6_SEND_FAIL;
    }

	if(fConfirm == FALSE)
	{
		//關socket
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
			vdSSLCloseSocket();
		else //if(srTxnData.srParameter.chTCPIP_SSL == TCPIP_PROTOCOL)
		{
			inRetVal = closesocket(ConnectSocket);
			if(inRetVal == SOCKET_ERROR)
			{
				log_msg(LOG_LEVEL_ERROR,"inTCPIPSendRecv close failed with error: %d",WSAGetLastError());
				WSACleanup();
				return R6_SEND_FAIL;
				//return ICER_ERROR;
				//return TCP_CLOSESOCKET_FAIL;
			}

			WSACleanup();
		}
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API//this is mean define LINUX_API
		if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
			vdSSLCloseSocket();
		else
			close(fd);
//#elif READER_MANUFACTURERS==ANDROID_API
//		vdSSLCloseSocket();
#else
		ECC_TCPIPClose();
#endif
		return inRetVal;
	}
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	Sleep(1000);
#else// READER_MANUFACTURERS==LINUX_API
//	sleep(1);
#endif

	vdIcnBankData(2);//2:送成功,收失敗也要+1

	//收資料
	if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
		inRetVal = inSSLRecvData((chPacketLenFlag == TRUE)?bRecvData:&bRecvData[2],(chPacketLenFlag == TRUE)?2:0);
	else //if(srTxnData.srParameter.chTCPIP_SSL == TCPIP_PROTOCOL)
		inRetVal = inTCPIPRecvData((chPacketLenFlag == TRUE)?bRecvData:&bRecvData[2],(chPacketLenFlag == TRUE)?2:0);
	
#if	RDEVN == NE_NEXSYS_V3
	/* For QA Debug使用 Start */
	{
		int			inLen = 0;
		TRANSACTION_OBJECT	pobTran;
		
		memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
		pobTran.srTRec.inTicketType = _TICKET_TYPE_ECC_;
		inLen = bRecvData[0] * 256 + bRecvData[1] - 2;
		if (inLen > 0)
		{
			/* 顯示ISO Debug */
			if (ginDebug == VS_TRUE)
			{
				vdNCCC_TICKET_ISO_FormatDebug_DISP(&pobTran, &bRecvData[2], inLen);
			}
			/* 列印ISO Debug */
			if (ginISODebug == VS_TRUE)
			{
				vdNCCC_TICKET_ISO_FormatDebug_PRINT(&pobTran, &bRecvData[2], inLen);
			}
		}
	}
	/* For QA Debug使用 End */
#endif
		
	if(inRetVal <= SUCCESS)
    {
        log_msg(LOG_LEVEL_ERROR,"inTCPIPSendRecv failed 2 with error: %d",inRetVal);
        return R6_RECV_FAIL;
    }

	if(chPacketLenFlag == FALSE)
	{
		bRecvData[0] = (inRetVal - 3) / 256;
		bRecvData[1] = (inRetVal - 3) % 256;
		inRetVal += 2;
	}

	vdIcnBankData(3);//3:送收成功,但RESPONSE CODE<>00也要+1

	inRecvLen = inRetVal;

	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA && fConfirm == TRUE)
	{
		inRetVal = inAddTcpipDataRecv(&srBankXMLData,&srAdditionalData,bRecvData,bRecvBuf,inRecvLen);
		if(inRetVal >= SUCCESS)
			inRecvLen -= inRetVal;
		else
			return inRetVal;
	}
	else
		memcpy(bRecvBuf,bRecvData,inRecvLen);

	return inRecvLen;

}

int inTXMLSendRecv(BYTE *bPCode,int inMTI,unsigned char *IP,unsigned short Port,BYTE *bSendBuf,int inSendLen,BYTE *bRecvBuf,char fConfirm)
{
int inRetVal = 0,inTotalSendLen = 0,inRecvLen = 0, inNCCCTotalSendLen;
BYTE bSendData[MAX_XML_FILE_SIZE],bRecvData[MAX_XML_FILE_SIZE];
//STRUCT_XML_DOC srBankXMLData;
//STRUCT_ADD_DATA srAdditionalData;
char chPacketLenFlag=0;

	log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 0-0,(%s)(%d)",IP,Port);
	if(inSendLen == 0)
		return SUCCESS;

	memset(bSendData,0x00,sizeof(bSendData));
	memset(bRecvData,0x00,sizeof(bRecvData));
	memcpy(bSendData,bSendBuf,inSendLen);
	inTotalSendLen = inSendLen;
	chPacketLenFlag = chGetchPacketLenFlag();
	log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 0,%d,%d",inRetVal,inTotalSendLen);
	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
	{
		if(inMTI == MTI_NORMAL_TXN/* || inMTI == MTI_UPLOAD_TXN*/)
		{
			memset((char *)&srAdditionalData,0x00,STRUCT_ADD_DATA_LEN);
			memcpy(srAdditionalData.bPCode,bPCode,6);
			srAdditionalData.inMTI = inMTI;
			if(chPacketLenFlag == FALSE)
			{
				inSendLen -= 3;
				inTotalSendLen -= 3;
			}
			inRetVal = inAddTcpipDataSend(&srBankXMLData,&srAdditionalData,bSendData,(chPacketLenFlag == TRUE)?bSendBuf:&bSendBuf[3],inSendLen);
			log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 1,%d",inRetVal);
			if(inRetVal > SUCCESS)
				inTotalSendLen += inRetVal;
		}
		chPacketLenFlag = TRUE;//此參數用來控制CMAS的電文長度,不控制銀行共用規格的電文長度

		if(srTxnData.srParameter.inAdditionalTcpipData == NCCC_ADD_DATA)
		{
			inNCCCTotalSendLen = inNCCCEncryptionData(&srAdditionalData, bSendData);
			if (inNCCCTotalSendLen != 0)
				inTotalSendLen = inNCCCTotalSendLen;
		}
	}

	vdIcnBankData(1);//1:送失敗也要+1

	log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 2(%02x),%d,%d",chPacketLenFlag,inRetVal,inTotalSendLen);
	//送資料
	if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
	{
		inRetVal = inSSLSendData(IP,Port,(chPacketLenFlag == TRUE)?bSendData:&bSendData[3],(chPacketLenFlag == TRUE)?inTotalSendLen:inTotalSendLen - 3);
	}
	else //if(srTxnData.srParameter.chTCPIP_SSL == TCPIP_PROTOCOL)
		inRetVal = inTCPIPSendData(IP,Port,(chPacketLenFlag == TRUE)?bSendData:&bSendData[3],(chPacketLenFlag == TRUE)?inTotalSendLen:inTotalSendLen - 3);
	log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 3,%d",inRetVal);
	
#if	RDEVN == NE_NEXSYS_V3
	/* For QA Debug使用 Start */
	{
		int			inLen = 0;
		TRANSACTION_OBJECT	pobTran;
		
		memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
		pobTran.srTRec.inTicketType = _TICKET_TYPE_ECC_;
		
		inLen = bSendData[0] * 256 + bSendData[1] - 2;
		if (inLen > 0)
		{
			/* 顯示ISO Debug */
			if (ginDebug == VS_TRUE)
			{
				vdNCCC_TICKET_ISO_FormatDebug_DISP(&pobTran, &bSendData[2], inLen);
			}
			/* 列印ISO Debug */
			if (ginISODebug == VS_TRUE)
			{
				vdNCCC_TICKET_ISO_FormatDebug_PRINT(&pobTran, &bSendData[2], inLen);
			}
		}
	}
	/* For QA Debug使用 End */
#endif
	
	if(inRetVal != SUCCESS)
    {
        log_msg(LOG_LEVEL_ERROR,"inTXMLSendRecv failed 1 with error: %d",inRetVal);
        return ICER_SEND_FAIL;
    }

	vdIcnBankData(2);//2:送成功,收失敗也要+1

	//if(fConfirm == FALSE)
	//	return inRetVal;

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		//Sleep(1000);
#else// READER_MANUFACTURERS==LINUX_API
//		sleep(1);
#endif

	//收資料
	if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
		inRetVal = inSSLRecvData((chPacketLenFlag == TRUE)?bRecvData:&bRecvData[3],(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)?2:(chPacketLenFlag == TRUE)?3:0);
	else //if(srTxnData.srParameter.chTCPIP_SSL == TCPIP_PROTOCOL)
		inRetVal = inTCPIPRecvData((chPacketLenFlag == TRUE)?bRecvData:&bRecvData[3],(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)?2:(chPacketLenFlag == TRUE)?3:0);
	log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 4,%d",inRetVal);
	
#if	RDEVN == NE_NEXSYS_V3
	/* For QA Debug使用 Start */
	{
		int			inLen = 0;
		TRANSACTION_OBJECT	pobTran;
		
		memset(&pobTran, 0x00, sizeof(TRANSACTION_OBJECT));
		pobTran.srTRec.inTicketType = _TICKET_TYPE_ECC_;
		
		inLen = bRecvData[0] * 256 + bRecvData[1] - 2;
		if (inLen > 0)
		{
			/* 顯示ISO Debug */
			if (ginDebug == VS_TRUE)
			{
				vdNCCC_TICKET_ISO_FormatDebug_DISP(&pobTran, &bRecvData[2], inLen);
			}
			/* 列印ISO Debug */
			if (ginISODebug == VS_TRUE)
			{
				vdNCCC_TICKET_ISO_FormatDebug_PRINT(&pobTran, &bRecvData[2], inLen);
			}
		}
	}
	/* For QA Debug使用 End */
#endif
	
	if(inRetVal <= SUCCESS)
    {
        log_msg(LOG_LEVEL_ERROR,"inTXMLSendRecv failed 2 with error: %d",inRetVal);
        return ICER_RECV_FAIL;
    }

	if(chPacketLenFlag == FALSE)
	{
		bRecvData[0] = (inRetVal - 3) / 256;
		bRecvData[1] = (inRetVal - 3) % 256;
		bRecvData[2] = 'B';
		inRetVal += 3;
		log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 2: %d",inRetVal);
	}

	vdIcnBankData(3);//3:送收成功,但RESPONSE CODE<>00也要+1

	inRecvLen = inRetVal;

	log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 5,%d",inRetVal);
	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA/* && (inMTI == MTI_NORMAL_TXN || inMTI == MTI_UPLOAD_TXN)*/)
	{
		chPacketLenFlag = chGetchPacketLenFlag();
		inRetVal = inAddTcpipDataRecv(&srBankXMLData,&srAdditionalData,bRecvData,&bRecvBuf[(chPacketLenFlag == TRUE)?0:3],inRecvLen);
		log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 6,%d",inRetVal);
		if(inRetVal >= SUCCESS)
			inRecvLen = inRetVal;
		else if(inRetVal == BANK_DECLINE_FAIL)
			return BANK_DECLINE_FAIL;
		else
			return ICER_RECV_FAIL;
			//return inRetVal;

		if(chPacketLenFlag == FALSE)
		{
			bRecvBuf[0] = inRecvLen / 256;
			bRecvBuf[1] = inRecvLen % 256;
			bRecvBuf[2] = 'B';
			inRecvLen += 3;
		}
	}
	else
		memcpy(bRecvBuf,bRecvData,inRecvLen);

	log_msg(LOG_LEVEL_FLOW,"inTXMLSendRecv 7,%d",inRetVal);
	return inRecvLen;

}

int inTCPSendAndReceive(int inDLLType,unsigned char * sendbuf,unsigned char * recebuf,char fConfirm)
{
int inRetVal,buflen,recelen,inRespCodeAdr,inCardIDAdr;
BYTE bSendBuf[512];

	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		buflen = srTCPIPParL1[inDLLType].inSendLen;
		recelen = srTCPIPParL1[inDLLType].inRecvLen;
		inRespCodeAdr = srTCPIPParL1[inDLLType].inRespCodeAdr;
		inCardIDAdr = srTCPIPParL1[inDLLType].inCardIDAdr;
	}
	else
	{
		buflen = srTCPIPParL2[inDLLType].inSendLen;
		recelen = srTCPIPParL2[inDLLType].inRecvLen;
		inRespCodeAdr = srTCPIPParL2[inDLLType].inRespCodeAdr;
		inCardIDAdr = srTCPIPParL2[inDLLType].inCardIDAdr;
	}

	if(buflen == 0)
	{
		log_msg(LOG_LEVEL_FLOW,"inTCPSendAndReceive buflen == 0");
        return SUCCESS;
	}

	memset(bSendBuf,0x00,sizeof(bSendBuf));
	bSendBuf[0] = ((buflen - 2) & 0xFF00) >> 8;
	bSendBuf[1] = (buflen - 2) & 0xFF;
	if(strlen((char *)srTxnData.srParameter.bMsgHeader) == 8)
		memcpy(&bSendBuf[2],srTxnData.srParameter.bMsgHeader,sizeof(srTxnData.srParameter.bMsgHeader) - 1);
	else
		sprintf((char *)&bSendBuf[2],"9990%03d0",srTxnData.srParameter.usECCPort % 1000);
	memcpy(&bSendBuf[10],sendbuf,buflen - 10);

	vdWriteLog(bSendBuf,buflen,WRITE_R6_SEND_LOG,inDLLType);
	inRetVal = inTCPIPSendRecv(&bSendBuf[2+8+4+3],srTxnData.srParameter.bECCIP,srTxnData.srParameter.usECCPort,bSendBuf,buflen,recebuf,fConfirm);
	if(fConfirm == FALSE)
		return SUCCESS;

	if(inRetVal != recelen)
	{
		if(inRetVal > 0)
			vdWriteLog(recebuf,inRetVal,WRITE_R6_RECV_LOG,inDLLType);
		log_msg(LOG_LEVEL_ERROR,"inTCPSendAndReceive failed 2 error: %d != %d",inRetVal,recelen);

		if(inRetVal == R6_SEND_FAIL)
			return R6_SEND_FAIL;

        return R6_RECV_FAIL;
	}

	vdWriteLog(recebuf,inRetVal,WRITE_R6_RECV_LOG,inDLLType);

	memcpy(srTxnData.srIngData.chR6ResponseCode,&recebuf[inRespCodeAdr],2);
    if(memcmp(&recebuf[inRespCodeAdr],"00",2))
    {
    	log_msg(LOG_LEVEL_ERROR,"Host Decline:[%c%c]",recebuf[inRespCodeAdr],recebuf[inRespCodeAdr + 1]);
		if(!memcmp(&recebuf[inRespCodeAdr],"04",2))
			inLockCard(&recebuf[inCardIDAdr],FALSE);
        return R6_DECLINE_FAIL;
    }

	vdIcnBankData(4);//4:送收成功,且RESPONSE CODE==00才要+1

	return SUCCESS;
}

int inSendRecvXML(int inHostType,STRUCT_XML_DOC *srXML,int inTxnType)
{
BYTE bSendBuf[MAX_XML_FILE_SIZE],bRecvBuf[MAX_XML_FILE_SIZE];
int inRetVal,inRecvLen;
BOOL fReversalFlag = REVERSAL_ON;

	memset(bSendBuf,0x00,sizeof(bSendBuf));
	memset(bRecvBuf,0x00,sizeof(bRecvBuf));

	inRetVal = inReversalProcess(inHostType);
	if(inRetVal != SUCCESS)
	{
		if(inRetVal == ICER_DECLINE_FAIL)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4100,(char *)TAG_NAME_3900,(BYTE *)srTxnData.srIngData.chICERResponseCode,FALSE);

		log_msg(LOG_LEVEL_ERROR,"inSendRecvXML Fail 1:%d",inRetVal);
		return inRetVal;
	}

	if(inTxnType == TXN_SETTLE || inTxnType == TXN_REFUND || inTxnType == TXN_ECC_SETTLE || inTxnType == TXN_ECC_SETTLE2)
	{
		inRetVal = inTCPIPAdviceProcess(inHostType,TRUE);
		if(inRetVal != SUCCESS)
		{
			if(inRetVal == ICER_DECLINE_FAIL)
			inXMLUpdateData(srXML,(char *)TAG_NAME_4100,(char *)TAG_NAME_3900,(BYTE *)srTxnData.srIngData.chICERResponseCode,FALSE);
			
			log_msg(LOG_LEVEL_ERROR,"inSendRecvXML Fail 2:%d",inRetVal);
			return inRetVal;
		}
	}

	if(inTxnType == TXN_SETTLE || inTxnType == TXN_ECC_SETTLE2 || (inTxnType >= TXN_E_READ_BASIC && inTxnType <= TXN_E_VOID_ADD) || (inTxnType >= TXN_E_READ_QR_CODE && inTxnType <= TXN_E_QR_VOID_ADD))
		fReversalFlag = REVERSAL_OFF;

	inRetVal = inBuildSendPackage(inHostType,srXML,bSendBuf,fReversalFlag,ADVICE_OFF,STORE_BATCH_OFF);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inSendRecvXML Fail 3:%d",inRetVal);
		return inRetVal;
	}

	vdWriteLog(bSendBuf,inRetVal,WRITE_XML_SEND_LOG,TXN_NORMAL);
	if(inHostType == HOST_CMAS)
		inRecvLen = inTXMLSendRecv((BYTE *)srTxnData.srIngData.chTMProcessCode,MTI_NORMAL_TXN,srTxnData.srParameter.bCMASIP,srTxnData.srParameter.usCMASPort,bSendBuf,inRetVal,bRecvBuf,TRUE);
	else //if(inHostType == HOST_ICER)
		inRecvLen = inTXMLSendRecv((BYTE *)srTxnData.srIngData.chTMProcessCode,MTI_NORMAL_TXN,srTxnData.srParameter.bICERIP,srTxnData.srParameter.usICERPort,bSendBuf,inRetVal,bRecvBuf,TRUE);
 	if(inRecvLen < SUCCESS)
	{
		if(inRecvLen == BANK_DECLINE_FAIL)	
			vdReversalOff(inHostType);

		log_msg(LOG_LEVEL_ERROR,"inSendRecvXML failed 4 error: %d",inRecvLen);
        return inRecvLen;
	}
	vdWriteLog(bRecvBuf,inRecvLen,WRITE_XML_RECV_LOG,TXN_NORMAL);

	vdFreeXMLDOC(srXML);
	inRetVal = inTCPIPAnalyzePackage(inHostType,TXN_NORMAL,bRecvBuf,inRecvLen,srXML);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inSendRecvXML failed 5:%d",inRetVal);
        return inRetVal;
	}

	//if(inTxnType != TXN_SETTLE && inTxnType != TXN_REFUND)
	//	inTCPIPAdviceProcess(FALSE);

	return SUCCESS;
}

int inReversalProcess(int inHostType)
{
	int	i = 0,inRetVal,inSendLen = 0,inCMASLen = 0,inBefLen = 0;
//	int	inAftLen = 0;
	BYTE bSendBuf[MAX_XML_FILE_SIZE],bRecvBuf[MAX_XML_FILE_SIZE];
	STRUCT_XML_DOC srXMLTmp;
	char chRevFile[MAX_PATH];
	BYTE bIP[MAX_IP_SIZE];
	unsigned short usPort;

	memset(chRevFile,0x00,sizeof(chRevFile));
	memset(bIP,0x00,sizeof(bIP));
	if(inHostType == HOST_CMAS)
	{
		memcpy(chRevFile,FILE_CMAS_REV,strlen(FILE_CMAS_REV));
		memcpy((char *)bIP,(char *)srTxnData.srParameter.bCMASIP,strlen((char *)srTxnData.srParameter.bCMASIP));
		usPort = srTxnData.srParameter.usCMASPort;
	}
	else if(inHostType == HOST_ICER)
	{
		memcpy(chRevFile,FILE_ICER_REV,strlen(FILE_ICER_REV));
		memcpy((char *)bIP,(char *)srTxnData.srParameter.bICERIP,strlen((char *)srTxnData.srParameter.bICERIP));
		usPort = srTxnData.srParameter.usICERPort;
	}
	else
	{
		log_msg(LOG_LEVEL_ERROR,"inReversalProcess failed 0:%d",inHostType);
		return ICER_ERROR;
	}

	inSendLen = inFileGetSize(chRevFile,FALSE);
	log_msg(LOG_LEVEL_FLOW,"inReversalProcess 0-0,%s,%d",chRevFile,inSendLen);//開了Android就當,所以關掉!!
	if(inSendLen <= SUCCESS)
		return SUCCESS;

	inRetVal = inFileRead(chRevFile,bSendBuf,MAX_XML_FILE_SIZE,inSendLen);
	log_msg(LOG_LEVEL_FLOW,"inReversalProcess 0-1,%s,%d",chRevFile,inSendLen);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inReversalProcess failed 1:%d",inRetVal);
		return inRetVal;
	}


	inCMASLen = inSendLen;
	log_msg(LOG_LEVEL_FLOW,"inReversalProcess 1,%d,%d",inCMASLen,inSendLen);
	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
	{
		//BYTE bBuffer[1024];
		BYTE bBuffer[3072];

		memset(bBuffer,0x00,sizeof(bBuffer));
		memcpy((char *)&srAdditionalData,&bSendBuf[inSendLen - STRUCT_ADD_DATA_LEN],STRUCT_ADD_DATA_LEN);
		inSendLen -= STRUCT_ADD_DATA_LEN;
		log_msg(LOG_LEVEL_FLOW,"inReversalProcess 2,%d,%d",inCMASLen,inSendLen);

		if(srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
		{
			bSendBuf[0] = ((((inSendLen - 2) / 100) / 10 ) * 16) + ((inSendLen - 2) / 100) % 10;
			bSendBuf[1] = ((((inSendLen - 2) % 100) / 10 ) * 16) + (inSendLen - 2) % 10;
		}
		else
		{
			bSendBuf[0] = (inSendLen - 2) / 256;
			bSendBuf[1] = (inSendLen - 2) % 256;
		}
		strcat((char *)bBuffer,"Bef 3:[");
		for(i=0;i<srAdditionalData.inBefLen;i++)
		//for(i=0;i<inSendLen;i++)
		{
			sprintf((char *)&bBuffer[strlen((char *)bBuffer)],"%02X",bSendBuf[i]);
			if(!memcmp(&bSendBuf[i + 4],TAG_TRANS_XML_HEADER,strlen(TAG_TRANS_XML_HEADER)))
			{
				log_msg(LOG_LEVEL_FLOW,"inReversalProcess 2-1,%d,[%02x%02x]",i,bSendBuf[0],bSendBuf[1]);
				break;
			}
		}
		strcat((char *)bBuffer,"]");
		//log_msg(LOG_LEVEL_FLOW,"inReversalProcess 2-2,%d[%02x%02x%02x%02x%02x]",strlen((char *)bBuffer),bBuffer[0],bBuffer[1],bBuffer[2],bBuffer[3],bBuffer[4]);
		log_msg(LOG_LEVEL_ERROR,(char *)bBuffer);
		inBefLen = i;
		//inBefLen = ++i;
		inCMASLen = ((bSendBuf[i + 0] / 16) * 1000) +
				    ((bSendBuf[i + 0] % 16) * 100) +
				    ((bSendBuf[i + 1] / 16) * 10) +
				     (bSendBuf[i + 1] % 16);
		inCMASLen += 2;
		log_msg(LOG_LEVEL_FLOW,"inReversalProcess 3,%d,%d,%d,[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]",i,inBefLen,inCMASLen,inSendLen,
			bSendBuf[0],bSendBuf[1],bSendBuf[2],bSendBuf[3],bSendBuf[4],
			bSendBuf[5],bSendBuf[6],bSendBuf[7],bSendBuf[8],bSendBuf[9]);
	}

	vdWriteLog(&bSendBuf[inBefLen],inCMASLen,WRITE_XML_SEND_LOG,TXN_REVERSAL);

	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
	{
		//BYTE bBuffer[1024];
		BYTE bBuffer[3072];

		memset(bBuffer,0x00,sizeof(bBuffer));
		strcat((char *)bBuffer,"Aft 3:[");
		if(inBefLen + inCMASLen < inSendLen)
		{
			for(i=inBefLen + inCMASLen;i<inSendLen;i++)
			{
				sprintf((char *)&bBuffer[strlen((char *)bBuffer)],"%02X",bSendBuf[i]);
			}
			strcat((char *)bBuffer,"]");
		}
		else
			log_msg(LOG_LEVEL_FLOW,"inReversalProcess 4,%d,%d,%d",inBefLen,inCMASLen,inSendLen);
		log_msg(LOG_LEVEL_ERROR,(char *)bBuffer);
	}
	log_msg(LOG_LEVEL_FLOW,"inReversalProcess 5,%s,%d,%s,%d,%d",srTxnData.srIngData.chTMProcessCode,MTI_REVERSAL_TXN,bIP,usPort,inSendLen);

	inRetVal = inTXMLSendRecv((BYTE *)srTxnData.srIngData.chTMProcessCode,MTI_REVERSAL_TXN,bIP,usPort,bSendBuf,inSendLen,bRecvBuf,TRUE);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inReversalProcess failed 2:%d",inRetVal);
        return inRetVal;
	}
	vdWriteLog(bRecvBuf,inRetVal,WRITE_XML_RECV_LOG,TXN_REVERSAL);

	inRetVal = inTCPIPAnalyzePackage(inHostType,TXN_REVERSAL,bRecvBuf,inRetVal,&srXMLTmp);

	vdFreeXMLDOC(&srXMLTmp);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inReversalProcess failed 3:%d",inRetVal);
        return inRetVal;
	}

	vdReversalOff(inHostType);

	return SUCCESS;
}

int inBuildSendPackage(int inHostType,STRUCT_XML_DOC *srXML,BYTE *bOutData,BOOL fReversalFlag,BOOL fAdviceFlag,BOOL fStoreBatchFlag)
{
BOOL fT0100Flag = FALSE;
int i,inCnt = 3,inSize,inReversalAdr = 0,inRetVal;
char *chValue = (char *)NULL;

	for(i=0;i<srXML->inXMLElementCnt;i++)
	{
		//if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE || (srTxnData.srParameter.chOnlineFlag == ICER_ONLINE && srTxnData.srIngData.inTransType != TXN_ADJECT_CARD))
		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE || srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)
		{
			if(!srXML->srXMLElement[i].fXMLSendFlag)
				continue;
		}
		/*else if(srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)
		{
			if(!memcmp(srXML->srXMLElement[i].chTag,(char *)TAG_NAME_0215,strlen(TAG_NAME_0215)))
			{
				if(srXML->srXMLElement[i].shTagLen == (int)strlen(TAG_NAME_0215))
					continue;
			}
			else if(!memcmp(srXML->srXMLElement[i].chTag,(char *)TAG_NAME_0221,strlen(TAG_NAME_0221)))
			{
				if(srXML->srXMLElement[i].shTagLen == (int)strlen(TAG_NAME_0221))
					continue;
			}
		}*/

		if(fAdviceFlag == ADVICE_ON)
		{
			if(!memcmp(srXML->srXMLElement[i].chTag,(char *)TAG_NAME_3900,strlen(TAG_NAME_3900)))
			{
				if(srXML->srXMLElement[i].shTagLen == (int)strlen(TAG_NAME_3900))
					continue;
			}
			else if(!memcmp(srXML->srXMLElement[i].chTag,(char *)TAG_NAME_6401,strlen(TAG_NAME_6401)))
			{
				if(srXML->srXMLElement[i].shTagLen == (int)strlen(TAG_NAME_6401))
					continue;
			}
			else if(!memcmp(srXML->srXMLElement[i].chTag,(char *)TAG_NAME_6409,strlen(TAG_NAME_6409)))
			{
				if(srXML->srXMLElement[i].shTagLen == (int)strlen(TAG_NAME_6409))
					continue;
			}
		}

		bOutData[inCnt++] = '<';
		inSize = srXML->srXMLElement[i].shTagLen;
		memcpy(&bOutData[inCnt],srXML->srXMLElement[i].chTag,inSize);
		inCnt += inSize;
		bOutData[inCnt++] = '>';

		if((inSize == (int)strlen(TAG_NAME_0100)) &&
			(!memcmp(srXML->srXMLElement[i].chTag,(char *)TAG_NAME_0100,strlen(TAG_NAME_0100))))
		{
			inReversalAdr = inCnt;
			fT0100Flag = TRUE;
		}

		if(srXML->srXMLElement[i].shValueLen > 0)
		{
			if(fT0100Flag == TRUE)
			{
				inSize = 4;
				memcpy(&bOutData[inCnt],"0400",inSize);
				inCnt += inSize;
				chValue = srXML->srXMLElement[i].chValue;
				fT0100Flag = FALSE;
			}
			else
			{
				inSize = srXML->srXMLElement[i].shValueLen;
				memcpy(&bOutData[inCnt],srXML->srXMLElement[i].chValue,inSize);
				inCnt += inSize;
			}

			bOutData[inCnt++] = '<';
			bOutData[inCnt++] = '/';
			inSize = srXML->srXMLElement[i].shTagLen;
			memcpy(&bOutData[inCnt],srXML->srXMLElement[i].chTag,inSize);
			inCnt += inSize;
			bOutData[inCnt++] = '>';
		}
	}

	bOutData[0] = (inCnt - 3) / 256;
	bOutData[1] = (inCnt - 3) % 256;
	bOutData[2] = 'B';

	log_msg(LOG_LEVEL_FLOW,"inBuildSendPackage 0:%d",inCnt);

	if(fAdviceFlag == ADVICE_ON)
	{
		if(inHostType == HOST_CMAS)
		{
			log_msg(LOG_LEVEL_FLOW,"inBuildSendPackage 0-1:%d",inReversalAdr);
			log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"bOutData :",10,bOutData,100);
			memcpy(&bOutData[inReversalAdr],chValue,2);
			memcpy(&bOutData[inReversalAdr + 2],"20",2);

			srTxnData.srIngData.ulCMASHostSerialNumber++;
			//vdCMASSerialNumberProcess(srXML,TRUE);
		}
		else //if(inHostType == HOST_ICER)
			memcpy(&bOutData[inReversalAdr],"0220",4);
		inRetVal = inStoreAdviceFile(inHostType,bOutData,inCnt,fStoreBatchFlag,inReversalAdr);
		log_msg(LOG_LEVEL_FLOW,"inBuildSendPackage 1:%d,%d",inRetVal,inCnt);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inBuildSendPackage Fail 1:%d",inRetVal);
			return inRetVal;
		}
	}

	//if(fReversalFlag == REVERSAL_ON)
	if(fReversalFlag == REVERSAL_ON && chValue != (char *)NULL)
	{
		if(inHostType == HOST_CMAS)
		{
			if(!memcmp(chValue,"0200",4) || !memcmp(chValue,"0100",4) || !memcmp(chValue,"0900",4))
			{
				BYTE bSendData[MAX_XML_FILE_SIZE];
				int inTotalCnt = 0;

				memset(bSendData,0x00,sizeof(bSendData));
				memcpy(bSendData,bOutData,inCnt);
				inTotalCnt = inCnt;

				if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
				{
					//STRUCT_ADD_DATA srAdditionalData;
					char chPacketLenFlag = chGetchPacketLenFlag();

					memset((char *)&srAdditionalData,0x00,STRUCT_ADD_DATA_LEN);
					memcpy(srAdditionalData.bPCode,srTxnData.srIngData.chTMProcessCode,6);
					srAdditionalData.inMTI = MTI_REVERSAL_TXN;
					inRetVal = inAddTcpipDataSend(&srBankXMLData,&srAdditionalData,bSendData,(chPacketLenFlag == TRUE)?bOutData:&bOutData[3],(chPacketLenFlag == TRUE)?inCnt:inCnt - 3);
					//inRetVal = inAddTcpipDataSend(&srBankXMLData,&srAdditionalData,bSendData,bOutData,inCnt);
					if(inRetVal > SUCCESS)
					{
						inTotalCnt += inRetVal;
						if(chPacketLenFlag == FALSE)
							inTotalCnt -= 3;
						log_msg(LOG_LEVEL_FLOW,"inBuildSendPackage 2:%d,%d",inRetVal,inTotalCnt);
						memcpy(&bSendData[inTotalCnt],(char *)&srAdditionalData,STRUCT_ADD_DATA_LEN);
						inTotalCnt += STRUCT_ADD_DATA_LEN;
						bSendData[0] = (inTotalCnt - 2) / 256;
						bSendData[1] = (inTotalCnt - 2) % 256;
						log_msg(LOG_LEVEL_FLOW,"inBuildSendPackage 3:%s,%s,%d,%d,%d,%d,%d,%d,%d",srAdditionalData.bPCode,srAdditionalData.bTxnTagName,srAdditionalData.inMTI,srAdditionalData.inBefLen,srAdditionalData.inAftLen,srAdditionalData.inF11Adr,srAdditionalData.inF39Adr,srAdditionalData.inF62Adr,srAdditionalData.inECCAdr);
					}
				}

				inFileWrite((char *)FILE_CMAS_REV,bSendData,inTotalCnt);
			}
		}
		else if(inHostType == HOST_ICER)
		{
			if(!memcmp(chValue,"0200",4))
				inFileWrite((char *)FILE_ICER_REV,bOutData,inCnt);
		}
		else
			log_msg(LOG_LEVEL_ERROR,"inBuildSendPackage Fail 2:%d",inHostType);
	}

	if(chValue != (char *)NULL)
		memcpy(&bOutData[inReversalAdr],chValue,4);

	log_msg(LOG_LEVEL_FLOW,"inBuildSendPackage 4:%d,%d",inRetVal,inCnt);
	return inCnt;
}

int inTCPIPAnalyzePackage(int inHostType,int inTCPIPTxnType,BYTE *bInData,int inLen,STRUCT_XML_DOC *srXMLOut)
{
int inRetVal,inCntStart,inCntEnd,inOffset,inCardStart,inCardEnd;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
UINT64 ui64CardID = 0L;
#else
BYTE ui64CardID[10];
#endif
BYTE bT3903[5],T1101[6 + 1];
//char *chInData;

	//移除inOffset = 0;不然會導致底下inParseXML()時,解錯,將TransXML & Trans都沒解到!!
	//if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
	//	inOffset = 0;
	//else //if(srTxnData.srParameter.chTCPIP_SSL == TCPIP_PROTOCOL)
		inOffset = 3;
	//inOffset = inECCstrchr(bInData,inLen,'<');
	/*{
		int i;
		BYTE bBuf[MAX_XML_FILE_SIZE];

		memset(bBuf,0x00,sizeof(bBuf));
		log_msg(LOG_LEVEL_ERROR,"inTCPIPAnalyzePackage Start(%d)(%d)!!",inLen,inOffset);
		strcat((char *)&bBuf[strlen((char *)bBuf)],"***");
		for(i=0;i<inLen;i++)
		{
			if((bInData[i] >= '0' && bInData[i] <= '9') || (bInData[i] >= 'A' && bInData[i] <= 'Z') || (bInData[i] >= 'a' && bInData[i] <= 'z'))
				bBuf[strlen((char *)bBuf)] = bInData[i];
			else
				sprintf((char *)&bBuf[strlen((char *)bBuf)],"%02X",bInData[i]);
		}
		strcat((char *)&bBuf[strlen((char *)bBuf)],"***");
		log_msg((char *)bBuf);
	}*/
	inRetVal = inParseXML((char *)NULL,(BYTE *)&bInData[inOffset],FALSE,inLen - inOffset,srXMLOut,0);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPAnalyzePackage Fail 1:%d",inRetVal);
		return ICER_RECV_FAIL;
	}

	if((inTCPIPTxnType != TXN_REVERSAL) && (inTCPIPTxnType != TXN_ADVICE))
		vdReversalOff(inHostType);

	memset(T1101,0x00,sizeof(T1101));
	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
	{
		unsigned long ulT1101;
		inRetVal = inXMLGetData(srXMLOut,(char *)TAG_NAME_1100,(char *)T1101,sizeof(T1101) - 1,0);
		if(inRetVal >= SUCCESS)
		{
			ulT1101 = atol((char *)T1101);
			
			if(ulT1101 > srTxnData.srIngData.ulCMASHostSerialNumber)
				srTxnData.srIngData.ulCMASHostSerialNumber = ulT1101;
		}

		if((srTxnData.srIngData.inTransType == TXN_ECC_SIGN_ON || srTxnData.srIngData.inTransType == TXN_ECC_MATCH) && inTCPIPTxnType == TXN_NORMAL)
		{//SignOn且授權交易時,需使用主機時間回覆POS!!
			char chCMASTxDate[8 + 1],chCMASTxTime[6 + 1];

			memset(chCMASTxDate,0x00,sizeof(chCMASTxDate));
			memset(chCMASTxTime,0x00,sizeof(chCMASTxTime));

			inRetVal = inXMLGetData(srXMLOut,(char *)TAG_NAME_1200,(char *)chCMASTxTime,sizeof(chCMASTxTime) - 1,0);
			if(inRetVal >= SUCCESS)
				memcpy(srTxnData.srIngData.chCMASTxTime,chCMASTxTime,sizeof(srTxnData.srIngData.chCMASTxTime) - 1);

			inRetVal = inXMLGetData(srXMLOut,(char *)TAG_NAME_1300,(char *)chCMASTxDate,sizeof(chCMASTxDate) - 1,0);
			if(inRetVal >= SUCCESS)
				memcpy(srTxnData.srIngData.chCMASTxDate,chCMASTxDate,sizeof(srTxnData.srIngData.chCMASTxDate) - 1);
		}
	}

	inRetVal = inXMLSearchValueByTag(srXMLOut,(char *)TAG_NAME_3900,&inCntStart,&inCntEnd,0);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPAnalyzePackage Fail 2(No T3900):%d",inRetVal);
		return ICER_RECV_FAIL;
	}

	memcpy(srTxnData.srIngData.chICERResponseCode,srXMLOut->srXMLElement[inCntStart].chValue,2);

	memset(bT3903,0x00,sizeof(bT3903));
	if(inXMLSearchValueByTag(srXMLOut,(char *)"T3903",&inCntStart,&inCntEnd,0) >= SUCCESS)
		memcpy(bT3903,srXMLOut->srXMLElement[inCntStart].chValue,2);

	if(memcmp(srTxnData.srIngData.chICERResponseCode,"00",2))
    {//非00時
    	if(!memcmp(srTxnData.srIngData.chICERResponseCode,"19",2) && srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		{//CMAS回19時,槫槫需使用CMAS的序號
			memset(T1101,0x00,sizeof(T1101));
			inRetVal = inXMLGetData(srXMLOut,(char *)TAG_NAME_1100,(char *)T1101,sizeof(T1101) - 1,0);
			if(inRetVal >= SUCCESS)
				srTxnData.srIngData.ulCMASHostSerialNumber = atol((char *)T1101);
		}

    	if(!memcmp(srTxnData.srIngData.chICERResponseCode,"04",2) || !memcmp(bT3903,"04",2))
    	{
    		log_msg(LOG_LEVEL_FLOW,"inTCPIPAnalyzePackage 0");
    		inRetVal = inXMLSearchValueByTag(srXMLOut,(char *)TAG_NAME_0200,&inCardStart,&inCardEnd,0);
			if(inRetVal >= SUCCESS)
			{
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAnalyzePackage 1");
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
				//memset((BYTE *)&ui64CardID,0x00,8);
				//ui64CardID = _atoi64(srXMLOut->srXMLElement[inCardStart].chValue);
				ECC_ASCIIToUINT64((BYTE *)srXMLOut->srXMLElement[inCardStart].chValue,(unsigned short)srXMLOut->srXMLElement[inCardStart].shValueLen,(BYTE *)&ui64CardID);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
				//ui64CardID = strtoull(srXMLOut->srXMLElement[inCardStart].chValue,NULL,10);
				ECC_ASCIIToUINT64((BYTE *)srXMLOut->srXMLElement[inCardStart].chValue,(unsigned short)srXMLOut->srXMLElement[inCardStart].shValueLen,(BYTE *)&ui64CardID);
#else
				memset(ui64CardID,0x00,sizeof(ui64CardID));
				ECC_ASCIIToUINT64((BYTE *)srXMLOut->srXMLElement[inCardStart].chValue,(unsigned short)srXMLOut->srXMLElement[inCardStart].shValueLen,ui64CardID);
#endif
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAnalyzePackage 2");
				inRetVal = inLockCard((BYTE *)&ui64CardID,FALSE);
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAnalyzePackage Host Lock Card:[%c%c]",srTxnData.srIngData.chICERResponseCode[0],srTxnData.srIngData.chICERResponseCode[1]);
				if(inRetVal == SUCCESS)
					inRetVal = ECC_LOCK_CARD;

				return inRetVal;
			}
    	}
    	log_msg(LOG_LEVEL_FLOW,"inTCPIPAnalyzePackage Host Decline:[%c%c]",srTxnData.srIngData.chICERResponseCode[0],srTxnData.srIngData.chICERResponseCode[1]);
        return ICER_DECLINE_FAIL;
    }
	log_msg(LOG_LEVEL_FLOW,"inTCPIPAnalyzePackage 3");

	vdIcnBankData(4);//4:送收成功,且RESPONSE CODE==00才要+1

	if(inHostType == HOST_CMAS)
	{
		srTxnData.srIngData.fOnlineHostSuccFlag = TRUE;
		return SUCCESS;
	}

	inRetVal = inCheckICERT6410(srXMLOut);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPAnalyzePackage Fail 2:%d",inRetVal);
		return inRetVal;
	}

	inRetVal = inXMLSearchValueByTag(srXMLOut,(char *)TAG_NAME_6400,&inCntStart,&inCntEnd,0);
	if(inRetVal < SUCCESS)
	{
		srTxnData.srIngData.fOnlineHostSuccFlag = TRUE;
		return SUCCESS;
	}

	if(inCheckHostSTAC(srXMLOut,inCntStart) < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPAnalyzePackage Fail 3:%d",inRetVal);
		return inRetVal;
	}

	if(inTCPIPTxnType == TXN_REVERSAL)
		vdReversalOff(inHostType);
	
	log_msg(LOG_LEVEL_FLOW,"inTCPIPAnalyzePackage 4");

	srTxnData.srIngData.fOnlineHostSuccFlag = TRUE;

	return SUCCESS;
}

int inStoreAdviceFile(int inHostType,BYTE *bInData,int inLen,BOOL fStoreBatchFlag,int inMTIAdr)
{
int inRetVal;
UnionUShort usCnt;
BYTE bBuf[10];
//char chAdvFileName[20];
char chAdvFile[20];
BYTE bSendData[MAX_XML_FILE_SIZE];
int inTotalCnt = 0;

	memset(chAdvFile,0x00,sizeof(chAdvFile));
	if(inHostType == HOST_ICER)
		memcpy(chAdvFile,FILE_ICER_ADV,strlen(FILE_ICER_ADV));
	else if(inHostType == HOST_CMAS)
		memcpy(chAdvFile,FILE_CMAS_ADV,strlen(FILE_CMAS_ADV));
	else
	{
		log_msg(LOG_LEVEL_ERROR,"inStoreAdviceFile (%s) failed 0:%d",chAdvFile,inHostType);
		return ICER_ERROR;
	}

	inRetVal = inFileGetSize(chAdvFile,FALSE);
	//log_msg(LOG_LEVEL_ERROR,"inStoreAdviceFile(%d) chAdvFile = %s",inRetVal,chAdvFile);
	if(inRetVal <= SUCCESS)
	{
		memset(bBuf,0x00,sizeof(bBuf));
	#ifdef ECC_BIG_ENDIAN
		memcpy(bBuf,"\x00\x01\x00\x00",4);
	#else
		memcpy(bBuf,"\x01\x00\x00\x00",4);
	#endif
		inFileAppend(chAdvFile,bBuf,4);
	}
	else
	{
		usCnt.Value = 0;
		inRetVal = inFileRead(chAdvFile,usCnt.Buf,sizeof(usCnt.Buf),sizeof(usCnt.Buf));
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"vdStoreAdviceFile failed 1:%d",inRetVal);
			return inRetVal;
		}

		usCnt.Value++;
		inRetVal = inFileModify(FALSE,FALSE,chAdvFile,usCnt.Buf,sizeof(usCnt.Buf),0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"vdStoreAdviceFile failed 2:%d",inRetVal);
			return inRetVal;
		}
	}

	memset(bSendData,0x00,sizeof(bSendData));
	memcpy(bSendData,bInData,inLen);
	inTotalCnt = inLen;

	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
	{

		if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
		{
			//STRUCT_ADD_DATA srAdditionalData;
			char chPacketLenFlag = chGetchPacketLenFlag();

			memset((char *)&srAdditionalData,0x00,STRUCT_ADD_DATA_LEN);
			memcpy(srAdditionalData.bPCode,srTxnData.srIngData.chTMProcessCode,6);
			srAdditionalData.inMTI = MTI_ADVICE_TXN;
			if(chPacketLenFlag == FALSE)
				inTotalCnt -= 3;
			inRetVal = inAddTcpipDataSend(&srBankXMLData,&srAdditionalData,bSendData,(chPacketLenFlag == TRUE)?bInData:&bInData[3],(chPacketLenFlag == TRUE)?inLen:inLen - 3);
			log_msg(LOG_LEVEL_FLOW,"srAdditionalData 3:%d,%d",inRetVal,inTotalCnt);
			//inRetVal = inAddTcpipDataSend(&srBankXMLData,&srAdditionalData,bSendData,bInData,inLen);
			if(inRetVal > SUCCESS)
			{
				inTotalCnt += inRetVal;
				log_msg(LOG_LEVEL_FLOW,"srAdditionalData 4:%d,%d",inRetVal,inTotalCnt);
				memcpy(&bSendData[inTotalCnt],(char *)&srAdditionalData,STRUCT_ADD_DATA_LEN);
				inTotalCnt += STRUCT_ADD_DATA_LEN;
				bSendData[0] = (inTotalCnt - 2) / 256;
				bSendData[1] = (inTotalCnt - 2) % 256;
				log_msg(LOG_LEVEL_FLOW,"srAdditionalData 5:%s,%s,%d,%d,%d,%d,%d,%d,%d",srAdditionalData.bPCode,srAdditionalData.bTxnTagName,srAdditionalData.inMTI,srAdditionalData.inBefLen,srAdditionalData.inAftLen,srAdditionalData.inF11Adr,srAdditionalData.inF39Adr,srAdditionalData.inF62Adr,srAdditionalData.inECCAdr);
			}
		}
	}

	inRetVal = inFileAppend(chAdvFile,bSendData,inTotalCnt);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"vdStoreAdviceFile failed 6:%d",inRetVal);
		return inRetVal;
	}

	if(fStoreBatchFlag == TRUE)
	{
		int inOffset = 0;
		memset(bSendData,0x00,sizeof(bSendData));
		memcpy(bSendData,bInData,inLen);
		// 銀行併機Batch upload
		memcpy((char *)&bSendData[inMTIAdr],"0300",4);
		inTotalCnt = inLen;

		if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
		{
			//STRUCT_ADD_DATA srAdditionalData;
			char chPacketLenFlag = chGetchPacketLenFlag();
			int inISO8583FlagIndex = 0;

			memset((char *)&srAdditionalData,0x00,STRUCT_ADD_DATA_LEN);
			memcpy(srAdditionalData.bPCode,srTxnData.srIngData.chTMProcessCode,6);
			srAdditionalData.inMTI = MTI_UPLOAD_TXN;
			if(chPacketLenFlag == FALSE)
				inTotalCnt -= 3;
			inISO8583FlagIndex = srTxnData.srIngData.inISO8583FlagIndex;
			inRetVal = inAddTcpipDataSend(&srBankXMLData,&srAdditionalData,bSendData,(chPacketLenFlag == TRUE)?bInData:&bInData[3],(chPacketLenFlag == TRUE)?inLen:inLen - 3);
			srTxnData.srIngData.inISO8583FlagIndex = inISO8583FlagIndex;
			log_msg(LOG_LEVEL_FLOW,"srAdditionalData 7:%d,%d",inRetVal,inTotalCnt);
			//inRetVal = inAddTcpipDataSend(&srBankXMLData,&srAdditionalData,bSendData,bInData,inLen);
			if(inRetVal > SUCCESS)
			{
				inTotalCnt += inRetVal;
				log_msg(LOG_LEVEL_ERROR,"srAdditionalData 8:%d,%d",inRetVal,inTotalCnt);
				memcpy(&bSendData[inTotalCnt],(char *)&srAdditionalData,STRUCT_ADD_DATA_LEN);
				inTotalCnt += STRUCT_ADD_DATA_LEN;
				bSendData[0] = (inTotalCnt - 2) / 256;
				bSendData[1] = (inTotalCnt - 2) % 256;
				log_msg(LOG_LEVEL_ERROR,"srAdditionalData 9:%s,%s,%d,%d,%d,%d,%d,%d,%d",srAdditionalData.bPCode,srAdditionalData.bTxnTagName,srAdditionalData.inMTI,srAdditionalData.inBefLen,srAdditionalData.inAftLen,srAdditionalData.inF11Adr,srAdditionalData.inF39Adr,srAdditionalData.inF62Adr,srAdditionalData.inECCAdr);
			}
		}
		else
			inOffset = 3;

		//不需要Upload!!
		//if(inFileGetSize((char *)FILE_RES_CMAS_BAT,FALSE) > SUCCESS)
		//	inFileWrite((char *)FILE_RES_CMAS2_BAT,&bSendData[inOffset],inTotalCnt - inOffset);
		//else
		//	inFileWrite((char *)FILE_RES_CMAS_BAT,&bSendData[inOffset],inTotalCnt - inOffset);
	}

	vdIcnBankData(7);

	if(srTxnData.srParameter.chCMASAdviceVerify == '1')
	{
		char chAdvFile2[40];

		memset(chAdvFile2,0x00,sizeof(chAdvFile2));
		sprintf(chAdvFile2,"%s%s",srTxnData.srIngData.chTxTime,chAdvFile);
		inFileAppend(chAdvFile2,&bInData[3],inLen - 3);
	}

	return SUCCESS;
}

int inTCPIPAdviceProcess(int inHostType,BOOL fSendAllAdvFile)
{
//#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
int i,inRetVal,inAdvLen,inRecvLen = 0;
unsigned long ulOffset = 0L;
BYTE bBuf[10];
UnionUShort usTotalCnt,usOKCnt;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
FILE *fp=(FILE *)NULL;
#else
int fp;
#endif
BYTE bSendBuf[MAX_XML_FILE_SIZE],bRecvBuf[MAX_XML_FILE_SIZE];
STRUCT_XML_DOC srXMLTmp;
//char chAdvFileName[20];
char chAdvFile[20];
char chAllFileName[2000];
char chTMProcessCode[6 + 1];
//STRUCT_ADD_DATA srAdditionalData;

//	do
//	{
//		memset(chAdvFileName,0x00,sizeof(chAdvFileName));
//		inGetOldAdvFileName(chAdvFileName);

		memset(bBuf,0x00,sizeof(bBuf));
		memset(chAdvFile,0x00,sizeof(chAdvFile));
		memset(chTMProcessCode,0x00,sizeof(chTMProcessCode));
		memset((char *)&srAdditionalData,0x00,STRUCT_ADD_DATA_LEN);

		if(inHostType == HOST_ICER)
			memcpy(chAdvFile,FILE_ICER_ADV,strlen(FILE_ICER_ADV));
		else if(inHostType == HOST_CMAS)
			memcpy(chAdvFile,FILE_CMAS_ADV,strlen(FILE_CMAS_ADV));
		else
		{
			log_msg(LOG_LEVEL_ERROR,"inTCPIPAdviceProcess (%s) failed 0:%d",chAdvFile,inHostType);
			return ICER_ERROR;
		}

		inRetVal = inFileGetSize(chAdvFile,FALSE);
		if(inRetVal <= SUCCESS)//無Advice資料,回傳成功!!
		{
			log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess fail 0,%d",inRetVal);
			return SUCCESS;
		}

		memset(chAllFileName,0x00,sizeof(chAllFileName));
		//if(chPathFlag == FALSE)//相對路徑
		{
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			sprintf(chAllFileName,"%s\\%s",gCurrentFolder,chAdvFile);
	#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
			sprintf(chAllFileName,"%s/%s",gCurrentFolder,chAdvFile);
	#else
			sprintf(chAllFileName,"%s",chAdvFile);
	#endif
		}

	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
		fp = fopen(chAllFileName,"r+b");
		if(fp == NULL)
	#else
		fp = ECC_FileOpen((BYTE *)chAllFileName,(BYTE *)"r+b");
		if(fp < 0)
	#endif
		{
			log_msg(LOG_LEVEL_ERROR,"inTCPIPAdviceProcess (%s) failed 1:%d",chAdvFile,fp);
			return ICER_ERROR;
			//return FILE_OPEN_FAIL;
		}

	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
		inRetVal = fread(bBuf,4,1,fp);
	#else
		inRetVal = ECC_FileRead(fp,bBuf,4);
	#endif

		if(inRetVal < SUCCESS)
		{
			//fclose(fp);
			log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess failed 2:%d",inRetVal);
			//return inRetVal;
		}
		ulOffset += 4;

		memcpy(usTotalCnt.Buf,&bBuf[0],2);
		memcpy(usOKCnt.Buf,&bBuf[2],2);

		for(i=0;i<usTotalCnt.Value;i++)
		{
			log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess 1,%d,%d",inRetVal,usTotalCnt.Value);
			memset(bSendBuf,0x00,sizeof(bSendBuf));
		#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
			inRetVal = fread(bSendBuf,2,1,fp);
		#else
			inRetVal = ECC_FileRead(fp,bSendBuf,2);
		#endif
			if(inRetVal < SUCCESS)
			{
				//fclose(fp);
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess failed 3:%d",inRetVal);
				//return inRetVal;
			}
			ulOffset += 2;
			inAdvLen = (bSendBuf[0] * 256) + bSendBuf[1];
			log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess 2,%d,%d,[%02x%02x]",inAdvLen,usTotalCnt.Value,bSendBuf[0],bSendBuf[1]);
			if(srTxnData.srParameter.inAdditionalTcpipData == NO_ADD_DATA)
				inAdvLen++;
//			log_msg(LOG_LEVEL_ERROR,"inTCPIPAdviceProcess 1:%d,%02x%02x",inAdvLen,bSendBuf[0],bSendBuf[1]);

		#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
			inRetVal = fread(&bSendBuf[2],inAdvLen,1,fp);
		#else
			inRetVal = ECC_FileRead(fp,&bSendBuf[2],inAdvLen);
		#endif

			if(inRetVal < SUCCESS)
			{
				//fclose(fp);
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess failed 4:%d",inRetVal);
				//return inRetVal;
			}

			ulOffset += inAdvLen;
			if(i < usOKCnt.Value)
				continue;

			inAdvLen += 2;
			log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess 3,%d,[%02x%02x]",inAdvLen,bSendBuf[0],bSendBuf[1]);

			if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE && srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
			{
				int inECCLen = 0;
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess 3-0,%d,%d,%d",inAdvLen,STRUCT_ADD_DATA_LEN,STRUCT_ADD_DATA_LEN);
				log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"srAdditionalData :",18,(BYTE *)&bSendBuf[inAdvLen - STRUCT_ADD_DATA_LEN],STRUCT_ADD_DATA_LEN);
				memcpy((char *)&srAdditionalData,&bSendBuf[inAdvLen - STRUCT_ADD_DATA_LEN],STRUCT_ADD_DATA_LEN);
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess 3-0-0,%d",srAdditionalData.inBefLen);
				inECCLen =  ((bSendBuf[srAdditionalData.inBefLen + 0] / 16) * 1000) +
							((bSendBuf[srAdditionalData.inBefLen + 0] % 16) * 100) +
							((bSendBuf[srAdditionalData.inBefLen + 1] / 16) * 10) +
							 (bSendBuf[srAdditionalData.inBefLen + 1] % 16);
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess 3-1,%d",inECCLen);
				inECCLen += 2;
				if(chGetchPacketLenFlag() == TRUE)
				{
					//inECCLen = (bSendBuf[srAdditionalData.inBefLen + 2] * 256) + (bSendBuf[srAdditionalData.inBefLen + 3]);
					inECCLen += 3;
				}
				//log_msg(LOG_LEVEL_ERROR,"srAdditionalData 1:%d,%d,%d,%d",inAdvLen,inECCLen,srAdditionalData.inBefLen,srAdditionalData.inAftLen);
				//inAddTcpipDataModify(&srAdditionalData,bSendBuf,inAdvLen);
				inAdvLen = inAdvLen - STRUCT_ADD_DATA_LEN;
				if(srTxnData.srParameter.inAdditionalTcpipData == FISC_ADD_DATA)
				{
					bSendBuf[0] = ((((inAdvLen - 2) / 100) / 10 ) * 16) + ((inAdvLen - 2) / 100) % 10;
					bSendBuf[1] = ((((inAdvLen - 2) % 100) / 10 ) * 16) + (inAdvLen - 2) % 10;
				}
				else
				{
					bSendBuf[0] = (inAdvLen - 2) / 256;
					bSendBuf[1] = (inAdvLen - 2) % 256;
				}

				log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess 3-2,%d",srAdditionalData.inBefLen);
				if(srAdditionalData.inBefLen > 0)
				{
					BYTE bBuffer[1024];
					int j;

					memset(bBuffer,0x00,sizeof(bBuffer));
					strcat((char *)bBuffer,"Bef 2:[");
					for(j=0;j<srAdditionalData.inBefLen;j++)
					{
						sprintf((char *)&bBuffer[strlen((char *)bBuffer)],"%02X",bSendBuf[j]);
					}
					strcat((char *)bBuffer,"]");
					log_msg(LOG_LEVEL_ERROR,(char *)bBuffer);
				}
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess 3-3,%d",inECCLen);

				vdWriteLog(&bSendBuf[srAdditionalData.inBefLen],inAdvLen - srAdditionalData.inBefLen - srAdditionalData.inAftLen,WRITE_XML_SEND_LOG,TXN_ADVICE);

				if(srAdditionalData.inAftLen > 0)
				{
					BYTE bBuffer[1024];
					int j;

					memset(bBuffer,0x00,sizeof(bBuffer));
					strcat((char *)bBuffer,"Aft 2:[");
					for(j=0;j<srAdditionalData.inAftLen;j++)
					{
						sprintf((char *)&bBuffer[strlen((char *)bBuffer)],"%02X",bSendBuf[inAdvLen - srAdditionalData.inAftLen + j]);
					}
					strcat((char *)bBuffer,"]");
					log_msg(LOG_LEVEL_ERROR,(char *)bBuffer);
				}
			}
			else
			{
				log_msg(LOG_LEVEL_FLOW,"inTCPIPAdviceProcess 3-4,%d,[%02x%02x]",inAdvLen,bSendBuf[0],bSendBuf[1]);
				vdWriteLog(bSendBuf,inAdvLen,WRITE_XML_SEND_LOG,TXN_ADVICE);
			}

			if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
				inRecvLen = inTXMLSendRecv((BYTE *)srAdditionalData.bPCode,MTI_ADVICE_TXN,srTxnData.srParameter.bCMASIP,srTxnData.srParameter.usCMASPort,bSendBuf,inAdvLen,bRecvBuf,TRUE);
			else if(srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)//if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
				inRecvLen = inTXMLSendRecv((BYTE *)srAdditionalData.bPCode,MTI_ADVICE_TXN,srTxnData.srParameter.bICERIP,srTxnData.srParameter.usICERPort,bSendBuf,inAdvLen,bRecvBuf,TRUE);
			if(inRecvLen < SUCCESS)
			{
			#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
				fclose(fp);
			#else
				ECC_FileClose(fp);
			#endif

				log_msg(LOG_LEVEL_ERROR,"inTCPIPAdviceProcess failed 5:%d",inRecvLen);
				return inRecvLen;
			}
			vdWriteLog(bRecvBuf,inRecvLen,WRITE_XML_RECV_LOG,TXN_ADVICE);

			inRetVal = inTCPIPAnalyzePackage(inHostType,TXN_ADVICE,bRecvBuf,inRecvLen,&srXMLTmp);
			vdFreeXMLDOC(&srXMLTmp);
			if(inRetVal < SUCCESS)
			{
			#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
				fclose(fp);
			#else
				ECC_FileClose(fp);
			#endif
				log_msg(LOG_LEVEL_ERROR,"inTCPIPAdviceProcess failed 6:%d",inRetVal);
				return inRetVal;
			}

			usOKCnt.Value++;

			inRetVal = inFileModify(fp,TRUE,chAdvFile,usOKCnt.Buf,sizeof(usOKCnt.Buf),2);
			if(inRetVal < SUCCESS)
			{
			#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
				fclose(fp);
			#else
				ECC_FileClose(fp);
			#endif
				log_msg(LOG_LEVEL_ERROR,"inTCPIPAdviceProcess failed 7:%d",inRetVal);
				return inRetVal;
			}

			if(fSendAllAdvFile == FALSE)
				break;

		#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
			fseek(fp,ulOffset,SEEK_SET);
		#else
			ECC_FileSeek(fp,ulOffset,SEEK_SET);
		#endif
		}

		#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
			fclose(fp);
		#else
			ECC_FileClose(fp);
		#endif

		if(usTotalCnt.Value == usOKCnt.Value)
		{
			inFileDelete(chAdvFile,FALSE);
			return SUCCESS;
		}

		/*inRetVal = inFileModify(chAdvFile,usOKCnt.Buf,sizeof(usOKCnt.Buf),2);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inTCPIPAdviceProcess failed 7:%d",inRetVal);
			return inRetVal;
		}*/
//	}
//	while(fSendAllAdvFile == TRUE);
/*#elif READER_MANUFACTURERS==CS_PAXSXX
#else
#warning "inTCPIPAdviceProcess Not Coding !!"
#endif*/

	return SUCCESS;
}

void vdReversalOn(int inHostType)
{

	log_msg(LOG_LEVEL_FLOW,"vdReversalOn 0");
	if(inHostType == HOST_ICER)
	{
		log_msg(LOG_LEVEL_FLOW,"vdReversalOn 1");
		if(inFileGetSize((char *)FILE_ICER_REV,FALSE) > SUCCESS)
			return;
		log_msg(LOG_LEVEL_FLOW,"vdReversalOn 2");
		inFileRename((char *)FILE_ICER_REV_BAK,(char *)FILE_ICER_REV);
	}
	else if(inHostType == HOST_CMAS)
	{
		log_msg(LOG_LEVEL_FLOW,"vdReversalOn 3");
		if(inFileGetSize((char *)FILE_CMAS_REV,FALSE) > SUCCESS)
			return;
		log_msg(LOG_LEVEL_FLOW,"vdReversalOn 4");
		inFileRename((char *)FILE_CMAS_REV_BAK,(char *)FILE_CMAS_REV);
	}
	else
		log_msg(LOG_LEVEL_ERROR,"vdReversalOn Fail(%d)!!",inHostType);
	log_msg(LOG_LEVEL_FLOW,"vdReversalOn 5");
	//log_msg(LOG_LEVEL_ERROR,"vdReversalOn !!");
}

void vdReversalOff(int inHostType)
{
	//log_msg(LOG_LEVEL_ERROR,"vdReversalOff 0!!");
	if(inHostType == HOST_ICER)
	{
		if(inFileGetSize((char *)FILE_ICER_REV_BAK,FALSE) > SUCCESS)
			inFileDelete((char *)FILE_ICER_REV_BAK,FALSE);
		inFileRename((char *)FILE_ICER_REV,(char *)FILE_ICER_REV_BAK);
	}
	else  if(inHostType == HOST_CMAS)
	{
		if(inFileGetSize((char *)FILE_CMAS_REV_BAK,FALSE) > SUCCESS)
			inFileDelete((char *)FILE_CMAS_REV_BAK,FALSE);
		inFileRename((char *)FILE_CMAS_REV,(char *)FILE_CMAS_REV_BAK);
	}
	else
		log_msg(LOG_LEVEL_ERROR,"vdReversalOff Fail(%d)!!",inHostType);
	//log_msg(LOG_LEVEL_ERROR,"vdReversalOff !!");
}
/*
int inGetNowAdvFileName(char *chOutName)
{
char chDate[20];

	memset(chDate,0x00,sizeof(chDate));
	GetCurrentDateTime((BYTE *)chDate);
	sprintf(chOutName,"ICER00000000.adv");
	memcpy(&chOutName[4],chDate,8);

	return SUCCESS;

}

int inGetOldAdvFileName(char *chOutName)
{
char chDate[20],*chFileName[10];
long hFile= 0x00;
struct _finddata_t c_file;
int i,j;

	memset(chDate,0x00,sizeof(chDate));
	memset(chFileName,0x00,sizeof(chFileName));
	sprintf(chOutName,"ICER????????.adv");

	hFile = _findfirst(chOutName,&c_file); //search file
	if(hFile!=-1)
	{
		i = 0;
		do
		{
			chFileName[i]=(char*)ECC_calloc(1,9);
			memcpy(chFileName[i++],&c_file.name[4],8);
		}
		while(_findnext(hFile,&c_file) == 0);
		_findclose(hFile);
		qsort((void *)chFileName,i,sizeof(chFileName[0]),compare);
		memcpy(&chOutName[4],chFileName[i - 1],8);
		for(j=0;i<i;j++)
			ECC_free(chFileName[j]);
	}
	else
		return ICER_ERROR;

	return SUCCESS;

}
*/

int inCheckHostSTAC(STRUCT_XML_DOC *srXMLOut,int inSTACIndex)
{
int inRetVal;
char chBuf[20];
long lnAmt,lnAddTotalAmt,lnBalTotalAmt;
char chTMLocationID[10 + 1],chTMID[2 + 1];

	inRetVal = 0;
	lnAmt = lnAddTotalAmt = lnBalTotalAmt = 0L;
	do
	{
		memset(chBuf,0x00,sizeof(chBuf));
		inRetVal = inXMLGetAmt(&srXMLData,(char *)TAG_NAME_553204,&lnAmt,inRetVal);
		if(inRetVal < SUCCESS)
			break;
		lnBalTotalAmt += lnAmt;
		inRetVal++;
	}
	while(1);

	inRetVal = 0;
	lnAmt = 0L;
	do
	{
		memset(chBuf,0x00,sizeof(chBuf));
		inRetVal = inXMLGetAmt(&srXMLData,(char *)TAG_NAME_553202,&lnAmt,inRetVal);
		if(inRetVal < SUCCESS)
			break;
		lnAddTotalAmt += lnAmt;
		inRetVal++;
	}
	while(1);

	if(lnAddTotalAmt > lnBalTotalAmt)
	{
		log_msg(LOG_LEVEL_ERROR,"inCheckHostSTAC Fail 1:(%ld) > (%ld)",lnAddTotalAmt,lnBalTotalAmt);
		return HOST_MAC_FAIL;
	}

	//備份chTMLocationID & chTMID
	memset(chTMLocationID,0x00,sizeof(chTMLocationID));
	memset(chTMID,0x00,sizeof(chTMID));
	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
		memcpy(chTMLocationID,srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,6);
	else
		memcpy(chTMLocationID,srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,10);
	memcpy(chTMID,srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID,2);

	//修改chTMLocationID & chTMID

	inRetVal = inGetCardData(0,TRUE);

	//還原chTMLocationID & chTMID
	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
		memcpy(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,chTMLocationID,6);
	else
		memcpy(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,chTMLocationID,10);
	memcpy(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID,chTMID,2);

	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inCheckHostSTAC Fail 2:%d",inRetVal);
		return HOST_MAC_FAIL;
	}

	if(memcmp(srTxnData.srIngData.anMAC,srXMLOut->srXMLElement[inSTACIndex].chValue,16))
	{
		log_msg(LOG_LEVEL_ERROR,"inCheckHostSTAC Fail 3:(%s) != (%s)",srTxnData.srIngData.anMAC,srXMLOut->srXMLElement[inSTACIndex].chValue);
		return HOST_MAC_FAIL;
	}

	return SUCCESS;
}

int inHelloTest(int inHostType)
{
BYTE bSendBuf[100],bRecvData[100],IP[100];
int inSendLen,inRetVal;
unsigned short usPort = 0;

	if(srTxnData.srParameter.chEchoTest != '1')
		return SUCCESS;

	memset(bSendBuf,0x00,sizeof(bSendBuf));
	memset(bRecvData,0x00,sizeof(bRecvData));
	memset(IP,0x00,sizeof(IP));
	if(inHostType == HOST_CMAS)
	{
		memcpy(IP,srTxnData.srParameter.bCMASIP,strlen((char *)srTxnData.srParameter.bCMASIP));
		usPort = srTxnData.srParameter.usCMASPort;
	}
	else
	{
		memcpy(IP,srTxnData.srParameter.bICERIP,strlen((char *)srTxnData.srParameter.bICERIP));
		usPort = srTxnData.srParameter.usICERPort;
	}
	memcpy(bSendBuf,"hello CMAS",10);
	inSendLen = 10;
	inRetVal = inTCPIPSendData(IP,usPort,bSendBuf,inSendLen);
	if(inRetVal != SUCCESS)
    {
        log_msg(LOG_LEVEL_ERROR,"inHelloTest failed 1 with error: %d",inRetVal);
        return ICER_ERROR;
    }
	inRetVal = inTCPIPRecvData(bRecvData,0);
	if(inRetVal != 3 || memcmp(bRecvData,"yes",3))
    {
        log_msg(LOG_LEVEL_ERROR,"inHelloTest failed 2 with error: %d",inRetVal);
        return ICER_ERROR;
    }

	return SUCCESS;
}

int inCMASBatchUploadProcess(BYTE *bSendData,long lnSendLen)
{
int inRetVal,inRecvLen = 0;
BYTE bRecvBuf[MAX_XML_FILE_SIZE];
STRUCT_XML_DOC srXMLTmp;

	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE && srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
	{
		int inECCLen = 0;
		memcpy((char *)&srAdditionalData,&bSendData[lnSendLen - STRUCT_ADD_DATA_LEN],STRUCT_ADD_DATA_LEN);
		inECCLen =  ((bSendData[srAdditionalData.inBefLen + 0] / 16) * 1000) +
					((bSendData[srAdditionalData.inBefLen + 0] % 16) * 100) +
					((bSendData[srAdditionalData.inBefLen + 1] / 16) * 10) +
					 (bSendData[srAdditionalData.inBefLen + 1] % 16);
		inECCLen += 2;
		if(chGetchPacketLenFlag() == TRUE)
		{
			//inECCLen = (bSendBuf[srAdditionalData.inBefLen + 2] * 256) + (bSendBuf[srAdditionalData.inBefLen + 3]);
			inECCLen += 3;
		}
		vdGetISO8583FlagIndex(srAdditionalData.bPCode,srAdditionalData.inMTI);
		//log_msg(LOG_LEVEL_ERROR,"srAdditionalData 1:%d,%d,%d,%d",inAdvLen,inECCLen,srAdditionalData.inBefLen,srAdditionalData.inAftLen);
		inAddTcpipDataModify(&srAdditionalData,bSendData,lnSendLen);
		lnSendLen = lnSendLen - STRUCT_ADD_DATA_LEN;
		bSendData[0] = (BYTE)((lnSendLen - 2) / 256);
		bSendData[1] = (BYTE)((lnSendLen - 2) % 256);

		if(srAdditionalData.inBefLen > 0)
		{
			BYTE bBuffer[1024];
			int j;

			memset(bBuffer,0x00,sizeof(bBuffer));
			strcat((char *)bBuffer,"Bef 4:[");
			for(j=0;j<srAdditionalData.inBefLen;j++)
			{
				sprintf((char *)&bBuffer[strlen((char *)bBuffer)],"%02X",bSendData[j]);
			}
			strcat((char *)bBuffer,"]");
			log_msg(LOG_LEVEL_ERROR,(char *)bBuffer);
		}

		vdWriteLog(&bSendData[srAdditionalData.inBefLen],lnSendLen - srAdditionalData.inBefLen - srAdditionalData.inAftLen,WRITE_XML_SEND_LOG,TXN_UPLOAD);

		if(srAdditionalData.inAftLen > 0)
		{
			BYTE bBuffer[1024];
			int j;

			memset(bBuffer,0x00,sizeof(bBuffer));
			strcat((char *)bBuffer,"Aft 4:[");
			for(j=0;j<srAdditionalData.inAftLen;j++)
			{
				sprintf((char *)&bBuffer[strlen((char *)bBuffer)],"%02X",bSendData[lnSendLen - srAdditionalData.inAftLen + j]);
			}
			strcat((char *)bBuffer,"]");
			log_msg(LOG_LEVEL_ERROR,(char *)bBuffer);
		}
	}
	else
	{
		bSendData[0] = (BYTE)(lnSendLen / 256);
		bSendData[1] = (BYTE)(lnSendLen % 256);
		bSendData[2] = 'B';

		vdWriteLog(bSendData,lnSendLen,WRITE_XML_SEND_LOG,TXN_UPLOAD);
	}
	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		inRecvLen = inTXMLSendRecv((BYTE *)srTxnData.srIngData.chTMProcessCode,MTI_UPLOAD_TXN,srTxnData.srParameter.bCMASIP,srTxnData.srParameter.usCMASPort,bSendData,lnSendLen,bRecvBuf,TRUE);
	else if(srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)//if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
		inRecvLen = inTXMLSendRecv((BYTE *)srTxnData.srIngData.chTMProcessCode,MTI_UPLOAD_TXN,srTxnData.srParameter.bICERIP,srTxnData.srParameter.usICERPort,bSendData,lnSendLen,bRecvBuf,TRUE);
	if(inRecvLen < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPAdviceProcess failed 5:%d",inRecvLen);
		return inRecvLen;
	}
	vdWriteLog(bRecvBuf,inRecvLen,WRITE_XML_RECV_LOG,TXN_UPLOAD);

	inRetVal = inTCPIPAnalyzePackage(HOST_CMAS,TXN_ADVICE,bRecvBuf,inRecvLen,&srXMLTmp);
	vdFreeXMLDOC(&srXMLTmp);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPAdviceProcess failed 6:%d",inRetVal);
		return inRetVal;
	}

	return SUCCESS;
}

char chGetchPacketLenFlag()
{
char chFlag = 0x00;

	if(srTxnData.srParameter.PacketLenFlag == '0')
		chFlag = FALSE;
	else if(srTxnData.srParameter.PacketLenFlag == '1')
		chFlag = TRUE;
	else
	{
		if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
			chFlag = FALSE;
		else //if(srTxnData.srParameter.chTCPIP_SSL == TCPIP_PROTOCOL)
		chFlag = TRUE;
	}

	return chFlag;
}

int inTCPIPRecvByLen(BYTE *lpBuf,unsigned long dwToRead,clock_t tTimeout)
{
//unsigned long dwRead = 0;
unsigned long dwTotalRead = 0;
unsigned short inRetVal,inReadLen = 0;

	while(1)
	{
		inReadLen = (unsigned short )(dwToRead - dwTotalRead);
		inRetVal = ECC_TCPIPRecv(&lpBuf[dwTotalRead],&inReadLen,(unsigned short)tTimeout);
		if(inRetVal != SUCCESS || (int)inReadLen < 0)
		{
			ECC_TCPIPClose();
			log_msg(LOG_LEVEL_ERROR,"inTCPIPRecvData error 1: %d,inRecvLen = %d",inRetVal,inReadLen);
			return ICER_ERROR;
		}

		/*{
			int i;
			BYTE bBuf[1000];

			memset(bBuf,0x00,sizeof(bBuf));

			sprintf((char *)bBuf,"inTCPIPRecvByLen (%d),[",dwRead);
			for(i=0;i<(int)dwRead;i++)
			{
				sprintf((char *)&bBuf[strlen((char *)bBuf)],"%02x",lpBuf[dwTotalRead + i]);
			}
			strcat((char *)bBuf,"]");
			log_msg(LOG_LEVEL_FLOW,(char *)bBuf);
		}*/
		dwTotalRead += inReadLen;
		if(dwTotalRead >= dwToRead)
			break;
	}

	return dwTotalRead;
}

int inCheckICERT6410(STRUCT_XML_DOC *srXML)
{
int inRetVal,inCnt = 0,inCntStart = 0,inCntEnd = 0;
BYTE bBuf[200],bTmp[200],bT6410Data[200],T6410[20],bPCode[10];

	memset(bT6410Data,0x00,sizeof(bT6410Data));
	memset(T6410,0x00,sizeof(T6410));
	memset(bPCode,0x00,sizeof(bPCode));

	//T6410
	inRetVal = inXMLSearchValueByTag(srXML,(char *)"T6410",&inCntStart,&inCntEnd,inCnt);
	if(inRetVal < 0)
	{
		log_msg(LOG_LEVEL_FLOW,"inCheckICERT6410 No T6410");
		return SUCCESS;
	}
	memcpy(T6410,srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);


	//T0211
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_0211,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T0300
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_0300,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy(bPCode,srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	if(memcmp(bPCode,"200163",6))
	{
		//T0400
		inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_0400,&inCntStart,&inCntEnd,inCnt);
		if(inRetVal >= 0)
		{
			memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
			inCnt = inCntStart;
		}

		//T0437
		if(!memcmp(bPCode,"801063",6) || !memcmp(bPCode,"801065",6))
		{
			inRetVal = inXMLSearchValueByTag(srXML,(char *)"T0437",&inCntStart,&inCntEnd,inCnt);
			if(inRetVal >= 0)
			{
				memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
				inCnt = inCntStart;
			}
		}

		//T0438
		inRetVal = inXMLSearchValueByTag(srXML,(char *)"T0438",&inCntStart,&inCntEnd,inCnt);
		if(inRetVal >= 0)
		{
			memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
			inCnt = inCntStart;
		}
	}

	//T0440
	inRetVal = inXMLSearchValueByTag(srXML,(char *)"T0440",&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T0441
	inRetVal = inXMLSearchValueByTag(srXML,(char *)"T0441",&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T1100
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_1100,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	if(!memcmp(bPCode,"226300",6))
	{	//T1103
		inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_1103,&inCntStart,&inCntEnd,inCnt);
		if(inRetVal >= 0)
		{
			memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
			inCnt = inCntStart;
		}
	}

	//T1107
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_1107,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T1200
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_1200,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T1300
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_1300,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T3700
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_3700,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T3800
	inRetVal = inXMLSearchValueByTag(srXML,(char *)"T3800",&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T3900
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_3900,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T4100
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_4100,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T4200
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_4200,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	//T5503
	inRetVal = inXMLSearchValueByTag(srXML,(char *)TAG_NAME_5503,&inCntStart,&inCntEnd,inCnt);
	if(inRetVal >= 0)
	{
		memcpy((char *)&bT6410Data[strlen((char *)bT6410Data)],srXML->srXMLElement[inCntStart].chValue,srXML->srXMLElement[inCntStart].shValueLen);
		inCnt = inCntStart;
	}

	memset(bBuf,0x00,sizeof(bBuf));
	memset(bTmp,0x00,sizeof(bTmp));
	sprintf((char *)bBuf,"0000000000000000");

	PKCS5Padding(bT6410Data,strlen((char *)bT6410Data),16);
	inRetVal = strlen((char *)bT6410Data);
	log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"bICERKey :",10,srTxnData.srParameter.bICERKey,strlen((char *)srTxnData.srParameter.bICERKey));
	log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"bT6410Data :",12,bT6410Data,inRetVal);
	ICER_AES_CBC_Func(0, srTxnData.srParameter.bICERKey, 2, bBuf, bT6410Data, inRetVal, bTmp);
	fnUnPack(&bTmp[inRetVal - 8],8,bBuf);
	if(memcmp(bBuf,T6410,16))
	{
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"bBuf :",6,bBuf,16);
		log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"T6410 :",7,T6410,16);
		log_msg(LOG_LEVEL_ERROR,"inCheckICERT6410 Fail 1:");
		return HOST_MAC_FAIL;
	}

	return SUCCESS;
}

int inTCPIPServerListen(unsigned short Port)
{
#if READER_MANUFACTURERS==WINDOWS_API

#elif READER_MANUFACTURERS==WINDOWS_CE_API

#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API

struct sockaddr_in addr;
//fd_set readfds;
//int fd,newsockfd,addr_len = sizeof(struct sockaddr_in);
//char buffer[256];

	log_msg(LOG_LEVEL_FLOW,"inTCPIPServerListen 1,%d !!",Port);
	if((sockfd_server = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPServerListen socket Fail : %d",sockfd_server);
		return ICER_ERROR;
	}

	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	log_msg(LOG_LEVEL_FLOW,"inTCPIPServerListen 2,%d !!",sockfd_server);
	if(bind(sockfd_server,(struct sockaddr*)&addr,sizeof(addr)) < 0)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPServerListen bind Fail !!");
		return ICER_ERROR;
	}

	log_msg(LOG_LEVEL_FLOW,"inTCPIPServerListen 3,%d !!",sockfd_server);
	if(listen(sockfd_server,3) < 0)
	{
		log_msg(LOG_LEVEL_ERROR,"inTCPIPServerListen listen Fail !!");
		return ICER_ERROR;
	}

  #if 0
	while(1)
	{
		int inLen = 0;

		log_msg(LOG_LEVEL_ERROR,"inTCPIPServerListen 10 !!");
		if((newsockfd = accept(sockfd_server,(struct sockaddr*)&addr,(socklen_t*)&addr_len)) < 0)
			log_msg(LOG_LEVEL_ERROR,"inTCPIPServerListen accept Fail ,%d!!",newsockfd);
		log_msg(LOG_LEVEL_ERROR,"inTCPIPServerListen 10-1,%d !!",newsockfd);

		while(1)
		{
			if((inLen = recv(newsockfd,buffer,sizeof(buffer),0)) <= 0)
			{
				log_msg(LOG_LEVEL_ERROR,"inTCPIPServerListen connect closed!!");
				close(newsockfd);
				break;
			}
			else
				log_msg_debug(LOG_LEVEL_ERROR,TRUE,(char *)"TCPIP Read",10,(BYTE *)buffer,inLen);

			//usleep(100000);
			inLen = send(newsockfd,"Welcome to server!",18,0);
			log_msg(LOG_LEVEL_ERROR,"inTCPIPServerListen 11,%d,%d !!",newsockfd,inLen);
			//is_connected[newsockfd] = 1;
		}
	}

	close(newsockfd);
	close(sockfd_server);


  #endif

#else

#endif

	log_msg(LOG_LEVEL_FLOW,"inTCPIPServerListen 12 !!");
	return SUCCESS;
}

