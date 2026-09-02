/* 
 * File:   NCCCsrc_Struct.h
 * Author: RussellBai
 *
 * Created on 2022年9月28日, 上午 10:57
 */

#ifndef NCCCSRC_STRUCT_H
#define	NCCCSRC_STRUCT_H

#ifdef	__cplusplus
extern "C" {
#endif

	



#ifdef	__cplusplus
}
#endif

#endif	/* NCCCSRC_STRUCT_H */

typedef struct
{
	long	lnTotalAmount;
	char	szDate[6 + 1];
	char	szTime[8 + 1];
	char	szCardNumber[19 + 1];
	char	szTransType[24 + 1];
}TotalAmtByCard;