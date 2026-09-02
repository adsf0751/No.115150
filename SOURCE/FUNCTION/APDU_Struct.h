/* 
 * File:   APDU_Struct.h
 * Author: RussellBai
 *
 * Created on 2022年9月29日, 下午 8:18
 */

#ifndef APDU_STRUCT_H
#define	APDU_STRUCT_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* APDU_STRUCT_H */

#include "APDU_Define.h"

typedef struct
{
        int             inSendLen;
        int             inRecevLen;
	int             inCommandDataLen;				/* DATA 欄資料長度 */
        unsigned char   uszSendData[512 + 1];
        unsigned char   uszRecevData[512 + 1];
	unsigned char   uszCommandCLAData[2 + 1];			/* Instruction code */
        unsigned char   uszCommandINSData[2 + 1];			/* Instruction code */
        unsigned char   uszCommandP1Data[2 + 1];			/* Parameter 1 */
        unsigned char   uszCommandP2Data[2 + 1];			/* Parameter 2 */
        unsigned char   uszCommandData[_WRITE_RECORD_SIZE_ + 1];	/* 指令資料內容, 長度最大為Write Record Command時 */
} APDU_COMMAND;