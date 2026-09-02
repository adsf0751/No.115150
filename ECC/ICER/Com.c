
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
//#include "Global.h"
#if READER_MANUFACTURERS==ANDROID_API
#include<icerapi_icerapi_Api.h>
#include <android/log.h>
extern JNIEnv *Jenv;
extern jobject Jni_usbManager,Jni_context;
jobject comObj;
jmethodID comInit,AndroidComOpen,AndroidComSend,AndroidComRead,AndroidComClose;
#endif

//extern STRUCT_TXN_DATA srTxnData;

#if READER_MANUFACTURERS==WINDOWS_API ||READER_MANUFACTURERS==WINDOWS_CE_API
HANDLE hDev;
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
static int fd_ = -1;
//static int fd2_ = -1;
static const char* dev_ = 0;
int usb_dev = FALSE;

#endif

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
HANDLE hUSB[10]  = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
DWORD instance=0x00;
#endif

/* ---------------------------------------------------------------------
  Open and Setting Com Port .
	Input parameters:
		. nPort -- com port
		. lBaud -- Baud rate (on NST Project is all of 57600)
	Returned status:
		. >= 0 if OK
		. < 0 on error
--------------------------------------------------------------------- */
int inComOpen(void)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
#if READER_MANUFACTURERS==WINDOWS_CE_API
WCHAR		szPort[30];
#else
char		szPort[30]   ;//Com Port
#endif
DCB			  dcb      ;//設備控制區塊
DWORD		  err      ;//錯誤函數回傳值
COMMTIMEOUTS  TimeOuts ;

#if READER_MANUFACTURERS==WINDOWS_API

  #ifdef CTOS_USB
	if (srTxnData.srParameter.gComPort == 0) //USB
	{
		DWORD count;
		DWORD dRetVal;

		dRetVal = EzrUSBSearchDevice(&count);
		if( !dRetVal )
		{
			log_msg(LOG_LEVEL_ERROR,"fnComOpen USB PORT_DEVICE_ERROR",(BYTE *)&dRetVal,4);
			return PORT_DEVICE_ERROR;
		}

		if(hUSB[instance] == NULL)
			hUSB[instance] = EzrUSBOpen(instance);

		if (hUSB[instance] == NULL)
			return PORT_OPENED_ERROR;

		return SUCCESS;
	}
	else
  #endif//CTOS_USB
	{
#endif //#if READER_MANUFACTURERS==WINDOWS_API
		memset(szPort,0,sizeof(szPort));

		hDev=INVALID_HANDLE_VALUE;
		if( hDev != INVALID_HANDLE_VALUE ) return PORT_NOTOPEN_ERROR;


		#if READER_MANUFACTURERS==WINDOWS_CE_API
			wsprintf(szPort,L"CXM%d:",srTxnData.srParameter.gComPort);
			hDev=CreateFileW(szPort,                    //Port
						   GENERIC_READ|GENERIC_WRITE, //讀取模式
						   0,                          //分享串列埠:0不分享
						   NULL,                       //Handel是否可繼承:NULL不繼承
						   OPEN_EXISTING,              //檔案開啟模式
						   0,       //啟用非同步傳輸:0為不啟用
						   NULL);                      //使用串列:0
		#elif READER_MANUFACTURERS==WINDOWS_API
			sprintf(szPort,"\\\\.\\COM%d",srTxnData.srParameter.gComPort);
			hDev=CreateFile(szPort,                    //Port
						   GENERIC_READ|GENERIC_WRITE, //讀取模式
						   0,                          //分享串列埠:0不分享
						   NULL,                       //Handel是否可繼承:NULL不繼承
						   OPEN_EXISTING,              //檔案開啟模式
						   0,       //啟用非同步傳輸:0為不啟用
						   NULL);                      //使用串列:0
		#endif

		if(hDev==INVALID_HANDLE_VALUE)
		{
			err = GetLastError();
		#if READER_MANUFACTURERS==WINDOWS_CE_API
			log_msg(LOG_LEVEL_ERROR,"inComOpen failed 1 %d,%d,%S",hDev, err,szPort);
		#else
			log_msg(LOG_LEVEL_ERROR,"inComOpen failed 1 %d,%s", err,szPort);
		#endif
			if(err!=5)
				return PORT_DEVICE_ERROR; //5為拒訪問
			return PORT_OPENED_ERROR;
		}

	#if READER_MANUFACTURERS==WINDOWS_API
		//If the function succeeds, the return value is nonzero.
		if (!SetupComm(hDev,4096,4096))   //輸入及輸出緩衝區大小都是4096
		{
			log_msg(LOG_LEVEL_ERROR,"inComOpen failed 2");
			return PORT_SETCOMM_ERROR;
		}
	#endif

		//If the function succeeds, the return value is nonzero.
		if (!PurgeComm(hDev, PURGE_TXABORT| //終止目前所有的傳輸的工作，立即返回，不管是否完成傳輸的動作
						PURGE_RXABORT| //終止目前所有的讀取的工作，立即返回，不管是否完成讀取的動作
						PURGE_TXCLEAR| //清除傳送暫存區中的所有資料
						PURGE_RXCLEAR)) //清除接收暫存區中的所有資料
		{
			log_msg(LOG_LEVEL_ERROR,"inComOpen failed 3");
			return PORT_PURGECOMM_ERROR;
		}

		//If the function succeeds, the return value is nonzero.
		if (!GetCommTimeouts(hDev,&TimeOuts))
		{
			log_msg(LOG_LEVEL_ERROR,"inComOpen failed 4");
			return PORT_GETCOMTIMEOUT_ERROR;
		}

		//  單位都是ms
		//ReadTotalTimeout= ReadTotalTimeOutMultiplier* BytesToRead+ ReadTotalTimeoutConstant
		TimeOuts.ReadIntervalTimeout=10;         //接收時每個byte跟byte的間隔時間 MAXDWORD
		TimeOuts.ReadTotalTimeoutMultiplier=20;   //讀取時間系數
		TimeOuts.ReadTotalTimeoutConstant=500;    //讀取時間常量
		TimeOuts.WriteTotalTimeoutMultiplier=20;  //寫入時間系數
		TimeOuts.WriteTotalTimeoutConstant=500;   //寫入時間常量

		//If the function succeeds, the return value is nonzero.
		if (!SetCommTimeouts(hDev,&TimeOuts))//設置超
		{
			log_msg(LOG_LEVEL_ERROR,"inComOpen failed 5");
			return PORT_SETCOMTIMEOUT_ERROR;
		}

		dcb.DCBlength = sizeof(DCB);   //結構長度

		//If the function succeeds, the return value is nonzero.
		if(!GetCommState(hDev,&dcb)) //取得通訊埠參數
		{
			log_msg(LOG_LEVEL_ERROR,"inComOpen failed 6");
			return PORT_GETCOMMSTATE_ERROR;
		}
		else
		{
			dcb.BaudRate		= srTxnData.srParameter.ulBaudRate;               // 通訊速度:NST 57600 / Dongle 115200
			dcb.fBinary			= TRUE;                // 是否採二進位方式傳送:1
			dcb.fParity			= FALSE;               // 是否極性位元檢查:1採用
			dcb.fOutxCtsFlow	= FALSE;               // 是否採用CTS硬體流量管制
			dcb.fOutxDsrFlow	= FALSE;               // 是否採用DSR硬體流量管制
			dcb.fDtrControl		= DTR_CONTROL_DISABLE;  // 是否採用DTR硬體流量管制:串列開啟後為高電位
			//dcb.fDtrControl		= DTR_CONTROL_ENABLE;  // 是否採用DTR硬體流量管制:串列開啟後為高電位
			dcb.fDsrSensitivity	= FALSE;
			dcb.fOutX			= FALSE;               // 是否採用軟體交握
			dcb.fInX			= FALSE;
			dcb.fNull			= FALSE;
			dcb.fRtsControl		= RTS_CONTROL_DISABLE;  // 是否採用RTS硬體流量管制:串列開啟後為高電位
			//dcb.fRtsControl		= RTS_CONTROL_ENABLE;  // 是否採用RTS硬體流量管制:串列開啟後為高電位
			dcb.fAbortOnError	= FALSE;
			if(srTxnData.srParameter.bRS232Parameter[0] >= '5' && srTxnData.srParameter.bRS232Parameter[0] <= '8')
				dcb.ByteSize		= srTxnData.srParameter.bRS232Parameter[0] - 0x30;
			else
				dcb.ByteSize		= 8;                   //資料位元數
			switch(srTxnData.srParameter.bRS232Parameter[1])
			{
				case 'N':
					dcb.Parity			= NOPARITY;
					break;
				case 'E':
					dcb.Parity			= EVENPARITY;
					break;
				case 'M':
					dcb.Parity			= MARKPARITY;
					break;
				case 'O':
					dcb.Parity			= ODDPARITY;
					break;
				case 'S':
					dcb.Parity			= SPACEPARITY;
					break;
				default:
					dcb.Parity			= NOPARITY;            //極性位元
					break;
			}
			if(srTxnData.srParameter.bRS232Parameter[2] >= '1' && srTxnData.srParameter.bRS232Parameter[2] <= '3')
				dcb.StopBits		= srTxnData.srParameter.bRS232Parameter[2] - 0x31;
			else
				dcb.StopBits		= ONESTOPBIT;          //停止位元
		}

		//If the function succeeds, the return value is nonzero.
		if(!SetCommState(hDev,&dcb))
		{
			log_msg(LOG_LEVEL_ERROR,"inComOpen failed 7");
			return PORT_SETCOMMSTATE_ERROR;
		}

#if READER_MANUFACTURERS==WINDOWS_API
	}
#endif //#if READER_MANUFACTURERS==WINDOWS_API

	return SUCCESS;
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
//#elif (READER_MANUFACTURERS==LINUX_API && RDEVN==NE_X86) || READER_MANUFACTURERS==ANDROID_API

  #if READER_MANUFACTURERS==ANDROID_API
	if(srTxnData.srParameter.gReaderMode == USB_TO_RS232_READER || srTxnData.srParameter.gReaderMode == USB_READER)
	{
		jclass comClazz;
		jstring appName;
		jint resultI;
        //log_msg(LOG_LEVEL_ERROR,"call Android API");
		appName = (*Jenv)->NewStringUTF(Jenv,"icerapi.icerapi.api");
		if(srTxnData.srParameter.gReaderMode == USB_READER)
		    comClazz = (*Jenv) -> FindClass(Jenv,"icerapi/icerapi/ComUSB");
		else
		{
			log_msg(LOG_LEVEL_FLOW,"srTxnData.srParameter.gUsbToRs232Chip = %d", srTxnData.srParameter.gUsbToRs232Chip);
			if (srTxnData.srParameter.gUsbToRs232Chip == 1)
			{
				comClazz = (*Jenv) -> FindClass(Jenv,"icerapi/icerapi/ComRS232_GC");
			}
			else
			{
		   		comClazz = (*Jenv) -> FindClass(Jenv,"icerapi/icerapi/ComRS232");
			}
		}
		comInit = (*Jenv) -> GetMethodID(Jenv,comClazz,"<init>","(Landroid/hardware/usb/UsbManager;Landroid/content/Context;Ljava/lang/String;)V");
		comObj = (*Jenv) -> NewObject(Jenv,comClazz,comInit,Jni_usbManager,Jni_context,appName);

		if(srTxnData.srParameter.gReaderMode == USB_TO_RS232_READER)
		{
			//RS232-Com Open
			AndroidComOpen = (*Jenv) -> GetMethodID(Jenv,comClazz,"ComOpen","(III[BI)I");//小USB --> RS232
			jint rate = 115200;
			jint timeout = 1000;
			jbyteArray jSendData=(*Jenv)->NewByteArray(Jenv, 1);

			(*Jenv)->SetByteArrayRegion(Jenv, jSendData, 0, 1, (jbyte*)&srTxnData.srParameter.bRS232Parameter[1]);
			resultI = (*Jenv) -> CallIntMethod(Jenv,comObj,AndroidComOpen,
						rate,
						timeout,
						srTxnData.srParameter.bRS232Parameter[0] - 0x30,
						jSendData,
						srTxnData.srParameter.bRS232Parameter[2] - 0x30);
		}
		else if(srTxnData.srParameter.gReaderMode == USB_READER)
		{
			int xProxVID = 3238,xProxPID = 40976,xProxBusID,xProxDeviceID;
			BOOL fFlag = FALSE;

			fFlag = fGetUSBInfoByPar(&xProxBusID,&xProxDeviceID);
			if(fFlag == TRUE)
			{
				AndroidComOpen = (*Jenv) -> GetMethodID(Jenv,comClazz,"ComOpen2","(IIII)I");//小USB --> RS232
				resultI = (*Jenv) -> CallIntMethod(Jenv,comObj,AndroidComOpen,xProxVID,xProxPID,xProxBusID,xProxDeviceID);
			}
			else
			{
				AndroidComOpen = (*Jenv) -> GetMethodID(Jenv,comClazz,"ComOpen1","(II)I");//小USB --> RS232
				resultI = (*Jenv) -> CallIntMethod(Jenv,comObj,AndroidComOpen,xProxVID,xProxPID);
			}
		}

		if(resultI != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"AndroidComOpen fail: %d",resultI);
			return PORT_OPENED_ERROR;
		}

		//RS232-Com Write
		AndroidComSend = (*Jenv) -> GetMethodID(Jenv,comClazz,"ComWrite","([B)I");

		//RS232-Com Read
		AndroidComRead = (*Jenv) -> GetMethodID(Jenv,comClazz,"ComRead","(I)[B");

		//RS232-Com Close
		AndroidComClose   = (*Jenv) -> GetMethodID(Jenv,comClazz,"ComClose","()I");
		//
		srTxnData.srIngData.fAPIOpenFlag = TRUE;
		return SUCCESS;
	}
	else

  #endif//READER_MANUFACTURERS==ANDROID_API

	if(srTxnData.srParameter.gReaderMode == RS232_READER
  #if READER_MANUFACTURERS==LINUX_API
	|| srTxnData.srParameter.gReaderMode == USB_READER
  #endif//READER_MANUFACTURERS==LINUX_API
		)
	{
		int fd;
		struct termios ios;
		char device[20] ;//Com Port
	#if READER_MANUFACTURERS==LINUX_API
//		DWORD count=0;
	#endif

		memset(device,0x00,sizeof(device));
		memcpy(device,(char *)srTxnData.srParameter.bComPort,strlen((char *)srTxnData.srParameter.bComPort));
		TRACE_("open_dev (%s)",device);
		if (!strlen(device)) {
			return DONGLE_PARGUMENTS_ERROR;
		}
		inComClose();

	#ifdef CTOS_USB
	//#if (READER_MANUFACTURERS==LINUX_API && RDEVN==NE_X86)
		if(srTxnData.srParameter.gReaderMode == USB_READER)
		//if(strcasecmp((char *)srTxnData.srParameter.bComPort,"usb") == 0 )
		{
			int xProxVID = 0x0CA6,xProxPID = 0xA010,xProxBusID,xProxDeviceID;

			fGetUSBInfoByPar(&xProxBusID,&xProxDeviceID);

			if(EzrUSBSearchDeviceEx2(&count ,xProxPID ,xProxVID ,xProxBusID ,xProxDeviceID))
			//DWORD count=0;
			//if(EzrUSBSearchDevice(&count))
			{
				fd=EzrUSBOpen(1);
				if(fd==d_USB_NOT_EXIST ) {
					log_msg(LOG_LEVEL_ERROR,"open %s failed(%d,%d): ", device,xProxBusID ,xProxDeviceID);
					return PORT_DEVICE_ERROR;
				}
				else
					usb_dev=TRUE;
			}
			else
				log_msg(LOG_LEVEL_ERROR,"%s not exist(%d,%d,%d): ", device,count,xProxBusID ,xProxDeviceID);
		}
		else if(srTxnData.srParameter.gReaderMode == RS232_READER && !strcasecmp((char *)srTxnData.srParameter.bComPort,"usb"))
		{
			if(EzrUSBSearchDevice(&count))
			{
				fd=EzrUSBOpen(1);
				if(fd==d_USB_NOT_EXIST ) {
					log_msg(LOG_LEVEL_ERROR,"open %s failed: ", device);
					return PORT_DEVICE_ERROR;
				}
				else
					usb_dev=TRUE;
			}
			else
				log_msg(LOG_LEVEL_ERROR,"%s not exist: ", device);
		}
		else
	#endif
		{
			if ((fd = open(device, O_RDWR|O_NOCTTY|O_SYNC)) < 0) {
				int err = errno;
				log_msg(LOG_LEVEL_ERROR,"open %s failed: %s", device, strerror(err));

				switch (err) {
				case EFAULT: // null pointer
					return DONGLE_PARGUMENTS_ERROR;
					// break;
				case EACCES: // access denied
					return PORT_OPENED_ERROR;
					// break;
				default:
					return PORT_DEVICE_ERROR;
					// break;
				}
			}
			if (tcgetattr(fd, &ios)) {
				close(fd);
				log_msg(LOG_LEVEL_ERROR,"tcgetattr(%s) failed: %s", device, strerror(errno));
				return PORT_GETCOMMSTATE_ERROR;
			}
			cfmakeraw(&ios);
  #if 1
			ios.c_cflag = CS8|CREAD|CLOCAL;
  #else
			switch(srTxnData.srParameter.bRS232Parameter[0])
			{
				case '5':
					ios.c_cflag |= CS5;
					break;
				case '6':
					ios.c_cflag |= CS6;
					break;
				case '7':
					ios.c_cflag |= CS7;
					break;
				default://'8'
					ios.c_cflag |= CS8;
					break;
			}
			switch(srTxnData.srParameter.bRS232Parameter[1])
			{
				case 'O':
					ios.c_cflag |= PARENB;     //允許同位元檢查
					ios.c_cflag |= PARODD;    //不是奇同位元檢查
					break;
				case 'E':
					ios.c_cflag |= PARENB;     //允許同位元檢查
					ios.c_cflag |= ~PARODD;    //不是奇同位元檢查
					break;
				default://'N'
					ios.c_cflag |= ~PARENB;     //不允許同位元檢查
					break;
			}
			switch(srTxnData.srParameter.bRS232Parameter[2])
			{
				case '2':
					ios.c_cflag |= CSTOPB;     //是2停止位元
					break;
				default://'1'
					ios.c_cflag |= ~CSTOPB;     //不是2停止位元
					break;
			}
  #endif
			//ios.c_cflag = CS8|CREAD|CLOCAL;
			//ios.c_cflag |= CS8|CREAD|CLOCAL|CRTSCTS;
	#if 1
			switch(srTxnData.srParameter.ulBaudRate)
			{
				case 115200:
					#if READER_MANUFACTURERS==LINUX_API
						cfsetspeed(&ios, B115200);
					#else
						cfsetispeed(&ios, B115200);
						cfsetospeed(&ios, B115200);
					#endif
					break;
				case 57600:
					#if READER_MANUFACTURERS==LINUX_API
						cfsetspeed(&ios, B57600);
					#else
						cfsetispeed(&ios, B57600);
						cfsetospeed(&ios, B57600);
					#endif
					break;
				case 38400:
					#if READER_MANUFACTURERS==LINUX_API
						cfsetspeed(&ios, B38400);
					#else
						cfsetispeed(&ios, B38400);
						cfsetospeed(&ios, B38400);
					#endif
					break;
				case 19200:
					#if READER_MANUFACTURERS==LINUX_API
						cfsetspeed(&ios, B19200);
					#else
						cfsetispeed(&ios, B19200);
						cfsetospeed(&ios, B19200);
					#endif
					break;
				default:
					close(fd);
					log_msg(LOG_LEVEL_ERROR,"ulBaudRate(%s) failed !!", srTxnData.srParameter.ulBaudRate);
					return PORT_GETCOMMSTATE_ERROR;
					break;
			}
	#else
			if(srTxnData.srParameter.ulBaudRate == 57600)
			{
		#if READER_MANUFACTURERS==LINUX_API
				cfsetspeed(&ios, B57600);
		#else
				cfsetispeed(&ios, B57600);
				cfsetospeed(&ios, B57600);
		#endif
			}
			else //if(srTxnData.srParameter.ulBaudRate == 115200)
			{
		#if READER_MANUFACTURERS==LINUX_API
				cfsetspeed(&ios, B115200);
		#else
				cfsetispeed(&ios, B115200);
				cfsetospeed(&ios, B115200);
		#endif
			}
	#endif
			//cfsetspeed(&ios, B57600);

			if (tcsetattr(fd, TCSANOW, &ios)) {
				close(fd);
				log_msg(LOG_LEVEL_ERROR,"tcsetattr(%s) failed: %s", device, strerror(errno));
				return PORT_SETCOMMSTATE_ERROR;
			}
			// confirme each setting
			memset(&ios, 0, sizeof(ios));

			if (tcgetattr(fd, &ios)) {
				close(fd);
				log_msg(LOG_LEVEL_ERROR,"tcgetattr(%s) failed: %s", device, strerror(errno));
				return PORT_GETCOMMSTATE_ERROR;
			}
			/*if ((cfgetispeed(&ios) != B115200) ||
				 (cfgetospeed(&ios) != B115200)) {*/

		//		if ((cfgetispeed(&ios) != B57600) ||
		//			 (cfgetospeed(&ios) != B57600)) {
		//
		//			close(fd);
		//			log_msg(LOG_LEVEL_ERROR,"set baud rate on %s failed", device);
		//			return PORT_SETCOMMSTATE_ERROR;
		//		}
			if (!(ios.c_cflag & CS8)) {
				close(fd);
				log_msg(LOG_LEVEL_ERROR,"set 8N1 on %s failed", device);
				return PORT_SETCOMMSTATE_ERROR;
			}
			if (!(ios.c_cflag & CREAD)) {
				close(fd);
				log_msg(LOG_LEVEL_ERROR,"cannot enable read ability on %s", device);
				return PORT_SETCOMMSTATE_ERROR;
			}
			if (!(ios.c_cflag & CLOCAL)) {
				// close(fd);
				log_msg(LOG_LEVEL_ERROR,"cannot disable modem control lines on %s", device);
				// return PORT_SETCOMMSTATE_ERROR;
			}
		//		if (!(ios.c_cflag & CRTSCTS)) {
		//			// close(fd);
		//			log_msg(LOG_LEVEL_ERROR,"cannot enable CTS/RTS flow control on %s", device);
		//			// return PORT_SETCOMMSTATE_ERROR;
		//		}
			if (ios.c_lflag & ICANON) {
				// close(fd);
				log_msg(LOG_LEVEL_ERROR,"cannot disable canonical mode on %s", device);
				// return PORT_SETCOMMSTATE_ERROR;
			}
		}
		fd_ = fd;

		if (dev_) {
			ECC_free((BYTE *)dev_);
			// dev_ = 0;
		}
		dev_ = strdup(device);
		atexit(finalizer_);
		TRACE_("open %s done", device);
		return SUCCESS;
	}
	
	return SUCCESS;
#else

int inRetVal;

  #if READER_MANUFACTURERS==NE_PAXSXX
	inRetVal = ECC_OpenCOM_Lib(srTxnData.srParameter.gComPort,srTxnData.srParameter.ulBaudRate,'N',8,1);
  #else
	inRetVal = ECC_OpenCOM_Lib(srTxnData.srParameter.gComPort,srTxnData.srParameter.ulBaudRate,'N',8,1);
      // 修改一般刷卡機設備Close Com
      if (inRetVal == SUCCESS)
	  	srTxnData.srIngData.fAPIOpenFlag = TRUE;
  #endif

	return inRetVal;

#endif
}

/* ---------------------------------------------------------------------
  Sent Data.
	Input parameters:
		. lpBuf -- sent data
		. dwToWrite -- total length to sent
	Returned status:
		. >= 0 if OK
		. < 0 on error
--------------------------------------------------------------------- */
int inComSend(BYTE *lpBuf, unsigned long dwToWrite)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	DWORD   dwWritten;
	COMSTAT ComStat;
	DWORD   dwErrorFlags;

	#ifdef CTOS_USB
	//#if READER_MANUFACTURERS==WINDOWS_API
	DWORD dRetVal;

		if (srTxnData.srParameter.gComPort == 0) //USB
		{
			if (hUSB[instance] != NULL) //USB
			{
			   // send
				//log_msg(LOG_LEVEL_ERROR,"EzrUSBWrite 1");
				dRetVal = EzrUSBWrite(hUSB[instance],dwToWrite,lpBuf);
				//log_msg(LOG_LEVEL_ERROR,"EzrUSBWrite 2,%d",dRetVal);
				if (dRetVal != SUCCESS)
					return PORT_WRITEFILE_ERROR ;
			}
			else
				log_msg(LOG_LEVEL_ERROR,"inComSend() hUSB = 0!!");
			return SUCCESS ;
		}
		else
	#endif //CTOS_USB
		{

			ClearCommError(hDev,&dwErrorFlags,&ComStat);

			if (!PurgeComm(hDev, PURGE_TXABORT| //終止目前所有的傳輸的工作，立即返回，不管是否完成傳輸的動作
							PURGE_RXABORT| //終止目前所有的讀取的工作，立即返回，不管是否完成讀取的動作
							PURGE_TXCLEAR| //清除傳送暫存區中的所有資料
							PURGE_RXCLEAR)) //清除接收暫存區中的所有資料
				return PORT_PURGECOMM_ERROR;

		   // Issue write
			if (!WriteFile(hDev, lpBuf, dwToWrite, &dwWritten, NULL))
				return PORT_WRITEFILE_ERROR ;
			return SUCCESS ;
	#if READER_MANUFACTURERS==WINDOWS_API
		}
	#endif

#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
//#elif (READER_MANUFACTURERS==LINUX_API && RDEVN==NE_X86) || READER_MANUFACTURERS==ANDROID_API

  #if READER_MANUFACTURERS==ANDROID_API
	if(srTxnData.srParameter.gReaderMode == USB_TO_RS232_READER || srTxnData.srParameter.gReaderMode == USB_READER)
	{
		jint resultI;
		jbyteArray jSendData=(*Jenv)->NewByteArray(Jenv, dwToWrite);

		//jbyte *jy = (jbyte*)lpBuf;
		(*Jenv)->SetByteArrayRegion(Jenv, jSendData, 0, dwToWrite, (jbyte*)lpBuf);
		resultI = (*Jenv) -> CallIntMethod(Jenv,comObj,AndroidComSend,jSendData);
		if(resultI != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"AndroidComSend fail: %d",resultI);
			return PORT_WRITEFILE_ERROR;
		}

		return SUCCESS;
	}
	else
  #endif
	if(srTxnData.srParameter.gReaderMode == RS232_READER || srTxnData.srParameter.gReaderMode == USB_READER)
	{
		time_t prev_time, curr_time,timeout = 2;
		struct pollfd fds;
		ssize_t bytes_written;

		if (!lpBuf) {
			return DONGLE_PARGUMENTS_ERROR;
		}
		if (fd_ < 0) {
			return PORT_NOTOPEN_ERROR;
		}
		if (dwToWrite <= 0) {
			return SUCCESS;
		}

  #ifdef CTOS_USB
  //#if (READER_MANUFACTURERS==LINUX_API && RDEVN==NE_X86)
		if (usb_dev)
		{
			if (EzrUSBWrite(fd_,dwToWrite,(BYTE *) lpBuf) != d_NO_ERROR) {
				log_msg(LOG_LEVEL_ERROR,"write() failed: %s", strerror(errno));
				return PORT_WRITEFILE_ERROR;
			}
			else
				//log_apdu_raw_msg(INBOUND_MSG, buf, buf_size);
				//vdWriteDongleLog(inAPIStatus,SUCCESS,"Request",(BYTE *)lpBuf,buf_size);
				return SUCCESS;
		}
		else
  #endif
		{
			fds.fd = fd_;
			fds.events = POLLOUT;//寫操作不會阻塞
			fds.revents = 0;

			if (timeout > 0) {
				prev_time = time((time_t*)0);
			}
			else {
				timeout = -1; // no timeout
			}
			switch (poll(&fds, 1, timeout * 1000)) {
			case -1:
				log_msg(LOG_LEVEL_ERROR,"poll() failed: %s", strerror(errno));
				return PORT_WRITEFILE_ERROR;
				// break;

			case 0: // timeout
				return PORT_WRITETIMEOUT_ERROR;
				// break;

			default:
				if (fds.revents & POLLERR) {//緊急數據可讀
					return PORT_WRITEFILE_ERROR;
				}
				else
				if (fds.revents & POLLHUP) {//挂起（只對輸出）
					return PORT_WRITEFILE_ERROR;
				}
				else
				if (fds.revents & POLLNVAL) {//無效請求：fd沒有打開只（只對輸出）
					log_msg(LOG_LEVEL_ERROR,"the file fd (%d) referenced is not opened yet", fd_);
					return PORT_WRITEFILE_ERROR;
				}
				if (!(fds.revents & POLLOUT)) { // should not to be reached here ~~
					if (timeout > 0) {
						curr_time = time((time_t*)0);

						if ((curr_time - prev_time) >= timeout) {
							return PORT_WRITETIMEOUT_ERROR;
						}
						timeout -= curr_time - prev_time;
						// prev_time = curr_time;
					}
					return inComSend(lpBuf, dwToWrite);
				}
				break;
			}
			switch (bytes_written = write(fd_, lpBuf, dwToWrite)) {
			case -1:
				log_msg(LOG_LEVEL_ERROR,"write() failed: %s", strerror(errno));
				return PORT_WRITEFILE_ERROR;
				// break;

			default:
				//if (bytes_written > 0)
				//	//log_apdu_raw_msg(INBOUND_MSG, buf, bytes_written);
				//	vdWriteDongleLog(inAPIStatus,SUCCESS,"Request",(BYTE *)lpBuf,bytes_written);
				if (bytes_written < dwToWrite) {
					if (timeout > 0) {
						curr_time = time((time_t*)0);

						if ((curr_time - prev_time) >= timeout) {
							return PORT_WRITETIMEOUT_ERROR;
						}
						timeout -= curr_time - prev_time;
						// prev_time = curr_time;
					}
					return inComSend((((BYTE*)lpBuf) + bytes_written),dwToWrite - bytes_written);
				}
				break;
			}
		}
		TRACE_("send_apdu() %s done", dev_);

		return SUCCESS;
	}

#else

unsigned long StartTime;
unsigned short RTN ;

  #if READER_MANUFACTURERS==NE_PAXSXX
	ECC_COMClearRxBuffer_Lib(srTxnData.srParameter.gComPort);
  #else
	ECC_COMClearRxBuffer_Lib(srTxnData.srParameter.gComPort);
  #endif

	StartTime = ECC_GetTickTime_Lib();

	do
	{
  #if READER_MANUFACTURERS==NE_PAXSXX
		RTN = ECC_COMTxReady_Lib( srTxnData.srParameter.gComPort );
  #else
		RTN = ECC_COMTxReady_Lib( srTxnData.srParameter.gComPort );
  #endif
	}
	while(((StartTime+200) > ECC_GetTickTime_Lib()) && RTN != SUCCESS);

	if(RTN == SUCCESS)
	{
  #if READER_MANUFACTURERS==NE_PAXSXX
		ECC_COMTxData_Lib( srTxnData.srParameter.gComPort, lpBuf, dwToWrite );
  #else
		ECC_COMTxData_Lib( srTxnData.srParameter.gComPort, lpBuf, dwToWrite );
  #endif
	}


	StartTime  = ECC_GetTickTime_Lib();

	do
	{
  #if READER_MANUFACTURERS==NE_PAXSXX
		RTN = ECC_COMTxReady_Lib( srTxnData.srParameter.gComPort );
  #else
		RTN = ECC_COMTxReady_Lib( srTxnData.srParameter.gComPort );
  #endif
	}
	while(((StartTime+200) > ECC_GetTickTime_Lib()) && RTN != SUCCESS );

	return SUCCESS;

#endif
	return SUCCESS;
}

/* ---------------------------------------------------------------------
  Recive Data.
	Input parameters:
		. lpBuf -- recive data
		. dwToRead -- total length to read
	Returned status:
		. >= 0 if OK
		. < 0 on error
--------------------------------------------------------------------- */
int inComRecv(BYTE *lpBuf,unsigned long dwToRead,clock_t tTimeout)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	DWORD			dwRead,dwTotalRead;
	COMMTIMEOUTS    TimeOuts ;
	//COMSTAT			ComStat;
	//DWORD			dwErrorFlags;

	char szLog[100];//1.0.5.5

	unsigned long ulHeaderLen = 0L;
	int inHeaderLne = 3;

  #ifdef CTOS_USB
	//#if READER_MANUFACTURERS==WINDOWS_API
		clock_t lnStartTime, lnEndTime;//1.0.5.5
		//DWORD dRetVal;
	if (srTxnData.srParameter.gComPort == 0) //USB
	{
		int inCnt = 0,inHeaderLen = 0;
		DWORD dRetVal;

		if (hUSB[instance] != NULL) //USB
		{
			if ( tTimeout > 20000 ) tTimeout = 20000;

			// Issue read operation.
			lnStartTime = clock();
			dwTotalRead = 0;

			do
			{
				dwRead = 0;
				//log_msg(LOG_LEVEL_FLOW,"EzrUSBRead 1");
				dRetVal = EzrUSBRead(hUSB[instance],1,&dwRead,&lpBuf[inCnt]);
				if (dRetVal != SUCCESS)
				{
					log_msg(LOG_LEVEL_ERROR,"USB 1");
					return PORT_READFILE_ERROR ;
				}

				if(dwRead > 0)
				{
					//log_msg(LOG_LEVEL_FLOW,"EzrUSBRead 2,%d,%d",dRetVal,dwRead);
					inCnt++;

					if(inHeaderLen == 0)//還沒收到NAD+PCB+Len
					{
						//log_msg(LOG_LEVEL_FLOW,"USB 1");
						if(inCnt == 3 || lpBuf[inCnt - 1] > 0)
						{
							if(lpBuf[inCnt - 1] > 0)
								inHeaderLen = lpBuf[inCnt - 1];
							else if(lpBuf[2] > 0)
							{
								log_msg(LOG_LEVEL_FLOW,"USB 2,%02x%02x%02x",lpBuf[0],lpBuf[1],lpBuf[2]);
								inHeaderLen = lpBuf[2];
							}
							else
								inCnt--;
						}
					}
					else //有收到NAD+PCB+Len
					{
						if(inCnt == inHeaderLen + 4)
						{
							if(inCnt == (int)dwToRead || inHeaderLen >= 2)//收到預期長度之資料
							{
								log_msg(LOG_LEVEL_FLOW,"USB 3-1,%02x%02x",lpBuf[inCnt - 2],lpBuf[inCnt - 1]);
								if(lpBuf[inCnt - 3] != 0xFF || lpBuf[inCnt - 2] != 0xFF)
								{
									/*if(inCnt == 259)
									{
										memset(lpBuf,0x00,300);
										memcpy(lpBuf,"\x00\x00\xFF\x00\x19\xF4\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x39\xBE\x62\x22\x01\x00\x71\x00\x00\x08\x00\x00\x39\xBE\x62\x01\xE5\xEF\x40\x01\x00\x00\x00\x04\x08\x00\x00\x01\x28\x05\x03\x01\xD0\x39\xB4\xD8\x9C\x00\x9F\x13\x0D\x2A\x03\x00\x9C\x2A\x03\x00\x00\x00\x00\x00\x00\x00\x02\x31\x00\x00\x54\x30\x36\x37\x33\x34\x39\x34\x00\x03\x5C\xAB\x95\x7D\xE6\x15\xFB\xBF\x5E\x12\x01\x61\x30\xD7\x0A\x13\xF0\x6B\xB1\x30\x30\x30\x32\x39\x31\x00\x6F\x00\x00\xE7\xDC\x9E\x5B\x40\xF4\x01\x00\x56\x02\x00\x9C\x00\x00\x01\x00\x0A\xD8\x9C\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x0B\xB4\xD8\x9C\x00\x9F\x13\x0D\x2A\x03\x00\x95\x83\xA6\x5B\x00\x11\x76\x01\x11\x00\x00\x00\x04\x95\x01\x00\xE6\x01\x00\x90\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x77\x0D\xFF\xDF\x1B\xA2\x86\x5D\x86\xA0\x3D\x21\x71\xA2\x02\x0B\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x64\x15\x4A",257);
										inCnt = 257;
									}*/
									dwTotalRead = inCnt;
									log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"USB 3:",6,lpBuf,inCnt);

									break;
								}

								vdSaveReaderDebugLog(&lpBuf[3],inCnt - 5);
								memset(lpBuf,0x00,inCnt);
								inHeaderLen = inCnt = 0;
							}
						}

					#if 0//底下是在安源iBon上測試時遇到的問題,但是,遇到ReadBasic且640E卡片時,會有問題,移除先!!
						if(lpBuf[inCnt - 2] == 0xFF && lpBuf[inCnt - 1] == 0xFF && inHeaderLen + 4 != (int)dwToRead)//SW1 + SW2 = 0xFFFF 表示此為 Debug Msg
						{
							if(lpBuf[2] + 3 == inCnt)//Debug Msg格式正確
							{
								vdSaveReaderDebugLog(&lpBuf[3],inCnt - 5);
								if(!memcmp(&lpBuf[3],"Phoenix_UartProcess OK (0)",26))
									log_msg(LOG_LEVEL_FLOW,"111");
							}
							else if(lpBuf[0] > 0)
							{
								vdSaveReaderDebugLog(&lpBuf[0],inCnt - 2);
								if(!memcmp(&lpBuf[0],"Phoenix_UartProcess OK (0)",26))
									log_msg(LOG_LEVEL_FLOW,"222");
							}
							else if(lpBuf[1] > 0)
							{
								vdSaveReaderDebugLog(&lpBuf[1],inCnt - 3);
								if(!memcmp(&lpBuf[1],"Phoenix_UartProcess OK (0)",26))
									log_msg(LOG_LEVEL_FLOW,"333");
							}
							else
							{
								//log_msg_debug(LOG_LEVEL_FLOW,FALSE,(char *)"USB 4:",6,lpBuf,inCnt);
								log_msg(LOG_LEVEL_ERROR,"USB 4(%d,%d,%d,%d)",inCnt,inHeaderLen,dwToRead,lpBuf[2]);
								log_msg_debug(LOG_LEVEL_ERROR,TRUE,"",0,lpBuf,inCnt);
							}

							dRetVal = EzrUSBRead(hUSB[instance],1,&dwRead,&lpBuf[inCnt++]);
							if (dRetVal != SUCCESS)
							{
								log_msg(LOG_LEVEL_ERROR,"USB 2");
								return PORT_READFILE_ERROR ;
							}

							memset(lpBuf,0x00,inCnt);
							inHeaderLen = inCnt = 0;
							//log_msg(LOG_LEVEL_FLOW,"USB 4");
						}
					#endif
					}
				}

				lnEndTime = clock();
				if(lnEndTime < lnStartTime)
					lnStartTime &= 0x80000000;
		        if((lnEndTime - lnStartTime) >= tTimeout)
				{
					log_msg(LOG_LEVEL_ERROR,"%ld(lnEndTime) - %ld(lnStartTime) = %ld",lnEndTime,lnStartTime,lnEndTime - lnStartTime);
					return PORT_READTIMEOUT_ERROR ;
				}
			}
			while(1);

		}
		else
			log_msg(LOG_LEVEL_ERROR,"inComRecv() hUSB = 0!!");
		return SUCCESS ;
	}
	else
  #endif //CTOS_USB
	{
		memset(szLog,0x00,sizeof(szLog));
		if ( tTimeout > 20000 ) tTimeout = 20000;

		GetCommTimeouts(hDev,&TimeOuts);

		//ReadTotalTimeout= ReadTotalTimeOutMultiplier* BytesToRead+ ReadTotalTimeoutConstant
		TimeOuts.ReadIntervalTimeout=10;         //接收時每個byte跟byte的間隔時間 MAXDWORD
		TimeOuts.ReadTotalTimeoutMultiplier=20;   //讀取時間系數
		TimeOuts.ReadTotalTimeoutConstant=tTimeout;    //讀取時間常量

		//If the function succeeds, the return value is nonzero.
		if (!SetCommTimeouts(hDev,&TimeOuts))//設置超
			return PORT_SETCOMTIMEOUT_ERROR;

		   // Issue read operation.
		dwTotalRead = 0;

		while(1)
		{
			ulHeaderLen = 0L;
			dwTotalRead = 0;
			memset(lpBuf,0x00,256);
			if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
				inHeaderLne = 11;

			dwRead = inComRecvByLen(&lpBuf[0],inHeaderLne,tTimeout);
			if(dwRead == PORT_READTIMEOUT_ERROR)
				return PORT_READTIMEOUT_ERROR;

			if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
			{
				ulHeaderLen = lpBuf[9] * 256;
				ulHeaderLen += lpBuf[10];
				ulHeaderLen += 2;
			}
			else
			{
				ulHeaderLen = lpBuf[2];
				if(srTxnData.srParameter.chReaderPortocol == READER_INFOCHAMP)
					ulHeaderLen += lpBuf[1] * 256;
			}
			dwTotalRead += dwRead;
			log_msg(LOG_LEVEL_FLOW,"1 dwTotalRead = %d,ulHeaderLen = %d",dwTotalRead,ulHeaderLen);

			if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
			{
				dwRead = inDLEComRecvByLen(&lpBuf[dwTotalRead],ulHeaderLen + 1,tTimeout);
				if(dwRead == PORT_READTIMEOUT_ERROR)
					return PORT_READTIMEOUT_ERROR;
 			}
			else
			{
				dwRead = inComRecvByLen(&lpBuf[dwTotalRead],ulHeaderLen + 1,tTimeout);
				if(dwRead == PORT_READTIMEOUT_ERROR)
					return PORT_READTIMEOUT_ERROR;
			}

			dwTotalRead += dwRead;

			//log_msg(LOG_LEVEL_ERROR,"3 dwTotalRead = %d,ulHeaderLen = %d[%02x%02x]",dwTotalRead,ulHeaderLen,lpBuf[ulHeaderLen + 1],lpBuf[ulHeaderLen + 2]);
			//if(lpBuf[ulHeaderLen + 1] == 0xFF && lpBuf[ulHeaderLen + 2] == 0xFF)
			if(srTxnData.srParameter.chReaderPortocol == READER_INFOCHAMP)
			{
				if(!memcmp(&lpBuf[ulHeaderLen],"\xFF\xFF",2))
					vdSaveReaderDebugLog(&lpBuf[3 + 3],ulHeaderLen - 2 - 4);
				else
					break;
			}
			else if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
			{
				/*if(!memcmp(&lpBuf[ulHeaderLen],"\xFF\xFF",2))
					vdSaveReaderDebugLog(&lpBuf[3 + 3],ulHeaderLen - 2 - 4);
				else*/
					break;
			}
			else if(!memcmp(&lpBuf[ulHeaderLen + 1],"\xFF\xFF",2))
				vdSaveReaderDebugLog(&lpBuf[3],ulHeaderLen - 2);
			else
				break;
		}

		//If the function succeeds, the return value is nonzero.
		if (!PurgeComm(hDev, PURGE_TXABORT| //終止目前所有的傳輸的工作，立即返回，不管是否完成傳輸的動作
						PURGE_RXABORT| //終止目前所有的讀取的工作，立即返回，不管是否完成讀取的動作
						PURGE_TXCLEAR| //清除傳送暫存區中的所有資料
						PURGE_RXCLEAR)) //清除接收暫存區中的所有資料
		{
			return PORT_PURGECOMM_ERROR;
		}

		return SUCCESS ;
	}

#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
//#elif (READER_MANUFACTURERS==LINUX_API && RDEVN==NE_X86) || READER_MANUFACTURERS==ANDROID_API
	DWORD 		  dwRead,dwTotalRead;
//	unsigned long ulHeaderLen = 0L;
	int inHeaderLne = 3;

  #if READER_MANUFACTURERS==ANDROID_API
	if(srTxnData.srParameter.gReaderMode == USB_TO_RS232_READER || srTxnData.srParameter.gReaderMode == USB_READER)
	{
		jint Timeout = tTimeout;
		jbyteArray readArray = (*Jenv) -> CallObjectMethod(Jenv,comObj,AndroidComRead,Timeout);

		if(readArray != NULL)
		{
			jsize len = (*Jenv) -> GetArrayLength(Jenv,readArray);
			//log_msg(LOG_LEVEL_ERROR,"AndroidComRead len = %d",len);
			jbyte* jbarray = (jbyte*) ECC_calloc(len,sizeof(jbyte));
			(*Jenv) -> GetByteArrayRegion(Jenv,readArray,0,len,jbarray);
			memcpy(lpBuf,jbarray,len);
			ECC_free(jbarray);
			if (len < 3)
			{
				log_msg(LOG_LEVEL_ERROR,"AndroidComRead fail 0: (%d),(%x),(%x),(%x)", len, lpBuf[0], lpBuf[1], lpBuf[2]);
				return PORT_READTIMEOUT_ERROR;
			}			
			return SUCCESS ;
		}
		else
		{
			log_msg(LOG_LEVEL_ERROR,"AndroidComRead fail:");
			return PORT_READTIMEOUT_ERROR ;
		}
	}
	else
  #endif
	if(srTxnData.srParameter.gReaderMode == RS232_READER || srTxnData.srParameter.gReaderMode == USB_READER)
	{
//		int ret;
		//ULONG ret2;
		int recv_size;

		if (!lpBuf/* || !recv_size*/)
			return DONGLE_PARGUMENTS_ERROR;
		recv_size = 0;

  #ifdef CTOS_USB
  //#if READER_MANUFACTURERS==LINUX_API && RDEVN==NE_X86
		if (usb_dev)
		{
			ret = EzrUSBRead(fd_, 300 , (ULONG*)&recv_size ,lpBuf , (int)(tTimeout));
			if (ret != d_NO_ERROR)
			{
				log_msg(LOG_LEVEL_ERROR,"EzrUSBRead failed: %s", ret);
				//log_msg(LOG_LEVEL_ERROR,"%sResponse EzrUSBRead failed: %x",chGetAPITypeName(inTxnType),ret2);//1.1.5
				//vdWriteDongleLog(inAPIStatus,ret2,"Response 1",buf,*recv_size);
				return PORT_READFILE_ERROR;

			}
		}
		else
  #endif
		{
	 		unsigned long ulHeaderLen = 0L;

			while(1)
			{
#if 1
				ulHeaderLen = 0L;
				dwTotalRead = 0;
				memset(lpBuf,0x00,256);
				if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
					inHeaderLne = 11;

				dwRead = recv_from2(&lpBuf[0],tTimeout / 1000,inHeaderLne);
				//dwRead = inComRecvByLen(&lpBuf[0],inHeaderLne,tTimeout);
				if(dwRead == PORT_READTIMEOUT_ERROR)
					return PORT_READTIMEOUT_ERROR;
				else if(dwRead != SUCCESS)
				{
					log_msg(LOG_LEVEL_ERROR,"receive APDU prolog failed: %d", dwRead);
					return dwRead;
				}
				else
					dwRead = inHeaderLne;

				if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
				{
					ulHeaderLen = lpBuf[9] * 256;
					ulHeaderLen += lpBuf[10];
					ulHeaderLen += 2;
				}
				else
				{
					ulHeaderLen = lpBuf[2];
					if(srTxnData.srParameter.chReaderPortocol == READER_INFOCHAMP)
						ulHeaderLen += lpBuf[1] * 256;
				}
				dwTotalRead += dwRead;
				log_msg(LOG_LEVEL_FLOW,"1 dwTotalRead = %d,ulHeaderLen = %d",dwTotalRead,ulHeaderLen);

				if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
				{
					dwRead = inDLEComRecvByLen(&lpBuf[dwTotalRead],ulHeaderLen + 1,tTimeout);
					log_msg(LOG_LEVEL_FLOW,"inDLEComRecvByLen dwRead = %d,dwTotalRead = %d",dwRead,dwTotalRead);
					if(dwRead == PORT_READTIMEOUT_ERROR)
						return PORT_READTIMEOUT_ERROR;
				}
				else
				{
					dwRead = recv_from2(&lpBuf[dwTotalRead],tTimeout / 1000,ulHeaderLen + 1);
					//dwRead = inComRecvByLen(&lpBuf[dwTotalRead],ulHeaderLen + 1,tTimeout);
					if(dwRead == PORT_READTIMEOUT_ERROR)
						return PORT_READTIMEOUT_ERROR;
					else if(dwRead != SUCCESS)
					{
						log_msg(LOG_LEVEL_ERROR,"receive APDU info. and epilog failed: %d,%d", dwRead,recv_size);
						log_msg_debug(LOG_LEVEL_ERROR,TRUE,(char *)"Data:",5,&lpBuf[OUT_DATA_OFFSET],(recv_size > 0)?recv_size:0);
						return dwRead;
					}
					else
						dwRead = ulHeaderLen + 1;
				}

				dwTotalRead += dwRead;

				//log_msg(LOG_LEVEL_ERROR,"3 dwTotalRead = %d,ulHeaderLen = %d[%02x%02x]",dwTotalRead,ulHeaderLen,lpBuf[ulHeaderLen + 1],lpBuf[ulHeaderLen + 2]);
				//if(lpBuf[ulHeaderLen + 1] == 0xFF && lpBuf[ulHeaderLen + 2] == 0xFF)
				if(srTxnData.srParameter.chReaderPortocol == READER_INFOCHAMP)
				{
					if(!memcmp(&lpBuf[ulHeaderLen],"\xFF\xFF",2))
						vdSaveReaderDebugLog(&lpBuf[3 + 3],ulHeaderLen - 2 - 4);
					else
						break;
				}
				else if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
				{
					/*if(!memcmp(&lpBuf[ulHeaderLen],"\xFF\xFF",2))
						vdSaveReaderDebugLog(&lpBuf[3 + 3],ulHeaderLen - 2 - 4);
					else*/
						break;
				}
				else if(!memcmp(&lpBuf[ulHeaderLen + 1],"\xFF\xFF",2))
					vdSaveReaderDebugLog(&lpBuf[3],ulHeaderLen - 2);
				else
					break;
#else
				memset(lpBuf,0x00,sizeof(lpBuf));
				recv_size = 0;
			// read prolog
				if ((ret = recv_from2(lpBuf, tTimeout / 1000, OUT_DATA_OFFSET)) != SUCCESS)
				{
					log_msg(LOG_LEVEL_ERROR,"receive APDU prolog failed: %d", ret);
					return ret;
				}

				ulHeaderLen = lpBuf[2];
				recv_size += lpBuf[2] + 1;
				log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"Header:",7,lpBuf,OUT_DATA_OFFSET);
				//log_msg(LOG_LEVEL_ERROR,"receive %d,%d,%02x", ulHeaderLen,recv_size,lpBuf[2]);
				// read info. and epilog
				if ((ret = recv_from2(&lpBuf[OUT_DATA_OFFSET],tTimeout / 1000,recv_size)) != SUCCESS)
				{
					log_msg(LOG_LEVEL_ERROR,"receive APDU info. and epilog failed: %d,%d", ret,recv_size);
					log_msg_debug(LOG_LEVEL_ERROR,TRUE,(char *)"Data:",5,&lpBuf[OUT_DATA_OFFSET],(recv_size > 0)?recv_size:0);
					return ret;
				}
				recv_size += OUT_DATA_OFFSET/*sizeof(epilog_t)*/;

				/*{
					int i = 0;
					BYTE bBuf[1000],bTmp[500];

					memset(bTmp,0x00,sizeof(bTmp));
					memset(bBuf,0x00,sizeof(bBuf));
					memcpy(bTmp,lpBuf,recv_size);
					sprintf((char *)bBuf,"read(%d)[",recv_size);
					for(i=0;i<recv_size;i++)
					{
						if(bTmp[i] >= 0x20 && bTmp[i] <= 0x7F)
							bBuf[strlen((char *)bBuf)] = bTmp[i];
						else
							sprintf((char *)&bBuf[strlen((char *)bBuf)],"[%02x]",bTmp[i]);
					}
					strcat((char *)bBuf,"]");
					log_msg(LOG_LEVEL_ERROR,(char *)bBuf);
				}*/

				if(lpBuf[ulHeaderLen + 1] == 0xFF && lpBuf[ulHeaderLen + 2] == 0xFF)
					vdSaveReaderDebugLog(&lpBuf[3],ulHeaderLen - 2);
				else
					break;
#endif
			}
		}

		//log_apdu_raw_msg(OUTBOUND_MSG, buf, *recv_size);
		//vdWriteDongleLog(inAPIStatus,SUCCESS,"Response",lpBuf,&recv_size);
		TRACE_("recv_apdu() %s done", dev_);

		//return ret;
		return SUCCESS;
	}

#else

int	dwRead,dwTotalRead;
unsigned long ulHeaderLen = 0L;

	while(1)
	{
		ulHeaderLen = 0L;
		dwTotalRead = 0;
		memset(lpBuf,0x00,256);

	#if READER_MANUFACTURERS==NE_VX520 || READER_MANUFACTURERS==UNI_TS890
		dwRead = inComRecvByLen(&lpBuf[0],3,tTimeout / 10);//ECC_GetTickTime_Lib的單位是10ms,所以要/10
	#else
		dwRead = inComRecvByLen(&lpBuf[0],3,tTimeout);
	#endif
		if(dwRead == PORT_READTIMEOUT_ERROR)
			return PORT_READTIMEOUT_ERROR;

		ulHeaderLen = lpBuf[2];
		dwTotalRead += dwRead;
		//log_msg(LOG_LEVEL_ERROR,"1 dwTotalRead = %d,ulHeaderLen = %d",dwTotalRead,ulHeaderLen);

	#if READER_MANUFACTURERS==NE_VX520 || READER_MANUFACTURERS==UNI_TS890
		dwRead = inComRecvByLen(&lpBuf[dwTotalRead],ulHeaderLen + 1,tTimeout / 10);//ECC_GetTickTime_Lib的單位是10ms,所以要/10
	#else
		dwRead = inComRecvByLen(&lpBuf[dwTotalRead],ulHeaderLen + 1,tTimeout);
	#endif
		if(dwRead == PORT_READTIMEOUT_ERROR)
			return PORT_READTIMEOUT_ERROR;

		dwTotalRead += dwRead;

		//log_msg(LOG_LEVEL_ERROR,"3 dwTotalRead = %d,ulHeaderLen = %d[%02x%02x]",dwTotalRead,ulHeaderLen,lpBuf[ulHeaderLen + 1],lpBuf[ulHeaderLen + 2]);
		if(lpBuf[ulHeaderLen + 1] == 0xFF && lpBuf[ulHeaderLen + 2] == 0xFF)
			vdSaveReaderDebugLog(&lpBuf[3],ulHeaderLen - 2);
		else
			break;
	}


	return SUCCESS;

#endif

return SUCCESS;
}

/* ---------------------------------------------------------------------
  Close Com Port.
	Returned status:
		. >= 0 if OK
		. < 0 on error
--------------------------------------------------------------------- */
int inComClose(void)
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
DWORD dRetVal;

  #ifdef CTOS_USB
  //#if READER_MANUFACTURERS==WINDOWS_API
	if (srTxnData.srParameter.gComPort == 0) //USB
	{
		if (hUSB[instance] != NULL) //USB
		{
			//If the function succeeds, the return value is nonzero

			dRetVal = EzrUSBClose(&hUSB[instance]);
			if(!dRetVal)
			{
				log_msg(LOG_LEVEL_ERROR,"fnComClose USB PORT_CLOSEHANDLE_ERROR(%d)",dRetVal);
				return PORT_CLOSEHANDLE_ERROR;
			}
			else
			{
				log_msg(LOG_LEVEL_FLOW,"fnComClose USB SUCCESS");
				return SUCCESS;
			}
		}
		else
			log_msg(LOG_LEVEL_FLOW,"inComClose() hUSB = 0!!");
		return SUCCESS ;
	}
	else
  #endif //CTOS_USB
  {
		if ( hDev == INVALID_HANDLE_VALUE ) return PORT_NOTOPEN_ERROR;

		//If the function succeeds, the return value is nonzero
		dRetVal = CloseHandle(hDev);
		if(!dRetVal)
		{
			//fnWriteLog("fnComClose RS232 PORT_CLOSEHANDLE_ERROR",(BYTE *)&dRetVal,4);
			return PORT_CLOSEHANDLE_ERROR;
		}
		else
		{
			//fnWriteLog("fnComClose RS232 SUCCESS",(BYTE *)&dRetVal,4);
			return SUCCESS;
		}
	}

#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
//#elif (READER_MANUFACTURERS==LINUX_API && RDEVN==NE_X86) || READER_MANUFACTURERS==ANDROID_API

  #if READER_MANUFACTURERS==ANDROID_API
	if(srTxnData.srParameter.gReaderMode == USB_TO_RS232_READER || srTxnData.srParameter.gReaderMode == USB_READER)
	{
		jint resultI;

		resultI = (*Jenv) -> CallIntMethod(Jenv,comObj,AndroidComClose);
		if(resultI != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"AndroidComClose fail: %d",resultI);
			return PORT_WRITEFILE_ERROR;
		}

		return SUCCESS;
	}
	else
  #endif
	if(srTxnData.srParameter.gReaderMode == RS232_READER || srTxnData.srParameter.gReaderMode == USB_READER)
	{
		if (fd_ < 0) {
			return PORT_NOTOPEN_ERROR;
		}
		tcflush(fd_, TCIOFLUSH); // ignore error

	#ifdef CTOS_USB
	//#if READER_MANUFACTURERS==LINUX_API && RDEVN==NE_X86
		if (usb_dev)
		{
			if (!EzrUSBClose(&fd_))
			{
				log_msg(LOG_LEVEL_ERROR,"close %s failed: %s", dev_, strerror(errno));
				return PORT_CLOSEHANDLE_ERROR;
			}

		}
		else
	#endif
		{
			if (close(fd_)) {
				log_msg(LOG_LEVEL_ERROR,"close %s failed: %s", dev_, strerror(errno));
				return PORT_CLOSEHANDLE_ERROR;
			}
		}

		usb_dev = FALSE;
		fd_ = -1;
		TRACE_("close %s done", dev_);

		if (dev_) {
			ECC_free((BYTE *)dev_);
			dev_ = 0;
		}

		return SUCCESS;
	}
#else

int inRetVal;

  #if READER_MANUFACTURERS==NE_PAXSXX
	inRetVal = ECC_CloseCOM_Lib(srTxnData.srParameter.gComPort);
  #else
	inRetVal = ECC_CloseCOM_Lib(srTxnData.srParameter.gComPort);
  #endif
  	// 
	srTxnData.srIngData.fAPIOpenFlag = FALSE;
	return inRetVal;

#endif
	
return SUCCESS;
}

#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
void finalizer_(void)
{
	//inComClose();
}

int recv_from(void* buf, size_t buf_size, float timeout, size_t* recv_size)
{
	time_t prev_time, curr_time;
	struct pollfd fds;
	ssize_t bytes_read;

	log_msg(LOG_LEVEL_FLOW,"recv_from() 1: %d,%d", buf_size,timeout);
	if (!buf || !recv_size) {
		return DONGLE_PARGUMENTS_ERROR;
	}
	if (fd_ < 0) {
		return PORT_NOTOPEN_ERROR;
	}
	*recv_size = 0;

	if (buf_size <= 0) {
		return SUCCESS;
	}
	fds.fd = fd_;//文件描述符
	fds.events = POLLIN;//請求的事件   POLLIN：數據可讀
	fds.revents = 0;//返回的事件

	if (timeout > 0) {
		prev_time = time((time_t*)0);
	}
	else {
		timeout = -1; // no timeout
	}
	switch (poll(&fds, 1, (int)(timeout * 1000))) {
	case -1:
		log_msg(LOG_LEVEL_ERROR,"poll() failed: %s", strerror(errno));
		return PORT_READFILE_ERROR;
		// break;

	case 0: // timeout

		log_msg(LOG_LEVEL_ERROR,"poll() timeout: %s", strerror(errno));
		return PORT_READTIMEOUT_ERROR;
		// break;

	default:
		log_msg(LOG_LEVEL_FLOW,"recv_from() 2:");
		if (fds.revents & POLLERR) {
			return PORT_READFILE_ERROR;
		}
		else
		if (fds.revents & POLLHUP) {
			return PORT_READFILE_ERROR;
		}
		else
		if (fds.revents & POLLNVAL) {
			log_msg(LOG_LEVEL_ERROR,"the file fd (%d) referenced is not opened yet", fd_);
			return PORT_READFILE_ERROR;
		}
		if (!(fds.revents & POLLIN)) { // should not to be reached here ~~
			if (timeout > 0) {
				curr_time = time((time_t*)0);

				if ((curr_time - prev_time) >= timeout) {
					return PORT_READTIMEOUT_ERROR;
				}
				timeout -= curr_time - prev_time;
				// prev_time = curr_time;
			}
			return recv_from(buf, buf_size, timeout, recv_size);
		}
		break;
	}
	switch (bytes_read = read(fd_, buf, buf_size)) {
	case 0: // EOF
		return PORT_READFILE_ERROR;
		// break;

	case -1:
		log_msg(LOG_LEVEL_ERROR,"read() failed: %s", strerror(errno));
		return PORT_READFILE_ERROR;
		// break;

	default:
		log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"recv_from:",10,(BYTE *)buf,bytes_read);
		*recv_size = bytes_read;
		break;
	}
	TRACE_("recv_from() %s done", dev_);

	return SUCCESS;
}

int recv_from2(BYTE* buf, clock_t timeout, int buf_size)
{
	time_t prev_time, curr_time;
	size_t recv_size = 0,total_recv = 0;
	int ret;

	if (timeout > 0) {
		prev_time = time((time_t*)0);
	}

	//log_msg(LOG_LEVEL_ERROR,"recv_from2 1 %d",buf_size);

	for (; (ret = recv_from(&buf[total_recv], buf_size, timeout, &recv_size)) == SUCCESS;total_recv += recv_size)
	{
		//log_msg(LOG_LEVEL_ERROR,"recv_from2 2 %d",buf_size);
		buf_size -= recv_size;

		if (buf_size <= 0) {
			return SUCCESS;
		}
		if (timeout > 0) {
			curr_time = time((time_t*)0);

			if ((curr_time - prev_time) >= timeout) {
				return PORT_READTIMEOUT_ERROR;
			}
			timeout -= curr_time - prev_time;
			prev_time = curr_time;
		}
	}
	TRACE_("recv_from2() %s done", dev_);

	return ret;
}
#endif

int inReponseDefaultCkeck(BYTE *cOutputData)
{
BYTE bEDC;

	if(cOutputData[0] != 0x00 || cOutputData[1] != 0x00)
	{
		log_msg(LOG_LEVEL_ERROR,"inReponseDefaultCkeck Fail NAD_PCB_ERROR(%02x%02x)",cOutputData[0],cOutputData[1]);
		return	NAD_PCB_ERROR;
	}
	else if(cOutputData[2] == 0x00)
	{
		log_msg(LOG_LEVEL_ERROR,"inReponseDefaultCkeck Fail DATA_ERROR");
		return	PORT_READFILE_ERROR;
	}

	bEDC = checksum(cOutputData[2]+3,cOutputData);
	if(cOutputData[cOutputData[2]+3] != bEDC)
	{
		log_msg(LOG_LEVEL_ERROR,"inReponseDefaultCkeck Fail EDC_ERROR(%02x != %02x)",cOutputData[cOutputData[2]+3],bEDC);
		log_msg_debug(LOG_LEVEL_ERROR,FALSE,(char *)"Data:",5,cOutputData,cOutputData[2] + 4);
		return	EDC_ERROR;
	}

	/*if(inResponseLenCkeck(inAPITxnType,cOutputData,&inDataLen) != SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"%sReponse Invalid Len = (%d),Must Be (%d)!!",chGetAPITypeName(inAPITxnType),(int )cOutputData[2],inDataLen);
		return PORT_READOVERLAPPED_ERROR;
	}*/

	return(SUCCESS);
}

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
int inComRecvByLen(BYTE *lpBuf,unsigned long dwToRead,clock_t tTimeout)
{
DWORD			dwRead = 0,dwTotalRead = 0;
COMSTAT 		ComStat;
DWORD			dwErrorFlags;
clock_t lnStartTime, lnEndTime;//1.0.5.5
char szLog[100];//1.0.5.5
int inReadLen = 0;

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		lnStartTime = GetTickCount();
	#else
		lnStartTime = clock();
	#endif

	while(1)
	{
		//log_msg(LOG_LEVEL_ERROR,"inComRecvByLen 1");
		ClearCommError(hDev,&dwErrorFlags,&ComStat);
		//log_msg(LOG_LEVEL_ERROR,"inComRecvByLen 2");
		//反覆讀取通訊埠中資料
		inReadLen = dwToRead - dwTotalRead;
	#ifdef CTOS_USB
	//#if READER_MANUFACTURERS==WINDOWS_API
		if(srTxnData.srParameter.gComPort == 0) //USB
			EzrUSBRead(hUSB[instance],inReadLen,&dwRead,&lpBuf[dwTotalRead]);
		else
	#endif
			ReadFile(hDev, &lpBuf[dwTotalRead],inReadLen,&dwRead, NULL);
	//log_msg(LOG_LEVEL_ERROR,"inComRecvByLen 3");

		if(dwToRead <= dwRead && dwTotalRead + dwRead > 1)
		{
			DWORD i;
			//log_msg(LOG_LEVEL_ERROR,"dwToRead = %d,dwRead = %d,dwTotalRead = %d",dwToRead,dwRead,dwTotalRead);
			for(i=0;i<dwTotalRead + dwRead;i++)
			{
				if(lpBuf[i] == 0xFF && lpBuf[i + 1] == 0xFF)
				{
					return dwTotalRead;
				}
			}
		}

		if(dwRead > 0)
		{
			int i;
			BYTE bBuf[1000];

			memset(bBuf,0x00,sizeof(bBuf));

			sprintf((char *)bBuf,"inComRecvByLen (%d),[",dwRead);
			for(i=0;i<(int)dwRead;i++)
			{
				sprintf((char *)&bBuf[strlen((char *)bBuf)],"%02x",lpBuf[dwTotalRead + i]);
			}
			strcat((char *)bBuf,"]");
			log_msg(LOG_LEVEL_FLOW,(char *)bBuf);
		}
		dwTotalRead += dwRead;
		if(dwTotalRead >= dwToRead)
			break;

		#if READER_MANUFACTURERS==WINDOWS_CE_API
			lnEndTime = GetTickCount();
		#else
			lnEndTime = clock();
		#endif

		if(lnEndTime < lnStartTime)
			lnStartTime &= 0x80000000;
		if((lnEndTime - lnStartTime) >= tTimeout)
		{
			clock_t lnTime;
			sprintf(szLog," 1(%ld)%ld(lnEndTime) - %ld(lnStartTime) =",dwTotalRead,lnEndTime,lnStartTime);
			lnTime = lnEndTime - lnStartTime;
			//fnWriteLog(szLog,(BYTE *)&lnTime,4);
			log_msg(LOG_LEVEL_ERROR,"1 dwToRead = %d,dwTotalRead = %d",dwToRead,dwTotalRead);
			return PORT_READTIMEOUT_ERROR ;
		}
	}

	return dwTotalRead;
}
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
int inComRecvByLen(BYTE *lpBuf,unsigned long dwToRead,clock_t tTimeout)
{
	DWORD dwTotalRead = 0;
	time_t prev_time, curr_time;
	int inReadLen = 0;
//	int ret;
	size_t dwRead;

	if (tTimeout > 0) {
		prev_time = time((time_t*)0);
	}

	while (1)
	{
		inReadLen = dwToRead - dwTotalRead;
		
		if (recv_from(&lpBuf[dwTotalRead], dwToRead, tTimeout, &dwRead) == SUCCESS)
		{
			if(dwToRead <= dwRead && dwTotalRead + dwRead > 1)
			{
				DWORD i;

				for(i=0;i<dwTotalRead + dwRead;i++)
				{
					if(lpBuf[i] == 0xFF && lpBuf[i + 1] == 0xFF)
					{
						return dwTotalRead;
					}
				}
			}

			if(dwRead > 0)
			{
				int i;
				BYTE bBuf[1000];

				memset(bBuf,0x00,sizeof(bBuf));

				sprintf((char *)bBuf,"inComRecvByLen (%d),[",dwRead);
				for(i=0;i<(int)dwRead;i++)
				{
					sprintf((char *)&bBuf[strlen((char *)bBuf)],"%02x",lpBuf[dwTotalRead + i]);
				}
				strcat((char *)bBuf,"]");
				log_msg(LOG_LEVEL_FLOW,(char *)bBuf);
			}

			dwTotalRead += dwRead;
			if(dwTotalRead >= dwToRead)
				break;
		}

		if (tTimeout > 0) 
		{
			curr_time = time((time_t*)0);

			if ((curr_time - prev_time) >= tTimeout) 
				return PORT_READTIMEOUT_ERROR;
			tTimeout -= curr_time - prev_time;
			prev_time = curr_time;
		}
	}
	
	TRACE_("inComRecvByLen() %s done", dev_);

	return dwTotalRead;

	//return 0;
}
#else
//#elif READER_MANUFACTURERS!=LINUX_API && READER_MANUFACTURERS!=ANDROID_API
//#elif (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86) && READER_MANUFACTURERS!=ANDROID_API
int inComRecvByLen(BYTE *lpBuf,unsigned long dwToRead,clock_t tTimeout)
{
unsigned long	dwTotalRead = 0;
//unsigned short dwRead = 0;
clock_t lnStartTime, lnEndTime;//1.0.5.5
char szLog[100];//1.0.5.5
int inReadLen = 0,inRetVal;

	lnStartTime = ECC_GetTickTime_Lib();
	while(1)
	{
		//反覆讀取通訊埠中資料
		inReadLen = dwToRead - dwTotalRead;
		//ReadFile(hDev, &lpBuf[dwTotalRead],inReadLen,&dwRead, NULL);

		//inRetVal = ECC_COMRxReady_Lib(srTxnData.srParameter.gComPort, &dwRead);
		//if(inRetVal == SUCCESS)
		{
		#if READER_MANUFACTURERS==NE_VX520
			inRetVal = ECC_COMRxData_Lib(srTxnData.srParameter.gComPort, &lpBuf[dwTotalRead], &inReadLen);
		#else
			inRetVal = ECC_COMRxData_Lib(srTxnData.srParameter.gComPort, &lpBuf[dwTotalRead], (unsigned short *)&inReadLen);
		#endif
			/*if(inRetVal != SUCCESS)
			{
				//log_msg(LOG_LEVEL_ERROR,"inComRecvByLen inRetVal = %d,dwRead = %d",inRetVal,dwRead);
				continue;
			}*/
		}

		if(inRetVal == SUCCESS)
		{
			dwTotalRead += inReadLen;
			if(dwTotalRead >= dwToRead)
				break;
		}

		lnEndTime = ECC_GetTickTime_Lib();
		if(lnEndTime < lnStartTime)
			lnStartTime &= 0x80000000;
		if((lnEndTime - lnStartTime) >= tTimeout)
		{
			long lnTime;
			sprintf(szLog," 1(%ld)%ld(lnEndTime) - %ld(lnStartTime) =",dwTotalRead,lnEndTime,lnStartTime);
			lnTime = lnEndTime - lnStartTime;
			//fnWriteLog(szLog,(BYTE *)&lnTime,4);
			log_msg(LOG_LEVEL_ERROR,"1 dwToRead = %d,dwTotalRead = %d",dwToRead,dwTotalRead);
			return PORT_READTIMEOUT_ERROR ;
		}
	}

	return dwTotalRead;
}
#endif

BOOL fGetUSBInfoByPar(int *xProxBusID,int *xProxDeviceID)
{
char chFlag = 0;
int i,inTmp = 0;

	log_msg(LOG_LEVEL_FLOW,"fGetUSBInfoByPar 0 = %s",srTxnData.srParameter.bComPort);
	if(strlen((char *)srTxnData.srParameter.bComPort) <= 0)
	{
		//log_msg(LOG_LEVEL_ERROR,"fGetUSBInfoByPar fail 1");
		return FALSE;
	}

	for(i=0;i<(int )strlen((char *)srTxnData.srParameter.bComPort);i++)
	{
		if(srTxnData.srParameter.bComPort[i] >= '0' && srTxnData.srParameter.bComPort[i] <= '9')
		{
			inTmp *= 10;
			inTmp += srTxnData.srParameter.bComPort[i] - '0';
		}
		else if(srTxnData.srParameter.bComPort[i] == ',')
		{
			if(chFlag == 0)
			{
				*xProxBusID = inTmp;
				inTmp = 0;
				chFlag = 1;
			}
		}
		else
		{
			log_msg(LOG_LEVEL_ERROR,"fGetUSBInfoByPar fail 2");
			return FALSE;
		}
	}

	if(chFlag == 0)
	{
		*xProxBusID = inTmp;
		//log_msg(LOG_LEVEL_ERROR,"fGetUSBInfoByPar fail 3");
		return FALSE;
	}
	else
		*xProxDeviceID = inTmp;

	log_msg(LOG_LEVEL_FLOW,"fGetUSBInfoByPar 1 = %s",srTxnData.srParameter.bComPort);
	return TRUE;

}

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
int inDLEComRecvByLen(BYTE *lpBuf,unsigned long dwToRead,clock_t tTimeout)
{
DWORD			dwRead = 0,dwTotalRead = 0;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
COMSTAT 		ComStat;
#endif
//DWORD			dwErrorFlags;
clock_t lnStartTime, lnEndTime;//1.0.5.5
char szLog[100];//1.0.5.5
int inReadLen = 0;

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		lnStartTime = GetTickCount();
	#else
		lnStartTime = clock();
	#endif

	while(1)
	{
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		ClearCommError(hDev,&dwErrorFlags,&ComStat);
	#endif
		//反覆讀取通訊埠中資料
		inReadLen = dwToRead - dwTotalRead;
	#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
		ReadFile(hDev, &lpBuf[dwTotalRead],1,&dwRead, NULL);
	#else
		dwRead = recv_from2(&lpBuf[dwTotalRead],1,1);
		if(dwRead == SUCCESS)
			dwRead = 1;
	#endif
		//ReadFile(hDev, &lpBuf[dwTotalRead],inReadLen,&dwRead, NULL);
		/*if(dwRead > 0)
		{
			int i;
			BYTE bBuf[1000];

			memset(bBuf,0x00,sizeof(bBuf));

			sprintf((char *)bBuf,"inComRecvByLen (%d),[",dwRead);
			for(i=0;i<(int)dwRead;i++)
			{
				sprintf((char *)&bBuf[strlen((char *)bBuf)],"%02x",lpBuf[dwTotalRead + i]);
			}
			strcat((char *)bBuf,"]");
			log_msg(LOG_LEVEL_FLOW,(char *)bBuf);
		}*/
		if(dwTotalRead > 0 && lpBuf[dwTotalRead] == 0x10 && lpBuf[dwTotalRead - 1] == 0x10)
		{
			log_msg(LOG_LEVEL_FLOW,"inDLEComRecvByLen 1:%d",dwTotalRead);
			lpBuf[dwTotalRead] = 0x00;
			continue;
		}
		else
			dwTotalRead += dwRead;
		if(dwTotalRead >= dwToRead)
			break;

		#if READER_MANUFACTURERS==WINDOWS_CE_API
			lnEndTime = GetTickCount();
		#else
			lnEndTime = clock();
		#endif

		if(lnEndTime < lnStartTime)
			lnStartTime &= 0x80000000;
		if((lnEndTime - lnStartTime) >= tTimeout)
		{
			clock_t lnTime;
			sprintf(szLog," 1(%u)%ld(lnEndTime) - %ld(lnStartTime) =",dwTotalRead,lnEndTime,lnStartTime);
			lnTime = lnEndTime - lnStartTime;
			//fnWriteLog(szLog,(BYTE *)&lnTime,4);
			log_msg(LOG_LEVEL_ERROR,"1 dwToRead = %d,dwTotalRead = %d",dwToRead,dwTotalRead);
			if(dwTotalRead >= 50)
				log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"DLE:",4,&lpBuf[dwTotalRead - 50],50);
			else
				log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"DLE:",4,lpBuf,dwTotalRead);
			return PORT_READTIMEOUT_ERROR ;
		}
	}

	return dwTotalRead;
}
#endif

int inCustomizrdReaderPorotocol(BOOL fEndPorotocol)
{
int inRetVal,inCnt = 0,inLoop = 3;
BYTE bSendData[READER_BUFFER],bRecvData[READER_BUFFER],bBuf[READER_BUFFER];

	do
	{
		memset(bSendData,0x00,sizeof(bSendData));
		memset(bRecvData,0x00,sizeof(bRecvData));
		memset(bBuf,0x00,sizeof(bBuf));
		if(fEndPorotocol == TRUE)
		{
			inCnt = 9 * 2;
			//memcpy(bSendData,"\x55\x55\x55\x55\x55\x55\x55\x55\x55",inCnt / 2);
			memcpy(bSendData,"\x55\x55\x55\x55\x55\x55\x55\x55\x66",inCnt / 2);
		}
		else if(strlen((char *)srTxnData.srParameter.bCRPDataReq) > 0 && strlen((char *)srTxnData.srParameter.bCRPDataReq) <= sizeof(srTxnData.srParameter.bCRPDataReq))
		{
			inCnt = strlen((char *)srTxnData.srParameter.bCRPDataReq);
			fnPack((char *)srTxnData.srParameter.bCRPDataReq,inCnt,bSendData);
		}
		else
		{
			inCnt = strlen((char *)srTxnData.srParameter.bCRPDataReq);
			log_msg(LOG_LEVEL_ERROR,"inCustomizrdReaderPorotocol Fail 1:%d",inCnt);
			log_msg_debug(LOG_LEVEL_ERROR,FALSE,(char *)"CRPDataReq 0:",13,srTxnData.srParameter.bCRPDataReq,inCnt);
			return ICER_REQ_DATA_FAIL;
		}

		log_msg_debug(LOG_LEVEL_ERROR,TRUE,(char *)"CRPDataReq 1:",13,bSendData,inCnt / 2);
		inRetVal = inComSend(bSendData,inCnt / 2);
		if(inRetVal != SUCCESS)
			return inRetVal;

		if(fEndPorotocol == TRUE)
		{
			log_msg(LOG_LEVEL_FLOW,"inCustomizrdReaderPorotocol OK 1!!");
			return SUCCESS;
		}
		else //if(fEndPorotocol == FALSE)
		{
			memset(bRecvData,0x00,sizeof(bRecvData));
			#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
				inRetVal = inComRecvByLen(bRecvData,1,10);//Ack 1
			#else
				inRetVal = inComRecvByLen(bRecvData,1,2);//Ack 1
			#endif
	
			log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"Ack/Nak 1",9,bRecvData,1);
			if(inRetVal != 1)
			{
				log_msg(LOG_LEVEL_ERROR,"inCustomizrdReaderPorotocol Fail 2:%d,[%02x]",inRetVal,bRecvData[0]);
				inLoop--;
				continue;
			}
			if(bRecvData[0] != 0x06)
			{
				log_msg(LOG_LEVEL_ERROR,"inCustomizrdReaderPorotocol Fail 3:%d,[%02x]",inRetVal,bRecvData[0]);
				inLoop--;
				continue;
			}
			
			memset(bRecvData,0x00,sizeof(bRecvData));
			#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
				inRetVal = inComRecvByLen(bRecvData,1,10);//Ack 2
			#else
				inRetVal = inComRecvByLen(bRecvData,1,2);//Ack 2
			#endif
			
			log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"Ack/Nak 2",9,bRecvData,1);
			if(inRetVal != 1)
			{
				log_msg(LOG_LEVEL_ERROR,"inCustomizrdReaderPorotocol Fail 4:%d,[%02x]",inRetVal,bRecvData[0]);
				inLoop--;
				continue;
			}
			if(bRecvData[0] != 0x06)
			{
				log_msg(LOG_LEVEL_ERROR,"inCustomizrdReaderPorotocol Fail 5:%d,[%02x]",inRetVal,bRecvData[0]);
				inLoop--;
				continue;
			}
		}

		if(strlen((char *)srTxnData.srParameter.bCRPDataRes) > 1)
		{
			memset(bRecvData,0x00,sizeof(bRecvData));
			inCnt = strlen((char *)srTxnData.srParameter.bCRPDataRes);
			fnPack((char *)srTxnData.srParameter.bCRPDataRes,inCnt,bBuf);
			inRetVal = inComRecvByLen(bRecvData,inCnt / 2,2);
			
			log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"CRPDataRes 1:",13,bRecvData,inCnt / 2);
			if(inRetVal != inCnt / 2)
			{
				log_msg(LOG_LEVEL_ERROR,"inCustomizrdReaderPorotocol Fail 6:%d,%d",inRetVal,inCnt);
				return PORT_READTIMEOUT_ERROR;
			}
			if(memcmp(bRecvData,bBuf,inCnt / 2))
			{
				log_msg(LOG_LEVEL_ERROR,"inCustomizrdReaderPorotocol Fail 7:%d",inCnt);
				log_msg_debug(LOG_LEVEL_ERROR,TRUE,(char *)"CRPDataRes 2:",13,bBuf,inCnt / 2);
				return PORT_READFILE_ERROR;
			}

			log_msg(LOG_LEVEL_FLOW,"inCustomizrdReaderPorotocol OK 2!!");
			log_msg(LOG_LEVEL_FLOW,"inCustomizrdReaderPorotocol Delay=%d", srTxnData.srParameter.gCRPDelay);

			if (srTxnData.srParameter.gCRPDelay > 0)
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
				Sleep(srTxnData.srParameter.gCRPDelay);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
				usleep(srTxnData.srParameter.gCRPDelay*1000);
#endif
			return SUCCESS;
		}
		else
		{
			log_msg(LOG_LEVEL_FLOW,"inCustomizrdReaderPorotocol Delay=%d", srTxnData.srParameter.gCRPDelay);
			if (srTxnData.srParameter.gCRPDelay > 0)
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
				Sleep(srTxnData.srParameter.gCRPDelay);
#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
				usleep(srTxnData.srParameter.gCRPDelay*1000);
#endif			
			return SUCCESS;
		}
	}
	while(inLoop > 0);

	log_msg(LOG_LEVEL_ERROR,"inCustomizrdReaderPorotocol Fail 8:");

	return PORT_READFILE_ERROR;
}

