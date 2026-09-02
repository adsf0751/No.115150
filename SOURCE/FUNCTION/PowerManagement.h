/* 
 * File:   PowerManagement.h
 * Author: user
 *
 * Created on 2018年3月14日, 上午 9:53
 */
#define _TIMER_POWER_MANAGEMENT_	_TIMER_NEXSYS_4_

#define _POWER_SOURCE_DC_WIRE_		d_PWRSRC_DCJACK		/* 電源線直連 */
#define _POWER_SOURCE_CRADLE_		d_PWRSRC_CRADLE		/* 底座充電 */
#define _POWER_SOURCE_BATTERY_		d_PWRSRC_BATTERY	/* 電池充電 */
#define _POWER_SOURCE_USB_		d_PWRSRC_USB		/* USB充電 */
#define _POWER_SOURCE_POWERBANK_	d_PWRSRC_POWERBANK	/* 行動電源???（不知道，猜的） */

#define	_POWER_MODE_STANDBY_		d_PWR_STANDBY_MODE
#define	_POWER_MODE_SLEEP_		d_PWR_SLEEP_MODE

int inPWM_PowerAutoModeEnable(void);
int inPWM_PowerAutoModeDisable(void);
int inPWM_PowerAutoMode_Standby_Time_Set(unsigned long ulSecond);
int inPWM_PowerAutoMode_Sleep_Time_Set(unsigned long ulSecond);
int inPWM_Get_PowerSource(unsigned char* uszSource);
int inPWM_Power_Mode(unsigned char uszMode);

int inPWM_Sleep_Mode_Timer_Start(void);
int inPWM_StandBy_Mode_Timer_Start(void);
int inPWM_Enter_Sleep_Mode(TRANSACTION_OBJECT *pobTran);
int inPWM_Enter_StandBy_Mode(TRANSACTION_OBJECT *pobTran);
int inPWM_IS_PWM_Enable(void);