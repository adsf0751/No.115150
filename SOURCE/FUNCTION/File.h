#define _SEEK_BEGIN_      d_SEEK_FROM_BEGINNING
#define _SEEK_CURRENT_    d_SEEK_FROM_CURRENT
#define _SEEK_END_        d_SEEK_FROM_EOF




int inFILE_Open(unsigned long *ulFileHandle, unsigned char *uszFileName);
int inFILE_OpenReadOnly(unsigned long *ulFileHandle, unsigned char *uszFileName);
int inFILE_Create(unsigned long *ulFileHandle, unsigned char *uszFileName);
int inFILE_Read(unsigned long *ulFileHandle, unsigned char *uszReadData, unsigned long ulReadSize);
int inFILE_Seek(unsigned long ulFileHandle, unsigned long ulOffset, int inSeekMode);
int inFILE_Write(unsigned long *ulFileHandle, unsigned char *uszWriteData, unsigned long ulWriteSize);
int inFILE_Delete(unsigned char *uszFileName);
int inFILE_Close(unsigned long *ulFileHandle);
int inFILE_Rename(unsigned char *uszOldFileName, unsigned char *uszNewFileName);
int inFILE_Check_Exist(unsigned char *uszFileName);
long lnFILE_GetSize(unsigned long *ulFileHandle, unsigned char *uszFileName);
int inFILE_Cut(unsigned char* uszFileName, unsigned long ulFileSize);
int inFILE_Data_Delete(unsigned char *uszFileName, int inOffset, int inFileLen);
int inFILE_Data_Insert(unsigned char *uszFileName, int inOffset, int inFileLen, unsigned char *uszInsertData);
int inFILE_Copy_File(unsigned char *uszOldFileName, unsigned char *uszNewFileName);

int inFILE_Test(void);

int inFile_Linux_Open(int* inFd, char* szFileName);
int inFile_Linux_Close(int inFd);
int inFile_Linux_Create(int* inFd, char* szFileName);
int inFile_Linux_Read(int inFd, char* szBuf, int* inCnt);
int inFile_Linux_Write(int inFd, char* szBuf, int* inCnt);
int inFile_Linux_Seek(int inFd, int inOffset, int inMode);
int inFile_Linux_Get_FileSize(int inFd, int* inFileSize);
int inFile_Linux_Get_FileSize_By_LSeek(int inFd, int* inFileSize);
int inFile_Linux_Get_FileSize_By_Stat(char* szPath, int* inFileSize);
int inFile_Linux_Open_In_Fs_Data(int* inFd, char* szFileName);
int inFile_Linux_Create_In_Fs_Data(int* inFd, char* szFileName);
int inFile_Linux_Delete_In_Fs_Data(char* szFileName);
int inFile_Linux_Rename_In_Fs_Data(char *szOldFileName, char *szNewFileName);
int inFile_Unlink_File(char *szFileName, char* szSource);
int inFile_Move_File(char *szSrcFileName, char* szSource, char* szDesFileName, char* szDestination);
int inFile_Linux_Check_File_Exist(char* szPath, char* szFileName);

int inFile_Open_File_Cnt_Increase(void);
int inFile_Open_File_Cnt_Decrease(void);
int inFile_Open_File_Cnt_Return(int* inFileOpenCnt);
