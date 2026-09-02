/* 
 * File:   Ticket.h
 * Author: user
 *
 * Created on 2016年11月2日, 上午 10:19
 */

#define _ETICKET_CARD_ECC_	d_ETICKET_ECC_CARD
#define _ETICKET_CARD_IPASS_	d_ETICKET_IPASS_CARD
#define _ETICKET_CARD_YHDP_	d_ETICKET_YHDP_CARD
#define _ETICKET_CARD_ICASH_	d_ETICKET_ICASH_CARD

/* 票證DEMO用 */
int inTicket_ETICKET_Init(void);
int inTicket_RunPollingCard(TRANSACTION_OBJECT *pobTran);
int inTicket_RunDeduct(TRANSACTION_OBJECT *pobTran);
int inTicket_RunCheckBalance(TRANSACTION_OBJECT *pobTran);
int inTicket_RunTopUp(TRANSACTION_OBJECT *pobTran);
int inTicket_Check_Amount(TRANSACTION_OBJECT *pobTran);
int inTicket_IPASS_Register(TRANSACTION_OBJECT *pobTran);
int inTicket_Set_Ethernet_Config(void);
int inTicket_LogOn(unsigned char uszCardIndex);