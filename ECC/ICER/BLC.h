#ifndef __BLC_H__
#define __BLC_H__


#if READER_MANUFACTURERS!=NE_PAXSXX && READER_MANUFACTURERS!=CS_PAXSXX && READER_MANUFACTURERS!=NE_SYMLINK_API
#pragma pack(push,1)
#pragma pack(1)
#endif

typedef struct __packed_back BLC_HEADER
{
	BYTE ucFileHead[4];
	BYTE ucFileData[8];
	BYTE ucFileTime[6];
	BYTE ucSISVerNo;
	BYTE ucSISSubNo;
	BYTE ucBlockingIDFlag;
	BYTE ucFileEnd[4];
} BLC_HEADER;

typedef struct BLC_DATA
{
	BYTE ucCardID[8];
	BYTE Reason;
} BLC_DATA;

typedef struct BLC_END
{
	BYTE ucFileTail[4];
	BYTE ucCheckCode;
	BYTE ucFileHash[16];
	BYTE ucDataCnt[8];
	BYTE ucFileEnd[3];
} BLC_END;

#if READER_MANUFACTURERS!=NE_PAXSXX && READER_MANUFACTURERS!=CS_PAXSXX && READER_MANUFACTURERS!=NE_SYMLINK_API
#pragma pack(pop)
#endif

	int compare(const void *arg1, const void *arg2);
	int compare2(const void *a, const void *b);
	int fnMaintainBLC(void);
	char fIsCheckBLC(int inTxnType,BYTE *bAPDUOut);
	int inCheckBLC(BYTE *ucCardID);
	int inBLCLockCard(int inTxnType,BYTE *cOutputData,BYTE *CardID,BYTE *TxnDateTime);

#if READER_MANUFACTURERS!=WINDOWS_API && READER_MANUFACTURERS!=LINUX_API && READER_MANUFACTURERS!=ANDROID_API && READER_MANUFACTURERS!=WINDOWS_CE_API
	short ECC_file_read(char* i_pc_filename, unsigned char* o_puc_buffer, int i_l_offset, int *i_l_length);
	short file_read2(char* i_pc_filename, unsigned char* o_puc_buffer, int i_l_offset, int *i_l_length);
	short IsInlist(char* i_pc_filename,BYTE *ucCardID);
	void vdChangeEndian(BYTE *bInData, BYTE *bOutData, int inInDataLen);
#endif

#endif

#if READER_MANUFACTURERS==WINDOWS_CE_API
void * __cdecl bsearch ( const void *key, const void *base, size_t num, size_t width, int (__cdecl *compare)(const void *, const void *) );
#endif
