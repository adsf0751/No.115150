#ifndef __LIB_ECC_HAL_H__
#define __LIB_ECC_HAL_H__

#define IN
#define OUT
#define INOUT

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
extern unsigned long ECC_GetDataFlashSize_Lib(void);
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

#ifdef __cplusplus
}
#endif

#endif // __LIB_ECC_HAL_H__
