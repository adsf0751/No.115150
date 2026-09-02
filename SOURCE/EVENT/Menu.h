/* 用以確認功能是否有開 */
typedef	struct	MENU_CHECK_TABLE
{
	int		inButtonPositionID;	/* 按鈕位置 */
	int		inCode;			/* 交易別 */
	int		(*inCheckFunc)(int)	/* 用來檢查的function */;
	char		szFileName[100 + 1];	/* 要反白的圖 */
} MENU_CHECK_TABLE;

#define	_MENU_CHECK_FORCE_DISPLAY_NONE_	0
#define	_MENU_CHECK_FORCE_DISPLAY_ON_	1
#define	_MENU_CHECK_FORCE_DISPLAY_OFF_	2

typedef	struct	MENU_CHECK_2_TABLE_
{
	int		inButtonPositionID;	/* 按鈕位置 */
	int		inCode;			/* 交易別 */
	int		inForceDisplay;		/* 0:不強制 1:是強制On 2是強制Off */
	int		(*inCheckFunc)(int)	/* 用來檢查的function，inForceDisplay = 0時生效 */;
	char		szLine1Msg[15 + 1];
	unsigned long	ulLine1FontSize;
	int		inLine1X;
	int		inLine1Y;
	unsigned char	uszLine1ReverseBit;
	char		szLine2Msg[15 + 1];
	unsigned long	ulLine2FontSize;
	int		inLine2X;
	int		inLine2Y;
	unsigned char	uszLine2ReverseBit;
	char		szLine3Msg[15 + 1];
	unsigned long	ulLine3FontSize;
	int		inLine3X;
	int		inLine3Y;
	unsigned char	uszLine3ReverseBit;
} MENU_CHECK_2_TABLE_;

#define _PAGE_1_		1
#define _PAGE_2_		2
#define _PAGE_3_		3
#define _PAGE_4_		4
#define _PAGE_5_		5
#define _PAGE_6_		6
#define _PAGE_7_		7
#define _PAGE_8_		8
#define _PAGE_9_		9
#define _PAGE_10_		10

/*for Touch*/
#define _TOUCH_PAGE_1_		1
#define _TOUCH_PAGE_2_		2
#define _TOUCH_PAGE_3_		3
#define _TOUCH_PAGE_4_		4
#define _TOUCH_PAGE_5_		5
#define _TOUCH_PAGE_6_		6
#define _TOUCH_PAGE_7_		7
#define _TOUCH_PAGE_8_		8
#define _TOUCH_PAGE_9_		9
#define _TOUCH_PAGE_10_		10
#define _TOUCH_PAGE_11_		11
#define _TOUCH_PAGE_12_		12
#define _TOUCH_PAGE_13_		13

#define _MENU_000_NORMAL_ 0

/* 在第幾層迴圈 */
#define _PAGE_LOOP_0_		0	/* 選定就設成0 */
#define _PAGE_LOOP_1_		1
#define _PAGE_LOOP_2_		2
#define _PAGE_LOOP_3_		3
#define _PAGE_LOOP_4_		4

/* Idle UI*/
int inMENU_Decide_Idle_Menu(void);
int inMENU_Old_UI_V3UL(void);
int inMENU_New_UI_MP200(void);
int inMENU_New_UI(void);
int inMENU_Load_Key_UI(void);

/* 功能UI */
int inMENU_000_MenuFlow(EventMenuItem *srEventMenuItem);
int inMENU_000_MenuFlow_V3UL(EventMenuItem *srEventMenuItem);
int inMENU_000_MenuFlow_MP200(EventMenuItem *srEventMenuItem);
int inMENU_000_MenuFlow_NEWUI(EventMenuItem *srEventMenuItem);
int inMENU_000_MenuFlow_LoadKeyUI(EventMenuItem *srEventMenuItem);
int inMENU_SELECT_KEY_F3(EventMenuItem *srEventMenuItem);
int inMENU_SELECT_KEY_FUNC(EventMenuItem *srEventMenuItem);
int inMENU_Swipe(EventMenuItem *srEventMenuItem);
int inMENU_MenuKeyInAndGetAmount(EventMenuItem *srEventMenuItem);
int inMENU_ICC(EventMenuItem *srEventMenuItem);
int inMENU_ECR(EventMenuItem *srEventMenuItem);
int inMENU_ECR_OPERATION_LOAD_KEY_FROM_520(EventMenuItem *srEventMenuItem);
int inMENU_ECR_OPERATION_LOAD_KEY_FROM_KEY_CARD(EventMenuItem *srEventMenuItem);
int inMENU_MULTIFUNC_SLAVE(EventMenuItem *srEventMenuItem);
int inMENU_ESC_IDLE_UPLOAD(EventMenuItem *srEventMenuItem);
int inMENU_DCC_SCHEDULE(EventMenuItem *srEventMenuItem);
int inMENU_TMS_DCC_SCHEDULE(EventMenuItem *srEventMenuItem);
int inMENU_DCC_HOUR_NOTIFY_(EventMenuItem *srEventMenuItem);
int inMENU_TMS_SCHEDULE_INQUIRE(EventMenuItem *srEventMenuItem);
int inMENU_TMS_SCHEDULE_DOWNLOAD(EventMenuItem *srEventMenuItem);
int inMENU_TMS_PROCESS_EFFECTIVE(EventMenuItem *srEventMenuItem);
int inMENU_EDC_BOOTING(EventMenuItem *srEventMenuItem);
int inMENU_POWER_MANAGEMENT(EventMenuItem *srEventMenuItem);
int inMENU_Check_Transaction_Enable(int inCode);
int inMENU_Check_SETTLE_Enable(int inCode);
int inMENU_Check_REVIEW_SETTLE_Enable(int inCode);
int inMENU_Check_Adjust(int inCode);
int inMENU_Check_CUP_Enable(int inCode);
int inMENU_Check_SMARTPAY_Enable(int inCode);
int inMENU_Check_ETICKET_Enable(int inCode);
int inMENU_Check_AWARD_Enable(int inCode);
int inMENU_Check_HG_Enable(int inCode);
int inMENU_Check_Mark_Enable(int inCode);
int inMENU_Display_ICON(char* szFileName, int inButtonPostionID);
int inMENU_CHECK_FUNCTION_ENABLE_DISPLAY(MENU_CHECK_TABLE *srMENU_CHECK_TABLE);
int inMENU_FUNCTION_NEWUI_PAGE1(EventMenuItem *srEventMenuItem);
int inMENU_FUNCTION_TRUST_UI_PAGE(EventMenuItem *srEventMenuItem);/* [113110-信託需求][UI] 畫面需依照信託功能是否開啟變動 2025/11/20  */
int inMENU_FUNCTION_NEWUI_PAGE2(EventMenuItem *srEventMenuItem);


/* 子選單 */
int inMENU_FUN3_COMM_TIME_SET(EventMenuItem *srEventMenuItem);
int inMENU_FUN5_TMS(EventMenuItem *srEventMenuItem);
int inMENU_FUN6_SELECT(EventMenuItem *srEventMenuItem);
int inMENU_Loyalty_Redeem(EventMenuItem *srEventMenuItem);
int inMENU_AWARD_SWIPE(EventMenuItem *srEventMenuItem);
int inMENU_AWARD_BARCODE(EventMenuItem *srEventMenuItem);
int inMENU_AWARD_VOID(EventMenuItem *srEventMenuItem);
int inMENU_DCC_RATE(EventMenuItem *srEventMenuItem);
int inMENU_Download_Parameter(EventMenuItem *srEventMenuItem);
int inMENU_VOID(EventMenuItem *srEventMenuItem);
int inMENU_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_CREDIT_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_REFUND_NORMAL(EventMenuItem *srEventMenuItem);
int inMENU_REFUND_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_REFUND_INST(EventMenuItem *srEventMenuItem);
int inMENU_HG_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_HG_REWARD_REDUND(EventMenuItem *srEventMenuItem);
int inMENU_HG_REDEEM_REDUND(EventMenuItem *srEventMenuItem);
int inMENU_INST_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_INST(EventMenuItem *srEventMenuItem);
int inMENU_SETTLE(EventMenuItem *srEventMenuItem);
int inMENU_SALEOFFLINE(EventMenuItem *srEventMenuItem);
int inMENU_TIP(EventMenuItem *srEventMenuItem);
int inMENU_PREAUTH(EventMenuItem *srEventMenuItem);
int inMENU_MAILORDER(EventMenuItem *srEventMenuItem);
int inMENU_SALE(EventMenuItem *srEventMenuItem);
int inMENU_ADJUST(EventMenuItem *srEventMenuItem);
int inMENU_REDEEM_ADJUST(EventMenuItem *srEventMenuItem);
int inMENU_INST_ADJUST(EventMenuItem *srEventMenuItem);
int inMENU_FISC_MENU(EventMenuItem *srEventMenuItem);
int inMENU_FISC_SALE(EventMenuItem *srEventMenuItem);
int inMENU_FISC_VOID(EventMenuItem *srEventMenuItem);
int inMENU_FISC_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_HG_MENU(EventMenuItem *srEventMenuItem);
int inMENU_HG_REWARD(EventMenuItem *srEventMenuItem);
int inMENU_HG_ONLINE_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_HG_POINT_CERTAIN(EventMenuItem *srEventMenuItem);
int inMENU_HG_FULL_REDEMPTION(EventMenuItem *srEventMenuItem);
int inMENU_HG_INQUIRY(EventMenuItem *srEventMenuItem);
int inMENU_SETTLE_AUTOSETTLE(EventMenuItem *srEventMenuItem);
int inMENU_SETTLE_BY_HOST(EventMenuItem *srEventMenuItem);
int inMENU_CUP_SALE(EventMenuItem *srEventMenuItem);
int inMENU_CUP_VOID(EventMenuItem *srEventMenuItem);
int inMENU_CUP_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_CUP_REFUND_NORMAL(EventMenuItem *srEventMenuItem);
int inMENU_CUP_REFUND_MAILORDER(EventMenuItem *srEventMenuItem);
int inMENU_CUP_PREAUTH(EventMenuItem *srEventMenuItem);
int inMENU_CUP_MAILORDER(EventMenuItem *srEventMenuItem);
int inMENU_CUP_INST_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_CUP_INST(EventMenuItem *srEventMenuItem);
int inMENU_CUP_REDEEM(EventMenuItem *srEventMenuItem);
int inMENU_REPRINT(EventMenuItem *srEventMenuItem);
int inMENU_REVIEW(EventMenuItem *srEventMenuItem);
int inMENU_TOTAL_REVIEW(EventMenuItem *srEventMenuItem);
int inMENU_DETAIL_REVIEW(EventMenuItem *srEventMenuItem);
int inMENU_TMS_PARAMETER_PRINT(EventMenuItem *srEventMenuItem);
int inMENU_DCC_PARAMETER_PRINT(EventMenuItem *srEventMenuItem);
int inMENU_TOTAL_REPORT(EventMenuItem *srEventMenuItem);
int inMENU_DETAIL_REPORT(EventMenuItem *srEventMenuItem);
int inMENU_SAM_REGISTER(EventMenuItem *srEventMenuItem);
int inMENU_EDIT_PASSWORD(EventMenuItem *srEventMenuItem);
int inMENU_TRACELOG_UP(EventMenuItem *srEventMenuItem);
int inMENU_TRACELOG_PRINT(EventMenuItem *srEventMenuItem);
int inMENU_CHECK_VERSION(EventMenuItem *srEventMenuItem);
int inMENU_COMM_SETTING(EventMenuItem *srEventMenuItem);
int inMENU_TIME_SETTING(EventMenuItem *srEventMenuItem);
int inMENU_TMS_PARAMETER_DOWNLOAD(EventMenuItem *srEventMenuItem);
int inMENU_DCC_PARAMETER_DOWNLOAD(EventMenuItem *srEventMenuItem);
int inMENU_TMS_TASK_REPORT(EventMenuItem *srEventMenuItem);
int inMENU_DELETE_BATCH(EventMenuItem *srEventMenuItem);
int inMENU_DEBUG_SWITCH(EventMenuItem *srEventMenuItem);
int inMENU_EDIT_TMEP_VERSION_ID(EventMenuItem *srEventMenuItem);
int inMENU_EDIT_TMSOK(EventMenuItem *srEventMenuItem);
int inMENU_UNLOCK_EDC(EventMenuItem *srEventMenuItem);
int inMENU_REBOOT(EventMenuItem *srEventMenuItem);
int inMENU_KEY_FUNCTION(EventMenuItem *srEventMenuItem);
int inMENU_TMK_Menu(EventMenuItem *srEventMenuItem);
int inMENU_TMK_By_KeyCard(EventMenuItem *srEventMenuItem);
int inMENU_EXIT_AP(EventMenuItem *srEventMenuItem);
int inMENU_CHECK_FILE(EventMenuItem *srEventMenuItem);
int inMENU_CHECK_FILE_In_SD(EventMenuItem *srEventMenuItem);
int inMENU_CHECK_FILE_In_USB(EventMenuItem *srEventMenuItem);
int inMENU_Engineer_Fuction(EventMenuItem *srEventMenuItem);
int inMENU_Developer_Fuction_Flow(EventMenuItem *srEventMenuItem);
int inMENU_Developer_Fuction(EventMenuItem *srEventMenuItem);
int inMENU_Developer_Fuction2(EventMenuItem *srEventMenuItem);
int inMENU_Developer_Fuction3(EventMenuItem *srEventMenuItem);
int inMENU_Developer_Fuction4(EventMenuItem *srEventMenuItem);
int inMENU_Developer_Fuction5(EventMenuItem *srEventMenuItem);
int inMENU_Developer_Fuction6(EventMenuItem *srEventMenuItem);
int inMENU_Developer_Fuction7(EventMenuItem *srEventMenuItem);
int inMENU_Developer_Fuction8(EventMenuItem *srEventMenuItem);
int inMENU_ESVC_AUTO_SIGNON(EventMenuItem *srEventMenuItem);
int inMENU_COMM_MENU(EventMenuItem *srEventMenuItem);
int inMENU_DEMO_SETTING(EventMenuItem *srEventMenuItem, VS_BOOL fSuperEntry);
int inMENU_QA_Fuction_Flow(EventMenuItem *srEventMenuItem);
int inMENU_QA_Fuction(EventMenuItem *srEventMenuItem);
int inMENU_EditPWD_Flow_Cus_075(void);
int inMENU_Duty_Free_075_PWD(void);
int inMENU_CHECK_2_FUNCTION_ENABLE_DISPLAY(MENU_CHECK_2_TABLE_ *srMENU_CHECK_TABLE);
int inMENU_MenuKeyInAndGetAmount_041(EventMenuItem *srEventMenuItem);
int inMENU_CASH_ADVANCE(EventMenuItem *srEventMenuItem);
int inMENU_FORCE_CASH_ADVANCE(EventMenuItem *srEventMenuItem);
int inMENU_TRACELOG_MENU(EventMenuItem *srEventMenuItem);
int inMENU_CHECK_FILE_Flow(EventMenuItem *srEventMenuItem);
int inMENU_CHECK_Nexsys_Log(EventMenuItem *srEventMenuItem);
void vdMENU_ReNew_Idle_Time(void);

int inMENU_NEWUI_TRANSACTION_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_CUP_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_SMARTPAY_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_ETICKET_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_AWARD_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_HAPPYGO_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_SETTLE_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_REVIEW_PRINT_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_SETTING_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_OTHER_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_FUNCTION1_MENU(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_FUNCTION2_MENU(EventMenuItem *srEventMenuItem);
int inMENU_FUNCTION_NEWUI_PAGE_Flow(EventMenuItem *srEventMenuItem);
int inMENU_NEWUI_NEWUI_TRUST_MENU(EventMenuItem *srEventMenuItem);
int inMENU_Trust_Sale(EventMenuItem *srEventMenuItem);
int inMENU_Trust_Void(EventMenuItem *srEventMenuItem);
int inMENU_Trust_Reprint(EventMenuItem *srEventMenuItem);
int inMENU_Check_Trust_Enable(int inCode);