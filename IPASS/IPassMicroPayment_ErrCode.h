/**
 * @file IPass小額消費函式庫
 * @author 000645@i-pass.com.tw
 */
#ifndef IPASSMICROPROGRAM_ERRCODE
#  define IPASSMICROPROGRAM_ERRCODE


#  define IPASS_ERROR_CMDFORMAT     (0xFFFE) ///< 命令格式錯誤
#  define IPASS_ERROR_CMDCLASS      (0xFFFD) ///< 命令類別錯誤
#  define IPASS_ERROR_CMDBBC        (0xFFFC) ///< BCC錯誤
#  define IPASS_ERROR_CMDTIMEOUT    (0xFFFB) ///< 命令接收逾時
#  define IPASS_ERROR_CMDLEN        (0xFFFA) ///< 資料長度錯誤
#  define IPASS_ERROR_NOAUTH        (0xFFF9) ///< 讀卡機尚未認證
#  define IPASS_ERROR_LOCK          (0xFFF8) ///< 讀卡機已鎖
#  define IPASS_ERROR_SAMNOAUTH     (0xFFF7) ///< SAM卡尚未認證成功"
#  define IPASS_ERROR_DBGMSG        (0xFF00) ///< Debug Print訊息

/// @brief SAM 卡指令執行錯誤代碼  0000-00FF
#  define IPASS_EXECUTE_OK          (0x0000) ///< 交易成功"
#  define IPASS_ERROR_NET           (0x0001) ///< 交易成功"網路失敗

//=== return_flow vike.2015_06_29 ===
#  define IPASS_ERROR_RECOVERY      (0x0002) ///< 票值回復
#  define IPASS_ERROR_AUTOLOAD_OFF  (0x0003) ///< 關閉自動加值
#  define IPASS_ERROR_AUTOLOAD_ON   (0x0004) ///< 開啟自動加值
//=== return_flow vike.2015_06_29 ===

/// @brief SAM 卡指令執行錯誤代碼  0100-01FF
#  define IPASS_ERROR_SAM_EXCUTE        (0x0100) ///< SAM 執行錯誤
#  define IPASS_ERROR_RESET_SAM_MODULR  (0x0101) ///< 1 . (1)  //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_SELECT_AID        (0x0102) ///< 2 . (2)  //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_SEND_RWID_TO_AUTH (0x0103) ///< 3 . (3)  //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_SAMID         (0x0104) ///< 4 . (4)  //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_RANDOM        (0x0105) ///< 5 . (5)  //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_SAM_AUTHENCATION  (0x0106) ///< 6 . (6)  //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_MIFAREKEY     (0x0107) ///< 7 . (7)  //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_APIUSENO      (0x0108) ///< 8 . (8)  //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_SAMTYPE       (0x0109) ///< 9 . (9)  //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_ATRC_CODE     (0x010A) ///< 10. (10) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_ATAC_CODE     (0x010B) ///< 11. (11) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_PTRC_CODE     (0x010C) ///< 12. (12) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_PTAC_CODE     (0x010D) ///< 13. (13) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_AAC_CODE      (0x010E) ///< 14. (14) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_GET_PAC_CODE      (0x010F) ///< 15. (15) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_SET_APIUSENO      (0x0110) ///< 16. (16) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_NO_SUCH_SLOT      (0x0111) ///< 17. (17) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_NOT_ENOUGH_SPACE  (0x0112) ///< g_multi_sys_info 空間不足夠


/// @brief 票卡區塊認證回傳錯誤代碼  0200-02FF
#  define IPASS_ERROR_CARD_AUTH   (0x0200) ///< 卡片無法讀取
#  define IPASS_ERROR_AUTH_S0     (0x0201) ///< 1. (25) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_AUTH_S1     (0x0202) ///< 2. (26) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_AUTH_S2     (0x0203) ///< 3. (27) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_AUTH_S3     (0x0204) ///< 4. (28) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_AUTH_S4     (0x0205) ///< 5. (29) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_AUTH_S5     (0x0206) ///< 6. (30) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_AUTH_S6     (0x0207) ///< 7. (31) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_AUTH_S7     (0x0208) ///< 8. (32) //ini_Error_Code vike.2015_11_10

/// @brief 票卡資料讀取回傳錯誤代碼  0300-03FF
#  define IPASS_ERROR_CARD_READ         (0x0300) ///< 卡片資料讀取錯誤
#  define IPASS_ERROR_CARD_READPURSE    (0x0301) ///< 電子票值重讀失敗
#  define IPASS_ERROR_READ_S0B0         (0x0302) ///< 1 . (41) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S1B0         (0x0303) ///< 2 . (42) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S1B1         (0x0304) ///< 3 . (43) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S1B2         (0x0305) ///< 4 . (44) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S2B0         (0x0306) ///< 5 . (45) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S2B1         (0x0307) ///< 6 . (46) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S2B2         (0x0308) ///< 7 . (47) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S3B0         (0x0309) ///< 8 . (48) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S4B0         (0x030A) ///< 9 . (49) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S4B1         (0x030B) ///< 10. (50) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S4B2         (0x030C) ///< 11. (51) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S5B0         (0x030D) ///< 12. (52) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S5B1         (0x030E) ///< 13. (53) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S5B2         (0x030F) ///< 14. (54) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S6B0         (0x0310) ///< 15. (55) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_READ_S7B0         (0x0311) ///< 16. (56) //ini_Error_Code vike.2015_11_10


/// @brief 票卡資料寫入回傳錯誤代碼  0400-04FF
#  define IPASS_ERROR_CARD_WRITE             (0x0400) ///< 卡片寫入錯誤
#  define IPASS_ERROR_WRITE_S1B0             (0x0401) ///< 1 .(71) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S1B1             (0x0402) ///< 2 .(72) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S2B0             (0x0403) ///< 3 .(73) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S2B1             (0x0404) ///< 4 .(74) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S2B2             (0x0405) ///< 5 .(75) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S3B0             (0x0406) ///< 6 .(76) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S4B0             (0x0407) ///< 7 .(77) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S4B1             (0x0408) ///< 8 .(78) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S4B2             (0x0409) ///< 9 .(79) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S5B0             (0x040A) ///< 10.(80) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S5B1             (0x040B) ///< 11.(81) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S5B2             (0x040C) ///< 12.(82) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S6B0             (0x040D) ///< 13.(83) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S6B1             (0x040E) ///< 14.(84) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_WRITE_S6B2             (0x040F) ///< 15.(85) //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_PURCHASE_AUTOLOAD_FAIL (0x0410) ///< 16.(86) //Copy Form ErrorCode.h //ini_Error_Code vike.2015_11_10
#  define IPASS_ERROR_PURCHASE_AUTOLOADOK_DEDUCT_FAIL (0x0411) ///< 17.(87)  //Copy Form ErrorCode.h //ini_Error_Code vike.2015_11_10

/// @brief 資料檢查回傳錯誤代碼  0500-05FF
#  define IPASS_ERROR_CARD_ISSUECODE         (0x0500) ///< 非一卡通票卡
#  define IPASS_ERROR_CARD_EXPIREDATE        (0x0501) ///< 票卡有效日期逾期
#  define IPASS_ERROR_CARD_VERSION           (0x0502) ///< 票卡版本異常
#  define IPASS_ERROR_CARD_STATUS            (0x0503) ///< 票卡狀態異常(已鎖卡)
#  define IPASS_ERROR_CARD_VERIFY_CAC        (0x0504) ///< 票卡防偽驗證錯誤
#  define IPASS_ERROR_EV_NOMATCH_BEV         (0x0505) ///< 電子票值異常
#  define IPASS_ERROR_CARD_TXNNO_INDEX       (0x0506) ///< 交易記錄指標錯誤
#  define IPASS_ERROR_PERSONAL_EXPIREDATE    (0x0507) ///< 個人有效日期逾期
#  define IPASS_ERROR_NOVALID_CARDTYPE       (0x0508) ///< 非儲值型票卡無法進行交易
#  define IPASS_ERROR_NOVALID_TXNVALUE       (0x0509) ///< 交易金額超過 1000 元上限
#  define IPASS_ERROR_NOAUTOLOAD_RIGHT       (0x0510) ///< 無自動加值權限
#  define IPASS_ERROR_EXCEED_VALUELIMIT      (0x0511) ///< 超過當日累計交易金額上限 3000 元
#  define IPASS_ERROR_NOVALID_PRECARD        (0x0512) ///< 使用非原加值交易票卡進行取消加值交易
#  define IPASS_ERROR_NOVALID_TXNTYPE        (0x0513) ///< 不合法的交易類型
#  define IPASS_ERROR_NOMATCH_AACCODE        (0x0514) ///< 授權驗證碼錯誤
#  define IPASS_ERROR_NOMATCH_CARD_CSN       (0x0515) ///< 使用不同票卡進行同一交易(卡號錯誤)
#  define IPASS_ERROR_NOMATCH_CARD_TXN       (0x0516) ///< 使用不同票卡進行同一交易(交易序號錯誤)
#  define IPASS_ERROR_INVALID_AUTOADDVALUE   (0x0517) ///< 無自動加值權限 黑名單鎖卡
#  define IPASS_ERROR_NOTENOUGH_KVALUE       (0x0518) ///< 票卡餘額不足
#  define IPASS_ERROR_EV_BEV_FORMAT          (0x0519) ///< 電子票值格式錯誤
#  define IPASS_ERROR_DYN_CPD                (0x0520) ///< 動態防偽驗證碼錯誤
#  define IPASS_INVALID_AUTOADDVALUE         (0x0521) ///< 自動加值金額非 200 元倍數
#  define IPASS_ERROR_TXNVALUE_UNDERMIN      (0x0522) ///< 交易金額不可小於或等於 0
#  define IPASS_ERROR_TXNVALUE_OVERMAX       (0x0523) ///< 加值後餘額不可超過 1 萬元上限
//=== Error_Code vike.2015_06_26 ===
// #define IPASS_ERROR_RECOVERY           (0x0505) ///< 票值回復
// #define IPASS_ERROR_AUTOLOAD_OFF       (0x0524) ///< 關閉自動加值
// #define IPASS_ERROR_AUTOLOAD_ON          (0x0525) ///< 開啟自動加值
//=== Error_Code vike.2015_06_26 ===

/// @brief 讀卡機確認票卡回傳錯誤代碼  0600-06FF
#  define IPASS_ERROR_NO_CARD             (0x0600) ///< 詢卡無卡錯誤
#  define IPASS_ERROR_MULTI_CARD          (0x0601) ///< 請勿多張票卡
#  define IPASS_ERROR_BLACK_LIST          (0x0705) ///< 黑名單鎖卡
//=== Error_Code vike.2015_06_26 ===
#  define IPASS_ERROR_NO_MAPPING_REQ_TXN  (0x0707) ///< //與請求交易不符
//=== Error_Code vike.2015_06_26 ===


//=== ini_Error_Code vike.2015_09_01 ===
#  define IPASS_ERROR_WRONG_TAG                   (0x0700)
#  define IPASS_ERROR_WRONG_ATRC_ATAC             (0x0701)
#  define IPASS_ERROR_WRONG_AAC                   (0x0702)
#  define IPASS_ERROR_DB_EXCEPTION                (0x0703)
#  define IPASS_ERROR_ILLEGAL_DEVICE              (0x0704)
#  define IPASS_ERROR_BLACK_LIST                  (0x0705) ///< 黑名單鎖卡
#  define IPASS_ERROR_UNKNOWN_SERVICE             (0x0706)
#  define IPASS_ERROR_NO_MAPPING_TXN              (0x0707)
#  define IPASS_ERROR_TXN_OVERTIME                (0x0708)
#  define IPASS_ERROR_ILLEGAL_SERVICE             (0x0709)
#  define IPASS_ERROR_RSP_OVERTIME                (0x0710)
#  define IPASS_ERROR_INVALID_COMMAND_LENGTH      (0x0711)
#  define IPASS_ERROR_INVALID_MESSAGE_BODY_LENGTH (0x0712)
#  define IPASS_ERROR_UNKNOWN_COMMAND_ID          (0x0713)
#  define IPASS_ERROR_UNKNOWN_MESSAGE_CODE        (0x0714)
#  define IPASS_ERROR_WRONG_CIPHER                (0x0715)
#  define IPASS_ERROR_INVALID_AUTOADD             (0x0716)
#  define IPASS_ERROR_INVALID_BANKID              (0x0717)

/// @brief 讀卡機資料傳送接收回傳錯誤代碼  0800-08FF
#  define IPASS_ERROR_RW_WRITE_FAIL           (0x0800) ///< 寫入讀卡機錯誤
#  define IPASS_ERROR_RW_RECEIVE_OVERTIME     (0x0801) ///< 讀卡機讀取資料逾時
#  define IPASS_ERROR_RW_RESPONSE_CHECKSUM    (0x0802) ///< 讀卡機檢查碼錯誤
#  define IPASS_ERROR_RW_RESPONSE_NOVALID     (0x0803) ///< 讀卡機回傳執行錯誤
#  define IPASS_ERROR_RW_RESPONSE_SAMNOVALID  (0x0804) ///< SAM 回傳執行錯誤




//=== ini_Error_Code vike.2015_09_01 ===
// #define IPASS_ERROR_RW_RESPONSE_NOVALID      (0x0917)
// #define IPASS_ERROR_RW_RESPONSE_CHECKSUM     (0x0918)
/// @brief 其餘錯誤回傳錯誤代碼    0900-09FF
#  define IPASS_ERROR_READ_TXLOG_FLASH     (0x0900) ///< 讀取 TxLog 錯誤
#  define IPASS_ERROR_INI_POSINI           (0x0901) //ini_Error_Code vike.2015_09_01
#  define IPASS_ERROR_INI_LASTAPI          (0x0902) //ini_Error_Code vike.2015_09_01
#  define IPASS_ERROR_INI_KRTCTICKET       (0x0903) //ini_Error_Code vike.2015_09_01
#  define IPASS_ERROR_LOAD_LIBRARY         (0x0904) //ini_Error_Code vike.2015_09_01
#  define IPASS_ERROR_NONE_CHECKRW         (0x0905) //ini_Error_Code vike.2015_09_01


#  define IPASS_ERROR_DETECT_NORW          (0x0906) //ini_Error_Code vike.2015_09_01
#  define IPASS_ERROR_PROCESS_PARSER_ERROR (0x0907) //ini_Error_Code vike.2015_09_01


#  define IPASS_ERROR_TXLOG_NODATA         (0x0908) ///< 無此交易類型
#  define IPASS_READ_EXE_INI_ERROR         (0x0909) //ini_Error_Code vike.2015_09_01
#  define IPASS_READ_INPUTTXT_ERROR        (0x0910) //ini_Error_Code vike.2015_09_01

#  define IPASS_ERROR_INPUT_LEN            (0x0911) ///< Input 長度錯誤
#  define IPASS_WRITE_TRANSLOG_ERROR       (0x0912) //ini_Error_Code vike.2015_11_10

#  define IPASS_AMS_CONNECT_ERROR          (0x0913) //ini_Error_Code vike.2015_09_01



#  define IPASS_DUPLICATE_TXNS_ERROR       (0x0914) //ini_Error_Code vike.2015_09_01
#  define IPASS_NO_BLACKLIST_FOLDER_ERROR  (0x0915) //ini_Error_Code vike.2015_09_01
#  define IPASS_AMS_MESSAGE_LEN_ERROR      (0x0916) //ini_Error_Code vike.2015_09_01
#  define IPASS_ERROR_INPUT_FMT            (0x0917) ///< Input 格式錯誤
#  define IPASS_ERROR_NO_AUTOLOAD_FIRST    (0x0918) ///< 未先做自動加值

#  define IPASS_ERROR_NO_SIGNON            (0x0905) ///< 尚末端末開機 //Error_Code vike.2015_06_26
#  define IPASS_ERROR_HOST_CRYPTION_TYPE   (0x0919) ///< 加密設定錯誤 //Error_Code vike.2015_06_26
#  define IPASS_ERROR_DEVICE_ID_UNSET      (0x0920) ///< 未設定就取用 DEVICE ID
#  define IPASS_ERROR_OUT_BUF_TOO_SMALL    (0x0921) ///< Output buffer 太小

// @brief 讀卡機交易回傳錯誤代碼
//=== Error_Code vike.2015_06_26 ===
//ORG
// #define IPASS_ERROR_NO_MAPPING_REQ_TXN (0x1001) ///< 與請求交易不符
// #define IPASS_ERROR_RECOVERY           (0x1002) ///< 票值回復
// #define IPASS_ERROR_AUTOLOAD_OFF       (0x1003) ///< 關閉自動加值
// #define IPASS_ERROR_AUTOLOAD_ON          (0x1004) ///< 開啟自動加值
// #define IPASS_ERROR_NO_SIGNON          (0x1005) ///< 尚末端末開機
// #define IPASS_ERROR_HOST_CRYPTION_TYPE (0x1006) ///< 加密設定錯誤
//=== Error_Code vike.2015_06_26 ===

#  define IPASS_ERROR_UNKNOWN         (0xFFFF) ///< 不明錯誤



// === SAM卡指令執行錯誤代碼 (1 ~ 24)  ===
#  define EXECUTE_OK              (0)
#  define ERROR_RESET_SAM_MODULR  (1)
#  define ERROR_SELECT_AID        (2)
#  define ERROR_SEND_RWID_TO_AUTH (3)
#  define ERROR_GET_SAMID         (4)
#  define ERROR_GET_RANDOM        (5)
#  define ERROR_SAM_AUTHENCATION  (6)
#  define ERROR_GET_MIFAREKEY   (7)
#  define ERROR_GET_APIUSENO    (8)
#  define ERROR_GET_SAMTYPE     (9)
#  define ERROR_GET_ATRC_CODE   (10)
#  define ERROR_GET_ATAC_CODE   (11)
#  define ERROR_GET_PTRC_CODE   (12)
#  define ERROR_GET_PTAC_CODE   (13)
#  define ERROR_GET_AAC_CODE    (14)
#  define ERROR_GET_PAC_CODE    (15)
#  define ERROR_SET_APIUSENO    (16)
#  define ERROR_NO_SUCH_SLOT    (17)

// === 票卡區塊認證回傳錯誤代碼 (25 ~ 40) ===
#  define ERROR_AUTH_S0     (25)
#  define ERROR_AUTH_S1     (26)
#  define ERROR_AUTH_S2     (27)
#  define ERROR_AUTH_S3     (28)
#  define ERROR_AUTH_S4     (29)
#  define ERROR_AUTH_S5     (30)
#  define ERROR_AUTH_S6     (31)
#  define ERROR_AUTH_S7     (32)

// === 票卡資料讀取回傳錯誤代碼 (41 ~ 70) ===
#  define ERROR_READ_S0B0   (41)
#  define ERROR_READ_S1B0   (42)
#  define ERROR_READ_S1B1   (43)
#  define ERROR_READ_S1B2   (44)
#  define ERROR_READ_S2B0   (45)
#  define ERROR_READ_S2B1   (46)
#  define ERROR_READ_S2B2   (47)
#  define ERROR_READ_S3B0   (48)
#  define ERROR_READ_S4B0   (49)
#  define ERROR_READ_S4B1   (50)
#  define ERROR_READ_S4B2   (51)
#  define ERROR_READ_S5B0   (52)
#  define ERROR_READ_S5B1   (53)
#  define ERROR_READ_S5B2   (54)
#  define ERROR_READ_S6B0   (55)
#  define ERROR_READ_S7B0   (56)
#  define ERROR_REREAD_S2B0 (70)

// === 票卡資料寫入回傳錯誤代碼 (71 ~ 100) ===
#  define ERROR_WRITE_S1B0  (71)
#  define ERROR_WRITE_S1B1  (72)
#  define ERROR_WRITE_S2B0  (73)
#  define ERROR_WRITE_S2B1  (74)
#  define ERROR_WRITE_S2B2  (75)
#  define ERROR_WRITE_S3B0  (76)
#  define ERROR_WRITE_S4B0  (77)
#  define ERROR_WRITE_S4B1  (78)
#  define ERROR_WRITE_S4B2  (79)
#  define ERROR_WRITE_S5B0  (80)
#  define ERROR_WRITE_S5B1  (81)
#  define ERROR_WRITE_S5B2  (82)
#  define ERROR_WRITE_S6B0  (83)
#  define ERROR_WRITE_S6B1  (84)
#  define ERROR_WRITE_S6B2  (85)
#  define ERROR_PURCHASE_AUTOLOAD_FAIL          (86) //Copy Form ErrorCode.h
#  define ERROR_PURCHASE_AUTOLOADOK_DEDUCT_FAIL (87) //Copy Form ErrorCode.h

// === 票卡資料檢查回傳錯誤代碼 (101 ~ 120) ===
#  define ERROR_CARD_ISSUECODE       (101)
#  define ERROR_CARD_EXPIREDATE      (102)
#  define ERROR_CARD_VERSION         (103)
#  define ERROR_CARD_STATUS          (104)
#  define ERROR_CARD_VERIFY_CAC      (105)
#  define ERROR_EV_NOMATCH_BEV       (106)
#  define ERROR_EV_BEV_FORMAT        (107)
#  define ERROR_CARD_TXNNO_INDEX     (108)
#  define ERROR_PERSONAL_EXPIREDATE  (109)
#  define ERROR_NOVALID_CARDTYPE     (110)
#  define ERROR_NOVALID_TXNVALUE     (111)
#  define ERROR_NOAUTOLOAD_RIGHT     (112)
#  define ERROR_EXCEED_VALUELIMIT    (113)
#  define ERROR_NOVALID_PRECARD      (114)
#  define ERROR_NOVALID_TXNTYPE      (115)
#  define ERROR_NOMATCH_AACCODE      (116)
#  define ERROR_NOMATCH_CARD_CSN     (117)
#  define ERROR_NOMATCH_CARD_TXN     (118)
#  define ERROR_INVALID_AUTOADDVALUE (119)
#  define ERROR_NOTENOUGH_KVALUE     (120)
#  define ERROR_DYN_CPD              (121)
#  define ERROR_BLACK_LIST_FRONT     (122)
#  define ERROR_TXNVALUE_UNDERMIN    (123)
#  define ERROR_TXNVALUE_OVERMAX     (124)

// === 讀卡機確認票卡回傳錯誤代碼 (131 ~ 135) ===
#  define ERROR_NO_CARD              (131)
#  define ERROR_MULTI_CARD           (132)

// === 讀卡機交易回傳錯誤代碼 (150 ~ 180) ===
#  define ERROR_NO_MAPPING_REQ_TXN   (151)
#  define ERROR_RECOVERY             (152)
#  define ERROR_AUTOLOAD_OFF         (153)
#  define ERROR_AUTOLOAD_ON          (154)
#  define ERROR_NO_SIGNON            (155)
#  define ERROR_HOST_CRYPTION_TYPE   (156)

// === 後台系統回傳錯誤代碼 (201 ~ 220) ===
#  define ERROR_INVALID_COMMAND_LENGTH      (201)
#  define ERROR_INVALID_MESSAGE_BODY_LENGTH (202)
#  define ERROR_UNKNOWN_COMMAND_ID          (203)
#  define ERROR_UNKNOWN_MESSAGE_CODE        (204)
#  define ERROR_WRONG_CIPHER                (205)
#  define ERROR_WRONG_TAG                   (206)
#  define ERROR_WRONG_ATRC_ATAC             (207)
#  define ERROR_WRONG_AAC                   (208)
#  define ERROR_DB_EXCEPTION                (209)
#  define ERROR_ILLEGAL_DEVICE              (210)
#  define ERROR_BLACK_LIST                  (211)
#  define ERROR_UNKNOWN_SERVICE             (212)
#  define ERROR_NO_MAPPING_TXN              (213)
#  define ERROR_TXN_OVERTIME                (214)
#  define ERROR_ILLEGAL_SERVICE             (215)
#  define ERROR_RSP_OVERTIME                (216)

//=== ini_Error_Code vike.2015_09_01 ===
#  define ERROR_INVALID_AUTOADD        (218)
#  define ERROR_INVALID_BANKID         (219)
//=== ini_Error_Code vike.2015_09_01 ===

// === 讀卡機資料傳送接收回傳錯誤代碼 (220 ~ 230) ===
#  define ERROR_RW_WRITE_FAIL          (221)
#  define ERROR_RW_RECEIVE_OVERTIME    (222)
#  define ERROR_RW_RESPONSE_CHECKSUM   (223)
#  define ERROR_RW_RESPONSE_NOVALID    (224)
#  define ERROR_RW_RESPONSE_SAMNOVALID (225)

//=== ini_Error_Code vike.2015_09_01 ===

#  define NOT_AUTOLOAD_FIRST_ERROR  (233)
#  define INPUTTXT_FORMAT_ERROR     (234)
#  define AMS_MESSAGE_LEN_ERROR     (235)
#  define NO_BLACKLIST_FOLDER_ERROR (236)
#  define DUPLICATE_TXNS_ERROR      (237)
#  define AMS_CONNECT_ERROR         (238)
#  define WRITE_TRANSLOG_ERROR      (239)
//=== ini_Error_Code vike.2015_09_01 ===

// === 其餘錯誤回傳錯誤代碼 (231 ~ 255) ===
#  define ERROR_INPUT_LEN              (240) //Add_UI vike.2015_07_08

//=== ini_Error_Code vike.2015_09_01 ===
#  define READ_INPUTTXT_ERROR          (241)
#  define READ_EXE_INI_ERROR           (242)
#  define NO_TRANSACTION_TYPE_ERROR    (243)
//=== ini_Error_Code vike.2015_09_01 ===
#  define ERROR_READ_TXLOG_FLASH       (244)
#  define ERROR_INI_POSINI             (245)
#  define ERROR_INI_LASTAPI            (246)
#  define ERROR_INI_KRTCTICKET         (247)
#  define ERROR_TXLOG_NODATA           (248)
#  define ERROR_LOAD_LIBRARY           (250)
#  define ERROR_NONE_CHECKRW           (251)
#  define ERROR_DETECT_NORW            (252)
#  define ERROR_PROCESS_PARSER_ERROR   (254)
#  define ERROR_EXECPTION_ERROR        (255)

#endif // IPASSMICROPROGRAM_ERRCODE
