/* ICERAPI.cpp : Defines the entry point for the console application.*/
/*
V1005:
 1.將Advice改成當筆交易做完馬上送!!
V1007:
 1.新增自動加值功能
 2.修改TMAgentNumber可由TM帶入!!
V1008:
 1.小額消費功能的ReadPurseFlag輸入錯誤,修改此問題!!
V1009:
 1.inTCPIPSendRecv()裡面當fConfirm == FALSE時,要closesocket()!!
V10010:
 1.新增參數-P / -p,使用絕對路徑!!
V10011:
 1.新增DLL_ANTENN_CONTROL交易
 2.inFileAppend()裡面的fopen(),"a+"改為"a+b"
V10012:
 1.小額小費功能不使用DLL
V1.0.0.13A
 1.新增T0219 & T0220
V1.0.0.13B
 1.T0219改為T0221
V1.0.0.13C
 1.inTCPIPRecvData()裡面要區分不同主機,有不同的Header長度
V1.0.0.13D
 1.TM.c裡面,組STMC時,unCPUSAMID需移到unHashType之前,否則會多一個Byte 0x00
 2.STRUCT_READER_COMMAND2裡面的DLL_LOCK_CARD交易的R6TMFunction_OK1與R6TMFunction_OK2,設定錯誤
 3.inBuildLockCardData8()裡面的ucLockReason錯誤
 4.inReaderCommand2()裡面,當黑名單鎖卡成功後,須回傳0x6406
V1.0.0.14A
 1.鎖卡時,組TLKR檔錯誤,修改此問題!!STRUCT_READER_COMMAND2裡面錯誤!!
 2.新增LCD Control Flag欄位,T4108
 3.T0219改為T0221
 4.TCP Header長度修改,R6與CMAS不同
 5.組R6電文時,CPU SAM ID移到Hash Type前
 6.Lock Reason修改
V1.0.0.14B
 1.主機回04鎖卡時,組TLKR檔錯誤!!
 2.inBuildReadDongleDeductData()裡面unTxnSNBeforeTxn欄位組錯,修改此問題!!
V1.0.0.15A
 1.fIsAutoloadProcess()裡面,當srTxnData.srParameter.chAutoLoadMode == '0'時,沒有處理bAutoLoadAmt,導致T0409="00"!!
 2.新增虹堡USB功能
V1.0.0.15B
 1.st_ReaderCommand2()裡面TimeOut參數設定錯誤!!
V1.0.0.16A
 1.inECCDeductOrRefund()裡面移除vdUpdateECCDataAfterTxn(),不然ICER主機會拒絕!!
 2.新增inReponseDefaultCkeck()功能!!
 3.inTCPIPRecvData()裡面的inTCPHeaderLen參數,由設定值讀取改為程式預設!!
 4.inReaderCommand()裡面,pInfo == -18錯誤,應改成inRetVal == -18!!
V1.0.0.17A
 1.ReadBasicData時,不要組加值重送檔,改到家值或扣款成功後再組!!
 2.TXN_ECC_READ_CARD交易時,須回傳T0200!!
 3.SA_MultiMifareSelectCard2交易尚未施作,補做!!
 4.inAutoloadProcess()裡面,inReaderCommand()失敗須判斷回傳DLL_DECLINE_FAIL!!
V1.0.0.17A
 1.TXN_ECC_ANTENN_ACONTROL交易前不須SignOn!!By Golden For 桃捷
 2.TXN_ECC_MULTI_SELECT交易前SignOn若失敗,需可繼續進行TXN_ECC_MULTI_SELECT交易!!By Leo For 全聯
V1.0.0.17E
 1.修改861799(聯名卡退卡)功能,新增支援R6交易!!
 2.fnMaintainBLC()裡面的compare要換成compare3,不然會當機!!(Windows + Linux)
 3.只有Auth指令TimeOut時,要回-125,其他指令TimeOut不須回-125,以免TM誤執行Retry!!
 4.TxnReqOnline/TxnReqOffline回6304/6305時,要做SignOn
 5.Bug Fix-->srXMLFormatCheck裡面新增4108!!
 6.在pAPDUFunction裡面的Auth交易時,將ucLCDControlFlag的值,改為bGetLCDControlFlag()輸入!!
V1.0.0.18A
 1.新增Android Project!!
 2.T4109 & T4110需回覆,每個交易都要有!!
 3.扣款含Autoload,時間要不同!!
V1.0.0.18B
 1.新增配對功能!!
 2.修改vdCountEVBefore()裡面的lnEV,需允許負值!!
 3.修改inBuildCMSAuthTxnOnlineAPDU()裡面T4807處理規則!!
 4.修改CMAS_APDU裡面的inBuildCMSAuthTxnOnlineAPDU()裡的T4807處理規則!!
 5.修改inTCPIPAnalyzePackage(),新增檢查T3903=04時,也要鎖卡!!
 6.inBuildCMSAuthTxnOnlineTM()裡面的T0410,vdUIntToAsc改成vdIntToAsc,會讓交易成功後的T0415負值表示正確!!
 7.新增ICER MAC欄位資料
V1.0.0.19A
 1.新增AccuntLink交易
 2.TXN_ECC_READ_DEDUCT交易回傳的T554801長度改為sizeof(ReadCardDeduct_APDU_Out)
 3.TXN_ECC_SIGN_ON_QUERY & TXN_ECC_SIGN_ON & TXN_ECC_READ_CARD 交易且BatchFlag != R6_BATCH時,須將Reader回傳資料傳給TM
V1.0.1.2A
 1.調正架構,將.h及.c分成ICERInc + ICERSrc資料夾
 2.修改T6004只有Tag Name之問題,修改inGetBLCName()功能!!
 3.inBuildCMSResetTM()裡面新增T1300,因為有人日期帶錯,主機回覆正確日期且6308後,沒做此步驟,導致給讀卡機跟給主機的日期不同,造成97.
 4.新增296062交易!!
V1.0.1.3A
 1.for金門案,版號進號!!
V2.0.0.1A
 1.整合Windows & Linux版!!
V2.0.0.2A
 1.連天S80定版!!
V2.0.0.3A
 1.同上
 2.修改inCMASSettle(),CMAS結帳前須把所有Advice送完.
V2.0.0.4A
 1.新增296060功能,For綁定功能用!!
V2.0.0.4B
 1.新增900097 & 900098功能!!
V2.0.0.4C
 1.新增銀行共用規格!!
V2.0.0.4D
 1.修改虹堡USB,可新增Bus ID & Device ID
V2.0.0.5A
 1.inLockCard()成功時,需回傳SUCCESS.且CMAS主機回鎖卡時,又鎖卡失敗時,需回傳-119
V2.0.0.5B
 1.新增PacketLenFlag參數.
 2.CMAS新增T5588欄位.
 3.inSignOnProcess裡,PPR_Reset回0x630D時,需自動執行PPR_Match!!
V2.0.0.5C
 1.IsInlist()裡面卡號比對,不使用ECC_UINT64ToASCII()
 2.inTCPIPAnalyzePackage()裡面,strtol改成strtoull
 3.CMAS序號原本在inDoECCAmtTxn()成功後+1,改為組Advice時+1
 4.CMAS序號原本主機只要有回覆就+1,改成回19才+1
 5.扣款含自動加值交易時,兩筆交易時間需不同.---------------->移除此需求
 5.某些平台的檔名有長度限制,注意檔名別太長!!並,若超過該限制,檔案處理時,需特殊處理!!
V2.0.0.5F
 1.同步S80版號
 2.CMAS時,回傳的T1101因為已經加過1,所以需減1處理.
V2.0.0.5G
 1.新增計程車讀取(296063)指令.
 2.TXN_ECC_DEDUCT_TAXI交易時,將T4835 & T4836 & T4837關掉!!
 3.新增社福展期(R6 + CMAS) + 學生展期(R6)交易??
 4.讀六筆交易(R6)的扣款交易設備編號錯誤,修改此問題!!
 5.修改聯名卡退卡的STMC的CPU Txn Amt負值錯誤問題!!
V2.0.0.5H
 1.R6學生展期交易STMC錯誤,修改此問題!!
V2.0.0.5I
 1.For桃捷,若參數檔為舊版時,API須能支援!!
 2.fnBINTODEVASC() Bug !! For六筆扣款!!
V2.0.0.5J
 1.fnBINTODEVASC() Bug !! For六筆扣款!!上次沒改好!!
V2.0.0.5K
 1.AuthAccuntLink會回覆BankCode & Autoload Flag & Autoload Amt,API需抓取該資料進行包檔動作,用以修改Retey交易時,該3欄位無資料之問題!!
V2.0.0.5L
 1.修改Android版RES內的T4109及T4110錯誤之問題.改fnBINTODEVASC()裡面.
V2.0.0.5M
 1.新增BLCFlag參數!!
 2.扣款兼自動加值及單獨自動加值時,CMAS的T0400有誤,修改此問題!!
V2.0.0.5N
 1.log_mas()增加參數.
 2.修改FILE_TMP,不要每次讀取檔案,改成全域變數,一開始讀一次,之後有寫入在更新,最後在寫回!!
 3.扣款兼Autoload的Autoload失敗時,需有Reversal機制!!
 4.銀行併機SAM的CMAS Advice交易需補傳T6000,否則,CTAC會驗證錯誤!!
V2.0.0.6A
 1.中信共用規格Bug修改!!
V2.0.0.6B
 1.inWriteFlasfData()調整成,最後在寫入!!
 2.CMAS的TXN_ECC_AUTOLOAD_ENABLE交易不須送Advice!!
V2.0.0.6C
 1.CMAS社福展期改走inBuildCMSAuthSetValueAPDU()!!
V2.0.0.6D
 1.Android版gReaderMode=USB_READER時,Android底層新增ComOpen1 & ComOpen2兩功能!!
V2.0.0.6E
 1.vdReSendR6ReaderAVR()裡面,因EZHost說02 35的交易不用送!!
V2.0.0.6.F-->C
 1.R6的帳戶連結交易,RES需回傳T0214 + T0409!!
 2.R6的T0215不隱藏!!
V2.0.0.6G
 1.新增chAdviceFlag參數,控制CMAS交易是否當下傳送Advice.
 2.新增900097交易,用來傳送CMAS Advice.
 3.CMAS的T3700由12碼改15碼,凡是卡片類交易,一律多送3碼卡片交易前序號(10進制),非卡片類交易,送3碼0.
V2.0.0.7A
 1.CTCB共用規格問題修正!!
 2.修改vdUpdateCardInfo2(),640E時,不回傳T0409!!
V2.0.0.7B
 1.R6的SignOn交易時,若6308,下一筆的時間需加1,否則R6會回19!!
V2.0.0.8A
 1.NCCC共用規格時,不檢查NCCC的Response Code.
V2.0.0.8B
 1.IsInlist()裡面,LOG_LEVEL_ERROR改為LOG_LEVEL_FLOW
 2.vdCMASBuildRRNProcess() Bug修改!!
V2.0.0.9A
 1.inBuildTxRefundInData() Bug修改,sprintf換成memcpy!!
 2.新增ReaderPortocol=1,For中冠Reader Portocol!!
 3.vdUpdateECCDataAfterTxn()裡面TXN_ECC_READ_BASIC時,T4800錯誤,修改此問題!!
 4.fngetUnixTimeCnt()有Bug,修改此問題!!
 5.STRUCT_READER_COMMAND2裡面的PPR_TaxiRead的pCMAS_TMFun_Lock1 & pCMAS_TMFun_Lock2加上inBuildCMASLockTM1 & inBuildCMASLockTM2
V2.0.0.9.C
 1.AccuntLink交易時,若卡機回640A,則API回-139收銀機
 2.inBuildCMSTaxiDeductTM()裡面的T0409要刪除後兩碼小數
V2.0.0.9D
 1.inDoECCAmtTxn()裡面f6403Check()之後,加上vdFreeXMLDOC(&srTmpXMLData);
 2.main()裡面,將vdWriteLog((BYTE *)NULL,0,WRITE_TM_REQ_LOG,0);之後的vdFreeXMLDOC(&srXMLData);移到inInitialSystemXML()裡面
 3.計程車讀取/扣款時,inBuildCMASLockTM2()裡面的T4803 & T4804需Off掉.
V2.0.0.9E
 1.inFinalizer()裡面新增vdFreeXMLDOC(&srBankXMLData);
V2.0.0.9F
 1.新增高鐵讀取交易.
 2.新增inICERAPI_ReaderMode().讀卡機模式!!
V2.0.0.9G
 1.修改-125卻回-124之問題!!
 2.黑名單鎖卡時,需支援DLE通訊!!
 3.inTCPIPAnalyzePackage()將inOffset = 0;移除!!
V2.0.0.9H
 1.改版號為2009H
V2.0.1.0A
 1.CMAS的T4210需區分為,一代卡送Location ID,二代卡送New Location ID.
 2.連線鎖卡時,需支援DLE通訊!!
 3.修改vdBuildXMLResponse(),若shValueLen為0,不要組RES.
 4.inXMLUpdateData()的bUpdateValue為NULL時,直接離開!!
V2.0.1.0B
 1.inBLCLockCard()的DLE通訊時,接收資料後,資料處理有誤(copy到指標)!!
V2.0.1.0C
 1.CMAS所有交易,電文須加上T4104,For NCCC需求!!
V2.0.1.0D
 1.vdBuildXMLResponse()裡面組RES時,chTag一定要組,不論有無shValueLen.
V2.0.1.0E
 1.CMAS的票卡查詢交易時,黑名單鎖卡時的鎖卡Advice的T4819錯誤,修改此問題!!
V2.0.1.0F
 1.數位付交易調整!!
V2.0.1.0G
 1.自動加值失敗時,RES不可回覆T0409.
V2.0.1.0H
 1.數位付交易調整!!
 2.中冠Porotol(READER_INFOCHAMP)調整!!
 3.R6的RES的T4109有錯,修改vdUpdateCardInfo(),從9碼改為10碼!!
 4.虹堡標準USB通訊接收資料時有誤,640E卡時有Bug,修改此問題!!
V2.0.1.0I
 1.CMAS主機時,Retry交易時,若卡機回6415,需Reversal前筆CMAS交易,不然會回19!!
 2.inBuildICERTxnReqOnlineTM()多送T6000!!
V2.0.1.0J
 1.inTCPIPAdviceProcess()裡面的usOKCnt加完馬上儲存,以免Advice沒送完掛掉時,usOKCnt沒存回!!
 2.vdUpdateCardInfo2()的T0212改為1Byte!!
V2.0.1.1A
 1.inTCPIPAdviceProcess()裡面的inFileModify()改成不開檔也不關檔,不然會造成之後檔案打不開!!
V3.0.0.1B
 1.inAutoloadProcess()裡面的社福卡時,不能inDLLTrans1 = DLL_ADD_VALUE;
 2.inComRecv()裡面windows+USB時,return PORT_READTIMEOUT_ERROR ;被移除,加回!!
 3.呼叫vd640EProcess()前,先確認是否EZHost,因bReaderOutData格式不同!!
 4.新增SignOnMode參數!!
V3.0.0.1C
 1.新增EDCA Read功能!!
 2.移除845799交易!!
 3.CMAS時TXN_ECC_MULTI_SELECT交易Bug修改!!
V3.0.0.1D
 1.不需要Upload!!
 2.新增EDCA Deduct功能!!
 3.Windows版inComRecvByLen()有問題,收資料判斷有誤,導致還沒收完時,若資料有0xFF0xFF,會被迫離開!!
V3.0.0.1E
 1.新增Linux For ARM版功能!!
 2.CMAS的T5501檢查,只在扣款交易進行!!
 3.新增淡海輕軌員工卡功能!!
 4.修改OnlienFlag=2時,SignOn交易+台北卡交易問題!!
V3.0.0.1F
 1.台北卡的T0212錯誤,修改inQueryCardData()!!
V3.0.0.1G
 1.Linux版的inComOpen()時,不讓使用者調8N1,改回預設ios.c_cflag = CS8|CREAD|CLOCAL;
V3.0.0.1.H
 1.vdReversalOff()裡面的CMAS_BATCH改成HOST_CMAS!!
 2.新增T4834 + T4834 + T4845轉乘相關欄位!!
V3.0.0.1I
 1.新增T4846欄位,For加值交易的SubType
 2.CBike的SIS2交易,新增轉乘相關欄位!!
 3.SIS2交易的自動加值調整!!
 4.新增801064交易!!
V3.0.0.1J
 1.IP Size改成MAX_IP_SIZE!!
 2.EDCA的T4835送給CMAS時,轉成10進制!!
V3.0.0.1K
 1.修改inTCPIPAdviceProcess(),inFileModify()完,要ECC_FileClose(),不然Windows版結帳時,會有問題!!
V3.0.0.1L
 1.扣款交易SIS2_Batch時,要回T554805!!
 2.inBuildCBikeDeductSIS2()裡面的表身的ucRFU3裡面的上筆轉乘資訊不倒置!!
V3.0.0.1M
 1.CMAS商店版功能修改!!
 2.使用ECC_COMRxData_Lib()要注意單位是1ms或10ms!!
V3.0.0.1N
 1.新增#ifdef CTOS_USB
 2.NO_SIGN_ON_AND_RESET需處理!!
V3.0.0.2A
 1.inECCReadCardBasicCheck()裡面的黑名單檢查移到基本檢查後!!
V3.0.0.2B
 1.inAddTcpipDataRecv調整,NCC拒絕時,回BANK_DECLINE_FAIL,且不要Reversal!!
 2.EDCADeduct交易有Autoload時,在執行一次inReaderCommand()會沒跑EDCARead,修改此問題!!
V3.0.0.2C
 1.NCCC時,inECCDataLen>0 --> 依照ECC規則,inECCDataLen<=0,回-138!!
 2.T4214改用T5503取代!!
 3.OneDayQuotaFlag+OnceQuotaFlag移除.新增T4847+T4848!!
 4.EDCADeduct_APDU_Out裡面的ucTransferGuoupCode + ucTransferGuoupCode2修改,會影響inBuildCMASEDCADeductTM()裡面的T4833+T483!!
V3.0.0.2D
 1.LINUX版recv_from()有Bug!!
 2.296000時,需判斷是否為數位付卡片!!
V3.0.0.2E
 1.Linux版需支援更多BaudRate!!
 2.Linux版需支援READER_CASTLES & READER_INFOCHAMP兩種porotol!!
 3.EDCA_Read新增回覆T0222(身分證字號)欄位!!
V3.0.0.2F
 1.inBuildCMASEDCADeductTM()的T4833有Bug!!
V3.0.0.2G
 1.新增EZHost的3合1扣款交易,新增T0442+T0443+T0444欄位!!
V3.0.0.2.H
 1.FILE2_RES檔案內容Bug修改!!
 2.READER_CASTLES時,Reset指令Timeout增加3秒!!
 3.inReversalProcess()的chRevFile宣告改成MAX_PATH!!
 4.新增chReadBasicVersion參數,用以支援新/舊版PPR_ReadBasicData的Le指令!!
 5.新增chMultiDeductFlag參數,在EZHost的讀1000筆扣款交易時,若讀到3合1代收售交易時,用來判斷STMC的長度是否包含12碼代收售資料!!
 6.SignOn且授權交易時,需使用主機時間回覆POS!!
V3.0.0.2I
 1.Android版修改SSL Timeout!!
V3.0.0.2J
 1.Android版#include "stdlib.h"!!
V3.0.0.2K
 1.TaxiRead新增回覆T4844!!
 2.Retry修改成只要T0100+T0300+T1200+T1300
 3.新增悠遊卡快速查詢2交易!!
V3.0.0.2L
 1.inAutoloadProcess()裡面的bTMData宣告加大,否則會溢位!!
 2.vdBuildAutolaodData()裡面若沒有T0409,須補上,否則,自動加值成功,但扣款(連線)失敗後,會沒有T0409欄位!!
V3.0.0.2M
 1.CMAS的查六筆時,回覆明碼資料!!
 2.log_msg()裡面的LOG_LEVEL_FLOW && ICERFlowDebug != '1',改成這樣!!
 3.CMAS的查六筆回覆資料,bReadCommandOut改成&bReadCommandOut[OUT_DATA_OFFSET]!!
V3.0.0.2N
 1.交易後回6304時,bInData會被SignOn改掉,需保留!!
 2.連線Autoload且非srTxnData.srParameter.chSignOnMode == SIGN_ON_ONLY模式時,有可能沒做SignOn,要補做!!
 3.TXN_ECC_DEDUCT_TAXI + TXN_ECC_DEDUCT_CBIKE + TXN_ECC_DEDUCT_EDCA的Read時,也要檢查黑名單!!
V3.0.0.2O
 1.自動加值成功但EDCA扣款失敗時,沒有vdBuildXMLResponse(),修改此問題!!
V3.0.0.2P
 1.TxnReqOffline_APDU_In & TxnReqOnline_APDU_In新增RetryFlag,用來告知卡機是否啟動Retry交易,若是,卡機將啟動Retry的AB卡檢核!!
 2.新增Ts_SystemParamentInialize2,讓設備初始化時帶入SLOT!!
V3.0.0.2Q
 1.f6403Check()裡面lnEV < srTxnData.srIngData.lnECCAmt且srTxnData.srIngData.lnECCAmt > 0才是於額不足!!
 2.台北卡的TXN_ADJECT_CARD新增vdBuildICERADataAuth()!!
V3.0.0.2R
 1.台北卡MAC調整完畢!!
 2.vdSetCMASTagDefault()裡面新增vdSetConfigFlag(srXML,2,(char *)TAG_NAME_4843);
V3.0.0.3A
 1.ICER點數折抵+折抵取消開發完成!!
 2.TXN_ECC_SIGN_ON_QUERY & TXN_ECC_SIGN_ON & TXN_ECC_MATCH的R6交易時,也要回T554801!!
V3.0.0.3B
 1.chCMASMode==2時,SignOn要帶T4211!!
 2.CMAS交易時,有些狀況沒回T1101,改到vdBuildRespData()裡面,確保都要回!!
V3.0.0.3C
 1.EzrUSBRead前後的log_msg改為LOG_LEVEL_FLOW!!
V3.0.0.3D
 1.USB的inComRecv()時,dwRead>0才表示有收到資料!!
 2.不論REQ有無T1101,都不管,直接去tmp裡面抓!!
 3.vdModifySAmtTag()裡面移除掉的組T1101資料,須加回來!!
V3.0.0.3E
 1.Windows & USB時,inComRecv()裡面針對0xFF 0xFF處理移除安源規則!!
 2.srTxnData.srREQData.ulT1101 = srTxnData.srIngData.ulCMASHostSerialNumber = atol(T1101);移到inCheckProcess裡面執行,以防RES回的T1101為0之問題!!
 3.inBuildCMSTxnReqOnlineTM()裡面的T6400預設要送,以防社福現金加值順便展期時,L1卡片需用Mifare展期之HTAC使用!!
V3.0.0.3F
 1.inCheckBLC()裡面的bBLCFileName大小改成200!!
 2.數位付SIS2 Blob開發完成!!
 3.修改Android & Linux版小Bug!!
V3.0.0.3G
 1.vdModifySAmtTag()裡面的srTxnData.srIngData.ulCMASHostSerialNumber不要--
V3.0.0.3H
 1.新增BasicCheckQuotaFlag參數!!
V3.0.0.3I
 1.新增DLL_REFUND_CARD_AUTH For連線退卡組SIS2 Blob用!!
 2.學生展期調整!!
V3.0.0.3J
 1.查詢類交易RES的T0409計算時,將long改成int,否則,EV為負值時,有可能計算錯誤!!
V4.0.0.1A
 1.修改台北卡指定加值retry
 2.AVM功能修改
V4.0.0.1B
 1.定義Reader_Type，區分讀卡機為內建或外接(Fro ANDROID_API)
V4.0.0.1C
 1.新增ReaderMode timeout時間
 2.修改ICERLib.app 中 chAllFileName的長度為500
V4.0.0.1D
 1.連線交易在retry時T1200改由ICERAPI.REQ帶入值而非原ICERAPI.REQ.bak帶入
 2.新增參數CRPDelay(毫秒)，當切換到ReaderMode時Dealy的時間，再送APDU
 3.AVM SIS2退卡名稱修正
V4.0.0.1E
 1.整合，訂版
V4.0.0.1F
 1. 修改ICERAPI.tmp，新增ucAutoload_fail_flag，在Autoload Auth時若回覆重試則改為TRUE，非重試交易和交易正常完成則改為FALSE
 2. ECC_FileCopy 修改Android和Linux一律改使用相對路徑
V4.0.0.1G
 1. 整合
V4.0.0.1H
 1.修改ECHOST包STMC檔案時SAM Ver >= 80 要包Device ID
 2.修改lnAmt3ByteToLong，如果Long為8bytes負值卡會判斷錯誤
 3. 整合
V4.0.0.2A
 1.修改Windows & Linux SSL 經常會Hung的問題
V4.0.0.2B
1. 因ICER平台可能由VPN或Internet連接，因此新增ICERConnectMode參數
2. 修改部份LOG_LEVEL_ERROR為LOG_LEVEL_FLOW，以減少一般LOG的複雜度
3.修改ReaderMode function RS232收的資料要為0x00,0x00開頭
4.修改ReaderMode function 設定Timeout卻無法跳出問題
5.新增inXMLCheckTagDuplicated function ，在做交易時判斷ICERAPI.REQ中T0300是否重複
6.整合
V4.0.0.2C
1.修改inSSLSendData()支援用domain name連線(windows & linux)
V4.0.0.2D
1.修改vdBuildRespData()，當chOnlineFlag == CMAS_ONLINE時將後台T1101存到ICERAPI.tmp
2.修改inFinalizer()，當chOnlineFlag == CMAS_ONLINE時執行vdModifySAmtTag()
3.修改ssl_ecc_connect() & ssl_ecc_read() timeout 由srTxnData.srParameter.gTCPIPTimeOut參數取代
V4.0.0.2E
1.修改商店卡模式交易和結帳由req帶入的T5503為特店代號(TM Location ID)
2.修改ssl_ecc_connect() & ssl_ecc_read() 只有 WINDOWS_API & WINDOWS_CE_API & LINUX_API開放
3.stdAfx.h新增NE_INGENICO定義
4.TM.cpp修改R6連線交易包檔使用配對SAM時，以DeviceID取代讀卡機序號
5.修改退卡交易SIS2包檔長度
6.修正EDCA Read票卡到期日長度由14Bytes改為8Bytes
V4.0.0.2F
1.修改自動加值交易ICERAPI.RES沒有帶T0409(自動加值金額)
2.新增欄位T4214 sub location id(以前的T4214沒跟系一通知，自行亂加的)
V4.0.0.2G
V4.0.0.2I
1.整合
V4.0.0.2J
1.新增T4853 淡海輕軌常客回饋金欄位！
V4.0.0.2K
1.新增801066 點數兌換加值金(HAMI POINT)
V4.0.0.2L
1.修改inCOMOpen 在android 系統中將 srTxnData.srIngData.fAPIOpenFlag = TRUE;
2.EDCA讀取時T0400金額後少2位0
V4.0.0.2M
1.整合
V4.0.0.2N
1.修改inReversalProcess在銀行併機時，放大bBuffer=3072
2.新增st_ISO8583Flag table
V4.0.0.2O
1.修改vdUpdateCardInfo2()以交易別判斷T0415
V4.0.0.3A
1.inTCPIPAnalyzePackage() 在inTCPIPTxnType = TXN_ADVICE時不清除ICERAPI_CMAS.rev
2.inSignOnProcess 將bBuf放置前方避免advice產生0x00
3.inSignOnProcess 將vdXMLBackUpData(&srXMLData,&srTmpXMLData,FALSE); 放到do while 內，避免advice沒有TRANSXml
V4.0.0.3B
1.修改BankData.c inAddTcpipDataRecv，解決AfterData資料位移2 byte問題
2.修改inProcessTxn 若未定義的ProcessCode回覆-106
3.新增TxnOnline展期功能
V4.0.0.3C
1.整合source code
2.修改購貨餘額不足時發生autoload但扣款失敗下一筆會送autoload reversal的問題
3.自動加值成功後，把Online Flag改成Off
V4.0.0.3D
1.修改vdUpdateECCDataAfterTxn()R6交易T0300都用STMC pCode替代
2. Android 加入ECC_RunIdle
3.SignOn後把TMLocationID存入ICERAPI.tmp中，交易以File中為主
4.新增T1401，視SetValidDate設定帶值，帶1後台才會回覆新展期日期
5.NCCC 資料加密功能
V4.0.0.3E
1.新增判斷讀卡機版本為8.10以上且SetCardValidDate值為1才帶T1401回CMAS
V4.0.0.3F
1.針對NCCC 加密多NCCC_ENCRYPTION define
V4.0.0.3G
1.新增菸酒功能
2.Android JNI_Init修正
V4.0.0.3H
1.修改NCCC API ECC_NCCC_MAC_Lib 第一個參數規格
V4.0.0.3I
1.EccRunIdle只有INTERNAL_READER需要呼叫
V4.0.0.3J
1.修改inTCPIPRecvData 在LINUX_API模式下可以支援FISC的Protocol
V4.0.0.3K
1.修改onlineFlag=R6_ONLINE時ICERAPI.RES的pcode以EZHOST的定義為主
2.修改onlineFlag=R6_ONLINE支援社福卡自動加值電文
3.新增onlineFlag=R6_ONLINE，現金加值20年展期功能
4.修改onlineFlag=R6_ONLINE，將展期加值改為後台加值
5.新增onlineFlag=R6_ONLINE，後台加值查詢
6.新增onlineFlag=R6_ONLINE，Pcode=390063獨立20年展期
7.新增CheckDirectoryPermission()
8.新增錯誤代碼定義:#define     DIRECTORY_PERMISSION_DENIED         -141//設備目錄權限不足
9.自動加值失敗後，存txn_offline資料到tmp檔，給retry時使用
V4.0.0.3L
1.新增Taxi_Deduct T4833 T4834欄位
2.SIS2.cpp Line438 Bug Fix
V4.0.0.3M
1.新增EDCA交易支援商店卡模式
V4.0.0.3N
1.CMAS未SignOn查詢千筆時,SignOn時不帶T5594
2.修改悠遊卡計程車讀取、悠遊卡卡片資料查詢交易不回T0415
3.修改CMAS 結帳交易上傳advice後台回覆失敗時RES不會帶T3900
4.啟用Android CMAS千筆查詢功能
5.修正Android tool跳出後在進來，直接千筆查詢會失敗問題
V4.0.0.3O
1.加入國外tag定義
2.整合
V4.0.0.5A
1.加大部份陣列大小，以避免overflow問題
2.跳過4.0.0.4編號
V4.0.0.5B
1.WINDOWS_API 將stdio.h加入取代iostream.h
2.加入購貨寫入菸酒和讀取千筆(含菸酒)
3.修改三合一扣款不會檢查黑名單
V4.0.0.5C
1.修改inBuildAuthAccuntLinkData中DongleOut->SingleAutoLoadTransactionAmount、DongleOut->AutoloadFlag位置定義錯誤
2.修改LINUX_API定義中UnixToDateTime()ulUnixTime,ulTolSecond的型態為uint32
V4.0.0.5D
1.新加入PL2303GC Driver for Android Project，廠商可以在ICERINI.xml裡新增UsbToRs232Chip參數去決定使用哪個driver。 0=PL2303HXD / 1=PL2303GC
V4.0.0.5E
1.修改代收售連線電文Pcode帶錯誤造成ICERAPI回覆-121
2.修改296000(悠遊卡卡片資料查詢)在onlineFlag=R6_ONLINE時,T4800(PVN)在讀L2時會帶成01
3.在onlineFlag=R6_ONLINE時做學生展期時，學生身分到期日依票卡內容帶入，其他票卡身分到期日依票卡到期日帶入
4.修改TM.cpp中fngetDosDateCnt API-> memset(DosDate,0,sizeof(DosDate)); => memset(DosDate,0, 2);
5.刪除20年展期在onlineFlag=R6_ONLINE的pcode811499定義
6.整合
V4.0.0.6A
1.修改autoloadMode=1時，不判斷MaxALAmt
V4.0.0.6B
1.onlineFlag=R6_ONLINE新增inBuildTxnReqOnlineAutoload在自動加值電文Field53帶入交易金額
2.ECC_FileRename 在windwos rename失敗後印出debug
3.將vdUpdateECCDataAfterTxn()移回原本位置
V4.0.0.6C
1.Log天紀錄天數改由LogCnt決定
2.修改純自動加值交易T0403金額應為0

待修改
 1.296000交易的REQ的T4108沒作用,修改此問題!!
 2.216000交易時,若要SignOn,T5593不能送!!

*/

#ifdef EZ_AIRLINKEDC
#include "_stdAfx.h"
#else
#include "stdAfx.h"
#endif
#include "Global.h"

#ifdef LIB_DEFINE

	#ifdef LIB_DEFINE2

		extern UART_RECIEVE_INFO g_sUart0RecieveInfo2;
		extern UART_SEND_INFO    g_sUart0AnswerInfo2;
		extern unsigned char chAnswerLen;
		extern volatile unsigned int iUart0RecieveOK;
		extern unsigned char chSerialPortMode;
		extern unsigned char chProtectFlag;

	#endif

	extern UART_RECIEVE_INFO g_sUart0RecieveInfo;
	extern UART_SEND_INFO    g_sUart0AnswerInfo;
	extern char g_cProtectFlag;

#endif

#if READER_MANUFACTURERS==ANDROID_API
	#define DEFAULT_LOG_FILENO (STDOUT_FILENO)
	////#include<icerapi_icerapi_Api.h>
	//#include<icerapi_icerapi_ICERAPI.h>
	//#include <android/log.h>
	int fd_ = DEFAULT_LOG_FILENO;
    int fd2_ = DEFAULT_LOG_FILENO;
    int log_filter(const struct dirent* entry);

#elif READER_MANUFACTURERS==LINUX_API
	#define DEFAULT_LOG_FILENO (STDOUT_FILENO)

	int fd_ = DEFAULT_LOG_FILENO;
	int fd2_ = DEFAULT_LOG_FILENO;

#endif

STRUCT_TXN_DATA srTxnData;
STRUCT_XML_ACTION_DATA srXMLActionData;
Store_Data stStore_DataInfo_t;
STRUCT_TMP_DATA gTmpData;

extern BYTE gCurrentFolder[MAX_PATH];

//STRUCT_XML_DOC srXMLResData;
int inMainRet = ICER_ERROR;
int inTransType;

/*
*/

#if READER_MANUFACTURERS==ANDROID_API

#define TAG "ICER LOG:" // 餈葵?航摰??OG??霂?
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 摰?LOGD蝐餃?
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 摰?LOGI蝐餃?
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 摰?LOGW蝐餃?
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 摰?LOGE蝐餃?
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 摰?LOGF蝐餃?

JNIEnv *Jenv;
jobject Jni_usbManager,Jni_context;
BYTE ANDROID_FILE_PATH[100];
const char *JNI_filePath;
const char *JNI_LogPath;
jobject ReaderObj;
jmethodID AndroidReaderInit;
  #ifdef LIB_DEFINE
	#ifdef LIB_DEFINE2
		jmethodID AndroidUart0infoProcess2;
		jmethodID AndroidEccRunIdle;
	#endif
  #endif

JNIEXPORT jint JNICALL Java_icerapi_icerapi_ICERAPI_icerapi_1exe(JNIEnv *env, jobject jobject1, jstring filePath, jstring LogPath,jobject usbManager,jobject context)

#elif READER_MANUFACTURERS==WINDOWS_CE_API
int _tmain(int argc, char* argv[])
#elif READER_MANUFACTURERS!=WINDOWS_API && (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86)
int ICERApi_exe(void)
#else
int main(int argc, char* argv[])
#endif
{
long lnProcessCode;
#if READER_MANUFACTURERS==WINDOWS_CE_API
WCHAR usTest[MAX_PATH];
#else
char usTest[MAX_PATH];
#endif
#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
int i;
#endif
int inRetVal;

	memset((char *)&srTxnData,0x00,sizeof(srTxnData));

	inReadFlasfData();

#if READER_MANUFACTURERS==ANDROID_API
	Jenv = env;
	memcpy(&Jni_usbManager,&usbManager,sizeof(jobject));
	memcpy(&Jni_context,&context,sizeof(jobject));

	memset(ANDROID_FILE_PATH,0x00,sizeof(ANDROID_FILE_PATH));
	JNI_filePath = (*Jenv)->GetStringUTFChars(Jenv,filePath,0);
	memcpy(ANDROID_FILE_PATH,JNI_filePath,strlen(JNI_filePath));
	log_msg(LOG_LEVEL_FLOW,"FilePath: %s , length: %d, ANDROID_FILE_PATH: %s",JNI_filePath,strlen(JNI_filePath),ANDROID_FILE_PATH);

	JNI_LogPath = (*Jenv)->GetStringUTFChars(Jenv,LogPath,0);
	log_msg(LOG_LEVEL_FLOW,"FilePath: %s , length: %d",JNI_LogPath,strlen(JNI_LogPath));

#ifdef LIB_DEFINE
  #if READER_TYPE==INTERNAL_READER
	jclass clazz3 = (*Jenv) -> FindClass(Jenv,"icerapi/icerapi/ReaderAPI_Interface");
	jmethodID AndroidReaderInit = (*Jenv) -> GetMethodID(Jenv,clazz3,"<init>","()V");
	AndroidUart0infoProcess2 = (*Jenv) -> GetMethodID(Jenv,clazz3,"Ts_Uart0infoProcess2","(Ljava/lang/String;Ljava/lang/String;Landroid/hardware/usb/UsbManager;Landroid/content/Context;[B[B[B[I[B[B)V");
	AndroidEccRunIdle = (*Jenv) -> GetMethodID(Jenv,clazz3,"Ts_RunIdle","(Ljava/lang/String;Ljava/lang/String;Landroid/content/Context;)V");
	ReaderObj = (*Jenv) -> NewObject(Jenv,clazz3,AndroidReaderInit);
  #endif
#endif

#endif

#if READER_MANUFACTURERS==WINDOWS_API || (READER_MANUFACTURERS==LINUX_API && RDEVN==NE_X86) || READER_MANUFACTURERS==ANDROID_API || READER_MANUFACTURERS==WINDOWS_CE_API
	atexit(inFinalizer);
#endif

	memset(usTest,0x00,sizeof(usTest));
	memset(gCurrentFolder,0x00,sizeof(gCurrentFolder));
#if READER_MANUFACTURERS==WINDOWS_API
	GetCurrentDirectory(sizeof(gCurrentFolder),(char *)gCurrentFolder);//呼叫ICERAPI.exe之應用程式路徑
#elif READER_MANUFACTURERS==WINDOWS_CE_API
	GetModuleFileName(NULL, (WCHAR *)gCurrentFolder, sizeof(gCurrentFolder));
#elif READER_MANUFACTURERS==LINUX_API//this is mean define LINUX_API
	strcat((char *)gCurrentFolder,getcwd(0,0));
#elif READER_MANUFACTURERS==ANDROID_API
	strcat(gCurrentFolder,ANDROID_FILE_PATH);
#endif

	//log_msg(LOG_LEVEL_ERROR,"main Fail 0-1:%s",gCurrentFolder);
	/*memset(gCurrentFolder,0x00,sizeof(gCurrentFolder));
	strcat(gCurrentFolder,get_current_dir_name());
	log_msg(LOG_LEVEL_ERROR,"main Fail 0-2:%s",gCurrentFolder);*/

#if READER_MANUFACTURERS==WINDOWS_API || READER_MANUFACTURERS==WINDOWS_CE_API
	if(argc > 1)
	{
		for(i=1;i<argc;i++)
		{
		#if READER_MANUFACTURERS==WINDOWS_CE_API
			if(wcslen((WCHAR *)argv[i]) == 2 && (!memcmp((WCHAR *)argv[i], L"-P",2) || !memcmp((WCHAR *)argv[i], L"-p",2)))
		#else
			if(strlen(argv[i]) == 2 && (!memcmp(argv[i],"-P",2) || !memcmp(argv[i],"-p",2)))
		#endif
			{
				memset(gCurrentFolder,0x00,sizeof(gCurrentFolder));
				#if READER_MANUFACTURERS==WINDOWS_CE_API
					memcpy(usTest,(WCHAR *)argv[i + 1],wcslen((WCHAR *)argv[i + 1]) * sizeof(WCHAR));//由Win CE帶入絕對路徑
					WideCharToMultiByte(950, 0, usTest, -1, (char *)gCurrentFolder, 260, NULL, NULL);
					//memcpy(gCurrentFolder,usTest,strlen((char *)usTest));
				#else
					GetModuleFileName(NULL,usTest,MAX_PATH);//ICERAPI.exe的絕對路徑
					memcpy(gCurrentFolder,usTest,strlen((char *)usTest) - 12);
				#endif
			}
		}
	}
#endif
	/*inMainRet = inCheckProcess();//要有參數才能檢查!!
	if(inMainRet < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"main Fail 0:%d",inMainRet);
		//return inMainRet;
	}*/

	inRetVal = inInitialTxn();

	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"main Fail 1:%d",inRetVal);
		inMainRet = inRetVal;
#if READER_MANUFACTURERS!=WINDOWS_API && (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86) && READER_MANUFACTURERS!=WINDOWS_CE_API
		inFinalizer();
#endif
		return inRetVal;
	}

	/*{
		inTCPIPServerListen(1234);
		return 0;
	}*/
	/*{
		int g_keyBitLen = 128;
		int g_TextByteLen = 128;
		unsigned char test_key[33] = {"TestEASYCARDTestEASYCARD99900000"};
		unsigned char test_IV[17] = {"0000000000000000"};
		unsigned char test_text[120] = {"2497868461801063100001000062990061990010193910193920171229171229101939171229GuCRYQ00255080130999000000009900011"};

		unsigned char temp_src[1024];

		memset(temp_src,0x00,sizeof(temp_src));
		PKCS5Padding(test_text,strlen((char *)test_text),16);
		//memcpy(test_text,"1299123456@easycard.com.tw20016312163712163720171220171220121637255080130999000000000010001",91);
		ICER_AES_CBC_Func(0, test_key, 2, test_IV, test_text, strlen((char *)test_text), temp_src);
		log_msg_debug(LOG_LEVEL_ERROR,TRUE,"ICER_AES_CBC_Func result:",25,temp_src,strlen((char *)temp_src));
	}*/
	/*{
		BYTE bBuf[10],bTmp[10];

		memset(bBuf,0x00,sizeof(bBuf));
		memset(bTmp,0x00,sizeof(bTmp));

		memcpy(bBuf,"\x79\xD1\xFE\x00",4);
		fnBINTODEVASC(bBuf,bTmp,9,MIFARE);
	}*/

	vdWriteLog((BYTE *)NULL,0,WRITE_TM_REQ_LOG,0);
	//vdFreeXMLDOC(&srXMLData);

	log_msg(LOG_LEVEL_FLOW,"gCurrentFolder REQ:%s",gCurrentFolder);
	inRetVal = inParseXML((char *)FILE_REQ,(BYTE *)NULL,TRUE,0,&srXMLData,0);
	if(inRetVal < SUCCESS)
	{
		inRetVal = ICER_REQ_FORMAT_FAIL;
		log_msg(LOG_LEVEL_ERROR,"main Fail 2:%d",inRetVal);
		inMainRet = inRetVal;
#if READER_MANUFACTURERS!=WINDOWS_API && (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86) && READER_MANUFACTURERS!=WINDOWS_CE_API
		inFinalizer();
#endif
		return inRetVal;
	}

	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
	{	//Analyse ICERAPI2.REQ
		inRetVal = inParseXML((char *)FILE2_REQ,(BYTE *)NULL,TRUE,0,&srBankXMLData,0);
		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"main Fail 2-1:%d",inRetVal);
			return ICER_ERROR;
		}
	}

	inRetVal = inGetREQData();
	if(inRetVal < SUCCESS)
	{
		inRetVal = ICER_REQ_DATA_FAIL;
		log_msg(LOG_LEVEL_ERROR,"inQueryPoint Fail 3:%d",inRetVal);
		inMainRet = inRetVal;
#if READER_MANUFACTURERS!=WINDOWS_API && (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86) && READER_MANUFACTURERS!=WINDOWS_CE_API
		inFinalizer();
#endif
		return inRetVal;
	}

	inRetVal = inDefaultDataCheck(atol(srTxnData.srIngData.chTMProcessCode));
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"inQueryPoint Fail 4:%d",inRetVal);
		inMainRet = inRetVal;
#if READER_MANUFACTURERS!=WINDOWS_API && (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86) && READER_MANUFACTURERS!=WINDOWS_CE_API
		inFinalizer();
#endif
		return inRetVal;
	}

	lnProcessCode = atol(srTxnData.srIngData.chTMProcessCode);
	//log_msg(LOG_LEVEL_ERROR,"lnProcessCode = %d",lnProcessCode);
	inRetVal = inProcessTxn(lnProcessCode);
	if(inRetVal < SUCCESS)
	{
		log_msg(LOG_LEVEL_ERROR,"main Fail 5:%d",inRetVal);
		inMainRet = inRetVal;
#if READER_MANUFACTURERS!=WINDOWS_API && (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86) && READER_MANUFACTURERS!=WINDOWS_CE_API
		inFinalizer();
#endif
		return inRetVal;
	}

	inMainRet = inRetVal;
#if READER_MANUFACTURERS!=WINDOWS_API && (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86) && READER_MANUFACTURERS!=WINDOWS_CE_API
	inFinalizer();
#endif

	return inMainRet;
}

void inFinalizer()
{
	//log_msg(LOG_LEVEL_ERROR,"inFinalizer 1(%c)",srTxnData.srParameter.chCloseAntenna);
	//log_msg(LOG_LEVEL_ERROR,"inFinalizer 3(%c)",srTxnData.srParameter.chCloseAntenna);

	if ((srTxnData.srIngData.inTransType >= TXN_ECC_DEDUCT && srTxnData.srIngData.inTransType <= TXN_ECC_ACCUNT_LINK)	||
		srTxnData.srIngData.inTransType == TXN_ECC_READ_BASIC || srTxnData.srIngData.inTransType == TXN_ECC_READ_TAXI	||
		srTxnData.srIngData.inTransType == TXN_ECC_READ_THSRC || srTxnData.srIngData.inTransType == TXN_ECC_READ_CBIKE	||
		srTxnData.srIngData.inTransType == TXN_ECC_READ_EDCA)
	{
		if(srTxnData.srParameter.chBatchFlag == SIS2_BATCH)
			inSIS2SaveBlob();
	}
	else if(srTxnData.srIngData.inTransType >= TXN_E_ADD && srTxnData.srIngData.inTransType < TXN_E_QR_VOID_ADD)//數位悠遊卡交易
	{
		if(srTxnData.srParameter.chETxnBatchFlag == SIS2_BATCH)
			inSIS2SaveBlob();
	}

	if(inMainRet != SUCCESS && srTxnData.srIngData.fCardActionOKFlag && fIsDeductTxn(srTxnData.srIngData.inTransType))//扣款伴隨自動加值交易,自動加值成功,扣款失敗!!
	//if(inMainRet != SUCCESS && srTxnData.srIngData.fCardActionOKFlag && srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT)//扣款伴隨自動加值交易,自動加值成功,扣款失敗!!
		vdBuildAutolaodData(&srXMLData);

	//if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH)
	if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)
		vdModifySAmtTag(&srXMLData,inMainRet);

	vdBuildDeviceIDResponse(&srXMLData);

	vdBuildRespData(inMainRet,&srXMLData);

	if(srTxnData.srParameter.chCloseAntenna == '1')
	{
		log_msg(LOG_LEVEL_ERROR,"inFinalizer 2(%c)",srTxnData.srParameter.chCloseAntenna);
		vdCloseAntenn();
	}

	if(srTxnData.srParameter.chReaderPortocol == READER_SYMLINK)
		inCustomizrdReaderPorotocol(TRUE);

	if(srTxnData.srIngData.fAPIOpenFlag == TRUE)
		inReaderCommand(DLL_CLOSECOM,&srXMLData,(BYTE *)NULL,(BYTE *)NULL,FALSE,0,FALSE);

	if(srTxnData.srIngData.fAPILoadFlag == TRUE)
		vdFreeLibrary();

	vdFreeXMLDOC(&srXMLData);
	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
		vdFreeXMLDOC(&srBankXMLData);

	if(inMainRet != API_IS_PROCESSING)
		vdProcessingFlag(FALSE);

#if READER_MANUFACTURERS==LINUX_API
	close_log();
#endif

	if(srTxnData.srParameter.chTCPIP_SSL == SSL_PROTOCOL)
		inSSLFinalizer();

#ifdef LIB_DEFINE
	#if READER_MANUFACTURERS==ANDROID_API
		#if READER_TYPE==INTERNAL_READER
		jstring str1 = (*Jenv)->NewStringUTF(Jenv,JNI_filePath);
		jstring str2 = (*Jenv)->NewStringUTF(Jenv,JNI_LogPath);
		(*Jenv) -> CallVoidMethod(Jenv,ReaderObj, AndroidEccRunIdle, str1, str2, Jni_context);
		#endif

	#else
		ECC_RunIdle();
	#endif
#endif

	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
		inFileAppend((char *)FILE2_RES,(BYTE *)"</TransXML>",strlen("</TransXML>"));
		//inFileAppend((char *)FILE2_RES,(BYTE *)TAG_TRANS_XML_END,strlen(TAG_TRANS_XML_END));

	srTxnData.srIngData.fOnlineHostSuccFlag = FALSE;
	inReadFlasfData();//一定要在最後!!
}

void vdBuildRespData(int inRet,STRUCT_XML_DOC *bInData)
{
BYTE bXMLFileData[MAX_XML_FILE_SIZE];

 	memset((char *)bXMLFileData,0x00,sizeof(bXMLFileData));
	if(inRet >= SUCCESS || fDLLResponseCheck(inRet) || fDLLResponseLock(srTxnData.srIngData.inReaderSW) || inRet == CALL_DLL_TIMEOUT_ERROR)
		vdBuildXMLHeaderMsg(bXMLFileData,bInData);

	if(srTxnData.srParameter.chXMLHeaderFlag == '1')
	{
		strcat((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<");
		memcpy(&bXMLFileData[strlen((char *)bXMLFileData)],TAG_TRANS_XML_HEADER,strlen(TAG_TRANS_XML_HEADER));
		strcat((char *)&bXMLFileData[strlen((char *)bXMLFileData)],">");
	}
	strcat((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<");
	memcpy(&bXMLFileData[strlen((char *)bXMLFileData)],TAG_TRANS_HEADER,strlen(TAG_TRANS_HEADER));
	strcat((char *)&bXMLFileData[strlen((char *)bXMLFileData)],">");
	memcpy(&bXMLFileData[strlen((char *)bXMLFileData)],"\n",1);

	if(srTxnData.srParameter.inAdditionalTcpipData != NO_ADD_DATA)
	{
		BYTE bBuf[10];

		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%ld",srTxnData.srIngData.ulBankSTAN);
		inXMLUpdateData(bInData,(char *)TAG_NAME_4200,(char *)TAG_NAME_1102,bBuf,TRUE);
		memset(bBuf,0x00,sizeof(bBuf));
		sprintf((char *)bBuf,"%ld",srTxnData.srIngData.ulBankInvoiceNumber);
		inXMLUpdateData(bInData,(char *)TAG_NAME_4200,(char *)TAG_NAME_1103,bBuf,TRUE);
	}

	sprintf((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<T3901>%d</T3901>\n",inRet);

	//sprintf((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<Response_Code>%d</Response_Code>\n",inRet);
	if(srTxnData.srIngData.inReaderSW > SUCCESS &&
	  (srTxnData.srIngData.inReaderSW != CARD_SUCCESS &&
	   srTxnData.srIngData.inReaderSW != 0x6415))
		sprintf((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<T3904>%04X</T3904>\n",srTxnData.srIngData.inReaderSW);
		//sprintf((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<Reader_SW>%04X</Reader_SW>\n",srTxnData.srIngData.inReaderSW);
	if(inRet == R6_DECLINE_FAIL)
		sprintf((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<T3902>%s</T3902>\n",srTxnData.srIngData.chR6ResponseCode);
	if(inRet == BANK_DECLINE_FAIL)
		sprintf((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<T3908>%s</T3908>\n",srTxnData.srIngData.chICERResponseCode);
	
	if(inRet >= SUCCESS || fDLLResponseCheck(inRet) || fDLLResponseLock(srTxnData.srIngData.inReaderSW) || inRet == CALL_DLL_TIMEOUT_ERROR)
		vdBuildXMLResponse(inTransType,bXMLFileData,strlen((char *)bXMLFileData),bInData,TRUE);
	else if(srTxnData.srIngData.fCardActionOKFlag && fIsDeductTxn(srTxnData.srIngData.inTransType))//扣款伴隨自動加值交易,自動加值成功,扣款失敗!!		
	//else if(srTxnData.srIngData.fCardActionOKFlag && srTxnData.srIngData.inTransType == TXN_ECC_DEDUCT)//扣款伴隨自動加值交易,自動加值成功,扣款失敗!!
		vdBuildXMLResponse(inTransType,bXMLFileData,strlen((char *)bXMLFileData),bInData,(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE)?TRUE:FALSE);
	else
	{
		//if(srTxnData.srParameter.chBatchFlag == CMAS_BATCH && srTxnData.srIngData.fTmpFileReaded == TRUE)
		if(srTxnData.srParameter.chOnlineFlag == CMAS_ONLINE && srTxnData.srIngData.fTmpFileReaded == TRUE)
		{
			//log_msg(LOG_LEVEL_ERROR,"1107 ulCMASHostSerialNumber:%d, ulT1101: %d", srTxnData.srIngData.ulCMASHostSerialNumber, srTxnData.srREQData.ulT1101);
			if(srTxnData.srIngData.ulCMASHostSerialNumber > srTxnData.srREQData.ulT1101)
				srTxnData.srIngData.ulCMASHostSerialNumber--;
			sprintf((char *)gTmpData.ucCMAS_REQ_T1101,"%06ld",srTxnData.srIngData.ulCMASHostSerialNumber);
			gTmpData.ucCMAS_REQ_T1101[6]=0x00;
			sprintf((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<T1101>%s</T1101>\n",gTmpData.ucCMAS_REQ_T1101);
		}
		sprintf((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<T1102>%06ld</T1102>\n",srTxnData.srIngData.ulBankSTAN);
		sprintf((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<T1103>%06ld</T1103>\n",srTxnData.srIngData.ulBankInvoiceNumber);
	}

	strcat((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<");
	memcpy(&bXMLFileData[strlen((char *)bXMLFileData)],TAG_TRANS_END,strlen(TAG_TRANS_END));
	strcat((char *)&bXMLFileData[strlen((char *)bXMLFileData)],">");
	if(srTxnData.srParameter.chXMLHeaderFlag == '1')
	{
		strcat((char *)&bXMLFileData[strlen((char *)bXMLFileData)],"<");
		memcpy(&bXMLFileData[strlen((char *)bXMLFileData)],TAG_TRANS_XML_END,strlen(TAG_TRANS_XML_END));
		strcat((char *)&bXMLFileData[strlen((char *)bXMLFileData)],">");
	}
	memcpy(&bXMLFileData[strlen((char *)bXMLFileData)],"\n",1);

	log_msg(LOG_LEVEL_ERROR,"gCurrentFolder RES:%s",gCurrentFolder);
	vdWriteLog(bXMLFileData,strlen((char *)bXMLFileData),WRITE_TM_RES_LOG,0);
	inFileWrite((char *)FILE_RES,bXMLFileData,strlen((char *)bXMLFileData));
	inFileWrite((char *)FILE_RES_OK,bXMLFileData,0);
}

int inProcessTxn(long lnProcCode)
{
int inRetVal = SUCCESS;

	if(!memcmp(srTxnData.srIngData.chTMMTI,"0300",4))
		inTransType = TXN_ECC_BATCH_UPLOAD;
	else
	{
		switch(lnProcCode)
		{
			case 708070:
				//inTransType = TXN_SALE2;
				inTransType = TXN_SALE;
				break;
			case 727080:
				inTransType = TXN_REFUND;
				break;
			case 607000:
				inTransType = TXN_REDEEM;
				break;
			case 620070:
				inTransType = TXN_REDEEM_REFUND;
				break;
			case 500060:
			case 505060:
				inTransType = TXN_ADJECT_CARD;
				break;
			case 528070:
				inTransType = TXN_GIFT;
				break;
			case 800070:
				inTransType = TXN_ADD_POINT;
				break;
			case 900000:
				inTransType = TXN_SETTLE;
				break;
			case 606100:
				if(srTxnData.srREQData.chDiscountType == 'D')
					inTransType = TXN_SALE;
				else
					inTransType = TXN_ECC_DEDUCT;
				break;
			case 620061:
				inTransType = TXN_ECC_REFUND;
				break;
			case 801061:
				inTransType = TXN_ECC_ADD;
				break;
			case 816100:
				inTransType = TXN_ECC_VOID;
				break;
			case 516108:
				inTransType = TXN_ECC_TX_REFUND;
				break;
			case 510861:
				inTransType = TXN_ECC_TX_ADD;
				break;
			case 216000:
				inTransType = TXN_ECC_READ_DEDUCT;
				break;
			case 214100:
				inTransType = TXN_ECC_READ_DONGLE;
				break;
			case 296000:
				if(srTxnData.srREQData.chDiscountType == 'D')
					inTransType = TXN_QUERY_POINT;
				else
					inTransType = TXN_ECC_READ_BASIC;
				break;
			case 390061:
				inTransType = TXN_ECC_SET_VALUE;
				break;
			case 390861:
				inTransType = TXN_ECC_AUTOLOAD_ENABLE;
				break;
			case 296061:
				inTransType = TXN_ECC_READ_CARD;
				break;
			case 881999:
				inTransType = TXN_ECC_SIGN_ON;
				break;
			case 881899:
				inTransType = TXN_ECC_RESET_OFF;
				break;
			case 881799:
				inTransType = TXN_ECC_SIGN_ON_QUERY;
				break;
			case 861799:
				inTransType = TXN_ECC_REFUND_CARD;
				break;
			case 825799:
				inTransType = TXN_ECC_AUTOLOAD;
				break;
			case 900099:
				inTransType = TXN_ECC_SETTLE;
				break;
			case 296099:
				inTransType = TXN_ECC_MULTI_SELECT;
				break;
			case 882099:
				inTransType = TXN_ECC_ANTENN_ACONTROL;
				break;
			case 630661:
				inTransType = TXN_ECC_ADD2;
				break;
			case 606102:
				inTransType = TXN_ECC_DEDUCT_TAXI;
				break;
			case 1:
				inTransType = TXN_ECC_TEST1;
				break;
			case 882999:
				inTransType = TXN_ECC_MATCH;
				break;
			case 882899:
				inTransType = TXN_ECC_MATCH_OFF;
				break;
			case 817799:
				inTransType = TXN_ECC_ACCUNT_LINK;
				break;
			case 296062:
				inTransType = TXN_ECC_FAST_READ_CARD;
				break;
			case 296060:
				inTransType = TXN_ECC_READ_DEVICE_INFO;
				break;
			case 900098:
				inTransType = TXN_ECC_SETTLE2;
				break;
			case 296063:
				inTransType = TXN_ECC_READ_TAXI;
				break;
			case 390062:
				inTransType = TXN_ECC_STUDENT_SET_VALUE;
				break;
			case 390063:	// 20年展期
				inTransType = TXN_ECC_EXTEND_VALID;
				break;
			case 801062:
				inTransType = TXN_ECC_ADD3;
				break;
			case 800062:
				inTransType = TXN_ECC_ADD_INQ;
				break;
			case 900097:
				inTransType = TXN_ECC_ADVICE;
				break;
			case 801063:
				inTransType = TXN_E_ADD;
				break;
			case 606300:
				inTransType = TXN_E_DEDUCT;
				break;
			case 607090:
				inTransType = TXN_GIFT_DEDUCT;
				break;
			case 296064:
				inTransType = TXN_ECC_READ_THSRC;
				break;
			case 606500:
				inTransType = TXN_E_QR_DEDUCT;
				break;
			case 620063:
				inTransType = TXN_E_REFUND;
				break;
			case 816300:
				inTransType = TXN_E_VOID_ADD;
				break;
			case 200163:
				inTransType = TXN_E_READ_BASIC;
				break;
			case 200164:
				inTransType = TXN_E_READ_QR_CODE;
				break;
			case 801065:
				inTransType = TXN_E_QR_ADD;
				break;
			case 816500:
				inTransType = TXN_E_QR_VOID_ADD;
				break;
			case 620065:
				inTransType = TXN_E_QR_REFUND;
				break;
			case 226300:
				inTransType = TXN_E_RETRY_QUERY;
				break;
			case 606103:
				inTransType = TXN_ECC_DEDUCT_CBIKE;
				break;
			case 296065:
				inTransType = TXN_ECC_READ_CBIKE;
				break;
			case 606104:
				inTransType = TXN_ECC_DEDUCT_EDCA;
				break;
			case 296066:
				inTransType = TXN_ECC_READ_EDCA;
				break;
			/*case 296067:
				inTransType = TXN_ECC_TLRT_AUTH;
				break;*/
			// 連線機具加值
			case 801064:
				inTransType = TXN_ECC_ADD4;
				break;
			// 點數兌換加值	
			case 801066:
				inTransType = TXN_ECC_PT_ADD;
				break;
			case 296067:
				inTransType = TXN_ECC_FAST_READ_CARD2;
				break;
			// 離線機具加值
			case 801081:
				inTransType = TXN_ECC_ADD_OFFLINE1;
				break;
			//離線取消加值
			case 811081:
				inTransType = TXN_ECC_VOID_OFFLINE1;
				break;
			// 連線機具取消加值
			case 811064:
				inTransType = TXN_ECC_VOID1;
				break;
			default:
				if(lnProcCode / 100000 == 2)
					inTransType = TXN_QUERY_POINT;
				else
					return TM_PROCESS_CODE_ERROR;
				break;
		}
	}

	srTxnData.srIngData.inTransType = inTransType;

	vdSetCMASTagDefault(&srXMLData,srTxnData.srIngData.inTransType);

	if(fIsECCTxn(inTransType) == TRUE)
		return inECCTxnProcess(inTransType);
	//if(inTransType == TXN_SETTLE)
	//	return inSettleProcess();

	if(fIsETxn(inTransType) == TRUE)//數位悠遊卡交易
		return inETxnProcess(inTransType);

	{
		char chOnlineFlag;//Online Flag,0=R6 / 1=CMAS
		char chBatchFlag;//Batch Flag,0=CMAS Batch / 1=SIS2 Batch / 2=R6
		char chTCPIP_SSL;
		char PacketLenFlag;//PacketLenFlag 0=電文沒有2Byte長度 / 1=電文有2Byte長度
		BYTE bBuf[20];
		STRUCT_XML_DOC srTmpXMLData;

		if(srTxnData.srIngData.inTransType == TXN_SALE && srTxnData.srREQData.chDiscountType == 'D')
		{
			vdSetICERTxnParameter(&chOnlineFlag,&chBatchFlag,&chTCPIP_SSL,&PacketLenFlag);
			vdXMLBackUpData(&srXMLData,&srTmpXMLData,FALSE);
			vdSetConfigFlag(&srXMLData,2,(char *)TAG_NAME_4108);
			memset(bBuf,0x00,sizeof(bBuf));
			sprintf((char *)bBuf,"%ld00",srTxnData.srIngData.lnECCAmt);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0100,(BYTE *)"0200",TRUE);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0300,(BYTE *)"708070",TRUE);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0403,bBuf,TRUE);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_4200,srTxnData.srParameter.bMerchantID,TRUE);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_5503,(BYTE *)srTxnData.srParameter.APIDefaultData.srAPIInData2.chTMLocationID,TRUE);
		}

		inRetVal = inQueryPoint(inTransType);

		if(inRetVal < SUCCESS)
		{
			log_msg(LOG_LEVEL_ERROR,"inProcessTxn(%ld) Fail 1:%d",lnProcCode,inRetVal);
			inReversalProcess(HOST_ICER);
			return inRetVal;
		}

		if(srTxnData.srIngData.inTransType == TXN_SALE && srTxnData.srREQData.chDiscountType == 'D')
		{
			vdGetICERTxnParameter(chOnlineFlag,chBatchFlag,chTCPIP_SSL,PacketLenFlag);
			memset(bBuf,0x00,sizeof(bBuf));
			inXMLGetData(&srXMLData,(char *)TAG_NAME_0406,(char *)bBuf,sizeof(bBuf),0);
			vdFreeXMLDOC(&srXMLData);
			vdXMLBackUpData(&srTmpXMLData,&srXMLData,TRUE);
			vdFreeXMLDOC(&srTmpXMLData);
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4200,(char *)TAG_NAME_0406,bBuf,FALSE);
			memset(srTxnData.srIngData.anAutoloadAmount,0x00,sizeof(srTxnData.srIngData.anAutoloadAmount));
		}

		inRetVal = inDeductOrRefundProcess(inTransType);

		if(srTxnData.srIngData.inTransType == TXN_SALE && srTxnData.srREQData.chDiscountType == 'D')
			vdSetICERTxnParameter(&chOnlineFlag,&chBatchFlag,&chTCPIP_SSL,&PacketLenFlag);

		if(inRetVal < SUCCESS && !srTxnData.srIngData.fCardActionOKFlag)
		{
			if(fIsOnlineTxn() == TRUE && fIsRetryTxn(inTransType,inRetVal) != TRUE)
				vdReversalOn(HOST_ICER);
			log_msg(LOG_LEVEL_ERROR,"inProcessTxn(%ld) Fail 2:%d",lnProcCode,inRetVal);
			inReversalProcess(HOST_ICER);
			return inRetVal;
		}
	}

	if((inRetVal < SUCCESS && srTxnData.srIngData.fCardActionOKFlag) ||
		srTxnData.srIngData.fICERAdviceFlag/* ||
		inRetVal == SUCCESS && fIsOnlineTxn() == FALSE*/)
	{
		if(fIsMustReversal() == TRUE)
		{
			vdReversalOn(HOST_ICER);
			//inXMLModifyData(&srXMLData,(char *)TAG_NAME_3900,"98");
			inXMLUpdateData(&srXMLData,(char *)TAG_NAME_4100,(char *)TAG_NAME_3900,(BYTE *)"98",FALSE);
		}
		else
		{
			BYTE bSendBuf[MAX_XML_FILE_SIZE];
			memset(bSendBuf,0x00,sizeof(bSendBuf));
			if(inBuildSendPackage(HOST_ICER,&srXMLData,bSendBuf,REVERSAL_OFF,ADVICE_ON,STORE_BATCH_OFF) < SUCCESS)
				log_msg(LOG_LEVEL_ERROR,"inProcessTxn(%ld) Fail 4:%d",lnProcCode,inRetVal);

			//if(inTransType != TXN_SETTLE && inTransType != TXN_REFUND)//Advice換到這裡來,當筆交易做完馬上送!!
			//	inTCPIPAdviceProcess(HOST_ICER,FALSE);
		}
		inRetVal = SUCCESS;
	}

	if(inRetVal == SUCCESS)
	{
		if(inTransType != TXN_SETTLE && inTransType != TXN_REFUND)//Advice換到這裡來,當筆交易做完馬上送!!
				inTCPIPAdviceProcess(HOST_ICER,FALSE);
	}

	inReversalProcess(HOST_ICER);

	vdTxnEndProcess(inTransType,srTxnData.srIngData.chTxDate);

	return inRetVal;
}

BOOL fDLLResponseLock(int inReaderSW)
{

	if(inReaderSW == 0x6304 || inReaderSW == 0x6305 || inReaderSW == 0x630D || inReaderSW == 0x630E)
		return TRUE;

	if(inReaderSW == 0x640E || inReaderSW == 0x6406)
		return TRUE;

	if(srTxnData.srParameter.gDLLVersion == 2)
		if(inReaderSW == 0x6103 || inReaderSW == 0x610F || inReaderSW == 0x6418)
			return TRUE;

	return FALSE;
}

BOOL fDLLResponseCheck(int inRet)
{

	if(inRet == ICER_DECLINE_FAIL || inRet == R6_DECLINE_FAIL || inRet == ECC_LOCK_CARD/* || inRet == BANK_DECLINE_FAIL*/)
		return TRUE;

	return FALSE;
}

#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
/**
 * setup the message log.
 *
 * @param pathname
 *        pathname of message log. NULL indicated the disable the facility.
 *
 * @return TRUE for successful, FALSE otherwise.
 */
BOOL set_log(const char* pathname)
{
	assert(pathname != 0);
	char szFullPathName[PATH_MAX+20];//1.1.5    //@ 原100，加大以避免overflow問題
	time_t utTime;
	struct tm * gTime;
	DIR* dir;
	int i,inFileCnt;
	struct dirent** log_list;
	char log_pathname[PATH_MAX];

	close_log();

	if(!pathname)
		return TRUE;

	time(&utTime);
	gTime=localtime(&utTime);
	memset(szFullPathName,0x00,sizeof(szFullPathName));
#if READER_MANUFACTURERS==ANDROID_API
	sprintf(szFullPathName,"%s/ICERLog",JNI_LogPath);
#else
	sprintf(szFullPathName,"%s/ICERLog",gCurrentFolder);
#endif
	dir = opendir(szFullPathName);
	if(dir <= 0)//銵函內鞈?憭曆?摮
		mkdir(szFullPathName,0777);
	else
	{
		closedir(dir);

#ifdef RHEL5__
		switch (inFileCnt = scandir(szFullPathName, &log_list, log_filter, versionsort)) {
#else
		switch (inFileCnt = scandir(szFullPathName, &log_list, log_filter, alphasort)) {
#endif
			case -1:
				call_stack("malloc() within scandir() failed: %s", strerror(errno));
				return 0;
			case 0:
				log_msg(LOG_LEVEL_FLOW,"set_log scandir return 0\n");
				break;
			default:
				for (i = 0; i < inFileCnt; ++i) {
					sprintf(log_pathname, "%s/%s", szFullPathName, log_list[i]->d_name);
					if(inFileCnt - i > MAX_LOG_FILE) {
						TRACE_("obsoleted LOG file: %s", log_pathname);

						if(remove(log_pathname)) {
							log_msg(LOG_LEVEL_ERROR,"remove %s failed: %s", log_pathname, strerror(errno));
						}
					}
					ECC_free((BYTE *)log_list[i]);
				}
				ECC_free((BYTE *)log_list);
				break;
		}
	}

	memset(szFullPathName,0x00,sizeof(szFullPathName));
#if READER_MANUFACTURERS==ANDROID_API
	sprintf(szFullPathName,"%s/ICERLog/ICER%04d%02d%02d.log",JNI_LogPath,gTime->tm_year+1900,gTime->tm_mon+1,gTime->tm_mday);
#else
	sprintf(szFullPathName,"%s/ICERLog/ICER%04d%02d%02d.log",gCurrentFolder,gTime->tm_year+1900,gTime->tm_mon+1,gTime->tm_mday);
#endif
	chmod(szFullPathName,0777);

	{//若不加這段,Log檔會多一堆0x00.不知為何?
		long lnSize = 0L;

		lnSize = inFileGetSize(szFullPathName,TRUE);
		//printf("inFileGetSize(%s) = %d \n",szFullPathName,lnSize);
	}

	if((fd_ = open(szFullPathName, O_CREAT|O_RDWR|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO)) >= 0) {
		//atexit(close_log);//閮剔蔭蝔?甇?虜蝯??矽?函??賣
		memset(szFullPathName,0x00,sizeof(szFullPathName));
	#if READER_MANUFACTURERS==ANDROID_API
		sprintf(szFullPathName,"%s/ICERLog/RDebug%02d%02d%02d.log",JNI_LogPath,gTime->tm_year-100,gTime->tm_mon+1,gTime->tm_mday);
	#else
		sprintf(szFullPathName,"%s/ICERLog/RDebug%02d%02d%02d.log",gCurrentFolder,gTime->tm_year-100,gTime->tm_mon+1,gTime->tm_mday);
	#endif
		chmod(szFullPathName,0777);
		if((fd2_ = open(szFullPathName, O_CREAT|O_RDWR|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO)) < 0)
			fd2_ = DEFAULT_LOG_FILENO;
		return TRUE;
	}

	fd_ = DEFAULT_LOG_FILENO;

	log_msg(LOG_LEVEL_ERROR,"set_log(%s) failed: %s, using standard output instead",
		szFullPathName, strerror(errno));

	return FALSE;
}

/**
 * close message log.
 */
void close_log(void)
{
	if((fd_ >= 0) &&(DEFAULT_LOG_FILENO != fd_))
	{
		if(close(fd_))
			TRACE_("close log failed: %s", strerror(errno));
		fd_ = -1;
	}
	if((fd2_ >= 0) &&(DEFAULT_LOG_FILENO != fd2_))
	{
		if(close(fd2_))
			TRACE_("close log failed: %s", strerror(errno));
		fd2_ = -1;
	}
}

/**
 * log application stack trace on message log, preceded by a header according
 * to specific format, fmt. the usage for this function is the same as printf().
 */
void call_stack(const char* fmt, ...)
{
time_t utTime;
struct tm * gTime;
char CurTime[20];
//time_t t;
char msg[MAX_MSG+50], mlog[MAX_MSG];    //@ 加大以避免overflow問題
va_list args;

	assert(fmt != 0);
	va_start(args, fmt);
	vsprintf(mlog, fmt, args);
	va_end(args);
	time(&utTime);
	gTime=localtime(&utTime);
	memset(CurTime,0,sizeof(CurTime));
	strftime(CurTime,20,"%Y/%m/%d %H:%M:%S",gTime);
	sprintf(msg, "%s(Start)\n%s\n", CurTime, mlog);//Ctime撠???ASCII摮葡
	//sprintf(msg, "%s%s\n\n", ctime(&t), mlog);
#if READER_MANUFACTURERS==ANDROID_API
	write(fd_, msg, strlen(msg));
#else
	if(write(fd_, msg, strlen(msg)) < 0)
		syslog(LOG_MAKEPRI(LOG_USER, LOG_INFO), "%s", mlog);
#endif
}

int log_filter(const struct dirent* entry)//1.1.5
{
	assert(entry != 0);
	return ere_matches(entry->d_name, LOG_FILE_PATTERN, FALSE);
}

void TRACE_(const char* fmt, ...) {}
void TRACE0_(const char* msg) {}

/**
 * compares the null-terminated string, str, with specified ERE
 * (Extended Regular Expression) pattern, pattern.
 *
 * @param str
 *        a string to be compared.
 * @param pattern
 *        a ERE pattern string
 * @param case_sensitive
 *        if TRUE, the comparison will be case-sensitive, FALSE otherwise.
 *
 * @return TRUE for successful, FALSE otherwise.
 */
BOOL ere_matches(const char* str, const char* pattern, BOOL case_sensitive)
{
	regex_t re;
	int err;
	BOOL ret = FALSE;

	assert(str != 0);
	assert(pattern != 0);

	if (!compile_ere(&re, pattern, case_sensitive, FALSE))
		return ret;
	err = regexec(&re, str, 0, 0, 0);
	if (err)
	{
		if (err != REG_NOMATCH)
		{
			const char *const msg = regerror_msg(&re, err);

			if (msg != 0)
			{
				log_msg(LOG_LEVEL_ERROR,"ere_matches(): regexec() failed: %s", msg);
				ECC_free((BYTE *)msg);
			}
		}
	}
	else
		ret = TRUE;
	regfree(&re);

	return ret;
}

/**
 * compile a regex_t object according to the specific ERE
 * (Extended Regular Expression) pattern, pattern.
 *
 * @return TRUE for successful, FALSE otherwise.
 */
BOOL compile_ere(regex_t* reg,const char* pattern,BOOL case_sensitive,BOOL no_detail)
{

	int err;
	int cflag = REG_EXTENDED;

	assert(reg != 0);
	assert(pattern != 0);

	if (!case_sensitive)
		cflag |= REG_ICASE;
	if (no_detail)
		cflag |= REG_NOSUB;
	err = regcomp(reg, pattern, cflag);
	if (err)
	{
		const char* const msg = regerror_msg(reg, err);

		if (msg != 0)
		{
			log_msg(LOG_LEVEL_ERROR,"compile_ere(): regcomp() failed: %s", msg);
			ECC_free((BYTE *)msg);
		}
	}
	return 0 == err;
}

/**
 * the memory which returned string pointered to by this function
 * (a NON-null value) should be released by calling free(void *)
 * if it is no longer to use.
 */
const char* const regerror_msg(const regex_t* reg, int err)
{
	size_t msgsize;
	char* msg = 0;

	assert(reg != 0);

	msgsize = regerror(err, reg, 0, 0);
	msg = (char*)ECC_calloc(1,msgsize);

	if (msg != 0)
		regerror(err, reg, msg, msgsize);
	else
		log_msg(LOG_LEVEL_ERROR,"regerror_msg(): malloc() failed: %s", strerror(errno));
	return msg;
}
#endif

#ifdef LIB_DEFINE

#if READER_MANUFACTURERS==ANDROID_API
JNIEXPORT jint JNICALL Java_icerapi_icerapi_ICERAPI_inICERAPI_1InitialReader(JNIEnv *env, jobject jobject1, jstring filePath, jstring LogPath, jobject context)
#else
int inICERAPI_InitialReader(void)
#endif
{

	#if READER_MANUFACTURERS==ANDROID_API
		Jenv = env;
		jobject LocReaderObj;

		//memcpy(&Jni_usbManager,&usbManager,sizeof(jobject));
		memcpy(&Jni_context,&context,sizeof(jobject));

		memset(ANDROID_FILE_PATH,0x00,sizeof(ANDROID_FILE_PATH));
		JNI_filePath = (*Jenv)->GetStringUTFChars(Jenv,filePath,0);
		memcpy(ANDROID_FILE_PATH,JNI_filePath,strlen(JNI_filePath));
		log_msg(LOG_LEVEL_FLOW,"FilePath: %s , length: %d, ANDROID_FILE_PATH: %s",JNI_filePath,strlen(JNI_filePath),ANDROID_FILE_PATH);

		JNI_LogPath = (*Jenv)->GetStringUTFChars(Jenv,LogPath,0);
		log_msg(LOG_LEVEL_FLOW,"FilePath: %s , length: %d",JNI_LogPath,strlen(JNI_LogPath));

		jclass clazz3 = (*Jenv) -> FindClass(Jenv,"icerapi/icerapi/ReaderAPI_Interface");
		jmethodID AndroidReaderInit = (*Jenv) -> GetMethodID(Jenv,clazz3,"<init>","()V");
		jmethodID AndroidSystemParamentInialize = (*Jenv) -> GetMethodID(Jenv,clazz3,"Ts_SystemParamentInialize","(Ljava/lang/String;Ljava/lang/String;Landroid/content/Context;)V");

		LocReaderObj = (*Jenv) -> NewObject(Jenv,clazz3,AndroidReaderInit);
		jstring str1 = (*Jenv)->NewStringUTF(Jenv,JNI_filePath);
		jstring str2 = (*Jenv)->NewStringUTF(Jenv,JNI_LogPath);
		(*Jenv) -> CallVoidMethod(Jenv,LocReaderObj, AndroidSystemParamentInialize, str1, str2, Jni_context);

	#else

		Ts_SystemParamentInialize();

	#endif

	return SUCCESS;
}

int inICERAPI_InitialReader2(char chSlot)
{

	#if READER_MANUFACTURERS==ANDROID_API
	#else
		Ts_SystemParamentInialize2(chSlot);
	#endif

	return SUCCESS;
}

#if READER_MANUFACTURERS==ANDROID_API
JNIEXPORT void JNICALL Java_icerapi_icerapi_ICERAPI_icerapi_1readerMode(JNIEnv *env, jobject jobject1, jstring filePath, jstring logPath,jobject context, jobject usbManager, jint inReaderMode, jint inComPort, jlong ulBaudrate, jbyte bParity, jbyte bDataBits, jbyte bStopBits, jbyte bContinueFlag, jbyte bOpenFlaged, jint inTimeout)
#else
void vdICERAPI_ReaderMode(int inReaderMode,int inComPort,unsigned long ulBaudrate,BYTE bParity,BYTE bDataBits,BYTE bStopBits,BYTE bContinueFlag,BYTE bOpenFlaged, int inTimeout)
#endif
{
int inRetVal,inRecvCnt = 0,inReadLen = 0,inCnt;
BYTE bSendData[READER_BUFFER],bRecvData[READER_BUFFER];

#if READER_MANUFACTURERS==WINDOWS_API  || READER_MANUFACTURERS==WINDOWS_CE_API
clock_t lnStartTime, lnEndTime;
clock_t tTimeout = inTimeout * 1000;
#elif  READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
time_t lnStartTime, lnEndTime;
time_t tTimeout = inTimeout;
#else
clock_t lnStartTime, lnEndTime;
clock_t tTimeout = inTimeout* 100;
#endif

#if READER_MANUFACTURERS==ANDROID_API

//jint inReaderMode,inComPort;
//unsigned long ulBaudrate;
//BYTE bParity,bDataBits,bStopBits,bContinueFlag,bOpenFlaged;
extern int sockfd_server;
struct sockaddr_in addr;
int newsockfd,addr_len = sizeof(struct sockaddr_in);

	/*jint *jaReaderMode = (*env)->GetIntArrayElements(Jenv, jintReaderMode, 0);
	inReaderMode = jaReaderMode[0];
	jint *jaComPort = (*env)->GetIntArrayElements(Jenv, jintComPort, 0);
	inComPort = jaComPort[0];*/
	Jenv = env;

	jclass clazz3 = (*Jenv) -> FindClass(Jenv,"icerapi/icerapi/ReaderAPI_Interface");
	jmethodID AndroidReaderInit = (*Jenv) -> GetMethodID(Jenv,clazz3,"<init>","()V");
	AndroidUart0infoProcess2 = (*Jenv) -> GetMethodID(Jenv,clazz3,"Ts_Uart0infoProcess2","(Ljava/lang/String;Ljava/lang/String;Landroid/hardware/usb/UsbManager;Landroid/content/Context;[B[B[B[I[B[B)V");
	ReaderObj = (*Jenv) -> NewObject(Jenv,clazz3,AndroidReaderInit);

	LOGD("vdICERAPI_ReaderMode 1 !!");

#endif

	if(bOpenFlaged == FALSE)
	{
		if(inReaderMode == TCPIP_READER)
		{
			inRetVal = inTCPIPServerListen((unsigned short)inComPort);

		#if READER_MANUFACTURERS==ANDROID_API
			LOGD("vdICERAPI_ReaderMode 2,%d !!",inRetVal);
		#endif
		}
		else
		{
			srTxnData.srParameter.gComPort = inComPort;
			srTxnData.srParameter.ulBaudRate = ulBaudrate;
			srTxnData.srParameter.bRS232Parameter[0] = bParity;
			srTxnData.srParameter.bRS232Parameter[1] = bDataBits;
			srTxnData.srParameter.bRS232Parameter[2] = bStopBits;
			inComOpen();
			//inRetVal = ECC_OpenCOM_Lib(inComPort,ulBaudrate,bParity,bDataBits,bStopBits);
		}
		if(inRetVal != SUCCESS)
			return;
	}

	while(1)
	{
		memset(bSendData,0x00,sizeof(bSendData));//接收外部資料,傳送給讀卡機之變數
		memset(bRecvData,0x00,sizeof(bRecvData));//接收讀卡機資料,傳送給外部之變數
		inReadLen = 1;
		inRecvCnt = 0;

	#if READER_MANUFACTURERS==ANDROID_API
		if(inReaderMode == TCPIP_READER)
		{
			newsockfd = accept(sockfd_server,(struct sockaddr*)&addr,(socklen_t*)&addr_len);

			LOGD("vdICERAPI_ReaderMode 3,%d !!",newsockfd);
			if(newsockfd < 0)
				continue;
		}
	#endif

		if (tTimeout > 0)
		{
			#if READER_MANUFACTURERS==WINDOWS_API 
				lnStartTime = lnEndTime = clock();
			#elif READER_MANUFACTURERS==WINDOWS_CE_API
				lnStartTime = lnEndTime = GetTickCount();
			#elif  READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
				lnStartTime = time((time_t*)0);
				lnEndTime = lnStartTime;
			#else
				lnStartTime = ECC_GetTickTime_Lib();
			#endif
		}

		while(1)
		{
			inReadLen = 1;
		#if READER_MANUFACTURERS==ANDROID_API
			if(inReaderMode == TCPIP_READER)
			{
				inRetVal = recv(newsockfd,&bSendData[inRecvCnt], sizeof(bSendData),0);

				LOGD("vdICERAPI_ReaderMode 4,%d,%d !!",inRetVal,inReadLen);
				if(inRetVal <= 0)
				{
					close(newsockfd);
					break;
				}
				else
				{
					inReadLen = inRetVal;
					//inRetVal = SUCCESS;
				}
			}
			else
		#endif
				inRetVal = inComRecvByLen(&bSendData[inRecvCnt], (unsigned long)inReadLen,10000);
		/*#if READER_MANUFACTURERS==NE_VX520
			inRetVal = ECC_COMRxData_Lib(inComPort, &bSendData[inRecvCnt], &inReadLen);
		#else
			inRetVal = ECC_COMRxData_Lib(inComPort, &bSendData[inRecvCnt], (unsigned short *)&inReadLen);
		#endif*/
			//if(inRetVal == SUCCESS)
			//	inRecvCnt += inReadLen;
			if(inRetVal > 0)
				inRecvCnt += inRetVal;
			//if(inRecvCnt >= 3 && memcmp(bSendData,"\x55\x55\x55",3))//接收頭3BYTE完成,且頭3Byte不是\x55\x55\x55
			if(inRecvCnt >= 9 && !memcmp(bSendData,"\x55\x55\x55\x55\x55\x55\x55\x55\x55",9))
			{
				inRecvCnt = 0;
				memset(bSendData,0x00,sizeof(bSendData));
				memset(bRecvData,0x00,sizeof(bRecvData));
				memcpy(bRecvData,"\x00\x00\x02\x90\x00\x92",6);
			#if READER_MANUFACTURERS==ANDROID_API
				if(inReaderMode == TCPIP_READER)
					send(newsockfd,bRecvData,6,0);
			#endif
					inComSend(bRecvData,6);
				/*ECC_COMClearRxBuffer_Lib(inComPort);
				inRetVal = ECC_COMTxReady_Lib(inComPort);
				if(inRetVal == SUCCESS)
					ECC_COMTxData_Lib(inComPort,bRecvData,6);*/
				continue;
			}
			else if(inRecvCnt >= 9 && !memcmp(bSendData,"\x55\x55\x55\x55\x55\x55\x55\x55\x66",9))
			{
			#if READER_MANUFACTURERS==ANDROID_API
				if(inReaderMode == TCPIP_READER)
				{
					close(newsockfd);
					close(sockfd_server);
				}
			#endif
				return;
			}
			else if (inRecvCnt >= 3)
			{
				if (memcmp(bSendData,"\x00\x00",2) == 0 )
					break;
				else if (memcmp(bSendData,"\x55\x55\x55",3) != 0)
					return;
			}

			if (tTimeout > 0)
			{
				#if READER_MANUFACTURERS==WINDOWS_API 
				if (((lnEndTime = clock()) - lnStartTime) >=  tTimeout)
					return;
				#elif READER_MANUFACTURERS==WINDOWS_CE_API
				if (((lnEndTime = GetTickCount()) - lnStartTime) >=  tTimeout)
					return;
				#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
				if (((lnEndTime = time((time_t*)0)) - lnStartTime) >=  tTimeout)
					return;
				#else
				if (((lnEndTime = ECC_GetTickTime_Lib()) - lnStartTime) >=  tTimeout)
					return;
				#endif
			}
		}

	#if READER_MANUFACTURERS==ANDROID_API
		LOGD("vdICERAPI_ReaderMode 5,%d !!",inRetVal);
	#endif

		inReadLen = bSendData[2] + 1;
		while(1)
		{
		#if READER_MANUFACTURERS==ANDROID_API
			if(inReaderMode == TCPIP_READER)
				break;
		#endif
				inRetVal = inComRecvByLen(&bSendData[inRecvCnt], (unsigned long)inReadLen,10000);
		/*#if READER_MANUFACTURERS==NE_VX520
			inRetVal = ECC_COMRxData_Lib(inComPort, &bSendData[inRecvCnt], &inReadLen);
		#else
			inRetVal = ECC_COMRxData_Lib(inComPort, &bSendData[inRecvCnt], (unsigned short *)&inReadLen);
		#endif*/
			//if(inRetVal == SUCCESS)
			//	inRecvCnt += inReadLen;
			if(inRetVal > 0)
				inRecvCnt += inRetVal;
			
			if(inRecvCnt >= bSendData[2] + 4)//接收頭所有資料完成
				break;
			if (tTimeout > 0)
			{
				#if READER_MANUFACTURERS==WINDOWS_API 
				if (((lnEndTime = clock()) - lnStartTime) >=  tTimeout)
					return;
				#elif READER_MANUFACTURERS==WINDOWS_CE_API
				if (((lnEndTime = GetTickCount()) - lnStartTime) >=  tTimeout)
					return;
				#elif READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
				if (((lnEndTime = time((time_t*)0)) - lnStartTime) >=  tTimeout)
					return;
				#else
				if (((lnEndTime = ECC_GetTickTime_Lib()) - lnStartTime) >=  tTimeout)
					return;
				#endif
			}
		}

		//ApTraceHexPrint((char*)"Rx:",bSendData,inRecvCnt,CPI_TRACE_PRT_BUF, PRT_TX_COMM_BUF);
		inCnt = inRecvCnt;
		//傳給讀卡機Lib
  //#ifdef LIB_DEFINE
	#ifdef LIB_DEFINE2

		if((inReaderMode == LIBARY_READER2) || (inReaderMode == TCPIP_READER))
		{
			memset((char *)&g_sUart0RecieveInfo2,0x00,sizeof(g_sUart0RecieveInfo2));
			memset((char *)&g_sUart0AnswerInfo2,0x00,sizeof(g_sUart0AnswerInfo2));

			memcpy(g_sUart0RecieveInfo2.cProLog,bSendData,sizeof(g_sUart0RecieveInfo2.cProLog));
			memcpy(g_sUart0RecieveInfo2.cInfoHeader,&bSendData[3],sizeof(g_sUart0RecieveInfo2.cInfoHeader));
			memcpy(g_sUart0RecieveInfo2.cInfoBody,&bSendData[3 + 4],inCnt - 4);
			g_sUart0RecieveInfo2.cEDC = bSendData[inCnt - 1];
			iUart0RecieveOK = chSerialPortMode = chProtectFlag = 0;

			log_msg(LOG_LEVEL_FLOW,"Ts_Uart0InfoProcess2 Start 2,%d,%d,%d,%d,%d,%d!!",sizeof(g_sUart0RecieveInfo2),sizeof(g_sUart0AnswerInfo2),chAnswerLen,iUart0RecieveOK,chSerialPortMode,chProtectFlag);
			log_msg(LOG_LEVEL_FLOW,"Ts_Uart0InfoProcess2 Start 2-1,%x,%x,%x,%x,%x,%x!!",&g_sUart0RecieveInfo2,&g_sUart0AnswerInfo2,&chAnswerLen,&iUart0RecieveOK,&chSerialPortMode,&chProtectFlag);
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
			//(*Jenv) -> CallVoidMethod(Jenv,ReaderObj,AndroidUart0infoProcess2,str1,str2,Jni_usbManager,Jni_context,jSendData,jRecvData,jAnswerLen,jUart0RecieveOK,jSerialPortMode,jProtectFlag);
			(*Jenv) -> CallVoidMethod(Jenv,ReaderObj,AndroidUart0infoProcess2,str1,str2,usbManager,context,jSendData,jRecvData,jAnswerLen,jUart0RecieveOK,jSerialPortMode,jProtectFlag);
			if(jRecvData != NULL)
			{
				jsize len = (*Jenv) -> GetArrayLength(Jenv,jRecvData);
				jbyte* jbarray = (jbyte*) ECC_calloc(len,sizeof(jbyte));
				(*Jenv) -> GetByteArrayRegion(Jenv,jRecvData,0,len,jbarray);
				memcpy(&g_sUart0AnswerInfo2,jbarray,len);
				ECC_free(jbarray);
			}
		#else
			inRetVal = Ts_Uart0InfoProcess2(&g_sUart0RecieveInfo2,&g_sUart0AnswerInfo2,&chAnswerLen,&iUart0RecieveOK,&chSerialPortMode,&chProtectFlag);
		#endif
			log_msg(LOG_LEVEL_FLOW,"Ts_Uart0InfoProcess2 End 2!!");
			memcpy(bRecvData,g_sUart0AnswerInfo2.cProLog,sizeof(g_sUart0AnswerInfo2.cProLog));
			if(g_sUart0AnswerInfo2.cProLog[2] >= 2)
			{
				memcpy(&bRecvData[3],g_sUart0AnswerInfo2.cInfoBody,g_sUart0AnswerInfo2.cProLog[2] - 2);
				memcpy(&bRecvData[3 + g_sUart0AnswerInfo2.cProLog[2] - 2],g_sUart0AnswerInfo2.cSW,2);
				bRecvData[3 + g_sUart0AnswerInfo2.cProLog[2]] = g_sUart0AnswerInfo2.cEDC;
			}
		}
		//else

	//#endif
	#else

		if(inReaderMode == LIBARY_READER || inReaderMode == TCPIP_READER)
		{
			memset((char *)&g_sUart0RecieveInfo,0x00,sizeof(g_sUart0RecieveInfo));
			memset((char *)&g_sUart0AnswerInfo,0x00,sizeof(g_sUart0AnswerInfo));

			memcpy(g_sUart0RecieveInfo.cProLog,bSendData,sizeof(g_sUart0RecieveInfo.cProLog));
			memcpy(g_sUart0RecieveInfo.cInfoHeader,&bSendData[3],sizeof(g_sUart0RecieveInfo.cInfoHeader));
			memcpy(g_sUart0RecieveInfo.cInfoBody,&bSendData[3 + 4],inCnt - 4);
			g_sUart0RecieveInfo.cEDC = bSendData[inCnt - 1];
			g_cProtectFlag = 0;

			log_msg(LOG_LEVEL_FLOW,"Ts_Uart0InfoProcess Start 2");
			Ts_Uart0InfoProcess();
			log_msg(LOG_LEVEL_FLOW,"Ts_Uart0InfoProcess End 2!!");
			//debug(1,g_sUart0AnswerInfo.cProLog,3,3)
			memcpy(bRecvData,g_sUart0AnswerInfo.cProLog,sizeof(g_sUart0AnswerInfo.cProLog));
			if(g_sUart0AnswerInfo.cProLog[2] >= 2)
			{
				//debug(2,g_sUart0AnswerInfo.cInfoBody,20,3)
				//debug(3,g_sUart0AnswerInfo.cSW,2,3)
				memcpy(&bRecvData[3],g_sUart0AnswerInfo.cInfoBody,g_sUart0AnswerInfo.cProLog[2] - 2);
				memcpy(&bRecvData[3 + g_sUart0AnswerInfo.cProLog[2] - 2],g_sUart0AnswerInfo.cSW,2);
				bRecvData[3 + g_sUart0AnswerInfo.cProLog[2]] = g_sUart0AnswerInfo.cEDC;
			}
		}
		//else

	#endif
		{
			//送資料給外部設備
		#if READER_MANUFACTURERS==ANDROID_API
			if(inReaderMode == TCPIP_READER)
			{
				send(newsockfd,bRecvData,bRecvData[2] + 4,0);
				close(newsockfd);
			}
		#endif
			inComSend(bRecvData,bRecvData[2] + 4);
		#if READER_MANUFACTURERS==ANDROID_API
			LOGD("vdICERAPI_ReaderMode 7,%d[%02x%02x%02x%02x%02x] !!",bRecvData[2] + 4,bRecvData[0],bRecvData[1],bRecvData[2],bRecvData[3],bRecvData[4]);
		#endif
			/*ECC_COMClearRxBuffer_Lib(inComPort);
			inRetVal = ECC_COMTxReady_Lib(inComPort);
			if(inRetVal == SUCCESS)
				ECC_COMTxData_Lib(inComPort,bRecvData,bRecvData[2] + 4);*/
		}

		if(bContinueFlag != TRUE)
			break;
	}

	#if READER_MANUFACTURERS==ANDROID_API
		if(inReaderMode == TCPIP_READER)
		{
			close(newsockfd);
			close(sockfd_server);
		}
	#endif

	return;
}

#endif
