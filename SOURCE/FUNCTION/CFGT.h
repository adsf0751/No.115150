typedef struct {
	char szCustomIndicator[3 + 1]; /* 客製化參數--依此參數客製化 */
	char szNCCCFESMode[2 + 1]; /* NCCC FES 模式 , 01 = 集訓機  02 = R-FES 03 = M-FES 04 = MPAS 05 = ATS */
	char szCommMode[1 + 1]; /* 通訊模式 Commucation_Mode ,預設值=0, 1=撥接, 2=TCP/IP, 3=RS485, 4=IP_BOX, 5=GPRS, 6=GSM (NCCC 0=撥接，1=TCP/IP，2=GSM，3=RS485，4=IP_BOX，5=GPRS，6=3G，7=4G)*/
	char szDialBackupEnable[1 + 1]; /* 撥接備援  Dial_Up_Backup_Enable ,預設值=N */
	char szEncryptMode[1 + 1]; /* 加密模式 Encryption_Mode_Enable,預設值=0, 0=不加密, 1=tSAM加密, 2=軟體加密, 3=PINPAD加密 */
	char szSplitTransCheckEnable[1 + 1]; /* Split_Transaction_Check_Enable 不可連續刷卡檢核  ,預設值=N */
	char szCityName[20 + 1]; /* 城市別 */
	char szStoreIDEnable[1 + 1]; /* 櫃號功能 ,預設值=N */
	char szMinStoreIDLen[2 + 1]; /* 櫃號輸入最短位數 ,預設值=00 */
	char szMaxStoreIDLen[2 + 1]; /* 櫃號輸入最長位數  ,預設值=18 */
	char szECREnable[1 + 1]; /* ECR連線是否啟動 ,預設值=N */
	char szECRCardNoTruncateEnable[1 + 1]; /* ECR卡號遮掩, 預設值=Y */
	char szECRExpDateReturnEnable[1 + 1]; /* ECR卡片有效期回傳, 預設值=N */
	char szProductCodeEnable[1 + 1]; /* 列印產品代碼, 預設值=N */
	char szPrtSlogan[1 + 1]; /* 列印Slogan, 預設值=N */
	char szSloganStartDate[8 + 1]; /* Slogan起始日(YYYYMMDD)包含此日， 預設值 = 即日 */
	char szSloganEndDate[8 + 1]; /* Slogan停用日(YYYYMMDD)不包含此日，預設值="20991231" 若此欄位為 "20991231"，即代表活動無結束日期 */
	char szSloganPrtPosition[1 + 1]; /* Slogan列印位置， 預設值= 2 (1=簽單上方，2=簽單下方 */
	char szPrtMode[1 + 1]; /* 帳單列印模式， 預設值=2, 0=不列印帳單，1=列印一聯，2=列印兩聯，3=列印三聯 */
	char szContactlessEnable[1 + 1]; /* 啟動非接觸式讀卡功能, 預設值=N */
	char szVISAPaywaveEnable[1 + 1]; /* 接受 Visa Paywave非接觸式卡片 */
	char szJCBJspeedyEnable[1 + 1]; /* 接受 JCB Jspeedy非接觸式卡片 */
	char szMCPaypassEnable[1 + 1]; /* 接受 MC Paypass非接觸式卡片 */
	char szCUPRefundLimit[12 + 1]; /* CUP退貨限額(含)，包含兩位小數位, 預設值="999999999900" */
	char szCUPKeyExchangeTimes[1 + 1]; /* CUP自動安全認證次數, 預設值 = 3 */
	char szMACEnable[1 + 1]; /* 交易電文是否上傳MAC驗證, 預設值 = Y */
	char szPinpadMode[1 + 1]; /* 密碼機模式, 預設值= 0, 0=未使用密碼機，1=使用內建密碼機，2=使用外接密碼機 */
	char szFORCECVV2[1 + 1]; /* 若輸入CVV2功能開啟，則強迫要輸入CVV2值, 預設值 = N, Y=要強制輸入不可Bypass，N=不強制輸入，可以Bypass */
	char szSpecialCardRangeEnable[1 + 1]; /* Special_Card_Range_Enable 啟動特殊卡別參數檔功能, 預設值= N, Y=支援，N=不支援 */
	char szPrtMerchantLogo[1 + 1]; /* 列印商店 Logo */
	char szPrtMerchantName[1 + 1]; /* 列印商店表頭, 預設值= Y, Y=要印，N=不印 , 若是表頭一、二欄位無資料，則該值為N。若是是表頭一、二欄位有資料(含空白)，則該值為Y */
	char szPrtNotice[1 + 1]; /* 列印商店提示語, 預設值= N, Y=要印，N=不印 */
	char szElecCommerceFlag[2 + 1]; /* Electronic_Commerce_Flag 郵購及定期性行業Indicator, 預設值 = 00, 00= 非一次性及定期性郵購行業，01= 一次性郵購行業，02= 定期性郵購行業 */
	char szDccFlowVersion[1 + 1]; /* DCC詢價版本, 預設值 = 0, 0 = 不支援DCC, 1 = 直接於詢價時由 DCC 依 Card Bin 回覆其外幣幣別及金額, 2 = 於EDC選擇交易幣別詢價 */
	char szSupDccVisa[1 + 1]; /* DCC是否接受VISA卡,若端末機不支援DCC, 預設值 = N, N = DCC不支援VISA，Y= DCC支援VISA */
	char szSupDccMasterCard[1 + 1]; /* DCC是否接受MasterCard卡，若端末機不支援DCC, 預設值 = N, N = DCC不支援MasterCard，Y= DCC支援MasterCard */
	char szDHCPRetryTimes[1 + 1]; /* DHCP Retry 次數, 預設值=0 */
	char szBarCodeReaderEnable[1 + 1]; /* 是否開啟BarCode Reader，預設值= N。 (Y=開，N=關) */
	char szEMVPINBypassEnable[1 + 1]; /* 是否開啟EMV PIN Bypass功能，預設值= Y。(Y=開，N=關) */
	char szCUPOnlinePINEntryTimeout[3 + 1]; /* 等候CUP Online PIN輸入的Time out時間。以秒為單位，右靠左補 0，預設值=020 (需求先Hard Code於TMS程式中) */
	char szSignPadMode[1 + 1]; /* 簽名板模式，預設值= 0。( 若 (0=未使用SignPad，1=使用內建SignPad，2=使用外接SignPad) */
	char szESCPrintMerchantCopy[1 + 1]; /* 有開啟簽名板功能(SignPad_Mode='1' || '2')，是否列印商店商店存根聯，預設值= N。(Y=要印，N=不印) */
	char szESCPrintMerchantCopyStartDate[8 + 1]; /* 若是ESC_Print_Merchant_Copy=Y(要印)，則需要參考此欄位。列印起始日(YYYYMMDD)包含此日，預設值 = 即日。(開始日會列印商店存根聯) */
	char szESCPrintMerchantCopyEndDate[8 + 1]; /* 若是ESC_Print_Merchant_Copy=Y(要印)，則需要參考此欄位。列印停用日(YYYYMMDD)不包含此日，預設值="20991231"。若此欄位為 "20991231"，即代表活動無結束日期。(結束日不列印商店存根聯) */
	char szESCReciptUploadUpLimit[3 + 1]; /* 電子簽帳單之上水位參數右靠左補 0，預設值=000(上水位不得小於下水位) */
	char szContactlessReaderMode[1 + 1]; /* Contactless_Reader_Mode模式，預設值= 0。(若支援Contactless，則必需選擇內建或外接Contactless_Reader)(0=未使用Contactless_Reader，1=使用內建Contactless_Reader，2=使用外接Contactless_Reader) */
	char szTMSDownloadMode[1 + 1]; /* 端末機依此參數判斷對TMS或Download Server(FTPS)下載，預設值=1。1=TMS下載(ISO8583)2=Download Server下載(FTPS)註：支援此參數之端末機版本，若參數值設定為'2'，則端末機應由Download Server自動下載TermInfo2.txt這個檔案。 */
	char szAMEXContactlessEnable[1 + 1]; /* 接受 AMEX 非接觸式卡片(保留未來使用) */
	char szCUPContactlessEnable[1 + 1]; /* 接受 CUP非接觸式卡片(QuickPass) */
	char szSmartPayContactlessEnable[1 + 1]; /* 接受 SmartPay非接觸式卡片 */
	char szPay_Item_Enable[1 + 1]; /* 開啟繳費項目功能，預設值= N。(此功能需參考PayItem.txt) */
	char szStore_Stub_CardNo_Truncate_Enable[1 + 1]; /* 商店自存聯卡號遮掩，預設值= Y (Y=遮掩，N=不遮掩)(此參數需要搭配判斷卡別參數檔(2) CardDef2.txt ) */
	char szIntegrate_Device[1 + 1]; /* 是否為整合型週邊設備，預設值= N。 */
	char szFES_ID[3 + 1]; /* FES請款代碼，預設值= 000。(派工單要增加此欄位) */
	char szIntegrate_Device_AP_ID[15 + 1]; /* 整合型周邊設備之AP ID，預設值= 空白。 */
	char szShort_Receipt_Mode[1 + 1]; /* 短式簽單模式，預設值=N。 */
	char szI_FES_Mode[1 + 1]; /* I-FES(Internet Gateway)模式，預設值=N。Y=表示端末機連線主機為I-FES，應具備下述功能：1.端末機與主機連線須先建立SSL TLS V1.2 加密通道。2.第一授權主機IP無法建立連線時，應自動改連第二授權主機IP。 */
	char szDHCP_Mode[1 + 1]; /* 是否開啟DHCP模式，預設值=N。Y=端末機為DHCP模式。N=端末機為固定IP模式。 */
	char szESVC_Priority[4 + 1]; /* 開啟的電票功能及自動詢卡優先順序。若是端末機未支援該電票功能，該電票代碼則不會出現。E=悠遊卡 I=一卡通 C=愛金卡 H=遠鑫卡 */
	char szDFS_Contactless_Enable[1 + 1]; /* 接受DFS非接觸式卡片 */
	char szCloud_MFES[1 + 1]; /* 是否為雲端MFES，預設值=N。Y=雲端MFES。 N=一般MFES。 */
	char szBIN_CHECK[1 + 1]; /* A = ATS check E = EDC check */
	char szNCCC_Contactless_Enable[1 + 1]; /* 接受NCCC品牌卡非接觸式卡片 */
	char szFORCE_CID[1 + 1]; /* 若DFS輸入CID功能開啟，則強迫要輸入CID值，預設值 = Y。 (Y=要強制輸入不可Bypass，N=不強制輸入，可以Bypass) */
	char szCHECK_ID[1 + 1]; /* "信託交易是否要查核病患/入住者ID。(Y=顯示病患或入住者ID畫面並進行查核)" */
	char szCHESG_FLAG[1 + 1]; /* [114197] 中心收單作業ESG永續發展優化 (Y=持卡人存根聯數位化) */
} CFGT_REC;

#define _CUSTOMER_INDICATOR_000_				"000"   /* 標準版 */
#define _CUSTOMER_INDICATOR_001_				"001"
#define _CUSTOMER_INDICATOR_002_NICE_PLAZA_	"002"   /* 耐斯廣場 & 王子飯店 */
#define _CUSTOMER_INDICATOR_003_WELLCOME_		"003"   /* 惠康超市 */
#define _CUSTOMER_INDICATOR_005_FPG_			"005"	/* 台塑生醫生技客製化專屬【005】 */
#define _CUSTOMER_INDICATOR_006_				"006"
#define _CUSTOMER_INDICATOR_010_				"010"
#define _CUSTOMER_INDICATOR_011_				"011"
#define _CUSTOMER_INDICATOR_013_				"013"
#define _CUSTOMER_INDICATOR_019_				"019"
#define _CUSTOMER_INDICATOR_020_				"020"
#define _CUSTOMER_INDICATOR_021_TAKAWEL_		"021"	/* 大高惠康jasons客製化專屬【021】 */
#define _CUSTOMER_INDICATOR_024_				"024"
#define _CUSTOMER_INDICATOR_026_TAKA_			"026"	/* 大高客製化專屬【026】*/
#define _CUSTOMER_INDICATOR_027_CHUNGHWA_TELECOM_	"027"	/* 中華電信 */
#define _CUSTOMER_INDICATOR_028_				"028"
#define _CUSTOMER_INDICATOR_031_				"031"
#define _CUSTOMER_INDICATOR_032_				"032"
#define _CUSTOMER_INDICATOR_033_UNIAIR_		"033"	/* 立榮航空 */
#define _CUSTOMER_INDICATOR_034_TK3C_EINVOICE_		"034"   /* 燦坤3C 144 (8N1) + 電子發票 */
#define _CUSTOMER_INDICATOR_035_MIRAMAR_CINEMAS_	"035"	/* 美麗華影城客製化專屬【035】 */
#define _CUSTOMER_INDICATOR_036_				"036"
#define _CUSTOMER_INDICATOR_038_BELLAVITA_		"038"	/* BELLAVITA客製化專屬【038】*/
#define _CUSTOMER_INDICATOR_039_CARD_NO_HIDE_F8_B4_	"039"	/* 400卡片遮掩前八後四客製化專屬【039】*/
#define _CUSTOMER_INDICATOR_040_				"040"
#define _CUSTOMER_INDICATOR_041_CASH_			"041"	/* 預借現金客製化專屬【041】*/
#define _CUSTOMER_INDICATOR_042_BDAU1_			"042"	/* 保險公司客製化專屬【042】*/
#define _CUSTOMER_INDICATOR_043_BDAU9_			"043"	/* 建設公司客製化專屬【043】*/
#define _CUSTOMER_INDICATOR_044_				"044"
#define _CUSTOMER_INDICATOR_046_DUTY_FREE_		"046"	/* 昇恆昌-桃園【046】*/
#define _CUSTOMER_INDICATOR_047_DUTY_FREE_		"047"	/* 昇恆昌-桃園【047】*/
#define _CUSTOMER_INDICATOR_048_				"048"
#define _CUSTOMER_INDICATOR_053_TAICHUNG_SOGO_	"053"	/* 台中廣三SOGO【053】 */
#define _CUSTOMER_INDICATOR_055_EINVOICE_		"055"   /* 標準400 + 電子發票 */
#define _CUSTOMER_INDICATOR_060_				"060"
#define _CUSTOMER_INDICATOR_061_GRAND_HOTEL_	"061"	/* 圓山大飯店客製化專屬【061】*/
#define _CUSTOMER_INDICATOR_065_				"065"
#define _CUSTOMER_INDICATOR_067_KOBAY			"067" /* [20260515] 新增067參數*/
#define _CUSTOMER_INDICATOR_068_				"068"
#define _CUSTOMER_INDICATOR_071_FORCE_ONLINE_	"071"   /* 強制晶片交易go online */
#define _CUSTOMER_INDICATOR_073_				"073"
#define _CUSTOMER_INDICATOR_075_DUTY_FREE_		"075"	/* 【075】= 【072】+ 交易失敗不印通知單 */
#define _CUSTOMER_INDICATOR_076_8_DIGITS_		"076"   /* 交易金額8位數 */
#define _CUSTOMER_INDICATOR_077_				"077"
#define _CUSTOMER_INDICATOR_078_				"078"
#define _CUSTOMER_INDICATOR_079_CINEMARK_		"079"	/* 喜滿客影城  */
#define _CUSTOMER_INDICATOR_081_				"081"
#define _CUSTOMER_INDICATOR_082_IKEA_			"082"	/* IKEA客製化參數【082】 */
#define _CUSTOMER_INDICATOR_084_ON_US_			"084"   /* on-us繳費平台【084】 */ /* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code added by Russell 2020/3/16 上午 10:16 */
#define _CUSTOMER_INDICATOR_087_ON_US_NO_ID_	"087"   /* on-us繳費平台【087】 不輸入ID */ /* 【需求單 - 104014】發卡機構on-us繳費平台需求 +【需求單 - 104152】發卡機構on-us繳費平台 Payitem Code added by Russell 2020/3/16 上午 10:16 */
#define _CUSTOMER_INDICATOR_088_TINTIN_		"088"   /* 標準400 + 電子發票 + ECR卡號遮掩客製 */
#define _CUSTOMER_INDICATOR_089_				"089"
#define _CUSTOMER_INDICATOR_090_TAIPEI_101_	"090"   /* 分期及紅利不對CUP Indicator 0做阻擋 */
#define _CUSTOMER_INDICATOR_092_				"092"
#define _CUSTOMER_INDICATOR_093_				"093"
#define _CUSTOMER_INDICATOR_094_				"094"
#define _CUSTOMER_INDICATOR_096_EDA_			"096"	/* 義大客製化【096】，ECR 400支援IPASS轉傳感應，DAUD RATE = 115200 */
#define _CUSTOMER_INDICATOR_097_EDA_			"097"	/* 義大客製化【097】，ECR 400不支援IPASS轉傳感應，DAUD RATE = 115200 */
#define _CUSTOMER_INDICATOR_098_MCDONALDS_		"098"   /* 麥當勞專用 */
#define _CUSTOMER_INDICATOR_099_SINYA_			"099"   /* 欣亞電腦客製化【099】*/
#define _CUSTOMER_INDICATOR_100_				"100"
#define _CUSTOMER_INDICATOR_103_DUTY_FREE_		"103"	/* 昇恆昌075-有開機檢核昇恆昌聯名卡參數 */
#define _CUSTOMER_INDICATOR_104_				"104"
#define _CUSTOMER_INDICATOR_105_MCDONALDS_		"105"   /* 99% 的客製化098，麥當勞專用 */
#define _CUSTOMER_INDICATOR_106_ON_US_BCA_		"106"   /* 領事事務局 */
#define _CUSTOMER_INDICATOR_107_BUMPER_		"107"
#define _CUSTOMER_INDICATOR_109_				"109"
#define _CUSTOMER_INDICATOR_111_KIOSK_STANDARD_	"111"
#define _CUSTOMER_INDICATOR_112_				"112"
#define _CUSTOMER_INDICATOR_113_H_AND_M_		"113"
#define _CUSTOMER_INDICATOR_114_				"114"
#define _CUSTOMER_INDICATOR_115_				"115"
#define _CUSTOMER_INDICATOR_116_				"116"
#define _CUSTOMER_INDICATOR_117_				"117"
#define _CUSTOMER_INDICATOR_118_				"118"
#define _CUSTOMER_INDICATOR_119_				"119"
#define _CUSTOMER_INDICATOR_122_QSQUARE_		"122"	/* 京站時尚百貨 */
#define _CUSTOMER_INDICATOR_123_IKEA_			"123"	/* IKEA(刪除收銀機連線SALE、REFUND選擇交易類別顯示畫面) */
#define _CUSTOMER_INDICATOR_124_EVER_RICH_		"124"
#define _CUSTOMER_INDICATOR_126_MASTERCARD_FLIGHT_TICKET_	"126"
#define _CUSTOMER_INDICATOR_SIZE_	3

/* 以欄位數決定 ex:欄位數是66個，理論上會有65個comma和兩個byte的0x0D 0X0A */
/* [114197] 中心收單作業ESG永續發展優化 由66修改成67 */
#define _SIZE_CFGT_COMMA_0D0A_   67
#define _SIZE_CFGT_REC_          (sizeof(CFGT_REC))
#define _CFGT_FILE_NAME_         "CFGT.dat"
#define _CFGT_FILE_NAME_BAK_     "CFGT.bak"

#define _NCCC_01_SPDH_MODE_	"01"
#define _NCCC_02_RFES_MODE_	"02"
#define _NCCC_03_MFES_MODE_	"03"
#define _NCCC_04_MPAS_MODE_	"04"
#define _NCCC_05_ATS_MODE_	"05"

/* 密碼機模式 */
#define _PINPAD_MODE_0_NO_		"0"	/* 未使用密碼機 */
#define _PINPAD_MODE_1_INTERNAL_	"1"	/* 使用內建密碼機 */
#define _PINPAD_MODE_2_EXTERNAL_	"2"	/* 使用外接密碼機 */

/* 感應模式 */
#define _CTLS_MODE_0_NO_		"0"	/* 未使用感應 */
#define _CTLS_MODE_1_INTERNAL_	        "1"	/* 使用內建感應 */
#define _CTLS_MODE_2_EXTERNAL_	        "2"	/* 使用外接感應 */

/* 簽名版模式 */
#define _SIGNPAD_MODE_0_NO_		"0"	/* 不使用簽名板 */
#define _SIGNPAD_MODE_1_INTERNAL_	"1"	/* 使用內建簽名板 */
#define _SIGNPAD_MODE_2_EXTERNAL_	"2"	/* 使用外接簽名板 */

/* TMS 下載模式 */
#define _TMS_DLMODE_ISO8583_	"1"
#define _TMS_DLMODE_FTPS_	"2"

/* 加密模式 */
#define _NCCC_ENCRYPTION_NONE_		"0"
#define _NCCC_ENCRYPTION_TSAM_		"1"
#define _NCCC_ENCRYPTION_SOFTWARE_	"2"

#define _NCCC_ENCRYPTION_LEN_		1

/* DCC FLOW VERSION */
#define _NCCC_DCC_FLOW_VER_NOT_SUPORTED_	"0"/* 不支援DCC */
#define _NCCC_DCC_FLOW_VER_BY_CARD_BIN_		"1"/* 直接於詢價時由 DCC 依 Card Bin 回覆其外幣幣別及金額 */
#define _NCCC_DCC_FLOW_VER_BY_MANUAL_		"2"/* 於EDC選擇交易幣別詢價 */

#define _NCCC_SLOGAN_PRINT_UP_		1
#define _NCCC_SLOGAN_PRINT_DOWN_	2

#define _PRT_MODE_4_TWO_RECEIPT_NO_MERCHANT_	"4"

#define _SHORT_RECEIPT_S_	"S"
#define _SHORT_RECEIPT_U_	"U"

/* Load & Save function */
int inLoadCFGTRec(int inCFGTRec);
int inSaveCFGTRec(int inCFGTRec);

/* Set function */
int inSetCustomIndicator(char* szCustomIndicator);
int inSetNCCCFESMode(char* szNCCCFESMode);
int inSetCommMode(char* szCommMode);
int inSetDialBackupEnable(char* szDialBackupEnable);
int inSetEncryptMode(char* szEncryptMode);
int inSetSplitTransCheckEnable(char* szSplitTransCheckEnable);
int inSetCityName(char* szCityName);
int inSetStoreIDEnable(char* szStoreIDEnable);
int inSetMinStoreIDLen(char* szMinStoreIDLen);
int inSetMaxStoreIDLen(char* szMaxStoreIDLen);
int inSetECREnable(char* szECREnable);
int inSetECRCardNoTruncateEnable(char* szECRCardNoTruncateEnable);
int inSetECRExpDateReturnEnable(char* szECRExpDateReturnEnable);
int inSetProductCodeEnable(char* szProductCodeEnable);
int inSetPrtSlogan(char* szPrtSlogan);
int inSetSloganStartDate(char* szSloganStartDate);
int inSetSloganEndDate(char* szSloganEndDate);
int inSetSloganPrtPosition(char* szSloganPrtPosition);
int inSetPrtMode(char* szPrtMode);
int inSetContactlessEnable(char* szContactlessEnable);
int inSetVISAPaywaveEnable(char* szVISAPaywaveEnable);
int inSetJCBJspeedyEnable(char* szJCBJspeedyEnable);
int inSetMCPaypassEnable(char* szMCPaypassEnable);
int inSetCUPRefundLimit(char* szCUPRefundLimit);
int inSetCUPKeyExchangeTimes(char* szCUPKeyExchangeTimes);
int inSetMACEnable(char* szMACEnable);
int inSetPinpadMode(char* szPinpadMode);
int inSetFORCECVV2(char* szFORCECVV2);
int inSetSpecialCardRangeEnable(char* szSpecialCardRangeEnable);
int inSetPrtMerchantLogo(char* szPrtMerchantLogo);
int inSetPrtMerchantName(char* szPrtMerchantName);
int inSetPrtNotice(char* szPrtNotice);
int inSetElecCommerceFlag(char* szElecCommerceFlag);
int inSetDccFlowVersion(char* szDccFlowVersion);
int inSetSupDccVisa(char* szSupDccVisa);
int inSetSupDccMasterCard(char* szSupDccMasterCard);
int inSetDHCPRetryTimes(char* szDHCPRetryTimes);
int inSetBarCodeReaderEnable(char* szBarCodeReaderEnable);
int inSetEMVPINBypassEnable(char* szEMVPINBypassEnable);
int inSetCUPOnlinePINEntryTimeout(char* szCUPOnlinePINEntryTimeout);
int inSetSignPadMode(char* szSignPadMode);
int inSetESCPrintMerchantCopy(char* szESCPrintMerchantCopy);
int inSetESCPrintMerchantCopyStartDate(char* szESCPrintMerchantCopyStartDate);
int inSetESCPrintMerchantCopyEndDate(char* szESCPrintMerchantCopyEndDate);
int inSetESCReciptUploadUpLimit(char* szESCReciptUploadUpLimit);
int inSetContactlessReaderMode(char* szContactlessReaderMode);
int inSetTMSDownloadMode(char* szTMSDownloadMode);
int inSetAMEXContactlessEnable(char* szAMEXContactlessEnable);
int inSetCUPContactlessEnable(char* szCUPContactlessEnable);
int inSetSmartPayContactlessEnable(char* szSmartPayContactlessEnable);
int inSetPayItemEnable(char* szPay_Item_Enable);
int inSetStore_Stub_CardNo_Truncate_Enable(char* szStore_Stub_CardNo_Truncate_Enable);
int inSetIntegrate_Device(char* szIntegrate_Device);
int inSetFES_ID(char* szFES_ID);
int inSetIntegrate_Device_AP_ID(char* szIntegrate_Device_AP_ID);
int inSetShort_Receipt_Mode(char* szShort_Receipt_Mode);
int inSetI_FES_Mode(char* szI_FES_Mode);
int inSetDHCP_Mode(char* szDHCP_Mode);
int inSetESVC_Priority(char* szESVC_Priority);
int inSetDFS_Contactless_Enable(char* szDFS_Contactless_Enable);
int inSetCloud_MFES(char* szCloud_MFES);
int inSetBIN_CHECK(char* szBIN_CHECK);
int inSetNCCC_Contactless_Enable(char* szNCCC_Contactless_Enable);
int inSetFORCE_CID(char* szFORCE_CID);
int inSetCHECK_ID(char* szCHECK_ID);
int inSetCHESG_FLAG(char* szCHESG_FLAG);/* [114197] 中心收單作業ESG永續發展優化 (Y=持卡人存根聯數位化) */

/* Get function */
int inGetCustomIndicator(char* szCustomIndicator);
int inGetNCCCFESMode(char* szNCCCFESMode);
int inGetCommMode(char* szCommMode);
int inGetDialBackupEnable(char* szDialBackupEnable);
int inGetEncryptMode(char* szEncryptMode);
int inGetSplitTransCheckEnable(char* szSplitTransCheckEnable);
int inGetCityName(char* szCityName);
int inGetStoreIDEnable(char* szStoreIDEnable);
int inGetMinStoreIDLen(char* szMinStoreIDLen);
int inGetMaxStoreIDLen(char* szMaxStoreIDLen);
int inGetECREnable(char* szECREnable);
int inGetECRCardNoTruncateEnable(char* szECRCardNoTruncateEnable);
int inGetECRExpDateReturnEnable(char* szECRExpDateReturnEnable);
int inGetProductCodeEnable(char* szProductCodeEnable);
int inGetPrtSlogan(char* szPrtSlogan);
int inGetSloganStartDate(char* szSloganStartDate);
int inGetSloganEndDate(char* szSloganEndDate);
int inGetSloganPrtPosition(char* szSloganPrtPosition);
int inGetPrtMode(char* szPrtMode);
int inGetContactlessEnable(char* szContactlessEnable);
int inGetVISAPaywaveEnable(char* szVISAPaywaveEnable);
int inGetJCBJspeedyEnable(char* szJCBJspeedyEnable);
int inGetMCPaypassEnable(char* szMCPaypassEnable);
int inGetCUPRefundLimit(char* szCUPRefundLimit);
int inGetCUPKeyExchangeTimes(char* szCUPKeyExchangeTimes);
int inGetMACEnable(char* szMACEnable);
int inGetPinpadMode(char* szPinpadMode);
int inGetFORCECVV2(char* szFORCECVV2);
int inGetSpecialCardRangeEnable(char* szSpecialCardRangeEnable);
int inGetPrtMerchantLogo(char* szPrtMerchantLogo);
int inGetPrtMerchantName(char* szPrtMerchantName);
int inGetPrtNotice(char* szPrtNotice);
int inGetElecCommerceFlag(char* szElecCommerceFlag);
int inGetDccFlowVersion(char* szDccFlowVersion);
int inGetSupDccVisa(char* szSupDccVisa);
int inGetSupDccMasterCard(char* szSupDccMasterCard);
int inGetDHCPRetryTimes(char* szDHCPRetryTimes);
int inGetBarCodeReaderEnable(char* szBarCodeReaderEnable);
int inGetEMVPINBypassEnable(char* szEMVPINBypassEnable);
int inGetCUPOnlinePINEntryTimeout(char* szCUPOnlinePINEntryTimeout);
int inGetSignPadMode(char* szSignPadMode);
int inGetESCPrintMerchantCopy(char* szESCPrintMerchantCopy);
int inGetESCPrintMerchantCopyStartDate(char* szESCPrintMerchantCopyStartDate);
int inGetESCPrintMerchantCopyEndDate(char* szESCPrintMerchantCopyEndDate);
int inGetESCReciptUploadUpLimit(char* szESCReciptUploadUpLimit);
int inGetContactlessReaderMode(char* szContactlessReaderMode);
int inGetTMSDownloadMode(char* szTMSDownloadMode);
int inGetAMEXContactlessEnable(char* szAMEXContactlessEnable);
int inGetCUPContactlessEnable(char* szCUPContactlessEnable);
int inGetSmartPayContactlessEnable(char* szSmartPayContactlessEnable);
int inGetPayItemEnable(char* szPay_Item_Enable);
int inGetStore_Stub_CardNo_Truncate_Enable(char* szStore_Stub_CardNo_Truncate_Enable);
int inGetIntegrate_Device(char* szIntegrate_Device);
int inGetFES_ID(char* szFES_ID);
int inGetIntegrate_Device_AP_ID(char* szIntegrate_Device_AP_ID);
int inGetShort_Receipt_Mode(char* szShort_Receipt_Mode);
int inGetI_FES_Mode(char* szI_FES_Mode);
int inGetDHCP_Mode(char* szDHCP_Mode);
int inGetESVC_Priority(char* szESVC_Priority);
int inGetDFS_Contactless_Enable(char* szDFS_Contactless_Enable);
int inGetCloud_MFES(char* szCloud_MFES);
int inGetBIN_CHECK(char* szBIN_CHECK);
int inGetNCCC_Contactless_Enable(char* szNCCC_Contactless_Enable);
int inGetFORCE_CID(char* szFORCE_CID);
int inGetCHECK_ID(char* szCHECK_ID);
int inGetCHESG_FLAG(char* szCHESG_FLAG);/* [114197] 中心收單作業ESG永續發展優化 (Y=持卡人存根聯數位化) */

int inCFGT_Edit_CFGT_Table(void);
