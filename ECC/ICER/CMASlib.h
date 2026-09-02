#ifndef __LIB_ECC_HAL_H__
#define __LIB_ECC_HAL_H__

#define IN
#define OUT
#define INOUT

#define CMAS_ERROR		(-1)
#define CMAS_SUCCESS		(0)

/* 2 RS232 Functions */
#define CMAS_COM1	0
#define CMAS_COM2	1

/* 4  Smart Card Function */
#define d_SC_SAM1	1
#define d_SC_SAM2	2
#define d_SC_SAM3	3
#define d_SC_SAM4	4

/* 5  Encrypt & Decrypt Functions */
#define ENC			0
#define DEC			1
#define DES_KEY_TYPE_56		0
#define DES_KEY_TYPE_112	1
#define DES_KEY_TYPE_168	2
#define AES_KEY_TYPE_128	0
#define AES_KEY_TYPE_192	1
#define AES_KEY_TYPE_256	2
#define HASH_TYPE_MD5		0
#define HASH_TYPE_SHA1		1
#define HASH_TYPE_SHA256	2
#define HASH_TYPE_SHA384	3
#define HASH_TYPE_SHA512	4

/* 6 Timer Functions */
#define TIMER_ID_1		0
#define TIMER_ID_2		1
#define TIMER_ID_3		2
#define TIMER_ID_4		3
#define TIMER_ID_5		4
#define TIMER_ID_6		5
#define TIMER_ID_7		6
#define TIMER_ID_8		7
#define TIMER_ID_9		8
#define TIMER_ID_10		9
#define TIMER_MODE_ADD		0
#define TIMER_MODE_DEC		1

/* 7  Display Functions */
#define LED_BLUE		0
#define LED_RED			1
#define LED_YELLOW		2
#define LED_GREEN		3
#define LED_OFF			0
#define LED_ON			1
#define LED_GET			2
#define LCD_BACKLIGHT_OFF	0
#define LCD_BACKLIGHT_ON	1
#define LCD_CLEAR_0		0
#define LCD_CLEAR_1		1
#define LCD_CLEAR_ALL		0xFF

typedef char CMAS_BOOL;

#define CMAS_FALSE           ((CMAS_BOOL) 0)
#define CMAS_TRUE            ((CMAS_BOOL) 1)

#define CMAS_READ_ERROR			(-93)	/* 讀檔失敗 */
#define CMAS_TIMEOUT			(-4)	/* Time out */
#define CMAS_CTLS_DEVICE_ERR	        (-43)	/* 要重新做連線 */
#define CMAS_CTLS_NO_DATA		(-44)	/* 沒有收到資料 */
#define CMAS_NO_RECORD			(-98)   /* 沒有交易記錄 */
#define CMAS_RETRY_ERROR		(-94)	/* 重試 */

#define CMAS_KEY_NAME	"CMASKEY"
#define CMAS_DATA_NAME	"CMASDATA"
#define CMAS_KEY_SIZE	(512)
#define CMAS_DATA_SIZE	(1024*5)

#define d_ECC_RC_OK                         0
#define d_ECC_RC_PARAM_FAIL                 0xF001
#define d_ECC_RC_LED_FLASH_OCCUPY           0xF011
#define d_ECC_RC_LED_FLASH_UNACTIVE         0xF012
#define d_ECC_RC_LED_FLASH_ID_UNMATCH       0xF013
#define d_ECC_RC_AES_SET_KEY_FAIL           0xF014
#define d_ECC_RC_MEM_MALLOC_FAIL            0xF015
#define d_ECC_RC_CREATE_THREAD_FAIL         0xF016
#define d_ECC_RC_TCP_IP_TIMEOUT             0xF017
#define d_ECC_RC_TCP_IP_NOT_CONNECT         0xF018
#define d_ECC_RC_OPEN_FILE_FAIL             0xF019
#define d_ECC_RC_FILE_NOT_EXIST             0xF01A
#define d_ECC_RC_SHA_INIT_FAIL              0xF01B
#define d_ECC_RC_SHA_UPDATE_FAIL            0xF01C
#define d_ECC_RC_SHA_FINAL_FAIL             0xF01D
#define d_ECC_RC_UNSUPPORTED                0xF0FF

#ifdef __cplusplus
extern "C"
{
#endif

#define d_EMVCL_LED_RED                    0x01
#define d_EMVCL_LED_GREEN                  0x02
#define d_EMVCL_LED_YELLOW                 0x04
#define d_EMVCL_LED_BLUE                   0x08
typedef struct
{
	unsigned char	uszLEDIndex;
	unsigned short	usTLedOn;
	unsigned short	usTLedOFF;
	unsigned long	ulDelayTime;
}LED_FLASH_PARA;

extern void ECC_ReadLibVersion_Lib(OUT unsigned char *bpVersion);
extern void ECC_WDTInit_Lib(void);
extern void ECC_WDTFeed_Lib(void);
extern short ECC_WriteSecurityMemory_Lib(IN unsigned long ulAddress, IN unsigned char *bpData, IN unsigned short usDataLen);
extern short ECC_ReadSecurityMemory_Lib(IN unsigned long ulAddress, OUT unsigned char *bpData, IN unsigned short usDataLen);
extern void ECC_Reboot_Lib(void);
extern short ECC_ReadMcuSN_Lib(OUT unsigned char *bpMcuSN, INOUT unsigned char *bpSNLen);
extern short ECC_WriteDataFlash_Lib(IN unsigned long ulAddress, IN unsigned char *bpData, IN unsigned short usDataLen);
extern short ECC_ReadDataFlash_Lib(IN unsigned long ulAddress, OUT unsigned char *bpData, IN unsigned short usDataLen);
extern short ECC_DataFlashSet_Lib(IN unsigned long ulAddress, IN unsigned char bData, IN unsigned short usDataLen);
extern short ECC_GetDataFlashSize_Lib(void);
extern short ECC_ReaderSystemInit_Lib(void);
extern short ECC_GetLocalIP_Lib(OUT unsigned char *usIP);

/********************************************************************
 * RS232 FUNCTIONS                                                  *
 ********************************************************************/
extern short ECC_OpenCOM_Lib(IN unsigned char bPort, IN unsigned long ulBaudrate, IN unsigned char bParity, IN unsigned char bDataBits, IN unsigned char bStopBits);
extern short ECC_CloseCOM_Lib(IN unsigned char bPort);
extern short ECC_COMRxReady_Lib(IN unsigned char bPort, OUT unsigned short *usLen);
extern short ECC_COMRxData_Lib(IN unsigned char bPort, OUT unsigned char *bpData, INOUT unsigned short *usLen);
extern short ECC_COMClearRxBuffer_Lib(IN unsigned char bPort);
extern short ECC_COMTxReady_Lib(IN unsigned char bPort);
extern short ECC_COMTxData_Lib(IN unsigned char bPort, IN unsigned char *bpData, IN unsigned short usLen);

/********************************************************************
 * CONTACTLESS FUNCTIONS                                            *
 ********************************************************************/
extern short ECC_SetReaderID_Lib(IN unsigned char *bpReaderID);
extern short ECC_ReadReaderID_Lib(OUT unsigned char *bpReaderID);
extern short ECC_PcdAntennaOn_Lib(void);
extern short ECC_PcdAntennaOff_Lib(void);
extern short ECC_PcdREQ_Lib(OUT unsigned char *bpATQA);
extern short ECC_PcdWakeUP_Lib(OUT unsigned char *bpATQA);
extern short ECC_PcdAnticollisionAndSelect_Lib(OUT unsigned char *bpCSN, OUT unsigned char *bpSAK, OUT unsigned char *bpCSNLen);
extern short ECC_PcdRATS_Lib(IN unsigned char bABUD, OUT unsigned char *bpATS, INOUT unsigned char *bpATSLen);
extern short ECC_PcdAPDU_Lib(IN unsigned char *bpInData, IN unsigned short usInDataLen, OUT unsigned char *bpOutData, OUT unsigned short *usOutDataLen);
extern short ECC_PcdHalt_Lib(void);
extern short ECC_PcdDeSelect_Lib(void);
extern short ECC_PcdMifareAuthCard_Lib(IN unsigned char bModel, IN unsigned char bBlockNum, IN unsigned char *bpCSN, IN unsigned char bCSNLen, IN unsigned char *bpKey);
extern short ECC_PcdRead_Lib(IN unsigned char bBlockNum, OUT unsigned char *bpData);
extern short ECC_PcdWrite_Lib(IN unsigned char bBlockNum, IN unsigned char *bpData);
extern short ECC_PcdDecrement_Lib(IN unsigned char bBlockNum, IN unsigned char *bpData);
extern short ECC_PcdIncrement_Lib(IN unsigned char bBlockNum, IN unsigned char *bpData);
extern short ECC_PcdRestore_Lib(IN unsigned char bBlockNum);
extern short ECC_PcdTransfer_Lib(IN unsigned char bBlockNum);

/********************************************************************
 * SMART CARD FUNCTIONS                                             *
 ********************************************************************/
extern short ECC_MSamEnable_Lib(IN unsigned char bEableFlag);
extern short ECC_SCReset_Lib(IN unsigned char bSlotNumber, OUT unsigned char *baATR, OUT unsigned char *bATRLen);
extern short ECC_SCSendAPDU_Lib(IN unsigned char bSlotNumber, IN unsigned char *baSBuf, IN unsigned short usSLen, OUT unsigned char *baRBuf, OUT unsigned short *usRLen);
extern short ECC_SCPowerOff_Lib(IN unsigned char bSlotNumber);

/********************************************************************
 * ENCRYPT & DECRYPT FUNCTIONS                                      *
 ********************************************************************/
extern short ECC_GetRandomNum_Lib(OUT unsigned char *bpData);
extern short ECC_DES_Lib(IN unsigned char bMode, IN unsigned char *bpKey, IN unsigned char bKeyType, IN unsigned char *bpInData, IN unsigned short usInLen, OUT unsigned char *bpOutData);
extern short ECC_DES_CBC_Lib(IN unsigned char bMode, IN unsigned char *bpKey, IN unsigned char bKeyType, IN unsigned char *bpIV, IN unsigned char *bpInData, IN unsigned short usInLen, OUT unsigned char *bpOutData);
extern short ECC_AES_Lib(IN unsigned char bMode, IN unsigned char *bpKey, IN unsigned char bKeyType, IN unsigned char *bpInData, IN unsigned short usInLen, OUT unsigned char *bpOutData);
extern short ECC_AES_CBC_Lib(IN unsigned char bMode, IN unsigned char *bpKey, IN unsigned char bKeyType, IN unsigned char *bpIV, IN unsigned char *bpInData, IN unsigned short usInLen, OUT unsigned char *bpOutData);
extern short ECC_HASH_Lib(IN unsigned char bMode, IN unsigned char *bpInData, IN unsigned short usInLen, OUT unsigned char *bpOutData);

/********************************************************************
 * TIMER FUNCTIONS                                                  *
 ********************************************************************/
extern unsigned long ECC_GetTickTime_Lib(void);
extern void ECC_Delay1ms_Lib(IN unsigned long ulT1ms);
extern short ECC_SetTimerCounter_Lib(IN unsigned char bTID, IN unsigned char bMode, IN unsigned long ulTimeValue);
extern unsigned long ECC_GetTimerCounter_Lib(IN unsigned char bTID);
extern short ECC_GetCurrentDateTime_Lib(OUT unsigned char *bDateTime, OUT unsigned long *ulMSec);

/********************************************************************
 * DISPLAY FUNCTIONS                                                *
 ********************************************************************/
extern short ECC_SetLED_Lib(IN unsigned char bLEDID, IN unsigned char bFlag);
extern short ECC_SetLEDFlash_Lib(IN unsigned char bLEDID, IN unsigned short usTLedOn, IN unsigned short usTLedOFF, IN unsigned long ulDelayTime);
extern short ECC_SetLEDFlashStop_Lib(IN unsigned char bLEDID);
extern short ECC_SetLCDBackLight_Lib(IN unsigned char bOnOff);
extern short ECC_LCDClear_Lib(IN unsigned char bClrMode);
extern short ECC_LCDDispString_Lib(IN unsigned char bAddrX, IN unsigned char bAddrY, IN unsigned char *bpStrData);
extern short ECC_GetLCDDispRang_Lib(OUT unsigned char *pbAddrX, OUT unsigned char *pbAddrY, OUT unsigned char *bpRFU);
extern short ECC_Sound_Lib(IN unsigned short usFrequency, IN unsigned short usDuration);

/********************************************************************
 * FILE FUNCTIONS                                                   *
 ********************************************************************/
extern int ECC_FileOpen_Lib(IN unsigned char *bFileName, IN unsigned char *bFileMode);
extern int ECC_FileRead_Lib(IN int FileHandle, OUT unsigned char *bFileData, IN unsigned short usRLen);
extern int ECC_FileWrite_Lib(IN int FileHandle, IN unsigned char *bFileData, IN unsigned short usWLen);
extern int ECC_FileClose_Lib(IN int FileHandle);
extern int ECC_FileSeek_Lib(IN int FileHandle, IN long lnOffset, IN unsigned char bFormWhere);
extern int ECC_FileSize_Lib(IN unsigned char *bFileName);
extern int ECC_FileRemove_Lib(IN unsigned char *bFileName);
extern int ECC_FolderCreat_Lib(IN unsigned char *bFileName);

/********************************************************************
 * TCP/IP FUNCTIONS                                                 *
 ********************************************************************/
extern short ECC_TCPIPConnect_Lib(IN unsigned char *bIP, IN unsigned short usPort, IN unsigned short usTimeOut);
extern short ECC_TCPIPSend_Lib(IN unsigned char *bSendData, IN unsigned short usSendLen, IN unsigned short usTimeOut);
extern short ECC_TCPIPRecv_Lib(OUT unsigned char *bRecvData, OUT unsigned short *usRecvLen, IN unsigned short usTimeOut);
extern short ECC_TCPIPClose_Lib(void);

/********************************************************************
 * DATA TRANSFER FUNCTIONS                                          *
 ********************************************************************/
extern short ECC_UnixToDateTime_Lib(IN unsigned char *bUnixTime, OUT unsigned char *bDate);
extern short ECC_DateTimeToUnix_Lib(IN unsigned char *bDate, OUT unsigned char *bUnixTime);
extern short ECC_INT64ToASCII_Lib(IN unsigned char *bData, OUT unsigned char *bAsciiData);
extern short ECC_UINT64ToASCII_Lib(IN unsigned char *bData, OUT unsigned char *bAsciiData);
extern short ECC_ASCIIToINT64_Lib(IN unsigned char *bAsciiData, IN unsigned short usLen, OUT unsigned char *bData);
extern short ECC_ASCIIToUINT64_Lib(IN unsigned char *bAsciiData, IN unsigned short usLen, OUT unsigned char *bData);

extern short ECC_SSLConnect_Lib(IN unsigned char* bIP, IN unsigned short usPort, IN unsigned short usTimeOut);
extern short ECC_SSLSend_Lib(IN unsigned char* bSendData, IN unsigned short usSendLen, IN unsigned short usTimeOut);
extern short ECC_SSLRecv_Lib(OUT unsigned char* bRecvData, INOUT unsigned short* usRecvLen, IN unsigned short usTimeOut);
extern short ECC_SSLClose_Lib(void);

extern void ECC_Debug_On_Lib(void);
extern void ECC_Debug_Off_Lib(void);

#ifdef __cplusplus
}
#endif

#endif // __LIB_ECC_HAL_H__

#define _AP_PUB_PATH_		"/home/ap/pub/"
#define _READER_ID_FILE_NAME_	"ECC_READER_ID.dat"