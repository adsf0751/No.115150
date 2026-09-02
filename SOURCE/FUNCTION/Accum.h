
int inACCUM_UpdateTotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_UpdateTotalAmountByCard(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_UpdateTotalAmountByUnknownCard(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_StoreRecord(ACCUM_TOTAL_REC *srAccumRec,unsigned char *szFileName);
int inACCUM_GetRecord(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_GetRecord_ESVC(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_ReviewReport_Total(TRANSACTION_OBJECT *pobTran);
int inACCUM_ReviewReport_Total_Settle(TRANSACTION_OBJECT *pobTran);
int inACCUM_UpdateFlow(TRANSACTION_OBJECT *pobTran);
int inACCUM_UpdateFlow_ESC(TRANSACTION_OBJECT *pobTran, int inUpdateType);
int inACCUM_Update_ESC_TotalAmount(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec, int inUpdateType);
int inACCUM_Check_Specific_Accum(TRANSACTION_OBJECT *pobTran, char *szHostName, ACCUM_TOTAL_REC* srACCUMRec);
int inACCUM_Check_Transaction_Count(TRANSACTION_OBJECT *pobTran, char *szHostName, char *szTrans);
int inACCUM_UpdateLoyaltyRedeem(TRANSACTION_OBJECT *pobTran, ACCUM_TOTAL_REC *srAccumRec);
/* 票證用 */
int inACCUM_Update_Ticket_Flow(TRANSACTION_OBJECT *pobTran);
int inACCUM_UpdateTotalAmount_Ticket(TRANSACTION_OBJECT *pobTran, TICKET_ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_StoreRecord_Ticket(TICKET_ACCUM_TOTAL_REC *srAccumRec, unsigned char *uszFileName);
/* 大高用 */
int inACCUM_UpdateFlow_TAKA(TRANSACTION_OBJECT* pobTran);
int inACCUM_UpdateTotalAmount_TAKA(TRANSACTION_OBJECT *pobTran, TAKA_ACCUM_TOTAL_REC *srAccumRec);
int inACCUM_UpdateTotalAmountByCard_TAKA(TRANSACTION_OBJECT *pobTran, TAKA_ACCUM_TOTAL_REC *srAccumRec);;
int inACCUM_StoreRecord_General(void* srAccumRec, unsigned char *uszFileName, int inAccumRecSize);
int inACCUM_GetRecord_General(TRANSACTION_OBJECT *pobTran, void *srAccumRec, int inAccumRecSize);
/* 信託使用 */
int inACCUM_Update_Trust_Flow(TRANSACTION_OBJECT *pobTran);
