/**
 * @file IPass MP Library (小額消費函式庫) API
 * @author 000645@i-pass.com.tw
 * @author 張佳媛 <000942@i-pass.com.tw>
 * @author 李彥學 <000967@i-pass.com.tw>
 */

#ifndef IPASSMICROPAYMENT_LIB
#  define IPASSMICROPAYMENT_LIB

#  include "IPassMicroPayment_Config.h"
#  include "IPassMicroPayment_IntTypes.h"

#  define IPASSMP_VERSION "v21.03"
#  define IPASSMP_VERSION_HEX (0x2103)

enum
{
    IPASS_SAMSLOT_1 = 1,
    IPASS_SAMSLOT_2,
    IPASS_SAMSLOT_3,
    IPASS_SAMSLOT_4,
    IPASS_SAMSLOT_5,
    IPASS_SAMSLOT_6,
    IPASS_SAMSLOT_7,
    IPASS_SAMSLOT_8
};

#  define CONFIG_SAM_SLOT IPASS_SAMSLOT_1
#  define SAM_SLOT_PRESERVE_SIZE 2
#  define IPASS_MEMORY_TX_LOG_SIZE 6

#  include "IPassMicroPayment_PackedStructBegin.h"

#  ifndef IPASSMICROPROGRAM_STRUCT_CARDCONTENT_RECORD
#    define IPASSMICROPROGRAM_STRUCT_CARDCONTENT_RECORD
/// @brief 票卡記錄內容
typedef STRUCT
{
    uint8_t txn_sn_lsb;     ///< 交易序號
    uint32_t txn_datetime;  ///< 交易時間, GMT時間(Unix Time, Little Endian)
    uint8_t txn_type;       ///< 交易類別 參考」交二版」
    uint16_t txn_amount;    ///< 交易金額
    int16_t txn_ev;         ///< 交易後餘額
    uint8_t txn_spid;       ///< 交易系統編碼, 參考」交二版」
    uint8_t txn_comid;      ///< 交易業者編號
    uint8_t txn_deviceid[4];///< 交易設備編號
}IPass_CardRecord;
#  endif // IPASSMICROPROGRAM_STRUCT_CARDCONTENT_RECORD

///@brief 取得讀卡機資訊.
typedef STRUCT
{
    uint8_t rwid[4];      ///< 交易讀寫器編號, Len=4, BYTE[]
    uint8_t samid[8];     ///< SAM ID, Len=8, BYTE[]
}RWInfoExcute_Out;


///@brief 取得讀卡機狀態.
typedef STRUCT
{
    uint8_t reader_status;///< (1)讀卡機未認證：0x00, (2)讀卡機已認證：0x01,
                          ///< (3)讀卡機認證失敗：0xFF, (4)SAM已認證：0x02,
                          ///< (5)SAM認證失敗：0xFE
    uint8_t rwid[4];      ///< 交易讀寫器編號, Len=4, BYTE[]
    uint8_t ap_version[2];///< AP版本, Len=2, BYTE[]
}RWDetectExcute_Out;

//依TMS_0F_1040507文件修改.
///@brief 端末開機請求入參
typedef STRUCT
{
    uint8_t client_ip[4];     ///< 主機IP
    uint8_t ams_ip[4];        ///< 加值主機IP
    uint8_t bms_ip[4];        ///< 銀行主機IP
    uint8_t sys_id;           ///< 系統代碼, Len=1, BYTE[]
    uint8_t spid;             ///< Service Provider, Len=1, BYTE[]
    uint8_t shop_no_hex[3];   ///< Shop No, Len=3, BYTE[]
    uint8_t pos_id;           ///< POS ID, Len=1, BYTE[]
    uint8_t pos_txn_no[5];    ///< POS Transaction No, Len=5, BYTE[]
    uint32_t txn_unix_time;   ///< Transaction Date Time, Len=4, BYTE[],
                              ///< GMT時間(Unix Time, Little Endian)
    uint8_t crypto_type;      ///< 加密方式
    uint8_t sub_company_id[2];///< Sub_Company_ID,由 POS 代入之子公司編號, Len=2, BYTE[].

    uint8_t equip_type;       ///< 交易設備類別, Len=1, BYTE
                              //=== equip_type vike.2015_07_07 ===
}RWCheckRequest_In;

///@brief 票卡查詢請求入參
typedef STRUCT
{
    uint8_t mode_seleced;  ///< 票卡查詢選擇
    // uint32_t txn_datetime; ///< 交易時間
    uint32_t txn_date_time;///< 交易時間 //== Lock BEV DAVTI time vike 2015_05_28.==
}TicketQueryRequest_In;

///@brief 票卡查詢請求出參0
typedef STRUCT
{
    uint8_t card_sn_len;    ///< 卡號長度
    uint8_t card_sn[10];    ///< 卡號[4/7/10Byte卡號] 左靠右補0
}TicketQueryRequest_Out0;

///@brief 票卡查詢請求出參1
typedef STRUCT
{
    uint8_t card_sn_len;                 ///< 卡號長度
    uint8_t card_sn[10];                 ///< 卡號[4/7/10Byte卡號] 左靠右補0
    uint8_t card_sn_detail[16];          ///< 卡號[16Byte完整卡號]
    uint8_t card_type;                   ///< 票卡種類
    uint8_t card_status;                 ///< 卡片狀態
    uint8_t issue_code;                  ///< 發卡單位編號
    int32_t ev;                          ///< 電子票值 Little Endian排列
    uint8_t profile_tpye;                /// @brief 個人身份別[0x00: 一般, 0x01: 兒童, 0x02: 學生, 0x03: 老人>70歲, 0x04: 老人65~70, 0x05: 身心障礙]
    uint8_t profile_id[6];               ///< 身份證字號
    uint16_t trasaction_sn;              ///< 卡片交易序號
    uint8_t autoload_flag;               ///< 自動加值旗標, 0: Disable, 1: Enable
    uint16_t autoload_amount;            ///< 可自動加值金額
    uint32_t date_autopay;               ///< 最後一次加值日期(Unixtime)
    uint8_t count_autopay;               ///< 自動加值次數
    uint32_t date_deduct;                ///< 小額消費日期(Unixtime)
    uint16_t value_one_day_deduct;       ///< 小額消費當日累計金額

    IPass_CardRecord last_add_txn_record;///< 最近加值交易記錄
    IPass_CardRecord last_dec_txn_record;///< 六筆最近一筆交易記錄

    uint8_t register_flag;               //記名旗標 //Debit卡判斷 vike.2015_07_10===
    uint8_t bankID;                      //銀行代碼 //Debit卡判斷 vike.2015_07_10===
}TicketQueryRequest_Out1;

///@brief 票卡查詢請求出參2
typedef STRUCT
{
    uint8_t card_sn_len;                ///< 卡號長度
    uint8_t card_sn[10];                ///< 卡號[4/7/10Byte卡號] 左靠右補0
    IPass_CardRecord dec_txn_records[6];///< 六筆扣值交易記錄，
    IPass_CardRecord add_txn_records;   ///< 加值記錄
}TicketQueryRequest_Out2;

///@brief 購貨交易入參
typedef STRUCT
{
    uint8_t txn_no[5];     ///< 交易序號
    uint32_t txn_date_time;///< 交易日期時間(GMT時間(Unix Time, Little Endian))
    uint16_t txn_amount;   ///< 交易金額
}DeductValueRequest_In;

///@brief 加值交易請求入參
typedef STRUCT
{
    uint8_t txn_type;      ///< 加值方式, 0x42: 自動加值, 0x89: 退貨加值, 0x33: 手動加值, 0x8B: 溢扣返還.
    uint8_t txn_no[5];     ///< 交易序號
    uint32_t txn_date_time;///< 交易日期時間(GMT時間(Unix Time, Little Endian))
    uint16_t txn_amount;   ///< 交易金額
}AddValueRequest_In;

///@brief 取消加值交易請求入參
typedef STRUCT
{
    uint8_t txn_no[5];     ///< 交易序號
    uint32_t txn_date_time;///< 交易日期時間(Unix)
}CancelAddValueRequest_In;


/// @brief DAVTI資料結構
typedef STRUCT
{
    uint8_t record_type;            ///< 01. 紀錄類型, Len=1, string, 固定D
    uint8_t record_no[8];           ///< 02. 該日交易紀錄編號, Len=8, Number string
    uint8_t card_sn[8];             ///< 03. 卡號, Len=8, HEX string
    uint8_t factory_sn[24];         ///< 04. 廠商批號, Len=24, HEX string
    uint8_t txn_type[2];            ///< 05. 交易類型, Len=2, HEX string
    uint8_t card_type[2];           ///< 06. 卡片種類, Len=2, HEX string
    uint8_t txn_no[6];              ///< 07. 卡片交易序號, Len=6, Number string
    uint8_t equip_type[2];          ///< 08. 交易設備種類, Len=2, HEX string
    uint8_t equip_id[8];            ///< 09. 交易設備編號, Len=8
    uint8_t rwid[8];                ///< 10. 交易讀寫器編號, Len=8, HEX string
    uint8_t txn_date[8];            ///< 11. 交易日期, Len=8, Number string
    uint8_t txn_time[6];            ///< 12. 交易時間, Len=6, Number string
    uint8_t shop_no[6];             ///< 13. 店舖編號, Len=6, string
    uint8_t pos_txn_no[10];         ///< 14. 收銀機交易序號, Len=10, string
    uint8_t before_txn_value[6];    ///< 15. 交易前卡片餘額, Len=6, Number string
    uint8_t txn_value[6];           ///< 16. 交易金額 Len=6, Number string
    uint8_t after_txn_value[6];     ///< 17. 交易後卡片餘額, Len=6, Number string
    uint8_t payment_type[2];        ///< 18. 付款方式, Len=2, HEX string
    uint8_t operator_id[8];         ///< 19. 操作人員, Len=8, string
    uint8_t adjust_flag;            ///< 20. 上傳類別, Len=1, string
    uint8_t operate_date[8];        ///< 21. 營運日期, Len=8, string
    uint8_t issue_code[2];          ///< 22. 發卡單位, Len=2, string
    uint8_t card_expire_date[8];    ///< 23. 卡片主效期, Len=8, string
    uint8_t personal_id[2];         ///< 24. 個人身份別, Len=2, string
    uint8_t personal_expire_date[8];///< 25. 個人有效期限, Len=8, string
    uint8_t area_priority[2];       ///< 26. 身份優惠識別單位, Len=2, string
    uint8_t pre_txn_lsb[2];         ///< 27. 前筆交易序號, Len=2, HEX string
    uint8_t pre_txn_date[8];        ///< 28. 前筆交易日期, Len=8, Number string
    uint8_t pre_txn_time[6];        ///< 29. 前筆交易時間, Len=6, Number string
    uint8_t pre_txn_class[2];       ///< 30. 前筆交易類別, Len=2, HEX string
    uint8_t pre_txn_value[6];       ///< 31. 前筆交易票值, Len=6, Number string
    uint8_t pre_after_txn_value[6]; ///< 32. 前筆交易後票值, Len=6, Number string
    uint8_t pre_system_id[2];       ///< 33. 前筆交易系統編號, Len=2, HEX string
    uint8_t pre_location_id[2];     ///< 34. 前筆交易地點編號, Len=2, HEX string
    uint8_t pre_equip_id[8];        ///< 35. 前筆交易設備編號, Len=8, HEX string
    uint8_t card_status_aft_txn[2]; ///< 36. 交易後卡片狀態, Len=2, HEX string
    uint8_t txn_error_code[4];      ///< 37. 錯誤代碼, Len=4, HEX string
    uint8_t execute_status[2];      ///< 38. 執行結果, Len=2, HEX string
    uint8_t api_version[4];         ///< 39. API版號, Len=4, HEX string
    uint8_t samid[16];              ///< 40. SAM_ID, Len=16, HEX string
    uint8_t accumulate_value[6];    ///< 41. 交易前累積金額, Len=6, Number string
    uint8_t rfu[30];                ///< 42. Reserved, Len=30, string
    uint8_t tac_value[8];           ///< 43. 交易驗證碼(TAC), Len=8, HEX string
    uint8_t tac_result;             ///< 44. 交易驗證碼結果, Len=1, string
    uint8_t record_separator[2];    ///< 45. RecordSeparator, Len=2, Number,2015_05_22.
}DAVTITxnRecord;


/// @brief 日結交易資料結構
typedef STRUCT
{
    uint32_t txn_count_deduct;
    uint32_t txn_count_cash_added;
    uint32_t txn_count_backto_added;
    uint32_t txn_count_auto_added;
    uint32_t txn_count_cancel_add;

    uint32_t txn_amount_deduct;
    uint32_t txn_amount_cash_added;
    uint32_t txn_amount_backto_added;
    uint32_t txn_amount_auto_added;
    uint32_t txn_amount_cancel_add;
} RecordTxnDayClose;

/// @brief Mifare Key
typedef struct
{
    uint8_t key_type;   ///< Key Type: 0x60 = KeyA, 0x61 = KeyB
    uint8_t key[6];     ///< 卡片金鑰
} IPass_MifareCardKey_Out;

#  ifdef CONFIG_GET_RF_MODULE_INFO
//=== IPassMP_GetRFmoduleInfo vike.2015_12_31 ===
/// @brief RF功率參數結構
typedef STRUCT
{
    uint8_t ModCond;
    uint8_t CwCond;
    uint8_t RxControl1;
    uint8_t RxThreshold;
} AntennaParam;
//=== IPassMP_GetRFmoduleInfo vike.2015_12_31 ===
#  endif

#  ifdef CONFIG_END_TRANSACTION
typedef STRUCT {
    uint8_t pos_txn_type;    // POS 端交易類型
    uint8_t pos_txn_result;  // POS 端交易結果
    uint8_t csn[4];          // 卡號
    uint16_t txn_value;      // 交易金額
    int16_t after_txn_value; // 交易後金額
    uint16_t autoload_value; // 自動加值金額
} EndTransaction_In;
#  endif

//=== V5S ===
#  ifdef CONFIG_FUN_SERVICE_INFO  //營運參數.端末設備狀態.function.

/// @brief 營運參數請求入參
typedef STRUCT
{
    uint8_t pos_txn_no[5];    ///< POS Transaction No, Len=5, BYTE[]
    uint32_t txn_unix_time;   ///< Transaction Date Time, Len=4, BYTE[], GMT時間(Unix Time, Little Endian)
    uint8_t operator_id[8];   ///< 19. 操作人員, Len=8, string //operator_id vike add.2015_08_24
}ServParaRequest_In;

/// @brief 營運參數命令出參
typedef STRUCT
{ //依TMS_0I_1040716文件修改.
    uint8_t  message_code[2];         //1.  2 (2) BIN Message Code 0xA111
    uint8_t  response_date_time[4];   //2.  4(6)  BIN 回覆參數更新之時間，UNIX Time
    uint8_t  sp_desc[20];             //3.  20(26)  Big5  業者名稱，最多10個中文字
    uint8_t  socket_ip[4];            //4.  4(30) BIN 連接Socket Server 之IP
    uint16_t socket_port;             //5.  2(32) BIN 連接Socket Server之 Port
    uint32_t socket_enable_time;      //6.  4(36) BIN 啟用時間(UNIX Time)
    uint8_t  ftp_ip[4];               //7.  4(40) BIN 登入FTP Server之IP
    uint16_t ftp_port;                //8.  2(42) BIN 登入FTP Server之 Port
    uint8_t  ftp_userid[10];          //9.  10(52)  ASCII 登入FTP Server之帳號
    uint8_t  ftp_userpwd[10];         //10. 10(62)  ASCII 登入FTP Server之密碼
    uint8_t  overtime_limit;          //11. 1(63) BIN 登入逾時時間，每次交易之間若超過此期限，將強制登出，單位為分鐘
    uint8_t  backlight_duration;      //12. 1(64) BIN 背光持續時間，OFAM開啟背光後之持續時間，單位為秒，若為0則表不會主動關閉背光
    uint16_t min_value;               //13. 2(66) BIN 加值最小金額
    uint16_t step_value;              //14. 2(68) BIN 步進金額，如MinValue = 500, StepValue = 2，則加值金額將允許為500 + 2n, n>=0
    uint8_t  log_reservation;         //15. 1(69) BIN Log檔保留天數，若為0則代表不使用天數限制
    uint8_t  eq_enable_flag;          //16. 1(70) BIN 讀寫器載具設備是否可啟用
    uint8_t  ipassuserid[10];         //17. 10 (80) ASCII 此機器於WCS所設定的高捷操作人員帳號名稱，ASCII編碼
    uint8_t  ipassuserpwd[10];        //18. 10 (90) ASCII 此機器於WCS所設定的高捷操作人員帳號密碼，ASCII編碼
    uint8_t  rw_firmware_version[30]; //19. 30(120) ASCII RW韌體版本編號
    //uint8_t  ap_api_version[30];      //20. 30(150) ASCII 應用程式/函式庫之版本編號
    uint8_t  reserve[28];             //20. 28(148) BIN  保留欄位default 0x00
    uint16_t ap_api_version;          //21. 2(150) BIN 應用程式/函式庫之版本編號
    uint8_t  Sub_SP_Desc[20];         //22. 20(170) Big5  次業者名稱，最多10個中文字//V5S Sub_SP_Desc vike.2015_07_17
}ServParaExecute_Out;

//依TMS_0F_1040507文件修改.
///@brief 端末設備狀態請求入參
typedef STRUCT
{
    uint8_t pos_txn_no[5];          ///< POS Transaction No, Len=5, BYTE[]
    uint32_t txn_unix_time;         ///< Transaction Date Time, Len=4, BYTE[], GMT時間(Unix Time, Little Endian)
    uint8_t status_code;            //Len=1, BYTE[], Status Code
    uint8_t status_code_message[30];//Len=30, ASCII  Status Code Message
}RWStatusRequest_In;
#  endif


//=== V5S RWRegister vike.2015_06_17 ===
#  ifdef FUN_RWREGISTER //FUN_RWREGISTER vike.2015_11_10
//依TMS_0I_1040716文件修改.
///@brief 卡機開機註冊請求入參
typedef STRUCT
{
    uint8_t client_ip[4];     ///< 主機IP
    uint8_t ams_ip[4];        ///< 加值主機IP
    uint8_t bms_ip[4];        ///< 銀行主機IP
    uint8_t sys_id;           ///< 系統代碼, Len=1, BYTE[]
    uint8_t spid;             ///< Service Provider, Len=1, BYTE[]
    uint8_t shop_no_hex[3];   ///< Shop No, Len=3, BYTE[]
    uint8_t pos_id;           ///< POS ID, Len=1, BYTE[]
    uint8_t crypto_type;      ///< 加密方式
    uint8_t sub_company_id[2];///< Sub_Company_ID,由 POS 代入之子公司編號, Len=2, BYTE[].

    uint8_t pos_txn_no[5];      //1.Len=5  BIN    由POS代入之收銀機交易序號
    uint32_t txn_unix_time;     //2.Len=4  BIN    要求首度開機(註冊)之時間，UNIX Time
    uint8_t ipass_user_id[10];  //3.Len=10 ASCII  登入此機器之IPASS操作人員帳號名稱，ASCII編碼
    uint8_t ipass_user_pwd[10]; //4.Len=10 ASCII  登入此機器之IPASS操作人員帳號密碼，ASCII編碼
    uint8_t sp_user_id[10];     //5.Len=10 ASCII  業者可結帳人員帳號名稱，指定後將成為該OFAM可執行結帳、上傳與要求水位重置授權之帳密，ASCII編碼
    uint8_t sp_user_pwd[10];    //6.Len=10 ASCII  業者可結帳人員帳號密碼，指定後將成為該OFAM之可執行結帳、上傳與要求水位重置授權權限之密碼，ASCII編碼
}RWRegisterRequest_In;

///@brief 卡機開機註冊命令出參
typedef STRUCT
{
    uint8_t message_code[2];    //1.Len=2 (2) BIN     Message Code 0xA110
    uint32_t txn_unix_time;     //2.Len=4 (6) BIN     回覆首度開機(註冊)之時間，UNIX Time
    uint8_t ipass_user_id[10];  //3.Len=10 (16) ASCII 此機器於WCS所設定的高捷操作人員帳號名稱，ASCII編碼
    uint8_t ipass_user_pwd[10]; //4.Len=10 (26) ASCII 此機器於WCS所設定的高捷操作人員帳號密碼，ASCII編碼
}RWRegisterExecute_Out;
#  endif
//=== V5S RWRegister vike.2015_06_17 ===

typedef struct {
    uint32_t txn_time;
} BMSQueryRequest_In;

#  include "IPassMicroPayment_PackedStructEnd.h"

#  ifdef __cplusplus
extern "C" {
#  endif // __cplusplus

#  ifdef CONFIG_WINDOWS
#    define IPASS_API __declspec(dllexport)
#  else
#    define IPASS_API extern
#  endif

//typedef void DebugPrintFunc( unsigned short cmd,
//                             unsigned char sn,
//                             unsigned short sc,
//                             unsigned char *pdata,
//                             unsigned int data_len );

#if defined(CONFIG_QP3000S) || defined(CONFIG_QP3000E) //Add_ParkingLot vike.2016_05_17
// SHORT ResponseProcess(USHORT dlen, USHORT cmd, BYTE sn, SHORT sc, BYTE *data)
    typedef void DebugPrintFunc( unsigned int data_len,
                                 unsigned short cmd,
                                 unsigned char sn,
                                 unsigned short sc,
                                 unsigned char *pdata ); //CONFIG_QP3000S
#else
typedef void DebugPrintFunc( unsigned short cmd,
                             unsigned char sn,
                             unsigned short sc,
                             unsigned char *pdata,
                             unsigned int data_len ); //ORG
#endif //Add_ParkingLot vike.2016_05_17

// =========================== Initialization ===========================
#ifdef CONFIG_SET_DEVICE_ID
IPASS_API uint16_t IPassMP_initLib( DebugPrintFunc *func, uint32_t samslot, const uint8_t deviceID[4] );
#else
IPASS_API uint16_t IPassMP_initLib( DebugPrintFunc *func, uint32_t samslot );
#endif

// ============================== Requests ==============================
IPASS_API uint16_t IPassMP_RequestCheckRW( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *len_output , const uint8_t *deviceID); //讀卡機授權請求

#  ifdef CONFIG_API_POLLING_TIME_AS_INPUT_2BYTE
IPASS_API uint16_t IPassMP_RequestQueryTicket( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//票卡查詢請求
IPASS_API uint16_t IPassMP_RequestDeductValue( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//購貨交易請求
IPASS_API uint16_t IPassMP_RequestAddValue( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output ); //退貨交易、加值交易、自動加值交易請求
IPASS_API uint16_t IPassMP_RequestCancelAddValue( uint8_t *pdata_input, uint32_t len_input,uint8_t *pdata_output,uint32_t *plen_output ); //取消加值交易請求
#  else
IPASS_API uint16_t IPassMP_RequestQueryTicket( uint32_t polling_time, uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//票卡查詢請求
IPASS_API uint16_t IPassMP_RequestDeductValue( uint32_t polling_time, uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//購貨交易請求
IPASS_API uint16_t IPassMP_RequestAddValue(uint32_t polling_time, uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output ); //退貨交易、加值交易、自動加值交易請求
IPASS_API uint16_t IPassMP_RequestCancelAddValue(uint32_t polling_time,uint8_t *pdata_input, uint32_t len_input,uint8_t *pdata_output,uint32_t *plen_output); //取消加值交易請求
#  endif

// ============================== Executes ==============================
IPASS_API uint16_t IPassMP_ExcuteCheckRW( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *len_output ); //讀卡機授權
IPASS_API uint16_t IPassMP_ExecuteQueryTicket( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output ); //票卡查詢命令
IPASS_API uint16_t IPassMP_ExecuteDeductValue( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//購貨交易
IPASS_API uint16_t IPassMP_ExecuteAddValue( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output ); //退貨交易、加值交易、自動加值交易
IPASS_API uint16_t IPassMP_ExecuteCancelAddValue(uint8_t *pdata_input,uint32_t len_input,uint8_t *pdata_output,uint32_t *plen_output); //取消加值交易

// =========================== Other functions ==========================
IPASS_API uint16_t IPassMP_ExecuteBlockTicket(uint8_t *pdata_input,uint32_t len_input,uint8_t *pdata_output,uint32_t *plen_output); //鎖卡交易
IPASS_API uint16_t IPassMP_IsRWCheck( uint8_t *pdata_input,uint32_t len_input,uint8_t *pdata_output,uint32_t *plen_output ); //查詢卡機狀態
IPASS_API uint16_t IPassMP_GetTxLog( uint32_t index,uint8_t *pdata_output,uint32_t *plen_output); //取得卡機交易紀錄、取得卡機最新一筆交易紀錄

#  ifdef CONFIG_GET_TX_LOG2
IPASS_API uint16_t IPassMP_GetTxLog2(
    uint8_t *pdata_input, uint32_t len_input,
    uint8_t *pdata_output, uint32_t *plen_output);
#  endif

IPASS_API uint16_t IPassMP_GetMemoryTxLog(uint32_t index,
                                          uint8_t *pdata_output,
                                          uint32_t *plen_output);

IPASS_API uint16_t IPassMP_GetTxnDayCloseRecord( uint8_t *pdata_output, uint32_t *plen_output ); //日結交易紀錄
IPASS_API uint16_t IPassMP_FastQueryTicket( uint32_t polling_time, uint8_t *pdata_output, uint32_t *plen_output );
IPASS_API uint16_t IPassMP_GetMifare_firstKey( uint8_t *card_id, uint8_t *pdata_output, uint32_t *plen_output ); //取得票卡金鑰命令,取得sector0的keyB.
IPASS_API uint16_t IPassMP_GetFirstKey( uint8_t *card_sn, uint8_t *card_sn_len );
IPASS_API uint16_t IPassMP_GetSystemInfo( uint8_t *pdata_output, uint32_t *plen_output ); //取得SAMID和RWID.

#  ifdef CONFIG_GET_RF_MODULE_INFO
IPASS_API uint16_t IPassMP_GetRFmoduleInfo( uint8_t *pdata_output, uint32_t *plen_output ); //讀取RF功率參數功能.
#  endif

#  ifdef CONFIG_GET_TICKS_COUNT_10MS
IPASS_API void IPassMP_getTicks_Count_10ms(void); //詢卡計算等待時間用.請將此function放在10ms的Timer中,每10ms呼叫一次,謝謝//add getTicks vike.2015_09_22
// void IPassMP_getTicks_Count_10ms(void)
// { //10ms的定時中斷
// extern unsigned int g_ticks;
// g_ticks += 10;
// }
#  endif

IPASS_API int32_t IPassMP_getSystemError( void );//Library內部錯誤,return 錯誤子代碼
IPASS_API int32_t IPassMP_translateReturnCodeString(int32_t ret_code, char **err_string ); //錯誤描述字表,return 錯誤描述字表長度
IPASS_API int32_t IPassMP_SAM_ReturnCode(uint8_t *pdata_output, uint32_t *plen_output ); //ImprovePollCard_MP507 vike.2017_09_01

#  ifdef CONFIG_FUN_SERVICE_INFO  //營運參數.端末設備狀態.function.
IPASS_API uint16_t IPassMP_RequestServiceParameter(uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output ); //營運參數請求命令.
IPASS_API uint16_t IPassMP_ExecuteServiceParameter(uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//營運參數命令.
IPASS_API uint16_t IPassMP_RequestGetRWStatus(uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//端末設備狀態請求命令
IPASS_API uint16_t IPassMP_ExecuteGetRWStatus(uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//端末設備狀態命令
#  endif

#  ifdef FUN_RWREGISTER //FUN_RWREGISTER vike.2015_11_10
IPASS_API uint16_t IPassMP_RequestRWRegister( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//卡機開機註冊請求命令.
IPASS_API uint16_t IPassMP_ExecuteRWRegister( uint8_t *pdata_input, uint32_t len_input, uint8_t *pdata_output, uint32_t *plen_output );//卡機開機註冊命令.
#  endif

#  ifdef CONFIG_END_TRANSACTION
IPASS_API uint16_t IPassMP_ExecuteEndTransaction(
    uint8_t *pdata_input, uint32_t len_input,
    uint8_t *pdata_output, uint32_t *plen_output ); // 結束交易命令.
#  endif

IPASS_API int32_t IPassMP_makeMac(uint8_t mac_out[4],
                                  const uint8_t *in, uint32_t in_len,
                                  uint32_t rec_len);
IPASS_API int32_t IPassMP_makeMac2(uint8_t mac_out[4],
                                   const uint8_t *in, uint32_t in_len,
                                   uint32_t rec_len, int reset_flag);
IPASS_API int32_t IPassMP_IZD(uint8_t *inout, uint32_t *inout_len,
                              uint32_t buf_len);
IPASS_API int32_t IPassMP_IZD2(uint8_t *inout, uint32_t in_len,
                               int reset_flag);

#ifdef CONFIG_DEVELOPMENT_TOOLS
IPASS_API uint16_t IPassMP_unlockTicket
(uint8_t *pdata_input, uint32_t len_input,
 uint8_t *pdata_output, uint32_t *plen_output);

IPASS_API uint16_t IPassMP_autoloadOnTicket
(uint8_t *pdata_input, uint32_t len_input,
 uint8_t *pdata_output, uint32_t *plen_output);

IPASS_API uint16_t IPassMP_setSecondaryPurse
(uint8_t *pdata_input, uint32_t len_input,
 uint8_t *pdata_output, uint32_t *plen_output);
#endif

IPASS_API uint16_t IPassMP_RequestQueryBMS(
    uint32_t polling_time,
    const uint8_t *pdata_input, uint32_t len_input,
    uint8_t *pdata_output, uint32_t *len_input_output);

IPASS_API uint16_t IPassMP_ExecuteQueryBMS(
    const uint8_t *pdata_input, uint32_t len_input,
    uint8_t *pdata_output, uint32_t *len_input_output);

#  ifdef CONFIG_MULTIPLE_ADDSAM
IPASS_API uint8_t IPassMP_SetSAMSlot(uint32_t samslot);
#  endif

IPASS_API uint8_t IPassMP_GetSelectMode( void );
//IPASS_API uint8_t IPassMP_SetSelectMode(uint8_t value);
#  ifdef __cplusplus
}
#  endif // __cplusplus

#endif // IPASSMICROPAYMENT_LIB
