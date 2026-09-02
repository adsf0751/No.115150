typedef struct
{
	char	szCUPFuncEnable[2 + 1];			/* 是否開啟CUP功能，該參數值為(Y/N)，預設為N */
	char	szFiscFuncEnable[2 + 1];		/* 是否開啟SmartPay功能，該參數值為(Y/N)，預設為N*/
	char	szECRComPort[4 + 1];			/* 決定ECR用哪一個ComPort */
	char	szECRVersion[2 + 1];			/* 決定哪一個ECR版本 */
	char	szPABXCode[12 + 1];			/* 撥外線的PABXCode */
	char	szTSAMRegisterEnable[2 + 1];		/* 是否已註冊tSAM，該參數值為(Y/N)，預設為N */
        char    szTMSOK[2 + 1];				/* TMS下載是否成功，該參數值為(Y/N)，預設為N */
	char	szDCCInit[2 + 1];			/* 是否還沒下過第一次DCC參數，該參數值為(1/0)，預設為1，新裝機或重裝機設為1，只要執行DCC下載過後無論成功與否，都設為0，若此bit為1則主機吐FullFileList (Installation Indicator, Request Message: If the terminal is newly installed, or re-installed, or re-initialized, the value is ‘1’ Else, the value is ‘0’)*/
	char	szDCCDownloadMode[2 + 1];		/* DCC排程下載模式 0:除非手動否則不下載 1:整點提示(如果沒有舊參數又下載失敗) 2:立即下載(用於TMS連動DCC重開機立即下載，在TMS更新完後從0設為2)*/
	char	szDCCLastUpdateDate[6 + 1];		/* YY/MM/DD 上次啟動DCC下載的時間 */
	char	szDCCSettleDownload[2 + 1];		/* 是否要在結帳後，再做一次DCC參數下載(DCC排程下載失敗時觸發，只有在結帳完或下一次自動結帳會重置) */
	char    szDCCBinVer[4 + 1];                     /* DCC BinTable Version */
        char    szEDCLOCK[2 + 1];			/* EDC鎖機 */
        char    szISODebug[20 + 1];			/* Debug列印模式(有開則設為1)，[0]＝ginDebug,[1]=ginISODebug,[2]=ginDisplayDebug,[3]ginEngineerDebug,[4]ESC Debug */
        char    szManagerPassword[12 + 1];		/* 工程師管理密碼 */
	char    szFunctionPassword[6 + 1];		/* 裝機工程師管理密碼 */
	char    szMerchantPassword[6 + 1];		/* 商店管理密碼 */
	char    szSuperPassword[6 + 1];			/* ??管理密碼 */
        char    szMCCCode[4 + 1];			
        char    szIssuerID[4 + 1];
        char    szEnterTimeout[4 + 1];			/* 輸入逾時時間 */
        char    szIPSendTimeout[2 + 1];			/* IP Send TimeOut時間 */
        char    szTermVersionID[16 + 1];		/* TermVersionID (目前不使用)*/
        char    szTermVersionDate[16 + 1];		/* TermVersionDate (目前不使用)*/
        char    szTermIPAddress[16 + 1];		/* EDC IP */
        char    szTermGetewayAddress[16 + 1];		/* TermGetewayAddress */
        char    szTermMASKAddress[16 + 1];		/* TermMASKAddress */
	char	szTermECRPort[6 + 1];			/* 當Server端時的port */
	char	szESCMode[2 + 1];			/* 是否要加上NE這個欄位 */
	char	szMultiComPort1[4 + 1];			/* 多接設備接口1(目前最多同時四個port，所以先開好欄位) */
	char	szMultiComPort1Version[2 + 1];		/* 決定第一個外接設備版本 */
	char	szMultiComPort2[4 + 1];			/* 多接設備接口2 */
	char	szMultiComPort2Version[2 + 1];		/* 決定第二個外接設備版本 */
	char	szMultiComPort3[4 + 1];			/* 多接設備接口3 */
	char	szMultiComPort3Version[2 + 1];		/* 決定第三個外接設備版本 */
	char	szEMVForceOnline[2 + 1];		/* 是否強制EMV kernel Online */
	char	szAutoConnect[10 + 1];			/* 是否自動連接(有開則設為1)，[0]＝Ethernet,[1]=WiFi, */
	char	szLOGONum[4 + 1];			/* LOGO編號 */
	char	szHostSAMSlot[2 + 1];			/* 信用卡SAM放的位置，一般來說固定為1 */
	char	szSAMSlotSN1[16 + 1];			/* 第一個SAM的SN */
	char	szSAMSlotSN2[16 + 1];			/* 第二個SAM的SN */
	char	szSAMSlotSN3[16 + 1];			/* 第三個SAM的SN */
	char	szSAMSlotSN4[16 + 1];			/* 第四個SAM的SN */
	char	szPWMEnable[2 + 1];			/* 是否開啟電源管理功能 */
	char	szPWMMode[2 + 1];			/* 設為0是Standby，設為1是Sleep */
	char	szPWMIdleTimeout[4 + 1];		/* 進省電模式Timeout */
	char	szDemoMode[2 + 1];			/* 是否開啟教育訓練模式 */
	char	szKBDLock[2 + 1];			/* 是否鎖住鍵盤螢幕 */
	char	szLastDownloadMode[2 + 1];		/* 上一次TMS下載的下載模式，用以判定更新參數的動作，Default為0，自動下載為1，手動下載為2 */
	char	szPMPassword[8 + 1];			/* Program Manager Password */
	char	szFunKeyPassword[8 + 1];		/* FunctionKey Password */
	char	szTMSDownloadTimes[6 + 1];		/* TMS下載次數 */
	char	szExamBit[10 + 1];			/* 是否跑驗測流程 */
	char	szTMSPacketSize[4 + 1];			/* 預設為998 */
	char	szSupECR_UDP[2 + 1];			/* 是否支援UDP ECR */
	char	szUDP_Port[5 + 1];			/* UDP使用，固定為50002 */
	char	szPOS_IP[16 + 1];			/* UDP使用，POS IP */
	char	szPOSTxUniqueNo[16 + 1];		/* UDP使用 */
	char	szUDP_ECRVersion[2 + 1];		/* UDP版本使用　*/
	char	szPOS_ID[6 + 1];			/* 客製化026使用 */
	char	szGPRS_APN[100 + 1];			/* GPRS APN使用 */
	char	szScreenBrightness[3 + 1];		/* 套用的螢幕亮度 */
	char	szSignpadBeepInterval[3 + 1];		/* 客製化123使用，簽名版出現時的提示聲間隔(10ms) */
} EDC_REC;

/* 以欄位數決定 ex:欄位數63個，理論上會有61個comma和兩個byte的0x0D 0X0A */
#define _SIZE_EDC_COMMA_0D0A_           64
#define _SIZE_EDC_REC_                  (sizeof(EDC_REC))
#define _EDC_FILE_NAME_                 "EDC.dat"
#define _EDC_FILE_NAME_BAK_             "EDC.bak"

#define _PWM_MODE_00_STANDBY_		"0"
#define _PWM_MODE_01_SLEEP_		"1"

#define _COMPORT_COM1_			"COM1"
#define _COMPORT_COM2_			"COM2"
#define _COMPORT_COM3_			"COM3"
#define _COMPORT_COM4_			"COM4"
#define _COMPORT_USB1_			"USB1"
#define _COMPORT_USB2_			"USB2"
#define _COMPORT_WIFI_			"WIFI"

#define _ECR_RS232_VERSION_LEN_			2
#define _ECR_RS232_VERSION_STANDARD_400_	"00"
#define _ECR_RS232_VERSION_STANDARD_144_	"01"
#define _ECR_RS232_VERSION_BUMPER_		"02"
#define _ECR_RS232_VERSION_STANDARD_KIOSK_	"03"
#define _ECR_RS232_VERSION_CARD_NO_HIDE_F8_B4_	"04"
#define _ECR_RS232_VERSION_DEXIAN_		"05"
#define _ECR_RS232_VERSION_NICE_PLAZA_		"06"
#define _ECR_RS232_VERSION_MCDONALDS_		"07"
#define _ECR_RS232_VERSION_TK3C_EINVOICE_	"08"
#define _ECR_RS232_VERSION_WELCOME_		"09"
#define _ECR_RS232_VERSION_CHUNGHWA_TELECOM_	"10"
#define _ECR_RS232_VERSION_UNIAIR_		"11"
#define _ECR_RS232_VERSION_TAICHUNG_SOGO_	"12"
#define _ECR_RS232_VERSION_MIRAMAR_CINEMAS_	"13"
#define _ECR_RS232_VERSION_EDA_144_		"14"
#define _ECR_RS232_VERSION_EDA_400_		"15"
#define _ECR_RS232_VERSION_CINEMARK_		"16"
#define _ECR_RS232_VERSION_082_IKEA_		"17"
#define _ECR_RS232_VERSION_038_BELLAVITA_	"18"
#define _ECR_RS232_VERSION_005_FPG_		"19"
#define _ECR_RS232_VERSION_026_TAKA_		"20"
#define _ECR_RS232_VERSION_021_TAKAWEL_		"21"
#define _ECR_RS232_VERSION_090_TAIPEI_101_	"22"

#define _ECR_UDP_VERSION_LEN_			2
#define _ECR_UDP_VERSION_STANDARD_400_		"00"
#define _ECR_UDP_VERSION_STANDARD_144_		"01"
#define _ECR_UDP_VERSION_BUMPER_		"02"
#define _ECR_UDP_VERSION_STANDARD_KIOSK_	"03"
#define _ECR_UDP_VERSION_CARD_NO_HIDE_F8_B4_	"04"
#define _ECR_UDP_VERSION_DEXIAN_		"05"
#define _ECR_UDP_VERSION_NICE_PLAZA_		"06"
#define _ECR_UDP_VERSION_MCDONALDS_		"07"
#define _ECR_UDP_VERSION_TK3C_EINVOICE_		"08"
#define _ECR_UDP_VERSION_WELCOME_		"09"
#define _ECR_UDP_VERSION_CHUNGHWA_TELECOM_	"10"
#define _ECR_UDP_VERSION_UNIAIR_		"11"
#define _ECR_UDP_VERSION_TAICHUNG_SOGO_		"12"
#define _ECR_UDP_VERSION_MIRAMAR_CINEMAS_	"13"
#define _ECR_UDP_VERSION_EDA_144_		"14"
#define _ECR_UDP_VERSION_EDA_400_		"15"
#define _ECR_UDP_VERSION_CINEMARK_		"16"
#define _ECR_UDP_VERSION_082_IKEA_		"17"
#define _ECR_UDP_VERSION_038_BELLAVITA_		"18"
#define _ECR_UDP_VERSION_005_FPG_		"19"
#define _ECR_UDP_VERSION_026_TAKA_		"20"
#define _ECR_UDP_VERSION_021_TAKAWEL_		"21"
#define _ECR_UDP_VERSION_090_TAIPEI_101_	"22"

#define _ISO_DEBUG_INDEX_DEBUG_		0
#define _ISO_DEBUG_INDEX_ISODEBUG_	1
#define _ISO_DEBUG_INDEX_DISPLAYDEBUG_	2
#define _ISO_DEBUG_INDEX_ENGINEERDEBUG_	3
#define _ISO_DEBUG_INDEX_ESCDEBUG_	4

#define _SCREEN_BRIGHTNESS_MAX_			"255"
#define _SCREEN_BRIGHTNESS_IKEA_IDLE_		"64"

#define _SIGNPAD_BEEP_INTERVAL_CUS_123_DEFAULT_	"100"
#define _SIGNPAD_BEEP_INTERVAL_CUS_123_MAX_	"300"	
#define _SIGNPAD_BEEP_INTERVAL_CUS_123_MIN_	"50"	

/* Load & Save function */
int inLoadEDCRec(int inEDCRec);
int inSaveEDCRec(int inEDCRec);
int inEDC_Edit_EDC_Table(void);
int inEDC_TMSDownloadTimes_Increase(void);

/* Set function */
int inSetCUPFuncEnable(char* szCUPFuncEnable);
int inSetFiscFuncEnable(char* szFiscFuncEnable);
int inSetECRComPort(char* szECRComPort);
int inSetECRVersion(char* szECRVersion);
int inSetPABXCode(char* szPABXCode);
int inSetTSAMRegisterEnable(char* szTSAMRegisterEnable);
int inSetTMSOK(char* szTMSOK);
int inSetDCCInit(char* szDCCInit);
int inSetDCCDownloadMode(char* szDCCDownloadMode);
int inSetDCCLastUpdateDate(char* szDCCLastUpdateDate);
int inSetDCCSettleDownload(char* szDCCSettleDownload);
int inSetDCCBinVer(char* szDCCBinVer);
int inSetEDCLOCK(char* szEDCLOCK);
int inSetISODebug(char* szISODebug);
int inSetManagerPassword(char* szManagerPassword);
int inSetFunctionPassword(char* szFunctionPassword);
int inSetMerchantPassword(char* szMerchantPassword);
int inSetSuperPassword(char* szSuperPassword);
int inSetMCCCode(char* szMCCCode);
int inSetIssuerID(char* szIssuerID);
int inSetEnterTimeout(char* szEnterTimeout);
int inSetIPSendTimeout(char* szIPSendTimeout);
int inSetTermVersionID(char* szTermVersionID);
int inSetTermVersionDate(char* szTermVersionDate);
int inSetTermIPAddress(char* szTermIPAddress);
int inSetTermGetewayAddress(char* szTermGetewayAddress);
int inSetTermMASKAddress(char* szTermMASKAddress);
int inSetTermECRPort(char* szTermECRPort);
int inSetESCMode(char* szESCMode);
int inSetMultiComPort1(char* szMultiComPort1);
int inSetMultiComPort1Version(char* szMultiComPort1Version);
int inSetMultiComPort2(char* szMultiComPort2);
int inSetMultiComPort2Version(char* szMultiComPort2Version);
int inSetMultiComPort3(char* szMultiComPort3);
int inSetMultiComPort3Version(char* szMultiComPort3Version);
int inSetEMVForceOnline(char* szEMVForceOnline);
int inSetAutoConnect(char* szAutoConnect);
int inSetLOGONum(char* szLOGONum);
int inSetHostSAMSlot(char* szSAMSlot);
int inSetSAMSlotSN1(char* szSAMSlotSN1);
int inSetSAMSlotSN2(char* szSAMSlotSN2);
int inSetSAMSlotSN3(char* szSAMSlotSN3);
int inSetSAMSlotSN4(char* szSAMSlotSN4);
int inSetPWMEnable(char* szPWMEnable);
int inSetPWMMode(char* szPWMMode);
int inSetPWMIdleTimeout(char* szPWMIdleTimeout);
int inSetDemoMode(char* szDemoMode);
int inSetKBDLock(char* szKBDLock);
int inSetLastDownloadMode(char* szLastDownloadMode);
int inSetPMPassword(char* szPMPassword);
int inSetFunKeyPassword(char* szFunKeyPassword);
int inSetTMSDownloadTimes(char* szTMSDownloadTimes);
int inSetExamBit(char* szExamBit);
int inSetTMSPacketSize(char* szTMSPacketSize);
int inSetSupECR_UDP(char* szSupECR_UDP);
int inSetUDP_Port(char* szUDP_Port);
int inSetPOS_IP(char* szPOS_IP);
int inSetPOSTxUniqueNo(char* szPOSTxUniqueNo);
int inSetUDP_ECRVersion(char* szUDP_ECRVersion);
int inSetPOS_ID(char* szPOS_ID);
int inSetGPRS_APN(char* szGPRS_APN);
int inSetScreenBrightness(char* szScreenBrightness);
int inSetSignpadBeepInterval(char* szSignpadBeepInterval);

/* Get function */
int inGetCUPFuncEnable(char* szCUPFuncEnable);
int inGetFiscFuncEnable(char* szFiscFuncEnable);
int inGetECRComPort(char* szECRComPort);
int inGetECRVersion(char* szECRVersion);
int inGetPABXCode(char* szPABXCode);
int inGetTSAMRegisterEnable(char* szTSAMRegisterEnable);
int inGetTMSOK(char* szTMSOK);
int inGetDCCInit(char* szDCCInit);
int inGetDCCDownloadMode(char* szDCCDownloadMode);
int inGetDCCLastUpdateDate(char* szDCCLastUpdateDate);
int inGetDCCSettleDownload(char* szDCCSettleDownload);
int inGetDCCBinVer(char* szDCCBinVer);
int inGetEDCLOCK(char* szEDCLOCK);
int inGetISODebug(char* szISODebug);
int inGetManagerPassword(char* szManagerPassword);
int inGetFunctionPassword(char* szFunctionPassword);
int inGetMerchantPassword(char* szMerchantPassword);
int inGetSuperPassword(char* szSuperPassword);
int inGetMCCCode(char* szMCCCode);
int inGetIssuerID(char* szIssuerID);
int inGetEnterTimeout(char* szEnterTimeout);
int inGetIPSendTimeout(char* szIPSendTimeout);
int inGetTermVersionID(char* szTermVersionID);
int inGetTermVersionDate(char* szTermVersionDate);
int inGetTermIPAddress(char* szTermIPAddress);
int inGetTermGetewayAddress(char* szTermGetewayAddress);
int inGetTermMASKAddress(char* szTermMASKAddress);
int inGetTermECRPort(char* szTermECRPort);
int inGetESCMode(char* szESCMode);
int inGetMultiComPort1(char* szMultiComPort1);
int inGetMultiComPort1Version(char* szMultiComPort1Version);
int inGetMultiComPort2(char* szMultiComPort2);
int inGetMultiComPort2Version(char* szMultiComPort2Version);
int inGetMultiComPort3(char* szMultiComPort3);
int inGetMultiComPort3Version(char* szMultiComPort3Version);
int inGetEMVForceOnline(char* szEMVForceOnline);
int inGetAutoConnect(char* szAutoConnect);
int inGetLOGONum(char* szLOGONum);
int inGetHostSAMSlot(char* szSAMSlot);
int inGetSAMSlotSN1(char* szSAMSlotSN1);
int inGetSAMSlotSN2(char* szSAMSlotSN2);
int inGetSAMSlotSN3(char* szSAMSlotSN3);
int inGetSAMSlotSN4(char* szSAMSlotSN4);
int inGetPWMEnable(char* szPWMEnable);
int inGetPWMMode(char* szPWMMode);
int inGetPWMIdleTimeout(char* szPWMIdleTimeout);
int inGetDemoMode(char* szDemoMode);
int inGetKBDLock(char* szKBDLock);
int inGetLastDownloadMode(char* szLastDownloadMode);
int inGetPMPassword(char* szPMPassword);
int inGetFunKeyPassword(char* szFunKeyPassword);
int inGetTMSDownloadTimes(char* szTMSDownloadTimes);
int inGetExamBit(char* szExamBit);
int inGetTMSPacketSize(char* szTMSPacketSize);
int inGetSupECR_UDP(char* szSupECR_UDP);
int inGetUDP_Port(char* szUDP_Port);
int inGetPOS_IP(char* szPOS_IP);
int inGetPOSTxUniqueNo(char* szPOSTxUniqueNo);
int inGetUDP_ECRVersion(char* szUDP_ECRVersion);
int inGetPOS_ID(char* szPOS_ID);
int inGetGPRS_APN(char* szGPRS_APN);
int inGetScreenBrightness(char* szScreenBrightness);
int inGetSignpadBeepInterval(char* szSignpadBeepInterval);