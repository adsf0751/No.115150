/**
 * @file IPass小額消費函式庫[系統參數]
 * @author 000645@i-pass.com.tw
 */

#ifndef IPASSMICROPAYMENT_SYSTEM
#  define IPASSMICROPAYMENT_SYSTEM

#  include "IPassMicroPayment_Config.h"
#  include "IPassMicroPayment_IntTypes.h"
//#  include "IPassMicroPayment_Time.h"
//
//#  include "IPassMicroPayment_Struct_CardContent.h"
//#  include "IPassMicroPayment_Struct.h"
#  include "IPassMicroPayment_ErrCode.h"
//#  include "IPassMicroPayment_DefineValue.h"
//
//#  include "IPassMicroPayment_ADDSAM.h"
//#  include "IPassMicroPayment_HAL.h"
//#  include "IPassMicroPayment_Mifare.h"
//#  include "IPassMicroPayment_Cryption.h"
//#  include "IPassMicroPayment_PAL.h"

#  include "IPassMicroPayment_PackedStructBegin.h"

typedef STRUCT
{
    uint8_t             sam_id[8];
    uint8_t             random_sam[8];
    uint8_t             sam_type;
    uint8_t             api_use_no[2];
}SamInfo;

typedef struct
{
    uint8_t client_ip[4];       ///< 主機IP
    uint8_t ams_ip[4];          ///< 加值主機IP
    uint8_t bms_ip[4];          ///< 銀行主機IP
    uint32_t txn_time;          ///< 交易時間
    uint32_t txn_amount;        ///< 交易金額
    uint32_t txn_type;          ///< 交易類別
    uint8_t txn_no[5];          ///< 交易序號
    uint8_t shop_no_hex[3];     ///< Shop No, 由POS代入之店家編號，若service provider為0x0B則此欄無作用，預設為0, Len=3, BYTE[]
    uint8_t pos_id;                     ///< POS ID, 由POS代入之收銀機編號，若service provider為0x0B則此欄無作用，預設為0, Len=1, BYTE[]
    uint8_t seq_no;                     ///< 0~65535，自0起算，若command的NTID前18 byte與上一個command相同，則Sequence No需+1，若相異則維持為0
    uint8_t rwid[4];
    SamInfo sam_info;
    uint8_t reader_status;

    uint8_t sys_id;                             ///< 系統代碼, Len=1, BYTE[]
    uint8_t spid;

    //uint8_t   host_cryption_type;

    /*
      uint8_t last_cs_esn[16];
      uint16_t last_card_txn_no;
      uint8_t last_txn_type;
      uint32_t last_txn_amount; ///< 交易金額
      uint32_t last_txn_time;           ///< 交易時間
    */

    int32_t rec_write_flag;

    NetworkTransactionID last_ntid;

    uint8_t cryption_type;
    //======端末開機 vike======
    //Sub_Company_ID,依TMS_0F_1040507文件修改,vike add 2015_05_08.
    uint8_t sub_company_id[2]; // Sub_Company_ID,由 POS 代入之子公司編號, Len=2, BYTE[].vike add 2015_05_08.
    //======端末開機 vike======
    //======Deduct mechanism On_Off vike======
    // uint8_t mechanism_type; // On=0x00, OFF=0x01.
    //======Deduct mechanism On_Off vike======

    //=== equip_type vike.2015_07_07 ===
    uint8_t equip_type;         ///< 交易設備類別, Len=1, BYTE
    //=== equip_type vike.2015_07_07 ===

    uint8_t operator_id[8];   ///< 19. 操作人員, Len=8, string //operator_id vike add.2015_08_24
}SystemInfo;

typedef struct {
    uint8_t is_occupied;
    uint32_t samslot;
    SystemInfo sys_info;
} MultiSystemInfo;

/* ORG
   typedef STRUCT
   {
   uint32_t index;
   uint8_t last_cs_esn[16];
   uint16_t last_card_txn_no;
   uint8_t last_txn_type;
   uint32_t last_txn_amount;    ///< 交易金額
   uint32_t last_txn_time;              ///< 交易時間
   uint16_t crc16;
   }SystemRecord;
*/
//===TxnDayClose vike.2015_06_11. ===
/*
  typedef STRUCT
  {
  // uint16_t txn_count_deduct;
  // uint32_t txn_amount_deduct;

  // uint16_t txn_count_cash_added;
  // uint32_t txn_amount_cash_added;

  // uint16_t txn_count_backto_added;
  // uint32_t txn_amount_backto_added;

  // uint16_t txn_count_auto_added;
  // uint32_t txn_amount_auto_added;

  // uint16_t txn_count_cancel_add;
  // uint32_t txn_amount_cancel_add;

  uint16_t txn_count_deduct;
  uint16_t txn_count_cash_added;
  uint16_t txn_count_backto_added;
  uint16_t txn_count_auto_added;
  uint16_t txn_count_cancel_add;

  uint32_t txn_amount_deduct;
  uint32_t txn_amount_cash_added;
  uint32_t txn_amount_backto_added;
  uint32_t txn_amount_auto_added;
  uint32_t txn_amount_cancel_add;
  } RecordTxnDayClose;
*/
typedef STRUCT
{
    uint32_t index;
    uint8_t last_cs_esn[16];
    uint16_t last_card_txn_no;
    uint8_t last_txn_type;
    uint32_t last_txn_amount;   ///< 交易金額
    uint32_t last_txn_time;             ///< 交易時間

    RecordTxnDayClose TxnDayClose; //日結交易紀錄.
    uint16_t crc16;

#  ifdef CONFIG_FLASH_DATA_ROUND_8
    uint8_t padding[7];
#  endif
}SystemRecord;

//===TxnDayClose vike.2015_06_11. ===

typedef STRUCT
{
    Record_TxLog rec;
    uint16_t crc16;

#  ifdef CONFIG_FLASH_DATA_ROUND_8
    uint8_t padding[4];
#  endif
}RecordTxLogInFlash;

typedef struct {
    uint8_t index;
    uint8_t length;
    RecordTxLogInFlash txlog[IPASS_MEMORY_TX_LOG_SIZE];
} MemoryTxLog;

enum
{
    MODE_OFFLINE_DEDUCT = 0,      // 0 : 小額消費
    MODE_ONLINE_DEDUCT,           // 1 : 小規模特店
    MODE_OFFLINE_DEDUCT_HOSPITAL, // 2 : 小額消費_醫院  A5/A6/A7/A8/A9
    MODE_ONLINE_DEDUCT_HOSPITAL,  // 3 : 小規模特店_醫院  A5/A6/A7/A8/A9
    MODE_UPPERLIMIT
};

#  include "IPassMicroPayment_PackedStructEnd.h"

#  ifdef __cplusplus
extern "C" {
#  endif // __cplusplus

extern int32_t IPassMP_addNewRecord( RecordTxLogInFlash *rec );
extern int32_t IPassMP_readRecord( uint32_t index, RecordTxLogInFlash *rec );

extern int32_t refreshSystemRecord( void );

extern SystemInfo g_sys_info;
extern SystemRecord g_sys_rec;
extern IPass_MifareCardKey g_mifare_keys[16];
extern IPassMP_CardMap g_ipass_card;
extern IPassMP_WriteCardMap g_card_for_write;
extern RecordTxLogInFlash *g_txnlog;
extern MemoryTxLog g_memory_tx_log;
extern uint32_t g_samslot;
extern int32_t g_system_err_code;
extern uint8_t g_output_tmp[300];

#  ifdef __cplusplus
}
#  endif // __cplusplus

#endif // IPASSMICROPAYMENT_SYSTEM
