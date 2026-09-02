typedef struct
{
        char    szInitialManagerEnable[1 + 1];		/* 開啟管理者號碼 */
        char    szInitialManagerPwd[4 + 1];		/* 管理者號碼 */
        char    szRebootPwdEnale[1 + 1];		/* 管理開機號碼 */
        char    szRebootPwd[4 + 1];			/* 開機號碼 */
        char    szSalePwdEnable[1 + 1];			/* 開啟銷售密碼 */
        char    szSalePwd[4 + 1];			/* 銷售密碼 */
        char    szPreauthPwdEnable[1 + 1];		/* 開啟預先授權密碼 */
        char    szPreauthPwd[4 + 1];			/* 預先授權密碼 */
        char    szInstallmentPwdEnable[1 + 1];		/* 開啟分期付款密碼 */
        char    szInstallmentPwd[4 + 1];		/* 分期付款密碼 */
        char    szRedeemPwdEnable[1 + 1];		/* 開啟紅利折扣密碼 */
        char    szRedeemPwd[4 + 1];			/* 紅利折扣密碼*/
        char    szMailOrderPwdEnable[1 + 1];		/* 開啟郵購密碼 */
        char    szMailOrderPwd[4 + 1];			/* 郵購密碼 */
        char    szOfflinePwdEnable[1 + 1];		/* 開啟交易補登密碼 */
        char    szOfflinePwd[4 + 1];			/* 交易補登密碼 */
        char    szInstallmentAdjustPwdEnable[1 + 1];	/* 開啟分期付款調帳密碼 */
        char    szInstallmentAdjustPwd[4 + 1];		/* 分期付款調帳密碼 */
        char    szRedeemAdjustPwdEnable[1 + 1];		/* 開啟紅利扣抵調帳密碼 */
        char    szRedeemAdjustPwd[4 + 1];		/* 紅利扣抵調帳密碼 */
        char    szVoidPwdEnable[1 + 1];			/* 開啟取消密碼 */
        char    szVoidPwd[4 + 1];			/* 取消密碼 */
        char    szSettlementPwdEnable[1 + 1];		/* 開啟結帳密碼 */
        char    szSettlementPwd[4 + 1];			/* 結帳密碼 */
        char    szRefundPwdEnable[1 + 1];		/* 開啟退貨密碼 */
        char    szRefundPwd[4 + 1];			/* 退貨密碼 */
        char    szHGRefundPwdEnable[1 + 1];		/* 開啟HappyGo退貨密碼 */
        char    szHGRefundPwd[4 + 1];			/* HappyGo退貨密碼*/
	char	szCallBankForcePwdEnable[1 + 1];	/* 開啟授權碼補登管理號碼，預設值= N。*/
	char	szCallBankForcePwd[4 + 1];		/* 授權碼補登管理號碼 */
} PWD_REC;


/* 以欄位數決定 ex:欄位數是30個，理論上會有29個comma和兩個byte的0x0D 0X0A */
#define _SIZE_PWD_COMMA_0D0A_            31
#define _SIZE_PWD_REC_                  (sizeof(PWD_REC))
#define _PWD_FILE_NAME_                  "PWD.dat"
#define _PWD_FILE_NAME_BAK_              "PWD.bak"

/* Load & Save function */
int inLoadPWDRec(int inPWDRec);
int inSavePWDRec(int inPWDRec);

/* Set function */
int inGetInitialManagerEnable(char *szInitialManagerEnable);
int inGetInitialManagerPwd(char *szInitialManagerPwd);
int inGetRebootPwdEnale(char *szRebootPwdEnale);
int inGetRebootPwd(char *szRebootPwd);
int inGetSalePwdEnable(char *szSalePwdEnable);
int inGetSalePwd(char *szSalePwd);
int inGetPreauthPwdEnable(char *szPreauthPwdEnable);
int inGetPreauthPwd(char *szPreauthPwd);
int inGetInstallmentPwdEnable(char *szInstallmentPwdEnable);
int inGetInstallmentPwd(char *szInstallmentPwd);
int inGetRedeemPwdEnable(char *szRedeemPwdEnable);
int inGetRedeemPwd(char *szRedeemPwd);
int inGetMailOrderPwdEnable(char *szMailOrderPwdEnable);
int inGetMailOrderPwd(char *szMailOrderPwd);
int inGetOfflinePwdEnable(char *szOfflinePwdEnable);
int inGetOfflinePwd(char *szOfflinePwd);
int inGetInstallmentAdjustPwdEnable(char *szInstallmentAdjustPwdEnable);
int inGetInstallmentAdjustPwd(char *szInstallmentAdjustPwd);
int inGetRedeemAdjustPwdEnable(char *szRedeemAdjustPwdEnable);
int inGetRedeemAdjustPwd(char *szRedeemAdjustPwd);
int inGetVoidPwdEnable(char *szVoidPwdEnable);
int inGetVoidPwd(char *szVoidPwd);
int inGetSettlementPwdEnable(char *szSettlementPwdEnable);
int inGetSettlementPwd(char *szSettlementPwd);
int inGetRefundPwdEnable(char *szRefundPwdEnable);
int inGetRefundPwd(char *szRefundPwd);
int inGetHGRefundPwdEnable(char *szHGRefundPwdEnable);
int inGetHGRefundPwd(char *szHGRefundPwd);
int inGetCallBankForcePwdEnable(char *szCallBankForcePwdEnable);
int inGetCallBankForcePwd(char *szCallBankForceEnablePwd);

/* Get function */
int inSetInitialManagerEnable(char *szInitialManagerEnable);
int inSetInitialManagerPwd(char *szInitialManagerPwd);
int inSetRebootPwdEnale(char *szRebootPwdEnale);
int inSetRebootPwd(char *szRebootPwd);
int inSetSalePwdEnable(char *szSalePwdEnable);
int inSetSalePwd(char *szSalePwd);
int inSetPreauthPwdEnable(char *szPreauthPwdEnable);
int inSetPreauthPwd(char *szPreauthPwd);
int inSetInstallmentPwdEnable(char *szInstallmentPwdEnable);
int inSetInstallmentPwd(char *szInstallmentPwd);
int inSetRedeemPwdEnable(char *szRedeemPwdEnable);
int inSetRedeemPwd(char *szRedeemPwd);
int inSetMailOrderPwdEnable(char *szMailOrderPwdEnable);
int inSetMailOrderPwd(char *szMailOrderPwd);
int inSetOfflinePwdEnable(char *szOfflinePwdEnable);
int inSetOfflinePwd(char *szOfflinePwd);
int inSetInstallmentAdjustPwdEnable(char *szInstallmentAdjustPwdEnable);
int inSetInstallmentAdjustPwd(char *szInstallmentAdjustPwd);
int inSetRedeemAdjustPwdEnable(char *szRedeemAdjustPwdEnable);
int inSetRedeemAdjustPwd(char *szRedeemAdjustPwd);
int inSetVoidPwdEnable(char *szVoidPwdEnable);
int inSetVoidPwd(char *szVoidPwd);
int inSetSettlementPwdEnable(char *szSettlementPwdEnable);
int inSetSettlementPwd(char *szSettlementPwd);
int inSetRefundPwdEnable(char *szRefundPwdEnable);
int inSetRefundPwd(char *szRefundPwd);
int inSetHGRefundPwdEnable(char *szHGRefundPwdEnable);
int inSetHGRefundPwd(char *szHGRefundPwd);
int inSetCallBankForcePwdEnable(char *szCallBankForcePwdEnable);
int inSetCallBankForcePwd(char *szCallBankForceEnablePwd);

int inPWD_Edit_PWD_Table(void);

int inPWD_Test (void);
