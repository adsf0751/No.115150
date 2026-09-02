/* 
 * File:   ECCLib.h
 * Author: RussellBai
 *
 * Created on 2021年4月19日, 下午 5:36
 */

#ifndef ECCLIB_H
#define	ECCLIB_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* ECCLIB_H */

#define NCCC_ENCRYPTION

short ECC_NCCC_Encryption_TSAM_Lib(BYTE *bData, unsigned int *dataLen, BYTE fieldNo, BYTE *keyField37, BYTE *checksumField57, BYTE *keyIndex);
short ECC_NCCC_MAC_Lib(unsigned int msgType, BYTE *bData, unsigned int idxField3, unsigned idxField4, unsigned int idxField11, unsigned int idxField35, unsigned int idxField59, unsigned int idxField63, BYTE *MACData);