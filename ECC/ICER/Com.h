#ifndef __COM_H__
#define __COM_H__

#ifdef __cplusplus
		 extern "C" {
#endif

#if READER_MANUFACTURERS==LINUX_API
	#if RDEVN != NE_NEXSYS_V3
		typedef unsigned long DWORD;
		typedef unsigned long ULONG;
	#endif
#endif

	 int inComOpen(void);
	 int inComClose(void);
	 int inComSend(BYTE *lpBuf, unsigned long dwToWrite);
	 int inComRecv(BYTE *lpBuf, unsigned long dwToRead,clock_t tTimeout);
//#if READER_MANUFACTURERS!=LINUX_API && READER_MANUFACTURERS!=ANDROID_API
//#if (READER_MANUFACTURERS!=LINUX_API || RDEVN!=NE_X86) && READER_MANUFACTURERS!=ANDROID_API
	 int inComRecvByLen(BYTE *lpBuf,unsigned long dwToRead,clock_t tTimeout);
//#endif

	 int inDLEComRecvByLen(BYTE *lpBuf,unsigned long dwToRead,clock_t tTimeout);

#if READER_MANUFACTURERS==LINUX_API || READER_MANUFACTURERS==ANDROID_API
	 void finalizer_(void);
	 int recv_from(void* buf, size_t buf_size, float timeout, size_t* recv_size);
	 int recv_from2(BYTE* buf, clock_t timeout, int buf_size);
#endif
	int inReponseDefaultCkeck(BYTE *cOutputData);

	BOOL fGetUSBInfoByPar(int *xProxBusID,int *xProxDeviceID);
	int inCustomizrdReaderPorotocol(BOOL fEndPorotocol);

#ifdef __cplusplus
		 }
#endif // __cplusplus

#endif
