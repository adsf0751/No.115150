/* 
 * File:   HG_Struct.h
 * Author: RussellBai
 *
 * Created on 2022年7月18日, 下午 6:29
 */

#ifndef HG_STRUCT_H
#define	HG_STRUCT_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct HG_Account_Total_File
{
	unsigned long    uslnRewardAmount;		/* 紅利積點 */
	unsigned long    uslnOnlineRedeemAmount;	/* 點數扣抵 */
	unsigned long    uslnOnlineRedeemPoint;		/* 點數扣抵 */
	unsigned long    uslnPointCertainAmount;	/* 加價購 */
	unsigned long    uslnPointCertainPoint;		/* 加價購 */
	unsigned long    uslnFullRedemptionPoint;	/* 點數兌換 */
	unsigned long    uslnRedeemRefundPoint;		/* 點數扣抵退貨 */
	unsigned long    uslnRewardRefundPoint;		/* 紅利積點退貨 */

	int              inRewardCnt;			/* 紅利積點 */
	int              inOnlineRedeemCnt;		/* 點數扣抵 */
	int              inPointCertainCnt;		/* 加價購 */
	int              inFullRedemptionCnt;		/* 點數兌換 */
	int              inRedeemRefundCnt;		/* 點數扣抵退貨 */
	int              inRewardRefundCnt;		/* 紅利積點退貨 */
} HG_ACCUM_TOTAL_REC;

#define _HG_ACCUM_REC_SIZE_	sizeof(HG_ACCUM_TOTAL_REC)

#ifdef	__cplusplus
}
#endif

#endif	/* HG_STRUCT_H */

