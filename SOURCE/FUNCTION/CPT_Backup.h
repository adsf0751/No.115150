#define _CPT_BACKUP_FILE_NAME_          "CPT_BACKUP.dat"
#define _CPT_BACKUP_BAK_FILE_NAME_	"CPT_BACKUP.bak"

/* Load & Save function */
int inLoadCPT_BackupRec(int inCPTRec);
int inSaveCPT_BackupRec(int inCPTRec);
int inCPT_Backup_Edit_CPT_Backup_Table(void);

/* Set function */
int inSetCommIndex_Backup(char* szCommIndex);
int inSetTPDU_Backup(char* szTPDU);
int inSetNII_Backup(char* szNII);
int inSetHostTelPrimary_Backup(char* szHostTelPrimary);
int inSetHostTelSecond_Backup(char* szHostTelSecond);
int inSetReferralTel_Backup(char* szReferralTel);
int inSetHostIPPrimary_Backup(char* szHostIPPrimary);
int inSetHostPortNoPrimary_Backup(char* szHostPortNoPrimary);
int inSetHostIPSecond_Backup(char* szHostIPSecond);
int inSetHostPortNoSecond_Backup(char* szHostPortNoSecond);
int inSetTCPHeadFormat_Backup(char* szTCPHeadFormat);
int inSetCarrierTimeOut_Backup(char* szCarrierTimeOut);
int inSetHostResponseTimeOut_Backup(char* szHostResponseTimeOut);

/* Get function */
int inGetCommIndex_Backup(char* szCommIndex);
int inGetTPDU_Backup(char* szTPDU);
int inGetNII_Backup(char* szNII);
int inGetHostTelPrimary_Backup(char* szHostTelPrimary);
int inGetHostTelSecond_Backup(char* szHostTelSecond);
int inGetReferralTel_Backup(char* szReferralTel);
int inGetHostIPPrimary_Backup(char* szHostIPPrimary);
int inGetHostPortNoPrimary_Backup(char* szHostPortNoPrimary);
int inGetHostIPSecond_Backup(char* szHostIPSecond);
int inGetHostPortNoSecond_Backup(char* szHostPortNoSecond);
int inGetTCPHeadFormat_Backup(char* szTCPHeadFormat);
int inGetCarrierTimeOut_Backup(char* szCarrierTimeOut);
int inGetHostResponseTimeOut_Backup(char* szHostResponseTimeOut);
