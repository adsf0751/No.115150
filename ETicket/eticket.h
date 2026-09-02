#ifndef __E_TICKET_H__
#define __E_TICKET_H__

#ifdef	__cplusplus
extern "C" {
#endif

/**
    Error Code
**/
#define d_ETICKET_ERR_OFSET                 0xE000
#define d_ETICKET_ERR_INVALID_PARAM         d_ETICKET_ERR_OFSET + 0x01
#define d_ETICKET_ERR_COLLISION             d_ETICKET_ERR_OFSET + 0x02
#define d_ETICKET_ERR_NO_CARD               d_ETICKET_ERR_OFSET + 0x03
#define d_ETICKET_ERR_RATS_CMD_FAIL         d_ETICKET_ERR_OFSET + 0x04
#define d_ETICKET_ERR_ACCESS_FAIL           d_ETICKET_ERR_OFSET + 0x05
#define d_ETICKET_ERR_TIMEOUT               d_ETICKET_ERR_OFSET + 0x06
#define d_ETICKET_ERR_INVALID_AID           d_ETICKET_ERR_OFSET + 0x07
#define d_ETICKET_ERR_INVALID_APDU          d_ETICKET_ERR_OFSET + 0x08
#define d_ETICKET_ERR_LOGON_FAIL            d_ETICKET_ERR_OFSET + 0x09
/**
    Card Index
**/
#define d_ETICKET_ECC_CARD                  0x01
#define d_ETICKET_IPASS_CARD                0x02
#define d_ETICKET_YHDP_CARD                 0x03
#define d_ETICKET_ICASH_CARD                0x04

/**
 * Config parameter type
**/
#define d_ETICKET_ETHERNET_IP               1
#define d_ETICKET_ETHERNET_MASK             2
#define d_ETICKET_ETHERNET_GATEWAY          3
#define d_ETICKET_ETHERNET_HOSTIP           4
#define d_ETICKET_ETHERNET_HOSTPORT         5
#define d_ETICKET_ETHERNET_AUTOCON          6
#define d_ETICKET_ETHERNET_DHCP             7
    

/* Struct declare    */
typedef struct
{
        ULONG       ulTimeout;
        // b31-b4   RFU
        // b3       iCASH Index = 4
        // b2       HappyCash Index = 3
        // b1       iPASS Index = 2
        // b0       EasyCard Index = 1
        IN DWORD    CheckListTable;
        OUT BYTE    CardIndex;
}cardInfo;

/* Struct declare    */
typedef struct
{
    UINT    uiAmount;
    BYTE    bBalance[8];
    BYTE    bcardType;      //output card type.
}eTicket_TxnParam;

typedef struct {
    BYTE    bClientIP[4];
    BYTE    systemID;
    BYTE    serviceProvider;
    BYTE    shopNumber[3];
    BYTE    posID;
    BYTE    subCompanyID[2];
    BYTE    ipassUserID[10];
    BYTE    ipassUserPWD[10];
    BYTE    spUserID[10];
    BYTE    spUserPWD[10];
}eTicket_IPASSRegister;

void ETICKET_Init(void);
USHORT ETICKET_Register(BYTE bCardType, void *pRegParam);
USHORT ETICKET_LogOn(BYTE bCardIndex);  //card index bit0:EasyCard, bit1:IPASS, bit2:ICASH, bit3:YHDP
USHORT ETICKET_SetParameter(BYTE bType, BYTE *pData, USHORT usDataLen);
USHORT ETICKET_GetCardInfo(cardInfo *pInfo);
USHORT ETICKET_CheckBalance(BYTE *pBalance, BYTE bBalLen, BYTE *pCardType);
USHORT ETICKET_Deduct(eTicket_TxnParam *pTxnParam);
USHORT ETICKET_TopUp(eTicket_TxnParam *pTxnParam);

#ifdef	__cplusplus
}
#endif

#endif
