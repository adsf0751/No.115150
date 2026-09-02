
#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
#include "Global.h"

/*
#include <windows.h>
#include <iostream.h>
#include <time.h>
#include <stdio.h>

#include "CallAPI.h"
#include "Error.h"
#include "Function.h"
#include "TCPIP.h"
*/

//extern STRUCT_TXN_DATA srTxnData;
#if READER_MANUFACTURERS==ANDROID_API
extern STRUCT_TXN_DATA srTxnData;
extern STRUCT_XML_ACTION_DATA srXMLActionData;
extern STRUCT_XML_DOC srXMLData;
#endif

BYTE ucSAMSN[4],ucReaderFWVersion[6];

#ifdef LIB_DEFINE

	#ifdef LIB_DEFINE2

		UART_RECIEVE_INFO g_sUart0RecieveInfo2;
		UART_SEND_INFO    g_sUart0AnswerInfo2;
		unsigned char chAnswerLen;
		volatile unsigned int iUart0RecieveOK;
		unsigned char chSerialPortMode;
		unsigned char chProtectFlag;

	#else

		unsigned int g_iUart0RecieveOK;
		unsigned char g_SerialPortMode;

	#endif

	UART_RECIEVE_INFO g_sUart0RecieveInfo;
	UART_SEND_INFO    g_sUart0AnswerInfo;
	char g_cProtectFlag;

#endif

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
STRUCT_READER_COMMAND st_ReaderCommand[DLL_TOTAL_CNT] = {
//	inTxnType					bReaderCommand				inInFuncType	inOutFuncType	inTMInLen	inTMOutLen9000	inTMOutLen6415	inTMOutLen6103	bReaderCommand				inInFuncType	inOutFuncType	inTMInLen	inTMOutLen9000	inTMOutLen6415	inTMOutLen6103
	{DLL_OPENCOM				,"OpenCom"					,1				,0				,0			,0				,0				,0				,"OpenCom"					,1				,0				,0			,0				,0				,0				},
	{DLL_SIGNONQUERY			,"PR_QuotaSignOnQuery"		,2				,4				,0			,56				,0				,0				,"PPR_SignOnQuery"			,2				,4				,0			,105			,0				,0				},
	{DLL_RESET					,"PR_QuotaReset"			,3				,4				,38			,162			,0				,0				,"PPR_Reset"				,3				,4				,55			,352			,0				,0				},
	{DLL_RESET_OFF				,"PR_QuotaReset"			,3				,4				,38 		,162			,0				,0				,"PPR_Reset"				,3				,4				,55 		,352			,0				,0				},
	{DLL_SIGNON					,"PR_QuotaSignOn"			,3				,5				,32			,0				,0				,0				,"PPR_SignOn"				,3				,4				,193		,59				,0				,0				},
	{DLL_READ_CODE_VERSION		,"PR_ReadCodeVersion"		,2				,4				,0			,8				,0				,0				,"PPR_ReadCodeVersion"		,2				,4				,0			,10				,0				,0				},
	{DLL_READ_CARD_BASIC_DATA	,"PR_ReadCardLogData"		,2				,4				,0			,154			,0				,40				,"PPR_ReadCardBasicData"	,3				,4				,27			,290			,0				,197			},
	{DLL_DEDUCT_VALUE			,"PR_TxnVerify"				,3				,4				,40			,43				,140			,40				,"PPR_TxnReqOffline"		,3				,4				,49			,76				,231			,197			},
	{DLL_DEDUCT_VALUE_AUTH		,"PR_AuthTxnVerify"			,3				,4				,9			,234			,0				,0				,"PPR_AuthTxnOffline"		,3				,4				,109		,529			,0				,0				},
	{DLL_ADD_VALUE				,"PR_AddValue"				,3				,4				,41			,178			,0				,40				,"PPR_TxnReqOnline"			,3				,4				,49			,76				,317			,197			},
	{DLL_ADD_VALUE_AUTH			,"PR_AuthAddValue"			,3				,4				,12			,201			,0				,0				,"PPR_AuthTxnOnline"		,3				,4				,117		,503			,0				,0				},
	{DLL_VOID_TXN				,"PR_CancelTxn"				,3				,4				,41			,178			,0				,40				,"PPR_CancelTxn"			,3				,4				,49			,79				,317			,197			},
	{DLL_AUTOLOAD				,"PR_AddValue"				,3				,4				,41			,178			,0				,40				,"PPR_TxnReqOnline"			,3				,4				,49			,76				,317			,197			},
	{DLL_AUTOLOAD_AUTH			,"PR_AuthAddValue"			,3				,4				,12			,201			,0				,0				,"PPR_AuthAutoload"			,3				,4				,120		,506			,0				,0				},
	{DLL_READ_DONGLE_LOG		,"PR_ReadDeductLog"			,3				,4				,5			,234			,0				,0				,"PPR_ReadDongleDeduct"		,3				,4				,5			,529			,0				,0				},
	{DLL_READ_CARD_LOG			,""							,3				,4				,0			,0				,0				,0				,"PPR_ReadCardDeduct"		,3				,4				,1			,462			,0				,0				},
	{DLL_TX_REFUND				,"PR_TxRefund"				,3				,4				,61			,180			,0				,40				,"PPR_TxRefund"				,3				,4				,68			,0				,317			,197			},
	{DLL_TX_REFUND_AUTH			,"PR_TxAuthRefund"			,3				,4				,15			,435			,0				,0				,"PPR_TxAuthRefund"			,3				,4				,117		,1032			,0				,0				},
	{DLL_TX_ADD_VALUE			,"PR_TxAddValue"			,3				,4				,41			,178			,0				,40				,"PPR_TxnReqOnline"			,3				,4				,49			,76				,317			,197			},
	{DLL_TX_ADD_VALUE_AUTH		,"PR_TxAuthAddValue"		,3				,4				,15			,201			,0				,0				,"PPR_AuthTxnOnline"		,3				,4				,117		,503			,0				,0				},
	{DLL_SET_VALUE				,"PR_SetValue"				,3				,4				,34			,111			,0				,0				,"PPR_SetValue"				,3				,4				,41			,0				,226			,0				},
	{DLL_SET_VALUE_AUTH			,"PR_AuthSetValue"			,3				,4				,12			,87				,0				,0				,"PPR_AuthSetValue"			,3				,4				,117		,67				,0				,0				},
	{DLL_AUTOLOAD_ENABLE		,"PR_AutoloadEnable"		,3				,4				,36			,181			,0				,0				,"PPR_AutoloadEnable"		,3				,4				,49			,76				,320			,197			},
	{DLL_AUTOLOAD_ENABLE_AUTH	,"PR_AuthAutoloadEnable"	,3				,4				,9			,204			,0				,0				,"PPR_AuthAutoloadEnable"	,3				,4				,120		,506			,0				,0				},
	{DLL_ACCUNT_LINK			,"PR_AccuntLink"			,3				,4				,36			,181			,0				,0				,"PPR_AccuntLink"			,3				,4				,49			,76				,320			,197			},
	{DLL_ACCUNT_LINK_AUTH		,"PR_AuthAccuntLink"		,3				,4				,9			,204			,0				,0				,"PPR_AuthAccuntLink"		,3				,4				,120		,506			,0				,0				},
	{DLL_LOCK_CARD				,"PR_LockCard"				,3				,4				,18			,40				,0				,0				,"PPR_LockCard"				,3				,4				,33			,197			,0				,0				},
	{DLL_READ_CARD_NUMBER		,"PR_ReadCardNumber"		,2				,4				,0			,4				,0				,0				,"PPR_ReadCardNumber"		,3				,4				,9			,18				,0				,0				},
	{DLL_ADD_VALUE2				,"PR_AddValue"				,3				,4				,41			,178			,0				,40				,"PPR_TxnReqOnline4"		,3				,4				,49			,76				,317			,197			},
	{DLL_MULTI_SELECT			,""							,2				,4				,0			,4				,0				,0				,"SA_MultiMifareSelectCard2",3				,4				,18			,21				,0				,0				},
	{DLL_ANTENN_CONTROL			,""							,3				,4				,41			,178			,0				,40				,"PPR_AntennaControl"		,4				,4				,1			,0				,0				,0				},
	{DLL_TAXI_READ 				,"" 						,3				,4				,41 		,178			,0				,40 			,""							,4				,4				,1			,0				,0				,0				},
	{DLL_TAXI_DEDUCT	 		,"" 						,3				,4				,41 		,178			,0				,40 			,""							,4				,4				,1			,0				,0				,0				},
	{DLL_ADD_OFFLINE 			,"PR_AddValue"				,3				,4				,41 		,178			,0				,40 			,"PPR_TxnReqOnline4"		,3				,4				,49 		,76 			,317			,197			},
	{DLL_MATCH					,"PPR_Match"				,3				,4				,38 		,162			,0				,0				,"PPR_Match"				,3				,4				,55 		,352			,0				,0				},
	{DLL_MATCH_AUTH				,"PPR_MatchAuth"			,3				,5				,32 		,0				,0				,0				,"PPR_MatchAuth"			,3				,4				,193		,59 			,0				,0				},
	{DLL_FAST_READ_CARD			,"PR_FastReadCard"			,5				,0				,0			,0				,0				,0				,"PPR_FastReadCard"			,5				,0				,0			,0				,0				,0				},
	{DLL_CLOSECOM				,"CloseCom"					,5				,0				,0			,0				,0				,0				,"CloseCom"					,5				,0				,0			,0				,0				,0				}
	};

HMODULE hDLL;

#else//READER_MANUFACTURERS==LINUX_API
STRUCT_READER_COMMAND st_ReaderCommand[DLL_TOTAL_CNT] = {
//	inTxnType					bReaderCommand				inInFuncType	inTMInLen	inTMOutLen9000	inTMOutLen6415	inTMOutLen6103
	{DLL_OPENCOM				,"OpenCom"					,1				,0			,0				,0				,0				},
	{DLL_SIGNONQUERY			,"PPR_SignOnQuery"			,2				,0			,105			,0				,0				},
	{DLL_RESET					,"PPR_Reset"				,3				,56			,352			,0				,0				},
	{DLL_RESET_OFF				,"PPR_Reset"				,3				,56 		,352			,0				,0				},
	{DLL_SIGNON					,"PPR_SignOn"				,3				,193		,59				,0				,0				},
	{DLL_READ_CODE_VERSION		,"PPR_ReadCodeVersion"		,2				,0			,10				,0				,0				},
	{DLL_READ_CARD_BASIC_DATA	,"PPR_ReadCardBasicData"	,3				,27			,290			,0				,197			},
	{DLL_DEDUCT_VALUE			,"PPR_TxnReqOffline"		,3				,49			,76				,231			,197			},
	{DLL_DEDUCT_VALUE_AUTH		,"PPR_AuthTxnOffline"		,3				,109		,529			,0				,0				},
	{DLL_ADD_VALUE				,"PPR_TxnReqOnline"			,3				,49			,76				,317			,197			},
	{DLL_ADD_VALUE_AUTH			,"PPR_AuthTxnOnline"		,3				,117		,503			,0				,0				},
	{DLL_VOID_TXN				,"PPR_CancelTxn"			,3				,49			,79				,317			,197			},
	{DLL_AUTOLOAD				,"PPR_TxnReqOnline"			,3				,49			,76				,317			,197			},
	{DLL_AUTOLOAD_AUTH			,"PPR_AuthAutoload"			,3				,120		,506			,0				,0				},
	{DLL_READ_DONGLE_LOG		,"PPR_ReadDongleDeduct"		,3				,5			,529			,0				,0				},
	{DLL_READ_CARD_LOG			,"PPR_ReadCardDeduct"		,3				,1			,462			,0				,0				},
	{DLL_TX_REFUND				,"PPR_TxRefund"				,3				,68			,0				,317			,197			},
	{DLL_TX_REFUND_AUTH			,"PPR_TxAuthRefund"			,3				,117		,1032			,0				,0				},
	{DLL_TX_ADD_VALUE			,"PPR_TxnReqOnline"			,3				,49			,76				,317			,197			},
	{DLL_TX_ADD_VALUE_AUTH		,"PPR_AuthTxnOnline"		,3				,117		,503			,0				,0				},
	{DLL_SET_VALUE				,"PPR_SetValue"				,3				,41			,0				,226			,0				},
	{DLL_SET_VALUE_AUTH			,"PPR_AuthSetValue"			,3				,117		,67				,0				,0				},
	{DLL_AUTOLOAD_ENABLE		,"PPR_AutoloadEnable"		,3				,49			,76				,320			,197			},
	{DLL_AUTOLOAD_ENABLE_AUTH	,"PPR_AuthAutoloadEnable"	,3				,120		,506			,0				,0				},
	{DLL_ACCUNT_LINK			,"PPR_AccuntLink"			,3				,49			,76				,320			,197			},
	{DLL_ACCUNT_LINK_AUTH		,"PPR_AuthAccuntLink"		,3				,124		,510			,0				,0				},
	{DLL_LOCK_CARD				,"PPR_LockCard"				,3				,33			,197			,0				,0				},
	{DLL_READ_CARD_NUMBER		,"PPR_ReadCardNumber"		,3				,9			,18				,0				,0				},
	{DLL_ADD_VALUE2				,"PPR_TxnReqOnline4"		,3				,49			,76				,317			,197			},
	{DLL_MULTI_SELECT			,"SA_MultiMifareSelectCard2",3				,18			,21				,0				,0				},
	{DLL_ANTENN_CONTROL			,"PPR_AntennaControl"		,4				,1			,0				,0				,0				},
	{DLL_TAXI_READ 				,""							,4				,1			,0				,0				,0				},
	{DLL_TAXI_DEDUCT	 		,""							,4				,1			,0				,0				,0				},
	{DLL_ADD_OFFLINE 			,"PPR_TxnReqOnline4"		,3				,49 		,76 			,317			,197			},
	{DLL_MATCH					,"PPR_Match"				,3				,56 		,352			,0				,0				},
	{DLL_MATCH_AUTH				,"PPR_MatchAuth"			,3				,193		,59 			,0				,0				},
	{DLL_FAST_READ_CARD			,"PPR_FastReadCard"			,5				,0			,0				,0				,0				},
	{DLL_CLOSECOM				,"CloseCom"					,5				,0			,0				,0				,0				}
	};

#endif

BYTE bReadCommandIn[256];
BYTE bReadCommandOut[READER_BUFFER];

STRUCT_READER_COMMAND2 st_ReaderCommand2[DLL_TOTAL_CNT] = {
//	inTxnType					TimeOut CardAccess	bReaderCommand					CLA 	INS 	P1		P2		LC		LE		R6APDUFunction							R6TMFunction_OK1					R6TMFunction_OK2					R6TMFunction_Lock1				R6TMFunction_Lock2			SIS2APDUFunction					SIS2TMFunction_OK				SIS2TMFunction_Lock1			SIS2TMFunction_Lock2		CMSAPDUFunction 					CMSTMFunction_OK				CMSTMFunction_Lock1 		CMSTMFunction_Lock2		ICERAPDUFunction 					ICERTMFunction_OK				ICERTMFunction_Lock1 		ICERTMFunction_Lock2
	{DLL_OPENCOM				,0		,'0' 		,{"OpenCom" 					,0x00	,0x00	,0x00	,0x00	,0x00	,0x00	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_SIGNONQUERY			,4000	,'0' 		,{"PPR_SignOnQuery" 			,0x80	,0x03	,0x00	,0x00	,0x00	,0x28	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_RESET					,4000	,'0' 		,{"PPR_Reset"					,0x80	,0x01	,0x00	,0x00	,0x40	,0xFA	,inBuildResetCommand					,inBuildResetData					,inBuildResetData					,inBuildNullTM					,inBuildNullTM				,inBuildResetCommand				,inBuildResetData				,inBuildNullTM					,inBuildNullTM				,inBuildResetCommand				,inBuildCMSResetTM				,inBuildNullTM				,inBuildNullTM			,inBuildResetCommand				,inBuildResetData				,inBuildNullTM				,inBuildNullTM		}},
	{DLL_RESET_OFF				,4000	,'0' 		,{"PPR_ResetOff"				,0x80	,0x01	,0x00	,0x01	,0x40	,0xFA	,inBuildResetOffCommand 				,inBuildResetData					,inBuildResetData					,inBuildNullTM					,inBuildNullTM				,inBuildResetOffCommand 			,inBuildResetData				,inBuildNullTM					,inBuildNullTM				,inBuildResetOffCommand 			,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_SIGNON 				,4000	,'0' 		,{"PPR_SignOn"					,0x80	,0x02	,0x00	,0x00	,0x80	,0x1D	,inBuildSignOnCommand					,inBuildSignOnData					,inBuildSignOnData					,inBuildNullTM					,inBuildNullTM				,inBuildSignOnCommand				,inBuildSignOnData				,inBuildNullTM					,inBuildNullTM				,inBuildCMSSignOnAPDU				,inBuildCMSSignOnTM 			,inBuildNullTM				,inBuildNullTM			,inBuildSignOnCommand				,inBuildSignOnData				,inBuildNullTM				,inBuildNullTM		}},
	{DLL_READ_CODE_VERSION		,4000	,'0' 		,{"PPR_ReadCodeVersion" 		,0x80	,0x51	,0x01	,0x00	,0x80	,0x0A	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_READ_CARD_BASIC_DATA	,12000	,'0' 		,{"PPR_ReadCardBasicData"		,0x80	,0x51	,0x04	,0x01	,0x10	,0xFA	,inBuildReadCardBasicDataCommand		,inBuildReadCardBasicDataData	,inBuildReadCardBasicDataData	,inBuildLockCardData8			,inBuildLockCardData9		,inBuildNullAPDU			,inBuildNullTM					,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildReadCardBasicDataCommand	,inBuildReadCardBasicDataData	,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildReadCardBasicDataCommand	,inBuildReadCardBasicDataData	,inBuildNullTM				,inBuildNullTM		}},
	{DLL_DEDUCT_VALUE			,12000	,'0' 		,{"PPR_TxnReqOffline"			,0x80	,0x32	,0x01	,0x00	,0x40	,0xFA	,inBuildTxnReqOfflineCommand			,inBuildTxnReqOfflineData1		,inBuildTxnReqOfflineData2		,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOfflineCommand		,inBuildTxnReqOfflineData2		,inBuildNullTM					,inBuildNullTM				,inBuildTxnReqOfflineCommand		,inBuildCMSTxnReqOfflineTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_DEDUCT_VALUE_AUTH		,4000	,'1' 		,{"PPR_AuthTxnOffline"			,0x80	,0x32	,0x02	,0x00	,0x16	,0x40	,inBuildAuthTxnOfflineCommand		,inBuildAuthTxnOfflineData		,inBuildAuthTxnOfflineData		,inBuildNullTM					,inBuildNullTM				,inBuildAuthTxnOfflineCommand		,inBuildAuthTxnOfflineSIS2		,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOfflineAPDU		,inBuildCMSAuthTxnOfflineTM		,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_ADD_VALUE				,12000	,'0' 		,{"PPR_TxnReqOnline"			,0x80	,0x11	,0x01	,0x00	,0x40	,0xFD	,inBuildTxnReqOnlineCommand 			,inBuildTxnReqOnlineData1			,inBuildTxnReqOnlineData2			,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOnlineCommand 		,inBuildTxnReqOnlineData2		,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildTxnReqOnlineCommand 		,inBuildCMSTxnReqOnlineTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildTxnReqOnlineCommand			,inBuildICERTxnReqOnlineTM		,inBuildNullTM				,inBuildNullTM		}},
	{DLL_ADD_VALUE_AUTH 		,4000	,'1' 		,{"PPR_AuthTxnOnline"			,0x80	,0x11	,0x02	,0x00	,0x28	,0x40	,inBuildAuthTxnOnlineCommand1		,inBuildAuthTxnOnlineData			,inBuildAuthTxnOnlineData			,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildAuthTxnOnlineSIS2		,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildCMSAuthTxnOnlineTM		,inBuildNullTM				,inBuildNullTM			,inBuildICERAuthTxnOnlineAPDU		,inBuildICERAuthTxnOnlineTM		,inBuildNullTM				,inBuildNullTM		}},
	{DLL_VOID_TXN				,12000	,'0' 		,{"PPR_CancelTxn"				,0x80	,0x11	,0x01	,0x00	,0x40	,0xFD	,inBuildTxnReqOnlineCommand 			,inBuildTxnReqOnlineData1			,inBuildTxnReqOnlineData2			,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOnlineCommand 		,inBuildTxnReqOnlineData2		,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildTxnReqOnlineCommand 		,inBuildCMSTxnReqOnlineTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_AUTOLOAD				,12000	,'0' 		,{"PPR_Autoload"				,0x80	,0x11	,0x01	,0x00	,0x40	,0xFD	,inBuildTxnReqOnlineCommand 			,inBuildTxnReqOnlineData1			,inBuildTxnReqOnlineAutoload		,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOnlineCommand 		,inBuildTxnReqOnlineData2		,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildTxnReqOnlineCommand 		,inBuildCMSTxnReqOnlineTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_AUTOLOAD_AUTH			,4000	,'1' 		,{"PPR_AuthAutoload"			,0x80	,0x11	,0x02	,0x00	,0x28	,0x40	,inBuildAuthAutoLoadCommand 			,inBuildAuthAutoloadData			,inBuildAuthAutoloadData			,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildAuthTxnOnlineSIS2		,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildCMSAuthTxnOnlineTM		,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_READ_DONGLE_LOG		,4000	,'0' 		,{"PPR_ReadDongleDeduct"		,0x80	,0x51	,0x05	,0x00	,0x02	,0xCA	,inBuildReadDongleDeductCommand 		,inBuildReadDongleDeductData		,inBuildReadDongleDeductData		,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildReadDongleDeductCommand		,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_READ_CARD_LOG			,4000	,'0' 		,{"PPR_ReadCardDeduct"			,0x80	,0x51	,0x06	,0x01	,0x01	,0xC6	,inBuildReadCardDeductCommand		,inBuildReadCardDeductData		,inBuildReadCardDeductData		,inBuildNullTM					,inBuildNullTM				,inBuildReadCardDeductCommand		,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildReadCardDeductCommand		,inBuildReadCardDeductData		,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_TX_REFUND				,0		,'0' 		,{"RFU" 						,0x00	,0x00	,0x00	,0x00	,0x00	,0x00	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_TX_REFUND_AUTH 		,0		,'1' 		,{"RFU" 						,0x00	,0x00	,0x00	,0x00	,0x00	,0x00	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_TX_ADD_VALUE			,0		,'0' 		,{"RFU" 						,0x00	,0x00	,0x00	,0x00	,0x00	,0x00	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_TX_ADD_VALUE_AUTH		,0		,'1' 		,{"RFU" 						,0x00	,0x00	,0x00	,0x00	,0x00	,0x00	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_SET_VALUE				,12000	,'0' 		,{"PPR_SetValue" 				,0x80	,0x31	,0x01	,0x00	,0x40	,0xB4	,inBuildSetValueCommand2				,inBuildSetValue2Data				,inBuildSetValue2Data				,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildSetValueCommand2			,inBuildCMSSetValueTM			,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_SET_VALUE_AUTH 		,4000	,'1' 		,{"PPR_AuthSetValue" 			,0x80	,0x31	,0x02	,0x00	,0x28	,0x20	,inBuildAuthSetValueCommand2			,inBuildAuthSetValue2Data			,inBuildAuthSetValue2Data			,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthSetValueAPDU			,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_AUTOLOAD_ENABLE		,12000	,'0' 		,{"PPR_AutoloadEnable"			,0x80	,0x11	,0x01	,0x00	,0x40	,0xFD	,inBuildTxnReqOnlineCommand 			,inBuildTxnReqOnlineData1			,inBuildAutoloadEnableData2 		,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOnlineCommand 		,inBuildTxnReqOnlineData2		,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildTxnReqOnlineCommand 		,inBuildCMSTxnReqOnlineTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_AUTOLOAD_ENABLE_AUTH	,4000	,'1' 		,{"PPR_AuthAutoloadEnable"		,0x80	,0x11	,0x02	,0x00	,0x28	,0x40	,inBuildAuthAutoLoadEnableCommand2	,inBuildAuthAutoloadEnableData2 	,inBuildAuthAutoloadEnableData2 	,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildAuthTxnOnlineSIS2		,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildCMSAuthTxnOnlineTM		,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_ACCUNT_LINK			,12000	,'0' 		,{"PPR_AccuntLink"				,0x80	,0x11	,0x01	,0x00	,0x40	,0xFD	,inBuildTxnReqOnlineCommand 			,inBuildTxnReqOnlineData1			,inBuildAutoloadEnableData2 		,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOnlineCommand 		,inBuildTxnReqOnlineData2		,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildTxnReqOnlineCommand 		,inBuildCMSTxnReqOnlineTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_ACCUNT_LINK_AUTH		,4000	,'1' 		,{"PPR_AuthAccuntLink"			,0x80	,0x11	,0x02	,0x00	,0x28	,0x40	,inBuildAuthAccuntLinkCommand		,inBuildAuthAccuntLinkData		,inBuildAuthAccuntLinkData		,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildAuthTxnOnlineSIS2		,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildCMSAuthTxnOnlineTM		,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_LOCK_CARD				,12000	,'0' 		,{"PPR_LockCard"				,0x80	,0x41	,0x01	,0x00	,0x0E	,0x28	,inBuildLockCardCommand2				,inBuildLockCardData9				,inBuildLockCardData9				,inBuildLockCardData8			,inBuildLockCardData9		,inBuildLockCardCommand2			,inBuildLock2SIS2				,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildLockCardCommand2			,inBuildCMASLockTM2				,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_READ_CARD_NUMBER		,1000	,'0' 		,{"PPR_ReadCardNumber"			,0x80	,0x51	,0x02	,0x02	,0x04	,0x09	,inBuildNullAPDU						,inBuildReadCardNumberData		,inBuildReadCardNumberData		,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildReadCardNumberData		,inBuildReadCardNumberData		,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_ADD_VALUE2 			,12000	,'0' 		,{"PPR_TxnReqOnline"			,0x80	,0x11	,0x01	,0x00	,0x40	,0xFD	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildTxnReqOnlineCommand 		,inBuildCMSTxnReqOnlineTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_MULTI_SELECT			,30000	,'0' 		,{"SA_MultiMifareSelectCard2"	,0x64	,0x99	,0x70	,0x10	,0x14	,0x20	,inBuildMMSelectCard2Command			,inBuildMMSelectCard2Data			,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildMMSelectCard2Command		,inBuildMMSelectCard2Data		,inBuildNullTM					,inBuildNullTM				,inBuildMMSelectCard2Command		,inBuildCMASMMSelectCard2Data	,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_ANTENN_CONTROL 		,4000	,'0' 		,{"PPR_AntennaControl"			,0x68	,0x54	,0x01	,0x00	,0x00	,0x00	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_TAXI_READ				,12000	,'0' 		,{"PPR_TaxiRead"				,0x80	,0x01	,0x03	,0x00	,0x20	,0xA0	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildTaxiReadCommand				,inBuildCMSTaxiReadTM			,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_TAXI_DEDUCT			,4000	,'1' 		,{"PPR_TaxiDeduct"				,0x80	,0x01	,0x04	,0x00	,0x50	,0x90	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildCMSTaxiDeductCommand		,inBuildCMSTaxiDeductTM 		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_ADD_OFFLINE			,12000	,'1'		,{"PPR_TxnReqOffline"			,0x80	,0x32	,0x01	,0x00	,0x40	,0xFA	,inBuildTxnReqOfflineCommand			,inBuildTxnReqOfflineData1		,inBuildTxnReqOfflineData2		,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOfflineCommand		,inBuildTxnOfflineSIS2		,inBuildNullTM					,inBuildNullTM				        ,inBuildTxnReqOfflineCommand		,inBuildTxnOfflineSIS2		,inBuildCMASLockTM1 		,inBuildCMASLockTM2 	,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_MATCH					,4000	,'0'		,{"PPR_Match"					,0x80	,0x01	,0x01	,0x00	,0x40	,0xFA	,inBuildResetCommand					,inBuildResetData					,inBuildResetData					,inBuildNullTM					,inBuildNullTM				,inBuildResetCommand				,inBuildResetData				,inBuildNullTM					,inBuildNullTM				,inBuildResetCommand				,inBuildCMSResetTM				,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_MATCH_AUTH				,4000	,'0'		,{"PPR_MatchAuth"				,0x80	,0x02	,0x01	,0x00	,0x80	,0x1D	,inBuildSignOnCommand					,inBuildSignOnData					,inBuildSignOnData					,inBuildNullTM					,inBuildNullTM				,inBuildSignOnCommand				,inBuildSignOnData				,inBuildNullTM					,inBuildNullTM				,inBuildCMSSignOnAPDU				,inBuildCMSSignOnTM 			,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_FAST_READ_CARD			,12000	,'0' 		,{"PPR_FastReadCard"			,0x68	,0x54	,0x02	,0x00	,0x00	,0x00	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_FAST_READ_CARD2		,12000	,'0' 		,{"PPR_FastReadCard2"			,0x68	,0x54	,0x02	,0x01	,0x02	,0x0C	,inBuildFastReadCardAPDU				,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildFastReadCardAPDU			,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildFastReadCardAPDU			,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildFastReadCardAPDU			,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_RESET2					,4000	,'0' 		,{"PPR_Reset"					,0x80	,0x01	,0x00	,0x00	,0x40	,0xFA	,inBuildResetCommand					,inBuildResetData					,inBuildResetData					,inBuildNullTM					,inBuildNullTM				,inBuildResetCommand				,inBuildResetData				,inBuildNullTM					,inBuildNullTM				,inBuildResetCommand				,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_MATCH2					,4000	,'0'		,{"PPR_Match"					,0x80	,0x01	,0x01	,0x00	,0x40	,0xFA	,inBuildResetCommand					,inBuildResetData					,inBuildResetData					,inBuildNullTM					,inBuildNullTM				,inBuildResetCommand				,inBuildResetData				,inBuildNullTM					,inBuildNullTM				,inBuildResetCommand				,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_S_SET_VALUE			,12000	,'0' 		,{"PPR_StudentSetValue"			,0x80	,0x31	,0x03	,0x00	,0x60	,0xFD	,inBuildStudentSetValueCommand 		,inBuildStudentSetValueData1		,inBuildStudentSetValueData2		,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOnlineCommand 		,inBuildTxnReqOnlineData2		,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildStudentSetValueCommand 		,inBuildCMSSetStudentValueTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_S_SET_VALUE_AUTH 		,4000	,'1' 		,{"PPR_AuthStudentSetValue"		,0x80	,0x31	,0x04	,0x00	,0x60	,0x60	,inBuildAuthStudentSetValueCommand	,inBuildAuthStudentSetValueData	,inBuildAuthStudentSetValueData	,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildAuthTxnOnlineSIS2		,inBuildNullTM					,inBuildNullTM				,inBuildAuthStudentSetValueCommand		,inBuildNullTM				,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_THSRC_READ				,12000	,'0' 		,{"PPR_THSRCRead"				,0x80	,0x0D	,0x01	,0x00	,0x10	,0xFA	,inBuildTHSRCReadCommand				,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_E_ADD_VALUE			,12000	,'0' 		,{"PPR_TxnReqOnline"			,0x80	,0x11	,0x01	,0x00	,0x40	,0xFD	,inBuildTxnReqOnlineCommand			,inBuildICERTxnReqOnlineTM		,inBuildICERTxnReqOnlineTM		,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU			 		,inBuildNullTM					,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildNullAPDU			 		,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildTxnReqOnlineCommand			,inBuildICERTxnReqOnlineTM		,inBuildNullTM				,inBuildNullTM		}},
	{DLL_VERIFY_HOST_CRYPT		,4000	,'0' 		,{"PPR_VerifyHostCrypt"			,0x80	,0x71	,0x02	,0x00	,0x00	,0x00	,inCMSVerifyHCryptCommand	 			,inBuildICERETxnData				,inBuildICERETxnData				,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU			 		,inBuildICERETxnSIS2			,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildNullAPDU			 		,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inCMSVerifyHCryptCommand			,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_E_QR_TXN				,0		,'0' 		,{"PPR_QRTxn"					,0x00	,0x00	,0x00	,0x00	,0x00	,0x00	,inBuildNullAPDU			 			,inBuildICERETxnData				,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU			 		,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU			 		,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildICERQRTxnTM				,inBuildNullTM				,inBuildNullTM		}},
	{DLL_QR_VERIFY_HOST_CRYPT	,4000	,'0' 		,{"PPR_QRVerifyHostCrypt"		,0x80	,0x71	,0x02	,0x00	,0x00	,0x00	,inCMSVerifyHCryptCommand	 			,inBuildICEREQRTxnData			,inBuildICEREQRTxnData			,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU			 		,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU			 		,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inCMSVerifyHCryptCommand			,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_MATCH_OFF				,4000	,'0'		,{"PPR_MatchOff"				,0x80	,0x01	,0x01	,0x01	,0x40	,0xFA	,inBuildResetOffCommand 				,inBuildResetData					,inBuildResetData					,inBuildNullTM					,inBuildNullTM				,inBuildResetOffCommand 			,inBuildResetData				,inBuildNullTM					,inBuildNullTM				,inBuildResetOffCommand 			,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_CBIKE_READ				,12000	,'0'		,{"PPR_CBikeRead"				,0x80	,0x0E	,0x01	,0x00	,0x10	,0xE0	,inBuildCBikeReadCommand				,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildCBikeReadCommand			,inBuildNullTM					,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildCBikeReadCommand 			,inBuildCMSCBikeReadTM			,inBuildCMASLockTM1 		,inBuildCMASLockTM2 	,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_CBIKE_DEDUCT			,4000	,'1'		,{"PPR_CBikeDeduct"				,0x80	,0x0E	,0x02	,0x00	,0x60	,0x7A	,inBuildCMASCBikeDeductCommand		,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildCMASCBikeDeductCommand		,inBuildCBikeDeductSIS2			,inBuildNullTM					,inBuildNullTM				,inBuildCMASCBikeDeductCommand		,inBuildCMSCBikeDeductTM 		,inBuildCMASLockTM1 		,inBuildCMASLockTM2 	,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_EDCA_READ				,12000	,'0'		,{"PPR_EDCARead"				,0x80	,0x01	,0x01	,0x00	,0x10	,0xA0	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildEDCAReadCommand 			,inBuildCMASEDCAReadTM			,inBuildCMASLockTM1 		,inBuildCMASLockTM2 	,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_EDCA_DEDUCT			,4000	,'1'		,{"PPR_EDCADeduct"				,0x80	,0x01	,0x02	,0x00	,0x40	,0x7A	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildEDCADeductSIS2			,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildCMASEDCADeductCommand		,inBuildCMASEDCADeductTM 		,inBuildCMASLockTM1 		,inBuildCMASLockTM2 	,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_TLRT_AUTH				,12000	,'0'		,{"PPR_TLRTAuth"				,0x80	,0x0C	,0x00	,0x04	,0x00	,0x01	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU		 			,inBuildNullTM					,inBuildNullTM		 		,inBuildNullTM		 	,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_ADD_VALUE1				,12000	,'0'		,{"PPR_TxnReqOnline1"			,0x80	,0x11	,0x01	,0x00	,0x40	,0xFD	,inBuildTxnReqOnlineCommand 			,inBuildTxnReqOnlineData1			,inBuildTxnReqOnlineData4			,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOnlineCommand 		,inBuildTxnReqOnlineData2		,inBuildLock1SIS2				,inBuildLock2SIS2			,inBuildTxnReqOnlineCommand 		,inBuildCMSTxnReqOnlineTM		,inBuildCMASLockTM1 		,inBuildCMASLockTM2 	,inBuildTxnReqOnlineCommand 		,inBuildICERTxnReqOnlineTM		,inBuildNullTM				,inBuildNullTM		}},
	{DLL_ADD_VALUE_AUTH1 		,4000	,'1'		,{"PPR_AuthTxnOnline1"			,0x80	,0x11	,0x02	,0x00	,0x28	,0x40	,inBuildAuthTxnOnlineCommand1		,inBuildAuthTxnOnlineData1		,inBuildAuthTxnOnlineData1		,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildAuthTxnOnlineSIS2		,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildCMSAuthTxnOnlineTM		,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_REFUND_CARD_AUTH 		,4000	,'1'		,{"PPR_AuthTxnOnline"			,0x80	,0x11	,0x02	,0x00	,0x28	,0x40	,inBuildAuthTxnOnlineCommand1		,inBuildAuthTxnOnlineData			,inBuildAuthTxnOnlineData			,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildAuthRefundCardSIS2		,inBuildNullTM					,inBuildNullTM				,inBuildCMSAuthTxnOnlineAPDU		,inBuildCMSAuthTxnOnlineTM		,inBuildNullTM				,inBuildNullTM			,inBuildICERAuthTxnOnlineAPDU		,inBuildICERAuthTxnOnlineTM 	,inBuildNullTM				,inBuildNullTM		}},
	{DLL_DEDUCT_VALUE1			,12000	,'0' 		,{"PPR_TxnReqOffline1"			,0x80	,0x32	,0x01	,0x00	,0x40	,0xFA	,inBuildTxnReqOfflineCommand			,inBuildTxnReqOfflineData1		,inBuildTxnReqOfflineData4		,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOfflineCommand		,inBuildTxnReqOfflineData2		,inBuildNullTM					,inBuildNullTM				,inBuildTxnReqOfflineCommand		,inBuildCMSTxnReqOfflineTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DDL_DEDUCT_VALUE2			,12000	,'0' 		,{"PPR_TxnReqOffline2"			,0x80	,0x32	,0x01	,0x00	,0x60	,0xFA	,inBuildTxnReqOfflineCommand2		,inBuildTxnReqOfflineData1		,inBuildTxnReqOfflineData2		,inBuildLockCardData8			,inBuildLockCardData9		,inBuildTxnReqOfflineCommand2		,inBuildTxnReqOfflineData2		,inBuildNullTM					,inBuildNullTM				,inBuildTxnReqOfflineCommand2		,inBuildCMSTxnReqOfflineTM		,inBuildCMASLockTM1			,inBuildCMASLockTM2		,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_GET_CMASMODE			,4000	,'0' 		,{"GET_CMASMODE"				,0x00	,0x00	,0x00	,0x00	,0x00	,0x00	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
	{DLL_CLOSECOM		 		,4000	,'1' 		,{"CloseCom"					,0x00	,0x00	,0x00	,0x00	,0x00	,0x00	,inBuildNullAPDU						,inBuildNullTM						,inBuildNullTM						,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM					,inBuildNullTM				,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM			,inBuildNullAPDU					,inBuildNullTM					,inBuildNullTM				,inBuildNullTM		}},
};

#if READER_MANUFACTURERS==ANDROID_API
//#include<icerapi_icerapi_Api.h>
//#include <android/log.h>
extern const char *JNI_filePath;
extern const char *JNI_LogPath;
extern JNIEnv *Jenv;
extern jobject Jni_usbManager,Jni_context;
extern jobject ReaderObj;
extern jmethodID AndroidReaderInit;
  #ifdef LIB_DEFINE
	#ifdef LIB_DEFINE2
		jmethodID AndroidUart0infoProcess2;
		jmethodID AndroidEccRunIdle;
	#endif
  #endif
#endif

BYTE checksum(int length,BYTE data[])
{
	unsigned char chk= 0x00;
	int i= 0x00;

	for(i=0;i<length;i++)
		chk^=data[i];
	return chk;
}

int inLoadLibrary()
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
char chAllFileName[200];

	memset(chAllFileName,0x00,sizeof(chAllFileName));
	sprintf(chAllFileName,"%s\\DongleAPI.dll",gCurrentFolder);

	if(srTxnData.srParameter.gReaderMode != DLL_READER)
		return SUCCESS;

	#if READER_MANUFACTURERS==WINDOWS_CE_API
		hDLL = LoadLibrary((WCHAR *)chAllFileName);
	#else
		hDLL = LoadLibrary(chAllFileName);
	#endif

	if(hDLL == NULL)
	{
		log_msg(LOG_LEVEL_ERROR,"inLoadLibrary LOAD_DLL_FAIL (%s)",chAllFileName);
		return LOAD_DLL_FAIL;
	}

	srTxnData.srIngData.fAPILoadFlag = TRUE;

/*#elif READER_MANUFACTURERS==ANDROID_API

	jclass clazz3 = (*Jenv) -> FindClass(Jenv,"icerapi/icerapi/ReaderAPI_Interface");
	AndroidReaderInit = (*Jenv) -> GetMethodID(Jenv,clazz3,"<init>","()V");
	AndroidUart0infoProcess2 = (*Jenv) -> GetMethodID(Jenv,clazz3,"Ts_Uart0infoProcess2","(Landroid/content/Context;Ljava/lang/String;Landroid/hardware/usb/UsbManager;Landroid/content/Context;[B[B[B[I[B[B)V");
    ReaderObj = (*Jenv) -> NewObject(Jenv,clazz3,AndroidReaderInit);*/

#endif
	return SUCCESS;
}

void vdFreeLibrary()
{
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	if(srTxnData.srParameter.gReaderMode != DLL_READER)
		return;

	FreeLibrary(hDLL);
#endif
}

int CheckDirectoryPermission(void)
{
	
	if(inFileGetSize((char *)FILE_PERMISSION,FALSE) > SUCCESS)
	{
		if (inFileDelete((char *)FILE_PERMISSION,FALSE) != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR, "CheckDirectoryPermission 01");
			return -1;
		}
	}

	if (inFileWrite((char *)FILE_PERMISSION,(BYTE *)"\x30", 1) == ICER_ERROR)
	{
		log_msg(LOG_LEVEL_ERROR, "CheckDirectoryPermission 02");
		return -1;	
	}

	if (inFileDelete((char *)FILE_PERMISSION,FALSE) != SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR, "CheckDirectoryPermission 03");
		return -1;
	}

	return SUCCESS;
}


int inReaderCommand(int inDLLType,STRUCT_XML_DOC *srXML,BYTE *bInData,BYTE *bOutData,BOOL bBatchFlag,int inLC,BOOL bAPILock)
{
int inRetVal = SUCCESS;
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
DWORD i = 0;
pFuncType1 pReqFunc1;
pFuncType2 pReqFunc2,pResFunc2;
pFuncType3 pReqFunc3,pResFunc3;
pFuncType4 pReqFunc4,pResFunc4;
pFuncType5 pReqFunc5;
pFuncType6 pReqFunc6;
int inMsgRet = 0;
MSG sMsg;
unsigned nThreadID;
int pInfo = 0x00;
char fRecvMsg;
clock_t lnStartTime, lnEndTime;
char fFuncSucc;
BYTE bReaderCommReq[50],bReaderCommRes[50];
clock_t tTimeout = srTxnData.srParameter.gTCPIPTimeOut * 1000;
//int inInFuncType,inOutFuncType,inTM_In_Len;
STRUCT_READER_INFO srReaderInfo;

	if(srTxnData.srParameter.gReaderMode == RS232_READER || srTxnData.srParameter.gReaderMode == LIBARY_READER || srTxnData.srParameter.gReaderMode == LIBARY_READER2)
		return(inReaderCommand2(inDLLType,srXML,bInData,bOutData,bBatchFlag,inLC,bAPILock));

	fFuncSucc = 0;
	memset(bReaderCommReq,0x00,sizeof(bReaderCommReq));
	memset(bReaderCommRes,0x00,sizeof(bReaderCommRes));
	if(srTxnData.srParameter.gDLLVersion == 1)
	{
		memcpy(&srReaderInfo,&st_ReaderCommand[inDLLType].srReaderInfoL1,sizeof(srReaderInfo));
		strcat((char *)bReaderCommReq,(const char *)st_ReaderCommand[inDLLType].srReaderInfoL1.bReaderCommand);
		strcat((char *)bReaderCommRes,(const char *)st_ReaderCommand[inDLLType].srReaderInfoL1.bReaderCommand);
		/*inInFuncType = st_ReaderCommand[inDLLType].srReaderInfoL1.inInFuncType;
		inOutFuncType = st_ReaderCommand[inDLLType].srReaderInfoL1.inOutFuncType;
		inTM_In_Len = st_ReaderCommand[inDLLType].srReaderInfoL1.inTM_In;*/

	}
	else
	{
		memcpy(&srReaderInfo,&st_ReaderCommand[inDLLType].srReaderInfoL2,sizeof(srReaderInfo));
		strcat((char *)bReaderCommReq,(const char *)st_ReaderCommand[inDLLType].srReaderInfoL2.bReaderCommand);
		strcat((char *)bReaderCommRes,(const char *)st_ReaderCommand[inDLLType].srReaderInfoL2.bReaderCommand);
		/*inInFuncType = st_ReaderCommand[inDLLType].srReaderInfoL2.inInFuncType;
		inOutFuncType = st_ReaderCommand[inDLLType].srReaderInfoL2.inOutFuncType;
		inTM_In_Len = st_ReaderCommand[inDLLType].srReaderInfoL2.inTM_In;*/
	}

	if(inDLLType == DLL_OPENCOM)
	{
		if(srTxnData.srParameter.chOpenComMode[0] == '3')
		{
			strcat((char *)bReaderCommReq,srTxnData.srParameter.chOpenComMode);
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pReqFunc6 = (pFuncType6)GetProcAddress(hDLL,(WCHAR *)bReaderCommReq);
			#else
				pReqFunc6 = (pFuncType6)GetProcAddress(hDLL,(const char *)bReaderCommReq);
			#endif
			if(pReqFunc6 == NULL)
			{
				log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 1-1:%s",bReaderCommReq);
				return LOAD_DLL_FUNCTION_ERROR;
			}
			nThreadID = GetCurrentThreadId();
			inRetVal = pReqFunc6(srTxnData.srParameter.gComPort,nThreadID,srTxnData.srParameter.ulBaudRate);
		}
		else
		{
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pReqFunc1 = (pFuncType1)GetProcAddress(hDLL,(WCHAR *)bReaderCommReq);
			#else
				pReqFunc1 = (pFuncType1)GetProcAddress(hDLL,(const char *)bReaderCommReq);
			#endif
			if(pReqFunc1 == NULL)
			{
				log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 1-2:%s",bReaderCommReq);
				return LOAD_DLL_FUNCTION_ERROR;
			}
			nThreadID = GetCurrentThreadId();
			inRetVal = pReqFunc1(srTxnData.srParameter.gComPort,nThreadID);
		}
		Sleep(50);
		if(inRetVal == SUCCESS)
			srTxnData.srIngData.fAPIOpenFlag = TRUE;
		else
			log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 2:%d",inRetVal);
		//FreeLibrary(hDLL);
		return inRetVal;
	}
	else if(inDLLType == DLL_CLOSECOM)
	{
		#if READER_MANUFACTURERS==WINDOWS_CE_API
			pReqFunc5 = (pFuncType5)GetProcAddress(hDLL,(WCHAR *)bReaderCommReq);
		#else
			pReqFunc5 = (pFuncType5)GetProcAddress(hDLL,(const char *)bReaderCommReq);
		#endif
		if(pReqFunc5 == NULL)
		{
			//FreeLibrary(hDLL);
			log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 3:%s,%d",bReaderCommReq,inDLLType);
			return LOAD_DLL_FUNCTION_ERROR;
		}
		inRetVal = pReqFunc5();
		if(inRetVal != SUCCESS)
			log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 4:%d",inRetVal);
		//FreeLibrary(hDLL);
		return inRetVal;
	}
	else if(inDLLType == DLL_ANTENN_CONTROL)
	{
		#if READER_MANUFACTURERS==WINDOWS_CE_API
		pReqFunc4 = (pFuncType4)GetProcAddress(hDLL,(WCHAR *)bReaderCommReq);
		#else
		pReqFunc4 = (pFuncType4)GetProcAddress(hDLL,(const char *)bReaderCommReq);
		#endif
		if(pReqFunc4 == NULL)
		{
			log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 4-1:%s",bReaderCommReq);
			return LOAD_DLL_FUNCTION_ERROR;
		}
		vdWriteLog(bInData,srReaderInfo.inTMInLen,WRITE_DLL_REQ_LOG,inDLLType);
		inRetVal = pReqFunc4(bInData);
		if(inRetVal != SUCCESS)
			log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 4-2:%d",inRetVal);
		return inRetVal;
	}

	vdWriteLog(bInData,srReaderInfo.inTMInLen,WRITE_DLL_REQ_LOG,inDLLType);

	strcat((char *)bReaderCommReq,"Request");
	strcat((char *)bReaderCommRes,"Response");

	if(srTxnData.srParameter.gDLLMode == DLL_FILE_MODE)
	{
		if(inFileGetSize("C:\\EasyCard\\EasyCard_Succ.log",TRUE) >= 0)
			inFileDelete("C:\\EasyCard\\EasyCard_Succ.log",TRUE);
		if(inFileGetSize("C:\\EasyCard\\EasyCard_Fail.log",TRUE) >= 0)
			inFileDelete("C:\\EasyCard\\EasyCard_Fail.log",TRUE);
	}

	switch(srReaderInfo.inInFuncType)
	{
//		case 1:
//			pFunc1 = (pFuncType1)GetProcAddress(hDLL,(const char *)bReaderCommReq);
//			if(pFunc1 == NULL)
//				break;
//			fFuncSucc = 1;
//			inRetVal = pFunc1(2,nThreadID);
//			break;
		case 2:
			PeekMessage(&sMsg, NULL, 0, 0, PM_REMOVE);
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pReqFunc2 = (pFuncType2)GetProcAddress(hDLL,(WCHAR *)bReaderCommReq);
			#else
				pReqFunc2 = (pFuncType2)GetProcAddress(hDLL,(const char *)bReaderCommReq);
			#endif
			if(pReqFunc2 == NULL)
				break;
			fFuncSucc = 1;
			pReqFunc2();
			break;
		case 3:
			PeekMessage(&sMsg, NULL, 0, 0, PM_REMOVE);
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pReqFunc3 = (pFuncType3)GetProcAddress(hDLL,(WCHAR *)bReaderCommReq);
			#else
				pReqFunc3 = (pFuncType3)GetProcAddress(hDLL,(const char *)bReaderCommReq);
			#endif
			if(pReqFunc3 == NULL)
				break;
			fFuncSucc = 1;
			pReqFunc3(bInData);
			break;
		case 4:
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pReqFunc4 = (pFuncType4)GetProcAddress(hDLL,(WCHAR *)bReaderCommReq);
			#else
				pReqFunc4 = (pFuncType4)GetProcAddress(hDLL,(const char *)bReaderCommReq);
			#endif
			if(pReqFunc4 == NULL)
				break;
			fFuncSucc = 1;
			inRetVal = pReqFunc4(bOutData);
			break;
		case 5:
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pReqFunc5 = (pFuncType5)GetProcAddress(hDLL,(WCHAR *)bReaderCommReq);
			#else
				pReqFunc5 = (pFuncType5)GetProcAddress(hDLL,(const char *)bReaderCommReq);
			#endif
			if(pReqFunc5 == NULL)
				break;
			fFuncSucc = 1;
			inRetVal = pReqFunc5();
			break;
	}

	if(fFuncSucc == 0)
	{
		//FreeLibrary(hDLL);
		log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 5:%s (%d)",bReaderCommReq,srReaderInfo.inInFuncType);
		return LOAD_DLL_FUNCTION_ERROR;
	}

	pInfo = fRecvMsg = 0;
	#if READER_MANUFACTURERS==WINDOWS_CE_API
			lnStartTime = lnEndTime = GetTickCount();
	#else
			lnStartTime = lnEndTime = clock();
	#endif

	if(srTxnData.srParameter.gDLLMode == DLL_FILE_MODE)
	{
		do
		{
			inRetVal = inFileGetSize("C:\\EasyCard\\EasyCard_Fail.log",TRUE);
			if(inRetVal < 0)
				inRetVal = inFileGetSize("C:\\EasyCard\\EasyCard_Succ.log",TRUE);
			if(inRetVal >=0)
			{
				fRecvMsg = 1;
				break;
			}
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				lnEndTime = GetTickCount();
			#else
				lnEndTime = clock();
			#endif
		}while((lnEndTime - lnStartTime) <= tTimeout);
	}
	else
	{
		do
		{
			//inMsgRet = GetMessage(&sMsg,NULL,0,0);
			inMsgRet = PeekMessage(&sMsg, NULL, 0, 0, PM_REMOVE);
			if(inMsgRet != 0) //get msg from message queue
			{
				switch(sMsg.message)
				{
					case WM_DONGLE_GETDATA:
						pInfo = sMsg.wParam;
						fRecvMsg = 1;
						break;
					case WM_DONGLE_ERROR:
						pInfo = sMsg.wParam;
						fRecvMsg = 1;
						break;
				}
			}
			if(fRecvMsg == 1)
				break;
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				lnEndTime = GetTickCount();
			#else
				lnEndTime = clock();
			#endif
		}while((lnEndTime - lnStartTime) <= tTimeout);
	}

	if(fRecvMsg == 0)
	{
		//FreeLibrary(hDLL);
		log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 6(%d)",srTxnData.srParameter.gDLLMode);
		return CALL_DLL_TIMEOUT_ERROR;
	}

	fFuncSucc = 0;
	switch(srReaderInfo.inOutFuncType)
	{
		case 2:
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pResFunc2 = (pFuncType2)GetProcAddress(hDLL,(WCHAR *)bReaderCommRes);
			#else
				pResFunc2 = (pFuncType2)GetProcAddress(hDLL,(const char *)bReaderCommRes);
			#endif
			if(pResFunc2 == NULL)
				break;
			fFuncSucc = 1;
			pResFunc2();
			break;
		case 3:
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pResFunc3 = (pFuncType3)GetProcAddress(hDLL,(WCHAR *)bReaderCommRes);
			#else
				pResFunc3 = (pFuncType3)GetProcAddress(hDLL,(const char *)bReaderCommRes);
			#endif
			if(pResFunc3 == NULL)
				break;
			fFuncSucc = 1;
			pResFunc3(bInData);
			break;
		case 4:
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pResFunc4 = (pFuncType4)GetProcAddress(hDLL,(WCHAR *)bReaderCommRes);
			#else
				pResFunc4 = (pFuncType4)GetProcAddress(hDLL,(const char *)bReaderCommRes);
			#endif
			if(pResFunc4 == NULL)
				break;
			fFuncSucc = 1;
			inRetVal = pResFunc4(bOutData);
			break;
		case 5:
			#if READER_MANUFACTURERS==WINDOWS_CE_API
				pReqFunc5 = (pFuncType5)GetProcAddress(hDLL,(WCHAR *)bReaderCommRes);
			#else
				pReqFunc5 = (pFuncType5)GetProcAddress(hDLL,(const char *)bReaderCommRes);
			#endif
			if(pReqFunc5 == NULL)
				break;
			fFuncSucc = 1;
			inRetVal = pReqFunc5();
			break;
	}

//	FreeLibrary(hDLL);

	if(fFuncSucc == 0)
	{
		log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 7");
		return LOAD_DLL_FUNCTION_ERROR;
	}

	srTxnData.srIngData.inReaderSW = inRetVal;
	vdWriteLog(bOutData,inGetTMOutLen(inRetVal,&srReaderInfo,inDLLType),WRITE_DLL_RES_LOG,inDLLType);
	if(inRetVal == CARD_TIMEOUT || inRetVal == -18 || inRetVal == -19 || inRetVal == -22 || pInfo == -18 || pInfo == -19 || pInfo == -22)
	//if(inRetVal == CARD_TIMEOUT || pInfo == -18 || pInfo == -19 || pInfo == -22)
	{
		log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 8,inRetVal = %d,pInfo = %d",inRetVal,pInfo);
		return CALL_DLL_TIMEOUT_ERROR;
	}
	else if(inRetVal == CARD_SUCCESS && (!memcmp((char *)bReaderCommRes,"PPR_Auth",8) || !memcmp((char *)bReaderCommRes,"PR_Auth",7)))
		srTxnData.srIngData.fCardActionOKFlag = TRUE;
#elif READER_MANUFACTURERS==LINUX_API// || READER_MANUFACTURERS==ANDROID_API
char DevicePort[20+1]="/dev/ttyS";//"USB"
char inputfile[30]="inputdata";
char outputfile[30]="outputdata";
char fRecvMsg;
BYTE bOutFile[1100];
time_t lnStartTime, lnEndTime;
int inTMOutLen;
UnionUShort usSW;

	//log_msg(LOG_LEVEL_FLOW,"inReaderCommand 0(%d)",srTxnData.srParameter.gReaderMode);
	//if(srTxnData.srParameter.gReaderMode == RS232_READER || srTxnData.srParameter.gReaderMode == LIBARY_READER || srTxnData.srParameter.gReaderMode == LIBARY_READER2)
	if(srTxnData.srParameter.gReaderMode != DLL_READER)
		return(inReaderCommand2(inDLLType,srXML,bInData,bOutData,bBatchFlag,inLC,bAPILock));

	if(inDLLType == DLL_OPENCOM || inDLLType == DLL_CLOSECOM)
		return inRetVal;

	if(st_ReaderCommand[inDLLType].inTMInLen > 0)
	{
		vdWriteLog(bInData,st_ReaderCommand[inDLLType].inTMInLen,WRITE_DLL_REQ_LOG,inDLLType);
		inFileWrite(inputfile,bInData,st_ReaderCommand[inDLLType].inTMInLen);
	}

	inFileDelete(outputfile,FALSE);
	DevicePort[9] = srTxnData.srParameter.gComPort - 1 + 0x30;
	if(vfork()==0)
	{
		if(st_ReaderCommand[inDLLType].inFuncType == 2)
			execlp("./dongle","./dongle","-d",DevicePort ,"-t",st_ReaderCommand[inDLLType].bReaderCommand,"-o",outputfile,(char *)0);
		else if(st_ReaderCommand[inDLLType].inFuncType == 3)
			execlp("./dongle","./dongle","-d",DevicePort ,"-t",st_ReaderCommand[inDLLType].bReaderCommand,"-i",inputfile,"-o",outputfile,(char *)0);
	}

	memset(bOutFile,0x00,sizeof(bOutFile));
	fRecvMsg = 0;
	lnStartTime = time((time_t*)0);
	lnEndTime = lnStartTime;

	do
	{
		inRetVal = inFileGetSize(outputfile,FALSE);
		if(inRetVal >= 4)
		{
			fRecvMsg = 1;
			break;
		}
		lnEndTime = time((time_t*)0);
	}while((lnEndTime - lnStartTime) <= srTxnData.srParameter.gTCPIPTimeOut);

	if(fRecvMsg == 0)
	{
		log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 1");
		return CALL_DLL_TIMEOUT_ERROR;
	}

	inFileRead(outputfile,bOutFile,sizeof(bOutFile),4);
	memset(usSW.Buf,0x00,sizeof(usSW.Buf));
	fnPack((char *)&bOutFile[0],2,&usSW.Buf[1]);
	fnPack((char *)&bOutFile[2],2,&usSW.Buf[0]);
	srTxnData.srIngData.inReaderSW = usSW.Value;
	inRetVal = srTxnData.srIngData.inReaderSW;
	inTMOutLen = inGetTMOutLen(srTxnData.srIngData.inReaderSW,&st_ReaderCommand[0],inDLLType);

	if(inTMOutLen > 0)
	{
		fRecvMsg = 0;
		lnStartTime = lnEndTime = time((time_t*)0);
		do
		{
			inRetVal = inFileGetSize(outputfile,FALSE);
			if(inRetVal >= inTMOutLen + 4)
			{
				fRecvMsg = 1;
				break;
			}
			lnEndTime = time((time_t*)0);
		}while((lnEndTime - lnStartTime) <= srTxnData.srParameter.gTCPIPTimeOut);

		if(fRecvMsg == 0)
		{
			log_msg(LOG_LEVEL_ERROR,"inReaderCommand Fail 2");
			return CALL_DLL_TIMEOUT_ERROR;
		}
		inFileRead2(outputfile,&bOutFile[4],inTMOutLen - 4,inTMOutLen,4);
	}

	memcpy(bOutData,&bOutFile[4],inTMOutLen);
	vdWriteLog(&bOutFile[4],inTMOutLen,WRITE_DLL_RES_LOG,inDLLType);
	inRetVal = srTxnData.srIngData.inReaderSW;
	if(inRetVal == CARD_SUCCESS && strstr((char *)st_ReaderCommand[inDLLType].bReaderCommand,"Auth"))
		srTxnData.srIngData.fCardActionOKFlag = TRUE;
	else if(inRetVal == CARD_TIMEOUT || inRetVal == 0xFFEE || inRetVal == 0xFFED || inRetVal == 0xFFEA)
	//else if(inRetVal == CARD_TIMEOUT || inRetVal == -18 || inRetVal == -19 || inRetVal == -22)
		return CALL_DLL_TIMEOUT_ERROR;
#else

	//log_msg(LOG_LEVEL_FLOW,"inReaderCommand 0:%d",inRetVal);
	if(srTxnData.srParameter.gReaderMode == RS232_READER || srTxnData.srParameter.gReaderMode == LIBARY_READER || srTxnData.srParameter.gReaderMode == LIBARY_READER2
	    || srTxnData.srParameter.gReaderMode == USB_READER || srTxnData.srParameter.gReaderMode == USB_TO_RS232_READER)
	{
		//log_msg(LOG_LEVEL_FLOW,"inReaderCommand 1:%d",inRetVal);
		return(inReaderCommand2(inDLLType,srXML,bInData,bOutData,bBatchFlag,inLC,bAPILock));
	}

#endif
	//log_msg(LOG_LEVEL_FLOW,"inReaderCommand 2:%d",inRetVal);

	return inRetVal;
}

int inReaderCommand2(int inDLLType,STRUCT_XML_DOC *srXML,BYTE *bInData,BYTE *bOutData,BOOL bBatchFlag,int inLC,BOOL bAPILock)
{
	int inRetVal = 0;
	//STRUCT_READER_INFO srReaderInfo;
	BYTE bSendData[READER_BUFFER],bRecvData[READER_BUFFER];
	int inCnt = 3;
	pAPDUFunction pAPDU_Fun;
	pTMFunction pTMFuncOK1,pTMFuncOK2,pTMFuncLock1,pTMFuncLock2;
	BOOL bCMASFuncFlag = FALSE,bR6FuncFlag = FALSE,bICERFuncFlag = FALSE,bCommandProcess = TRUE,bTransXMLFlag = FALSE,bTransFlag = FALSE;

	//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 0-1:%d",inRetVal);
	if((srTxnData.srParameter.gReaderMode == LIBARY_READER || srTxnData.srParameter.gReaderMode == LIBARY_READER2) && (inDLLType == DLL_OPENCOM || inDLLType == DLL_CLOSECOM))
		return SUCCESS;

	if(inDLLType == DLL_OPENCOM)
		return inComOpen();
	else if(inDLLType == DLL_CLOSECOM)
		return inComClose();

	//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 0-2:%d",inRetVal);
	//if(srTxnData.srParameter.gDLLVersion == 1)
	//	memcpy(&srReaderInfo,&st_ReaderCommand2[inDLLType].srReaderInfoL1,sizeof(srReaderInfo));
	//else
	//	memcpy(&srReaderInfo,&st_ReaderCommand2[inDLLType].srReaderInfoL2,sizeof(srReaderInfo));

	memset(bSendData,0x00,sizeof(bSendData));
	memset(bRecvData,0x00,sizeof(bRecvData));

	bSendData[inCnt++] = st_ReaderCommand2[inDLLType].srReaderInfoL2.CLA;
	bSendData[inCnt++] = st_ReaderCommand2[inDLLType].srReaderInfoL2.INS;
	bSendData[inCnt++] = st_ReaderCommand2[inDLLType].srReaderInfoL2.P1;
	if(inDLLType == DLL_ANTENN_CONTROL)
		bSendData[inCnt++] = bInData[0];
	else
		bSendData[inCnt++] = st_ReaderCommand2[inDLLType].srReaderInfoL2.P2;
	if(st_ReaderCommand2[inDLLType].srReaderInfoL2.LC > 0 || ((inDLLType == DLL_VERIFY_HOST_CRYPT || inDLLType == DLL_QR_VERIFY_HOST_CRYPT) && inLC > 0))
	{
		if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
			pAPDU_Fun = st_ReaderCommand2[inDLLType].srReaderInfoL2.pR6_APDUFun;
		else if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
			pAPDU_Fun = st_ReaderCommand2[inDLLType].srReaderInfoL2.pCMAS_APDUFun;
		else if(srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)
			pAPDU_Fun = st_ReaderCommand2[inDLLType].srReaderInfoL2.pICER_APDUFun;
		else //if(srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)保留,尚未開發
		{
			log_msg(LOG_LEVEL_ERROR,"inReaderCommand2 Fail 0:(%c)(%c)",srTxnData.srParameter.chOnlineFlag,srTxnData.srParameter.chBatchFlag);
			return ICER_ERROR;
		}

		bSendData[inCnt++] = st_ReaderCommand2[inDLLType].srReaderInfoL2.LC & 0xFF;
		inRetVal = pAPDU_Fun(srXML,&bSendData[inCnt],bInData);
		if((inDLLType == DLL_VERIFY_HOST_CRYPT || inDLLType == DLL_QR_VERIFY_HOST_CRYPT) && inLC > 0)
		{
			if(inRetVal == ICER_ERROR)
			{
				log_msg(LOG_LEVEL_ERROR,"inReaderCommand2 Fail 1-0:(%d)",inRetVal);
				return DLL_DECLINE_FAIL;
			}
			bSendData[inCnt-1] = inRetVal;
			inCnt += inRetVal;
		}
		else
			inCnt += st_ReaderCommand2[inDLLType].srReaderInfoL2.LC;
	}

	if(st_ReaderCommand2[inDLLType].srReaderInfoL2.LE > 0)
	{	//PPR_ReadBasicData & 舊版指令
		if(/*srTxnData.srIngData.inTransType == TXN_ECC_READ_BASIC && */
			inDLLType == DLL_READ_CARD_BASIC_DATA &&
			srTxnData.srParameter.chReadBasicVersion == '1')
			bSendData[inCnt++] = 0x88;
		else
			bSendData[inCnt++] = st_ReaderCommand2[inDLLType].srReaderInfoL2.LE & 0xFF;
	}

	bSendData[2] = inCnt - 3;//LEN
	bSendData[inCnt] = checksum(inCnt,&bSendData[0]);//EDC
	inCnt++;

	log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 0:(%d)(%s)(%s)",inDLLType,chGetDLLName(inDLLType),st_ReaderCommand2[inDLLType].srReaderInfoL2.bReaderCommand);

	if(srTxnData.srParameter.chCommandMode == '1' &&
	  ((srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_TAXI && inDLLType == DLL_TAXI_READ) ||
	   (srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_CBIKE && inDLLType == DLL_CBIKE_READ) ||
	   (srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_EDCA && inDLLType == DLL_EDCA_READ)))
	{
		BYTE bCommand[READER_BUFFER + READER_BUFFER];

		memset(bCommand,0x00,sizeof(bCommand));
		if(inFileGetSize((char *)FILE_COMMAND1_BAK,FALSE) == sizeof(bCommand))
		{
			inFileRead((char *)FILE_COMMAND1_BAK,bCommand,sizeof(bCommand),sizeof(bCommand));
			memcpy(bSendData,bCommand,READER_BUFFER);
			memcpy(bRecvData,&bCommand[READER_BUFFER],READER_BUFFER);
			inRetVal = (bRecvData[bRecvData[2] + 1] << 8) | (bRecvData[bRecvData[2] + 2]);
			bCommandProcess = FALSE;
		}
		else
			log_msg(LOG_LEVEL_ERROR,"inReaderCommand2 1-0-1 Fail!!");
	}

	if(bCommandProcess == TRUE)
	{
		vdWriteLog(bSendData,inCnt,WRITE_DLL_REQ_LOG,inDLLType);
		//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 0:%d",inRetVal);

		if(st_ReaderCommand2[inDLLType].ulTimeOut > 0)
		{
		#ifdef LIB_DEFINE

		  #ifdef LIB_DEFINE2

			if(srTxnData.srParameter.gReaderMode == LIBARY_READER2)
			{
				int inRet = 0;

				//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 1:%d",inRetVal);
				memset((char *)&g_sUart0RecieveInfo2,0x00,sizeof(g_sUart0RecieveInfo2));
				memset((char *)&g_sUart0AnswerInfo2,0x00,sizeof(g_sUart0AnswerInfo2));

				memcpy(g_sUart0RecieveInfo2.cProLog,bSendData,sizeof(g_sUart0RecieveInfo2.cProLog));
				memcpy(g_sUart0RecieveInfo2.cInfoHeader,&bSendData[3],sizeof(g_sUart0RecieveInfo2.cInfoHeader));
				memcpy(g_sUart0RecieveInfo2.cInfoBody,&bSendData[3 + 4],inCnt - 4);
				g_sUart0RecieveInfo2.cEDC = bSendData[inCnt - 1];
				iUart0RecieveOK = chSerialPortMode = chProtectFlag = 0;
				log_msg(LOG_LEVEL_FLOW,"Ts_Uart0InfoProcess2 Start!!");

			#if READER_MANUFACTURERS==ANDROID_API

				jbyteArray jSendData=(*Jenv)->NewByteArray(Jenv, sizeof(g_sUart0RecieveInfo2));
				jbyteArray jRecvData=(*Jenv)->NewByteArray(Jenv, sizeof(g_sUart0AnswerInfo2));
				jbyteArray jAnswerLen=(*Jenv)->NewByteArray(Jenv, 1);
				jintArray jUart0RecieveOK=(*Jenv)->NewIntArray(Jenv, 1 * sizeof(jint));
				jbyteArray jSerialPortMode=(*Jenv)->NewByteArray(Jenv, 1);
				jbyteArray jProtectFlag=(*Jenv)->NewByteArray(Jenv, 1);

				(*Jenv)->SetByteArrayRegion(Jenv, jSendData, 0, sizeof(g_sUart0RecieveInfo2), (jbyte*)&g_sUart0RecieveInfo2);
				//jUart0RecieveOK[0] = 0;
				jstring str1 = (*Jenv)->NewStringUTF(Jenv,JNI_filePath);
				jstring str2 = (*Jenv)->NewStringUTF(Jenv,JNI_LogPath);
				(*Jenv) -> CallVoidMethod(Jenv,ReaderObj,AndroidUart0infoProcess2,str1,str2,Jni_usbManager,Jni_context,jSendData,jRecvData,jAnswerLen,jUart0RecieveOK,jSerialPortMode,jProtectFlag);
				if(jRecvData != NULL)
				{
					jsize len = (*Jenv) -> GetArrayLength(Jenv,jRecvData);
					jbyte* jbarray = (jbyte*) ECC_calloc(len,sizeof(jbyte));
					(*Jenv) -> GetByteArrayRegion(Jenv,jRecvData,0,len,jbarray);
					memcpy(&g_sUart0AnswerInfo2,jbarray,len);
					ECC_free(jbarray);
				}

			#else

				inRet = Ts_Uart0InfoProcess2(&g_sUart0RecieveInfo2,&g_sUart0AnswerInfo2,&chAnswerLen,&iUart0RecieveOK,&chSerialPortMode,&chProtectFlag);

			#endif

				log_msg(LOG_LEVEL_FLOW,"Ts_Uart0InfoProcess2 End!!");
				memcpy(bRecvData,g_sUart0AnswerInfo2.cProLog,sizeof(g_sUart0AnswerInfo2.cProLog));
				//if(Tscc_cAnswerLen > 0)
				if(g_sUart0AnswerInfo2.cProLog[2] >= 2)
				{
					memcpy(&bRecvData[3],g_sUart0AnswerInfo2.cInfoBody,g_sUart0AnswerInfo2.cProLog[2] - 2);
					memcpy(&bRecvData[3 + g_sUart0AnswerInfo2.cProLog[2] - 2],g_sUart0AnswerInfo2.cSW,2);
					bRecvData[3 + g_sUart0AnswerInfo2.cProLog[2]] = g_sUart0AnswerInfo2.cEDC;
				}
			}
			else

		  #else

			if(srTxnData.srParameter.gReaderMode == LIBARY_READER)
			{
				//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 2:%d",inRetVal);
				memset((char *)&g_sUart0RecieveInfo,0x00,sizeof(g_sUart0RecieveInfo));
				memset((char *)&g_sUart0AnswerInfo,0x00,sizeof(g_sUart0AnswerInfo));

				memcpy(g_sUart0RecieveInfo.cProLog,bSendData,sizeof(g_sUart0RecieveInfo.cProLog));
				memcpy(g_sUart0RecieveInfo.cInfoHeader,&bSendData[3],sizeof(g_sUart0RecieveInfo.cInfoHeader));
				memcpy(g_sUart0RecieveInfo.cInfoBody,&bSendData[3 + 4],inCnt - 4);
				g_sUart0RecieveInfo.cEDC = bSendData[inCnt - 1];
				g_cProtectFlag = 0;

				//log_msg(LOG_LEVEL_ERROR,"Ts_Uart0InfoProcess Start!!");
				Ts_Uart0InfoProcess(/*(BYTE *)&g_sUart0RecieveInfo,(BYTE *)&g_sUart0AnswerInfo*/);
				//log_msg(LOG_LEVEL_ERROR,"Ts_Uart0InfoProcess End!!");
				memcpy(bRecvData,g_sUart0AnswerInfo.cProLog,sizeof(g_sUart0AnswerInfo.cProLog));
				//if(Tscc_cAnswerLen > 0)
				if(g_sUart0AnswerInfo.cProLog[2] >= 2)
				{
					memcpy(&bRecvData[3],g_sUart0AnswerInfo.cInfoBody,g_sUart0AnswerInfo.cProLog[2] - 2);
					memcpy(&bRecvData[3 + g_sUart0AnswerInfo.cProLog[2] - 2],g_sUart0AnswerInfo.cSW,2);
					bRecvData[3 + g_sUart0AnswerInfo.cProLog[2]] = g_sUart0AnswerInfo.cEDC;
				}
			}
			else

		  #endif

		#endif
			{
				unsigned long ulTimeOut;
				//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 3:%d",inRetVal);

				if(srTxnData.srParameter.chReaderPortocol == READER_INFOCHAMP)
				{
					BYTE bTmp[500];

					memset(bTmp,0x00,sizeof(bTmp));

					bTmp[0] = 0xFA;
					bTmp[1] = (inCnt + 4) / 256;
					bTmp[2] = (inCnt + 4) % 256;
					memcpy(&bTmp[3],"\x02\x01\x00\x00",4);
					memcpy(&bTmp[7],bSendData,inCnt);
					inCnt += 7;
					bTmp[inCnt] = checksum(inCnt,&bTmp[0]);//EDC
					inCnt ++;

					memset(bSendData,0x00,sizeof(bSendData));
					memcpy(bSendData,bTmp,inCnt);

					log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"READER_INFOCHAMP",16,bSendData,inCnt);
				}
				else if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
				{
					BYTE bTmp[500];
					int inCnt1 = 0,inCnt2 = 0,i;

					memset(bTmp,0x00,sizeof(bTmp));

					memcpy(&bTmp[inCnt1],"\x10\x02\x08",3);
					inCnt1 += 9;
					bTmp[inCnt1++] = inCnt / 256;
					bTmp[inCnt1++] = inCnt % 256;
					//memcpy(&bTmp[inCnt1],bSendData,inCnt);
					//inCnt1 += inCnt;
					//DLE:遇到0x10時,要多補一個0x10
					for(i=0;i<inCnt;i++)
					{
						bTmp[inCnt1++] = bSendData[i];
						if(bSendData[i] == 0x10)
						{
							bTmp[inCnt1++] = 0x10;
							inCnt2++;
							//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 DLE 0x10:(%d)",i);
						}
					}
					bTmp[inCnt1++] = checksum(inCnt + 9 + inCnt2,&bTmp[2]);//EDC
					if(inCnt2 % 2)
						bTmp[inCnt1 - 1] = bTmp[inCnt1 - 1] ^ 0x10;
					if(bTmp[inCnt1 - 1] == 0x10)
					{
						bTmp[inCnt1++] = 0x10;
						inCnt2++;
					}
					memcpy(&bTmp[inCnt1],"\x10\x03",2);
					inCnt1 += 2;

					memset(bSendData,0x00,sizeof(bSendData));
					memcpy(bSendData,bTmp,inCnt1);
					inCnt = inCnt1;
				}

				inRetVal = inComSend(bSendData,inCnt);
				if(inRetVal != SUCCESS)
					return inRetVal;

				ulTimeOut = st_ReaderCommand2[inDLLType].ulTimeOut;
				if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
					ulTimeOut += 3000;

				//log_msg(LOG_LEVEL_ERROR,"ulTimeOut = %d,%d,%c",ulTimeOut,st_ReaderCommand2[inDLLType].ulTimeOut,srTxnData.srParameter.chReaderPortocol);
				inRetVal = inComRecv(bRecvData,st_ReaderCommand2[inDLLType].srReaderInfoL2.LE + 2 + 4,ulTimeOut);
				if(inRetVal != SUCCESS)
				{
					log_msg(LOG_LEVEL_ERROR,"inReaderCommand2 Fail 1-1:(%04x)",inRetVal);
					if(st_ReaderCommand2[inDLLType].chCardAccess == '1')
						return CALL_DLL_TIMEOUT_ERROR;
					else
						return PORT_READTIMEOUT_ERROR;
				}

				if(srTxnData.srParameter.chReaderPortocol == READER_INFOCHAMP)
				{
					BYTE bTmp[500];
					int inLen = 0;

					memset(bTmp,0x00,sizeof(bTmp));

					inLen = (bRecvData[1] * 256) + bRecvData[2];
					bTmp[2] = (inLen - 4);
					memcpy(&bTmp[3],&bRecvData[7],sizeof(bRecvData) - 7);

					memset(bRecvData,0x00,sizeof(bRecvData));
					memcpy(bRecvData,bTmp,sizeof(bRecvData));

					memset(bTmp,0x00,sizeof(bTmp));
					memcpy(bTmp,&bSendData[7],bSendData[10]);
					memset(bSendData,0x00,sizeof(bSendData));
					memcpy(bSendData,bTmp,sizeof(bTmp));
				}
				else if(srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
				{
					BYTE bTmp[READER_BUFFER];
					int inLen = 0;

					inLen = bRecvData[9] * 256 + bRecvData[10];;
					log_msg_debug(LOG_LEVEL_ERROR,TRUE,(char *)"DLE Header",10,bRecvData,inLen + 14);

					memset(bTmp,0x00,sizeof(bTmp));

					inLen = bRecvData[13];
					memcpy(bTmp,&bRecvData[11],inLen + 4);

					memset(bRecvData,0x00,sizeof(bRecvData));
					memcpy(bRecvData,bTmp,sizeof(bRecvData));

					inLen = bSendData[9] * 256 + bSendData[10];
					memcpy(bSendData,&bSendData[11],inLen);
				}
			}
			//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 4:%d",inRetVal);

			if(srTxnData.srParameter.chReaderPortocol == READER_INFOCHAMP || srTxnData.srParameter.chReaderPortocol == READER_CASTLES)
				inRetVal = SUCCESS;
			else
				inRetVal = inReponseDefaultCkeck(bRecvData);
			//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 5:%d",inRetVal);
			if(inRetVal != SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inReaderCommand2 Fail 1-2:(%04x)",inRetVal);
				return inRetVal;
			}

			//memcpy(bOutData,&bRecvData[3],bRecvData[2]);
			inRetVal = (bRecvData[bRecvData[2] + 1] << 8) | (bRecvData[bRecvData[2] + 2]);
			srTxnData.srIngData.inReaderSW = inRetVal;
			vdWriteLog(bRecvData,st_ReaderCommand2[inDLLType].srReaderInfoL2.LE + 2 + 4,WRITE_DLL_RES_LOG,inDLLType);
		}
		else
			inRetVal = 0x6415;
	}

	//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 6:%d",inRetVal);
	if(fIsCheckBLC(inDLLType,bRecvData) == TRUE)
	{
		int inRet1;
		BYTE ucCardID[10],ucTxnDateTime[5];
		memset(ucCardID,0x00,sizeof(ucCardID));
		memset(ucTxnDateTime,0x00,sizeof(ucTxnDateTime));
		if((inDLLType == DLL_DEDUCT_VALUE) || (inDLLType == DDL_DEDUCT_VALUE2))
		{
			memcpy(ucCardID,((TxnReqOffline_APDU_Out *)&bRecvData[OUT_DATA_OFFSET])->ucCardID,7);
			memcpy(ucTxnDateTime,((TxnReqOffline_APDU_In *)&bRecvData[IN_DATA_OFFSET])->ucTxnDateTime,4);
		}
		else if(inDLLType == DLL_TAXI_READ)
		{
			memcpy(ucCardID,((TaxiRead_APDU_Out *)&bRecvData[OUT_DATA_OFFSET])->ucCardID,7);
			memcpy(ucTxnDateTime,((TaxiRead_APDU_In *)&bRecvData[IN_DATA_OFFSET])->ucTxnDateTime,4);
		}
		else if(inDLLType == DLL_CBIKE_READ)
		{
			memcpy(ucCardID,((CBikeRead_APDU_Out *)&bRecvData[OUT_DATA_OFFSET])->ucCardID,7);
			memcpy(ucTxnDateTime,((CBikeRead_APDU_In *)&bRecvData[IN_DATA_OFFSET])->ucTxnDateTime,4);
		}
		else if(inDLLType == DLL_EDCA_READ)
		{
			memcpy(ucCardID,((EDCARead_APDU_Out *)&bRecvData[OUT_DATA_OFFSET])->ucCardID,7);
			memcpy(ucTxnDateTime,((EDCARead_APDU_In *)&bRecvData[IN_DATA_OFFSET])->ucTxnDateTime,4);
		}
		inRet1 = inCheckBLC(ucCardID);
		//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 7:%d",inRetVal);
		if(inRet1 == SUCCESS)
		{
			memset(bRecvData,0x00,sizeof(bRecvData));
			inRet1 = inBLCLockCard(DLL_LOCK_CARD,bRecvData,ucCardID,ucTxnDateTime);
			if(inRet1 == 0x6406)
				inRetVal = srTxnData.srIngData.inReaderSW = inRet1;
			else
				inRetVal = LOAD_DLL_FUNCTION_ERROR;
		}
		//else
		//	inRet1 = CARD_SUCCESS;
	}
	//log_msg(LOG_LEVEL_FLOW,"inReaderCommand2 8:%d",inRetVal);

	if(inDLLType == DLL_LOCK_CARD && bAPILock == TRUE && inRetVal == CARD_SUCCESS)
	{
		memcpy(((LockCard_APDU_Out_2 *)&bRecvData[OUT_DATA_OFFSET])->ucStatusCode,"\x64\x06",2);
		bRecvData[bRecvData[2]+3] = checksum(bRecvData[2] + 3,bRecvData);//EDC
		//inRetVal = (bRecvData[bRecvData[2] + 1] << 8) | (bRecvData[bRecvData[2] + 2]);
	}

	if(inRetVal == CARD_SUCCESS)
	{
		if(srTxnData.srParameter.chCommandMode == '1' &&
		  ((srTxnData.srIngData.inTransType == TXN_ECC_READ_TAXI && inDLLType == DLL_TAXI_READ) ||
		   (srTxnData.srIngData.inTransType == TXN_ECC_READ_CBIKE && inDLLType == DLL_CBIKE_READ) ||
		   (srTxnData.srIngData.inTransType == TXN_ECC_READ_EDCA && inDLLType == DLL_EDCA_READ)))
		{
			BYTE bCommand[READER_BUFFER + READER_BUFFER];

			if(inFileGetSize((char *)FILE_COMMAND1_BAK,FALSE) >= SUCCESS)
				inFileDelete((char *)FILE_COMMAND1_BAK,FALSE);

			memset(bCommand,0x00,sizeof(bCommand));
			memcpy(bCommand,bSendData,READER_BUFFER);
			memcpy(&bCommand[READER_BUFFER],bRecvData,READER_BUFFER);
			inFileWrite((char *)FILE_COMMAND1_BAK,bCommand,sizeof(bCommand));
		}
	}

	if(bBatchFlag == TRUE || inRetVal == 0x640E || inRetVal == 0x610F || inRetVal == 0x6418 || inRetVal == 0x6103 || inRetVal == 0x6406)
	{
		switch(srTxnData.srParameter.chBatchFlag)
		{
			case R6_BATCH:
				pTMFuncOK1 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pR6_TMFun_OK1;
				pTMFuncOK2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pR6_TMFun_OK2;
				pTMFuncLock1 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pR6_TMFun_Lock1;
				pTMFuncLock2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pR6_TMFun_Lock2;
				bR6FuncFlag = TRUE;
				break;
			case CMAS_BATCH:
				pTMFuncOK1 = pTMFuncOK2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pCMAS_TMFun_OK;
				pTMFuncLock1 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pCMAS_TMFun_Lock1;
				pTMFuncLock2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pCMAS_TMFun_Lock2;
				bCMASFuncFlag = TRUE;
				break;
			case SIS2_BATCH:
				pTMFuncOK1 = pTMFuncOK2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pSIS2_TMFun_OK;
				pTMFuncLock1 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pSIS2_TMFun_Lock1;
				pTMFuncLock2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pSIS2_TMFun_Lock2;
				break;
			case ICER_BATCH:
				pTMFuncOK1 = pTMFuncOK2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pICER_TMFun_OK;
				pTMFuncLock1 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pICER_TMFun_Lock1;
				pTMFuncLock2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pICER_TMFun_Lock2;
				bCMASFuncFlag = TRUE;
				break;
			default:
				log_msg(LOG_LEVEL_ERROR,"inReaderCommand2 Fail 2:(%x)(%c)(%c)",bBatchFlag,srTxnData.srParameter.chOnlineFlag,srTxnData.srParameter.chBatchFlag);
				return ICER_ERROR;
		}
	}
	else if(bBatchFlag == FALSE)
	{
		switch(srTxnData.srParameter.chOnlineFlag)
		{
			case R6_ONLINE:
				pTMFuncOK1 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pR6_TMFun_OK1;
				pTMFuncOK2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pR6_TMFun_OK2;
				pTMFuncLock1 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pR6_TMFun_Lock1;
				pTMFuncLock2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pR6_TMFun_Lock2;
				bR6FuncFlag = TRUE;
				break;
			case CMAS_ONLINE:
				pTMFuncOK1 = pTMFuncOK2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pCMAS_TMFun_OK;
				pTMFuncLock1 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pCMAS_TMFun_Lock1;
				pTMFuncLock2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pCMAS_TMFun_Lock2;
				bCMASFuncFlag = TRUE;
				break;
			case ICER_ONLINE:
				pTMFuncOK1 = pTMFuncOK2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pICER_TMFun_OK;
				pTMFuncLock1 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pICER_TMFun_Lock1;
				pTMFuncLock2 = st_ReaderCommand2[inDLLType].srReaderInfoL2.pICER_TMFun_Lock2;
				bICERFuncFlag = TRUE;
				break;
			default:
				log_msg(LOG_LEVEL_ERROR,"inReaderCommand2 Fail 3:(%x)(%c)(%c)",bBatchFlag,srTxnData.srParameter.chOnlineFlag,srTxnData.srParameter.chBatchFlag);
				return ICER_ERROR;
		}
	}
	else
	{
		log_msg(LOG_LEVEL_ERROR,"inReaderCommand2 Fail 4:(%x)(%c)(%c)",bBatchFlag,srTxnData.srParameter.chOnlineFlag,srTxnData.srParameter.chBatchFlag);
		return ICER_ERROR;
	}

	if(bCMASFuncFlag == TRUE || bICERFuncFlag == TRUE)
	{
		int inRet1,inCntStart,inCntEnd;

		inRet1 = inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_XML_END,&inCntStart,&inCntEnd,0);
		if(inRet1 >= SUCCESS)//有 /TransXML
		{
			bTransXMLFlag = TRUE;
			inXMLDeleteData(srXML,inCntStart,1);
		}

		inRet1 = inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_END,&inCntStart,&inCntEnd,0);
		if(inRet1 >= SUCCESS)//有 /TRANS
		{
			bTransFlag = TRUE;
			inXMLDeleteData(srXML,inCntStart,1);
		}
	}

	/*if(bICERFuncFlag == TRUE)
	{
		int inRet1,inCntStart,inCntEnd;

		inRet1 = inXMLSearchValueByTag(srXML,(char *)TAG_TRANS_END,&inCntStart,&inCntEnd,0);
		if(inRet1 >= SUCCESS)//有 /TRANS
			inXMLDeleteData(srXML,inCntStart,1);
	}*/

	inCnt = 0;
	if(inRetVal == CARD_SUCCESS || inRetVal == 0x6415 || inRetVal == 0x6308 || inRetVal == 0x6403)
	{
		if(bR6FuncFlag == TRUE && (inRetVal == CARD_SUCCESS || inRetVal == 0x6403))
			inCnt = pTMFuncOK1(inDLLType,&srXMLData,bOutData,&bSendData[IN_DATA_OFFSET],&bRecvData[OUT_DATA_OFFSET],&bReadCommandOut[OUT_DATA_OFFSET]);
		else
			inCnt = pTMFuncOK2(inDLLType,&srXMLData,bOutData,&bSendData[IN_DATA_OFFSET],&bRecvData[OUT_DATA_OFFSET],&bReadCommandOut[OUT_DATA_OFFSET]);
		//inCnt = st_ReaderCommand2[inDLLType].srReaderInfoL2.pTMFun_OK(inDLLType,bOutData,&bSendData[IN_DATA_OFFSET],&bRecvData[OUT_DATA_OFFSET],&bReadCommandOut[OUT_DATA_OFFSET]);
	}
	else if(inRetVal == 0x640E || inRetVal == 0x610F || inRetVal == 0x6418)
		inCnt = pTMFuncLock1(inDLLType,&srXMLData,bOutData,&bSendData[IN_DATA_OFFSET],&bRecvData[OUT_DATA_OFFSET],(BYTE *)NULL);
		//inCnt = st_ReaderCommand2[inDLLType].srReaderInfoL2.pTMFun_Lock1(inDLLType,bOutData,&bSendData[IN_DATA_OFFSET],&bRecvData[OUT_DATA_OFFSET],NULL);
	else if(inRetVal == 0x6103 || inRetVal == 0x6406)
		inCnt = pTMFuncLock2(inDLLType,&srXMLData,bOutData,&bSendData[IN_DATA_OFFSET],&bRecvData[OUT_DATA_OFFSET],(BYTE *)NULL);
		//inCnt = st_ReaderCommand2[inDLLType].srReaderInfoL2.pTMFun_Lock2(inDLLType,bOutData,&bSendData[IN_DATA_OFFSET],&bRecvData[OUT_DATA_OFFSET],NULL);
	else if(inRetVal == CARD_TIMEOUT)
	{
		log_msg(LOG_LEVEL_ERROR,"inReaderCommand2 Fail 5:(%04x)",inRetVal);
		if(st_ReaderCommand2[inDLLType].chCardAccess == '1')
			return CALL_DLL_TIMEOUT_ERROR;
		else
			return DLL_DECLINE_FAIL;
	}

	if(inCnt == 0)
		memcpy(bOutData,&bRecvData[OUT_DATA_OFFSET],bRecvData[2]);
	//else
	//	fnUnPack(bTmpData,inCnt,bOutData);

	//for AVM 批次設定為SIS2_Batch，交易成功之後還要另外送R6 confirm
	//因為bOutData已經存放SIS2包檔的資料，所以拿bInData來存放R6資料(包含confirm)
	if (srTxnData.srParameter.chAVM != 0 && srTxnData.srParameter.chBatchFlag == SIS2_BATCH && inRetVal == CARD_SUCCESS)
	{
		switch (inDLLType)
		{
			case DLL_ADD_VALUE_AUTH:
			case DLL_REFUND_CARD_AUTH:
				inBuildAuthTxnOnlineData(inDLLType,&srXMLData, bInData,&bSendData[IN_DATA_OFFSET],&bRecvData[OUT_DATA_OFFSET],&bReadCommandOut[OUT_DATA_OFFSET]);
				break;
			case DLL_AUTOLOAD_AUTH:
				inBuildAuthAutoloadData(inDLLType,&srXMLData, bInData,&bSendData[IN_DATA_OFFSET],&bRecvData[OUT_DATA_OFFSET],&bReadCommandOut[OUT_DATA_OFFSET]);
				break;
		}
	}

	memset(bReadCommandOut,0x00,sizeof(bReadCommandOut));
	memcpy(bReadCommandOut,bRecvData,st_ReaderCommand2[inDLLType].srReaderInfoL2.LE + 2 + 4);
	memset(bReadCommandIn,0x00,sizeof(bReadCommandIn));
	memcpy(bReadCommandIn,bSendData,st_ReaderCommand2[inDLLType].srReaderInfoL2.LC + 4 + 4);

	if(bCMASFuncFlag == TRUE || bICERFuncFlag == TRUE)
	{
		if(bTransFlag == TRUE)
			inXMLAppendData(srXML,(char *)TAG_TRANS_END,strlen(TAG_TRANS_END),bSendData,0,TAG_NAME_END,TRUE);
		if(bTransXMLFlag == TRUE)
			inXMLAppendData(srXML,(char *)TAG_TRANS_XML_END,strlen(TAG_TRANS_XML_END),bSendData,0,TAG_NAME_END,(bCMASFuncFlag == TRUE)?TRUE:FALSE);
	}

	//if(bICERFuncFlag == TRUE)
	//	inXMLAppendData(srXML,(char *)TAG_TRANS_END,strlen(TAG_TRANS_END),bSendData,0,TAG_NAME_END,TRUE);

	/*if(st_ReaderCommand2[inDLLType].chCardAccess == '1')
	{
		vdGetISO8583FlagIndex((BYTE *)srTxnData.srIngData.chTMProcessCode,MTI_ADVICE_TXN);
		vdIcnBankData(6);//6:加/扣成功才+1
	}*/

	return inRetVal;

}

int inSignOnQueryProcess(BYTE *bInData,BYTE *bOutData)
{
int inRetVal;

	inRetVal = inReaderCommand(DLL_SIGNONQUERY,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inSignOnQueryProcess Fail 1:%d",inRetVal);
		return inRetVal;
	}

	if( srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY ||
		srTxnData.srParameter.chSignOnMode == RESET_OFF_ONLY ||
		srTxnData.srParameter.chSignOnMode == SIGN_ON_AND_RESET_OFF ||
		srTxnData.srParameter.chSignOnMode == NO_SIGN_ON_AND_RESET)
		return(inSignOnQueryProcess2(inRetVal,bInData,bOutData));

	if(bIsResetOfflineTxn(srTxnData.srIngData.inTransType) == TRUE)
	{
		if(inRetVal == 0x6304)
		{
			inRetVal = inResetOfflineProcess(DLL_RESET_OFF,&srXMLData,bInData,bOutData);

			//inBuildAPIDefaultData(DLL_RESET_OFF,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);
			//inRetVal = inReaderCommand(DLL_RESET_OFF,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
			if(inRetVal != CARD_SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inSignOnQueryProcess Fail 2:%d",inRetVal);
				return inRetVal;
			}
		}
		else if(inRetVal == CARD_SUCCESS || inRetVal == 0x6305)
			inRetVal = CARD_SUCCESS;
		else
		{
			log_msg(LOG_LEVEL_ERROR,"inSignOnQueryProcess Fail 3:%d",inRetVal);
			return DLL_DECLINE_FAIL;
		}
	}
	else
	{
		if(inRetVal == 0x6304 || inRetVal == 0x6305)
			return inSignOnProcess(TXN_ECC_SIGN_ON,bOutData,TRUE);
		else if(inRetVal == 0x630D)
			return inSignOnProcess(TXN_ECC_MATCH,bOutData,TRUE);
		else if(inRetVal != CARD_SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inSignOnQueryProcess Fail 4:%d",inRetVal);
			return DLL_DECLINE_FAIL;
		}
	}

	return inRetVal;
}

int inSignOnQueryProcess2(int inSW,BYTE *bInData,BYTE *bOutData)
{
	int inRetVal = inSW,inDLLType;

	log_msg(LOG_LEVEL_FLOW,"inSignOnQueryProcess2 1:%d,%c",inSW,srTxnData.srParameter.chSignOnMode);
	if(inRetVal == 0x6304 || inRetVal == 0x630D)
	{
		if( srTxnData.srParameter.chSignOnMode == RESET_OFF_ONLY ||
			srTxnData.srParameter.chSignOnMode == SIGN_ON_AND_RESET_OFF)
		{
			if(inRetVal == 0x6304)
				inDLLType = DLL_RESET_OFF;
			else
				inDLLType = DLL_MATCH_OFF;

			inRetVal = inResetOfflineProcess(inDLLType,&srXMLData,bInData,bOutData);

			//inBuildAPIDefaultData(inDLLType,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);
			//inRetVal = inReaderCommand(inDLLType,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
			if(inRetVal != CARD_SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inSignOnQueryProcess2 Fail 1:%d",inRetVal);
				return inRetVal;
			}
		}

		if( srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY ||
			srTxnData.srParameter.chSignOnMode == SIGN_ON_AND_RESET_OFF)
		{
			if(inRetVal == 0x6304)
				inDLLType = TXN_ECC_SIGN_ON;
			else
				inDLLType = TXN_ECC_MATCH;

			return inSignOnProcess(inDLLType,bOutData,TRUE);
		}
	}
	else if(inRetVal == 0x6305)
	{
		if( srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY)
		{
			return inSignOnProcess(TXN_ECC_SIGN_ON,bOutData,TRUE);
		}
	}
	else if (inRetVal == 0x640F)
	{
		if( srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY ||
  		    srTxnData.srParameter.chSignOnMode == SIGN_ON_AND_RESET_OFF)
		{
  		    inDLLType = TXN_ECC_SIGN_ON;
		    return inSignOnProcess(inDLLType,bOutData,TRUE);
		}

		log_msg(LOG_LEVEL_ERROR,"inSignOnQueryProcess2 Fail 2-1:%d",inRetVal);
		return DLL_DECLINE_FAIL;
	}
	else if(inRetVal != CARD_SUCCESS)
	{
		if(srTxnData.srParameter.chSignOnMode == NO_SIGN_ON_AND_RESET)
		{
			log_msg(LOG_LEVEL_FLOW,"inSignOnQueryProcess2 Fail 2-0:%d",inRetVal);
			return SUCCESS;
		}

		log_msg(LOG_LEVEL_ERROR,"inSignOnQueryProcess2 Fail 2:%d",inRetVal);
		return DLL_DECLINE_FAIL;
	}

	return inRetVal;
}


int inSignOnProcess(int inTxnType,BYTE *bOutData,BOOL fClearData)
{
	int inRetVal = 0;
	BYTE bInData[READER_BUFFER],bRecvBuf[READER_BUFFER],chTMProcessCode[6 + 1];
	BYTE bBuf[MAX_XML_FILE_SIZE];
//unsigned short usPort = 8902;
	STRUCT_XML_DOC srTmpXMLData;
	int inDLLTrans1,inDLLTrans2;
	int inContinueFlag = 0;
	BYTE SAM_Applet_Ver = 0x00;

	memset(chTMProcessCode,0x00,sizeof(chTMProcessCode));
	memcpy(chTMProcessCode,srTxnData.srIngData.chTMProcessCode,sizeof(chTMProcessCode));
	fnMaintainBLC();

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	if (CheckDirectoryPermission() != SUCCESS)
		return DIRECTORY_PERMISSION_DENIED;
#endif

	do
	{
		if(fClearData == TRUE)
			vdXMLBackUpData(&srXMLData,&srTmpXMLData,FALSE);
		
		if(inTxnType == TXN_ECC_SIGN_ON)
		{
			inDLLTrans1 = DLL_RESET;
			inDLLTrans2 = DLL_SIGNON;
		}
		else
		{
			inDLLTrans1 = DLL_MATCH;
			inDLLTrans2 = DLL_MATCH_AUTH;
		}

		memset(bInData,0x00,sizeof(bInData));
		inBuildAPIDefaultData(inDLLTrans1,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);//TM序號不加1,以免影響Retry機制!!
		//inBuildAPIDefaultData(DLL_RESET,srTxnData.srIngData.ulTMSerialNumber++,bInData);

		if(inContinueFlag > 0)
			inBuildTxRefundInData(&bInData[10 + 2],srTxnData.srIngData.ulTMSerialNumber,inContinueFlag);

		inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,FALSE,0,FALSE);

		if(inRetVal == 0x630D)
		{
			inDLLTrans1 = DLL_MATCH;
			inDLLTrans2 = DLL_MATCH_AUTH;
			inTxnType = TXN_ECC_MATCH;
			inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
		}

		if(inRetVal != CARD_SUCCESS)
		{
			vdSignOnReturn(FALSE,fClearData,&srTmpXMLData,&srXMLData,bInData,bOutData);
			//if(fClearData == TRUE)
			//	vdFreeXMLDOC(&srTmpXMLData);
			log_msg(LOG_LEVEL_ERROR,"inSignOnProcess Fail 1:%d",inRetVal);
			if(inRetVal < SUCCESS)
				return LOAD_DLL_FUNCTION_ERROR;
			return DLL_DECLINE_FAIL;
		}

		SAM_Applet_Ver = bReadCommandOut[13];

		if(srTxnData.srParameter.gReaderMode == DLL_READER || srTxnData.srParameter.chBatchFlag == R6_BATCH)
		{
			Reset_TM_In *tDongleIn = (Reset_TM_In *)bInData;
			Reset_TM_Out *tDongleOut = (Reset_TM_Out *)bOutData;
			BYTE ucLocationID;
			BYTE ucCPULocationID[2];

			fnASCTOBIN(&ucLocationID,tDongleIn->ucLocationID,3,1,DECIMAL);//舊場站代碼
			fnASCTOBIN(ucCPULocationID,tDongleIn->ucCPULocationID,5,2,DECIMAL);//新場站代碼
			vdStoreDeviceID(tDongleOut->ucDeviceID,tDongleOut->ucDeviceID[2],ucLocationID,tDongleOut->ucCPUDeviceID,&tDongleOut->ucCPUDeviceID[3],ucCPULocationID,gTmpData.ucReaderID,gTmpData.ucReaderFWVersion,tDongleOut->ucCPUSAMID);
		}
		else //if(srTxnData.srParameter.gReaderMode == RS232_READER || srTxnData.srParameter.gReaderMode == LIBARY_READER)
		{
			Reset_TM_In *tDongleIn = (Reset_TM_In *)bInData;
			Reset_APDU_Out *tDongleOut = (Reset_APDU_Out *)bOutData;
			BYTE ucLocationID;
			BYTE ucCPULocationID[2];

			fnASCTOBIN(&ucLocationID,tDongleIn->ucLocationID,3,1,DECIMAL);//舊場站代碼
			fnASCTOBIN(ucCPULocationID,tDongleIn->ucCPULocationID,5,2,DECIMAL);//新場站代碼
			vdStoreDeviceID(tDongleOut->ucDeviceID,tDongleOut->ucDeviceID[2],ucLocationID,tDongleOut->ucCPUDeviceID,&tDongleOut->ucCPUDeviceID[3],ucCPULocationID,tDongleOut->ucReaderID,tDongleOut->ucReaderFWVersion,tDongleOut->ucCPUSAMID);
		}

		if(inTxnType == TXN_ECC_SIGN_ON)
			memcpy(srTxnData.srIngData.chTMProcessCode,"881999",sizeof(chTMProcessCode));
		else
			memcpy(srTxnData.srIngData.chTMProcessCode,"882999",sizeof(chTMProcessCode));

		memset(bRecvBuf,0x00,sizeof(bRecvBuf));
		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
			inRetVal = inSendRecvXML(HOST_CMAS,&srXMLData,TXN_ECC_SIGN_ON);
		else //if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE || srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)
			inRetVal = inTCPSendAndReceive(inDLLTrans1,bOutData,bRecvBuf,TRUE);

		memcpy(srTxnData.srIngData.chTMProcessCode,chTMProcessCode,sizeof(chTMProcessCode));

		if(inRetVal != SUCCESS)
		{
			vdSignOnReturn(FALSE,fClearData,&srTmpXMLData,&srXMLData,bInData,bOutData);
			//if(fClearData == TRUE)
			//	vdFreeXMLDOC(&srTmpXMLData);
			log_msg(LOG_LEVEL_ERROR,"inSignOnProcess Fail 2:%d",inRetVal);
			return inRetVal;
		}

		memset(bInData,0x00,sizeof(bInData));
		memset(bOutData,0x00,sizeof(bOutData));
		if(srTxnData.srParameter.gDLLVersion == 1)//1代API
		{
			memcpy(&bInData[0],&bRecvBuf[71],8);
			memcpy(&bInData[8],&bRecvBuf[84],24);
		}
		else
			memcpy(bInData,&bRecvBuf[10],193);
		inRetVal = inReaderCommand(inDLLTrans2,&srXMLData,bInData,bOutData,FALSE,0,FALSE);

		if(inRetVal != CARD_SUCCESS && inRetVal != 0x6308)
		{
			vdSignOnReturn(FALSE,fClearData,&srTmpXMLData,&srXMLData,bInData,bOutData);
			//if(fClearData == TRUE)
			//	vdFreeXMLDOC(&srTmpXMLData);
			log_msg(LOG_LEVEL_ERROR,"inSignOnProcess Fail 3:%d",inRetVal);
			if(inRetVal < SUCCESS)
				return LOAD_DLL_FUNCTION_ERROR;
			return DLL_DECLINE_FAIL;
		}

		if(srTxnData.srParameter.gDLLVersion == 1)//1代API
			break;
		else if(srTxnData.srParameter.gDLLVersion == 2)//2代API
		{
			if(inTxnType == TXN_ECC_SIGN_ON)
			{
				if(inTxnType == TXN_ECC_SIGN_ON)
					memcpy(srTxnData.srIngData.chTMProcessCode,"881999",sizeof(chTMProcessCode));
				else
					memcpy(srTxnData.srIngData.chTMProcessCode,"882999",sizeof(chTMProcessCode));

				if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
				{
					memset(bBuf, 0x00, sizeof(bBuf));
					if(inBuildSendPackage(HOST_CMAS,&srXMLData,bBuf,REVERSAL_OFF,ADVICE_ON,STORE_BATCH_OFF) < SUCCESS)
						log_msg(LOG_LEVEL_ERROR,"inSignOnProcess Fail 4");
					else
						inTCPIPAdviceProcess(HOST_CMAS,FALSE);
				}
				else //if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE || srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)
					inTCPSendAndReceive(inDLLTrans2,bOutData,bRecvBuf,FALSE);

				memcpy(srTxnData.srIngData.chTMProcessCode,chTMProcessCode,sizeof(chTMProcessCode));
			}
			else if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)//CMAS && MatchAuth
				srTxnData.srIngData.ulCMASHostSerialNumber++;
				//vdCMASSerialNumberProcess(&srXMLData,TRUE);

			if(inRetVal == CARD_SUCCESS)
			{
				if ((inTxnType == TXN_ECC_MATCH) && (SAM_Applet_Ver == 0x81 || SAM_Applet_Ver == 0x82))
					vdSignOnReturn(TRUE,fClearData,&srTmpXMLData,&srXMLData,bInData,bOutData); //不執行break，因為需要再做一次Sign On流程
				else
					break;
			}
			srTxnData.srIngData.ulTMSerialNumber++;
			if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
			{
				memcpy(&srTxnData.srIngData.anRRN[0],&srTxnData.srIngData.chTxDate[2],6);
				sprintf((char *)&srTxnData.srIngData.anRRN[6],"%06ld",srTxnData.srIngData.ulTMSerialNumber);
			}
			else
				inContinueFlag++;

			inTxnType = TXN_ECC_SIGN_ON;
		}
	}
	while(1);

	vdSignOnReturn(TRUE,fClearData,&srTmpXMLData,&srXMLData,bInData,bOutData);
	/*if(fClearData == TRUE)
	{
		vdFreeXMLDOC(&srXMLData);
		vdXMLBackUpData(&srTmpXMLData,&srXMLData,TRUE);
		vdFreeXMLDOC(&srTmpXMLData);
	}*/

	return inRetVal;
}

void vdSignOnReturn(BOOL bRetFlag,BOOL fClearData,STRUCT_XML_DOC *srTmpXML,STRUCT_XML_DOC *srXML,BYTE *bInData,BYTE *bOutData)
{
	if(bRetFlag == FALSE)//SignOn Fail
	{
		int inRetVal;

		if(srTxnData.srParameter.chSignOnMode == SIGN_ON_AND_RESET_OFF)
		{
			inRetVal = inResetOfflineProcess(DLL_RESET_OFF,&srXMLData,bInData,bOutData);
			//inBuildAPIDefaultData(DLL_RESET_OFF,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);
			//inRetVal = inReaderCommand(DLL_RESET_OFF,srXML,bInData,bOutData,FALSE,0,FALSE);
			if(inRetVal != CARD_SUCCESS)
				log_msg(LOG_LEVEL_ERROR,"vdSignOnReturn Fail 1:%d",inRetVal);
		}

		if(fClearData == TRUE)
			vdFreeXMLDOC(srTmpXML);
	}
	else//SignOn OK
	{
		//@ SignOn 成功後把TMLocationID存到ICERAPI.tmp內 add by EricChan
		if (srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		{
			if(srTxnData.srParameter.gDLLVersion == 1)
				vdStoreTMLocationID((BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID);
			else
				vdStoreTMLocationID((BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID);
		}
		
		if(fClearData == TRUE)
		{
			vdFreeXMLDOC(srXML);
			vdXMLBackUpData(srTmpXML,srXML,TRUE);
			vdFreeXMLDOC(srTmpXML);
		}
	}
}

int inQueryCardData(int inTxnType)
{
BYTE bInData[READER_BUFFER],bOutData[READER_BUFFER],bBuf[10];
int inRetVal,inCnt,inSize;

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	inRetVal = inSignOnQueryProcess(bInData,bOutData);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inQueryCardData Fail 1:%d",inRetVal);
		return inRetVal;
	}

	//Sleep(1000);
	if(srTxnData.srParameter.gDLLVersion == 2)//2代API
	{
		inCnt = 0;
		memset(bInData,0x00,sizeof(bInData));
		memset(bOutData,0x00,sizeof(bOutData));
		bInData[inCnt++] = 0x01;//LCD Flag = 0 --> 請勿移動票卡(不Beep)
		inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID) - 1;
		memcpy(&bInData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID) - 1;
		memcpy(&bInData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srIngData.chTxDate) - 1;
		memcpy(&bInData[inCnt],srTxnData.srIngData.chTxDate,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srIngData.chTxTime) - 1;
		memcpy(&bInData[inCnt],srTxnData.srIngData.chTxTime,inSize);
		inCnt += inSize;
	}
	inRetVal = inReaderCommand(DLL_READ_CARD_BASIC_DATA,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
	if(inRetVal != CARD_SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inQueryCardData Fail 2:%d",inRetVal);
		if(inRetVal < SUCCESS)
			return LOAD_DLL_FUNCTION_ERROR;
		return DLL_DECLINE_FAIL;
	}

	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		ReadCardBasicDataL1_TM_Out *TMOut = (ReadCardBasicDataL1_TM_Out *)bOutData;

		vdTrimData((char *)srTxnData.srIngData.anCardID,(char *)TMOut->anCardID,sizeof(TMOut->anCardID));
		//memcpy(srTxnData.srIngData.anExpiryDate,"9912",4);
		memcpy(srTxnData.srIngData.anExpiryDate,TMOut->anExpiryDate,8);
		srTxnData.srIngData.anExpiryDate[8] = 0x00;//有效期只要YYYYMMDD就好
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,TMOut->anEV,sizeof(TMOut->anEV));
		srTxnData.srIngData.lnECCEV = atol((char *)bBuf);
		sprintf((char *)srTxnData.srIngData.anCardEVBeforeTxn,"%ld00",srTxnData.srIngData.lnECCEV);
		srTxnData.srIngData.anAutoloadFlag[0] = (TMOut->anAutoLoad[2] == '1')?'1':'0';
		vdTrimData((char *)srTxnData.srIngData.anAutoloadAmount,(char *)TMOut->anAutoLoadAmt,sizeof(TMOut->anAutoLoadAmt));
		strcat((char *)srTxnData.srIngData.anAutoloadAmount,"00");
	}
	else
	{
		ReadCardBasicDataL2_TM_Out *TMOut = (ReadCardBasicDataL2_TM_Out *)bOutData;

		vdTrimData((char *)srTxnData.srIngData.anCardID,(char *)TMOut->anCardID,sizeof(TMOut->anCardID));
		//memcpy(srTxnData.srIngData.anExpiryDate,"9912",4);
		memcpy(srTxnData.srIngData.anExpiryDate,TMOut->anExpiryDate,8);
		srTxnData.srIngData.anExpiryDate[8] = 0x00;//有效期只要YYYYMMDD就好
		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,TMOut->anEV,sizeof(TMOut->anEV));
		srTxnData.srIngData.lnECCEV = atol((char *)bBuf);
		sprintf((char *)srTxnData.srIngData.anCardEVBeforeTxn,"%ld00",srTxnData.srIngData.lnECCEV);
		srTxnData.srIngData.anAutoloadFlag[0] = TMOut->ucAutoLoad;
		//srTxnData.srIngData.anAutoloadFlag[0] = TMOut->ucAutoLoad + '0';
		vdTrimData((char *)srTxnData.srIngData.anAutoloadAmount,(char *)TMOut->anAutoLoadAmt,sizeof(TMOut->anAutoLoadAmt));
		strcat((char *)srTxnData.srIngData.anAutoloadAmount,"00");
	}

	return inRetVal;
}

int inBuildAPIDefaultData(int inDLLType,unsigned long ulTMSerialNo,BYTE *bOutData,int *inDateTimeAdr)
{
int inCnt = 0,inSize;

	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID) - 1;
		//@ SignOn相關的交易以ini內的TMLocationID，其他則以tmp file內的
		if ((inDLLType != DLL_RESET) && (inDLLType != DLL_SIGNON) && (inDLLType != DLL_MATCH) && (inDLLType != DLL_MATCH_AUTH))
		{
			int iRet = inGetTMLocationID(&bOutData[inCnt]);
			if (iRet != SUCCESS)	//@ 若無法取得tmp內的，還是取ini內的
		memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,inSize);
		}
		else
			memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMLocationID,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID) - 1;
		memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMID,inSize);
		inCnt += inSize;

		*inDateTimeAdr = inCnt;
		inSize = sizeof(srTxnData.srIngData.chTxDate) - 1;
		memcpy(&bOutData[inCnt],srTxnData.srIngData.chTxDate,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srIngData.chTxTime) - 1;
		memcpy(&bOutData[inCnt],srTxnData.srIngData.chTxTime,inSize);
		inCnt += inSize;

		/*inSize = 14;
		GetCurrentDateTime(&bOutData[inCnt]);
		inCnt += inSize;*/

		inSize = 6;
		sprintf((char *)&bOutData[inCnt],"%06ld",ulTMSerialNo);
		//memcpy(&bOutData[inCnt],bTMSerialNo,6);
		inCnt += inSize;

		inSize = 4;
		//if(fIsECCTxn(srTxnData.srIngData.inTransType) == TRUE)//小額消費交易
			memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData1.chTMAgentNumber,inSize);
		//else//行銷平台交易
		//	memcpy(&bOutData[inCnt],srTxnData.srIngData.chICERTMAgentNumber,inSize);
		inCnt += inSize;
	}
	else//2代API
	{
		inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID) - 1;
		if ((inDLLType != DLL_RESET) && (inDLLType != DLL_SIGNON) && (inDLLType != DLL_MATCH) && (inDLLType != DLL_MATCH_AUTH))
		{
			int iRet = inGetTMLocationID(&bOutData[inCnt]);
			if (iRet != SUCCESS)	//@ 若無法取得tmp內的，還是取ini內的
				memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,inSize);
		}
		else
		memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID) - 1;
		memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,inSize);
		inCnt += inSize;

		*inDateTimeAdr = inCnt;
		inSize = sizeof(srTxnData.srIngData.chTxDate) - 1;
		memcpy(&bOutData[inCnt],srTxnData.srIngData.chTxDate,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srIngData.chTxTime) - 1;
		memcpy(&bOutData[inCnt],srTxnData.srIngData.chTxTime,inSize);
		inCnt += inSize;

		/*inSize = 14;
		GetCurrentDateTime(&bOutData[inCnt]);
		inCnt += inSize;*/

		inSize = 6;
		sprintf((char *)&bOutData[inCnt],"%06ld",ulTMSerialNo);
		//memcpy(&bOutData[inCnt],bTMSerialNo,6);
		inCnt += inSize;

		inSize = 4;
		//if(fIsECCTxn(srTxnData.srIngData.inTransType) == TRUE)//小額消費交易
			memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,inSize);
		//else
		//	memcpy(&bOutData[inCnt],srTxnData.srIngData.chICERTMAgentNumber,inSize);
		inCnt += inSize;

		if( inDLLType == DLL_RESET ||
			inDLLType == DLL_RESET_OFF ||
			inDLLType == DLL_MATCH ||
			inDLLType == DLL_RESET2 ||
			inDLLType == DLL_MATCH2 ||
			inDLLType == DLL_MATCH_OFF)
		{
			inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chLocationID) - 1;
			memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chLocationID,inSize);
			inCnt += inSize;

			inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewLocationID) - 1;
			memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewLocationID,inSize);
			inCnt += inSize;

			inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chSPID) - 1;
			memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chSPID,inSize);
			inCnt += inSize;

			inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewSPID) - 1;
			memcpy(&bOutData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chNewSPID,inSize);
			inCnt += inSize;

			fnPack((char *)srTxnData.srParameter.bSlot,2,&bOutData[inCnt++]);

			/*if(srTxnData.srParameter.chAccFreeRidesMode == '1')
				bOutData[inCnt] = 0x01;
			inCnt++;*/
		}
	}

	return inCnt;
}

int inGetCardData(int inTxnType,BOOL fHostMACCheckFlag)
{
BYTE bInData[READER_BUFFER],bOutData[READER_BUFFER],ucAData[47],ucAData2[100];
int inRetVal,inCnt = 0,inCnt2 = 0,inSize,inDataTimeAdr = 0;

	do
	{
		memset(bInData,0x00,sizeof(bInData));
		memset(bOutData,0x00,sizeof(bOutData));
		memset(ucAData,0x00,sizeof(ucAData));
		memset(ucAData2,0x00,sizeof(ucAData2));

		inSize = 2;
		memcpy(&bInData[inCnt],"02",inSize);
		inCnt += inSize;

		inSize = 2;
		if(inTxnType == TXN_ADJECT_CARD)//指定加值
			memcpy(&bInData[inCnt],"35",inSize);
		else
			memcpy(&bInData[inCnt],"30",inSize);
		inCnt += inSize;

		if(srTxnData.srIngData.fTMRetryFlag)
		{
			if(srTxnData.srParameter.gDLLVersion == 1)//1代API
			{
				memcpy(&bInData[4],srTxnData.srIngData.bTMData,TM1_DATA_LEN + 5);
				memset(&srTxnData.srIngData.bTMData[8],'X',14);
			}
			else
			{
				memcpy(&bInData[4],srTxnData.srIngData.bTMData,TM2_DATA_LEN + 8);
				memset(&srTxnData.srIngData.bTMData[12],'X',14);
			}
		}
		else
		{
			/*inRetVal = inSignOnQueryProcess(bInData,bOutData);
			if(inRetVal < SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inGetCardData Fail 1:%d",inRetVal);
				return inRetVal;
			}*/

			//Sleep(1000);
			inCnt += inBuildAPIDefaultData(DLL_ADD_VALUE,srTxnData.srIngData.ulTMSerialNumber++,&bInData[inCnt],&inDataTimeAdr);

			if(srTxnData.srParameter.gDLLVersion == 1)//1代API
				inSize = 5;
			else
				inSize = 8;
			if(inTxnType == TXN_ADJECT_CARD)
			{
				if(srTxnData.srParameter.gDLLVersion == 1)//1代API
					sprintf((char *)&bInData[inCnt],"%05ld",srTxnData.srIngData.lnECCAmt);
				else
					sprintf((char *)&bInData[inCnt],"%08ld",srTxnData.srIngData.lnECCAmt);
			}
			else
				memset(&bInData[inCnt],0x30,inSize);
			inCnt += inSize;

			bInData[inCnt++] = 0x00;

			if(srTxnData.srParameter.gDLLVersion == 1)//1代API
				memcpy(srTxnData.srIngData.bTMData,&bInData[4],TM1_DATA_LEN + 5);
			else
				memcpy(srTxnData.srIngData.bTMData,&bInData[4],TM2_DATA_LEN + 8);
			memset(&srTxnData.srIngData.bTMData[inDataTimeAdr],'X',14);
		}

		{
			char chOnlineFlag = srTxnData.srParameter.chOnlineFlag;

			srTxnData.srParameter.chOnlineFlag = R6_ONLINE;
			inRetVal = inReaderCommand(DLL_ADD_VALUE,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
			srTxnData.srParameter.chOnlineFlag = chOnlineFlag;
		}

		if(inRetVal != 0x6415 && inRetVal != CARD_SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetCardData Fail 1:%d",inRetVal);
			if(inRetVal < SUCCESS)
				return LOAD_DLL_FUNCTION_ERROR;
			return DLL_DECLINE_FAIL;
		}

		if(fIsRetry(inRetVal))//Reader回覆需Retry
			return SUCCESS;
			//continue;
		else if(srTxnData.srIngData.fTMRetryFlag)//TM要求Retry交易
		{
			BYTE anCardID[20],anTmp[20],anEV[10 + 1];
			UnionLong ulECCEV;

			memset(anCardID,0x00,sizeof(anCardID));
			memset(anTmp,0x00,sizeof(anTmp));
			memset(anEV,0x00,sizeof(anEV));
			ulECCEV.Value = 0L;
			if(srTxnData.srParameter.gDLLVersion == 1)//1代API
			{
				AddValueL1_TM_Out_9000 *TMOut = (AddValueL1_TM_Out_9000 *)bOutData;

				vdIntToAsc(TMOut->ucCardID,sizeof(TMOut->ucCardID),anCardID,17,FALSE,' ',10);
				vdTrimData((char *)anCardID,(char *)anCardID,17);
				memcpy((char *)ulECCEV.Buf,TMOut->ucEV,sizeof(TMOut->ucEV));
				if(TMOut->ucEV[1] >= 0x80)
				{
					ulECCEV.Buf[2] = 0xFF;
					ulECCEV.Buf[3] = 0xFF;
				}

				sprintf((char *)anEV,"%ld00",ulECCEV.Value);
			}
			else //if(inRetVal == 0x6415)
			{
				AddValueL2_TM_Out_6415 *TMOut = (AddValueL2_TM_Out_6415 *)bOutData;

				vdIntToAsc(TMOut->ucCardID,sizeof(TMOut->ucCardID),anTmp,17,FALSE,' ',10);
				vdTrimData((char *)anCardID,(char *)anTmp,17);
				memcpy((char *)ulECCEV.Buf,TMOut->ucEV,sizeof(TMOut->ucEV));
				if(TMOut->ucEV[2] >= 0x80)
				{
					ulECCEV.Buf[3] = 0xFF;
				}

				sprintf((char *)anEV,"%ld00",ulECCEV.Value);
			}

			if((inXMLCompareData(&srXMLData,(char *)TAG_NAME_0200,(char *)anCardID,0) == SUCCESS) && (inXMLCompareData(&srXMLData,(char *)TAG_NAME_0410,(char *)anEV,0) == SUCCESS))//TM的Retry資料與實際卡片資料相符,需重送Reversal
				vdReversalOn(HOST_ICER);
			else//TM的Retry資料與實際卡片資料不相符,拒絕
			{
				log_msg(LOG_LEVEL_ERROR,"inGetCardData Fail 2-0");
				return DLL_NOT_RETRY;
			}
		}
		else if(inRetVal == CARD_SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inGetCardData Fail 2-1");
			return DLL_NOT_RETRY;
		}

		if(inTxnType == TXN_ADJECT_CARD/* && srTxnData.srParameter.chReSendReaderAVR == '1'*/)
			vdReSendR6ReaderAVR(bOutData);

		if(srTxnData.srParameter.gDLLVersion == 1)//1代API
		{
			AddValueL1_TM_Out_9000 *TMOut = (AddValueL1_TM_Out_9000 *)bOutData;
			BYTE bBuf[30];

			srTxnData.srIngData.ucPurseVersionNumber = MIFARE;
			inCnt = inSize = 0;

			ucAData[inCnt++] = TMOut->ucMsgType;

			ucAData[inCnt++] = TMOut->ucSubType;

			inSize = sizeof(TMOut->ucDeviceID);
			memcpy(&ucAData[inCnt],TMOut->ucDeviceID,inSize);
			inCnt += inSize;
			memset(bBuf,0x00,sizeof(bBuf));
			vdStoreDeviceID(TMOut->ucDeviceID,TMOut->ucSPID,LOCATION_ID,bBuf,bBuf,bBuf,bBuf,bBuf,bBuf);
			//vdStoreDeviceID(TMOut->ucDeviceID);
			//sprintf((char *)srTxnData.srIngData.anDeviceID,"%03d%02d%04d",TMOut->ucDeviceID[2],TMOut->ucDeviceID[1] >> 4,((TMOut->ucDeviceID[1] & 0x0F) * 256) + TMOut->ucDeviceID[0]);

			inSize = sizeof(TMOut->ucTxnDateTime);
			memcpy(&ucAData[inCnt],TMOut->ucTxnDateTime,inSize);
			inCnt += inSize;

			inSize = sizeof(TMOut->ucCardID);
			memcpy(&ucAData[inCnt],TMOut->ucCardID,inSize);
			inCnt += inSize;

			inSize = sizeof(TMOut->ucTxnAmt);
			memcpy(&ucAData[inCnt],TMOut->ucTxnAmt,inSize);
			inCnt += inSize;

			inSize = sizeof(TMOut->ucTMLocationID);
			memcpy(&ucAData[inCnt],TMOut->ucTMLocationID,inSize);
			inCnt += inSize;

			inSize = sizeof(TMOut->ucTMID);
			memcpy(&ucAData[inCnt],TMOut->ucTMID,inSize);
			inCnt += inSize;

			inSize = sizeof(TMOut->ucEV);
			memcpy(&ucAData[inCnt],TMOut->ucEV,inSize);
			inCnt += inSize;

			ucAData[inCnt++] = TMOut->ucKeyVersion;

			inSize = sizeof(TMOut->ucSAMID);
			memcpy(&ucAData[inCnt],TMOut->ucSAMID,inSize);
			memcpy(srTxnData.srIngData.anSAMID,TMOut->ucSAMID,inSize);
			inCnt += inSize;

			inSize = sizeof(TMOut->ucSAMSN);
			memcpy(&ucAData[inCnt],TMOut->ucSAMSN,inSize);
			inCnt += inSize;

			inSize = sizeof(TMOut->ucSAMCRN);
			memcpy(&ucAData[inCnt],TMOut->ucSAMCRN,inSize);
			inCnt += inSize;

			fnUnPack(TMOut->ucSTAC,sizeof(TMOut->ucSTAC),srTxnData.srIngData.anMAC);

			if(fHostMACCheckFlag)
				return SUCCESS;

			fnUnPack(ucAData,inCnt,srTxnData.srIngData.anAData);
			memset(srTxnData.srIngData.anTLKR_TLRC,0x00,sizeof(srTxnData.srIngData.anTLKR_TLRC));
			srTxnData.srIngData.anTLKR_TLRC[0] = ' ';
		}
		else //if(srTxnData.srParameter.gDLLVersion == 2)
		{
			AddValueL2_TM_Out_6415 *TMOut = (AddValueL2_TM_Out_6415 *)bOutData;
			BYTE bBuf[30];

			memset(bBuf,0x00,sizeof(bBuf));
			srTxnData.srIngData.ucPurseVersionNumber = TMOut->ucPurseVersionNumber;
			vdStoreDeviceID(TMOut->ucDeviceID,TMOut->ucSPID,TMOut->ucLocationID,TMOut->ucCPUDeviceID,TMOut->ucCPUSPID,TMOut->ucCPULocationID,gTmpData.ucReaderID,gTmpData.ucReaderFWVersion,TMOut->ucCPUSAMID);
			//vdStoreDeviceID(TMOut->ucDeviceID);
			/*if(srTxnData.srParameter.ucPurseVersionNumber != MIFARE)
			{
				log_msg(LOG_LEVEL_ERROR,"inGetCardData Fail 2:%d",inRetVal);
				return DLL_NOT_SUPPORT_CPU_CARD;
			}*/

			{
				memset(bBuf,0x00,sizeof(bBuf));
				vdUIntToAsc(TMOut->ucCardID,sizeof(TMOut->ucCardID),bBuf,17,FALSE,' ',10);
				vdTrimData((char *)srTxnData.srIngData.anCardID,(char *)bBuf,17);

				memset(bBuf,0x00,sizeof(bBuf));
				UnixToDateTime((BYTE*)TMOut->ucExpiryDate,bBuf,14);
				memcpy(srTxnData.srIngData.anExpiryDate,bBuf,8);

				srTxnData.srIngData.lnECCEV = lnAmt3ByteToLong(TMOut->ucEV);
				sprintf((char *)srTxnData.srIngData.anCardEVBeforeTxn,"%ld00",srTxnData.srIngData.lnECCEV);
				srTxnData.srIngData.anAutoloadFlag[0] = ((TxnReqOnline_APDU_Out *)&bReadCommandOut[OUT_DATA_OFFSET])->bAutoLoad + '0';
				vdUIntToAsc(((TxnReqOnline_APDU_Out *)&bReadCommandOut[OUT_DATA_OFFSET])->ucAutoLoadAmt,3,srTxnData.srIngData.anAutoloadAmount,10,TRUE,0x00,10);
				strcat((char *)srTxnData.srIngData.anAutoloadAmount,"00");
			}

			if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE/* || srTxnData.srIngData.ucPurseVersionNumber == LEVEL1*/)
			{
				inCnt = inSize = 0;

				ucAData[inCnt++] = TMOut->ucMsgType;

				ucAData[inCnt++] = TMOut->ucSubType;

				inSize = sizeof(TMOut->ucDeviceID);
				memcpy(&ucAData[inCnt],TMOut->ucDeviceID,inSize);
				inCnt += inSize;
				//sprintf((char *)srTxnData.srIngData.anDeviceID,"%03d%02d%04d",TMOut->ucDeviceID[2],TMOut->ucDeviceID[1] >> 4,((TMOut->ucDeviceID[1] & 0x0F) * 256) + TMOut->ucDeviceID[0]);

				inSize = sizeof(TMOut->ucTxnDateTime);
				memcpy(&ucAData[inCnt],TMOut->ucTxnDateTime,inSize);
				inCnt += inSize;

				inSize = 4;
				//inSize = sizeof(TMOut->ucCardID);
				memcpy(&ucAData[inCnt],TMOut->ucCardID,inSize);
				inCnt += inSize;

				inSize = 2;
				//inSize = sizeof(TMOut->ucTxnAmt);
				memcpy(&ucAData[inCnt],TMOut->ucTxnAmt,inSize);
				inCnt += inSize;

				inSize = 6;
				memcpy(&ucAData[inCnt],&TMOut->ucTMLocationID[4],inSize);
				//inSize = sizeof(TMOut->ucTMLocationID);
				//memcpy(&ucAData[inCnt],TMOut->ucTMLocationID,inSize);
				inCnt += inSize;

				inSize = sizeof(TMOut->ucTMID);
				memcpy(&ucAData[inCnt],TMOut->ucTMID,inSize);
				inCnt += inSize;

				inSize = 2;
				//inSize = sizeof(TMOut->ucEV);
				memcpy(&ucAData[inCnt],TMOut->ucEV,inSize);
				inCnt += inSize;

				ucAData[inCnt++] = TMOut->ucKeyVersion;

				inSize = sizeof(TMOut->ucSAMID);
				memcpy(&ucAData[inCnt],TMOut->ucSAMID,inSize);
				memcpy(srTxnData.srIngData.anSAMID,TMOut->ucSAMID,inSize);
				inCnt += inSize;

				inSize = sizeof(TMOut->ucSAMSN);
				memcpy(&ucAData[inCnt],TMOut->ucSAMSN,inSize);
				inCnt += inSize;

				inSize = sizeof(TMOut->ucSAMCRN);
				memcpy(&ucAData[inCnt],TMOut->ucSAMCRN,inSize);
				inCnt += inSize;

				fnUnPack(TMOut->ucSTAC,sizeof(TMOut->ucSTAC),srTxnData.srIngData.anMAC);
			}
			else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL1 || srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
			{
				ucAData2[inCnt2++] = TMOut->ucCreditKeyKVN;

				inSize = sizeof(TMOut->ucPID);
				memcpy(&ucAData2[inCnt2],TMOut->ucPID,inSize);
				inCnt2 += inSize;

				inSize = sizeof(TMOut->ucCTC);
				memcpy(&ucAData2[inCnt2],TMOut->ucCTC,inSize);
				inCnt2 += inSize;

				ucAData2[inCnt2++] = 0x36;

				inSize = sizeof(TMOut->ucTxnAmt);
				memcpy(&ucAData2[inCnt2],TMOut->ucTxnAmt,inSize);
				inCnt2 += inSize;

				inSize = sizeof(TMOut->ucTxnDateTime);
				memcpy(&ucAData2[inCnt2],TMOut->ucTxnDateTime,inSize);
				inCnt2 += inSize;

				ucAData2[inCnt2++] = TMOut->ucTxnQuqlifier;

				inSize = sizeof(TMOut->ucCPUDeviceID);
				memcpy(&ucAData2[inCnt2],TMOut->ucCPUDeviceID,inSize);
				inCnt2 += inSize;

				ucAData2[inCnt2++] = TMOut->ucTxnMode;

				inSize = sizeof(TMOut->ucTMLocationID);
				memcpy(&ucAData2[inCnt2],TMOut->ucTMLocationID,inSize);
				inCnt2 += inSize;

				inSize = sizeof(TMOut->ucPID);
				memcpy(&ucAData2[inCnt2],TMOut->ucPID,inSize);
				inCnt2 += inSize;

				inSize = sizeof(TMOut->ucCardID);
				memcpy(&ucAData2[inCnt2],TMOut->ucCardID,inSize);
				inCnt2 += inSize;

				inSize = sizeof(TMOut->ucEV);
				memcpy(&ucAData2[inCnt2],TMOut->ucEV,inSize);
				inCnt2 += inSize;

				inSize = sizeof(TMOut->ucTxnSN);
				memcpy(&ucAData2[inCnt2],TMOut->ucTxnSN,inSize);
				inCnt2 += inSize;

				ucAData2[inCnt2++] = TMOut->ucHostAdminKVN;

				inSize = sizeof(TMOut->ucCPUSAMID);
				memcpy(&ucAData2[inCnt2],TMOut->ucCPUSAMID,inSize);
				inCnt2 += inSize;

				fnUnPack(TMOut->ucCACrypto,sizeof(TMOut->ucCACrypto) / 2,srTxnData.srIngData.anMAC2);
			}

			if(fHostMACCheckFlag)
				return SUCCESS;

			if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE/* || srTxnData.srIngData.ucPurseVersionNumber == LEVEL1*/)
				fnUnPack(ucAData,inCnt,srTxnData.srIngData.anAData);
			else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL1 || srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
				fnUnPack(ucAData2,inCnt2,srTxnData.srIngData.anAData2);
			memset(srTxnData.srIngData.anTLKR_TLRC,0x00,sizeof(srTxnData.srIngData.anTLKR_TLRC));
			srTxnData.srIngData.anTLKR_TLRC[0] = ' ';

			//T6000
			memcpy(ucReaderFWVersion,TMOut->ucReaderFWVersion,sizeof(TMOut->ucReaderFWVersion));
			memcpy(ucSAMSN,TMOut->ucSAMSN,sizeof(TMOut->ucSAMSN));
			vdUnPackToXMLData(ucReaderFWVersion,sizeof(ucReaderFWVersion),&srXMLData,(char *)"T6000",strlen("T6000"),VALUE_NAME,TRUE);
		}

		return SUCCESS;
	}
	while(1);

}

int inECCDeductOrRefund(BOOL fDeductFlag,int inTxnType,long lnTxnAmt)
{
BYTE bInData[READER_BUFFER],bOutData[READER_BUFFER],bRecvBuf[READER_BUFFER],bBuf[10]/*,bTMData[32 + 1]*/;
int inRetVal,inCnt = 0,inSize;
int inDLLTrans1,inDLLTrans2;
int inTMDataLen;

	if(inTxnType != TXN_ADJECT_CARD)//指定加值不能SignOnQuery,否則會失敗!!
	{
		//SignOnQuery處理
		inRetVal = inSignOnQueryProcess(bInData,bOutData);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inECCDeductOrRefund Fail 1:%d",inRetVal);
			return inRetVal;
		}
	}

	//Sleep(1000);
	//memset(bTMData,0x00,sizeof(bTMData));
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));

	if(fDeductFlag)//扣款交易
	{
		log_msg(LOG_LEVEL_FLOW,"ucAutoload_fail_flag = %d, RetryFlag = %d", gTmpData.ucAutoload_fail_flag, srTxnData.srIngData.fTMRetryFlag);
		if (gTmpData.ucAutoload_fail_flag && srTxnData.srIngData.fTMRetryFlag)	//是重試且上次在Autoload時出現問題，先進行Autoload的retry
		{
			if (inGetAutoloadData(bOutData) == SUCCESS)
			{
				log_msg(LOG_LEVEL_FLOW, "CARD ID in TMP = %s", gTmpData.ucAutoloadData);
				inRetVal = inAutoloadProcess(bOutData);
				if(inRetVal != SUCCESS)
				{
					log_msg(LOG_LEVEL_ERROR,"inECCDeductOrRefund Fail (AutoloadRetry):%d",inRetVal);
					//return AMOUNT_FAIL;
				}
			}
			gTmpData.ucAutoload_fail_flag = FALSE;
		}
		inDLLTrans1 = DLL_DEDUCT_VALUE;
		inDLLTrans2 = DLL_DEDUCT_VALUE_AUTH;

		inSize = 2;
		memcpy(&bInData[inCnt],"01",inSize);
		inCnt += inSize;

		if(srTxnData.srParameter.gDLLVersion == 2)
		{
			inSize = 2;
			memcpy(&bInData[inCnt],"00",inSize);
			inCnt += inSize;
		}
	}
	else//加值交易
	{
		inDLLTrans1 = DLL_ADD_VALUE;
		inDLLTrans2 = DLL_ADD_VALUE_AUTH;

		inSize = 2;
		memcpy(&bInData[inCnt],"02",inSize);
		inCnt += inSize;

		inSize = 2;
		if(inTxnType == TXN_ADJECT_CARD)//指定加值
			memcpy(&bInData[inCnt],"35",inSize);
		else if(inTxnType == TXN_REFUND)//退貨的加值
			memcpy(&bInData[inCnt],"0B",inSize);
		inCnt += inSize;
	}

	inCnt += inBuildAPIDefaultData(inDLLTrans1,srTxnData.srIngData.ulTMSerialNumber++,&bInData[inCnt],&inRetVal);

	if(srTxnData.srParameter.gDLLVersion == 1)
		inTMDataLen = TM1_DATA_LEN;
	else
		inTMDataLen = TM2_DATA_LEN;

	inCnt += inBuildTxnAmt(&bInData[inCnt],lnTxnAmt);

	bInData[inCnt++] = 0x00;

	if(inTxnType == TXN_ADJECT_CARD)//指定加值
	{//H-TAC直接從TAG_NAME_6401抓取,不需執行第一道加值指令,因為之前已做過,也不需連線R6
		char chHTAC[40];

		memset(bInData,0x00,sizeof(bInData));
		memset(chHTAC,0x00,sizeof(chHTAC));
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_6401,chHTAC,16,0);
		if(inRetVal >= SUCCESS)
		{
			if(srTxnData.srParameter.gDLLVersion == 1)//1代API
				fnPack(chHTAC,16,&bInData[4]);
			else //if(srTxnData.srParameter.gDLLVersion == 2)//2代API
				fnPack(chHTAC,16,&bInData[53]);
		}
		else
		{
			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_6409,chHTAC,32,0);
			if(inRetVal < SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inECCDeductOrRefund Fail 2:%d",inRetVal);
				return inRetVal;
			}
			fnPack(chHTAC,32,&bInData[53 + 46]);
			bInData[42 + 46] = LEVEL2;
		}
	}
	else
	{
		if(fDeductFlag)//扣款交易
			memcpy(srTxnData.srIngData.bTMData,&bInData[2],inTMDataLen);
		else//非扣款交易
			memcpy(srTxnData.srIngData.bTMData,&bInData[4],inTMDataLen);

		//第一道指令
		{
			char chOnlineFlag = srTxnData.srParameter.chOnlineFlag;

			srTxnData.srParameter.chOnlineFlag = R6_ONLINE;
			inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
			srTxnData.srParameter.chOnlineFlag = chOnlineFlag;
		}

		if(inRetVal == 0x6403)//餘額不足
		{	//自動加值處理
			inRetVal = inAutoloadProcess(bOutData);
			if(inRetVal != SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inECCDeductOrRefund Fail 3:%d",inRetVal);
				return AMOUNT_FAIL;
			}

			inCnt = 0;
			inSize = 2;
			memcpy(&bInData[inCnt],"01",inSize);
			inCnt += inSize;

			if(srTxnData.srParameter.gDLLVersion == 2)
			{
				inSize = 2;
				memcpy(&bInData[inCnt],"00",inSize);
				inCnt += inSize;
			}

			inCnt += inBuildAPIDefaultData(inDLLTrans1,srTxnData.srIngData.ulTMSerialNumber++,&bInData[inCnt],&inRetVal);

			inCnt += inBuildTxnAmt(&bInData[inCnt],lnTxnAmt);

			bInData[inCnt++] = 0x01;
			//在執行一次扣款第一道指令
			{
				char chOnlineFlag = srTxnData.srParameter.chOnlineFlag;

				srTxnData.srParameter.chOnlineFlag = R6_ONLINE;
				inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
				srTxnData.srParameter.chOnlineFlag = chOnlineFlag;
			}
		}

		if((inRetVal == CARD_SUCCESS && !fDeductFlag && srTxnData.srParameter.gDLLVersion == 1) ||
		   inRetVal == 0x6415)//一代指令 & 非扣款交易 & 回應碼9000,或回應碼6415,即需連線R6
		{
			memset(bInData,0x00,sizeof(bInData));
			inCnt = 0;
			memset(bRecvBuf,0x00,sizeof(bRecvBuf));
			inRetVal = inTCPSendAndReceive(inDLLTrans1,bOutData,bRecvBuf,TRUE);
			if(inRetVal != SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inECCDeductOrRefund Fail 4:%d",inRetVal);
				return inRetVal;
			}
			if(srTxnData.srParameter.gDLLVersion == 1)//1代API
			{
				if(!fDeductFlag)
				{
					memcpy(&bInData[inCnt],&bRecvBuf[39],4);
					inCnt += 4;
				}
				memcpy(&bInData[inCnt],&bRecvBuf[75],8);
				//bInData[8] = 0x00;
			}
			else
				memcpy(&bInData[inCnt],&bRecvBuf[10],sizeof(bRecvBuf) - 10);
		}
		else if(inRetVal == CARD_SUCCESS)//回應碼9000,第二道指令的Input_Data直接帶0
			memset(bInData,0x00,sizeof(bInData));
		else if(inRetVal == 0x6406 ||
				inRetVal == 0x640E ||
				inRetVal == 0x6103 ||
				inRetVal == 0x610F ||
				inRetVal == 0x6418)//需組鎖卡回饋檔之回應碼
		{
			memcpy(srTxnData.srIngData.anTLKR_TLRC,bOutData,TLKR_TLRC_LEN);
			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
			if(inRetVal < SUCCESS)//尚未有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,TRUE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
			}
			else//已有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
			}
			log_msg(LOG_LEVEL_ERROR,"inECCDeductOrRefund Fail 5:%d",inRetVal);
			return DLL_DECLINE_FAIL;
		}
		else if(inRetVal != CARD_SUCCESS)//失敗之回應碼
		{
			log_msg(LOG_LEVEL_ERROR,"inECCDeductOrRefund Fail 6:%d",inRetVal);
			if(inRetVal < SUCCESS)
				return inRetVal;

			if(inRetVal < SUCCESS)
				return LOAD_DLL_FUNCTION_ERROR;
			return DLL_DECLINE_FAIL;
		}
	}

	memset(bOutData,0x00,sizeof(bOutData));
	//第二道指令
	{
		char chOnlineFlag = srTxnData.srParameter.chOnlineFlag;
		char chBatchFlag = srTxnData.srParameter.chBatchFlag;

		srTxnData.srParameter.chOnlineFlag = R6_ONLINE;
		srTxnData.srParameter.chBatchFlag = R6_BATCH;
		inRetVal = inReaderCommand(inDLLTrans2,&srXMLData,bInData,bOutData,TRUE,0,FALSE);
		srTxnData.srParameter.chOnlineFlag = chOnlineFlag;
		srTxnData.srParameter.chBatchFlag = chBatchFlag;
	}

	//inRetVal = inReaderCommand(inDLLTrans2,&srXMLData,bInData,bOutData,TRUE,0,FALSE);

	//Debug_Mode
	if(srTxnData.srParameter.chDebugMode == '1')
	{
		if(inTxnType == TXN_ADJECT_CARD)
		{
			inRetVal = CALL_DLL_TIMEOUT_ERROR;
			srTxnData.srIngData.fCardActionOKFlag = FALSE;
		}
	}

	if(inTxnType == TXN_REFUND && fDeductFlag)//退貨交易的扣款
	{	//需帶5589,成功帶1,失敗帶9
		char chValue[5];

		memset(chValue,0x00,sizeof(chValue));
		chValue[0] = (inRetVal != CARD_SUCCESS)?'9':'1';
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5589,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,TRUE);
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_558901,chValue,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
		vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5589,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
		inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		if(inRetVal == CARD_SUCCESS)
			srTxnData.srIngData.fICERAdviceFlag = TRUE;/*成功時將此Flag On起來*/
	}
	else if(inTxnType == TXN_ADJECT_CARD && inRetVal == CARD_SUCCESS)/*指定加值成功*/
		srTxnData.srIngData.fICERAdviceFlag = TRUE;//成功時將此Flag On起來

	if(inRetVal != CARD_SUCCESS)//第二道指令失敗即離開
	{
		log_msg(LOG_LEVEL_ERROR,"inECCDeductOrRefund Fail 7:%d",inRetVal);

		if(inTxnType == TXN_ADJECT_CARD && inRetVal == CALL_DLL_TIMEOUT_ERROR)//指定加值
		{
			if((inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0)) < SUCCESS)//沒有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,TRUE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554809,(char *)srTxnData.srIngData.bTMData,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
			}
			else
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554809,(char *)srTxnData.srIngData.bTMData,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
				//inXMLUpdateData(&srXMLData,,(char *)TAG_NAME_TAG_NAME_4200,(char *)TAG_NAME_554809,srTxnData.srIngData.bTMData,FALSE);
			}
		}

		if(inRetVal < SUCCESS)
			return inRetVal;

		/*if(inRetVal < SUCCESS)
			return LOAD_DLL_FUNCTION_ERROR;*/
		return DLL_DECLINE_FAIL;
	}

	//vdUpdateECCDataAfterTxn(inTxnType,bOutData);

	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		srTxnData.srIngData.ucPurseVersionNumber = MIFARE;

		vdUpdateCardEV(srTxnData.srIngData.anCardEV,&bOutData[35],sizeof(srTxnData.srIngData.anCardEV),5,(char *)TAG_NAME_0410);
		if(fDeductFlag)//扣款交易
		{
			if(inTxnType == TXN_REFUND)//退貨之扣款交易要帶在anSETM_STMC_3裡面
				vdBatchDataProcess(srTxnData.srIngData.anSETM_STMC_3,bOutData,SIZE_SETM,(BYTE *)NULL,0);
			else//非退貨之扣款交易要帶在anSETM_STMC_1裡面
			{
				memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,SIZE_SETM);
				memcpy(srTxnData.srIngData.anTAVR_TARC,&bOutData[SIZE_SETM],TAVR_TARC_LEN);
			}
		}
		else//加值交易
		{
			if(inTxnType == TXN_ADJECT_CARD)//指令加值交易要帶在anSETM_STMC_3裡面
			{
				memset(srTxnData.srIngData.anAData,0x00,sizeof(srTxnData.srIngData.anAData));
				vdMakeAData2(MIFARE,bOutData);
				vdBatchDataProcess(srTxnData.srIngData.anSETM_STMC_3,bOutData,148,srTxnData.srIngData.bTMData,TM1_DATA_LEN);
			}
			else//非指令加值交易要帶在anSETM_STMC_1裡面
			{
				memcpy(&srTxnData.srIngData.anSETM_STMC_1[0],bOutData,148);
				memcpy(&srTxnData.srIngData.anSETM_STMC_1[148],srTxnData.srIngData.bTMData,TM1_DATA_LEN);
				inTCPSendAndReceive(inDLLTrans2,&bOutData[148],bRecvBuf,FALSE);
			}
		}

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
		if(inRetVal < SUCCESS)//沒有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,TRUE);
			//指定加值及退貨的扣款交易,要帶554806
			if((inTxnType == TXN_ADJECT_CARD) || (inTxnType == TXN_REFUND && fDeductFlag))
			{
				if(inTxnType == TXN_ADJECT_CARD)//指令加值要多帶554801及554807
				{
					fnUnPack(((AuthAddValueL1_TM_Out *)bOutData)->ucCTAC,8,srTxnData.srIngData.anC_TAC);
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)srTxnData.srIngData.anAData,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554807,(char *)srTxnData.srIngData.anC_TAC,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
				}
				else
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
			}
			else//非指定加值及非退貨的扣款交易,要帶554802
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
				if(fDeductFlag)//扣款要多帶554805
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
			}
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
		else//有5548
		{
			//指定加值及退貨的扣款交易,要帶554806
			if((inTxnType == TXN_ADJECT_CARD) || (inTxnType == TXN_REFUND && fDeductFlag))
			{
				if(inTxnType == TXN_ADJECT_CARD)//指令加值要多帶554801及554807
				{
					fnUnPack(((AuthAddValueL1_TM_Out *)bOutData)->ucCTAC,8,srTxnData.srIngData.anC_TAC);
					//fnUnPack(&bOutData[148 + 45],8,srTxnData.srIngData.anC_TAC);
					//memcpy(srTxnData.srIngData.anC_TAC,&bOutData[148 + 45],8);
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)srTxnData.srIngData.anAData,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554807,(char *)srTxnData.srIngData.anC_TAC,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
				}
				else
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
			}
			else//非指定加值及非退貨的扣款交易,要帶554802
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
				if(fDeductFlag)
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
			}
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
		}
	}
	else //if(srTxnData.srParameter.gDLLVersion == 2)//2代API
	{
		char chPurseVersion = ((AuthAddValueL2_TM_Out *)bOutData)->stSTMC_t.unPurseVersionNumber[1] - 0x30;
		char chTag6404Flag = FALSE;

		srTxnData.srIngData.ucPurseVersionNumber = chPurseVersion;

		//if(chPurseVersion == MIFARE)
		//{
			vdUpdateCardEV(srTxnData.srIngData.anCardEV,((AuthAddValueL2_TM_Out *)bOutData)->stSTMC_t.anEV,sizeof(srTxnData.srIngData.anCardEV),8,(char *)TAG_NAME_0410);
			//vdUpdateCardEV(srTxnData.srIngData.anCardEV,&bOutData[67],sizeof(srTxnData.srIngData.anCardEV),8,(char *)TAG_NAME_0410);
			if(fDeductFlag)//扣款交易
			{
				if(inTxnType == TXN_REFUND)//退貨之扣款交易要帶在anSETM_STMC_3裡面
					vdBatchDataProcess(srTxnData.srIngData.anSETM_STMC_3,bOutData,SIZE_STMC,(BYTE *)NULL,0);
				else//非退貨之扣款交易要帶在anSETM_STMC_1裡面
				{
					memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,SIZE_STMC);
					memcpy(srTxnData.srIngData.anTAVR_TARC,&bOutData[SIZE_STMC],TAVR_TARC_LEN);
				}
			}
			else//加值交易
			{
				if(inTxnType == TXN_ADJECT_CARD)//指令加值交易要帶在anSETM_STMC_3裡面
				{
					/*if(chPurseVersion == MIFARE || chPurseVersion == LEVEL1)
						vdMakeAData2(chPurseVersion,bOutData);
 					if(chPurseVersion == LEVEL1 || chPurseVersion == LEVEL2)*/
					vdMakeAData2(chPurseVersion,bOutData);
					vdBatchDataProcess(srTxnData.srIngData.anSETM_STMC_3,bOutData,SIZE_STMC,srTxnData.srIngData.bTMData,TM2_DATA_LEN);
				}
				else//非指令加值交易要帶在anSETM_STMC_1裡面
				{
					memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,SIZE_STMC);
					inTCPSendAndReceive(inDLLTrans2,&bOutData[SIZE_STMC],bRecvBuf,FALSE);
				}
			}

			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
			if(inRetVal < SUCCESS)//沒有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,TRUE);
				//指定加值及退貨的扣款交易,要帶554806
				if((inTxnType == TXN_ADJECT_CARD) || (inTxnType == TXN_REFUND && fDeductFlag))
				{
					if(inTxnType == TXN_ADJECT_CARD)//指令加值要多帶554801及554807
					{
						if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)//要用NODE_NO_ADD
						{
							fnUnPack(((AuthAddValueL2_TM_Out *)bOutData)->ucCTAC,8,srTxnData.srIngData.anC_TAC);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)srTxnData.srIngData.anAData,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554807,(char *)srTxnData.srIngData.anC_TAC,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
						}
						else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)//用PVN來決定要用NODE_NO_SAME或NODE_NO_ADD
						{
							fnUnPack(((AuthAddValueL2_TM_Out *)bOutData)->ucCPUMAC,16,srTxnData.srIngData.anC_TAC2);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554808,(char *)srTxnData.srIngData.anAData2,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
							//vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_6404,(char *)srTxnData.srIngData.anC_TAC2,FALSE,NODE_NO_SAME,VALUE_NAME);
							chTag6404Flag = TRUE;
						}
					}
					else
						vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
				}
				else//非指定加值及非退貨的扣款交易,要帶554802
				{
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,FALSE,NODE_NO_ADD,VALUE_NAME,TRUE);
					if(fDeductFlag)//扣款要多帶554805
						vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
				}
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,TRUE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
				if(chTag6404Flag == TRUE)
					inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_6404,srTxnData.srIngData.anC_TAC2,FALSE);
			}
			else//有5548
			{
				//指定加值及退貨的扣款交易,要帶554806
				if((inTxnType == TXN_ADJECT_CARD) || (inTxnType == TXN_REFUND && fDeductFlag))
				{
					if(inTxnType == TXN_ADJECT_CARD)//指令加值要多帶554801及554807
					{
						if(srTxnData.srIngData.ucPurseVersionNumber == MIFARE)
						{
							fnUnPack(((AuthAddValueL2_TM_Out *)bOutData)->ucCTAC,8,srTxnData.srIngData.anC_TAC);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)srTxnData.srIngData.anAData,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554807,(char *)srTxnData.srIngData.anC_TAC,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
						}
						else //if(srTxnData.srIngData.ucPurseVersionNumber == LEVEL2)
						{
							fnUnPack(((AuthAddValueL2_TM_Out *)bOutData)->ucCPUMAC,16,srTxnData.srIngData.anC_TAC2);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554808,(char *)srTxnData.srIngData.anAData2,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
							//vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_6404,(char *)srTxnData.srIngData.anC_TAC2,FALSE,NODE_NO_SAME,VALUE_NAME);
							chTag6404Flag = TRUE;
						}
					}
					else
						vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554806,(char *)srTxnData.srIngData.anSETM_STMC_3,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
				}
				else//非指定加值及非退貨的扣款交易,要帶554802
				{
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,TRUE,NODE_NO_SAME,VALUE_NAME,TRUE);
					if(fDeductFlag)
						vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,TRUE);
				}
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
				if(chTag6404Flag == TRUE)
					inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_6404,srTxnData.srIngData.anC_TAC2,FALSE);
			}


			//T5362
			vdUnPackToXMLData(ucSAMSN,sizeof(ucSAMSN),&srXMLData,(char *)"T5362",strlen("T5362"),VALUE_NAME,TRUE);
			//T6000
			vdUnPackToXMLData(ucReaderFWVersion,sizeof(ucReaderFWVersion),&srXMLData,(char *)"T6000",strlen("T6000"),VALUE_NAME,TRUE);
		//}
	}

	return SUCCESS;
}

BOOL fIsAutoloadProcess(BYTE *bAPIOut,long *lnOutALAmt)
{
BYTE bBuf[10],bAutoLoadFlag[5],bAutoLoadAmt[10];
int shAutoloadAmt = 0,shTxnAmt = 0,shEV = 0;

	memset(bAutoLoadFlag,0x00,sizeof(bAutoLoadFlag));
	memset(bAutoLoadAmt,0x00,sizeof(bAutoLoadAmt));
	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		DeductValueL1_TM_Out_9000 *APIOut = (DeductValueL1_TM_Out_9000 *)bAPIOut;

		memcpy(bAutoLoadFlag,APIOut->unAutoPayFlag,strlen((char *)APIOut->unAutoPayFlag));
		memcpy(bAutoLoadAmt,APIOut->unAutoPayAmt,sizeof(APIOut->unAutoPayAmt));
		if(APIOut->unAutoPayFlag[2] != '1')
			return FALSE;

		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,APIOut->unAutoPayAmt,sizeof(APIOut->unAutoPayAmt));
		shAutoloadAmt = atoi((char *)bBuf);

		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,APIOut->unTxnAmt,sizeof(APIOut->unTxnAmt));
		shTxnAmt = atoi((char *)bBuf);

		memset(bBuf,0x00,sizeof(bBuf));
		memcpy(bBuf,APIOut->unEVBeforeTxn,sizeof(APIOut->unEVBeforeTxn));
		shEV = atoi((char *)bBuf);

		//memset(bAutoLoadAmt,0x00,sizeof(bAutoLoadAmt));
		if(srTxnData.srParameter.chAutoLoadMode == '1' || srTxnData.srParameter.chAutoLoadMode == '2')
			shAutoloadAmt = lnConvertAutoloadAmt(shTxnAmt,APIOut->unAutoPayAmt,sizeof(APIOut->unAutoPayAmt),APIOut->unEVBeforeTxn,sizeof(APIOut->unEVBeforeTxn),bAutoLoadAmt);
	}
	else if(srTxnData.srParameter.gDLLVersion == 2)//2代API
	{
		if(srTxnData.srParameter.gReaderMode == DLL_READER || srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
		{
			DeductValueL2_TM_Out_9000 *APIOut = (DeductValueL2_TM_Out_9000 *)bAPIOut;

			memset(bBuf,0x00,sizeof(bBuf));
			sprintf((char *)bAutoLoadFlag,"%02X",APIOut->ucAutoLoad);
			memcpy(bAutoLoadAmt,APIOut->anAutoLoadAmt,sizeof(APIOut->anAutoLoadAmt));
			if(APIOut->ucAutoLoad != 0x01)
			{
				log_msg(LOG_LEVEL_ERROR,"fIsAutoloadProcess 1 FALSE");
				return FALSE;
			}

			memset(bBuf,0x00,sizeof(bBuf));
			memcpy(bBuf,APIOut->anAutoLoadAmt,sizeof(APIOut->anAutoLoadAmt));
			shAutoloadAmt = atoi((char *)bBuf);

			memset(bBuf,0x00,sizeof(bBuf));
			memcpy(bBuf,APIOut->anTxnAmt,sizeof(APIOut->anTxnAmt));
			shTxnAmt = atoi((char *)bBuf);

			memset(bBuf,0x00,sizeof(bBuf));
			memcpy(bBuf,APIOut->anEVBeforeTxn,sizeof(APIOut->anEVBeforeTxn));
			shEV = atoi((char *)bBuf);

			//memset(bAutoLoadAmt,0x00,sizeof(bAutoLoadAmt));
			if(srTxnData.srParameter.chAutoLoadMode == '1' || srTxnData.srParameter.chAutoLoadMode == '2')
				shAutoloadAmt = lnConvertAutoloadAmt(shTxnAmt,APIOut->anAutoLoadAmt,sizeof(APIOut->anAutoLoadAmt),APIOut->anEVBeforeTxn,sizeof(APIOut->anEVBeforeTxn),bAutoLoadAmt);
		}
		else //if(srTxnData.srParameter.gReaderMode == RS232_READER)
		{
			BYTE anAutoLoadAmt[8],anEVBeforeTxn[8];
			BYTE bAutoLoad;
			BYTE ucAutoLoadAmt[3];
			BYTE ucEV[3];
			BYTE ucTxnAmt[3];
			if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_TAXI)
			{
				TaxiRead_APDU_Out *APIOut = (TaxiRead_APDU_Out *)bAPIOut;
				bAutoLoad = APIOut->bAutoLoad;
				memcpy(ucAutoLoadAmt,APIOut->ucAutoLoadAmt,sizeof(ucAutoLoadAmt));
				memcpy(ucEV,APIOut->ucEV,sizeof(ucEV));
				memcpy(ucTxnAmt,(char *)&srTxnData.srIngData.lnECCAmt,sizeof(ucTxnAmt));
			}
			else if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_EDCA)
			{
				EDCARead_APDU_Out *APIOut = (EDCARead_APDU_Out *)bAPIOut;
				bAutoLoad = APIOut->bAutoLoad;
				memcpy(ucAutoLoadAmt,APIOut->ucAutoLoadAmt,sizeof(ucAutoLoadAmt));
				memcpy(ucEV,APIOut->ucEV,sizeof(ucEV));
				memcpy(ucTxnAmt,(char *)&srTxnData.srIngData.lnECCAmt,sizeof(ucTxnAmt));
			}
			else
			{
				TxnReqOffline_APDU_Out *APIOut = (TxnReqOffline_APDU_Out *)bAPIOut;
				bAutoLoad = APIOut->bAutoLoad;
				memcpy(ucAutoLoadAmt,APIOut->ucAutoLoadAmt,sizeof(ucAutoLoadAmt));
				memcpy(ucEV,APIOut->ucEV,sizeof(ucEV));
				memcpy(ucTxnAmt,APIOut->ucTxnAmt,sizeof(ucTxnAmt));
			}

			memset(bBuf,0x00,sizeof(bBuf));
			memset(anAutoLoadAmt,0x00,sizeof(anAutoLoadAmt));
			memset(anEVBeforeTxn,0x00,sizeof(anEVBeforeTxn));
			sprintf((char *)bAutoLoadFlag,"%02X",bAutoLoad);
			vdUIntToAsc(ucAutoLoadAmt,sizeof(ucAutoLoadAmt),bAutoLoadAmt,sizeof(bAutoLoadAmt),FALSE,' ',10);
			if(bAutoLoad != 0x01)
			{
				log_msg(LOG_LEVEL_ERROR,"fIsAutoloadProcess 2 FALSE");
				return FALSE;
			}

			memcpy((char *)&shAutoloadAmt,ucAutoLoadAmt,sizeof(ucAutoLoadAmt));
	#ifdef ECC_BIG_ENDIAN
			vdChangeEndian((BYTE *)&shAutoloadAmt,(BYTE *)&shAutoloadAmt,sizeof(shAutoloadAmt));
	#endif

			memcpy((char *)&shTxnAmt,ucTxnAmt,sizeof(ucTxnAmt));
	#ifdef ECC_BIG_ENDIAN
			vdChangeEndian((BYTE *)&shTxnAmt,(BYTE *)&shTxnAmt,sizeof(shTxnAmt));
	#endif
			shEV = lnAmt3ByteToLong(ucEV);
			//memcpy((char *)&shEV,APIOut->ucEV,sizeof(APIOut->ucEV));

			vdUIntToAsc(ucAutoLoadAmt,sizeof(ucAutoLoadAmt),anAutoLoadAmt,sizeof(anAutoLoadAmt),FALSE,' ',10);
			vdIntToAsc(ucEV,sizeof(ucEV),anEVBeforeTxn,sizeof(anEVBeforeTxn),FALSE,' ',10);
			//memset(bAutoLoadAmt,0x00,sizeof(bAutoLoadAmt));
			log_msg(LOG_LEVEL_FLOW,"[%02x],shTxnAmt = %d,shAutoloadAmt = %d,shEV = %d,ucEV = %02x%02x%02x,%d",
							&srTxnData.srParameter.chAutoLoadMode,shTxnAmt,shAutoloadAmt,shEV,ucEV[0],ucEV[1],ucEV[2],sizeof(long));
			log_msg_debug(LOG_LEVEL_FLOW,TRUE,(char *)"shEV",4,(BYTE *)&shEV,sizeof(int));
			if(srTxnData.srParameter.chAutoLoadMode == '1' || srTxnData.srParameter.chAutoLoadMode == '2')
				shAutoloadAmt = lnConvertAutoloadAmt(shTxnAmt,anAutoLoadAmt,sizeof(anAutoLoadAmt),anEVBeforeTxn,sizeof(anEVBeforeTxn),bAutoLoadAmt);
		}
	}

	log_msg(LOG_LEVEL_FLOW,"fIsAutoloadProcess 3 %d ,%d ,%d ,%d" ,shTxnAmt,shEV,shAutoloadAmt,shEV + shAutoloadAmt);
	log_msg(LOG_LEVEL_FLOW,"fIsAutoloadProcess 4 %ld,%ld,%ld,%ld",shTxnAmt,shEV,shAutoloadAmt,shEV + shAutoloadAmt);
	if(shTxnAmt > (shEV + shAutoloadAmt))
	{
		log_msg(LOG_LEVEL_ERROR,"fIsAutoloadProcess FALSE %d > %d + %d",shTxnAmt,shEV,shAutoloadAmt);
		return FALSE;
	}

	memset(srTxnData.srIngData.anAutoloadFlag,0x00,sizeof(srTxnData.srIngData.anAutoloadFlag));
	memset(srTxnData.srIngData.anAutoloadAmount,0x00,sizeof(srTxnData.srIngData.anAutoloadAmount));
	srTxnData.srIngData.anAutoloadFlag[0] = bAutoLoadFlag[strlen((char *)bAutoLoadFlag) - 1];
	sprintf((char *)srTxnData.srIngData.anAutoloadAmount,"%d",shAutoloadAmt);
	//vdTrimData((char *)srTxnData.srIngData.anAutoloadAmount,(char *)bAutoLoadAmt,strlen((char *)bAutoLoadAmt));
	strcat((char *)srTxnData.srIngData.anAutoloadAmount,"00");
	*lnOutALAmt = shAutoloadAmt;
	log_msg(LOG_LEVEL_FLOW,"srTxnData.srIngData.anAutoloadAmount 1 = %s,*lnOutALAmt = %d",
	srTxnData.srIngData.anAutoloadAmount,*lnOutALAmt);

	return TRUE;
}

int inAutoloadProcess(BYTE *bAPIOut)
{
//DeductValueL1_TM_Out_9000 *APIOut = (DeductValueL1_TM_Out_9000 *)bAPIOut;
BYTE bInData[READER_BUFFER],bOutData[READER_BUFFER],bRecvBuf[READER_BUFFER],bTMData[50 + 1],bBuf[10],bReadPurseFlag[2];
int inRetVal,inCnt = 0,inSize,inDLLTrans1;
long lnTxnAmt = 0L;
long  lnforeign_currency_ALAmt = 0L;
STRUCT_XML_DOC srTmpXMLData;
unsigned char ucT4108;

	if(fIsAutoloadProcess(bAPIOut,&lnTxnAmt) == FALSE)
		return AMOUNT_FAIL;
	
	if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
		vdXMLBackUpData(&srXMLData,&srTmpXMLData,FALSE);

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	memset(bTMData,0x00,sizeof(bTMData));
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bReadPurseFlag,0x00,sizeof(bReadPurseFlag));

	inSize = 2;
	memcpy(&bInData[inCnt],"02",inSize);
	inCnt += inSize;

	inSize = 2;
	memcpy(&bInData[inCnt],"40",inSize);
	inCnt += inSize;

	inCnt += inBuildAPIDefaultData(DLL_AUTOLOAD,srTxnData.srIngData.ulTMSerialNumber,&bInData[inCnt],&inRetVal);//TM序號不加1,以免影響Retry機制!!
	//inCnt += inBuildAPIDefaultData(DLL_AUTOLOAD,srTxnData.srIngData.ulTMSerialNumber++,&bInData[inCnt]);

	/*(srTxnData.srParameter.gDLLVersion == 1)//1代API
		memcpy(bBuf,((DeductValueL1_TM_Out_9000 *)bAPIOut)->unAutoPayAmt,5);
	else //if(srTxnData.srParameter.gDLLVersion == 2)//2代API
		memcpy(bBuf,((DeductValueL2_TM_Out_9000 *)bAPIOut)->anAutoLoadAmt,8);
	lnTxnAmt = atol((char *)bBuf);*/
	inCnt += inBuildTxnAmt(&bInData[inCnt],lnTxnAmt);
	//inSize = 5;
	//vdPad((char *)APIOut->unAutoPayAmt,(char *)&bInData[inCnt],inSize,FALSE,'0');
	//inCnt += inSize;

	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4830,(char *)bReadPurseFlag,1,0);
	if(inRetVal >= SUCCESS)//沒有4830
	{
		if(bReadPurseFlag[0] == '1')
			bReadPurseFlag[0] = 0x01;
		else
			bReadPurseFlag[0] = 0x00;
	}
	else
		bReadPurseFlag[0] = 0x00;
	bInData[inCnt++] = bReadPurseFlag[0];

	memcpy(bTMData,&bInData[4],(srTxnData.srParameter.gDLLVersion == 1)?TM1_DATA_LEN:TM2_DATA_LEN);

	inRetVal = inReaderCommand(DLL_AUTOLOAD, &srXMLData,bInData, bOutData, FALSE, 0, FALSE);
	//非srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY模式時,有可能沒做SignOn,要補做!!
	if(inRetVal == 0x6304 || inRetVal == 0x6305 || inRetVal == 0x630D)
	{
		if(inRetVal == 0x6304 || inRetVal == 0x6305)
			inRetVal = inSignOnProcess(TXN_ECC_SIGN_ON,bOutData,TRUE);
		else //if(inRetVal == 0x630D)
			inRetVal = inSignOnProcess(TXN_ECC_MATCH,bOutData,TRUE);
		if(inRetVal != CARD_SUCCESS)
		{
			if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
				vdFreeXMLDOC(&srTmpXMLData);

			log_msg(LOG_LEVEL_ERROR,"inAutoloadProcess Fail 1-0:%d",inRetVal);
			return inRetVal;
		}

		//再執行一次扣款第一道指令
		inRetVal = inReaderCommand(DLL_AUTOLOAD,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
	}
	else if(inRetVal != CARD_SUCCESS && inRetVal != 0x6415)
	{
		if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
			vdFreeXMLDOC(&srTmpXMLData);

		log_msg(LOG_LEVEL_ERROR,"inAutoloadProcess Fail 1:%d",inRetVal);
		if(inRetVal < SUCCESS)
			return inRetVal;
		return DLL_DECLINE_FAIL;
		//return inRetVal;
	}
	else
	{//保留自動加值資料,以備扣款失敗時使用.
		TxnReqOnline_APDU_Out *DongleOut = (TxnReqOnline_APDU_Out *)&bOutData;

		srTxnData.srAutoloadData.ucPurseVersionNumber = DongleOut->ucPurseVersionNumber;
		memcpy(srTxnData.srAutoloadData.ucCardID,DongleOut->ucCardID,sizeof(DongleOut->ucCardID));
		memcpy(srTxnData.srAutoloadData.ucPID,DongleOut->ucPID,sizeof(DongleOut->ucPID));
		srTxnData.srAutoloadData.ucCardType = DongleOut->ucCardType;
		srTxnData.srAutoloadData.ucPersonalProfile = DongleOut->ucPersonalProfile;
		memcpy(srTxnData.srAutoloadData.ucExpiryDate,DongleOut->ucExpiryDate,sizeof(DongleOut->ucExpiryDate));
		memcpy(srTxnData.srAutoloadData.anRRN,srTxnData.srIngData.anRRN,sizeof(srTxnData.srIngData.anRRN));
		memcpy(srTxnData.srAutoloadData.ucDeviceID,DongleOut->ucDeviceID,sizeof(DongleOut->ucDeviceID));
		memcpy(srTxnData.srAutoloadData.ucCPUDeviceID,DongleOut->ucCPUDeviceID,sizeof(DongleOut->ucCPUDeviceID));
		// 保留自動加值資料，以備重試交易時使用
		vdStoreAutoloadData(bOutData, sizeof(TxnReqOnline_APDU_Out));
	}

	if(inRetVal != CARD_SUCCESS || srTxnData.srParameter.gDLLVersion != 2)
	{
		memset(bRecvBuf,0x00,sizeof(bRecvBuf));
		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		{
			//autoload金額換算外幣幣值
			if(srTxnData.srIngData.fForeignTxnFlag == TRUE)
			{
				lnforeign_currency_ALAmt = lnTxnAmt * srTxnData.srIngData.lnForeignAmt / srTxnData.srIngData.lnECCAmt;
				sprintf((char *)bBuf,"%ld", lnforeign_currency_ALAmt);
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)"T0421", bBuf,TRUE);
			}
			inRetVal = inSendRecvXML(HOST_CMAS,&srXMLData,TXN_ECC_AUTOLOAD);
		}
		else //if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
		{
			//if(!memcmp(&bOutData[7],"845799",6))//社福卡自動加值,因R6回覆資料長度不同,需增加此判斷!!
			//	inDLLTrans1 = DLL_ADD_VALUE;
			//else
				inDLLTrans1 = DLL_AUTOLOAD;

			inRetVal = inTCPSendAndReceive(inDLLTrans1,bOutData,bRecvBuf,TRUE);
		}
		if(inRetVal != SUCCESS)
		{
			if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
			{
				if(inRetVal == ECC_LOCK_CARD)
				{
					vdFreeXMLDOC(&srXMLData);
					vdXMLBackUpData(&srTmpXMLData,&srXMLData,TRUE);
				}
				vdFreeXMLDOC(&srTmpXMLData);
			}

			log_msg(LOG_LEVEL_ERROR,"inAutoloadProcess Fail 2:%d",inRetVal);
			if(inRetVal < SUCCESS)
				return inRetVal;
			return DLL_DECLINE_FAIL;
			//return inRetVal;
		}
		memset(bInData,0x00,sizeof(bInData));
		if(srTxnData.srParameter.gDLLVersion == 1)//1代API
		{
			memcpy(&bInData[0],&bRecvBuf[39],4);
			memcpy(&bInData[4],&bRecvBuf[75],8);
			//bInData[8] = 0x00;
		}
		else
			memcpy(&bInData[0],&bRecvBuf[10],sizeof(bRecvBuf) - 10);
	}
	else
		memset(bInData,0x00,sizeof(bInData));

	if(srTxnData.srParameter.gDLLVersion == 2)//2代API
		bInData[119] = 0x01;

	ucT4108 = srTxnData.srREQData.ucT4108;
	srTxnData.srREQData.ucT4108 = '1';

	memset(bOutData,0x00,sizeof(bOutData));
	inRetVal = inReaderCommand(DLL_AUTOLOAD_AUTH,&srXMLData,bInData,bOutData,TRUE,0,FALSE);
	srTxnData.srREQData.ucT4108 = ucT4108;
	if(inRetVal != CARD_SUCCESS)
	{
		if(srTxnData.srIngData.fOnlineHostSuccFlag == TRUE && srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
			vdReversalOn(HOST_CMAS);

		if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
			vdFreeXMLDOC(&srTmpXMLData);

		log_msg(LOG_LEVEL_ERROR,"inAutoloadProcess Fail 3:%d",inRetVal);
		if (inRetVal == CALL_DLL_TIMEOUT_ERROR)	// Autoload 若需Retry時，ICERAPI.TMP加入註記
		{
			gTmpData.ucAutoload_fail_flag = TRUE;
		}
		log_msg(LOG_LEVEL_ERROR,"ucAutoload_fail_flag 1= %d", gTmpData.ucAutoload_fail_flag);
		if(inRetVal < SUCCESS)
			return inRetVal;
		return DLL_DECLINE_FAIL;
		//return inRetVal;
	}
	else
	{
		AuthTxnOnline_APDU_Out *DongleOut = (AuthTxnOnline_APDU_Out *)&bOutData;

		memcpy(srTxnData.srAutoloadData.ucEV,DongleOut->ucEV,sizeof(DongleOut->ucEV));
	}

	if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
	{
		BYTE bBuffer[MAX_XML_FILE_SIZE];

		//autoload金額換算外幣幣值
		if(srTxnData.srIngData.fForeignTxnFlag == TRUE)
		{
			sprintf((char *)bBuf,"%ld", lnforeign_currency_ALAmt);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)"T0421", bBuf,TRUE);
		}

		if(inBuildSendPackage(HOST_CMAS,&srXMLData,bBuffer,REVERSAL_OFF,ADVICE_ON,STORE_BATCH_ON) < SUCCESS)
			log_msg(LOG_LEVEL_ERROR,"inAutoloadProcess Fail 4");
		else
			inTCPIPAdviceProcess(HOST_CMAS,FALSE);

		/*vdXMLChangeTagName(&srXMLData,(char *)TAG_NAME_0408,(char *)TAG_NAME_0410,(char *)"00");
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_0200);
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_0211);
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_0213);
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_0214);
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_0215);
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_0410);
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_1402);
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_3700);
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_4109);
		inXMLCopyXMLDataByTag(&srXMLData,&srTmpXMLData,TAG_NAME_4110);*/

		vdFreeXMLDOC(&srXMLData);
		vdXMLBackUpData(&srTmpXMLData,&srXMLData,TRUE);
		vdFreeXMLDOC(&srTmpXMLData);

		vdIcnBankData(6);//6:加/扣成功才+1
		srTxnData.srIngData.fOnlineHostSuccFlag = FALSE;
		return SUCCESS;
	}
	else if(srTxnData.srParameter.chBatchFlag == SIS2_BATCH)
	{
		memset(srTxnData.srIngData.anSETM_STMC_2,0x00,sizeof(srTxnData.srIngData.anSETM_STMC_2));
		fnUnPack(bOutData, sizeof(SIS2_HEADER) + sizeof(SIS2_END) + sizeof(SIS2_BODY_ADD),srTxnData.srIngData.anSETM_STMC_2);
		inSIS2SaveBlob();
	}

	inTCPSendAndReceive(DLL_AUTOLOAD_AUTH,&bOutData[SIZE_STMC],bRecvBuf,FALSE);

	vdUpdateECCDataAfterTxn(TXN_ECC_ADD,bOutData);
	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		vdUpdateCardEV(srTxnData.srIngData.anCardEV,&bOutData[35],sizeof(srTxnData.srIngData.anCardEV),5,(char *)TAG_NAME_0410);
		memcpy(srTxnData.srIngData.anSETM_STMC_2,bOutData,148);
		memcpy(&srTxnData.srIngData.anSETM_STMC_2[148],bTMData,TM1_DATA_LEN);
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bTMData,0,0);
		if(inRetVal < SUCCESS)
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
		else
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
		}
	}
	else
	{
		vdUpdateCardEV(srTxnData.srIngData.anCardEV,&bOutData[67],sizeof(srTxnData.srIngData.anCardEV),8,(char *)TAG_NAME_0410);
		memcpy(srTxnData.srIngData.anSETM_STMC_2,bOutData,SIZE_STMC);
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bTMData,0,0);
		if(inRetVal < SUCCESS)
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
			vdGetECCEVBefore(((STMC_t *)srTxnData.srIngData.anSETM_STMC_2)->anEVBeforeTxn,8);
		}
		else
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
		}
	}

	return SUCCESS;
}

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
int inGetTMOutLen(int inDLLRet,STRUCT_READER_INFO *srReaderInfo,int inDLLType)
#else//READER_MANUFACTURERS==LINUX_API
int inGetTMOutLen(int inDLLRet,STRUCT_READER_COMMAND *srReaderInfo,int inDLLType)
#endif
{
int inOutLen = 0;

	switch(inDLLRet)
	{
		case 0x6304:
			if(srTxnData.srParameter.gDLLVersion == 1 && inDLLType == DLL_SIGNONQUERY)
				inOutLen = 0;
			else
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
				inOutLen = srReaderInfo->inTMOutLen9000;
#else//READER_MANUFACTURERS==LINUX_API
				inOutLen = srReaderInfo[inDLLType].inTMOutLen9000;
#endif
			break;
		case 0x9000:
		case 0x6305:
		case 0x6308:
		case 0x6403:
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			inOutLen = srReaderInfo->inTMOutLen9000;
#else//READER_MANUFACTURERS==LINUX_API
			inOutLen = srReaderInfo[inDLLType].inTMOutLen9000;
#endif
			break;
		case 0x6415:
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			inOutLen = srReaderInfo->inTMOutLen6415;
#else//READER_MANUFACTURERS==LINUX_API
			inOutLen = srReaderInfo[inDLLType].inTMOutLen6415;
#endif
			break;
		case 0x6103:
		case 0x610F:
		case 0x6406:
		case 0x640E:
		case 0x6418:
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
			inOutLen = srReaderInfo->inTMOutLen6103;
#else//READER_MANUFACTURERS==LINUX_API
			inOutLen = srReaderInfo[inDLLType].inTMOutLen6103;
#endif
			break;
		default:
			break;
	}

	return inOutLen;
}

char *chGetDLLName(int inDLLType)
{

	if(srTxnData.srParameter.gReaderMode == RS232_READER)
		return ((char *)st_ReaderCommand2[inDLLType].srReaderInfoL2.bReaderCommand);
	else
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	//if(srTxnData.srParameter.gReaderMode == DLL_READER)
	{
		if(srTxnData.srParameter.gDLLVersion == 1)
			return ((char *)st_ReaderCommand[inDLLType].srReaderInfoL1.bReaderCommand);
		else
			return ((char *)st_ReaderCommand[inDLLType].srReaderInfoL2.bReaderCommand);
	}
#else
	if(srTxnData.srParameter.gReaderMode == DLL_READER)
		return ((char *)st_ReaderCommand[inDLLType].bReaderCommand);
	else
		return ((char *)st_ReaderCommand2[inDLLType].srReaderInfoL2.bReaderCommand);
#endif

}

void vdMakeAData2(char chPurseVersion,BYTE *bInData)
{
int inCnt = 0,inLen = 0;

	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		AuthAddValueL1_TM_Out *TM_Out = (AuthAddValueL1_TM_Out *)bInData;
		BYTE *bOutData = srTxnData.srIngData.anAData;

		inCnt = 0;
		memset(srTxnData.srIngData.anAData,0x00,sizeof(srTxnData.srIngData.anAData));

		inLen = sizeof(TM_Out->ucTxnDateTime);
		fnUnPack(TM_Out->ucTxnDateTime,inLen,&bOutData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(TM_Out->stAddValueSETM_t.unCardID_8);
		memcpy(&bOutData[inCnt],TM_Out->stAddValueSETM_t.unCardID_8,inLen);
		inCnt += inLen;

		inLen = sizeof(TM_Out->ucTxnSN_2);
		fnUnPack(TM_Out->ucTxnSN_2,inLen,&bOutData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(TM_Out->ucEV_2);
		fnUnPack(TM_Out->ucEV_2,inLen,&bOutData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(TM_Out->ucMAC);
		fnUnPack(TM_Out->ucMAC,inLen,&bOutData[inCnt]);
		inCnt += inLen * 2;

		inLen = 2;
		sprintf((char *)&bOutData[inCnt],"01");
		inCnt += inLen;

		inLen = 8;
		fnUnPack(srTxnData.srIngData.anSAMID,inLen,&bOutData[inCnt]);
		inCnt += inLen * 2;

		inLen = sizeof(TM_Out->ucConfirmCode);
		fnUnPack(TM_Out->ucConfirmCode,inLen,&bOutData[inCnt]);
		inCnt += inLen * 2;
	}
	else
	{
		AuthAddValueL2_TM_Out *TM_Out = (AuthAddValueL2_TM_Out *)bInData;
		if(chPurseVersion == MIFARE/* || chPurseVersion == LEVEL1*/)
		{
			BYTE *bOutData = srTxnData.srIngData.anAData;

			inCnt = 0;
			memset(srTxnData.srIngData.anAData,0x00,sizeof(srTxnData.srIngData.anAData));

			inLen = sizeof(TM_Out->ucTxnDateTime);
			fnUnPack(TM_Out->ucTxnDateTime,inLen,&bOutData[inCnt]);
			inCnt += inLen * 2;

			inLen = 8;
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unCardID,inLen);
			inCnt += inLen;

			inLen = 2;
			fnUnPack(TM_Out->ucTxnSN,inLen,&bOutData[inCnt]);
			inCnt += inLen * 2;

			inLen = 2;
			fnUnPack(TM_Out->ucEV,inLen,&bOutData[inCnt]);
			inCnt += inLen * 2;

			inLen = sizeof(TM_Out->ucMAC);
			fnUnPack(TM_Out->ucMAC,inLen,&bOutData[inCnt]);
			inCnt += inLen * 2;

			inLen = 2;
			sprintf((char *)&bOutData[inCnt],"01");
			inCnt += inLen;

			inLen = 8;
			fnUnPack(srTxnData.srIngData.anSAMID,inLen,&bOutData[inCnt]);
			inCnt += inLen * 2;

			inLen = sizeof(TM_Out->ucConfirmCode);
			fnUnPack(TM_Out->ucConfirmCode,inLen,&bOutData[inCnt]);
			inCnt += inLen * 2;

			//新增MAC欄位資料
			inLen = sizeof(TM_Out->stSTMC_t.unMsgType);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unMsgType,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unSubType);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unSubType,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unDeviceID) - 2;
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unDeviceID,inLen);
			inCnt += inLen;

			inLen = 2;
			memcpy(&bOutData[inCnt],&TM_Out->stSTMC_t.unDeviceID[4],inLen);
			//sprintf((char *)&bOutData[inCnt],"%02X",&TM_Out->stSTMC_t.unDeviceID[2]);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unIssuerCode);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unIssuerCode,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unTxnAmt) - 2;
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unTxnAmt,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unLocationID);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unLocationID,inLen);
			inCnt += inLen;

			inLen = 2;
			memcpy(&bOutData[inCnt],&TM_Out->stSTMC_t.unDeviceID[4],inLen);
			//sprintf((char *)&bOutData[inCnt],"%02X",&TM_Out->stSTMC_t.unDeviceID[2]);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unBankCode);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unBankCode,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unLoyaltyCounter);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unLoyaltyCounter,inLen);
			inCnt += inLen;

		}
		else //if(chPurseVersion == LEVEL1 || chPurseVersion == LEVEL2)
		{
			BYTE *bOutData = srTxnData.srIngData.anAData2;

			inCnt = 0;
			memset(srTxnData.srIngData.anAData2,0x00,sizeof(srTxnData.srIngData.anAData2));

			inLen = sizeof(TM_Out->stSTMC_t.unHostAdminKVN);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unHostAdminKVN,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unCPUSAMID);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unCPUSAMID,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unHashType);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unHashType,inLen);
			inCnt += inLen;

			//inLen = 4;
			//memcpy(&bOutData[inCnt],"0094",inLen);
			//inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unMsgType);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unMsgType,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unSubType);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unSubType,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unTxnDateTime);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unTxnDateTime,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unCardID);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unCardID,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unIssuerCode);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unIssuerCode,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unTxnSN);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unTxnSN,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unTxnAmt);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unTxnAmt,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unEV);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unEV,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unAreaCode);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unAreaCode,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unCPUDeviceID);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unCPUDeviceID,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unCPUSPID);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unCPUSPID,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unCPULocationID);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unCPULocationID,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unPersonalProfile);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unPersonalProfile,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unPID);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unPID,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unPurseVersionNumber);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unPurseVersionNumber,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unSubAreaCode);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unSubAreaCode,inLen);
			inCnt += inLen;

			//新增MAC欄位資料
			inLen = sizeof(TM_Out->stSTMC_t.unCTC);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unCTC,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unTxnQuqlifier);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unTxnQuqlifier,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unSignatureKeyKVN);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unSignatureKeyKVN,inLen);
			inCnt += inLen;

			inLen = sizeof(TM_Out->stSTMC_t.unSignature);
			memcpy(&bOutData[inCnt],TM_Out->stSTMC_t.unSignature,inLen);
			inCnt += inLen;
		}
	}
}

int inBuildTxnAmt(BYTE *bOutData,long lnTxnAmt)
{

	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		sprintf((char *)bOutData,"%05ld",lnTxnAmt);
		return 5;
	}
	else
	{
		sprintf((char *)bOutData,"%08ld",lnTxnAmt);
		return 8;
	}
}

int inDoECCAmtTxn(int inTxnType)
{
	BYTE bInData[READER_BUFFER],bOutData[READER_BUFFER],bRecvBuf[READER_BUFFER],bBuf[10],bReadPurseFlag[2]/*,bTMData[32 + 1]*/;
	int inRetVal,inCnt = 0,inSize;
	int inDLLTrans1 = 0,inDLLTrans2 = 0;
	int inTMDataLen;
	long lnTxnAmt = 0L;
	STRUCT_XML_DOC srTmpXMLData;
	char chOnlineFlag;//Online Flag,0=R6 / 1=CMAS
	char chBatchFlag;//Batch Flag,0=CMAS Batch / 1=SIS2 Batch / 2=R6
	char chTCPIP_SSL;
	char PacketLenFlag;//PacketLenFlag 0=電文沒有2Byte長度 / 1=電文有2Byte長度

	log_msg(LOG_LEVEL_FLOW,"ucAutoload_fail_flag 4= %d", gTmpData.ucAutoload_fail_flag);
	if (gTmpData.ucAutoload_fail_flag && srTxnData.srIngData.fTMRetryFlag)	//是重試且上次在Autoload時出現問題，先進行Autoload的retry
	{
		// 從tmp檔中取出上一筆交易, V4003K
		inGetAutoloadData(bOutData);
		inRetVal = inAutoloadProcess(bOutData);
		if(inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inECCDeductOrRefund Fail (AutoloadRetry):%d",inRetVal);
			//return AMOUNT_FAIL;
		}
		gTmpData.ucAutoload_fail_flag = FALSE;
	}
		
	if(inTxnType != TXN_ECC_TX_ADD)//餘加
		lnTxnAmt = srTxnData.srIngData.lnECCAmt;
	if(inTxnType == TXN_ECC_AUTOLOAD)
		sprintf((char *)srTxnData.srIngData.anAutoloadAmount,"%ld00",srTxnData.srIngData.lnECCAmt);

	if(inTxnType == TXN_ADJECT_CARD)
	{
		srTxnData.srParameter.chOnlineFlag = ICER_ONLINE;
		srTxnData.srParameter.PacketLenFlag = '1';
		srTxnData.srParameter.chTCPIP_SSL = '0';
		srTxnData.srParameter.chBatchFlag = ICER_BATCH;

		chOnlineFlag = srTxnData.srParameter.chOnlineFlag;
		chBatchFlag = srTxnData.srParameter.chBatchFlag;
		chTCPIP_SSL = srTxnData.srParameter.chTCPIP_SSL;
		PacketLenFlag = srTxnData.srParameter.PacketLenFlag;

		//ICER的SignOn要看參數chETxnSignOnMode,來決定走哪個主機!!
		if(srTxnData.srParameter.chETxnSignOnMode == '1')//R6 VPN
		{
			srTxnData.srParameter.chOnlineFlag = R6_ONLINE;
			srTxnData.srParameter.chBatchFlag = R6_BATCH;
			srTxnData.srParameter.chTCPIP_SSL = '0';
			srTxnData.srParameter.PacketLenFlag = '1';
		}
		else if(srTxnData.srParameter.chETxnSignOnMode == '2')//CMAS VPN
		{
			srTxnData.srParameter.chOnlineFlag = CMAS_ONLINE;
			srTxnData.srParameter.chBatchFlag = CMAS_BATCH;
			srTxnData.srParameter.chTCPIP_SSL = '0';
			srTxnData.srParameter.PacketLenFlag = '1';
		}
		else if(srTxnData.srParameter.chETxnSignOnMode == '3')//CMAS Internet
		{
			srTxnData.srParameter.chOnlineFlag = CMAS_ONLINE;
			srTxnData.srParameter.chBatchFlag = CMAS_BATCH;
			srTxnData.srParameter.chTCPIP_SSL = '1';
			srTxnData.srParameter.PacketLenFlag = '0';
		}
	}

	//SignOnQuery處理
	inRetVal = inSignOnQueryProcess(bInData,bOutData);

	if(inTxnType == TXN_ADJECT_CARD)
	{
		srTxnData.srParameter.chOnlineFlag = chOnlineFlag;
		srTxnData.srParameter.chBatchFlag = chBatchFlag;
		srTxnData.srParameter.chTCPIP_SSL = chTCPIP_SSL;
		srTxnData.srParameter.PacketLenFlag = PacketLenFlag;
	}

	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 1:%d",inRetVal);
		return inRetVal;
	}

	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		vdXMLBackUpData(&srXMLData,&srTmpXMLData,FALSE);

	//Sleep(1000);
	//memset(bTMData,0x00,sizeof(bTMData));
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	memset(bReadPurseFlag,0x00,sizeof(bReadPurseFlag));

	switch(inTxnType)//扣款交易
	{
		case TXN_ECC_DEDUCT:
			if ((srTxnData.srIngData.lnECCAmt1 +
				 srTxnData.srIngData.lnECCAmt2 +
				 srTxnData.srIngData.lnECCAmt3 +
				 srTxnData.srIngData.lnECCAmt4 + 
				 srTxnData.srIngData.lnECCAmt5) > 0L)
			{
				inDLLTrans1 = DDL_DEDUCT_VALUE2;
				inDLLTrans2 = DLL_DEDUCT_VALUE_AUTH;
			}
			else
			{
				inDLLTrans1 = DLL_DEDUCT_VALUE;
				inDLLTrans2 = DLL_DEDUCT_VALUE_AUTH;
			}
			
			break;
		case TXN_ECC_REFUND:
			if(srTxnData.srIngData.lnECCAmt1 + 
		       srTxnData.srIngData.lnECCAmt2 + 
		       srTxnData.srIngData.lnECCAmt3 + 
		       srTxnData.srIngData.lnECCAmt4 + 
		       srTxnData.srIngData.lnECCAmt5 > 0L)
			{
				inDLLTrans1 = DLL_ADD_VALUE1;
				inDLLTrans2 = DLL_ADD_VALUE_AUTH1;
			}
			else
			{
				inDLLTrans1 = DLL_ADD_VALUE;
				inDLLTrans2 = DLL_ADD_VALUE_AUTH;
			}
			break;
		case TXN_ECC_ADD:
		case TXN_ECC_ADD2:
		case TXN_ECC_ADD3:
		case TXN_ECC_ADD4:
		case TXN_ECC_PT_ADD:
		case TXN_ADJECT_CARD:
			inDLLTrans1 = DLL_ADD_VALUE;
			inDLLTrans2 = DLL_ADD_VALUE_AUTH;
			break;
		case TXN_ECC_VOID:
		case TXN_ECC_VOID1:					//悠遊卡加值取消交易
			if(srTxnData.srParameter.gReaderMode == DLL_READER)
				inDLLTrans1 = DLL_VOID_TXN;
			else
				inDLLTrans1 = DLL_ADD_VALUE;
			inDLLTrans2 = DLL_ADD_VALUE_AUTH;
			break;
		case TXN_ECC_REFUND_CARD:
			inDLLTrans1 = DLL_ADD_VALUE;
			//inDLLTrans1 = DLL_ADD_VALUE2;
			inDLLTrans2 = DLL_REFUND_CARD_AUTH;
			//inDLLTrans2 = DLL_ADD_VALUE_AUTH;
			break;
		case TXN_ECC_TX_REFUND:
			inDLLTrans1 = DLL_TX_REFUND;
			inDLLTrans2 = DLL_TX_REFUND_AUTH;
			break;
		case TXN_ECC_TX_ADD:
			inDLLTrans1 = DLL_TX_ADD_VALUE;
			inDLLTrans2 = DLL_TX_ADD_VALUE_AUTH;
			break;
		case TXN_ECC_SET_VALUE:
			inDLLTrans1 = DLL_SET_VALUE;
			inDLLTrans2 = DLL_SET_VALUE_AUTH;
			break;
		case TXN_ECC_AUTOLOAD_ENABLE:
			inDLLTrans1 = DLL_AUTOLOAD_ENABLE;
			inDLLTrans2 = DLL_AUTOLOAD_ENABLE_AUTH;
			break;
		case TXN_ECC_AUTOLOAD:
			inDLLTrans1 = DLL_AUTOLOAD;
			inDLLTrans2 = DLL_AUTOLOAD_AUTH;
			break;
		case TXN_ECC_DEDUCT_TAXI:
			inDLLTrans1 = DLL_TAXI_READ;
			inDLLTrans2 = DLL_TAXI_DEDUCT;
			break;
		case TXN_ECC_TEST1:
		case TXN_ECC_ADD_OFFLINE1: // 離線機具加值
			inDLLTrans1 = DLL_ADD_OFFLINE;
			inDLLTrans2 = DLL_CLOSECOM;
			break;
		case TXN_ECC_VOID_OFFLINE1: // 離線機具加值取消
			inDLLTrans1 = DLL_ADD_OFFLINE;
			inDLLTrans2 = DLL_CLOSECOM;
			break;
		case TXN_ECC_ADD_INQ: // 後台加值查詢
			inDLLTrans1 = DLL_DEDUCT_VALUE1;
			inDLLTrans2 = DLL_CLOSECOM;
			break;
		case TXN_ECC_ACCUNT_LINK:
			inDLLTrans1 = DLL_ACCUNT_LINK;
			inDLLTrans2 = DLL_ACCUNT_LINK_AUTH;
			break;
		case TXN_ECC_STUDENT_SET_VALUE:
		case TXN_ECC_EXTEND_VALID:
			inDLLTrans1 = DLL_S_SET_VALUE;
			inDLLTrans2 = DLL_S_SET_VALUE_AUTH;
			break;
		case TXN_ECC_DEDUCT_CBIKE:
			inDLLTrans1 = DLL_CBIKE_READ;
			inDLLTrans2 = DLL_CBIKE_DEDUCT;
			break;
		case TXN_ECC_DEDUCT_EDCA:
			inDLLTrans1 = DLL_EDCA_READ;
			inDLLTrans2 = DLL_EDCA_DEDUCT;
			break;
	}

	if(inTxnType == TXN_ECC_DEDUCT || inTxnType == TXN_ECC_SET_VALUE || inTxnType == TXN_ECC_STUDENT_SET_VALUE || inTxnType == TXN_ECC_ADD_INQ || inTxnType == TXN_ECC_EXTEND_VALID)//扣款或展期
	{
		inSize = 2;
		if(inTxnType == TXN_ECC_SET_VALUE)
			memcpy(&bInData[inCnt],"80",inSize);
		else if(inTxnType == TXN_ECC_STUDENT_SET_VALUE || inTxnType == TXN_ECC_EXTEND_VALID)
			memcpy(&bInData[inCnt],"50",inSize);
		else
			memcpy(&bInData[inCnt],"01",inSize);
		inCnt += inSize;

		if(srTxnData.srParameter.gDLLVersion == 2)
		{
			inSize = 2;
			if (inTxnType == TXN_ECC_ADD_INQ) // 後台加值查詢
				memcpy(&bInData[inCnt],"FF",inSize);	
			else
				memcpy(&bInData[inCnt],"00",inSize);
			inCnt += inSize;
		}
	}
	else if(inTxnType == TXN_ECC_DEDUCT_TAXI || inTxnType == TXN_ECC_DEDUCT_CBIKE || inTxnType == TXN_ECC_DEDUCT_EDCA)
	{
		bInData[inCnt++] = 0x01;//LCD Control Flag
	}
	else
	{
		inSize = 2;
		if((inTxnType == TXN_ECC_VOID) || (inTxnType == TXN_ECC_VOID1) ||(inTxnType == TXN_ECC_VOID_OFFLINE1)) //取消 & 機具加值取消
			memcpy(&bInData[inCnt],"00",inSize);
		else if(inTxnType == TXN_ECC_TX_REFUND || inTxnType == TXN_ECC_REFUND_CARD)//餘退 or 退卡
			memcpy(&bInData[inCnt],"05",inSize);
		else if(inTxnType == TXN_ECC_AUTOLOAD_ENABLE || inTxnType == TXN_ECC_ACCUNT_LINK)//悠遊卡自動加值開啟 || Accunt Link
		{
			if(srTxnData.srParameter.gReaderMode == DLL_READER)
				memcpy(&bInData[inCnt],"35",inSize);
			else
				memcpy(&bInData[inCnt],"23",inSize);
		}
		else
			memcpy(&bInData[inCnt],"02",inSize);
		inCnt += inSize;

		inSize = 2;
		if(srTxnData.srIngData.inTransType == TXN_ECC_ADD2)
			memcpy(&bInData[inCnt],"18",inSize);
		else if(inTxnType == TXN_ECC_ADD)//加值
			memcpy(&bInData[inCnt],"30",inSize);
		else if(inTxnType == TXN_ECC_REFUND)//退貨的加值
			memcpy(&bInData[inCnt],"0B",inSize);
		else if(inTxnType == TXN_ECC_TX_ADD)//餘加
			memcpy(&bInData[inCnt],"0C",inSize);
		else if(inTxnType == TXN_ECC_AUTOLOAD)//自動加值
			memcpy(&bInData[inCnt],"40",inSize);
		else if(inTxnType == TXN_ECC_ACCUNT_LINK)//Accunt Link
			memcpy(&bInData[inCnt],"01",inSize);
		else if(inTxnType == TXN_ECC_ADD3)//展期加值 ->二代後台加值
			memcpy(&bInData[inCnt],"34",inSize);
		else if(inTxnType == TXN_ECC_ADD4)//機具加值
			memcpy(&bInData[inCnt],"B5",inSize);
		else if(inTxnType == TXN_ADJECT_CARD)
			memcpy(&bInData[inCnt],"35",inSize);
		else if (inTxnType == TXN_ECC_ADD_OFFLINE1) // 離線機具加值
			memcpy(&bInData[inCnt],"B5",inSize);
		else if (inTxnType == TXN_ECC_PT_ADD) //點數兌換加值
			memcpy(&bInData[inCnt],"38",inSize);
		else //if(inTxnType == TXN_ECC_VOID)//取消 & 離線機具加值取消
			memcpy(&bInData[inCnt],"00",inSize);
		inCnt += inSize;
	}

	inCnt += inBuildAPIDefaultData(inDLLTrans1,srTxnData.srIngData.ulTMSerialNumber,&bInData[inCnt],&inRetVal);//TM序號不加1,以免影響Retry機制!!
	//inCnt += inBuildAPIDefaultData(inDLLTrans1,srTxnData.srIngData.ulTMSerialNumber++,&bInData[inCnt]);

	if(srTxnData.srParameter.gDLLVersion == 1)
		inTMDataLen = TM1_DATA_LEN;
	else
		inTMDataLen = TM2_DATA_LEN;

	if(inTxnType != TXN_ECC_SET_VALUE)
		inCnt += inBuildTxnAmt(&bInData[inCnt],lnTxnAmt);

	if(inTxnType == TXN_ECC_TX_REFUND)//餘退
		inCnt += inBuildTxRefundInData(&bInData[inCnt],srTxnData.srIngData.ulTMSerialNumber+1,1);

	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4830,(char *)bReadPurseFlag,1,0);
	if(inRetVal >= SUCCESS)//沒有4830
	{
		if(bReadPurseFlag[0] == '1')
			bReadPurseFlag[0] = 0x01;
		else
			bReadPurseFlag[0] = 0x00;
	}
	else
		bReadPurseFlag[0] = 0x00;
	bInData[inCnt++] = bReadPurseFlag[0];
	/*if(inTxnType == TXN_ECC_SET_VALUE)
		bInData[inCnt++] = 0x00;//Read Purse Flag
	else
		bInData[inCnt++] = 0x01;//LCD Control Flag*/

	if(inTxnType == TXN_ECC_REFUND_CARD)//退卡
		inCnt += inBuildRefundCardInData(&bInData[inCnt]);

	if(inTxnType == TXN_ECC_DEDUCT)//扣款交易
		memcpy(srTxnData.srIngData.bTMData,&bInData[2],inTMDataLen);
	else if (inTxnType == TXN_ADJECT_CARD)
	{
		memcpy(srTxnData.srIngData.bTMData,&bInData[4],inTMDataLen + 8);
	}
	else//非扣款交易
		memcpy(srTxnData.srIngData.bTMData,&bInData[4],inTMDataLen);

	if(inTxnType == TXN_ECC_TX_REFUND)//餘退交易
	{
		if(srTxnData.srParameter.gDLLVersion == 1)
			memcpy(&srTxnData.srIngData.bTMData[8],&bInData[41],20);
		else //if(srTxnData.srParameter.gDLLVersion == 2)
			memcpy(&srTxnData.srIngData.bTMData[12],&bInData[48],20);
	}
	// 指定加值
	if (inTxnType == TXN_ADJECT_CARD)
		memset(srTxnData.srIngData.bTMData + 12, 'X', 14);

	//第一道指令
	inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,FALSE,0,FALSE);

	if(inRetVal == 0x6304 || inRetVal == 0x6305 || inRetVal == 0x630D  || inRetVal == 0x640F || inRetVal == 0x6630)
	{
		BYTE bInData2[READER_BUFFER];

		memset(bInData2,0x00,sizeof(bInData2));
		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		{//將srXMLData回復成初始狀態,以便進行SignOn交易
			vdFreeXMLDOC(&srXMLData);
			vdXMLBackUpData(&srTmpXMLData,&srXMLData,TRUE);
		}

		if( srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY ||
			srTxnData.srParameter.chSignOnMode == RESET_OFF_ONLY ||
			srTxnData.srParameter.chSignOnMode == SIGN_ON_AND_RESET_OFF ||
			srTxnData.srParameter.chSignOnMode == NO_SIGN_ON_AND_RESET)
			inRetVal = inSignOnQueryProcess2(inRetVal,bInData2,bOutData);
		else
		{
			if(inRetVal == 0x6304 || inRetVal == 0x6305 || inRetVal == 0x6630)
				inRetVal = inSignOnProcess(TXN_ECC_SIGN_ON,bOutData,TRUE);
			else //if(inRetVal == 0x630D)
				inRetVal = inSignOnProcess(TXN_ECC_MATCH,bOutData,TRUE);
		}
		if(inRetVal != CARD_SUCCESS)
		{
			if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
				vdFreeXMLDOC(&srTmpXMLData);

			log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 2-1:%d",inRetVal);
			return inRetVal;
		}

		//在執行一次扣款第一道指令
		inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
	}

	if(f6403Check(&srXMLData,bInData,bOutData,inRetVal) == TRUE)
	//if(inRetVal == 0x6403)//餘額不足//自動加值處理
	{
		char chTMProcessCode[6 + 1],chCommandMode;

		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		{//將srXMLData回復成初始狀態,以便進行AutoLoad交易

			vdFreeXMLDOC(&srXMLData);
			vdXMLBackUpData(&srTmpXMLData,&srXMLData,TRUE);
			if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_TAXI)
				vdIntToAsc(((TaxiRead_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
			else if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_CBIKE)
				vdIntToAsc(((CBikeRead_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
			else if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_EDCA)
				vdIntToAsc(((EDCARead_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
			else
				vdIntToAsc(((TxnReqOffline_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
		}
			

		//srTxnData.srIngData.chTMProcessCode在inBuildCMSTxnReqOnlineTM()裡面修改!!
		memset(chTMProcessCode,0x00,sizeof(chTMProcessCode));
		memcpy(chTMProcessCode,srTxnData.srIngData.chTMProcessCode,sizeof(chTMProcessCode));
		memcpy(srTxnData.srIngData.chTMProcessCode,"825799",sizeof(chTMProcessCode));

		inRetVal = inAutoloadProcess(bOutData);

		memcpy(srTxnData.srIngData.chTMProcessCode,chTMProcessCode,sizeof(chTMProcessCode));

		if(inRetVal != SUCCESS)
		{
			int inRet,inCntStart,inCntEnd;
			inRet = inXMLSearchValueByTag(&srXMLData,(char *)TAG_NAME_0409,&inCntStart,&inCntEnd,0);
			if(inRet >= SUCCESS)//有0409
				srXMLData.srXMLElement[inCntStart].fNoResponseTM = TRUE;

			if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
			{
				if(inRetVal == ECC_LOCK_CARD)
				{
					vdFreeXMLDOC(&srXMLData);
					vdXMLBackUpData(&srTmpXMLData,&srXMLData,TRUE);
				}
				vdFreeXMLDOC(&srTmpXMLData);
			}

			log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 2:%d",inRetVal);
			return inRetVal;
		}
		/*else
		{
			UnionUlong ulDateTime;
			BYTE bDateTime[20];

			ulDateTime.Value = lnDateTimeToUnix((BYTE *)srTxnData.srIngData.chTxDate,(BYTE *)srTxnData.srIngData.chTxTime);
			ulDateTime.Value++;
			memset(bDateTime,0x00,sizeof(bDateTime));
			UnixToDateTime(ulDateTime.Buf,&bDateTime[0],14);
			memcpy(srTxnData.srIngData.chTxDate,&bDateTime[0],8);
			memcpy(srTxnData.srIngData.chTxTime,&bDateTime[8],6);
		}*/

		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		{
			vdFreeXMLDOC(&srXMLData);
			vdXMLBackUpData(&srTmpXMLData,&srXMLData,TRUE);
			vdFreeXMLDOC(&srTmpXMLData);

			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0409,srTxnData.srIngData.anAutoloadAmount,FALSE);
		}

		srTxnData.srIngData.fCardActionOKFlag = TRUE;//表示Autoload成功!!
		inCnt = 0;
		if(inTxnType == TXN_ECC_DEDUCT_TAXI || inTxnType == TXN_ECC_DEDUCT_CBIKE || inTxnType == TXN_ECC_DEDUCT_EDCA)
			bInData[inCnt++] = 0x01;//LCD Control Flag
		else
		{
			inSize = 2;
			memcpy(&bInData[inCnt],"01",inSize);
			inCnt += inSize;

			if(srTxnData.srParameter.gDLLVersion == 2)
			{
				inSize = 2;
				memcpy(&bInData[inCnt],"00",inSize);
				inCnt += inSize;
			}
		}
		inCnt += inBuildAPIDefaultData(inDLLTrans1,srTxnData.srIngData.ulTMSerialNumber,&bInData[inCnt],&inRetVal);//TM序號不加1,以免影響Retry機制!!
		//inCnt += inBuildAPIDefaultData(inDLLTrans1,srTxnData.srIngData.ulTMSerialNumber++,&bInData[inCnt]);

		inCnt += inBuildTxnAmt(&bInData[inCnt],lnTxnAmt);

		bInData[inCnt++] = bReadPurseFlag[0];
		//bInData[inCnt++] = 0x01;
		//在執行一次扣款第一道指令
		chCommandMode = srTxnData.srParameter.chCommandMode;
		srTxnData.srParameter.chCommandMode = '0';
		inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
		srTxnData.srParameter.chCommandMode = chCommandMode;
	}
	else if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		vdFreeXMLDOC(&srTmpXMLData);

	if((inRetVal == CARD_SUCCESS && inTxnType != TXN_ECC_DEDUCT && srTxnData.srParameter.gDLLVersion == 1) ||
	   inRetVal == 0x6415)//(一代指令 & 非扣款交易 & 回應碼9000),或回應碼6415,即需連線R6
	{
		BOOL fFlag = FALSE;
		BYTE bTmp[50];

		memset(bInData,0x00,sizeof(bInData));
		inCnt = 0;
		memset(bRecvBuf,0x00,sizeof(bRecvBuf));

		if(inTxnType == TXN_ECC_VOID && srTxnData.srREQData.chDiscountType == 'D')//取消
		{
			if(inCheckDiscountCanVoid(bOutData) != SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 2-1:%d",inRetVal);
				return CAN_NOT_DISCOUNT_VOID;
			}
		}

		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		{
			memset(bTmp,0x00,sizeof(bTmp));
			inXMLGetData(&srXMLData,(char *)TAG_NAME_0200,(char *)bTmp,sizeof(bTmp),0);

			if(!srTxnData.srIngData.fCardActionOKFlag)
			{
				if(inTxnType == TXN_ECC_DEDUCT)
					vdIntToAsc(((TxnReqOffline_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
				else if(inTxnType == TXN_ECC_DEDUCT_TAXI)
					vdIntToAsc(((TaxiRead_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
				else if(inTxnType == TXN_ECC_DEDUCT_CBIKE)
					vdIntToAsc(((CBikeRead_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
				else if(inTxnType == TXN_ECC_DEDUCT_EDCA)
					vdIntToAsc(((EDCARead_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
				else if (inTxnType == TXN_ECC_EXTEND_VALID)	// 20年展期
					vdIntToAsc(((StudentSetValue_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
			}
		}

		memset(bTmp,0x00,sizeof(bTmp));
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4105,(char *)bTmp,20,0);
		if(inRetVal >= SUCCESS)//有4105
			fFlag = TRUE;

		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE || srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)
			inRetVal = inSendRecvXML((srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE) ? HOST_CMAS:HOST_ICER,&srXMLData,inTxnType);
		else //if(srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
			inRetVal = inTCPSendAndReceive(inDLLTrans1,bOutData,bRecvBuf,TRUE);
		//else if(srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)保留,尚未開發
		if(inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 3:%d",inRetVal);
			return inRetVal;
		}
		if(srTxnData.srParameter.gDLLVersion == 1)//1代API
		{
			if(inTxnType != TXN_ECC_DEDUCT && inTxnType != TXN_ECC_AUTOLOAD_ENABLE)
			{
				if(inTxnType != TXN_ECC_TX_REFUND)
					memcpy(&bInData[inCnt],&bRecvBuf[39],4);
				inCnt += 4;
			}
			memcpy(&bInData[inCnt],&bRecvBuf[75],8);
			//bInData[8] = 0x00;
		}
		else
		{
			memcpy(&bInData[inCnt],&bRecvBuf[10],sizeof(bRecvBuf) - 10);

			if(inTxnType == TXN_ECC_STUDENT_SET_VALUE || inTxnType == TXN_ECC_EXTEND_VALID)
			{
				if( ((AuthStudentSetValue_TM_In *)bInData)->ucCPUCardSettingParameter[1] == 0x30 &&
					((AuthStudentSetValue_TM_In *)bInData)->ucCPUCardSettingParameter[2] == 0x30 &&
					((AuthStudentSetValue_TM_In *)bInData)->ucCPUCardSettingParameter[3] == 0x30)
				{
					//log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 3:%d",inRetVal);
					return DATA_NO_NEED_CHANGE_FAIL;
				}
			}
		}

		if(fFlag == TRUE)
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4105,bTmp,FALSE);
		
		if (inTxnType == TXN_ECC_ADD_INQ) // 後台加值查詢
			inRetVal = CARD_SUCCESS;
			
	}
	else if(inRetVal == CARD_SUCCESS)//回應碼9000,第二道指令的Input_Data直接帶0
	{
		memset(bInData,0x00,sizeof(bInData));

		if(/*inTxnType == TXN_ECC_DEDUCT && */srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE && !srTxnData.srIngData.fCardActionOKFlag)
		{
			if(inTxnType == TXN_ECC_DEDUCT)
				vdIntToAsc(((TxnReqOffline_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
			else if(inTxnType == TXN_ECC_DEDUCT_TAXI)
				vdIntToAsc(((TaxiRead_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
			else if(inTxnType == TXN_ECC_DEDUCT_CBIKE)
				vdIntToAsc(((CBikeRead_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
			else if(inTxnType == TXN_ECC_DEDUCT_EDCA)
				vdIntToAsc(((EDCARead_APDU_Out *)bOutData)->ucEV,3,srTxnData.srIngData.anCardEVBeforeTxn,10,TRUE,0x00,10);
		}

		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE && inTxnType != TXN_ECC_DEDUCT)//該Online沒Online,表示Retry
			vdReversalOff((srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE) ? HOST_CMAS : HOST_ICER);
	}
	else if(inRetVal == 0x6406 ||
			inRetVal == 0x640E ||
			inRetVal == 0x6103 ||
			inRetVal == 0x610F ||
			inRetVal == 0x6418)//需組鎖卡回饋檔之回應碼
	{
		int inSW = inRetVal;

		log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 4:%d",inRetVal);
		vd640EProcess(inRetVal,bOutData,bReadCommandOut);
		/*if(srTxnData.srParameter.gReaderMode == DLL_READER || srTxnData.srParameter.chBatchFlag == R6_BATCH)
			memcpy(srTxnData.srIngData.anTLKR_TLRC,bOutData,TLKR_TLRC_LEN);
		else //if(srTxnData.srParameter.gReaderMode != DLL_READER)
			fnUnPack(bOutData,SIS2_LOCK_LEN,srTxnData.srIngData.anTLKR_TLRC);
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
		if(inRetVal < SUCCESS)//尚未有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
		else//已有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
		}*/
		//log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 5:%d",inRetVal);
		if(inSW == 0x6406)
			return ECC_LOCK_CARD;
		else
			return DLL_DECLINE_FAIL;
	}
	else if(inRetVal != CARD_SUCCESS)//失敗之回應碼
	{
		log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 6:%d",inRetVal);
		if(inRetVal < SUCCESS)
			return inRetVal;

		if(inRetVal < SUCCESS)
			return LOAD_DLL_FUNCTION_ERROR;
		return DLL_DECLINE_FAIL;
	}

	//第二道指令
	if(inDLLTrans2 != DLL_CLOSECOM)
	{
		if(inTxnType == TXN_ECC_DEDUCT_TAXI || inTxnType == TXN_ECC_DEDUCT_CBIKE || inTxnType == TXN_ECC_DEDUCT_EDCA)
		{
			inCnt = 0;

			memcpy(&bInData[inCnt],"01",2);
			inCnt += 2;

			memcpy(&bInData[inCnt],"00",2);
			inCnt += 2;

			inCnt += inBuildAPIDefaultData(inDLLTrans2,srTxnData.srIngData.ulTMSerialNumber,&bInData[inCnt],&inRetVal);

			inCnt += inBuildTxnAmt(&bInData[inCnt],lnTxnAmt);
		}

		memset(bOutData,0x00,sizeof(bOutData));
		inRetVal = inReaderCommand(inDLLTrans2,&srXMLData,bInData,bOutData,TRUE,0,FALSE);
	}

	if(inRetVal != CARD_SUCCESS)//第二道指令失敗即離開
	{
		if(srTxnData.srIngData.fOnlineHostSuccFlag == TRUE && srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
			vdReversalOn((srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE) ? HOST_CMAS : HOST_ICER);

		log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 7:%d",inRetVal);
		if(inTxnType == TXN_ECC_ACCUNT_LINK && inRetVal == 0x640A)
			return DATA_NO_NEED_CHANGE_FAIL;
		
		if ((inTxnType == TXN_ADJECT_CARD) &&( inRetVal == CALL_DLL_TIMEOUT_ERROR))//指定加值
		{
			if(inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0) < SUCCESS)//沒有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554810,(char *)srTxnData.srIngData.bTMData,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
			}
			else//有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554810,(char *)srTxnData.srIngData.bTMData,TRUE,NODE_NO_ADD,VALUE_NAME,FALSE);
				//inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_554810,srTxnData.srIngData.bTMData,FALSE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
			}
		}

		if(inRetVal < SUCCESS)
			return inRetVal;

		/*if(inRetVal < SUCCESS)
			return LOAD_DLL_FUNCTION_ERROR;*/
		return DLL_DECLINE_FAIL;
	}

	if(inTxnType == TXN_ECC_AUTOLOAD)//自動加值交易已成功,將anAutoloadFlag設定成1
		srTxnData.srIngData.anAutoloadFlag[0] = '1';

	// AVM 因BatchFlag=SIS2_BATCH所以將此function移到此 //V4006B再移回下面
	//if (srTxnData.srParameter.chOnlineFlag == R6_ONLINE)
	//	vdUpdateECCDataAfterTxn(inTxnType,bOutData);

	if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH || srTxnData.srParameter.chBatchFlag == ICER_BATCH)
	{
		BYTE bBuffer[MAX_XML_FILE_SIZE];

		if (inTxnType == TXN_ECC_SET_VALUE || inTxnType == TXN_ECC_AUTOLOAD_ENABLE || inTxnType == TXN_ECC_EXTEND_VALID)//社福展期不用Advice 20年展期不用Advice
			return SUCCESS;

		inRetVal = inBuildSendPackage((srTxnData.srParameter.chBatchFlag == CMAS_BATCH)?HOST_CMAS:HOST_ICER,&srXMLData,bBuffer,REVERSAL_OFF,ADVICE_ON,STORE_BATCH_ON);
		//log_msg(LOG_LEVEL_ERROR,"inDoECCTxn 1,%d",inRetVal);
		if(inRetVal < SUCCESS)
			log_msg(LOG_LEVEL_ERROR,"inDoECCTxn Fail 8");
		else if(srTxnData.srParameter.chAdviceFlag != '1')
			inTCPIPAdviceProcess((srTxnData.srParameter.chBatchFlag == CMAS_BATCH)?HOST_CMAS:HOST_ICER,FALSE);

		vdIcnBankData(6);//6:加/扣成功才+1
		//vdModifySAmtTag(&srXMLData);//移到inFinalizer()去做!!
		//vdXMLChangeTagName(&srXMLData,(char *)TAG_NAME_0410,(char *)TAG_NAME_0410,(char *)"00");
		return SUCCESS;
	}
	else if(srTxnData.srParameter.chBatchFlag == SIS2_BATCH)
	{
		int inSTMCLen = sizeof(SIS2_HEADER) + sizeof(SIS2_END);

		//for AVM 現金加值 自動加值 聯名卡退卡送R6 confirm
		if (srTxnData.srParameter.chAVM != 0)
			if (inTxnType == TXN_ECC_ADD || inTxnType == TXN_ECC_AUTOLOAD || inTxnType == TXN_ECC_REFUND_CARD)
				inTCPSendAndReceive(inDLLTrans2,&bInData[SIZE_STMC],bRecvBuf,FALSE);

		memset(srTxnData.srIngData.anSETM_STMC_1,0x00,sizeof(srTxnData.srIngData.anSETM_STMC_1));
		memset(srTxnData.srIngData.anSETM_STMC_2,0x00,sizeof(srTxnData.srIngData.anSETM_STMC_2));

		if(inTxnType == TXN_ECC_DEDUCT || inTxnType == TXN_ECC_DEDUCT_CBIKE || inTxnType == TXN_ECC_DEDUCT_EDCA)//扣款交易
			inSTMCLen += sizeof(SIS2_BODY_DEDUCT);
		else if(inTxnType == TXN_ECC_ADD ||
				inTxnType == TXN_ECC_ADD_OFFLINE ||
				inTxnType == TXN_ECC_ADD_OFFLINE1||
				inTxnType == TXN_ECC_VOID ||
				inTxnType == TXN_ECC_VOID1 ||
				inTxnType == TXN_ECC_VOID_OFFLINE ||
				inTxnType == TXN_ECC_VOID_OFFLINE1 ||
				inTxnType == TXN_ECC_ADD2 ||
				inTxnType == TXN_ECC_ADD3 ||
				inTxnType == TXN_ECC_AUTOLOAD ||
				inTxnType == TXN_ECC_ADD4 ||
				inTxnType == TXN_ECC_PT_ADD)
			inSTMCLen += sizeof(SIS2_BODY_ADD);
		else if(inTxnType == TXN_ECC_SALE_CARD_OFFLINE)
			inSTMCLen += sizeof(SIS2_BODY_SALE_CARD);
		else if(inTxnType == TXN_ECC_PENALTY)
			inSTMCLen += sizeof(SIS2_BODY_PENALTY);
		else if(inTxnType == TXN_ECC_STUDENT_SET_VALUE ||
			    inTxnType == TXN_ECC_EXTEND_VALID)
			inSTMCLen += sizeof(SIZE_TXET);
		else if(inTxnType == TXN_ECC_DEDUCT2)
			inSTMCLen += sizeof(SIS2_BODY_FARE_SALE);
		else if(inTxnType == TXN_ECC_REFUND_CARD)
			inSTMCLen += sizeof(SIS2_BODY_REFUND_CARD);
		else if(inTxnType == TXN_ECC_TRTC_SET_VALUE || inTxnType == TXN_ECC_SET_VALUE_OFFLINE)
			return SUCCESS;

		if(inTxnType == TXN_ECC_STUDENT_SET_VALUE ||
			inTxnType == TXN_ECC_EXTEND_VALID)
			memcpy(srTxnData.srIngData.anSETM_STMC_1,&bOutData[18],inSTMCLen);
		else
			fnUnPack(bOutData,inSTMCLen,srTxnData.srIngData.anSETM_STMC_1);

		memset(bBuf,0x00,sizeof(bBuf));
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
		if(inRetVal < SUCCESS)//沒有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			//if(inTxnType == TXN_ECC_TX_REFUND)//餘退交易
			//	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			if(inTxnType == TXN_ECC_DEDUCT || inTxnType == TXN_ECC_TX_REFUND || inTxnType == TXN_ECC_DEDUCT2)//扣款或餘退要多帶554805
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
		else//有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
			//if(inTxnType == TXN_ECC_TX_REFUND)//餘退交易
			//	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			if(inTxnType == TXN_ECC_DEDUCT || inTxnType == TXN_ECC_TX_REFUND || inTxnType == TXN_ECC_DEDUCT2)
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
		}
		if(inTxnType == TXN_ECC_STUDENT_SET_VALUE ||
			inTxnType == TXN_ECC_EXTEND_VALID)
		{
			BYTE bData[30];

			memset(bData,0x00,sizeof(bData));
			sprintf((char *)bData,"%ld00",srTxnData.srIngData.lnECCAmt);//n_Card Physical ID
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bData,FALSE);
			//inTCPSendAndReceive(inDLLTrans2,&bOutData[inSTMCLen + 18],bRecvBuf,FALSE);
		}

		return SUCCESS;
	}
	//else if(srTxnData.srParameter.chBatchFlag == R6_BATCH)

	vdUpdateECCDataAfterTxn(inTxnType,bOutData);

	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		if(inTxnType == TXN_ECC_DEDUCT)//扣款交易
		{
			memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,SIZE_SETM);
			memcpy(srTxnData.srIngData.anTAVR_TARC,&bOutData[SIZE_SETM],TAVR_TARC_LEN);
		}
		else if(inTxnType == TXN_ECC_TX_REFUND)//餘退交易
		{
			memcpy(&srTxnData.srIngData.anSETM_STMC_1[0],bOutData,148);
			memcpy(&srTxnData.srIngData.anSETM_STMC_1[148],srTxnData.srIngData.bTMData,TM1_DATA_LEN);
			inTCPSendAndReceive(inDLLTrans2,&bOutData[148],bRecvBuf,FALSE);
			memcpy(srTxnData.srIngData.anSETM_STMC_2,&bOutData[201],SIZE_SETM);
			memcpy(srTxnData.srIngData.anTAVR_TARC,&bOutData[201 + SIZE_SETM],TAVR_TARC_LEN);
		}
		else//加值交易
		{
			memcpy(&srTxnData.srIngData.anSETM_STMC_1[0],bOutData,148);
			memcpy(&srTxnData.srIngData.anSETM_STMC_1[148],srTxnData.srIngData.bTMData,TM1_DATA_LEN);
			inTCPSendAndReceive(inDLLTrans2,&bOutData[148],bRecvBuf,FALSE);
		}

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
		if(inRetVal < SUCCESS)//沒有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			if(inTxnType == TXN_ECC_TX_REFUND)//餘退交易
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			if(inTxnType == TXN_ECC_DEDUCT || inTxnType == TXN_ECC_TX_REFUND)//扣款或餘退要多帶554805
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
		else//有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
			if(inTxnType == TXN_ECC_TX_REFUND)//餘退交易
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			if(inTxnType == TXN_ECC_DEDUCT || inTxnType == TXN_ECC_TX_REFUND)
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
		}
	}
	else //if(srTxnData.srParameter.gDLLVersion == 2)//2代API
	{
		//char chPurseVersion = ((AuthAddValueL2_TM_Out *)bOutData)->stSTMC_t.unPurseVersionNumber[1] - 0x30;

		//if(chPurseVersion == MIFARE)
		{
			memset(srTxnData.srIngData.anSETM_STMC_1,0x00,sizeof(srTxnData.srIngData.anSETM_STMC_1));
			memset(srTxnData.srIngData.anSETM_STMC_2,0x00,sizeof(srTxnData.srIngData.anSETM_STMC_2));
			if(inTxnType == TXN_ECC_DEDUCT)//扣款交易
			{
				memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,SIZE_STMC + 12 + 8);
				memcpy(srTxnData.srIngData.anTAVR_TARC,&bOutData[SIZE_STMC + 12 + 8],TAVR_TARC_LEN);
			}
			else if(inTxnType == TXN_ECC_TX_REFUND)//餘退交易
			{
				memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,SIZE_STMC);
				inTCPSendAndReceive(inDLLTrans2,&bOutData[SIZE_STMC],bRecvBuf,FALSE);
				memcpy(srTxnData.srIngData.anSETM_STMC_2,&bOutData[503],SIZE_STMC);
				memcpy(srTxnData.srIngData.anTAVR_TARC,&bOutData[503 + SIZE_STMC],TAVR_TARC_LEN);
			}
			else if(inTxnType == TXN_ECC_SET_VALUE)
			{
				memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,67);
			}
			else if(inTxnType == TXN_ECC_STUDENT_SET_VALUE ||
				    inTxnType == TXN_ECC_EXTEND_VALID)
			{
				memcpy(srTxnData.srIngData.anSETM_STMC_1,&((AuthStudentSetValue_TM_Out *)bOutData)->stSTMC_t,SIZE_STMC);
				inTCPSendAndReceive(inDLLTrans2,&bOutData[SIZE_STMC + 18],bRecvBuf,FALSE);
			}
			else if(inTxnType == TXN_ECC_REFUND && srTxnData.srIngData.lnECCAmt1 + srTxnData.srIngData.lnECCAmt2 + srTxnData.srIngData.lnECCAmt3 + srTxnData.srIngData.lnECCAmt4 + srTxnData.srIngData.lnECCAmt5 > 0L)
			{
				memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,SIZE_STMC + 12 + 8);
				inTCPSendAndReceive(inDLLTrans2,&bOutData[SIZE_STMC + 12 + 8],bRecvBuf,FALSE);
			}
			else if (inTxnType == TXN_ECC_ADD_INQ)
			{
				memset(bBuf,0x00,sizeof(bBuf));
				vdIntToAsc(((AuthTxnOffline_TM_In2 *)bInData)->ucTXNAMT, 3, bBuf, 5,TRUE,0x00,10);
				strcat((char *)bBuf,"00");
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0400,bBuf,FALSE);
				return SUCCESS;
			}
			else//加值交易
			{
				memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,SIZE_STMC);
				inTCPSendAndReceive(inDLLTrans2,&bOutData[SIZE_STMC],bRecvBuf,FALSE);
			}

			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
			if(inRetVal < SUCCESS)//沒有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
				if(inTxnType == TXN_ECC_TX_REFUND)//餘退交易
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
				if(inTxnType == TXN_ECC_DEDUCT || inTxnType == TXN_ECC_TX_REFUND)//扣款或餘退要多帶554805
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
			}
			else//有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
				if(inTxnType == TXN_ECC_TX_REFUND)//餘退交易
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554803,(char *)srTxnData.srIngData.anSETM_STMC_2,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
				if(inTxnType == TXN_ECC_DEDUCT || inTxnType == TXN_ECC_TX_REFUND)
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
			}
		}
	}

	return SUCCESS;
}

int inDoECCQueryTxn(int inTxnType)
{
BYTE bInData[READER_BUFFER],bOutData[READER_BUFFER],bBuf[20], bBuf2[20];;
int inRetVal = SUCCESS,inCnt = 0,inSize,inDLLType = 0;
STRUCT_XML_DOC srTmpXMLData;

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	memset(bBuf,0x00,sizeof(bBuf));
	memset(bBuf2,0x00,sizeof(bBuf2));
	
	if(inTxnType == TXN_ECC_RESET_OFF || inTxnType == TXN_ECC_SIGN_ON_QUERY || inTxnType == TXN_ECC_SIGN_ON || inTxnType == TXN_ECC_MATCH || inTxnType == TXN_ECC_MATCH_OFF)
	{
		int inLen = 0;
		inRetVal = ICER_ERROR;
		if(inTxnType == TXN_ECC_RESET_OFF || inTxnType == TXN_ECC_MATCH_OFF)
		{
			inRetVal = inResetOfflineProcess((inTxnType == TXN_ECC_RESET_OFF)?DLL_RESET_OFF:DLL_MATCH_OFF,&srXMLData,bInData,bOutData);
			//inBuildAPIDefaultData(DLL_RESET_OFF,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);
			//inRetVal = inReaderCommand((inTxnType == TXN_ECC_RESET_OFF)?DLL_RESET_OFF:DLL_MATCH_OFF,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
			if(inRetVal == CARD_SUCCESS)
			{
				inLen = sizeof(Reset_APDU_Out);
				inRetVal = SUCCESS;
			}
			else
			{
				if(inRetVal < SUCCESS)
					return inRetVal;
				return DLL_DECLINE_FAIL;
			}
		}
		else if(inTxnType == TXN_ECC_SIGN_ON_QUERY)
		{
			inRetVal = inReaderCommand(DLL_SIGNONQUERY,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
			if(inRetVal == CARD_SUCCESS || inRetVal == 0x6304 || inRetVal == 0x6305 || inRetVal == 0x630D || inRetVal == 0x630E)
			{
				if (inRetVal == 0x6304 || inRetVal == 0x6305)
				{
					inSignOnProcess(TXN_ECC_SIGN_ON,bOutData,TRUE);
					inRetVal = inReaderCommand(DLL_SIGNONQUERY,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
				}
				/*if(srTxnData.srParameter.chBatchFlag == R6_BATCH)
					inLen = 0;
				else*/
					inLen = sizeof(SignOnQuery_APDU_Out);
				if(inRetVal == CARD_SUCCESS)
					inRetVal = SUCCESS;
				else
					inRetVal = DLL_DECLINE_FAIL;
			}
			else
			{
				if(inRetVal < SUCCESS)
					return inRetVal;
				return DLL_DECLINE_FAIL;
			}
		}
		else if(inTxnType == TXN_ECC_SIGN_ON || inTxnType == TXN_ECC_MATCH)
		{
			inRetVal = inSignOnProcess(inTxnType,bOutData,FALSE);
			if(inRetVal == CARD_SUCCESS)
			{
				/*inLen = 0;
				if(srTxnData.srParameter.chBatchFlag == R6_BATCH)
					inLen = 0;
				else*/
					inLen = sizeof(SignOn_APDU_Out);
				inRetVal = SUCCESS;
			}
			else
			{
				if(inRetVal < SUCCESS)
					return inRetVal;
				return DLL_DECLINE_FAIL;
			}
		}

		if(inLen > 0)
		{
			int inRet;
			//if(inTxnType != TXN_ECC_SIGN_ON && inTxnType != TXN_ECC_MATCH)
			{
				memset(srTxnData.srIngData.anSETM_STMC_1,0x00,sizeof(srTxnData.srIngData.anSETM_STMC_1));
				fnUnPack(bOutData,inLen,srTxnData.srIngData.anSETM_STMC_1);
			}
			inRet = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
			if(inRet < SUCCESS)//沒有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)srTxnData.srIngData.anSETM_STMC_1,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
			}
			else//有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)srTxnData.srIngData.anSETM_STMC_1,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
			}

			//return SUCCESS;
		}

		return inRetVal;
	}

	if(inTxnType == TXN_ECC_MULTI_SELECT)//備分
		vdXMLBackUpData(&srXMLData,&srTmpXMLData,FALSE);

	if(inTxnType != TXN_ECC_READ_CARD && inTxnType != TXN_ECC_ANTENN_ACONTROL && inTxnType != TXN_ECC_READ_DEVICE_INFO && inTxnType != TXN_ECC_TLRT_AUTH)
		inRetVal = inSignOnQueryProcess(bInData,bOutData);

	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inDoECCQueryTxn Fail 1:%d",inRetVal);
		if(inTxnType != TXN_ECC_MULTI_SELECT)
			return inRetVal;
		else
			vdXMLBackUpData(&srTmpXMLData,&srXMLData,TRUE);//還原
	}
	if(inTxnType == TXN_ECC_MULTI_SELECT)
		vdFreeXMLDOC(&srTmpXMLData);//清除備份

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	//Sleep(1000);
	if(inTxnType == TXN_ECC_READ_BASIC ||
	   inTxnType == TXN_ECC_READ_TAXI ||
	   inTxnType == TXN_ECC_READ_THSRC ||
	   inTxnType == TXN_ECC_READ_CBIKE ||
	   inTxnType == TXN_ECC_READ_EDCA)
	{
		int inCntStart = 0,inCntEnd = 0;

		if(inTxnType == TXN_ECC_READ_BASIC)
		{
			if(srTxnData.srParameter.chAutoLoadMode == '1' && inXMLSearchValueByTag(&srXMLData,(char *)TAG_NAME_0400,&inCntStart,&inCntEnd,0) < SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inDoECCQueryTxn Fail 1-1:(AutoLoadMode = 1,But No T0400)");
				return ICER_REQ_DATA_FAIL;
			}

			inDLLType = DLL_READ_CARD_BASIC_DATA;
		}
		else if(inTxnType == TXN_ECC_READ_TAXI)
			inDLLType = DLL_TAXI_READ;
		else if(inTxnType == TXN_ECC_READ_THSRC)
			inDLLType = DLL_THSRC_READ;
		else if(inTxnType == TXN_ECC_READ_CBIKE)
			inDLLType = DLL_CBIKE_READ;
		else if(inTxnType == TXN_ECC_READ_EDCA)
			inDLLType = DLL_EDCA_READ;

		if(srTxnData.srParameter.gDLLVersion == 2)//2代API
		{
			inCnt = 0;

			bInData[inCnt++] = bGetLCDControlFlag();
			//inCnt++;//LCD Control Flag

			inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID) - 1;
			memcpy(&bInData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,inSize);
			inCnt += inSize;

			inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID) - 1;
			memcpy(&bInData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,inSize);
			inCnt += inSize;

			inSize = sizeof(srTxnData.srIngData.chTxDate) - 1;
			memcpy(&bInData[inCnt],srTxnData.srIngData.chTxDate,inSize);
			inCnt += inSize;

			inSize = sizeof(srTxnData.srIngData.chTxTime) - 1;
			memcpy(&bInData[inCnt],srTxnData.srIngData.chTxTime,inSize);
			inCnt += inSize;

			if(inTxnType == TXN_ECC_READ_TAXI || inTxnType == TXN_ECC_READ_THSRC || inTxnType == TXN_ECC_READ_CBIKE || inTxnType == TXN_ECC_READ_EDCA || inTxnType == TXN_ECC_READ_BASIC)
			{
				inSize = 6;
				sprintf((char *)&bInData[inCnt],"%06ld",srTxnData.srIngData.ulTMSerialNumber);
				inCnt += inSize;

				inSize = 4;
				memcpy(&bInData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMAgentNumber,inSize);
				inCnt += inSize;

				if(srTxnData.srParameter.chBasicCheckQuotaFlag == '1' && inTxnType == TXN_ECC_READ_BASIC)
					inCnt += inBuildTxnAmt(&bInData[inCnt],srTxnData.srIngData.lnECCAmt);
			}
		}
	}
	else if(inTxnType == TXN_ECC_READ_DONGLE)
	{
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5594,(char *)bBuf,5,0);
		if(inRetVal < SUCCESS)//沒有5594
			memset(bBuf,0x30,5);
		else if(inRetVal < 5)
			vdPad((char *)bBuf,(char *)bBuf2,5,FALSE,0x30);
		memcpy(&bInData[inCnt],bBuf2,5);
		inCnt += 5;
		inDLLType = DLL_READ_DONGLE_LOG;
	}
	else if(inTxnType == TXN_ECC_READ_DEDUCT)
	{
		inDLLType = DLL_READ_CARD_LOG;
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5593,(char *)bBuf,1,0);
		if(inRetVal < SUCCESS)//沒有5595
			bInData[inCnt] = 0x01;
		if(bBuf[0] == '2')
			bInData[inCnt] = 0x02;
		else
			bInData[inCnt] = 0x01;
		inCnt++;
	}
	else if(inTxnType == TXN_ECC_READ_CARD)
	{
		if(srTxnData.srParameter.gDLLVersion == 2)//2代API
		{
			inDLLType = DLL_READ_CARD_NUMBER;
			memset(&bInData[inCnt],'0',8);
			inCnt += 8;
			inCnt++;//LCD Control Flag
		}
	}
	else if(inTxnType == TXN_ECC_MULTI_SELECT)
	{
		inDLLType = DLL_MULTI_SELECT;
		memset(bInData,0x00,sizeof(bInData));
		memset(bInData,0x30,18);
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483100,(char *)bBuf,1,0);
		if(inRetVal >= SUCCESS)//有483100
			bInData[inCnt] = bBuf[0];
		inCnt++;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483101,(char *)bBuf,1,0);
		if(inRetVal >= SUCCESS)//有483101
			bInData[inCnt] = bBuf[0];
		inCnt++;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483102,(char *)bBuf,1,0);
		if(inRetVal >= SUCCESS)//有483102
			bInData[inCnt] = bBuf[0];
		inCnt++;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483103,(char *)bBuf,1,0);
		if(inRetVal >= SUCCESS)//有483103
			bInData[inCnt] = bBuf[0];
		inCnt++;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483104,(char *)bBuf,2,0);
		if(inRetVal >= SUCCESS)//有483104
			memcpy(&bInData[inCnt],&bBuf[0],2);
		inCnt += 2;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483105,(char *)bBuf,2,0);
		if(inRetVal >= SUCCESS)//有483105
			memcpy(&bInData[inCnt],&bBuf[0],2);
		inCnt += 2;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483106,(char *)bBuf,2,0);
		if(inRetVal >= SUCCESS)//有483106
			memcpy(&bInData[inCnt],&bBuf[0],2);
		inCnt += 2;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483107,(char *)bBuf,2,0);
		if(inRetVal >= SUCCESS)//有483107
			memcpy(&bInData[inCnt],&bBuf[0],2);
		inCnt += 2;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483108,(char *)bBuf,2,0);
		if(inRetVal >= SUCCESS)//有483108
			memcpy(&bInData[inCnt],&bBuf[0],2);
		inCnt += 2;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483109,(char *)bBuf,1,0);
		if(inRetVal >= SUCCESS)//有483109
			bInData[inCnt] = bBuf[0];
		inCnt++;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483110,(char *)bBuf,1,0);
		if(inRetVal >= SUCCESS)//有483110
			bInData[inCnt] = bBuf[0];
		inCnt++;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_483111,(char *)bBuf,2,0);
		if(inRetVal >= SUCCESS)//有483111
		{
			int inTimeOut = (bBuf[0] - 0x30) * 10 + (bBuf[1] - 0x30);
			sprintf((char *)&bInData[inCnt],"%02x",inTimeOut);
		}
		inCnt += 2;
	}
	else if(inTxnType == TXN_ECC_ANTENN_ACONTROL)
	{
		inDLLType = DLL_ANTENN_CONTROL;
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4832,(char *)bBuf,1,0);
		if(inRetVal < SUCCESS)//尚未有4832
			bInData[0] = 0x00;//天線開
		else if(bBuf[0] == 0x31)
			bInData[0] = 0x01;//天線關
		else
			bInData[0] = 0x00;//天線開
	}
	else if(inTxnType == TXN_ECC_FAST_READ_CARD)
	{
		if(srTxnData.srParameter.gDLLVersion == 2)//2代API
		{
			inDLLType = DLL_FAST_READ_CARD;
		}
	}
	else if(inTxnType == TXN_ECC_READ_DEVICE_INFO)
	{
		inDLLType = DLL_RESET2;
		inBuildAPIDefaultData(inDLLType,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);//TM序號不加1,以免影響Retry機制!!
	}
	/*else if(inTxnType == TXN_ECC_READ_TAXI)
	{
		inDLLType = DLL_TAXI_READ;
		inBuildAPIDefaultData(inDLLType,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);//TM序號不加1,以免影響Retry機制!!
	}*/
	else if(inTxnType == TXN_ECC_TLRT_AUTH)
	{
		inDLLType = DLL_TLRT_AUTH;
	}
	else if(inTxnType == TXN_ECC_FAST_READ_CARD2)
	{
		inDLLType = DLL_FAST_READ_CARD2;
		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4849,(char *)bBuf,1,0);
		if(inRetVal >= SUCCESS)
			srTxnData.srREQData.chReadPVNFlag = bBuf[0];
	}

	inRetVal = inReaderCommand(inDLLType,&srXMLData,bInData,bOutData,FALSE,0,FALSE);

	if(inRetVal == 0x6304 || inRetVal == 0x6305 || inRetVal == 0x630D)
	{
		BYTE bInData2[READER_BUFFER];

		if(inTxnType == TXN_ECC_READ_DEVICE_INFO)
			inDLLType = DLL_MATCH;
		else
		{
			memset(bInData2,0x00,sizeof(bInData2));
			if( srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY ||
				srTxnData.srParameter.chSignOnMode == RESET_OFF_ONLY ||
				srTxnData.srParameter.chSignOnMode == SIGN_ON_AND_RESET_OFF ||
				srTxnData.srParameter.chSignOnMode == NO_SIGN_ON_AND_RESET)
				inRetVal = inSignOnQueryProcess2(inRetVal,bInData2,bOutData);
			else
			{
				if(inRetVal == 0x6304 || inRetVal == 0x6305)
					inRetVal = inSignOnProcess(TXN_ECC_SIGN_ON,bOutData,TRUE);
				/*else if(inTxnType == TXN_ECC_READ_DEVICE_INFO)//Reset不會回這個!!
				{
					inDLLType = DLL_MATCH2;
					inBuildAPIDefaultData(inDLLType,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);//TM序號不加1,以免影響Retry機制!!
					inRetVal = CARD_SUCCESS;
				}*/
				else //if(inRetVal == 0x630D)
					inRetVal = inSignOnProcess(TXN_ECC_MATCH,bOutData,TRUE);
			}
			if(inRetVal != CARD_SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inDoECCQueryTxn Fail 1-2:%d",inRetVal);
				return inRetVal;
			}
		}

		//在執行一次扣款第一道指令
		inRetVal = inReaderCommand(inDLLType,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
	}

	if(srTxnData.srParameter.gReaderMode == DLL_READER || srTxnData.srParameter.chBatchFlag == R6_BATCH)
	{
		if(inRetVal == 0x6406 ||
		   inRetVal == 0x640E ||
		   inRetVal == 0x6103 ||
		   inRetVal == 0x610F ||
		   inRetVal == 0x6418)//需組鎖卡回饋檔之回應碼
		{
			log_msg(LOG_LEVEL_ERROR,"inDoECCQueryTxn Fail 2:%d",inRetVal);
			vd640EProcess(inRetVal,bOutData,bReadCommandOut);
			/*memcpy(srTxnData.srIngData.anTLKR_TLRC,bOutData,TLKR_TLRC_LEN);
			inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
			if(inRetVal < SUCCESS)//尚未有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
			}
			else//已有5548
			{
				vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
				inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
			}*/
			return DLL_DECLINE_FAIL;
		}
		else if(inRetVal != CARD_SUCCESS)
		{
			if(inTxnType == TXN_ECC_READ_BASIC && srTxnData.srParameter.chBasicCheckQuotaFlag == '1' && (inRetVal == 0x640C || inRetVal == 0x640D))
			{
				memset(bInData,0x00,sizeof(bInData));
				memcpy(bInData,bOutData,sizeof(bInData));
				inBuildReadCardBasicDataData(inDLLType,&srXMLData,bOutData,bBuf,bInData,bBuf);
			}
			else
			{
				log_msg(LOG_LEVEL_ERROR,"inDoECCQueryTxn Fail 3:%d",inRetVal);
				if(inRetVal < SUCCESS)
					return LOAD_DLL_FUNCTION_ERROR;
				return DLL_DECLINE_FAIL;
			}
		}

		if(inTxnType == TXN_ECC_ANTENN_ACONTROL || inTxnType == TXN_ECC_FAST_READ_CARD)
			return SUCCESS;
		else if(inTxnType == TXN_ECC_READ_BASIC)//票卡查詢
		{
			if(fECCCheckETxnCard(bOutData) == TRUE)//數位付卡片
			{
				//srTxnData.srParameter.chOnlineFlag = ICER_ONLINE;
				return inDoICERTxn(TXN_E_READ_BASIC);//離開
			}
		}

		vdUpdateECCDataAfterTxn(inTxnType,bOutData);

		if(inTxnType == TXN_ECC_READ_CARD)
			return SUCCESS;

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
		if(inRetVal < SUCCESS)//沒有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)bOutData,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
		else//有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)bOutData,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
		}
	}
	else //if(srTxnData.srParameter.gReaderMode != DLL_READER)
	{
		if( inRetVal == CARD_SUCCESS ||
			inRetVal == 0x6403 ||
		  ((inRetVal == 0x640C || inRetVal == 0x640D) && inTxnType == TXN_ECC_READ_BASIC && srTxnData.srParameter.chBasicCheckQuotaFlag == '1'))
		//if(inRetVal == CARD_SUCCESS || inRetVal == 0x6403)
		{
			if(inTxnType == TXN_ECC_READ_BASIC ||
			   inTxnType == TXN_ECC_READ_TRTC_DATA ||
			   inTxnType == TXN_ECC_READ_TAXI ||
			   inTxnType == TXN_ECC_READ_THSRC ||
			   inTxnType == TXN_ECC_READ_CBIKE ||
			   inTxnType == TXN_ECC_READ_EDCA)
			{
				if(inTxnType == TXN_ECC_READ_BASIC)//票卡查詢
				{
					if(fECCCheckETxnCard(&bReadCommandOut[OUT_DATA_OFFSET]) == TRUE)//數位付卡片
					{
						//srTxnData.srParameter.chOnlineFlag = ICER_ONLINE;
						return inDoICERTxn(TXN_E_READ_BASIC);//離開
					}
				}

				inRetVal = inECCReadCardBasicCheck(inTxnType,&bReadCommandOut[OUT_DATA_OFFSET]);
				log_msg(LOG_LEVEL_FLOW,"inDoECCQueryTxn test 2:(%d)",inRetVal);
				//else //if(inTxnType == TXN_ECC_READ_TRTC_DATA)
				//	inRetVal = inECCReadTRTCCheck(bOutData);
				if(inRetVal != CARD_SUCCESS && inRetVal != ECC_LOCK_CARD)
				{
					//if(fTag554802 == TRUE)
					{
						BYTE bTmp[600 + 1];
						int inLen;

						if(inTxnType == TXN_ECC_READ_BASIC)
						{
							if(srTxnData.srParameter.chReadBasicVersion == '1')
								inLen = 0x88;
							else
								inLen = sizeof(ReadCardBasicData_APDU_Out);
						}
						else if(inTxnType == TXN_ECC_READ_TAXI)
							inLen = sizeof(TaxiRead_APDU_Out);
						else if(inTxnType == TXN_ECC_READ_CBIKE)
							inLen = sizeof(CBikeRead_APDU_Out);
						else if(inTxnType == TXN_ECC_READ_EDCA)
							inLen = sizeof(EDCARead_APDU_Out);
						else
							inLen = sizeof(TRTCRead_APDU_Out);
						memset(bTmp,0x00,sizeof(bTmp));
						fnUnPack(bOutData,inLen,bTmp);

						if(inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0) < SUCCESS)//沒有5548
						{
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)bTmp,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
							inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
						}
						else//有5548
						{
							vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)bTmp,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
							inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
						}
					}
					log_msg(LOG_LEVEL_ERROR,"inDoECCQueryTxn Fail 4:%d",inRetVal);
					return inRetVal;
				}
				else if(inRetVal == ECC_LOCK_CARD)
					return inRetVal;
			}
			else if(inTxnType == TXN_ECC_READ_CARD)
			{
				//unsigned long ulCardID = 0L;
				BYTE anCardID[17 + 1];
				ReadCardNumber_APDU_Out *DongleOut = (ReadCardNumber_APDU_Out *)bOutData;

				memset(anCardID,0x00,sizeof(anCardID));
				vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),anCardID,sizeof(anCardID) - 1,FALSE,' ',10);//n_Card Physical ID
				vdTrimData((char *)srTxnData.srIngData.anCardID,(char *)anCardID,sizeof(anCardID));
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0200,srTxnData.srIngData.anCardID,FALSE);
			}
			/*else if(inTxnType == TXN_ECC_READ_DONGLE)//缺T3700(RRN),CMAS主機暫無此功能
			{
				BYTE bBuffer[MAX_XML_FILE_SIZE];

				if(inBuildSendPackage(HOST_CMAS,&srXMLData,bBuffer,REVERSAL_OFF,ADVICE_ON,STORE_BATCH_OFF) < SUCCESS)
					log_msg(LOG_LEVEL_ERROR,"inDoECCQueryTxn Fail 4-1");
				else
					inTCPIPAdviceProcess(HOST_CMAS,FALSE);
			}*/
			else if(inTxnType == TXN_ECC_READ_DEVICE_INFO)
			{
				Reset_APDU_Out *DongleOut = (Reset_APDU_Out *)bOutData;
				//T4109 off
				memset(bBuf,0x00,sizeof(bBuf));
				fnBINTODEVASC(DongleOut->ucDeviceID,bBuf,9,MIFARE);//n_TXN Device ID
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)"T4109",bBuf,FALSE);

				//T4110 off
				memset(bBuf,0x00,sizeof(bBuf));
				fnBINTODEVASC(DongleOut->ucCPUDeviceID,bBuf,16,LEVEL2);//n_TXN Device ID
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)"T4110",bBuf,FALSE);
			}
			else if(inTxnType == TXN_ECC_FAST_READ_CARD2)
			{
				BYTE bBuf[50],bTmp[50];
				FastReadCard_APDU_Out *DongleOut = (FastReadCard_APDU_Out *)bOutData;

				//T0200
				memset(bBuf,0x00,sizeof(bBuf));
				memset(bTmp,0x00,sizeof(bTmp));
				vdUIntToAsc(DongleOut->ucCardID,sizeof(DongleOut->ucCardID),bTmp,17,TRUE,0x00,10);
				vdTrimDataRight((char *)bBuf,(char *)bTmp,17);
				inXMLAppendData(&srXMLData,(char *)TAG_NAME_0200,strlen(TAG_NAME_0200),bBuf,strlen((char *)bBuf),VALUE_NAME,FALSE);

				//T0216
				vdUnPackToXMLData(&DongleOut->ucCardIDLen,sizeof(DongleOut->ucCardIDLen),&srXMLData,(char *)TAG_NAME_0216,strlen(TAG_NAME_0216),VALUE_NAME,FALSE);

				//T0223
				vdUnPackToXMLData(DongleOut->ucSAK,sizeof(DongleOut->ucSAK),&srXMLData,(char *)TAG_NAME_0223,strlen(TAG_NAME_0223),VALUE_NAME,FALSE);

				//T4800
				vdUnPackToXMLData(&DongleOut->ucPurseVersionNumber,sizeof(DongleOut->ucPurseVersionNumber),&srXMLData,(char *)TAG_NAME_4800,strlen(TAG_NAME_4800),VALUE_NAME,FALSE);
			}
		}

		if(inRetVal == 0x6406 ||
		   inRetVal == 0x640E ||
		   inRetVal == 0x6103 ||
		   inRetVal == 0x610F ||
		   inRetVal == 0x6418)//需組鎖卡回饋檔之回應碼
		{
			vd640EProcess(inRetVal,bOutData,bReadCommandOut);

			log_msg(LOG_LEVEL_ERROR,"inDoECCQueryTxn Fail 5:%d",inRetVal);
			if(inRetVal != ECC_LOCK_CARD)
				return DLL_DECLINE_FAIL;
			else
				return inRetVal;
		}
		else if(inRetVal == ECC_LOCK_CARD)
			return inRetVal;
		else if(inRetVal != CARD_SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inDoECCQueryTxn Fail 6:%d",inRetVal);
			if(inRetVal < SUCCESS)
				return LOAD_DLL_FUNCTION_ERROR;
			return DLL_DECLINE_FAIL;
		}

		//vdUpdateECCDataAfterTxn(inTxnType,bOutData);

		if(inTxnType == TXN_ECC_ANTENN_ACONTROL || inTxnType == TXN_ECC_FAST_READ_CARD)
			return SUCCESS;

		if(inTxnType == TXN_ECC_READ_CARD)
			inRetVal = SUCCESS;

		if(inRetVal == SUCCESS || inRetVal == CARD_SUCCESS)
		{
			BYTE bTmp[600 + 1];
			int inLen;

			if(inTxnType == TXN_ECC_READ_BASIC)
			{
				if(srTxnData.srParameter.chBatchDataFlag == '1')
					inLen = sizeof(ReadCardBasicDataL2_TM_Out);
				else if(srTxnData.srParameter.chReadBasicVersion == '1')
					inLen = 0x88;
				else
					inLen = sizeof(ReadCardBasicData_APDU_Out);
			}
			else if(inTxnType == TXN_ECC_READ_DEDUCT)
			{
				if(srTxnData.srParameter.chBatchDataFlag == '1')
					inLen = sizeof(ReadCardDeduct_TM_Out);
				else
					inLen = sizeof(ReadCardDeduct_APDU_Out);
			}
			else if(inTxnType == TXN_ECC_READ_CARD)
				inLen = sizeof(ReadCardNumber_APDU_Out);
			else if(inTxnType == TXN_ECC_READ_DEVICE_INFO)
				inLen = sizeof(Reset_APDU_Out);
			else if(inTxnType == TXN_ECC_READ_TAXI)
				inLen = sizeof(TaxiRead_APDU_Out);
			else if(inTxnType == TXN_ECC_READ_CBIKE)
				inLen = sizeof(CBikeRead_APDU_Out);
			else if(inTxnType == TXN_ECC_READ_EDCA)
				inLen = sizeof(EDCARead_APDU_Out);
			else if(inTxnType == TXN_ECC_MULTI_SELECT)
				inLen = sizeof(MMSelectCard2_APDU_Out);
			else if(inTxnType == TXN_ECC_FAST_READ_CARD2)
				inLen = sizeof(FastReadCard_APDU_Out);
			else
				inLen = sizeof(TRTCRead_APDU_Out);

			//if(fTag554802 == TRUE)
			{
				memset(bTmp,0x00,sizeof(bTmp));
				if(inTxnType == TXN_ECC_READ_DEDUCT)
				{
					if(srTxnData.srParameter.chBatchDataFlag == '1')
						memcpy(bTmp,bOutData,inLen);
					else
						fnUnPack(&bReadCommandOut[OUT_DATA_OFFSET],inLen,bTmp);
				}
				else if(inTxnType == TXN_ECC_READ_BASIC)
				{
					if(srTxnData.srParameter.chBatchDataFlag == '1')
						memcpy(bTmp,bOutData,inLen);
					else
						fnUnPack(&bReadCommandOut[OUT_DATA_OFFSET],inLen,bTmp);
				}
				else
					fnUnPack(bOutData,inLen,bTmp);

				inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
				if(inRetVal < SUCCESS)//沒有5548
				{
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)bTmp,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
					inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
				}
				else//有5548
				{
					vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554801,(char *)bTmp,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
					inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
				}
			}
			/*else
			{
				memset(bBasicOut,0x00,inLen);
				memcpy(bBasicOut,bOutData,inLen);
			}*/
		}
	}

	return SUCCESS;
}

int inBuildTxRefundInData(BYTE *bOutData,unsigned long ulTMSerialNo,int inAddCnt)
{
int inCnt = 0;
UnionUlong ulDateTime;
BYTE bBuf[10];

	ulDateTime.Value = lnDateTimeToUnix((BYTE *)srTxnData.srIngData.chTxDate,(BYTE *)srTxnData.srIngData.chTxTime);
	ulDateTime.Value += inAddCnt;
	UnixToDateTime(ulDateTime.Buf,&bOutData[0],14);
	inCnt += 14;
	memset(bBuf,0x00,sizeof(bBuf));
	sprintf((char *)bBuf,"%06ld",ulTMSerialNo);
	memcpy(&bOutData[14],bBuf,6);
	inCnt += 6;

	return inCnt;
}

int inLockCard(BYTE *bCardID,BOOL bAPILock)
{
BYTE bInData[READER_BUFFER],bOutData[READER_BUFFER],bBuf[10];
int inRetVal,inSize,inCnt;

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	memset(bBuf,0x00,sizeof(bBuf));
	inCnt = 0;

	if(srTxnData.srParameter.gDLLVersion == 1)//1代API
	{
		/*
		14 TM TXN Date Time
		4 Card Physical ID
		*/
		inSize = sizeof(srTxnData.srIngData.chTxDate) - 1;
		memcpy(&bInData[inCnt],srTxnData.srIngData.chTxDate,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srIngData.chTxTime) - 1;
		memcpy(&bInData[inCnt],srTxnData.srIngData.chTxTime,inSize);
		inCnt += inSize;

		inSize = 4;
		memcpy(&bInData[inCnt],bCardID,inSize);
		inCnt += inSize;
	}
	else
	{
		/*
		7 Lock Parameter
		10 TM Location ID
		2 TM ID
		14 TM TXN Date Time
		*/
		inSize = 7;
		memcpy(&bInData[inCnt],bCardID,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID) - 1;
		memcpy(&bInData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID) - 1;
		memcpy(&bInData[inCnt],srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMID,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srIngData.chTxDate) - 1;
		memcpy(&bInData[inCnt],srTxnData.srIngData.chTxDate,inSize);
		inCnt += inSize;

		inSize = sizeof(srTxnData.srIngData.chTxTime) - 1;
		memcpy(&bInData[inCnt],srTxnData.srIngData.chTxTime,inSize);
		inCnt += inSize;
	}

	//第一道指令
	inRetVal = inReaderCommand(DLL_LOCK_CARD,&srXMLData,bInData,bOutData,TRUE,0,bAPILock);

	if(inRetVal != CARD_SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inLockCard Fail 1:%d",inRetVal);
		if(inRetVal < SUCCESS)
			return LOAD_DLL_FUNCTION_ERROR;
		return DLL_DECLINE_FAIL;
	}
	else
	{
		vd640EProcess(inRetVal,bOutData,bReadCommandOut);
#if 0
		if(srTxnData.srParameter.chBatchFlag == R6_BATCH)
			memcpy(srTxnData.srIngData.anTLKR_TLRC,bOutData,TLKR_TLRC_LEN);
		else //if(srTxnData.srParameter.chBatchFlag != SIS2_BATCH)
			fnUnPack(bOutData,SIS2_LOCK_LEN,srTxnData.srIngData.anTLKR_TLRC);

		/*if(srTxnData.srParameter.gReaderMode == DLL_READER)
			memcpy(srTxnData.srIngData.anTLKR_TLRC,bOutData,TLKR_TLRC_LEN);
		else //if(srTxnData.srParameter.gReaderMode != DLL_READER)
			fnUnPack(bOutData,SIS2_LOCK_LEN,srTxnData.srIngData.anTLKR_TLRC);*/

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
		if(inRetVal < SUCCESS)//尚未有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
		else//已有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554804,(char *)srTxnData.srIngData.anTLKR_TLRC,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
		}
#endif
		log_msg(LOG_LEVEL_ERROR,"inLockCard OK 2:%d",inRetVal);
		//return DLL_DECLINE_FAIL;
	}

	return SUCCESS;
}

int inBuildNullAPDU(STRUCT_XML_DOC *srXML,BYTE *bAPDU_Req,BYTE *bTM_In)
{

	return SUCCESS;
}

int inBuildNullTM(int inTxnType,STRUCT_XML_DOC *srXML,BYTE *API_Out,BYTE *Dongle_In,BYTE *Dongle_Out,BYTE *Dongle_ReqOut)
{
	return SUCCESS;
}

int inBuildRefundCardInData(BYTE *bOutData)
{
int inCnt = 0;
long lnAmt1,lnAmt2,lnAmt3;

	inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0416,&lnAmt1,0);
	inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0417,&lnAmt2,0);
	inXMLGetAmt(&srXMLData,(char *)TAG_NAME_0418,&lnAmt3,0);
	sprintf((char *)&bOutData[inCnt],"%05ld",lnAmt1);
	inCnt += 5;
	sprintf((char *)&bOutData[inCnt],"%05ld",lnAmt2);
	inCnt += 5;
	sprintf((char *)&bOutData[inCnt],"%05ld",lnAmt3);
	inCnt += 5;

	return inCnt;
}

int inECCReadCardBasicCheck(int inTxnType,BYTE *bInOutData)
{
int inRetVal = SUCCESS,inDLLType;
BYTE bTmp[40],ucTxnDateTime[4];
BYTE bRecvData[SIS2_ADD_VALUE_RESEND_LEN + 1];
//STRUCT_TMP_DATA srTmpData;

struct __packed_back BASIC_DATA
{
	BYTE ucPurseVersionNumber;
	BYTE bActivated 		:1;
	BYTE bBlocked			:1;
	BYTE bRefunded			:1;
	BYTE bAutoLoad			:1;
	BYTE ucAutoLoadAmt[3];
	BYTE ucPID[8];
	BYTE ucSubAreaCode[2];
	BYTE ucExpiryDate[4];
	BYTE ucEV[3];
	BYTE ucCardType;
	BYTE ucPersonalProfile;
	BYTE ucAreaCode;
	BYTE ucCardID[7];
	BYTE ucDeposit[3];
	BYTE ucBankCode;
	BYTE ucStatusCode[2];
	BYTE ucAccumulatedFreeRides[2];				   //社福卡免費搭乘累積優惠點數
	BYTE ucDateofAccumulatedFreeRides[2];		   //社福卡免費搭乘交易日期，跨月時，點數歸零重計
	BYTE ucTxnSN[3];
	BYTE ucSocial_Code[16]; 								//身份證號碼
}BasicData;
TRANSPORT_DATA URT_Data;

	memset(&BasicData,0x00,sizeof(BasicData));
	memset(&URT_Data,0x00,sizeof(URT_Data));
	memset(bTmp,0x00,sizeof(bTmp));
	memset(ucTxnDateTime,0x00,sizeof(ucTxnDateTime));
	strcat((char *)bTmp,srTxnData.srIngData.chTxDate);
	strcat((char *)bTmp,srTxnData.srIngData.chTxTime);
	fngetUnixTimeCnt(ucTxnDateTime,bTmp);

	if(inTxnType == TXN_ECC_READ_THSRC)
	{
		THSRCRead_APDU_Out *tReadBasicOut = (THSRCRead_APDU_Out *)bInOutData;

		BasicData.ucPurseVersionNumber = tReadBasicOut->ucPurseVersionNumber;
		BasicData.bActivated = tReadBasicOut->bActivated;
		BasicData.bBlocked = tReadBasicOut->bBlocked;
		BasicData.bRefunded = tReadBasicOut->bRefunded;
		BasicData.bAutoLoad = tReadBasicOut->bAutoLoad;
		memcpy(BasicData.ucAutoLoadAmt,tReadBasicOut->ucAutoLoadAmt,sizeof(BasicData.ucAutoLoadAmt));
		memcpy(BasicData.ucPID,tReadBasicOut->ucPID,sizeof(BasicData.ucPID));
		memcpy(BasicData.ucSubAreaCode,tReadBasicOut->ucSubAreaCode,sizeof(BasicData.ucSubAreaCode));
		memcpy(BasicData.ucExpiryDate,tReadBasicOut->ucExpiryDate,sizeof(BasicData.ucExpiryDate));
		memcpy(BasicData.ucEV,tReadBasicOut->ucEV,sizeof(BasicData.ucEV));
		BasicData.ucCardType = tReadBasicOut->ucCardType;
		BasicData.ucPersonalProfile = tReadBasicOut->ucPersonalProfile;
		BasicData.ucAreaCode = tReadBasicOut->ucAreaCode;
		memcpy(BasicData.ucCardID,tReadBasicOut->ucCardID,sizeof(BasicData.ucCardID));
		//memcpy(BasicData.ucDeposit,tReadBasicOut->ucDeposit,sizeof(BasicData.ucDeposit));
		BasicData.ucBankCode = tReadBasicOut->ucBankCode;
		memcpy(BasicData.ucStatusCode,tReadBasicOut->ucStatusCode,sizeof(BasicData.ucStatusCode));
		memcpy(BasicData.ucTxnSN,tReadBasicOut->ucTxnSN,sizeof(BasicData.ucTxnSN));
		memcpy((char *)&URT_Data,(char *)&tReadBasicOut->ucURT,sizeof(tReadBasicOut->ucURT));
		memcpy(BasicData.ucSocial_Code,tReadBasicOut->ucSocial_Code,sizeof(tReadBasicOut->ucSocial_Code));//ucSocial_Code
	}
	else if(inTxnType == TXN_ECC_READ_TAXI)
	{
		TaxiRead_APDU_Out *tReadBasicOut = (TaxiRead_APDU_Out *)bInOutData;

		BasicData.ucPurseVersionNumber = tReadBasicOut->ucPurseVersionNumber;
		BasicData.bActivated = tReadBasicOut->bActivated;
		BasicData.bBlocked = tReadBasicOut->bBlocked;
		BasicData.bRefunded = tReadBasicOut->bRefunded;
		BasicData.bAutoLoad = tReadBasicOut->bAutoLoad;
		memcpy(BasicData.ucAutoLoadAmt,tReadBasicOut->ucAutoLoadAmt,sizeof(BasicData.ucAutoLoadAmt));
		memcpy(BasicData.ucPID,tReadBasicOut->ucPID,sizeof(BasicData.ucPID));
		memcpy(BasicData.ucSubAreaCode,tReadBasicOut->ucSubAreaCode,sizeof(BasicData.ucSubAreaCode));
		memcpy(BasicData.ucExpiryDate,tReadBasicOut->ucExpiryDate,sizeof(BasicData.ucExpiryDate));
		memcpy(BasicData.ucEV,tReadBasicOut->ucEV,sizeof(BasicData.ucEV));
		BasicData.ucCardType = tReadBasicOut->ucCardType;
		BasicData.ucPersonalProfile = tReadBasicOut->ucPersonalProfile;
		BasicData.ucAreaCode = tReadBasicOut->ucAreaCode;
		memcpy(BasicData.ucCardID,tReadBasicOut->ucCardID,sizeof(BasicData.ucCardID));
		//memcpy(BasicData.ucDeposit,tReadBasicOut->ucDeposit,sizeof(BasicData.ucDeposit));
		BasicData.ucBankCode = tReadBasicOut->ucBankCode;
		memcpy(BasicData.ucStatusCode,tReadBasicOut->ucStatusCode,sizeof(BasicData.ucStatusCode));
		memcpy(BasicData.ucAccumulatedFreeRides,tReadBasicOut->ucAccFreeRides,sizeof(BasicData.ucAccumulatedFreeRides));
		memcpy(BasicData.ucDateofAccumulatedFreeRides,tReadBasicOut->ucAccFreeRidesDate,sizeof(BasicData.ucDateofAccumulatedFreeRides));
		memcpy(BasicData.ucTxnSN,tReadBasicOut->ucTxnSN,sizeof(BasicData.ucTxnSN));
		memcpy((char *)&URT_Data,(char *)&tReadBasicOut->ucURT,sizeof(tReadBasicOut->ucURT));
		memcpy(BasicData.ucSocial_Code,tReadBasicOut->ucSocial_Code,sizeof(tReadBasicOut->ucSocial_Code));//ucSocial_Code
		log_msg(LOG_LEVEL_FLOW,"ucAutoLoadAmt 1:%02x%02x%02x,%d",BasicData.ucAutoLoadAmt[0],BasicData.ucAutoLoadAmt[1],BasicData.ucAutoLoadAmt[2],srTxnData.srIngData.lnECCAmt);
	}
	else if(inTxnType == TXN_ECC_READ_CBIKE)
	{
		CBikeRead_APDU_Out *tReadBasicOut = (CBikeRead_APDU_Out *)bInOutData;

		BasicData.ucPurseVersionNumber = tReadBasicOut->ucPurseVersionNumber;
		BasicData.bActivated = tReadBasicOut->bActivated;
		BasicData.bBlocked = tReadBasicOut->bBlocked;
		BasicData.bRefunded = tReadBasicOut->bRefunded;
		BasicData.bAutoLoad = tReadBasicOut->bAutoLoad;
		memcpy(BasicData.ucAutoLoadAmt,tReadBasicOut->ucAutoLoadAmt,sizeof(BasicData.ucAutoLoadAmt));
		memcpy(BasicData.ucPID,tReadBasicOut->ucPID,sizeof(BasicData.ucPID));
		memcpy(BasicData.ucSubAreaCode,tReadBasicOut->ucSubAreaCode,sizeof(BasicData.ucSubAreaCode));
		memcpy(BasicData.ucExpiryDate,tReadBasicOut->ucExpiryDate,sizeof(BasicData.ucExpiryDate));
		memcpy(BasicData.ucEV,tReadBasicOut->ucEV,sizeof(BasicData.ucEV));
		BasicData.ucCardType = tReadBasicOut->ucCardType;
		BasicData.ucPersonalProfile = tReadBasicOut->ucPersonalProfile;
		BasicData.ucAreaCode = tReadBasicOut->ucAreaCode;
		memcpy(BasicData.ucCardID,tReadBasicOut->ucCardID,sizeof(BasicData.ucCardID));
		//memcpy(BasicData.ucDeposit,tReadBasicOut->ucDeposit,sizeof(BasicData.ucDeposit));
		BasicData.ucBankCode = tReadBasicOut->ucBankCode;
		memcpy(BasicData.ucStatusCode,tReadBasicOut->ucStatusCode,sizeof(BasicData.ucStatusCode));
		memcpy(BasicData.ucAccumulatedFreeRides,tReadBasicOut->ucAccFreeRides,sizeof(BasicData.ucAccumulatedFreeRides));
		memcpy(BasicData.ucDateofAccumulatedFreeRides,tReadBasicOut->ucAccFreeRidesDate,sizeof(BasicData.ucDateofAccumulatedFreeRides));
		memcpy(BasicData.ucTxnSN,tReadBasicOut->ucTxnSN,sizeof(BasicData.ucTxnSN));
		memcpy((char *)&URT_Data,(char *)&tReadBasicOut->ucURT,sizeof(tReadBasicOut->ucURT));
		memcpy(BasicData.ucSocial_Code,tReadBasicOut->ucSocial_Code,sizeof(tReadBasicOut->ucSocial_Code));//ucSocial_Code
	}
	else if(inTxnType == TXN_ECC_READ_EDCA)
	{
		EDCARead_APDU_Out *tReadBasicOut = (EDCARead_APDU_Out *)bInOutData;

		BasicData.ucPurseVersionNumber = tReadBasicOut->ucPurseVersionNumber;
		BasicData.bActivated = tReadBasicOut->bActivated;
		BasicData.bBlocked = tReadBasicOut->bBlocked;
		BasicData.bRefunded = tReadBasicOut->bRefunded;
		BasicData.bAutoLoad = tReadBasicOut->bAutoLoad;
		memcpy(BasicData.ucAutoLoadAmt,tReadBasicOut->ucAutoLoadAmt,sizeof(BasicData.ucAutoLoadAmt));
		memcpy(BasicData.ucPID,tReadBasicOut->ucPID,sizeof(BasicData.ucPID));
		memcpy(BasicData.ucSubAreaCode,tReadBasicOut->ucSubAreaCode,sizeof(BasicData.ucSubAreaCode));
		memcpy(BasicData.ucExpiryDate,tReadBasicOut->ucExpiryDate,sizeof(BasicData.ucExpiryDate));
		memcpy(BasicData.ucEV,tReadBasicOut->ucEV,sizeof(BasicData.ucEV));
		BasicData.ucCardType = tReadBasicOut->ucCardType;
		BasicData.ucPersonalProfile = tReadBasicOut->ucPersonalProfile;
		BasicData.ucAreaCode = tReadBasicOut->ucAreaCode;
		memcpy(BasicData.ucCardID,tReadBasicOut->ucCardID,sizeof(BasicData.ucCardID));
		//memcpy(BasicData.ucDeposit,tReadBasicOut->ucDeposit,sizeof(BasicData.ucDeposit));
		BasicData.ucBankCode = tReadBasicOut->ucBankCode;
		memcpy(BasicData.ucStatusCode,tReadBasicOut->ucStatusCode,sizeof(BasicData.ucStatusCode));
		memcpy(BasicData.ucAccumulatedFreeRides,tReadBasicOut->ucAccFreeRides,sizeof(BasicData.ucAccumulatedFreeRides));
		memcpy(BasicData.ucDateofAccumulatedFreeRides,tReadBasicOut->ucAccFreeRidesDate,sizeof(BasicData.ucDateofAccumulatedFreeRides));
		memcpy(BasicData.ucTxnSN,tReadBasicOut->ucTxnSN,sizeof(BasicData.ucTxnSN));
		memcpy((char *)&URT_Data,(char *)&tReadBasicOut->ucURT,sizeof(tReadBasicOut->ucURT));
		memcpy(BasicData.ucSocial_Code,tReadBasicOut->ucSocial_Code,sizeof(tReadBasicOut->ucSocial_Code));//ucSocial_Code
	}
	else
	{
		ReadCardBasicData_APDU_Out *tReadBasicOut = (ReadCardBasicData_APDU_Out *)bInOutData;

		BasicData.ucPurseVersionNumber = tReadBasicOut->ucPurseVersionNumber;
		BasicData.bActivated = tReadBasicOut->bActivated;
		BasicData.bBlocked = tReadBasicOut->bBlocked;
		BasicData.bRefunded = tReadBasicOut->bRefunded;
		BasicData.bAutoLoad = tReadBasicOut->bAutoLoad;
		memcpy(BasicData.ucAutoLoadAmt,tReadBasicOut->ucAutoLoadAmt,sizeof(BasicData.ucAutoLoadAmt));
		memcpy(BasicData.ucPID,tReadBasicOut->ucPID,sizeof(BasicData.ucPID));
		memcpy(BasicData.ucSubAreaCode,tReadBasicOut->ucSubAreaCode,sizeof(BasicData.ucSubAreaCode));
		memcpy(BasicData.ucExpiryDate,tReadBasicOut->ucExpiryDate,sizeof(BasicData.ucExpiryDate));
		memcpy(BasicData.ucEV,tReadBasicOut->ucEV,sizeof(BasicData.ucEV));
		BasicData.ucCardType = tReadBasicOut->ucCardType;
		BasicData.ucPersonalProfile = tReadBasicOut->ucPersonalProfile;
		BasicData.ucAreaCode = tReadBasicOut->ucAreaCode;
		memcpy(BasicData.ucCardID,tReadBasicOut->ucCardID,sizeof(BasicData.ucCardID));
		memcpy(BasicData.ucDeposit,tReadBasicOut->ucDeposit,sizeof(BasicData.ucDeposit));
		BasicData.ucBankCode = tReadBasicOut->ucBankCode;
		memcpy(BasicData.ucStatusCode,tReadBasicOut->ucStatusCode,sizeof(BasicData.ucStatusCode));
		memcpy(BasicData.ucAccumulatedFreeRides,tReadBasicOut->ucLoyaltyCounter,sizeof(BasicData.ucAccumulatedFreeRides));
		memcpy(BasicData.ucTxnSN,tReadBasicOut->ucTxnSN,sizeof(BasicData.ucTxnSN));
		if(tReadBasicOut->ucPurseVersionNumber == MIFARE)
			memcpy(BasicData.ucSocial_Code,tReadBasicOut->ucMifareSocialCode,sizeof(tReadBasicOut->ucMifareSocialCode));//ucSocial_Code
		else
			memcpy(BasicData.ucSocial_Code,tReadBasicOut->ucCPUSocialCode,sizeof(tReadBasicOut->ucCPUSocialCode));//ucSocial_Code
	}

	log_msg(LOG_LEVEL_FLOW,"ucAutoLoadAmt 2:%02x%02x%02x,%d",BasicData.ucAutoLoadAmt[0],BasicData.ucAutoLoadAmt[1],BasicData.ucAutoLoadAmt[2],srTxnData.srIngData.lnECCAmt);
	//是否已開卡
	if(!BasicData.bActivated)
		return ECC_CARD_NOT_ACTIVE;

	//是否已鎖卡
	if(BasicData.bBlocked)
		return ECC_CARD_BLOCKED;

	//是否已退卡
	if(BasicData.bRefunded)
		return ECC_CARD_REFUNDED;

	//卡別檢查
	if(BasicData.ucCardType == 0x09 || BasicData.ucCardType == 0x0B || BasicData.ucCardType == 0x0C)
		return ECC_CARD_TYPE_ERR;

	//黑名單檢查
	inRetVal = inCheckBLC(BasicData.ucCardID);
	if(inRetVal == SUCCESS)
	{
#if 0
		BYTE bOutData[READER_BUFFER];
		UnionUlong ulDateTime;

		memset(bOutData,0x00,sizeof(bOutData));

		ulDateTime.Value = lnDateTimeToUnix((BYTE *)srTxnData.srIngData.chTxDate,(BYTE *)srTxnData.srIngData.chTxTime);
		inRetVal = inBLCLockCard(DLL_LOCK_CARD,bOutData,BasicData.ucCardID,ulDateTime.Buf);
		//inRetVal = inReaderCommand(DLL_LOCK_CARD,&srXMLData,bInData,bOutData,TRUE,0,FALSE);

		if(inRetVal != CARD_SUCCESS && inRetVal != 0x6406)
		{
			log_msg(LOG_LEVEL_ERROR,"inECCReadCardBasicCheck Fail 1:%d",inRetVal);
			if(inRetVal < SUCCESS)
				return LOAD_DLL_FUNCTION_ERROR;
			return DLL_DECLINE_FAIL;
		}
		else
		{
			vd640EProcess(inRetVal,bOutData,bReadCommandOut);
			log_msg(LOG_LEVEL_ERROR,"inECCReadCardBasicCheck OK 2:%d",inRetVal);
			return ECC_LOCK_CARD;
		}
#else
		inRetVal = inLockCard(BasicData.ucCardID,TRUE);
		if(inRetVal == SUCCESS)
			inRetVal = ECC_LOCK_CARD;

		return inRetVal;
#endif
	}
	log_msg(LOG_LEVEL_FLOW,"ucAutoLoadAmt 3:%02x%02x%02x,%d",BasicData.ucAutoLoadAmt[0],BasicData.ucAutoLoadAmt[1],BasicData.ucAutoLoadAmt[2],srTxnData.srIngData.lnECCAmt);

	//srTxnData.srIngData.ucAutoloadCounter = tReadBasicOut->ucAutoloadCounter;
	//memcpy(srTxnData.srIngData.ucAutoloadDate,tReadBasicOut->ucAutoloadDate,sizeof(srTxnData.srIngData.ucAutoloadDate));
	//srTxnData.srIngData.ucLastSubType = tReadBasicOut->stLastCreditTxnLogInfo_t.ucSubType;
	//memcpy(srTxnData.srIngData.ucLastCPUSPID,tReadBasicOut->stLastCreditTxnLogInfo_t.ucCPUSPID,sizeof(srTxnData.srIngData.ucLastCPUSPID));
	//memcpy(srTxnData.srIngData.ucLastDateTime,tReadBasicOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,sizeof(srTxnData.srIngData.ucLastDateTime));
	srTxnData.srIngData.lnECCEV = lnAmt3ByteToLong(BasicData.ucEV);
	//srTxnData.srIngData.lnECCAutoLoadAmt = lnAmt3ByteToLong(tReadBasicOut->ucAutoLoadAmt);
	srTxnData.srIngData.anAutoloadFlag[0] = BasicData.bAutoLoad;
	//srTxnData.srIngData.ucCardType = tReadBasicOut->ucCardType;

	if(   !memcmp(BasicData.ucStatusCode,"\x90\x00",2) ||
		((!memcmp(BasicData.ucStatusCode,"\x64\x0C",2) || !memcmp(BasicData.ucStatusCode,"\x64\x0D",2)) && inTxnType == TXN_ECC_READ_BASIC && srTxnData.srParameter.chBasicCheckQuotaFlag == '1'))
	{

		if (inTxnType ==TXN_ECC_READ_BASIC && srTxnData.srParameter.chBatchFlag == SIS2_BATCH && srTxnData.srParameter.chSaveReSendDataForReadBasic == '1')
		{
		
		}
		else
		{
			//組加值重送
			memset(bRecvData,0x00,sizeof(bRecvData));
			memset(srTxnData.srIngData.anTAVR_TARC,0x00,sizeof(srTxnData.srIngData.anTAVR_TARC));
		}
#if 0//這裡不組加值重送,改到加值成功後再組!!
		inGetDeviceID(srTxnData.srIngData.anDeviceID,&srTmpData);
		inBuildSIS2AddValueResend(bRecvData,tReadBasicOut->stLastCreditTxnLogInfo_t.ucPurseVersionNumber_Index,
											0x0C,
											tReadBasicOut->stLastCreditTxnLogInfo_t.ucSubType,
											tReadBasicOut->stLastCreditTxnLogInfo_t.ucCPUDeviceID,
											tReadBasicOut->stLastCreditTxnLogInfo_t.ucCPUSPID,
											tReadBasicOut->stLastCreditTxnLogInfo_t.ucTxnDateTime,
											tReadBasicOut->ucCardID,
											tReadBasicOut->ucIssuerCode,
											tReadBasicOut->stLastCreditTxnLogInfo_t.ucTxnSN,
											tReadBasicOut->stLastCreditTxnLogInfo_t.ucTxnAmt,
											tReadBasicOut->stLastCreditTxnLogInfo_t.ucEV,
											tReadBasicOut->stLastCreditTxnLogInfo_t.ucCPULocationID,
											tReadBasicOut->ucCardIDLen,
											tReadBasicOut->ucPersonalProfile,
											tReadBasicOut->ucPID,
											tReadBasicOut->ucBankCode,
											tReadBasicOut->ucLoyaltyCounter,
											srTmpData.ucDeviceID,
											srTmpData.ucSPID,
											srTmpData.ucLocationID,
											srTmpData.ucCPUDeviceID,
											srTmpData.ucCPUSPID,
											srTmpData.ucCPULocationID);

		fnUnPack(bRecvData,SIS2_ADD_VALUE_RESEND_LEN,srTxnData.srIngData.anTAVR_TARC);
#endif
		
		if(inTxnType == TXN_ECC_READ_TAXI)
			inDLLType = DLL_TAXI_READ;
		else if(inTxnType == TXN_ECC_READ_CBIKE)
			inDLLType = DLL_CBIKE_READ;
		else if(inTxnType == TXN_ECC_READ_EDCA)
			inDLLType = DLL_EDCA_READ;
		else
			inDLLType = DLL_READ_CARD_BASIC_DATA;
		log_msg(LOG_LEVEL_FLOW,"ucAutoLoadAmt 4:%02x%02x%02x,%d",BasicData.ucAutoLoadAmt[0],BasicData.ucAutoLoadAmt[1],BasicData.ucAutoLoadAmt[2],srTxnData.srIngData.lnECCAmt);
		vdUpdateCardInfo2(inDLLType,
						 BasicData.ucCardID,
						 BasicData.ucPID,
						 (BYTE )BasicData.bAutoLoad,
						 BasicData.ucCardType,
						 BasicData.ucPersonalProfile,
						 BasicData.ucAutoLoadAmt,
						 BasicData.ucEV,
						 BasicData.ucExpiryDate,
						 BasicData.ucPurseVersionNumber,
						 BasicData.ucBankCode,
						 BasicData.ucAreaCode,
						 BasicData.ucSubAreaCode,
						 BasicData.ucDeposit,
						 BasicData.ucTxnSN);

		memset(bTmp,0x00,sizeof(bTmp));
		fnUnPack(BasicData.ucSocial_Code,sizeof(BasicData.ucSocial_Code),bTmp);//ucSocial_Code
		inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0222,bTmp,FALSE);

		if(inTxnType == TXN_ECC_READ_BASIC && srTxnData.srParameter.chReadBasicVersion != '1')
		{
			ReadCardBasicData_APDU_Out *tReadBasicOut = (ReadCardBasicData_APDU_Out *)bInOutData;
			
			memset(bTmp,0x00,sizeof(bTmp));
			fnUnPack(tReadBasicOut->RetailCardId,sizeof(tReadBasicOut->RetailCardId),bTmp);//RetailCardId
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4841,bTmp,FALSE);

			memset(bTmp,0x00,sizeof(bTmp));
			vdIntToAsc(BasicData.ucAccumulatedFreeRides,2,bTmp,5,TRUE,0x00,10);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4852,bTmp,FALSE);
			// 淡海輕軌累積回饋金額
			memset(bTmp, 0x00, sizeof(bTmp));
			fnUnPack(tReadBasicOut->TLRTACCTXNValue, sizeof(tReadBasicOut->TLRTACCTXNValue), bTmp);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4853,bTmp,FALSE);
		}
		else if(inTxnType == TXN_ECC_READ_TAXI || inTxnType == TXN_ECC_READ_EDCA)
		{
			BYTE bData[30],bT553301[10 + 1],bT553307[8 + 1],bT553309[5 + 1],bT553311[8 + 1];

			if(inTxnType == TXN_ECC_READ_EDCA)
			{
				memset(bData,0x00,sizeof(bData));
				fngetDosDate(((EDCARead_APDU_Out *)bInOutData)->ucDateOfFirstTransaction,bData);
				inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4843,bData,FALSE);
			}

			memset(bData,0x00,sizeof(bData));
			if(inTxnType == TXN_ECC_READ_TAXI)
				sprintf((char *)bData,"%02x",((TaxiRead_APDU_Out *)bInOutData)->ucPersonalProfileAuth[0]);
			else //if(inTxnType == TXN_ECC_READ_EDCA)
				sprintf((char *)bData,"%02x",((EDCARead_APDU_Out *)bInOutData)->ucPersonalProfileAuthorization[0]);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4844,bData,FALSE);

			vdURTDataProcess(&srXMLData,BasicData.ucPurseVersionNumber,&URT_Data);

			memset(bT553301,0x00,sizeof(bT553301));
			memset(bT553307,0x00,sizeof(bT553307));
			memset(bT553309,0x00,sizeof(bT553309));
			memset(bT553311,0x00,sizeof(bT553311));

			memset(bData,0x00,sizeof(bData));
			if(inTxnType == TXN_ECC_READ_TAXI)
				UnixToDateTime(((TaxiRead_APDU_Out *)bInOutData)->ucProfileExpiryDate,bData,8);//Expiry Date
			else //if(inTxnType == TXN_ECC_READ_EDCA)
				UnixToDateTime(((EDCARead_APDU_Out *)bInOutData)->ucProfileExpiryDate,bData,8);//Expiry Date
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_1404,bData,FALSE);

			sprintf((char *)bT553301,"WELFARE%02X",BasicData.ucAreaCode);
			sprintf((char *)bT553307,"WELFARE");
			vdIntToAsc(BasicData.ucAccumulatedFreeRides,sizeof(BasicData.ucAccumulatedFreeRides),bT553309,5,TRUE,0x00,10);
			fngetDosDate(BasicData.ucDateofAccumulatedFreeRides,bT553311);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5533,(char *)bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_553301,(char *)bT553301,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_553307,(char *)bT553307,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_553309,(char *)bT553309,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_553311,(char *)bT553311,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5533,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
		else if(inTxnType == TXN_ECC_READ_CBIKE)
		{
			BYTE bT484200[1 + 1],bT484201[14 + 1],bT484202[8 + 1],bT484203[2 + 1];

			memset(bT484200,0x00,sizeof(bT484200));
			memset(bT484201,0x00,sizeof(bT484201));
			memset(bT484202,0x00,sizeof(bT484202));
			memset(bT484203,0x00,sizeof(bT484203));

			vdURTDataProcess(&srXMLData,BasicData.ucPurseVersionNumber,&URT_Data);

			bT484200[0] = '0';
			if((((CBikeRead_APDU_Out *)bInOutData)->ucRentalFlag[0] & 0x80) == 0x80)
				bT484200[0] = '1';
			UnixToDateTime(((CBikeRead_APDU_Out *)bInOutData)->ucTransactionDateAndTime,(unsigned char*)bT484201,14);
			vdUIntToAsc(((CBikeRead_APDU_Out *)bInOutData)->ucCBikePlateNumber,3,bT484202,8,TRUE,0x00,10);
			fnUnPack(((CBikeRead_APDU_Out *)bInOutData)->ucAreaCodeOfMembership,1,bT484203);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_4842,(char *)bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484200,(char *)bT484200,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484201,(char *)bT484201,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484202,(char *)bT484202,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_484203,(char *)bT484203,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_4842,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
	}

	return CARD_SUCCESS;
}

BOOL bIsResetOfflineTxn(int inTransType)
{

	if(inTransType == TXN_ECC_READ_THSRC)
		return TRUE;
	else
		return FALSE;
}

BOOL f6403Check(STRUCT_XML_DOC *srXML,BYTE *bAPDUInData,BYTE *bAPDUOutData,int inAPDURet)
{

	if(inAPDURet == 0x6403)
		return TRUE;
	else if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT)
		return FALSE;
	else if(inAPDURet == 0x9000 && fIsDeductTxn(srTxnData.srIngData.inTransType))
	//else if(inAPDURet == 0x9000 && (srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_TAXI || srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_EDCA))
	{
		long lnEV;

		if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_TAXI)
			lnEV = lnAmt3ByteToLong(((TaxiRead_APDU_Out *)bAPDUOutData)->ucEV);
		else if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_EDCA)
			lnEV = lnAmt3ByteToLong(((EDCARead_APDU_Out *)bAPDUOutData)->ucEV);
		else if(srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT_CBIKE)
			lnEV = lnAmt3ByteToLong(((CBikeRead_APDU_Out *)bAPDUOutData)->ucEV);

		//允許負值卡0元扣款 以及 交易金額為0 --> 回傳非餘額不足
		if(srTxnData.srParameter.chNegativeZeroDeduct == '1' && srTxnData.srIngData.lnECCAmt == 0)
			return FALSE;

		if(lnEV < srTxnData.srIngData.lnECCAmt)
			return TRUE;
	}

	return FALSE;
}

int inDoICERTxn(int inTxnType)
{
BYTE bT6407Flag = FALSE,bInData[READER_BUFFER],bOutData[READER_BUFFER],bBuf[20],bReadPurseFlag[2];
int inRetVal = SUCCESS,inCnt = 0,inSize,inTMDataLen;
int inDLLTrans1 = 0/*,inDLLTrans2 = 0*/;
char chOnlineFlag;//Online Flag,0=R6 / 1=CMAS
char chBatchFlag;//Batch Flag,0=CMAS Batch / 1=SIS2 Batch / 2=R6
char chTCPIP_SSL;
char PacketLenFlag;//PacketLenFlag 0=電文沒有2Byte長度 / 1=電文有2Byte長度

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	memset(bBuf,0x00,sizeof(bBuf));
	srTxnData.srParameter.chOnlineFlag = ICER_ONLINE;
	srTxnData.srParameter.PacketLenFlag = '1';
	srTxnData.srParameter.chTCPIP_SSL = '0';

	chOnlineFlag = srTxnData.srParameter.chOnlineFlag;
	chBatchFlag = srTxnData.srParameter.chBatchFlag;
	chTCPIP_SSL = srTxnData.srParameter.chTCPIP_SSL;
	PacketLenFlag = srTxnData.srParameter.PacketLenFlag;

	//ICER的SignOn要看參數chETxnSignOnMode,來決定走哪個主機!!
	if(srTxnData.srParameter.chETxnSignOnMode == '1')//R6 VPN
	{
		srTxnData.srParameter.chOnlineFlag = R6_ONLINE;
		srTxnData.srParameter.chBatchFlag = R6_BATCH;
		srTxnData.srParameter.chTCPIP_SSL = '0';
		srTxnData.srParameter.PacketLenFlag = '1';
	}
	else if(srTxnData.srParameter.chETxnSignOnMode == '2')//CMAS VPN
	{
		srTxnData.srParameter.chOnlineFlag = CMAS_ONLINE;
		srTxnData.srParameter.chBatchFlag = CMAS_BATCH;
		srTxnData.srParameter.chTCPIP_SSL = '0';
		srTxnData.srParameter.PacketLenFlag = '1';
	}
	else if(srTxnData.srParameter.chETxnSignOnMode == '3')//CMAS Internet
	{
		srTxnData.srParameter.chOnlineFlag = CMAS_ONLINE;
		srTxnData.srParameter.chBatchFlag = CMAS_BATCH;
		srTxnData.srParameter.chTCPIP_SSL = '1';
		srTxnData.srParameter.PacketLenFlag = '0';
	}

	inRetVal = inSignOnQueryProcess(bInData,bOutData);

	srTxnData.srParameter.chOnlineFlag = chOnlineFlag;
	srTxnData.srParameter.chBatchFlag = chBatchFlag;
	srTxnData.srParameter.chTCPIP_SSL = chTCPIP_SSL;
	srTxnData.srParameter.PacketLenFlag = PacketLenFlag;

	if(srTxnData.srParameter.chETxnBatchFlag >= R6_BATCH && srTxnData.srParameter.chETxnBatchFlag <= ICER_BATCH)
		srTxnData.srParameter.chBatchFlag = srTxnData.srParameter.chETxnBatchFlag;

	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inDoICERTxn Fail 1:%d",inRetVal);
		return inRetVal;
	}

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));

	if(inTxnType >= TXN_E_RETRY_QUERY)
	{
		inDLLTrans1 = DLL_E_QR_TXN;
		memset(bReadCommandOut,0x00,sizeof(bReadCommandOut));
	}
	else
	//if(inTxnType == TXN_E_ADD)
	{
		inDLLTrans1 = DLL_E_ADD_VALUE;
		inSize = 2;
		memcpy(&bInData[inCnt],"02",inSize);
		inCnt += inSize;

		inSize = 2;
		memcpy(&bInData[inCnt],"30",inSize);
		inCnt += inSize;
	}

	inCnt += inBuildAPIDefaultData(inDLLTrans1,srTxnData.srIngData.ulTMSerialNumber,&bInData[inCnt],&inRetVal);//TM序號不加1,以免影響Retry機制!!

	inTMDataLen = TM2_DATA_LEN;

	inCnt += inBuildTxnAmt(&bInData[inCnt],srTxnData.srIngData.lnECCAmt);

	inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_4830,(char *)bReadPurseFlag,1,0);
	if(inRetVal >= SUCCESS)//沒有4830
	{
		if(bReadPurseFlag[0] == '1')
			bReadPurseFlag[0] = 0x01;
		else
			bReadPurseFlag[0] = 0x00;
	}
	else
		bReadPurseFlag[0] = 0x00;

	bInData[inCnt++] = bReadPurseFlag[0];

	memcpy(srTxnData.srIngData.bTMData,&bInData[4],inTMDataLen);

	inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
	log_msg(LOG_LEVEL_ERROR,"inDoICERTxn 1:%d",inRetVal);

	if(inRetVal == 0x6304 || inRetVal == 0x6305 || inRetVal == 0x630D)
	{
		if( srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY ||
			srTxnData.srParameter.chSignOnMode == RESET_OFF_ONLY ||
			srTxnData.srParameter.chSignOnMode == SIGN_ON_AND_RESET_OFF ||
			srTxnData.srParameter.chSignOnMode == NO_SIGN_ON_AND_RESET)
			inRetVal = inSignOnQueryProcess2(inRetVal,bInData,bOutData);
		else
		{
			if(inRetVal == 0x6304 || inRetVal == 0x6305)
				inRetVal = inSignOnProcess(TXN_ECC_SIGN_ON,bOutData,TRUE);
			/*else if(inTxnType == TXN_ECC_READ_DEVICE_INFO)//Reset不會回這個!!
			{
				int inType = inDLLTrans1;

				inDLLTrans1 = DLL_MATCH2;
				inBuildAPIDefaultData(inDLLTrans1,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);//TM序號不加1,以免影響Retry機制!!
				inDLLTrans1 = inType;
				inRetVal = CARD_SUCCESS;
			}*/
			else //if(inRetVal == 0x630D)
				inRetVal = inSignOnProcess(TXN_ECC_MATCH,bOutData,TRUE);
		}
		if(inRetVal != CARD_SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inDoICERTxn Fail 2:%d",inRetVal);
			return inRetVal;
		}

		//在執行一次扣款第一道指令
		inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,FALSE,0,FALSE);
	}

	if(inRetVal == 0x6415)
	{
		memset(bInData,0x00,sizeof(bInData));
		inCnt = 0;

		if(strlen((char *)srTxnData.srREQData.bT0200) > 0)//POS傳卡號時,需檢查是否相同卡片
		{
			BYTE bT0200[10 + 1];

			memset(bT0200,0x00,sizeof(bT0200));
			vdUIntToAsc(((TxnReqOnline_APDU_Out *)bOutData)->ucCardID,7,bT0200,sizeof(bT0200) - 1,TRUE,0x00,10);//n_Card Physical ID
			if(strlen((char *)srTxnData.srREQData.bT0200) != strlen((char *)bT0200) || memcmp(srTxnData.srREQData.bT0200,bT0200,strlen((char *)bT0200)))
			{
				srTxnData.srIngData.inReaderSW = 0x6404;
				log_msg(LOG_LEVEL_ERROR,"inDoICERTxn Fail 3-0:%04X",srTxnData.srIngData.inReaderSW);
				return DLL_DECLINE_FAIL;
			}
		}

		inRetVal = inSendRecvXML(HOST_ICER,&srXMLData,inTxnType);
		log_msg(LOG_LEVEL_FLOW,"inDoICERTxn 2:%d",inRetVal);
		if(inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inDoICERTxn Fail 3:%d",inRetVal);
			return inRetVal;
		}

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_6407,(char *)bBuf,0,0);
		log_msg(LOG_LEVEL_ERROR,"inDoICERTxn 3:%d",inRetVal);
		if(inRetVal >= SUCCESS)//有6407
		{
			inDLLTrans1 = DLL_VERIFY_HOST_CRYPT;
			if(inTxnType != TXN_E_READ_BASIC)
				bT6407Flag = TRUE;
			if(inTxnType >= TXN_E_RETRY_QUERY)
				inDLLTrans1 = DLL_QR_VERIFY_HOST_CRYPT;
			inRetVal = inReaderCommand(inDLLTrans1,&srXMLData,bInData,bOutData,TRUE,1,FALSE);
			if(inRetVal != CARD_SUCCESS)
			{
				log_msg(LOG_LEVEL_ERROR,"inDoICERTxn Fail 4:%d",inRetVal);

				if(inRetVal < SUCCESS)
					return inRetVal;

				return DLL_DECLINE_FAIL;
			}
		}

		if(inTxnType == TXN_QUERY_POINT)
		{
			vdUpdateCardInfo2(inDLLTrans1,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucCardID,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucPID,
						 (BYTE )((TxnReqOnline_APDU_Out *)bOutData)->bAutoLoad,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucCardType,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucPersonalProfile,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucAutoLoadAmt,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucEV,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucExpiryDate,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucPurseVersionNumber,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucBankCode,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucAreaCode,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucSubAreaCode,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucDeposit,
						 ((TxnReqOnline_APDU_Out *)bOutData)->ucTxnSN);
		}
	}
	else if(inRetVal == CARD_SUCCESS)//回應碼9000,第二道指令的Input_Data直接帶0
	{
		if(srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)//該Online沒Online,表示Retry
			vdReversalOff(HOST_CMAS);
	}
	else if(inRetVal == 0x6406 ||
			inRetVal == 0x640E ||
			inRetVal == 0x6103 ||
			inRetVal == 0x610F ||
			inRetVal == 0x6418)//需組鎖卡回饋檔之回應碼
	{
		if(inRetVal == 0x6406)
			return ECC_LOCK_CARD;
		else
			return DLL_DECLINE_FAIL;
	}
	else if(inRetVal != CARD_SUCCESS)//失敗之回應碼
	{
		log_msg(LOG_LEVEL_ERROR,"inDoICERTxn Fail 5:%d",inRetVal);
		if(inRetVal < SUCCESS)
			return inRetVal;

		if(inRetVal < SUCCESS)
			return LOAD_DLL_FUNCTION_ERROR;
		return DLL_DECLINE_FAIL;
	}

	if(bT6407Flag == TRUE && (srTxnData.srParameter.chBatchFlag == SIS2_BATCH || srTxnData.srParameter.chBatchFlag == R6_BATCH || srTxnData.srParameter.chETxnBatchFlag == SIS2_BATCH))
	{
		int inSTMCLen;

		memset(srTxnData.srIngData.anSETM_STMC_1,0x00,sizeof(srTxnData.srIngData.anSETM_STMC_1));
		memset(srTxnData.srIngData.anSETM_STMC_2,0x00,sizeof(srTxnData.srIngData.anSETM_STMC_2));

		if(srTxnData.srParameter.chBatchFlag == SIS2_BATCH || srTxnData.srParameter.chETxnBatchFlag == SIS2_BATCH)
		{
			inSTMCLen = sizeof(SIS2_HEADER) + sizeof(SIS2_END);

			if(inTxnType == TXN_E_DEDUCT || inTxnType == TXN_E_QR_DEDUCT)//扣款交易
				inSTMCLen += sizeof(SIS2_BODY_DEDUCT);
			else if(inTxnType == TXN_E_ADD || inTxnType == TXN_E_REFUND || inTxnType == TXN_E_VOID_ADD ||
					inTxnType == TXN_E_QR_ADD || inTxnType == TXN_E_QR_REFUND || inTxnType == TXN_E_QR_VOID_ADD)
				inSTMCLen += sizeof(SIS2_BODY_ADD);

			fnUnPack(bOutData,inSTMCLen,srTxnData.srIngData.anSETM_STMC_1);
			//log_msg_debug(LOG_LEVEL_ERROR,TRUE,"bOutData",8,bOutData,inSTMCLen);
		}
		else
		{
			inSTMCLen = SIZE_STMC;

			memcpy(srTxnData.srIngData.anSETM_STMC_1,bOutData,inSTMCLen);
		}
		log_msg(LOG_LEVEL_ERROR,"inDoICERTxn 4:%d",inRetVal);

		inRetVal = inXMLGetData(&srXMLData,(char *)TAG_NAME_5548,(char *)bBuf,0,0);
		if(inRetVal < SUCCESS)//沒有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,TRUE,NODE_NO_SAME,TAG_NAME_START,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,FALSE,NODE_NO_ADD,VALUE_NAME,FALSE);
			//if(inTxnType == TXN_E_DEDUCT)//扣款或餘退要多帶554805
			//	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_5548,(char *)srTxnData.srIngData.bTmp,FALSE,NODE_NO_DEC,TAG_NAME_END,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/TRANS",FALSE);
		}
		else//有5548
		{
			vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554802,(char *)srTxnData.srIngData.anSETM_STMC_1,TRUE,NODE_NO_SAME,VALUE_NAME,FALSE);
			//if(inTxnType == TXN_E_DEDUCT)
			//	vdSetXMLActionData(&srXMLActionData,(char *)TAG_NAME_554805,(char *)srTxnData.srIngData.anTAVR_TARC,FALSE,NODE_NO_SAME,VALUE_NAME,FALSE);
			inXMLInsertData(&srXMLData,&srXMLActionData,(char *)"/T5548",FALSE);
		}

		return SUCCESS;
	}

	return SUCCESS;
}

int inDoICERQRTxn(int inTxnType)
{
#if 0
BYTE bInData[READER_BUFFER],bOutData[READER_BUFFER],bBuf[20],bReadPurseFlag[2];
int inRetVal = SUCCESS,inCnt = 0,inSize,inTMDataLen;
int inDLLTrans1 = 0/*,inDLLTrans2 = 0*/;
char chOnlineFlag;//Online Flag,0=R6 / 1=CMAS
char chBatchFlag;//Batch Flag,0=CMAS Batch / 1=SIS2 Batch / 2=R6
char chTCPIP_SSL;
char PacketLenFlag;//PacketLenFlag 0=電文沒有2Byte長度 / 1=電文有2Byte長度

	memset(bInData,0x00,sizeof(bInData));
	memset(bOutData,0x00,sizeof(bOutData));
	memset(bBuf,0x00,sizeof(bBuf));

	inRetVal = inReaderCommand(DLL_E_QR_TXN,&srXMLData,bInData,bOutData,FALSE,0,FALSE);

	if(inRetVal == CARD_SUCCESS)
	{
		memset(bInData,0x00,sizeof(bInData));
		inCnt = 0;

		if(strlen((char *)srTxnData.srREQData.bT0200) > 0)//POS傳卡號時,需檢查是否相同卡片
		{
			BYTE bT0200[10 + 1];

			memset(bT0200,0x00,sizeof(bT0200));
			vdUIntToAsc(((TxnReqOnline_APDU_Out *)bOutData)->ucCardID,7,bT0200,sizeof(bT0200) - 1,FALSE,' ',10);//n_Card Physical ID
			if(strlen((char *)srTxnData.srREQData.bT0200) != strlen((char *)bT0200) || memcmp(srTxnData.srREQData.bT0200,bT0200,strlen((char *)bT0200)))
			{
				srTxnData.srIngData.inReaderSW = 0x6404;
				log_msg(LOG_LEVEL_ERROR,"inDoICERQRTxn Fail 3-0:%04X",srTxnData.srIngData.inReaderSW);
				return DLL_DECLINE_FAIL;
			}
		}

		inRetVal = inSendRecvXML(HOST_ICER,&srXMLData,inTxnType);
		if(inRetVal != SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inDoICERQRTxn Fail 3:%d",inRetVal);
			return inRetVal;
		}
	}

	/*else if(inRetVal == CARD_SUCCESS)//回應碼9000,第二道指令的Input_Data直接帶0
	{
		if(srTxnData.srParameter.chOnlineFlag == ICER_ONLINE)//該Online沒Online,表示Retry
			vdReversalOff(CMAS_BATCH);
	}
	else if(inRetVal == 0x6406 ||
			inRetVal == 0x640E ||
			inRetVal == 0x6103 ||
			inRetVal == 0x610F ||
			inRetVal == 0x6418)//需組鎖卡回饋檔之回應碼
	{
		if(inRetVal == 0x6406)
			return ECC_LOCK_CARD;
		else
			return DLL_DECLINE_FAIL;
	}*/
	else if(inRetVal != CARD_SUCCESS)//失敗之回應碼
	{
		log_msg(LOG_LEVEL_ERROR,"inDoICERQRTxn Fail 4:%d",inRetVal);
		if(inRetVal < SUCCESS)
			return inRetVal;

		if(inRetVal < SUCCESS)
			return LOAD_DLL_FUNCTION_ERROR;
		return DLL_DECLINE_FAIL;
	}
#endif

	return SUCCESS;
}

int inResetOfflineProcess(int inDLLType,STRUCT_XML_DOC *srXML,BYTE *bInData,BYTE *bOutData)
{
int inRetVal;

	inBuildAPIDefaultData(inDLLType,srTxnData.srIngData.ulTMSerialNumber,bInData,&inRetVal);
	inRetVal = inReaderCommand(inDLLType,srXML,bInData,bOutData,FALSE,0,FALSE);
	if(inRetVal == 0x630D && inDLLType == DLL_RESET_OFF)
		inRetVal = inReaderCommand(DLL_MATCH_OFF,srXML,bInData,bOutData,FALSE,0,FALSE);

	if(inRetVal != CARD_SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inResetOfflineProcess Fail 1:%d",inRetVal);
		return DLL_DECLINE_FAIL;
	}

	if(srTxnData.srParameter.gReaderMode == DLL_READER || srTxnData.srParameter.chBatchFlag == R6_BATCH)
	{
		Reset_TM_In *tDongleIn = (Reset_TM_In *)bInData;
		Reset_TM_Out *tDongleOut = (Reset_TM_Out *)bOutData;
		BYTE ucLocationID;
		BYTE ucCPULocationID[2],bBuf[10];

		memset(bBuf,0x00,sizeof(bBuf));
		fnASCTOBIN(&ucLocationID,tDongleIn->ucLocationID,3,1,DECIMAL);//舊場站代碼
		fnASCTOBIN(ucCPULocationID,tDongleIn->ucCPULocationID,5,2,DECIMAL);//新場站代碼
		vdStoreDeviceID(tDongleOut->ucDeviceID,tDongleOut->ucDeviceID[2],ucLocationID,tDongleOut->ucCPUDeviceID,&tDongleOut->ucCPUDeviceID[3],ucCPULocationID,gTmpData.ucReaderID,gTmpData.ucReaderFWVersion,tDongleOut->ucCPUSAMID);
	}
	else //if(srTxnData.srParameter.gReaderMode == RS232_READER || srTxnData.srParameter.gReaderMode == LIBARY_READER)
	{
		Reset_TM_In *tDongleIn = (Reset_TM_In *)bInData;
		Reset_APDU_Out *tDongleOut = (Reset_APDU_Out *)bOutData;
		BYTE ucLocationID;
		BYTE ucCPULocationID[2];

		fnASCTOBIN(&ucLocationID,tDongleIn->ucLocationID,3,1,DECIMAL);//舊場站代碼
		fnASCTOBIN(ucCPULocationID,tDongleIn->ucCPULocationID,5,2,DECIMAL);//新場站代碼
		vdStoreDeviceID(tDongleOut->ucDeviceID,tDongleOut->ucDeviceID[2],ucLocationID,tDongleOut->ucCPUDeviceID,&tDongleOut->ucCPUDeviceID[3],ucCPULocationID,tDongleOut->ucReaderID,tDongleOut->ucReaderFWVersion,tDongleOut->ucCPUSAMID);
	}

	return inRetVal;
}

BOOL fECCCheckETxnCard(BYTE *bOutData)
{
ReadCardBasicData_APDU_Out *tReadBasicOut = (ReadCardBasicData_APDU_Out *)bOutData;
ReadCardBasicDataL2_TM_Out *srSTMC_Info = (ReadCardBasicDataL2_TM_Out *)bOutData;

	if(srTxnData.srParameter.gReaderMode == DLL_READER || srTxnData.srParameter.chBatchFlag == R6_BATCH)
	{
		if(srSTMC_Info->ucActivated == '0' && !memcmp(srSTMC_Info->anCardType,"0C",2))
			return TRUE;
	}
	else
	{
		if(!tReadBasicOut->bActivated && tReadBasicOut->ucCardType == 0x0C)//未開+CardType=C
			return TRUE;
	}

	return FALSE;
}


