/* 
 * File:   Ftps_Struct.h
 * Author: RussellBai
 *
 * Created on 2024年5月23日, 下午 3:23
 */

#ifndef FTPS_STRUCT_H
#define	FTPS_STRUCT_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* FTPS_STRUCT_H */

typedef struct
{
        int     inFtpsPort;			/* FTPS的PORT */
	int	inFtpRetryTimes;		/* 要顯示的重試次數 */
        char    szFtpsURL[512 + 1];		/* FTPS的路徑 */
        char    szFtpsID[64 + 1];		/* FTPS的使用者名稱 */
        char    szFtpsPW[64 + 1];		/* FTPS的密碼 */
        char    szFtpsFileName[64 + 1];		/* 要儲存的檔案名稱  FTPS_file.txt轉換成其他檔名移動到fs_data 資料夾 */
	char	szCACertFileName[50 + 1];	/* 憑證名稱 */
	char	szCACertFilePath[100 + 1];	/* 憑證路徑 */
} FTPS_REC;

typedef struct FtpFile
{
	char	szFilename[100 + 1];
	FILE	*pStream;
} FTP_FILE;