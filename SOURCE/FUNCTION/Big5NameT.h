/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Big5NameT.h
 * Author: 1500414
 *
 * Created on 2025年5月21日, 下午 1:42
 */

#ifndef BIG5NAMET_H
#define BIG5NAMET_H

#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif /* BIG5NAMET_H */

typedef struct TagBig5NameT_REC
{
	char	szMerchant_Name_Index[2 + 1];	/* 表頭索引(最多兩行) */
	char	szMerchant_Name[48 + 1];	/* 表頭(32個半形字，16個全形字) */
}Big5NameT_REC;

/* 以欄位數決定 ex:欄位數是2個，理論上會有1個comma和兩個byte的0x0D 0X0A */
#define _SIZE_Big5NameT_COMMA_0D0A_                   3
#define _SIZE_Big5NameT_REC_				(sizeof(Big5NameT_REC))				/* 一個Record不含Comma和0D0A的長度 */
#define _Big5NameT_FILE_NAME_				"Big5NameT.dat"					/* File Name *//* 因為VISA和MASTERCARD會調成一樣，所以用VISA的就好了 */
#define _Big5NameT_FILE_NAME_BAK_			"Big5NameT.bak"					/* Bak Name */


/* Load & Save function */
int inLoadBig5NameTRec(int inBig5NameTRec);
int inSaveBig5NameTRec(int inBig5NameTRec);

/* Set function */
int inSetMerchant_Name_Index(char* szMerchant_Name_Index);
int inSetMerchant_Name(char* szMerchant_Name);

/* Get function */
int inGetMerchant_Name_Index(char* szMerchant_Name_Index);
int inGetMerchant_Name(char* szMerchant_Name);