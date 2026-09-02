typedef struct
{
        char    szContactlessSchemeIndex[2 + 1];                        /* 各非接觸式卡片索引 */
        char    szVWTTerminalType[2 + 1];				/* Terminal Type (paywave wave2 only)*/
        char    szCVMRequirement[2 + 1];                                /* CVM Requirement */
        char    szVLPSupportIndicator[2 + 1];                           /* VLP Support Indicator */
        char    szTerminalCountryCode[4 + 1];                           /* Terminal Country Code */
        char    szVWTTransactionCurrencyCode[4 + 1];			/* Transaction Currency Code */
        char    szTransactionType[2 + 1];                               /* Transaction Type */
        char    szContactlessTransactionLimit[12 + 1];                  /* Contactless Transaction Limit */       
        char    szCVMRequiredLimit[12 + 1];                             /* CVM Required Limit */     
        char    szContactlessFloorLimit[12 + 1];                        /* Contactless Floor Limit */
        char    szEnhancedDDAVersionNum[2 + 1];                         /* Enhanced DDA Version Num */
        char    szDisplayOfflineFunds[2 + 1];                           /* Display Offline Funds */
        char    szTerminalTransactionQualifier[8 + 1];                  /* Terminal Transaction Qualifier */
        char    szPaypassTerminalType[2 + 1];                           /* Paypass Terminal Type */
        char    szPaypassTerminalCapabilities[6 + 1];                   /* Paypass Terminal Capabilities (paypass、NewJspeedy、D-PAS、NCCC)*/
        char    szPaypassTerminalCountryCode[4 + 1];                    /* Paypass Terminal Country Code */
        char    szPaypassTransactionCurrencyCode[4 + 1];                /* Paypass Transaction Currency Code */
        char    szPaypassDefaultTAC[10 + 1];                            /* Paypass Default TAC */
        char    szPaypassDenialTAC[10 + 1];                             /* Paypass Denial TAC */
        char    szPaypassOnlineTAC[10 + 1];                             /* Paypass Online TAC */
        char    szPaypassDefaultTDOL[6 + 1];                            /* Paypass Default TDOL */
        char    szPaypassEMVFloorLimit[12 + 1];                         /* Paypass EMV Floor Limit */
        char    szPaypassRandomSelectionThreshold[12 + 1];              /* Paypass Random Selection Threshold */
        char    szPaypassTargetPercentforRandomSelection[2 + 1];        /* Paypass Target Percent for Random Selection */
        char    szPaypassMaxTargetPercentforRandomSelection[2 + 1];     /* Paypass Max Target Percent for Random Selection */ 
        char    szPaypassCVMRequiredLimit[12 + 1];                      /* Paypass CVM Required Limit */
        char    szVWTMerchantCategoryCode[4 + 1];                          /* Merchant Category Code */
        char    szVWTTransactionCategoryCode[1 + 1];                       /* Transaction Category Code */
        char    szCombinationOption[4 + 1];                             /* Combination Option */
        char    szTerminalInterchangeProfile[6 + 1];                    /* Terminal Interchange Profile */
        char    szPayWaveAPID[32 + 1];                                  /* PayWave AP ID */
        char    szPayWaveAPIDContactlessTransactionLimit[12 + 1];       /* PayWave AP ID Contactless Transaction Limit */
        char    szPayWaveAPIDCVMRequiredLimit[12 + 1];                  /* PayWave AP ID CVM Required Limit */
        char    szPayWaveAPIDContactlessFloorLimit[12 + 1];             /* PayWave AP ID Contactless Floor Limit */
} VWT_REC;


/* 以欄位數決定 ex:欄位數是34個，理論上會有33個comma和兩個byte的0x0D 0X0A */
#define _SIZE_VWT_COMMA_0D0A_   35
#define _SIZE_VWT_REC_          (sizeof(VWT_REC))
#define _VWT_FILE_NAME_         "VWT.dat"
#define _VWT_FILE_NAME_BAK_     "VWT.bak"

#define _PAYWAVE_VWT_INDEX_		0
#define _JSPEEDY_VWT_INDEX_		1
#define _PAYPASS_VWT_INDEX_		2
#define _QP_DEBIT_VWT_INDEX_		3
#define _QP_CREDIT_VWT_INDEX_		4
#define _QP_QUASI_CREDIT_VWT_INDEX_	5
#define _AE_EXPRESSPAY_VWT_INDEX_	6
#define _NCCC_VWT_INDEX_		7
#define _D_PAS_VWT_INDEX_		8

/* Load & Save function */
int inLoadVWTRec(int inVWTRec);
int inSaveVWTRec(int inVWTRec);
int inVWT_Edit_VWT_Table(void);

/* Set function */
int inSetContactlessSchemeIndex(char* szContactlessSchemeIndex);
int inSetVWTTerminalType(char* szTerminalType);
int inSetCVMRequirement(char* szCVMRequirement);
int inSetVLPSupportIndicator(char* szVLPSupportIndicator);
int inSetTerminalCountryCode(char* szTerminalCountryCode);
int inSetVWTTransactionCurrencyCode(char* szTransactionCurrencyCode);
int inSetTransactionType(char* szTransactionType);
int inSetContactlessTransactionLimit(char* szContactlessTransactionLimit);
int inSetCVMRequiredLimit(char* szCVMRequiredLimit);
int inSetContactlessFloorLimit(char* szContactlessFloorLimit);
int inSetEnhancedDDAVersionNum(char* szEnhancedDDAVersionNum);
int inSetDisplayOfflineFunds(char* szDisplayOfflineFunds);
int inSetTerminalTransactionQualifier(char* szTerminalTransactionQualifier);
int inSetPaypassTerminalType(char* szPaypassTerminalType);
int inSetPaypassTerminalCapabilities(char* szPaypassTerminalCapabilities);
int inSetPaypassTerminalCountryCode(char* szPaypassTerminalCountryCode);
int inSetPaypassTransactionCurrencyCode(char* szPaypassTransactionCurrencyCode);
int inSetPaypassDefaultTAC(char* szPaypassDefaultTAC);
int inSetPaypassDenialTAC(char* szPaypassDenialTAC);
int inSetPaypassOnlineTAC(char* szPaypassOnlineTAC);
int inSetPaypassDefaultTDOL(char* szPaypassDefaultTDOL);
int inSetPaypassEMVFloorLimit(char* szPaypassEMVFloorLimit);
int inSetPaypassRandomSelectionThreshold(char* szPaypassRandomSelectionThreshold);
int inSetPaypassTargetPercentforRandomSelection(char* szPaypassTargetPercentforRandomSelection);
int inSetPaypassMaxTargetPercentforRandomSelection(char* szPaypassMaxTargetPercentforRandomSelection);
int inSetPaypassCVMRequiredLimit(char* szPaypassCVMRequiredLimit);
int inSetVWTMerchantCategoryCode(char* szMerchantCategoryCode);
int inSetVWTTransactionCategoryCode(char* szTransactionCategoryCode);
int inSetCombinationOption(char* szCombinationOption);
int inSetTerminalInterchangeProfile(char* szTerminalInterchangeProfile);
int inSetPayWaveAPID(char* szPayWaveAPID);
int inSetPayWaveAPIDContactlessTransactionLimit(char* szPayWaveAPIDContactlessTransactionLimit);
int inSetPayWaveAPIDCVMRequiredLimit(char* szPayWaveAPIDCVMRequiredLimit);
int inSetPayWaveAPIDContactlessFloorLimit(char* szPayWaveAPIDContactlessFloorLimit);



/* Get function */
int inGetContactlessSchemeIndex(char* szContactlessSchemeIndex);
int inGetVWTTerminalType(char* szTerminalType);
int inGetCVMRequirement(char* szCVMRequirement);
int inGetVLPSupportIndicator(char* szVLPSupportIndicator);
int inGetTerminalCountryCode(char* szTerminalCountryCode);
int inGetVWTTransactionCurrencyCode(char* szTransactionCurrencyCode);
int inGetTransactionType(char* szTransactionType);
int inGetContactlessTransactionLimit(char* szContactlessTransactionLimit);
int inGetCVMRequiredLimit(char* szCVMRequiredLimit);
int inGetContactlessFloorLimit(char* szContactlessFloorLimit);
int inGetEnhancedDDAVersionNum(char* szEnhancedDDAVersionNum);
int inGetDisplayOfflineFunds(char* szDisplayOfflineFunds);
int inGetTerminalTransactionQualifier(char* szTerminalTransactionQualifier);
int inGetPaypassTerminalType(char* szPaypassTerminalType);
int inGetPaypassTerminalCapabilities(char* szPaypassTerminalCapabilities);
int inGetPaypassTerminalCountryCode(char* szPaypassTerminalCountryCode);
int inGetPaypassTransactionCurrencyCode(char* szPaypassTransactionCurrencyCode);
int inGetPaypassDefaultTAC(char* szPaypassDefaultTAC);
int inGetPaypassDenialTAC(char* szPaypassDenialTAC);
int inGetPaypassOnlineTAC(char* szPaypassOnlineTAC);
int inGetPaypassDefaultTDOL(char* szPaypassDefaultTDOL);
int inGetPaypassEMVFloorLimit(char* szPaypassEMVFloorLimit);
int inGetPaypassRandomSelectionThreshold(char* szPaypassRandomSelectionThreshold);
int inGetPaypassTargetPercentforRandomSelection(char* szPaypassTargetPercentforRandomSelection);
int inGetPaypassMaxTargetPercentforRandomSelection(char* szPaypassMaxTargetPercentforRandomSelection);
int inGetPaypassCVMRequiredLimit(char* szPaypassCVMRequiredLimit);
int inGetVWTMerchantCategoryCode(char* szMerchantCategoryCode);
int inGetVWTTransactionCategoryCode(char* szTransactionCategoryCode);
int inGetCombinationOption(char* szCombinationOption);
int inGetTerminalInterchangeProfile(char* szTerminalInterchangeProfile);
int inGetPayWaveAPID(char* szPayWaveAPID);
int inGetPayWaveAPIDContactlessTransactionLimit(char* szPayWaveAPIDContactlessTransactionLimit);
int inGetPayWaveAPIDCVMRequiredLimit(char* szPayWaveAPIDCVMRequiredLimit);
int inGetPayWaveAPIDContactlessFloorLimit(char* szPayWaveAPIDContactlessFloorLimit);
