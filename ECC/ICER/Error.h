#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef __cplusplus
	extern "C" {
#endif

#define		SUCCESS						0

#define 	NAD_PCB_ERROR				0x6005
#define 	EDC_ERROR					0x6004
#define		CARD_SUCCESS				0x9000
#define		CARD_TIMEOUT				0x6088
#define		ICER_ERROR					-99

#define 	PORT_OPENED_ERROR					-1
#define 	PORT_DEVICE_ERROR					-2
#define 	PORT_SETCOMM_ERROR					-3
#define 	PORT_NOTOPEN_ERROR					-4
#define 	PORT_PURGECOMM_ERROR				-5
#define 	PORT_GETCOMTIMEOUT_ERROR			-6
#define 	PORT_SETCOMTIMEOUT_ERROR			-7
#define 	PORT_GETCOMMSTATE_ERROR 			-8
#define 	PORT_SETCOMMSTATE_ERROR 			-9
#define 	PORT_CREATEWRITEEVENT_ERROR 		-10
#define 	PORT_CLOSEHANDLE_ERROR				-11
#define 	PORT_NOTWRITEPENDING_ERROR			-12
#define 	PORT_WRITEOVERLAPPED_ERROR			-13
#define 	PORT_WRITETIMEOUT_ERROR 			-14
#define 	PORT_WRITEWAITOBJECT_ERROR			-15
#define 	PORT_CREATEREADEVENT_ERROR			-16
#define 	PORT_NOTREADPENDING_ERROR			-17
#define 	PORT_READOVERLAPPED_ERROR			-18
#define 	PORT_READTIMEOUT_ERROR				-19
#define 	PORT_READWAITOBJECT_ERROR			-20
#define 	PORT_WRITEFILE_ERROR				-21
#define 	PORT_READFILE_ERROR 				-22
#define 	PORT_LOG_ERROR						-23
#define 	DONGLE_CONFIRM_ADDVALUE_ERROR		-24
#define 	DONGLE_CONFIRM_CANCELADDVALUE_ERROR -25
#define 	DONGLE_CONFIRM_CARDSALE_ERROR		-26
#define 	DONGLE_CONFIRM_CANCELCARDSALE_ERROR -27
#define 	DONGLE_BLOCKCARD_NOT_FIND			-28
#define 	DONGLE_CHDIR_ERROR					-29
#define 	DONGLE_OPENFILE_ERROR				-30
#define 	DONGLE_FSEEK_ERROR					-31
#define 	DONGLE_FERROR_ERROR 				-32
#define 	DONGLE_NO_BLACKLISTR				-33
#define 	DONGLE_PARGUMENTS_ERROR 			-34
#define 	DONGLE_MALLOC_ERROR 				-35
#define 	DONGLE_CAN_NOT_REQUEST_ERROR		-36

#define		ICER_DECLINE_FAIL					-101//ICER主機拒絕交易
#define		R6_DECLINE_FAIL					-102//R6主機拒絕交易
#define		ICER_PAR_INI_FAIL					-103//ICERINI.xml的XML格式錯誤
#define		ICER_REQ_FORMAT_FAIL				-104//ICERAPI.REQ的XML格式錯誤
#define		TM_INPUT_DATA_ERROR				-105//T5597='Y'或T5599='N'時,T0404及T5535不等於0
#define		TM_PROCESS_CODE_ERROR				-106//Processing Code錯誤,程式碼待補充
#define		ICER_REQ_DATA_FAIL				-109//ICERAPI.REQ的XML內容格式錯誤
#define		ICER_SEND_FAIL						-110//ICER送資料失敗
#define		ICER_RECV_FAIL						-111//ICER收資料失敗
#define		MUST_SETTLE						-117//需先結帳
#define		LOAD_DLL_FAIL						-118//DongleAPI.dll載入失敗
#define		DLL_DECLINE_FAIL					-119//DLL主機拒絕交易
#define		R6_SEND_FAIL						-120//R6送資料失敗
#define		R6_RECV_FAIL						-121//R6收資料失敗
#define		HOST_MAC_FAIL						-122//HOST MAC驗證失敗
#define		AMOUNT_FAIL						-123//餘額不足
#define		LOAD_DLL_FUNCTION_ERROR			-124//DongleAPI.dll執行失敗
#define		CALL_DLL_TIMEOUT_ERROR			-125//考慮要不要留
#define		DLL_NOT_RETRY						-126//TM要求Retry交易,但Dongle回應非Retry交易
#define		DLL_NOT_SUPPORT_CPU_CARD			-127//ICER系統目前不支援CPU卡
#define 	RC_NOT_RETRY						-128//TM的Retry資料與實際卡片不符
#define 	API_IS_PROCESSING					-129//ICER API執行中,不可同時執行
#define 	ECC_CARD_REFUNDED					-130//票卡已退
#define 	ECC_CARD_TYPE_ERR					-131//卡別錯誤
#define 	ECC_LOCK_CARD						-132//黑名單卡,API鎖卡
#define 	ECC_CARD_BLOCKED					-133//票卡已鎖
#define 	ECC_CARD_NOT_ACTIVE 				-134//未開卡之票卡
//#define 	ECC_AUTOLOAD_SUCC 					-135//自動加值成功,但扣款失敗
#define 	ECC_SSL_LOAD_FAIL	 				-136//載入SSL DLL失敗
//#define		ICER_REQ2_FORMAT_FAIL			-137//ICERAPI2.REQ的XML格式錯誤
#define		BANK_DECLINE_FAIL					-138//R6主機拒絕交易
#define		DATA_NO_NEED_CHANGE_FAIL			-139//主機回覆,資料不需修改(學生展期時:票種與身份到期日不需修改)
#define		CAN_NOT_DISCOUNT_VOID				-140//折扣交易與原交易不符,不能取消
#define     DIRECTORY_PERMISSION_DENIED      -141//設備目錄權限不足

//#define		MALLOC_ERROR				-101
//#define		TCP_SOCKET_FAIL				-104
//#define		TCP_CONNECT_FAIL			-105
//#define		TCP_SEND_FAIL				-106
//#define		TCP_RECV_FAIL				-107
//#define		TCP_RECV_TIMEOUT_FAIL		-108
//#define		TCP_SHUTDOWN_FAIL			-109
//#define		TCP_CLOSESOCKET_FAIL		-110
//#define		TCP_HOST_DECLINE_FAIL		-111
//#define		FILE_OPEN_FAIL				-112
//#define		FILE_SEEK_FAIL				-113
//#define		FILE_TELL_FAIL				-114
//#define		FILE_READ_FAIL				-115
//#define		FILE_WRITE_FAIL				-116
//#define		XML_INITIAL_FAIL			-117
//#define		XML_READ_FAIL				-118
//#define		XML_WRITE_FAIL				-119
//#define		XML_PARSE_FAIL				-120

////#define		PACK_REQ_DATA_FAIL			-128

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif
