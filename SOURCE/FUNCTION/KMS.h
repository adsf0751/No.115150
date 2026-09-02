/* 
 * File:   KMS.h
 * Author: user
 *
 * Created on 2017年7月6日, 上午 11:09
 */

/* Master key*/
#define _TMK_KEYSET_NCCC_		0xC000
#define _TMK_KEYSET_TAISHIN_		0xC001
#define _TMK_KEYSET_CUB_		0xC002

#define _TMK_KEYINDEX_NCCC_		0x0000	/* 0x0000放第一把key */
#define _TMK_KEYINDEX_CUB_		0x0000	/* 0x0000放第一把key */

/* 可將Production Key暫時換到這個位置 */
#define _TMK_TEMP_KEYSET_NCCC_		0xC000
#define _TMK_TEMP_KEYINDEX_NCCC_	0xF000	/* 0x0000放第一把key */

/* Working key */
#define _TWK_KEYSET_NCCC_			0x2000
#define _TWK_KEYSET_TAISHIN_			0x2001
#define _TWK_KEYSET_CUB_			0x2002

#define _TWK_KEYINDEX_NCCC_PIN_ONLINE_		0x0000
#define _TWK_KEYINDEX_NCCC_MAC_			0x0001
#define _TWK_KEYINDEX_NCCC_ESC_			0x0002
#define _TWK_KEYINDEX_NCCC_PIN_OFFLINE_		0x0003
#define _TWK_KEYINDEX_NCCC_PIN_MULTIFUNC_	0x0004
#define _TMK_KEYINDEX_TAISHIN_CMK_		0x0000	/* 0x0000放第一把key */
#define _TMK_KEYINDEX_TAISHIN_TMK_		0x0001	/* 0x0000放第一把key */
#define _TWK_KEYINDEX_TAISHIN_DUKPT_		0x0002
#define _TWK_KEYINDEX_CUB_PIN_ONLINE_		0x0000

typedef struct
{
	char	szKey[15][32 + 1];
} KEY_TABLE;

typedef struct
{
	char	szKCV[12 + 1];
} KCV_TABLE;

int inKMS_Initial (void);
int inKMS_Write(CTOS_KMS2KEYWRITE_PARA *stKeyWritePara);
int inKMS_DataEncrypt(CTOS_KMS2DATAENCRYPT_PARA *srDataEncryptPara);
int inKMS_DeleteKey(unsigned short usKeySet,unsigned short usKeyIndex);
int inKMS_DeleteAllKey(void);
int inKMS_CheckKey(unsigned short usKeySet,unsigned short usKeyIndex);
int inKMS_GetKeyInfo(CTOS_KMS2KEYGETINFO_PARA* para);
int inKMS_GetKeyInfo_LookUp(void);
int inKMS_Key_Swap(unsigned short usKeySet1, unsigned short usKeyIndex1, unsigned short usKeySet2, unsigned short usKeyIndex2);
int inKMS_Write_NULL_Key(unsigned short usKeySet, unsigned short usKeyIndex);
int inKMS_Check_NULL_Key(unsigned short usKeySet, unsigned short usKeyIndex, char *szNull);
