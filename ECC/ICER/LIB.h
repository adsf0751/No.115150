#ifndef LIB_H
#define LIB_H

#include "stdAfx.h"

#define PR_READCARD_CMD					(0x99)


#define OPERATION_OK					0x9000		//成功完成命令
#define PRET_OPERATION_OK					0x9000
#define PRET_DATA_ERR						0x6005	// DATA ERROR
#define PRET_TIME_OUT						0x6088	// 線路不良/Time Out

#define CLA_ERR							0x666E		//CLA 參數錯誤(APDU)
#define INS_ERR							0x666D		//INS 參數錯誤(APDU)
#define P1_P2_ERR						0x666C		//P1 P2 參數錯誤(APDU)
#define LC_LE_ERR						0x6667		//Lc Le 參數錯誤(APDU)
#define DATA_ERR						0x666A		//Data 參數錯誤(APDU)
#define EDC_ERR							0x6668		//EDC錯誤(APDU)
//uart0
#define UNKNOWN_STA					(0x00000000)		//未知狀態
#define PROLOG_RECIEVE_STA			(0x00000001)		//接收ProLog Byte狀態
#define INFO_HEADER_RECIEVE_STA 	(0x00000002)		//接收Information Header狀態
#define INFO_BODY_RECIEVE_STA		(0x00000003)		//接收Information Body狀態
#define EDC_RECIEVE_STA				(0x00000004)		//接收EDC校驗 Byte狀態




#define SERIAL_PORT_REV_TIMEOUT_ERR	 		(0x7F00)		//
#define SAM_ADPU_COMMAND_ERR				(0x7F01)		//
#define CLADPU_COMMAND_ERR					(0x7F02)		//
#define SERIAL_PORT_AUTO_BAUD_RATE			(0x7F03)		//
#define SERIAL_PORT_NOT_READY_ERR			(0x7F04)		//
#define READER_HW_NO_SUPPORT_ERR 			(0x7F05)
#define SERIAL_PORT_NO_DATA_ERR		 		(0x7F06)		//
#define SERIAL_PORT_ADPU_DATA_FORMAT_ERR 	(0x7F07)		//


#define PRET_CLA_INS_ERR					0x6001	// CLA, INS ERROR
#define PRET_P1_P2_ERR						0x6002	// P1, P2 ERROR
#define PRET_LC_LE_ERR						0x6003	// LC, LE ERROR





#define PRET_SAM_ERR						0x6301	// SAM認證失敗
#define SAMKEY_LEN (6)
#define PICC_AUTHENT1A        0x60                                     /*驗證A密鑰*/
#define PICC_AUTHENT1B        0x61                                     /*驗證B密鑰*/
#define PRET_RFID_ACCESS_FAIL				0x6201	// 找不到卡片
#define PRET_RFID_READ_ERR					0x6202	// 讀卡失敗
#define PRET_RFID_WRITE_ERR					0x6203	// 寫卡失敗
#define PRET_RFID_ACCESS_MULTI				0x6204	// 多張卡
#define PRET_RFID_LOAD_FAIL					0x6205	// RC531 load key失敗
#define PRET_RFID_AUTH_FAIL					0x6206	// RC531 Auth失敗



#define ALL_LINE			4


#define CHECK_BAUD_RECIEVE_COUNTERS 9		//modify by james 20120521
#define TIME_20MS					 (2)					//20ms延時
#define MI_OK                 0
#define ANTENNA_POWER_SAVING_TIME		3000//300			//SEC



//----------------------------------------------------------------------------------------------------
extern unsigned char Tscc_cAnswerLen;
extern void ECC_RunIdle(void);
unsigned short Ts_Uart0InfoProcess (void);
//unsigned short Ts_Uart0InfoProcess(unsigned char* inData,unsigned char* outData,unsigned char*  answerLen,unsigned char serialPortMode);
void Ts_SystemParamentInialize (void);
void Ts_SystemParamentInialize2 (char fSlot);
void Ts_EraseKey(void);
void Ts_Timer0Handler (void);

//-------=--------------------------------------------------------------------------------------------
//Uart
//RS232接收訊息


//RS232接收訊息
typedef __packed struct __packed_back
{
	unsigned char cProLog[3];		//NAD,PCB,LEN

	unsigned char cInfoHeader[4];	//CLA,INS,P1,P2

	unsigned char cInfoBody[525];	//訊息Buffer的大小

	unsigned char cEDC;				//校驗
}UART_RECIEVE_INFO;

//RS232回應訊息
typedef __packed struct __packed_back
{
	unsigned char cProLog[3]; 		//NAD,PCB,LEN

	unsigned char cInfoBody[525];	//訊息Buffer的大小

	unsigned char cSW[2];			//狀態SW1,SW2

	unsigned char cEDC;
}UART_SEND_INFO;


#ifdef LIB_DEFINE2
	#if READER_MANUFACTURERS==ANDROID_API
	#else
		unsigned short Ts_Uart0InfoProcess2(UART_RECIEVE_INFO *bRecieveInfo, UART_SEND_INFO *bAnswerInfo, unsigned char *chAnswerLen, volatile unsigned int *iUart0RecieveOK, unsigned char *chSerialPortMode, unsigned char *chProtectFlag);
	#endif
#endif

extern void ECC_SetLED(unsigned char index,unsigned char flag);
//extern void ShowString(unsigned char cLine, unsigned char cColumn, unsigned char * cpString, unsigned char cNegativeDis);
extern unsigned short mADPUCommandProcess(UART_RECIEVE_INFO adpuRev,unsigned char* outData,unsigned char* cAnswerLen );
extern 	void vdShowIdle(char fForceShow);

#define LED0_ON						ECC_SetLED(0,1)
#define LED0_OFF					ECC_SetLED(0,0)
#define LED1_ON						ECC_SetLED(1,1)
#define LED1_OFF					ECC_SetLED(1,0)

#define LED2_ON						ECC_SetLED(2,1)
#define LED2_OFF					ECC_SetLED(2,0)
#define LED3_ON						ECC_SetLED(3,1)
#define LED3_OFF					ECC_SetLED(3,0)


#else
#endif
