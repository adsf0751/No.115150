/* 
 * File:   APDU.h
 * Author: user
 *
 * Created on 2018年6月13日, 上午 9:57
 */
/* Define APDU Command */

/* 僅紀錄用，無使用 */
/* _FISC_AID_						"\xA0\x00\x00\x01\x72\x95\x00\x01" */

int inAPDU_APDUTransmit(unsigned char uszSlotID, unsigned char* uszSendBuffer, unsigned short usSendLen, unsigned char* uszReceiveBuffer, unsigned short* usReceiveLen);
int inAPDU_Send_APDU_User_Slot_Process(APDU_COMMAND* srAPDU);
int inAPDU_APDUTransmit_CTLS(unsigned char* uszSendBuffer, unsigned short usSendLen, unsigned char* uszReceiveBuffer, unsigned short* usReceiveLen);
int inAPDU_Send_APDU_CTLS_Process(APDU_COMMAND* srAPDU);
int inAPDU_APDUTransmit_Flow(TRANSACTION_OBJECT *pobTran, APDU_COMMAND *srAPDU_COMMAND);
int inAPDU_BuildAPDU(APDU_COMMAND *srAPDUData);
