#include "stdio.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "string.h"
#include "hal_trace.h"
#include "apps.h"
#include "app_bt.h"
#include "app_thread.h"
#include "tgt_hardware.h"
#include "app_user.h"
#include "philips_ble_api.h"
#include "app_bt_stream.h"
#include "bt_sco_chain.h"
#include "hal_codec.h"

//#if (defined(__AMP_SWITCH_CTR__)&&defined(__USE_AMP_MUTE_CTR__))
#include "btapp.h"
#include "app_anc.h"
#include "app_bt_media_manager.h"
//#endif

//add by cai
#ifdef MEDIA_PLAYER_SUPPORT
#include "app_media_player.h"
#endif
//end add

#if defined(__AC107_ADC__)
#include "ac107.h"
#endif

#if defined(__USE_AMP_MUTE_CTR__)

#define DEFINE_PLAY_REBOOT

#ifdef DEFINE_PLAY_REBOOT
static bool play_reboot=0;
uint8_t play_num=0;
#endif
#endif

extern struct BT_DEVICE_T  app_bt_device;

enum
{
    USER_EVENT_3P5JACK = 0,
    USER_EVENT_LINEIN=1,
    USER_EVENT_IR=2,
    USER_EVENT_PWM=3,
    USER_EVENT_MOTOR=4,
	USER_EVENT_AMPCTR=5,
	USER_EVENT_AUDIO_FADEIN=6,
    USER_EVENT_NONE
};

#if defined(__EVRCORD_USER_DEFINE__)
static uint8_t sleep_time=DEFAULT_SLEEP_TIME;
static uint8_t vibrate_mode=1;
static uint8_t eq_set_index=0;
static uint8_t anc_set_index=1;
static uint8_t monitor_level=20;
static uint8_t focus_on=0;
static uint8_t sensor_enable=1;
static uint8_t touch_lock=0;
static uint8_t sidetone=1;
static uint8_t anc_table_value=1;
static uint8_t fota_flag=0;
static uint8_t multipoint=1;
static uint8_t auto_powoff=0;
static uint8_t talkmic_led=1;
#endif

//static bool boom_mic_enable=0;

static void user_event_post(uint32_t id)
{
    APP_MESSAGE_BLOCK msg;
			
    msg.mod_id = APP_MODUAL_USERDEF;
    msg.msg_body.message_id = id;
    app_mailbox_put(&msg);
}

static int app_user_event_handle_process(APP_MESSAGE_BODY *msg_body)
{   
    uint32_t evt = msg_body->message_id;
    //uint32_t arg0 = msg_body->message_Param0;

    //TRACE(3," %s evt: %d, arg0: %d", __func__, evt, arg0);

    switch (evt)
    {
#if defined(__USE_3_5JACK_CTR__)    
		case USER_EVENT_LINEIN:
		    apps_jack_event_process();
		break;
#endif

#if defined(__USE_IR_CTL__)
		case USER_EVENT_IR:
		    apps_ir_event_process();
		break;
#endif

#if defined(__PWM_LED_CTL__)
		case USER_EVENT_PWM:
		    apps_pwm_event_process();
		break;
#endif

#if defined(__USE_MOTOR_CTL__)
		case USER_EVENT_MOTOR:
		break;
#endif	

#if defined(__USE_AMP_MUTE_CTR__)
		case USER_EVENT_AMPCTR:
			apps_amp_switch_event_process();
		break;
#endif

#if defined(__AUDIO_FADEIN__)
		case USER_EVENT_AUDIO_FADEIN:
			apps_audio_fadein_event_process();
		break;
#endif

		default:
		break;
    }
	
	return 0;
}

int app_user_event_open_module(void)
{       
    app_set_threadhandle(APP_MODUAL_USERDEF, app_user_event_handle_process);
	
#if defined(__USE_3_5JACK_CTR__)    
	app_jack_start_timer();
#endif

#if defined(__USE_AMP_MUTE_CTR__)
    app_amp_open_start_timer();
#endif

#if defined(__USE_IR_CTL__)
	app_ir_start_timer();
#endif

#if defined(__PWM_LED_CTL__)
	app_pwm_start_timer();
#endif

#if defined(__USE_MOTOR_CTL__)
	app_motor_init_timer();
#endif

#if defined(__EVRCORD_USER_DEFINE__)
	app_nvrecord_para_get();
#endif

    return 0;
}

int app_user_event_open_module_for_charge(void)
{       
    app_set_threadhandle(APP_MODUAL_USERDEF, app_user_event_handle_process);

#if defined(__PWM_LED_CTL__)
	app_pwm_start_timer();
#endif
    return 0;
}

void app_user_event_close_module(void)
{
	app_set_threadhandle(APP_MODUAL_USERDEF, NULL);

#if defined(__USE_3_5JACK_CTR__)    
	app_jack_stop_timer();
#endif

#if defined(__USE_AMP_MUTE_CTR__)
    app_amp_open_stop_timer();
#endif

#if defined(__USE_IR_CTL__)
	app_ir_stop_timer();
#endif

#if defined(__PWM_LED_CTL__)
	app_pwm_stop_timer();
#endif
}

#if defined(__USE_3_5JACK_CTR__)
bool reconncect_null_by_user=false;
static bool jack_3p5_plug_in_flag=false;
static int8_t jack_count=0;

extern int app_play_linein_onoff(bool onoff);

bool apps_3p5_jack_get_val(void)
{
    //return (bool)hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_3p5_jack_detecter[0].pin);
	return (bool)hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_3p5_jack_detecter.pin);
}

bool  apps_3p5jack_plugin_check(void)
{
	bool checkfg = false;
	uint8_t plugin_count = 0;
#if 0	
	if(false == apps_3p5_jack_get_val())
	{
	    if(true==hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_3p5_jack_detecter[1].pin)){
			TRACE(0,"3_5jack is plug in!");
			return checkfg;
	    }
		else{
			return (false);
		}
	}
	else
	{
	    TRACE(0,"3_5jack is plug out!");
		checkfg = false;
	}
	return checkfg;
#else
	if(apps_3p5_jack_get_val())
	{	
	    plugin_count++;
		TRACE(0,"3_5jack is plug in!");			
	}
	else
	{
	    plugin_count=0;
		TRACE(0,"3_5jack is plug out!");		
	}

	if( plugin_count>0)
	   checkfg = true;	
	
	return checkfg;
#endif
}

bool app_apps_3p5jack_plugin_flag(bool clearcount)
{
    if(clearcount)
		jack_count=0;

	return(jack_3p5_plug_in_flag);
}

osTimerId jack_3p5_timer = NULL;
static void app_jack_open_timehandler(void const *param);
osTimerDef(JACK_SW_TIMER, app_jack_open_timehandler);// define timers
#define JACK_TIMER_IN_MS (400)
#define CHECK_3_5JACK_MAX_NUM (3)

static void app_jack_open_timehandler(void const *param)
{
	user_event_post(USER_EVENT_LINEIN);
}

void app_jack_start_timer(void)
{
	if(jack_3p5_timer == NULL)
			jack_3p5_timer = osTimerCreate(osTimer(JACK_SW_TIMER), osTimerPeriodic, NULL);
	
	osTimerStart(jack_3p5_timer,JACK_TIMER_IN_MS);
}

void app_jack_stop_timer(void)
{	
	osTimerStop(jack_3p5_timer);
}

void apps_jack_event_process(void)
{ 
#if 0
	static int8_t in_val = 0, out_val = 0 , mic_val=0;
	
	if(false == apps_3p5_jack_get_val()){
		 if(true==hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_3p5_jack_detecter[1].pin)){
			in_val++;
			out_val=0;
		 	mic_val=0;
		 }
		 else{
			mic_val++;
			in_val=0;
		    out_val=0;
			if(mic_val>CHECK_3_5JACK_MAX_NUM)
				mic_val=CHECK_3_5JACK_MAX_NUM;
		 }
	}
	else{
		out_val++;
		in_val=0;
	    mic_val=0;
	}

	if(mic_val==CHECK_3_5JACK_MAX_NUM){
		//TRACE(0,"***detected boom_mic in!");
		mic_out(1);
		//boom_mic_enable=1;
	}
	else{
		//TRACE(0,"***detected boom_mic out!");
	    mic_out(0);
		//boom_mic_enable=0;
	}
#else
	static int8_t in_val = 0, out_val = 0 ;
	
	if(apps_3p5_jack_get_val()){
		in_val++;
		out_val=0;		
	}
	else{
		out_val++;
		in_val=0;
	}
#endif
	
	if((in_val==CHECK_3_5JACK_MAX_NUM)&&(jack_3p5_plug_in_flag==0)){
		TRACE(0,"***detected 3_5jack in!");
	    reconncect_null_by_user=true;
		//app_disconnect_all_bt_connections();
	    //app_bt_accessmode_set(BTIF_BAM_NOT_ACCESSIBLE);	
		//app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);		
#if defined(__AC107_ADC__)
		ac107_hw_open();
		ac107_i2c_init();
#endif
	   hal_codec_dac_mute(1);
		jack_3p5_plug_in_flag=1;
		jack_count=0;
	
		//app_poweroff_flag = 1;
		app_shutdown();//shutdown
	}
	/*
    if(in_val>(CHECK_3_5JACK_MAX_NUM+1) && (jack_3p5_plug_in_flag==0)){		
#if defined(__AC107_ADC__)
		ac107_hw_init();
#endif
		jack_3p5_plug_in_flag=1;
		jack_count=0;

		//app_poweroff_flag = 1;
		app_shutdown();//shutdown
	}*/
/*	
	if((out_val>CHECK_3_5JACK_MAX_NUM)&&(jack_3p5_plug_in_flag==1)){
		TRACE(0,"***detected 3_5jack out!");
		out_val=CHECK_3_5JACK_MAX_NUM;
		reconncect_null_by_user=false;
		lostconncection_to_pairing=0;		
		jack_3p5_plug_in_flag=0;
#if defined(AUDIO_LINEIN)
		app_play_linein_onoff(0);
#endif
		app_bt_profile_connect_manager_opening_reconnect();
#if defined(__AC107_ADC__)
		ac107_hw_close();
#endif
	}

	if(++jack_count>2){
		jack_count=0;
#if defined(AUDIO_LINEIN)		
		if(!bt_media_is_media_active()&&jack_3p5_plug_in_flag){
			app_play_linein_onoff(1);
		}
#endif
	}
*/
}
#endif


#if defined(__USE_IR_CTL__)
osTimerId ir_detect_timer = NULL;
static void app_ir_timehandler(void const *param);
osTimerDef(IR_DETECT_TIMER, app_ir_timehandler);// define timers
#define IR_DETECT_TIMER_IN_MS	(200)
#define wear_in_count 5
#define wear_out_count 5

static void app_ir_timehandler(void const *param)
{
	user_event_post(USER_EVENT_IR);
}

void app_ir_start_timer(void)
{
	if(ir_detect_timer == NULL)
		ir_detect_timer = osTimerCreate(osTimer(IR_DETECT_TIMER), osTimerPeriodic, NULL);
	
	osTimerStart(ir_detect_timer,IR_DETECT_TIMER_IN_MS);
}

void app_ir_stop_timer(void)
{
	osTimerStop(ir_detect_timer);
}


void apps_ir_event_process(void)
{
    static uint8_t L_IR_detect_h=0,L_IR_detect_l=0;
	static uint8_t R_IR_detect_h=0,R_IR_detect_l=0;
	static bool a2dp_pause_ir=0;
	static bool mic_mute_ir=0;
	static bool nc_off_ir=0;
	static bool out_ear_flag=1;
	
	hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_ir_ctl[0].pin);	
	hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_ir_ctl[1].pin);
	osDelay(1);	
	hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_ir_detecter[0].pin, HAL_GPIO_DIR_IN, 1);
    if(hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_ir_detecter[0].pin)){
		L_IR_detect_h++;
		L_IR_detect_l=0;
	
		if(L_IR_detect_h>wear_in_count) L_IR_detect_h=wear_in_count;		
    }
	else{
		L_IR_detect_l++;
		L_IR_detect_h=0;
	
	    if(L_IR_detect_l>wear_out_count) L_IR_detect_l=wear_out_count;
	}
	 
	hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_ir_detecter[1].pin, HAL_GPIO_DIR_IN, 1);	
	if(hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_ir_detecter[1].pin)){
		R_IR_detect_h++;
		R_IR_detect_l=0;

		if(R_IR_detect_h>wear_in_count) R_IR_detect_h=wear_in_count;
	}
	else{
		R_IR_detect_l++;
		R_IR_detect_h=0;

		if(R_IR_detect_l>wear_out_count) R_IR_detect_l=wear_out_count;	
	}
	
	hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_ir_ctl[0].pin);	
	hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_ir_ctl[1].pin);

	TRACE(4,"L_IR_detect in=%d out=%d --- R_IR_detect in=%d out=%d",L_IR_detect_h,L_IR_detect_l,R_IR_detect_h,R_IR_detect_l);
	//if(sensor_enable){
		if((L_IR_detect_h==wear_in_count)||(R_IR_detect_h==wear_in_count)){	

		    if(sensor_enable){
			if(a2dp_pause_ir && (app_bt_device.a2dp_play_pause_flag == 0)){		
            	a2dp_handleKey(AVRCP_KEY_PLAY);
				Notification_Media_Change();
			}
		    }
			
			if(out_ear_flag && app_bt_is_connected()){
				app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
			}

			if(nc_off_ir){
				app_nc_moment(NC_INVALID,0);
			}

			if(HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE!=app_get_hfcall_machine()){
				if(mic_mute_ir &&(app_bt_device.hf_mute_flag == 1)){
					hfp_handle_key(HFP_KEY_CLEAR_MUTE);
					app_bt_device.hf_mute_flag = 0;
					app_voice_report(APP_STATUS_INDICATION_BUTTON_49, 0);
				}
			}

			a2dp_pause_ir=0;
			mic_mute_ir=0;
			nc_off_ir=0;
			out_ear_flag=0;
		}
		else if((L_IR_detect_l==wear_out_count)&&(R_IR_detect_l==wear_out_count)){
			if(sensor_enable){
			if((app_bt_device.a2dp_play_pause_flag == 1) && (out_ear_flag==0)){
				a2dp_pause_ir=1;
				a2dp_handleKey(AVRCP_KEY_PAUSE);
		    	Notification_Media_Change();				
			}
			}
		
			if((out_ear_flag==0) && app_bt_is_connected()){
				app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
			}

			if(app_anc_is_on()){
				app_nc_moment(NC_INVALID,1);
				nc_off_ir=1;
			}

			if(HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE!=app_get_hfcall_machine()){
				if(app_bt_device.hf_mute_flag == 0){
					hfp_handle_key(HFP_KEY_MUTE);
					app_bt_device.hf_mute_flag = 1;
					mic_mute_ir=1;
					app_voice_report(APP_STATUS_INDICATION_BUTTON_50, 0);
				}
			}
							
			if(out_ear_flag==0){
				app_voice_report(APP_STATUS_INDICATION_BEEP_22, 0);			
			}
			
			out_ear_flag=1;
		}
	//}
}
#endif

#if defined(__PWM_LED_CTL__)
typedef struct
{	
	uint8_t timer_type;
	uint8_t timer_en;
	uint8_t timer_count;
	uint8_t timer_period_on;
	uint8_t timer_period_off;
	bool timer_level;
	bool timer_action;
}APP_PWM_TIMER_STRUCT;

APP_PWM_TIMER_STRUCT app_pwm_array[1];//app_pwm_array[3]
static uint8_t blink_flag=0;

osTimerId pwm_ctl_timer = NULL;
static void app_pwm_timehandler(void const *param);
osTimerDef(PWM_CTL_TIMER, app_pwm_timehandler);// define timers
#define PWM_CTL_TIMER_IN_MS	(100)//200
#define PWM_FRE 1000//1000

static void app_pwm_timehandler(void const *param)
{
	user_event_post(USER_EVENT_PWM);
}

void app_pwm_start_timer(void)
{
	if(pwm_ctl_timer == NULL)
		pwm_ctl_timer = osTimerCreate(osTimer(PWM_CTL_TIMER), osTimerPeriodic, NULL);
	
	osTimerStop(pwm_ctl_timer);
}

void app_pwm_stop_timer(void)
{
	osTimerStop(pwm_ctl_timer);
}

static void apps_pwm_enable(uint8_t i);
void apps_pwm_event_process(void)
{
	APP_PWM_TIMER_STRUCT *timer = app_pwm_array;
    unsigned int i;

    for(i = 0; i < ARRAY_SIZE(app_pwm_array); i++) {
        if (timer->timer_en){
            timer->timer_count++;
			if(PWM_EN == timer->timer_type)
			{
				if (timer->timer_count >= timer->timer_period_on)
			     	timer->timer_action=1;									
			}
			else{
				if ((timer->timer_count >= timer->timer_period_on)&&(timer->timer_level))
			        timer->timer_action=1;
				else if ((timer->timer_count >= timer->timer_period_off)&&(!(timer->timer_level)))
			        timer->timer_action=1;			
			}
        }
        timer++;
    }

    timer=app_pwm_array;
    for(i = 0; i < ARRAY_SIZE(app_pwm_array); i++) {
		 if(timer->timer_action){
		 	timer->timer_action=0;
			if(timer->timer_type==PWM_EN){
				timer->timer_count=0;
				apps_pwm_enable(i);
		 	}
			else if(timer->timer_type==BlINK_1x){				
				timer->timer_en=0;
				timer->timer_level=0;
				timer->timer_count=0;
				hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_io_led[i].pin);
				osTimerStop(pwm_ctl_timer);
		 	}
			else if(timer->timer_type==BlINK_2x){				
				if(timer->timer_level){
					timer->timer_level=0;
					timer->timer_count=0;
					hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_io_led[i].pin);
					if(blink_flag%2){
						timer->timer_period_off=LED_ON;
					}
					else{
						timer->timer_period_off=LED_OFF_3S;
					}
				}
				else{
					timer->timer_level=1;
					timer->timer_count=0;
					hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_io_led[i].pin);
					blink_flag++;
				}
		 	}		
			else{
				if(timer->timer_level){
					timer->timer_level=0;
					timer->timer_count=0;
					hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_io_led[i].pin);
				}
				else{
					timer->timer_level=1;
					timer->timer_count=0;
					hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_io_led[i].pin);					
				}
		 	}
		 }
		 timer++;
    }
}

HAL_PWM_CFG_T pwm_cfg;
bool up=0;
static void apps_pwm_enable(uint8_t i)
{	
	enum HAL_PWM_ID_T id;

	if(up){
		pwm_cfg.ratio+=2;
		if(pwm_cfg.ratio>=100){
			//pwm_cfg.ratio=100;
			up=0;
		}
	}
	else{
		pwm_cfg.ratio-=2;
		if(pwm_cfg.ratio<=6){
			//pwm_cfg.ratio=5;
			up=1;
		}
	}
	pwm_cfg.freq=PWM_FRE;
	pwm_cfg.inv=false;
	pwm_cfg.sleep_on=false;

	if(i==0)
		id=RED_PWM_LED;
	else
		id=COL1_PWM_LED;
	
	hal_pwm_enable(id, &pwm_cfg);
}

void app_pwm_clear(void)
{
	 osTimerStop(pwm_ctl_timer);
	 
	 hal_pwm_disable(RED_PWM_LED);
     
	 for(uint8_t i=0;i<ARRAY_SIZE(app_pwm_array);i++){
	 	hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_hw_io_led[i], 1);
		hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_hw_io_led[i].pin, HAL_GPIO_DIR_OUT, 0);

		app_pwm_array[i].timer_type = PWM_IDLE;
		app_pwm_array[i].timer_en = 0;
	 	app_pwm_array[i].timer_action = 0;
		hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_io_led[i].pin);
	}
}

void apps_pwm_set(enum HAL_PWM_ID_T id, uint8_t enable)
{
   	uint8_t i=0;
    TRACE(2," %s id=%d", __func__,id);
	
//	if(PWM_disable)
//		return;
		
  	if(RED_PWM_LED==id)
   		i=0;
//   	else if(GREEN_PWM_LED==id)
//   		i=2;
  
    APP_PWM_TIMER_STRUCT *timer = &app_pwm_array[i];		
    if(enable){	
		
		app_pwm_clear();

		if(RED_PWM_LED==id)
   		   	hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_hw_pwm_led[0], 1);
	
    	timer->timer_en = 1;
		timer->timer_type = PWM_EN;
		timer->timer_count = 0;
    	timer->timer_period_on = 1;
		timer->timer_period_off = 0;
		timer->timer_level = 0;
		timer->timer_action=0;

		pwm_cfg.freq=PWM_FRE;
		pwm_cfg.ratio=0;
		pwm_cfg.inv=false;
	    pwm_cfg.sleep_on=false;
		up=1;

		osTimerStart(pwm_ctl_timer,PWM_CTL_TIMER_IN_MS);
    }
	else{
		timer->timer_en = 0;
		timer->timer_action = 0;
		hal_pwm_disable(id);
	
		osTimerStop(pwm_ctl_timer);
	}
}

void apps_blink_nx_set(uint8_t id, uint8_t type, uint8_t period_on, uint8_t period_off)
{
	//if(!boom_mic_enable)
	//	return;

	//if(0==talkmic_led)
	//	return;

	APP_PWM_TIMER_STRUCT *timer = &app_pwm_array[id];

	if(timer->timer_type == type)
		return;
	
	app_pwm_clear();
	blink_flag=0;
		
   	timer->timer_en = 1;
	timer->timer_type = type;
	timer->timer_count = 0;
    timer->timer_period_on = period_on;
	timer->timer_period_off = period_off;
	timer->timer_level = 0;
	timer->timer_action=0;
		
	hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_io_led[id].pin);
		
	osTimerStart(pwm_ctl_timer,PWM_CTL_TIMER_IN_MS);
}
/*
void apps_blink_1x_set(uint8_t id,uint8_t period_on)
{
	if(PWM_disable)
		return;

	APP_PWM_TIMER_STRUCT *timer = &app_pwm_array[id];

	app_pwm_clear();

    timer->timer_en = 1;
	timer->timer_type = BlINK_1x;
	timer->timer_count = 0;
    timer->timer_period_on = period_on;
	timer->timer_period_off = 0;
	timer->timer_level = 1;
	timer->timer_action=0;

	hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_io_led[id].pin);
	osTimerStart(pwm_ctl_timer,PWM_CTL_TIMER_IN_MS);
}
*/
bool app_pwm_idle(void)
{
	APP_PWM_TIMER_STRUCT *timer = app_pwm_array;
    unsigned int i;

    for(i = 0; i < ARRAY_SIZE(app_pwm_array); i++) {
		if(timer->timer_type > PWM_IDLE)
			return(0);

		timer++;
    }
	return(1);
}
/*
void app_pwm_disable(void)
{
    if(PWM_disable==false){
		PWM_disable=true;
		app_pwm_clear();
    }
	else{
		PWM_disable=false;
		HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();	
		if(hfcall_machine != HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE){	
		#if defined(__PWM_LED_CTL__)
			apps_blink_nx_set(RED_LED, BlINK_2x, LED_ON, LED_OFF_3S);
		#endif

		}
	}	
	app_voice_report(APP_STATUS_INDICATION_BEEP_22, 0);
}
*/
#endif

#if defined(__USE_MOTOR_CTL__)
static HWTIMER_ID motor_timerid = NULL;
static uint8_t motor_vibrate_count=0;
static uint32_t motor_vibrate_time=0;

static void motor_timer_handler(void *p);
void app_motor_init_timer(void)
{
    if (motor_timerid == NULL)
        motor_timerid = hwtimer_alloc((HWTIMER_CALLBACK_T)motor_timer_handler, &motor_vibrate_count);
}

void app_motor_timer_set(uint8_t vibrate_num, uint32_t vibrate_ms,uint16_t delay_ms)
{
	if(vibrate_mode==0)
		return;

    if(motor_timerid == NULL)
        return;

	hwtimer_stop(motor_timerid);  
    motor_vibrate_count = vibrate_num;
	motor_vibrate_time = MS_TO_TICKS(vibrate_ms);	      
	hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_motor_ctl.pin);
	hwtimer_start(motor_timerid,MS_TO_TICKS(delay_ms));	
}

void app_motor_timer_close(void)
{
    if (motor_timerid == NULL)
        return;
	
    motor_vibrate_count = 0;
    hwtimer_stop(motor_timerid);
	hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_motor_ctl.pin);
}

static void motor_timer_handler(void *p)
{
    if(motor_vibrate_count){
		hwtimer_stop(motor_timerid);
		if(hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_motor_ctl.pin)){
			hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_motor_ctl.pin);
			//hwtimer_start(motor_timerid, MS_TO_TICKS(300));	

			if(motor_vibrate_count%2)
				hwtimer_start(motor_timerid, MS_TO_TICKS(300));
			else				
				hwtimer_start(motor_timerid, MS_TO_TICKS(1500)); 
			
		}
        else{
			hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_motor_ctl.pin);
			motor_vibrate_count--;
    		hwtimer_start(motor_timerid,motor_vibrate_time);			
        }
    }
	else{		
    	hwtimer_stop(motor_timerid);
		hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_motor_ctl.pin);
	}
}
#endif

#if defined(__USE_AMP_MUTE_CTR__)
osTimerId amp_switch_timer = NULL;
static void app_amp_open_timehandler(void const *param);
osTimerDef(AMP_SWITCH_TIMER, app_amp_open_timehandler);// define timers
#define AMP_SWITCH_TIMER (1000)

static void app_amp_open_timehandler(void const *param)
{
	user_event_post(USER_EVENT_AMPCTR);
}

void app_amp_open_start_timer(void)
{
	if(amp_switch_timer == NULL)
		amp_switch_timer = osTimerCreate(osTimer(AMP_SWITCH_TIMER), osTimerPeriodic, NULL);
	
	osTimerStart(amp_switch_timer,AMP_SWITCH_TIMER);
}

void app_amp_open_stop_timer(void)
{
	osTimerStop(amp_switch_timer);
}

void apps_amp_switch_event_process(void)
{
#if 0
    //TRACE(1,"apps_amp_switch_event_process a2dp_play_pause_flag=%d",app_bt_device.a2dp_play_pause_flag);
#ifdef __BT_ONE_BRING_TWO__
	if((HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE==app_get_hfcall_machine())&&(app_bt_device.a2dp_play_pause_flag == 0)){
#else
	if((HFCALL_MACHINE_CURRENT_IDLE==app_get_hfcall_machine())&&(app_bt_device.a2dp_play_pause_flag == 0)){
#endif

		if(reconncect_null_by_user){
			hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin);
		}	
     #ifdef ANC_APP
    	else if(app_anc_work_status()) {      		
			hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin);
    	}
	 #endif
		else{
			if(!bt_media_is_media_active())
				hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin);
			else
				hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin);
		}
	}
	else{		
		hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin);
	}
#endif

#ifdef DEFINE_PLAY_REBOOT
	if(app_bt_is_connected()){
		play_num++;
		if((play_reboot==1)&&(play_num==5)&&((app_bt_device.a2dp_state[0] > 0)||(app_bt_device.a2dp_state[1] > 0)))
		{
		    TRACE(0,"***play reboot");
			a2dp_handleKey(AVRCP_KEY_PLAY);
			play_reboot=0;
		}
		if(play_num>5)
			play_num=0;
	}
#endif
}

void play_reboot_set(void)
{
#ifdef DEFINE_PLAY_REBOOT
	play_reboot=1;
#endif
}
#endif

#if defined(__AUDIO_FADEIN__)
uint8_t audio_fadein_volume = 1;
uint8_t audio_fade_volume=0;

osTimerId audio_fadein__timer = NULL;
static void app_audio_fadein_timehandler(void const *param);
osTimerDef(AUDIO_FADEIN_TIMER, app_audio_fadein_timehandler);// define timers
#define AUDIO_FADEIN_TIME	(100)//100
		
static void app_audio_fadein_timehandler(void const *param)
{
	user_event_post(USER_EVENT_AUDIO_FADEIN);
}

void app_audio_fadein_start(uint8_t audio_type)
{
//	if(audio_type)
	  	audio_fade_volume=app_bt_stream_a2dpvolume_get_user();
//	else
//		audio_fade_volume=app_bt_stream_lineinvolume_get_user();

/*	
	if(audio_fade_volume>8)
     audio_fadein_volume=0;
    else
   	 audio_fadein_volume=1; 
*/
	audio_fadein_volume=1; 

	if(audio_fadein__timer == NULL)
		audio_fadein__timer = osTimerCreate(osTimer(AUDIO_FADEIN_TIMER), osTimerPeriodic, NULL);

	osTimerStart(audio_fadein__timer,AUDIO_FADEIN_TIME);
}

void apps_audio_fadein_event_process(void)
{
/*    if(audio_fade_volume>8)
     audio_fadein_volume+=2;
   else*/
   	 audio_fadein_volume+=1;

   if(audio_fadein_volume>audio_fade_volume)
	   audio_fadein_volume=audio_fade_volume;
	 
   app_bt_stream_volumeset(audio_fadein_volume+17);//for volume independent 
   //app_bt_stream_volumeset(audio_fadein_volume);

   if(audio_fadein_volume>=audio_fade_volume){
   	osTimerStop(audio_fadein__timer);
   }   
}
#endif

#if defined(__EVRCORD_USER_DEFINE__)
IIR_CFG_T eq_custom_para={
    .gain0 = -8,
    .gain1 = -8,
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, .0,   100, 0.8},
        {IIR_TYPE_PEAK, .0,   400, 1.0},
        {IIR_TYPE_PEAK, .0,  1000, 0.8},
        {IIR_TYPE_PEAK, .0,  2500, 1.0},
        {IIR_TYPE_PEAK, .0,  6300, 0.8},
		{IIR_TYPE_PEAK, .0, 12000, 1.0},
		{IIR_TYPE_PEAK, -6,  7000, 0.6},
		{IIR_TYPE_PEAK, -6, 28000, 0.6},
	}
};
/*
IIR_CFG_T eq_custom_para_ancoff={
    .gain0 = -8,
    .gain1 = -8,
    .num = 9,
    .param = {
        {IIR_TYPE_PEAK, .0,   100, 0.8},
        {IIR_TYPE_PEAK, .0,   400, 1.0},
        {IIR_TYPE_PEAK, .0,  1000, 0.8},
        {IIR_TYPE_PEAK, .0,  2500, 1.0},
        {IIR_TYPE_PEAK, .0,  6300, 0.8},
		{IIR_TYPE_PEAK, .0, 12000, 1.0},
		//{IIR_TYPE_PEAK,-11,    30, 0.5},
		{IIR_TYPE_PEAK,-9,   140, 1.2},
		{IIR_TYPE_PEAK, -2,   200, 1.8},
		{IIR_TYPE_PEAK, -7,  8000, 1.8},
	}
};
*/
/*
IIR_CFG_T eq_custom_para_linein={
    .gain0 = -6,
    .gain1 = -6,
    .num = 6,
    .param = {
        {IIR_TYPE_PEAK, .0,   100, 0.8},
        {IIR_TYPE_PEAK, .0,   400, 1.0},
        {IIR_TYPE_PEAK, .0,  1000, 0.8},
        {IIR_TYPE_PEAK, .0,  2500, 1.0},
        {IIR_TYPE_PEAK, .0,  6300, 0.8},
		{IIR_TYPE_PEAK, .0, 12000, 1.0},
	}
};
*/
uint8_t app_eq_index_get(void)
{   
	return (eq_set_index);
}

void app_nvrecord_eq_set(uint8_t eq_index)
{
    eq_set_index=eq_index;
		
	struct nvrecord_env_t *nvrecord_env;
			
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->eq_mode=eq_set_index;
	nv_record_env_set(nvrecord_env);

#if FPGA==0
    nv_record_flash_flush();
#endif
}

void app_eq_custom_para_get(uint8_t customization_eq_value[6])
{
	int8_t temp;	
		
	for(uint8_t i=0;i<6;i++){
		if(eq_custom_para.param[i].gain<0){
			temp=(int8_t)(2*(eq_custom_para.param[i].gain));
			temp*=-1;
			customization_eq_value[i]=(uint8_t)temp;
			customization_eq_value[i]|=0x80;
		}
		else{
			customization_eq_value[i]=(uint8_t)(2*(eq_custom_para.param[i].gain));
		}
		//TRACE(2,"***get customization_eq_value[%d]=%x",i,customization_eq_value[i]);
	}
}

void app_nvrecord_eq_param_set(uint8_t customization_eq_value[6])
{
	struct nvrecord_env_t *nvrecord_env;
	int8_t temp;
	uint8_t i=0;

	for(i=0;i<6;i++){
		if(customization_eq_value[i]>0x80){
			temp=(int8_t)(0x0f & customization_eq_value[i]);
			temp*=-1;
			eq_custom_para.param[i].gain=(float)temp;
			eq_custom_para.param[i].gain=(eq_custom_para.param[i].gain)/2;
			if(eq_custom_para.param[i].gain<-5)
				eq_custom_para.param[i].gain=-5;
			/*
			eq_custom_para_ancoff.param[i].gain=(float)temp;
			eq_custom_para_ancoff.param[i].gain=(eq_custom_para_ancoff.param[i].gain)/2;
			if(eq_custom_para_ancoff.param[i].gain<-5)
				eq_custom_para_ancoff.param[i].gain=-5;
			*/
		}
		else{
			temp=(int8_t)customization_eq_value[i];
			eq_custom_para.param[i].gain=(float)temp;
			eq_custom_para.param[i].gain=(eq_custom_para.param[i].gain)/2;
			if(eq_custom_para.param[i].gain>5)
				eq_custom_para.param[i].gain=5;
			/*
			eq_custom_para_ancoff.param[i].gain=(float)temp;
			eq_custom_para_ancoff.param[i].gain=(eq_custom_para_ancoff.param[i].gain)/2;
			if(eq_custom_para_ancoff.param[i].gain>5)
				eq_custom_para_ancoff.param[i].gain=5;
			*/
		}
		//TRACE(1,"***set customization_eq_value [%d]=%x",i,customization_eq_value[i]);
	}
	
#if 0// defined(AUDIO_LINEIN)
	for(i=0;i<6;i++){
		if(customization_eq_value[i]>0x80){
			temp=(int8_t)(0x0f & customization_eq_value[i]);
			temp*=-1;
			eq_custom_para_linein.param[i].gain=(float)temp;
			eq_custom_para_linein.param[i].gain=(eq_custom_para_linein.param[i].gain)/2;
			if(eq_custom_para_linein.param[i].gain<-5)
				eq_custom_para_linein.param[i].gain=-5;
		}
		else{
			temp=(int8_t)customization_eq_value[i];
			eq_custom_para_linein.param[i].gain=(float)temp;
			eq_custom_para_linein.param[i].gain=(eq_custom_para_linein.param[i].gain)/2;
			if(eq_custom_para_linein.param[i].gain>5)
				eq_custom_para_linein.param[i].gain=5;
		}
		//TRACE(1,"***set customization_eq_value [%d]=%x",i,customization_eq_value[i]);
	}
#endif

	nv_record_env_get(&nvrecord_env);
	for(i=0;i<6;i++){
		nvrecord_env->iir_gain[i] = customization_eq_value[i];
	}	
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}

uint8_t app_nvrecord_anc_get(void)
{
	return(anc_set_index);
}

uint8_t app_nvrecord_anc_table_get(void)
{
	return(anc_table_value);
}

void app_nvrecord_anc_set(uint8_t nc)
{	
	anc_set_index=nc;
	
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->anc_mode = nc;
	if(nc>0 && nc<5){ 
		nvrecord_env->anc_table_value = nc;
		anc_table_value=nc;
	}
	nv_record_env_set(nvrecord_env);

#if FPGA==0
    nv_record_flash_flush();
#endif
}

void app_nvrecord_demo_mode_set(uint8_t mod)
{
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->demo_mode = mod;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}

uint8_t app_nvrecord_demo_mode_get(void)
{
	struct nvrecord_env_t *nvrecord_env;
	
	nv_record_env_get(&nvrecord_env);

	return(nvrecord_env->demo_mode);
}


uint8_t app_get_sleep_time(void)
{
	if(sleep_time ==SLEEP_TIME_PERM) 
		return (0x00);
	else if(sleep_time==SLEEP_TIME_10MIN)
		return (0x03); 
	else if(sleep_time==SLEEP_TIME_5MIN)
		return (0x02);
    else	
		return (0x01);
}

uint8_t get_sleep_time(void)
{
    if(sleep_time<SLEEP_TIME_3MIN)
		return(DEFAULT_SLEEP_TIME);
	else
		return(sleep_time);
}

void app_nvrecord_sleep_time_set(uint8_t sltime)
{
   if(sltime==0x00)
   	sleep_time =SLEEP_TIME_PERM;
   else if(sltime==0x03)
   	sleep_time =SLEEP_TIME_10MIN;
   else if(sltime==0x02)
	sleep_time =SLEEP_TIME_5MIN;
   else
   	{sleep_time =SLEEP_TIME_3MIN;}

	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->sleep_time = sleep_time;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}

/*
uint8_t app_get_vibrate_mode(void)
{
	return (vibrate_mode);
}	

void app_nvrecord_vibrate_mode_set(uint8_t mod)
{
   if(mod==0x00)
   	vibrate_mode=0x00;
   else
   	{vibrate_mode=0x01;}
   
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->vibrate_mode = vibrate_mode;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}
*/

uint8_t app_get_monitor_level(void)
{
	return (monitor_level);
}	

void app_nvrecord_monitor_level_set(uint8_t level)
{
   	monitor_level=level;
   
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->monitor_level = monitor_level;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}


uint8_t app_get_focus(void)
{
	return (focus_on);
}	

void app_nvrecord_focus_set(uint8_t focus)
{
   	focus_on=focus;
   
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->focus_on = focus_on;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}
/*
uint8_t app_get_sensor(void)
{
	return (sensor_enable);
}	

void app_nvrecord_sensor_set(uint8_t on)
{
   	sensor_enable=on;
   
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->sensor_enable = sensor_enable;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}
*/
uint8_t app_get_touchlock(void)
{
	return (touch_lock);
}	

void app_nvrecord_touchlock_set(uint8_t on)
{
   	touch_lock=on;
   
	//struct nvrecord_env_t *nvrecord_env;
	//nv_record_env_get(&nvrecord_env);
	//nvrecord_env->touch_lock = touch_lock;
	//nv_record_env_set(nvrecord_env);
	
//#if FPGA==0
//    nv_record_flash_flush();
//#endif

}

uint8_t app_get_auto_poweroff(void)
{
	return (auto_powoff);
}	

void app_auto_poweroff_set(uint8_t pftime)
{
   	auto_powoff=pftime;
}


uint8_t app_get_sidetone(void)
{
	return (sidetone);
}	

void app_nvrecord_sidetone_set(uint8_t on)
{
   	sidetone=on;
   
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->sidetone = sidetone;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}


uint8_t app_get_fota_flag(void)
{
	return (fota_flag);
}	

void app_nvrecord_fotaflag_set(uint8_t on)
{
   	fota_flag=on;
   
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->fota_flag = fota_flag;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}


static uint8_t new_multipoint=1;
uint8_t app_get_new_multipoint_flag(void)
{
	return (new_multipoint);
}	

uint8_t app_get_multipoint_flag(void)
{
    if(new_multipoint)
		return 1;
	else
		return (multipoint);
}

void app_nvrecord_multipoint_set(uint8_t on)
{
   	new_multipoint=on;
   
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->multipoint = on;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
    if(new_multipoint){
		app_multipoint_api_set_on();
    }
}
/*
void app_nvrecord_set_bt_name(void)
{ 
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->bt_name_len = bt_name_len;

	for(uint8_t i=0;i<bt_name_len;i++){
	  nvrecord_env->bt_name[i] =bt_name[i];
	}	
	nv_record_env_set(nvrecord_env);

#if FPGA==0
    nv_record_flash_flush();
#endif
}
*/

//add by cai
void app_nvrecord_language_set(uint8_t lang)
{
	struct nvrecord_env_t *nvrecord_env;
	
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->media_language.language = lang;
	app_play_audio_set_lang(lang);
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif
}

uint8_t app_nvrecord_language_get(void)
{
	struct nvrecord_env_t *nvrecord_env;
	
	nv_record_env_get(&nvrecord_env);

	return(nvrecord_env->media_language.language);
}
//end add

void app_nvrecord_para_get(void)
{
	struct nvrecord_env_t *nvrecord_env;
	uint8_t i=0;
	int8_t temp;
	uint8_t igain=0;
	
	nv_record_env_get(&nvrecord_env);

	sleep_time=nvrecord_env->sleep_time;
	vibrate_mode=nvrecord_env->vibrate_mode;
	eq_set_index=nvrecord_env->eq_mode;
	anc_set_index=nvrecord_env->anc_mode;
	monitor_level=nvrecord_env->monitor_level;
	focus_on=nvrecord_env->focus_on;
	sensor_enable=nvrecord_env->sensor_enable;
	touch_lock=nvrecord_env->touch_lock;
	sidetone=nvrecord_env->sidetone;
	anc_table_value=nvrecord_env->anc_table_value;
	fota_flag=nvrecord_env->fota_flag;
	multipoint=nvrecord_env->multipoint;
	new_multipoint=multipoint;
	talkmic_led=nvrecord_env->talkmic_led;

	//for(i=0;i<bt_name_len;i++){
		//bt_name[i]=nvrecord_env->bt_name[i];
	//}

	for(i=0;i<6;i++){
		igain=nvrecord_env->iir_gain[i];
		if(igain>0x80){
			temp=(int8_t)(0x0f & igain);
			temp*=-1;
			eq_custom_para.param[i].gain=(float)temp;		
			eq_custom_para.param[i].gain=(eq_custom_para.param[i].gain)/2;
			if(eq_custom_para.param[i].gain<-5)
				eq_custom_para.param[i].gain=-5;
			
			//eq_custom_para_ancoff.param[i].gain=(float)temp;		
			//eq_custom_para_ancoff.param[i].gain=(eq_custom_para_ancoff.param[i].gain)/2;
			//if(eq_custom_para_ancoff.param[i].gain<-5)
			//	eq_custom_para_ancoff.param[i].gain=-5;
			
		}
		else{
			temp=(int8_t)igain;
			eq_custom_para.param[i].gain=(float)temp;
			eq_custom_para.param[i].gain=(eq_custom_para.param[i].gain)/2;
			if(eq_custom_para.param[i].gain>5)
				eq_custom_para.param[i].gain=5;
			
			//eq_custom_para_ancoff.param[i].gain=(float)temp;
			//eq_custom_para_ancoff.param[i].gain=(eq_custom_para_ancoff.param[i].gain)/2;
			//if(eq_custom_para_ancoff.param[i].gain>5)
			//	eq_custom_para_ancoff.param[i].gain=5;
			
		}
		TRACE(1,"***get customization_eq_gain [%d]=%x",i,nvrecord_env->iir_gain[i]);		
	}
#if defined(AUDIO_LINEIN)
	for(i=0;i<6;i++){
		igain=nvrecord_env->iir_gain[i];
		if(igain>0x80){
			temp=(int8_t)(0x0f & igain);
			temp*=-1;
			eq_custom_para_linein.param[i].gain=(float)temp/100;		
			eq_custom_para_linein.param[i].gain=(eq_custom_para_linein.param[i].gain)/2;
			if(eq_custom_para_linein.param[i].gain<-5)
				eq_custom_para_linein.param[i].gain=-5;
		}
		else{
			temp=(int8_t)igain;
			eq_custom_para_linein.param[i].gain=(float)temp/100;		
			eq_custom_para_linein.param[i].gain=(eq_custom_para_linein.param[i].gain)/2;
			if(eq_custom_para_linein.param[i].gain>5)
				eq_custom_para_linein.param[i].gain=5;
		}
			TRACE(1,"***get customization_linein_eq_gain [%d]=%x",i,nvrecord_env->iir_gain[i]);		
	}
#endif

#if defined(CUSTOM_BIN_CONFIG)
	app_get_custom_bin_config();//for debug
#endif

	TRACE(5,"sleep=%d,eq=%d,monitor=%d,focus=%d,multipoint=%d",sleep_time,eq_set_index,monitor_level,focus_on,multipoint);
}


uint8_t app_get_TalkMicLed_flag(void)
{
	return (talkmic_led);
}

void app_set_TalkMicLed_flag(uint8_t on)
{
	talkmic_led=on;
}

void app_nvrecord_TalkMicLed_set(uint8_t on)
{ 
	talkmic_led=on;

	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	nvrecord_env->talkmic_led = on;
	nv_record_env_set(nvrecord_env);
	
#if FPGA==0
    nv_record_flash_flush();
#endif

#if defined(__PWM_LED_CTL__)
	if((!talkmic_led)&&(HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE!=app_get_hfcall_machine())&&(!app_pwm_idle())){
		app_pwm_clear();
	}
	else if((talkmic_led)&&(HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE!=app_get_hfcall_machine())&&(app_pwm_idle())){
	   apps_blink_nx_set(RED_LED, BlINK_2x, LED_ON, LED_OFF_3S);
	}	
#endif	
}
#endif

#if defined(CUSTOM_BIN_CONFIG)
extern uint32_t __custom_bin_start[];
//extern uint32_t __custom_bin_start[];
//uint8_t binconfig[2]={0x00,0x00};
uint8_t binconfig[1]={0x00};

void app_get_custom_bin_config(void)
{		
	memcpy(binconfig,__custom_bin_start,1);
	//TRACE(1,"***app_get_custom_bin_config %d %d",binconfig[0],binconfig[1]);
	TRACE(1,"***app_get_custom_bin_config %d",binconfig[0]);
	//if((binconfig[0]==0x00)&&(binconfig[1]==0x00)){
	//	binconfig[0]=0x01;
	//    binconfig[1]=0x01;
	//}
}

uint8_t get_custom_bin_config(uint8_t config_num)
{	
    //if(config_num==0)
		return (binconfig[0]);
	//else
		//return (binconfig[1]);
}

#endif
