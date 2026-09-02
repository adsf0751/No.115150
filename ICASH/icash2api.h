#ifndef ICASH2API_H
#define	ICASH2API_H

#ifdef	__cplusplus
extern "C" {
#endif

    //************************************************************************
    // ICASH LIB EXPORT DEFINITION
    //************************************************************************
    //申裝開通Part1
    unsigned long icasH_SetupReader_1(unsigned char *pucInData, unsigned char *pucOutData);
    //申裝開通Part2
    unsigned long icasH_SetupReader_2(unsigned char *pucInData, unsigned char *pucOutData);
    //開機授權Part1
    unsigned long icasH_TermSysAuth_1(unsigned char *pucInData, unsigned char *pucOutData);
    //開機授權Part2
    unsigned long icasH_TermSysAuth_2(unsigned char *pucInData, unsigned char *pucOutData);
    //尋卡
    unsigned long icasH2_SeCheckService(unsigned char *pucPICCUID);
    //初始化Part1
    unsigned long icasH_InitializeAuth(unsigned char *pucInData, unsigned char *pucOutData);
    //初始化Part2
    unsigned long icasH_InitializeEnv(unsigned char *pucInData);
    //讀取卡片資訊
    unsigned long icasH2_TradeReferCard(unsigned char *pucInData, unsigned char *pucOutData);
    //扣款認證
    unsigned long icasH2_TradeSaleAuth(unsigned char *pucInData,unsigned char *pucOutData);
    //扣款
    unsigned long icasH2_TradeSale(unsigned char *pucInData, unsigned char *pucOutData);
    //讀取icashAPI版本號
    unsigned long icasH2_GetApVersion(unsigned char *ucVer);
    //退貨認證
    unsigned long icasH2_TradeRefundAuth(unsigned char *pucInData, unsigned char *pucOutData);
    //退貨
    unsigned long icasH2_TradeRefund(unsigned char *pucInData, unsigned char *pucOutData);
    //加值認證
    unsigned long icasH2_TradeChargeAuth(unsigned char *pucInData, unsigned char *pucOutData);
    //加值
    unsigned long icasH2_TradeCharge(unsigned char *pucInData, unsigned char *pucOutData);
    //加值取消認證
    unsigned long icasH2_TradeChargeCancelAuth(unsigned char *pucInData,unsigned char *pucOutData);
    //加值取消
    unsigned long icasH2_TradeChargeCancel(unsigned char *pucInData, unsigned char *pucOutData);
    //自動加值認證
    unsigned long icasH2_TradeAutoLoadAuth(unsigned char *pucInData, unsigned char *pucOutData);
    //自動加值
    unsigned long icasH2_TradeAutoLoad(unsigned char *pucInData, unsigned char *pucOutData);
    //鎖卡
    unsigned long icasH2_LockCard(unsigned char *pucInData, unsigned char *pucOutData);

#ifdef	__cplusplus
}
#endif

#endif	/* ICASH2API_H */

