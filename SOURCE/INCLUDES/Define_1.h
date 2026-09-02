/*
 * File:   Define.h
 * Author: user
 *
 * Created on 2015年6月7日, 下午 5:02
 */

/* 宣示是那一種機型 並宣告有哪些能力 START */
#define	_CASTLE_TYPE_V3C_	0
#define	_CASTLE_TYPE_V3M_	1
#define	_CASTLE_TYPE_V3P_	2
#define	_CASTLE_TYPE_V3UL_	3
#define	_CASTLE_TYPE_MP200_	4
#define	_CASTLE_TYPE_UPT1000_	5
#define	_CASTLE_TYPE_UPT1000F_	6

#define	_CASTLE_TYPE_NAME_UNKNOWN_	"UNKNOWN"
#define	_CASTLE_TYPE_NAME_V3C_		"V3C"
#define	_CASTLE_TYPE_NAME_V3M_		"V3M"
#define	_CASTLE_TYPE_NAME_V3P_		"V3P"
#define	_CASTLE_TYPE_NAME_V3UL_		"V3UL"
#define	_CASTLE_TYPE_NAME_MP200_	"MP200"
#define	_CASTLE_TYPE_NAME_UPT1000_	"UPT1000"
#define	_CASTLE_TYPE_NAME_UPT1000F_	"UPT1000F"

/* 決定是當程式用，還是當librarry用 */
#define _NEXSYS_APP_MODE_AP_	0
#define _NEXSYS_APP_MODE_SDK_	1

/* 宣告是那一個銀行的版本，由HDT第一個Record的HostLabel判別 */
#define _APVERSION_TYPE_NCCC_	0


#if	_MACHINE_TYPE_ == _CASTLE_TYPE_V3C_
	#define _TOUCH_CAPBILITY_		/* 是否有觸控能力 */
	#define _COMMUNICATION_CAPBILITY_	/* 是否有通訊能力 */
	#define _FULL_LCD_			/* 320 * 480 */
#elif	_MACHINE_TYPE_ == _CASTLE_TYPE_V3M_
	#define _TOUCH_CAPBILITY_		/* 是否有觸控能力 */
	#define _COMMUNICATION_CAPBILITY_	/* 是否有通訊能力 */
	#define _CRADLE_CAPBILITY_		/* 有底座 */
	#define _FULL_LCD_			/* 320 * 480 */
#elif	_MACHINE_TYPE_ == _CASTLE_TYPE_V3P_
	#define _TOUCH_CAPBILITY_		/* 是否有觸控能力 */
	#define _COMMUNICATION_CAPBILITY_	/* 是否有通訊能力 */
//	#define _HALF_LCD_			/* 320 * 240 */
	#define _FULL_LCD_			/* 320 * 480 */
#elif	_MACHINE_TYPE_ == _CASTLE_TYPE_V3UL_
	#define _HALF_LCD_			/* 320 * 240 */
#elif	_MACHINE_TYPE_ == _CASTLE_TYPE_MP200_
//	#define _TOUCH_CAPBILITY_		/* 是否有觸控能力 */
	#define _COMMUNICATION_CAPBILITY_	/* 是否有通訊能力 */
	#define _HALF_LCD_			/* 320 * 240 */
#elif	_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_
	#define _COMMUNICATION_CAPBILITY_	/* 是否有通訊能力 */
	#define _HALF_LCD_			/* 320 * 240 */
#elif	_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000F_
	#define _TOUCH_CAPBILITY_		/* 是否有觸控能力 */
	#define _COMMUNICATION_CAPBILITY_	/* 是否有通訊能力 */
	#define _FULL_LCD_			/* 320 * 480 */
#else	
	#define _TOUCH_CAPBILITY_		/* 是否有觸控能力 */
	#define _COMMUNICATION_CAPBILITY_	/* 是否有通訊能力 */
//	#define _HALF_LCD_			/* 320 * 240 */
	#define _FULL_LCD_			/* 320 * 480 */
#endif

/* 宣示是那一種機型 並宣告有哪些能力 END */

/* 定義回傳值 */
#define VS_TRUE                                 1
#define	VS_FALSE                                0

#define VS_SUCCESS                              0                                 /* 成功回傳值 */
#define VS_ERROR                                (-1)                              /* 錯誤回傳值 */
#define VS_FAILURE                              (-2)                              /* 失敗回傳值 */
#define VS_ESCAPE                               (-3)                              /* 空白回傳值 */
#define VS_TIMEOUT                              (-4)                              /* TIME OUT */
#define VS_HANDLE_NULL                          (-1)                              /* Ingenico機型定義為0x00，V5為-1 */

/* [114197] 中心收單作業ESG永續發展優化
 * [20260515] 新增選擇項目判斷值 */
#define VS_USER_AGREE                           1
#define VS_USER_DISAGREE                        0

/* 定義按鍵值 */
#define _KEY_0_					d_KBD_0
#define _KEY_1_					d_KBD_1
#define _KEY_2_					d_KBD_2
#define _KEY_3_					d_KBD_3
#define _KEY_4_					d_KBD_4
#define _KEY_5_					d_KBD_5
#define _KEY_6_					d_KBD_6
#define _KEY_7_					d_KBD_7
#define _KEY_8_					d_KBD_8
#define _KEY_9_					d_KBD_9
#define _KEY_F1_				d_KBD_F1

#if	(_MACHINE_TYPE_ == _CASTLE_TYPE_V3C_) || (_MACHINE_TYPE_ == _CASTLE_TYPE_V3P_)
	#define _KEY_F1_				d_KBD_F1
	#define _KEY_F2_				d_KBD_F2
	#define _KEY_F3_				d_KBD_F3
	#define _KEY_UP_HALF_				d_KBD_UP
	#define _KEY_DOWN_HALF_				d_KBD_DOWN
	#define _KEY_UP_				d_KBD_00
	#define _KEY_DOWN_				d_KBD_DOT
	#define _KEY_F4_				d_KBD_F4
	#define _KEY_CLEAR_				d_KBD_CLEAR
	#define _KEY_ENTER_				d_KBD_ENTER
	#define _KEY_CANCEL_				d_KBD_CANCEL
	#define _KEY_DOT_				d_KBD_DOT
	#define _KEY_FUNCTION_				d_KBD_00
	#define _KEY_ALPHA_				d_KBD_DOT
	#define _KEY_INVALID_                           d_KBD_INVALID
#elif	_MACHINE_TYPE_ == _CASTLE_TYPE_V3UL_
	#define _KEY_F1_				d_KBD_F1
	#define _KEY_F2_				d_KBD_F2
	#define _KEY_F3_				d_KBD_F3
	#define _KEY_UP_				d_KBD_00
	#define _KEY_DOWN_				d_KBD_DOT
	#define _KEY_F4_				d_KBD_F4
	#define _KEY_CLEAR_				d_KBD_CLEAR
	#define _KEY_ENTER_				d_KBD_ENTER
	#define _KEY_CANCEL_				d_KBD_CANCEL
	#define _KEY_DOT_				d_KBD_DOT
	#define _KEY_FUNCTION_				d_KBD_00
	#define _KEY_ALPHA_				d_KBD_DOT
	#define _KEY_INVALID_                           d_KBD_INVALID
#elif	_MACHINE_TYPE_ == _CASTLE_TYPE_V3M_
	#define _KEY_F2_				d_KBD_F2
	#define _KEY_F3_				d_KBD_F3
	#define _KEY_UP_				d_KBD_00
	#define _KEY_DOWN_				d_KBD_DOT
	#define _KEY_F4_				d_KBD_F4
	#define _KEY_CLEAR_				d_KBD_CLEAR
	#define _KEY_ENTER_				d_KBD_ENTER
	#define _KEY_CANCEL_				d_KBD_CANCEL
	#define _KEY_DOT_				d_KBD_DOT
	#define _KEY_FUNCTION_				d_KBD_00
	#define _KEY_ALPHA_				d_KBD_DOT
	#define _KEY_INVALID_                           d_KBD_INVALID
#elif	_MACHINE_TYPE_ == _CASTLE_TYPE_MP200_
	#define _KEY_F2_				d_KBD_F2
	#define _KEY_F3_				d_KBD_F3
	#define _KEY_UP_				d_KBD_UP
	#define _KEY_DOWN_				d_KBD_DOWN
	#define _KEY_F4_				d_KBD_F4
	#define _KEY_CLEAR_				d_KBD_CLEAR
	#define _KEY_ENTER_				d_KBD_ENTER
	#define _KEY_CANCEL_				d_KBD_CANCEL
	#define _KEY_DOT_				d_KBD_DOWN
	#define _KEY_FUNCTION_				d_KBD_00
	#define _KEY_ALPHA_				d_KBD_DOWN
	#define _KEY_INVALID_                           d_KBD_INVALID
#elif	(_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000_) || (_MACHINE_TYPE_ == _CASTLE_TYPE_UPT1000F_)
	#define _KEY_F2_				d_KBD_F2
	#define _KEY_F3_				d_KBD_F3
	#define _KEY_UP_				d_KBD_UP
	#define _KEY_DOWN_				d_KBD_DOWN
	#define _KEY_F4_				d_KBD_F4
	#define _KEY_CLEAR_				d_KBD_CLEAR
	#define _KEY_ENTER_				d_KBD_ENTER
	#define _KEY_CANCEL_				d_KBD_CANCEL
	#define _KEY_DOT_				d_KBD_DOWN
	#define _KEY_FUNCTION_				d_KBD_00
	#define _KEY_ALPHA_				d_KBD_DOWN
	#define _KEY_INVALID_                           d_KBD_INVALID
#else
	#define _KEY_F2_				d_KBD_F2
	#define _KEY_F3_				d_KBD_F3
	#define _KEY_UP_				d_KBD_0
	#define _KEY_DOWN_				d_KBD_DOT
	#define _KEY_F4_				d_KBD_F4
	#define _KEY_CLEAR_				d_KBD_CLEAR
	#define _KEY_ENTER_				d_KBD_ENTER
	#define _KEY_CANCEL_				d_KBD_CANCEL
	#define _KEY_DOT_				d_KBD_DOT
	#define _KEY_FUNCTION_				d_KBD_00
	#define _KEY_ALPHA_				d_KBD_DOT
	#define _KEY_INVALID_                           d_KBD_INVALID
#endif

#define _SAM_SLOT_1_				d_SC_SAM1
#define _SAM_SLOT_2_				d_SC_SAM2
#define _SAM_SLOT_3_				d_SC_SAM3
#define _SAM_SLOT_4_				d_SC_SAM4

#define _KEY_LEFT_				-1                              /* 目前沒用到的就塞-1 */
#define _KEY_RIGHT_				-1                              /* 目前沒用到的就塞-1 */
#define _KEY_STR_				-1                              /* 目前沒用到的就塞-1 */
#define _KEY_PND_				-1                              /* 目前沒用到的就塞-1 */
#define _KEY_TIMEOUT_				127				/* 原先為80，會與Function Key重複，但char只能放-128到127 */

/* 注意:Event類用的是Unsigned char 最多到255 */
/* 沒觸發EVENT */

typedef enum
{
	_NONE_EVENT_			= 0,	/* 沒有觸發事件 */
	_SWIPE_EVENT_			= 140,	/* 磁條觸發 */
	_EMV_DO_EVENT_			,	/* 晶片觸發 */
	_MENUKEYIN_EVENT_		,	/* MenuKeyIn */	
	_SENSOR_EVENT_			,	/* 感應觸發 */
	_TICKET_EVENT_			,	/* 票證流程 */
	_ECR_EVENT_			,	/* ECR觸發 */
	_MULTIFUNC_SLAVE_EVENT_		,	/* 當外接設備時 */
	_BARCODE_READER_EVENT_		,	/* 端末機接Barcode Reader Event */
	_POWER_MANAGEMENT_EVENT_	,	/* 進入省電模式 */
	_ESC_IDLE_UPLOAD_EVENT_		,	/* Idle上傳ESC */
	_TMS_SCHEDULE_INQUIRE_EVENT_	,	/* TMS 排程詢問 */
	_TMS_SCHEDULE_DOWNLOAD_EVENT_	,	/* TMS 排程下載 */
	_TMS_PROCESS_EFFECTIVE_EVENT_	,	/* TMS 參數生效 */
	_DCC_SCHEDULE_EVENT_		,	/* DCC排程下載 */
	_DCC_HOUR_NOTIFY_EVENT_		,	/* DCC 整點下載 */
	_TMS_DCC_SCHEDULE_EVENT_	,	/* TMS連動DCC下載 */
	_BOOTING_EVENT_			,	/* 開機流程 */
	_ESVC_AUTO_SIGNON_EVENT_	,	/* 電票自動SignOn流程 */
}EVENT;

typedef enum
{
	VS_USER_CANCEL			= (-1000),		/* 使用者取消交易 */
	VS_WAVE_INVALID_SCHEME_ERR		, 		/* 感應卡使用 */
	VS_WAVE_AMOUNT_ERR			, 		/* 感應卡使用 */
	VS_WAVE_ERROR				, 		/* 感應卡使用 */

	VS_NO_CARD_BIN				,		/* 找不到cardbin */
	VS_CARD_PAN_ERROR			,		/* 卡號錯誤 */
	VS_CARD_EXP_ERR				,		/* 有效期錯誤 */
	VS_LAST_PAGE				,		/* 回上一層 */
	VS_PREVIOUS_PAGE			,		/* 上一頁 */
	VS_NEXT_PAGE				,		/* 下一頁 */
	VS_FUNC_CLOSE_ERR			,		/* 功能未開，回上一頁 */
	VS_HG_REWARD_COMM_ERR			,		/* 紅利積點失敗 */
	VS_PRINTER_OVER_HEAT			,		/* 印表機過熱 */
	VS_PRINTER_PAPER_OUT			,		/* 列印沒紙 */
	VS_CALLBANK				,		/* call bank */
	VS_ISO_PACK_ERR				,		/* ISO PACK 失敗 */
	VS_ISO_UNPACK_ERROR			,		/* ISO UNPACK 失敗 */
	VS_COMM_ERROR				,		/* 通訊失敗 */
	VS_ICC_INSERT_ERROR			,		/* 插卡失敗 */
	VS_SWIPE_ERROR				,		/* 刷卡失敗 */
	VS_FILE_ERROR				,		/* 檔案失敗 */
	VS_READ_ERROR				, 		/* 讀檔失敗 */
	VS_WRITE_ERROR				, 		/* 寫檔失敗 */
	VS_USER_OPER_ERR			, 		/* 使用者操作錯誤 */
	VS_CLOSE_ERROR				, 		/* 檔案關閉失敗 */
	VS_OPEN_ERROR				, 		/* 檔案開啟失敗 */
	VS_NO_RECORD				, 		/* 沒有交易記錄 */
	VS_ABORT				, 		/* Operation Aborted (obsolete) */
	VS_EMV_CARD_OUT				,		/* 晶片卡被取出 */
	VS_TAP_AGAIN				,		/* 表示要再次感應，目前for悠遊卡使用 */
	VS_WRITE_KEY_ERROR			,		/* 表示寫key失敗 */
}RESPONSE_V3;

#define VS_BOOL                                 BOOL

#define _NULL_CH_                               ((char) 0)

/* Values for chip status flag */
#define _NOT_USING_CHIP_		0
#define _EMV_CARD_			1
#define _EMV_TABLE_NOT_USED_		-1

/* Transaction Result (避開0，避免沒設定以為是cancel)*/
#define _TRAN_RESULT_CANCELLED_			1		/* 被主機拒絕 */
#define _TRAN_RESULT_AUTHORIZED_		2		/* 授權(不論online or offline)*/
#define _TRAN_RESULT_REFERRAL_			3		/* Call Bank */
#define _TRAN_RESULT_SETTLE_UPLOAD_BATCH_	4		/* Batch Upload */
#define _TRAN_RESULT_DECLINED_			-1		/* 卡片拒絕 */
#define _TRAN_RESULT_COMM_ERROR_		-2		/* 通訊失敗 */
#define _TRAN_RESULT_GEN2AC_ERR_		-3		/* Second Gen AC 失敗 */
#define _TRAN_RESULT_PACK_ERR_			-4		/* ISO PACK 失敗 */
#define _TRAN_RESULT_UNPACK_ERR_		-5		/* ISO UNPACK 失敗*/
#define _TRAN_RESULT_HG_REWARD_COMM_ERR_	-6              /* HG通訊失敗 */
#define _TRAN_RESULT_HG_REWARD_CANCELLED_ERR_	-7              /* HG主機拒絕 */

#define _AP_ROOT_DIR_NAME_		"."
#define _AP_ROOT_PATH_			"./"
#define _FS_DIR_NAME_			"fs_data"
#define _FS_DATA_PATH_			"./fs_data/"
#define _CA_DIR_NAME_			"CA"
#define _CA_DATA_PATH_			"./fs_data/CA/"
#define _CLIENT_CER_DIR_NAME_		"CA"
#define _CLIENT_CER_DATA_PATH_		"./fs_data/CA/"
#define _EMV_EMVCL_DIR_NAME_		"EMVxml"
#define _EMV_EMVCL_DATA_PATH_		"./fs_data/EMVxml/"
#define _SD_PATH_			"/media/mdisk/"
#define _USB_PATH_			"/media/udisk/"
#define _AP_PUB_PATH_			"/home/ap/pub/"
#define _EDC_TRACE_LOG_FILE_NAME_	"TraceLog.txt"
#define _SHELL_RESPONSE_TEMP_		"Shell_Response_Temp.dat"
#define _SHELL_RESPONSE_		"Shell_Response.dat"

#define _FS_DATA_PATH_FROM_FS_DATA_	"./"
#define _CA_DATA_PATH_FROM_FS_DATA_	"./CA/"

#define _ERRORMSG_VERSION_NCCC_		0x0000
#define _ERRORMSG_VERSION_SDK_		0x0001

/* 錯誤訊息代碼 自定義 */
typedef enum
{
	_ERROR_CODE_V3_NONE_ = 0,					/* 0，代表無錯誤碼 */
	_ERROR_CODE_V3_USER_CANCEL_,					/* 使用者取消 */
	_ERROR_CODE_V3_COMM_,						/* 連線失敗 */
	_ERROR_CODE_V3_ISO_PACK_,					/* 電文PACK錯誤 */
	_ERROR_CODE_V3_ISO_UNPACK_,					/* 電文UNPACK錯誤 */
	_ERROR_CODE_V3_SETTLE_NOT_SUCCESS_,				/* 結帳未成功 */
        _ERROR_CODE_V3_SETTLE_RETRY_,                                   /* 結帳未成功重試 */        
	_ERROR_CODE_V3_REVERSAL_PROCESS_FAIL_,				/* Reversal 處理失敗 */
	_ERROR_CODE_V3_COMPOSE_FILE_NAME_ERROR_,			/* 檔名組成失敗 */
	_ERROR_CODE_V3_GET_HDPT_TAG_FAIL_,				/* 獲取HDPT TAG Fail */
	_ERROR_CODE_V3_SET_HDPT_TAG_FAIL_,				/* 獲取HDPT TAG Fail */
	_ERROR_CODE_V3_TSAM_ENCRYPT_FAIL_,				/* TSAM加密失敗 */
	_ERROR_CODE_V3_PAN_SIZE_ERROR_,					/* 卡號長度錯誤 */
	_ERROR_CODE_V3_PAN_TOO_LONG_ERROR_,				/* 卡號過長 */
	_ERROR_CODE_V3_EXP_DATE_NOT_VALID_,				/* 有效期不合法 */
	_ERROR_CODE_V3_TXN_FAIL_,					/* 交易失敗 */
	_ERROR_CODE_V3_TXN_CALL_BANK_,					/* 請聯絡銀行 */
	_ERROR_CODE_V3_TXN_TIMEOUT_,					/* 交易逾時 */
	_ERROR_CODE_V3_TXN_NOT_MEMBER_CARD_,				/* 非參加機構卡片 */
	_ERROR_CODE_V3_OPERATION_,					/* 操作錯誤 */
	_ERROR_CODE_V3_PLS_SELECT_CREDIT_HOST_,				/* 請選擇信用卡主機 */
	_ERROR_CODE_V3_NOT_SUPPORT_THIS_PAY_ITEM_,			/* 不支援該繳費項目 */
	_ERROR_CODE_V3_AUTH_CODE_NOT_VALID_,				/* 拒絕交易 */
	_ERROR_CODE_V3_WAVE_ERROR_,					/* 感應失敗 請改插卡或刷卡 */
	_ERROR_CODE_V3_WAVE_ERROR_NOT_ONE_CARD_,			/* 感應失敗 超過一張卡 */
	_ERROR_CODE_V3_WAVE_ERROR_FALLBACK_MEG_ICC,			/* 不接受此感應卡 請改刷卡或插卡 */
	_ERROR_CODE_V3_FUNC_CLOSE_,					/* 此功能已關閉 */
	_ERROR_CODE_V3_FUNC_NOT_SUPPORT_,				/* 此功能不支援 */
	_ERROR_CODE_V3_CTLS_DATA_SHORT_,				/* 感應資料不足 */
	_ERROR_CODE_V3_WAVE_ERROR_Z1_,					/* 感應資料不足 */
	_ERROR_CODE_V3_TICKET_AMOUNT_TOO_MUCH_IN_ONE_TRANSACTION_,	/* 金額超過單筆上限 */
	_ERROR_CODE_V3_TICKET_AMOUNT_NOT_ENOUGH_,			/* 餘額不足 */
	_ERROR_CODE_V3_TRT_NOT_FOUND_,					/* 找不到對應的TRT */
	_ERROR_CODE_V3_ECR_UNPACK_,					/* 接收資料錯誤 */
	_ERROR_CODE_V3_ECR_INST_FEE_NOT_0_,				/* 分期手續費不為0 */
	_ERROR_CODE_V3_EMV_CARD_OUT_,					/* 晶片卡被取出 */
	_ERROR_CODE_V3_EMV_FALLBACK_,					/* 請改刷磁條 */
	_ERROR_CODE_V3_EMV_PLS_READ_EMV_,				/* 請改讀晶片卡 */
	_ERROR_CODE_V3_EMV_PLS_READ_EMV_CUP_,				/* 請讀晶片，無晶片 請持卡人洽發卡行 */
	_ERROR_CODE_V3_FISC_01_READ_FAIL_,			/* 顯示讀卡失敗 *//* Smartpay錯誤，為了統一顯示 */
	_ERROR_CODE_V3_FISC_02_6982_,				/* 顯示卡片失效 */
	_ERROR_CODE_V3_FISC_03_TIME_ERROR_,			/* 目前無錯誤訊息(端末機抓不到時間)*/
	_ERROR_CODE_V3_FISC_04_MAC_TAC_,			/* 目前無錯誤訊息(MAC和TAC產生失敗)*/
	_ERROR_CODE_V3_FISC_05_NO_INCODE_,			/* 目前無錯誤訊息(沒有此交易別)*/
	_ERROR_CODE_V3_FISC_06_TMS_NOT_SUPPORT_,		/* 不接受此感應卡 請改插卡 */
	_ERROR_CODE_V3_FISC_07_AMT_OVERLIMIT_,			/* 超過感應限額 請改插卡 */
	_ERROR_CODE_V3_FISC_08_NO_CARD_BIN_,			/* 目前無錯誤訊息(CDT找不到Smartpay) */
	_ERROR_CODE_V3_FISC_09_NOT_RIGHT_INCODE_,		/* 請依正確卡別操作 */
	_ERROR_CODE_V3_FISC_10_LOGON_FAIL_,			/* 目前無錯誤訊息(安全認證失敗) */
	_ERROR_CODE_V3_FISC_11_FISC_FALLBACK_,			/* 請改插金融卡 */
	_ERROR_CODE_V3_FISC_12_SEND_APDU_FAIL_,			/* SEND APDU ERROR */
	_ERROR_CODE_V3_MULTI_FUNC_CTLS_,			/* 外接設備時，感應有問題 *//* 外接設備錯誤 */
	_ERROR_CODE_V3_MULTI_FUNC_TERMINAL_,			/* Terminal設定導致錯誤問題 */
	_ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_REAL_,		/* 非感應SmartPay錯誤時 */
	_ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_CTLS_REAL_,		/* 感應時，Smartpay錯誤要自己亮燈號 */
	_ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_OVER_AMOUNT_,	/* 感應時，Smartpay超過感應限額，要自己亮燈號 */
	_ERROR_CODE_V3_MULTI_FUNC_SMARTPAY_TIP_TOO_BIG_,	/* 感應時，Smartpay金額小於手續費，要自己亮燈號 */
	_ERROR_CODE_V3_FPG_FTC_ICC_CHECK_CARD,			/* 客製化005，聯名卡錯誤 */
	_ERROR_CODE_V3_WRITE_KEY_INIT_FAIL_,			/* 初始化失敗 */
	_ERROR_CODE_V3_SELECT_AID_TIMEOUT_,			/* 處理逾時，請重新交易 */
	_ERROR_CODE_V3_DO_NOT_CUP_KEY_,				/* 請勿按銀聯鍵 */
        _ERROR_CODE_V3_ID_INVALID_,                         /* ID身份證字號錯誤 */
}ERROR_CODE_V3;

/* 定義Table儲存方式 */
#define _PARAMETER_SAVE_WAY_DAT_CTOS_	1
#define _PARAMETER_SAVE_WAY_DAT_LINUX_	2
#define _PARAMETER_SAVE_WAY_DAT_SQLITE_	3

#define _PARAMETER_SAVE_WAY_DAT_	_PARAMETER_SAVE_WAY_DAT_SQLITE_

//#define _ECR_GET_CARD_NO_FLOW_		/* DEMO只回傳卡號、有效期使用 */

#define	_RESPONSE_CODE_APPROVAL_	"00"
#define	_RESPONSE_CODE_APPROVAL_LEN_	2

#define	_RESPONSE_CODE_CALL_BANK_	"01"
#define	_RESPONSE_CODE_CALL_BANK_LEN_	2
/* [20251219_BUG_MDF][ECC] 開啟電票退貨的判斷日期失敗時要拒絕 */
#define _ECR_CHECK_FILE_34_

/* 開啟時監控結果顯示在LOG  */
#define _DISP_REC_
