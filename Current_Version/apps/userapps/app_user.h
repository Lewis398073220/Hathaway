#ifndef __USER_APPS_H__
#define __USER_APPS_H__

#if defined(__PWM_LED_CTL__)
#include "hal_cmu.h"
#include "hal_pwm.h"

#define COL1_PWM_LED 	HAL_PWM_ID_1  //P2_5
#define COL2_PWM_LED 	HAL_PWM_ID_2  //P2_6
#define RED_PWM_LED 	HAL_PWM_ID_3  //P2_7

#define RED_LED 	0
#define BLUE_LED 	1
#define GREEN_LED 	2

#define PWM_IDLE 0
#define PWM_EN 1
#define BlINK_1x 2
#define BLINK_1S 3
#define BLINK_5S 4
#define BlINK_2x 5

#define LED_ON  2
#define LED_OFF_1S 5
#define LED_OFF_3S 15
#define LED_OFF_5S 25
#endif

#if defined(__USE_MOTOR_CTL__)
#define MOTOR_POWER_ON  500
#define MOTOR_POWER_OFF 500
#define MOTOR_PAIRING   200
#define MOTOR_CALLING   200
#define MOTOR_ENDCALL   500
#define MOTOR_MUTE      200
#define MOTOR_UNMUTE    200
#endif


#if defined(__EVRCORD_USER_DEFINE__)
#include "nvrecord_env.h"

#define DEMO_MODE 0xEE

#define SLEEP_TIME_3MIN  24
#define SLEEP_TIME_5MIN  39//39
#define SLEEP_TIME_10MIN 77
#define SLEEP_TIME_PERM  255

#define DEFAULT_SLEEP_TIME SLEEP_TIME_5MIN

extern IIR_CFG_T eq_custom_para;
extern IIR_CFG_T eq_custom_para_ancoff;
#if defined(AUDIO_LINEIN)
extern IIR_CFG_T eq_custom_para_linein;
#endif
extern uint8_t bt_name_len;
extern char bt_name[27];
#endif

#if defined(__USE_3_5JACK_CTR__)
extern bool reconncect_null_by_user;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int app_user_event_open_module(void);
int app_user_event_open_module_for_charge(void);
void app_user_event_close_module(void);

#if defined(__USE_3_5JACK_CTR__)
bool apps_3p5_jack_get_val(void);
bool  apps_3p5jack_plugin_check(void);	
bool app_apps_3p5jack_plugin_flag(bool clearcount);
void app_jack_start_timer(void);
void app_jack_stop_timer(void);
void apps_jack_event_process(void);
#endif

#if defined(__USE_IR_CTL__)
void app_ir_start_timer(void);
void app_ir_stop_timer(void);
void apps_ir_event_process(void);
#endif

#if defined(__PWM_LED_CTL__)
void app_pwm_disable(void);
void app_pwm_start_timer(void);
void app_pwm_stop_timer(void);
void apps_pwm_event_process(void);
void app_pwm_clear(void);
void apps_pwm_set(enum HAL_PWM_ID_T id, uint8_t enable);
void apps_blink_nx_set(uint8_t id, uint8_t enable, uint8_t period_on, uint8_t period_off);
void apps_blink_1x_set(uint8_t id,uint8_t period_on);
bool app_pwm_idle(void);
#endif

#if defined(__USE_MOTOR_CTL__)
void app_motor_init_timer(void);
void app_motor_timer_set(uint8_t vibrate_num, uint32_t vibrate_ms,uint16_t delay_ms);
void app_motor_timer_close(void);
#endif

#if defined(__USE_AMP_MUTE_CTR__)
void app_amp_open_start_timer(void);
void app_amp_open_stop_timer(void);
void apps_amp_switch_event_process(void);
void play_reboot_set(void);
#endif

#if defined(__AUDIO_FADEIN__)
void app_audio_fadein_start(uint8_t audio_type);
void apps_audio_fadein_event_process(void);
#endif

#if defined(__EVRCORD_USER_DEFINE__)
uint8_t app_eq_index_get(void);
void app_nvrecord_eq_set(uint8_t eq_index);
//void app_eq_para_get(uint8_t *p);
void app_eq_custom_para_get(uint8_t customization_eq_value[6]);
void app_nvrecord_eq_param_set(uint8_t customization_eq_value[6]);
uint8_t app_nvrecord_anc_get(void);
uint8_t app_nvrecord_anc_table_get(void);
void app_nvrecord_anc_set(uint8_t nc);
void app_nvrecord_demo_mode_set(uint8_t mod);
uint8_t app_nvrecord_demo_mode_get(void);
uint8_t app_get_sleep_time(void);
uint8_t get_sleep_time(void);
void app_nvrecord_sleep_time_set(uint8_t sltime);
uint8_t app_get_vibrate_mode(void);
void app_nvrecord_vibrate_mode_set(uint8_t mod);
uint8_t app_get_monitor_level(void);
uint8_t app_get_focus(void);
void app_nvrecord_monitor_level_set(uint8_t level);
void app_nvrecord_focus_set(uint8_t focus);
uint8_t app_get_focus(void);
void app_nvrecord_sensor_set(uint8_t on);
uint8_t app_get_touchlock(void);
void app_nvrecord_touchlock_set(uint8_t on);
uint8_t app_get_auto_poweroff(void);
void app_auto_poweroff_set(uint8_t pftime);
uint8_t app_get_sidetone(void);
void app_nvrecord_sidetone_set(uint8_t on);
uint8_t app_get_fota_flag(void);
void app_nvrecord_fotaflag_set(uint8_t on);
uint8_t app_get_new_multipoint_flag(void);
uint8_t app_get_multipoint_flag(void);
void app_nvrecord_multipoint_set(uint8_t on);
//add by cai
void app_nvrecord_language_set(uint8_t lang);
uint8_t app_nvrecord_language_get(void);
//end add
void app_nvrecord_para_get(void);
uint8_t app_get_TalkMicLed_flag(void);
void app_nvrecord_TalkMicLed_set(uint8_t on);
void app_nvrecord_set_bt_name(void);
#endif

#if defined(CUSTOM_BIN_CONFIG)
void app_get_custom_bin_config(void);
uint8_t get_custom_bin_config(uint8_t config_num);

#endif


#ifdef __cplusplus
	}//extern "C" {
#endif

#endif
