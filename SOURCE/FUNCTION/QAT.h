typedef struct
{
        char    szQuestionIndex[2 + 1];        /* 問卷題目索引 */
        char    szQADBIndex[3 + 1];            /* 題庫索引 */
        char    szQuestion[80 + 1];            /* 問卷題目內容 */
} QAT_REC;


/* 以欄位數決定 ex:欄位數是3個，理論上會有2個comma和兩個byte的0x0D 0X0A */
#define _SIZE_QAT_COMMA_0D0A_                   4
#define _SIZE_QAT_REC_				(sizeof(QAT_REC))		/*一個record不含comma和0D0A的長度*/


/* Load & function */
int inLoadQATRec(int inQATRec, unsigned char *uszFileName);

/*Get function*/
int inGetQuestionIndex(char* szQuestionIndex);
int inGetQADBIndex(char* szQADBIndex);
int inGetQuestion(char* szQuestion);
