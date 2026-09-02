

#ifndef __D_ICERAESFUNCTION__
#define __D_ICERAESFUNCTION__

#ifdef __cplusplus
	extern "C" {
#endif

/*
void KeyExpansion(unsigned char * Key);
void AddRoundKey(int round);
void SubBytes();
void ShiftRows();
void MixColumns();
void Cipher();
void InvSubBytes();
void InvShiftRows();
void InvMixColumns();
void InvCipher();
*/
short ICER_AES_Func(unsigned char bMode, unsigned char * bpKey, unsigned char bKeyType, unsigned char * bpInData, unsigned short usInLen, unsigned char * bpOutData);
short ICER_AES_CBC_Func(unsigned char bMode, unsigned char * bpKey, unsigned char bKeyType, unsigned char * bpIV, unsigned char * bpInData, unsigned short usInLen, unsigned char * bpOutData);

#ifdef __cplusplus
	}
#endif // __cplusplus

#endif

