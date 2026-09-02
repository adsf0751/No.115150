/* 
 * File:   Utility.h
 * Author: RussellBai
 *
 * Created on 2019年4月23日, 下午 4:40
 */

#define MAX_TOTAL_LOG_SIZE (1024 * 1024 * 10)  // 10 MB
#define LOG_FSYNC_INTERVAL 50 // fsync every N writes
#define SIZE_CHECK_INTERVAL 100
#define _AP_PUB_LOG_PATH_ "/home/ap/pub/sysfinlog/"
#define _AP_PUB_LOG_DIR_NAME_	"sysfinlog"

/* 確保結構體以 1 byte 對齊，避免編譯器自動補齊導致解析錯誤 */
#pragma pack(push, 1)
typedef struct {
    unsigned short bfType;      /* 檔案類型，必須為 'BM' (0x4D42) */
    unsigned int   bfSize;      /* 檔案大小 */
    unsigned short bfReserved1; /* 保留字，必須為 0 */
    unsigned short bfReserved2; /* 保留字，必須為 0 */
    unsigned int   bfOffBits;   /* 像素資料的起始偏移量 */
} BMPFILEHEADER;

typedef struct {
    unsigned int   biSize;          /* 資訊標頭大小，通常為 40 */
    int            biWidth;         /* 圖片寬度 */
    int            biHeight;        /* 圖片高度 (正數表示由下到上) */
    unsigned short biPlanes;        /* 必須為 1 */
    unsigned short biBitCount;      /* 每像素位元數，此為 24 */
    unsigned int   biCompression;   /* 壓縮類型，0 表示不壓縮 */
    unsigned int   biSizeImage;     /* 像素資料大小 */
    int            biXPelsPerMeter; /* 水平解析度 */
    int            biYPelsPerMeter; /* 垂直解析度 */
    unsigned int   biClrUsed;       /* 使用的顏色數，通常為 0 */
    unsigned int   biClrImportant;  /* 重要的顏色數，通常為 0 */
} BMPINFOHEADER;
#pragma pack(pop)


/**
 * 為了支援個別設定寬高，我們定義一個設定用結構體
 */
typedef struct {
    const char* filename;
    int targetWidth;   /* 指定輸出的寬度，若填 0 則代表維持原圖寬度 */
    int targetHeight;  /* 指定輸出的高度，若填 0 則代表維持原圖高度 */
} MergeBMPItem;

#define MAX_BMP_WIDTH     4096
#define MAX_BMP_HEIGHT    4096
#define BIN_THRESHOLD     180   /* 1-bit 列印文字加粗門檻值 (127~220，越大越粗) */


int inUtility_OpenTraceLogFile(void);		//2019/3/12 下午 4:42
int inUtility_CloseTraceLogFile(void);		//2019/3/12 下午 4:42
int inUtility_StoreTraceLog(char *szMsg); 	//2019.02.11 added by Hachi
int inUtility_ClearFile (void);			//2019/2/26 上午 9:03
int inUtility_StoreTraceLog_OneStep(char *szMsg, ...);	/* 2019/4/8 下午 5:20 added by Russell */
void vdUtility_SYSFIN_OpenLogFile(void);
void vdUtility_SYSFIN_CloseLogFile(void);
void vdUtility_SYSFIN_LogMessage(char* szLocation, const char *format, ...);
void vdUtility_SYSFIN_ManageLogFileSize(void);
int inUtility_SYSFIN_GetTotalLogSize(void);
void vdUtility_SYSFIN_DeleteOldestLogFile(void);
void vdUtility_SYSFIN_get_log_filename(char *szFilename, size_t size);
void vdUtility_SYSFIN_check_log_date(void);
int inUtility_TryDelete(const char *path);

unsigned char* pszUtility_SYSFIN_ResizeBMP24Data(const unsigned char* inData, int inWidth, int inHeight, int outWidth, int outHeight);
unsigned char* pszUtility_SYSFIN_Convert24BitTo1Bit(const unsigned char* in24Bit, int width, int height);
unsigned char* pszUtility_SYSFIN_ReadBMP_AutoNormalized24(const char* filename, BMPFILEHEADER* fh, BMPINFOHEADER* ih);
int inUtility_SYSFIN_MergeAndScaleMultipleBMPUniversal(const MergeBMPItem* itemList, int itemCount, const char* outFile, int outBitCount);