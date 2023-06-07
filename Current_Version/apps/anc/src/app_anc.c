/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "plat_types.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "aud_section.h"
#include "audioflinger.h"
#include "anc_process.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hwtimer_list.h"
#include "app_thread.h"
#include "tgt_hardware.h"
#include "pmu.h"
#include "hal_bootmode.h"
#include "hal_codec.h"
#include "app_anc.h"
#include "anc_wnr.h"

/** add by pang **/
#include "philips_ble_api.h"
#include "apps.h"
#if (defined(__PWM_LED_CTL__)||defined(__EVRCORD_USER_DEFINE__))
#include "app_user.h"
#endif
/** end add **/

#include "app_status_ind.h"
#ifdef __SIMPLE_INTERNAL_PLAYER_SUPPORT__
#include "simple_internal_player.h"
#endif
#include "hal_aud.h"
#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#endif

#if defined(IBRT) && defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
#include "app_tota_cmd_handler.h"
static uint32_t app_anc_cmd_from_phone = 0xffffffff;
static uint32_t app_anc_param_from_phone = 0xffffffff;
#endif

#include "co_math.h"
//#define ANC_MODE_SWITCH_WITHOUT_FADE //Comment this line if you need fade function between anc mode

#ifndef _ANC_FADE_STACK_SIZE
#define _ANC_FADE_STACK_SIZE (1*1024)
#endif

static osThreadId anc_fade_thread_tid;

#define FADE_IN   0x0001
#define FADE_OUT  0x0002
#define CHANGE_FROM_ANC_TO_TT_DIRECTLY 0x0003


static void anc_fade_thread(void const *argument);
osThreadDef(anc_fade_thread, osPriorityBelowNormal,1,_ANC_FADE_STACK_SIZE,"anc_fade_thread");
//osThreadDef(anc_fade_thread, osPriorityLow,1,_ANC_FADE_STACK_SIZE,"anc_fade_thread");//m by pang


extern uint8_t app_poweroff_flag;
uint32_t app_anc_get_anc_status(void);

void app_anc_disable(void);
void app_anc_close_anc(void);
osStatus osDelay(uint32_t millisec);
void app_anc_timer_set(uint32_t request, uint32_t delay);
void app_anc_timer_close(void);
extern bool app_mode_is_usbaudio(void); //96k 384k
extern bool app_mode_is_i2s(void);      //96k 384k
void app_anc_init_timer(void);
extern void i2s_player_send_stop(void);
extern void fb_anti_howl_start(void);
extern void fb_anti_howl_stop(void);
extern void hal_codec_reconfig_pll_freq(enum AUD_SAMPRATE_T dac_rate, enum AUD_SAMPRATE_T adc_rate);
extern void anc_status_sync(void);
extern void anc_status_sync_init(void);

extern uint8_t is_sbc_mode (void);
extern uint8_t is_sco_mode (void);

void app_anc_switch_set_edge(uint8_t down_edge);


#ifdef __ANC_STICK_SWITCH_USE_GPIO__
typedef void (*ANC_KEY_CALLBACK)(uint8_t status);
static ANC_KEY_CALLBACK app_switch_callback;
#endif

extern int app_shutdown(void);
extern int app_reset(void);

extern void analog_aud_codec_speaker_enable(bool en);

/** add by pang **/
extern void change_anc_on_set_eq_num(uint8_t eq_num);
#define ANC_OFF_EQ 	(0)
#define ANC_ON1_EQ 	(1)
#define ANC_ON2_EQ 	(2)
#define ANC_ON3_EQ 	(3)
/** end add **/

#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
extern bool anc_single_mode_is_on(void);
#endif
enum
{
    ANC_STATUS_OFF = 0,
    ANC_STATUS_ON,
    ANC_STATUS_WAITING_ON,
    ANC_STATUS_WAITING_OFF,
    ANC_STATUS_INIT_ON,
    ANC_STATUS_NONE
};

enum
{
    ANC_EVENT_INIT = 0,
    ANC_EVENT_OPEN,
    ANC_EVENT_CLOSE,
    ANC_EVENT_FADE_IN,
    ANC_EVENT_FADE_OUT,
    ANC_EVENT_CHANGE_SAMPLERATE,
    ANC_EVENT_CHANGE_STATUS,
    ANC_EVENT_HOWL_PROCESS,
    ANC_EVENT_SYNC_STATUS,
    ANC_EVENT_SYNC_INIT,
    ANC_EVENT_PWR_KEY_MONITOR,
    ANC_EVENT_PWR_KEY_MONITOR_REBOOT,
    ANC_EVENT_SWITCH_KEY_DEBONCE,
    SIMPLE_PLAYER_CLOSE_CODEC_EVT,
    SIMPLE_PLAYER_DELAY_STOP_EVT,
    ANC_EVENT_NONE
};

static uint32_t anc_work_status = ANC_STATUS_OFF;
static uint32_t anc_timer_request = ANC_EVENT_NONE;

static enum AUD_SAMPRATE_T anc_sample_rate[AUD_STREAM_NUM];

static HWTIMER_ID anc_timerid = NULL;
#define anc_gain_adjust_gap (60)

#define anc_init_switch_off_time (MS_TO_TICKS(1000 * 60 * 2))
#define anc_auto_power_off_time (MS_TO_TICKS(1000 * 60 * 60))
#define anc_switch_on_time (MS_TO_TICKS(600))
#define anc_close_delay_time (MS_TO_TICKS(1000 * 1)) //(1000 * 20)  m by pang
#define anc_pwr_key_monitor_time (MS_TO_TICKS(1500))
#define anc_switch_key_debonce_time (MS_TO_TICKS(40))

enum
{
    APP_ANC_IDLE = 0,
    APP_ANC_FADE_IN,
    APP_ANC_FADE_OUT,
};

uint32_t app_anc_fade_status = APP_ANC_IDLE;


static bool app_init_done = false;
#ifdef ANC_SWITCH_PIN
static bool app_anc_shutdown = false;
#endif
bool anc_set_dac_pa_delay = false;
static enum ANC_INDEX anc_coef_idx = 0;

extern void analog_aud_enable_dac_pa(uint8_t dac);

/** add by pang **/
#include "app_bt_stream.h"

enum
{
	anc_high = 0,
	anc_low,
	anc_wind,
};


enum
{
	monitor1 = 3,
	monitor2,
	monitor3,
	monitor4,
	monitor5,

	clearvoice1,
	clearvoice2,
	clearvoice3,
	clearvoice4,
	clearvoice5,
};

static uint8_t anc_current_mode=anc_on;
static uint8_t anc_on_mode=anc_high;//add by pang
static uint8_t monitor_mode=monitor1;//add by pang


uint8_t app_get_anc_mode(void)
{
	return (anc_current_mode);
}

void poweron_set_anc(void)
{
	uint8_t anc_mode_poweron;
	anc_mode_poweron=app_nvrecord_anc_get();
	
	if(anc_mode_poweron==0){
		anc_current_mode=anc_off;
		anc_on_mode=anc_high;
	}
	else if(anc_mode_poweron==1){
		anc_current_mode=anc_on;
		anc_on_mode=anc_high;
	}
	else if(anc_mode_poweron==2){
		anc_current_mode=anc_on;
		anc_on_mode=anc_low;
	}
	else if(anc_mode_poweron==3){
		anc_current_mode=anc_on;
		anc_on_mode=anc_wind;
	}
	else if(anc_mode_poweron==4){
		anc_current_mode=monitor;
		anc_on_mode=anc_high;
	}

	app_set_monitor_mode(app_get_monitor_level());
	set_anc_mode(anc_current_mode);	
}

uint8_t app_get_anc_on_mode(void)
{
	return (anc_on_mode);
}

void app_set_anc_on_mode(uint8_t anc_on_new_mode)
{
    if(anc_on_new_mode==1)
	   anc_on_mode=anc_high;
	else if(anc_on_new_mode==2)
	   anc_on_mode=anc_low;
	else //if(anc_on_new_mode==3)
	   anc_on_mode=anc_wind;
}

uint8_t app_get_monitor_mode(void)
{
	return (monitor_mode);
}

void app_set_monitor_mode(uint8_t monitor_new_level)
{
   if(monitor_new_level<=4)
		monitor_mode=monitor1;
	else if(monitor_new_level<=8)
		monitor_mode=monitor2;
	else if(monitor_new_level<=12)
		monitor_mode=monitor3;
	else if(monitor_new_level<=16)
		monitor_mode=monitor4;
	else
		monitor_mode=monitor5;

    if(app_get_focus()){
		monitor_mode+=5;
    }
}

void app_set_clearvoice_mode(uint8_t clear_mode)
{
   if(clear_mode){
   	 if(monitor_mode>clearvoice1)
		monitor_mode+=5;
   }
   else{
		if(monitor_mode>monitor5)
			monitor_mode-=5;
   }
}

uint8_t api_get_anc_mode(void)
{
    uint8_t ancmode=0;
	if(anc_current_mode==anc_off){
		ancmode=NC_OFF;
	}
	else if(anc_current_mode==anc_on){
		if(anc_on_mode==anc_high)
		  ancmode=ANC_HIGH;
		else if(anc_on_mode==anc_low)
		  ancmode=ANC_LOW;
		else //(anc_on_mode==anc_wind)
		  ancmode=ANC_WIND;
	}
	else{
		ancmode=MONITOR_ON;
	}
	return (ancmode);
}

/** end add **/

bool app_anc_is_on(void)
{
    return (ANC_STATUS_ON == anc_work_status);
}

uint32_t app_anc_get_sample_rate(void)
{
    return (uint32_t)anc_sample_rate[AUD_STREAM_CAPTURE]; 
}

void app_anc_set_coef(uint8_t index)
{
    anc_coef_idx = index;
}

uint8_t app_anc_get_coef(void)
{
    return anc_coef_idx;
}

void app_anc_set_init_done(void)
{
    app_init_done = true;
}

void app_anc_switch_turnled(bool on);

#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__

static ANC_KEY_CALLBACK app_pwr_key_monitor_callback;

static void app_pwr_key_monitor_handler(enum HAL_GPIO_PIN_T pin)
{
    uint8_t gpio_val = hal_gpio_pin_get_val(pin);

    TRACE(2," %s :%d ", __func__, gpio_val);
    if (app_pwr_key_monitor_callback)
        app_pwr_key_monitor_callback(gpio_val);
}

bool app_pwr_key_monitor_get_val(void)
{
    return (bool)hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)PWR_KEY_MONITOR_PIN);
}

void app_pwr_key_monitor_set_int_edge(uint8_t down_edge)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = true;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpiocfg.irq_polarity = (down_edge == 1) ? HAL_GPIO_IRQ_POLARITY_LOW_FALLING : HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_handler = app_pwr_key_monitor_handler;
    TRACE(2," %s :%d ", __func__, down_edge);
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)PWR_KEY_MONITOR_PIN, &gpiocfg);
}

void app_pwr_key_monitor_init(ANC_KEY_CALLBACK callback)
{
    app_pwr_key_monitor_callback = callback;
}
#endif

#ifdef ANC_LED_PIN
#if 0
void app_anc_switch_turnled(bool on)
{
    TRACE(2," %s on %d ", __func__, on);

    if (cfg_anc_led.pin != HAL_IOMUX_PIN_NUM)
    {
        if (on)
        {
            TRACE(0,"on\n");
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_anc_led.pin);
        }
        else
        {
            TRACE(0,"off\n");
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_anc_led.pin);
        }
    }
}
#else //m by pang
void app_anc_switch_turnled(bool on)
{
    //TRACE(2," %s on %d ", __func__, on);

    if (cfg_anc_led[0].pin != HAL_IOMUX_PIN_NUM)
    {
        if (on)
        {
            //TRACE(0,"on\n");
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_anc_led[0].pin);
        }
        else
        {
            //TRACE(0,"off\n");
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_anc_led[0].pin);
        }
    }
}

void app_monitor_switch_turnled(bool on)
{
    //TRACE(2," %s on %d ", __func__, on);

    if (cfg_anc_led[1].pin != HAL_IOMUX_PIN_NUM)
    {
        if (on)
        {
            //TRACE(0,"on\n");
            hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_anc_led[1].pin);
        }
        else
        {
            //TRACE(0,"off\n");
            hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_anc_led[1].pin);
        }
    }
}
#endif
#endif

#ifdef __ANC_STICK_SWITCH_USE_GPIO__

static void app_anc_switch_int_handler(enum HAL_GPIO_PIN_T pin)
{
    // uint8_t gpio_val;
    uint8_t gpio_val = hal_gpio_pin_get_val(pin);
    TRACE(3," %s ,pin %d,status %d", __func__, pin, gpio_val);
#ifdef ANC_LED_PIN
    if (gpio_val)
        app_anc_switch_turnled(true);
    else
        app_anc_switch_turnled(false);
#endif
    app_anc_switch_set_edge(gpio_val);

    app_anc_timer_set(ANC_EVENT_SWITCH_KEY_DEBONCE, anc_switch_key_debonce_time);
    //if (app_switch_callback)
    //    app_switch_callback(gpio_val);
    // app_anc_key_handler();
}

void app_anc_switch_init(ANC_KEY_CALLBACK callback)
{
    app_switch_callback = callback;
}
#endif

#ifdef ANC_SWITCH_PIN
bool app_anc_set_reboot(void)
{
    return (app_anc_shutdown && hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)ANC_SWITCH_PIN));
}

bool app_anc_switch_get_val(void)
{
    return (bool)hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)ANC_SWITCH_PIN);
}

void app_anc_switch_set_edge(uint8_t down_edge)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = true;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpiocfg.irq_polarity = (down_edge == 1) ? HAL_GPIO_IRQ_POLARITY_LOW_FALLING : HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_handler = app_anc_switch_int_handler;
    TRACE(2," %s :%d ", __func__, down_edge);
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)ANC_SWITCH_PIN, &gpiocfg);
}
#endif

int __anc_usb_app_fadein(enum ANC_TYPE_T type)
{
    int32_t gain0_curr, gain1_curr;
    int32_t gain0_tg, gain1_tg;

    anc_get_gain(&gain0_curr, &gain1_curr, type);
    anc_get_cfg_gain(&gain0_tg, &gain1_tg, type);

    if (gain0_tg > 0)
    {
        if (gain0_curr < gain0_tg)
        {
            gain0_curr++;
        }
    }
    else
    {
        if (gain0_curr > gain0_tg)
        {
            gain0_curr--;
        }
    }

    if (gain1_tg > 0)
    {
        if (gain1_curr < gain1_tg)
        {
            gain1_curr++;
        }
    }
    else
    {
        if (gain1_curr > gain1_tg)
        {
            gain1_curr--;
        }
    }

    hal_sys_timer_delay_us(25);
    anc_set_gain(gain0_curr, gain1_curr, type);

    if ((gain0_curr == gain0_tg) && (gain1_curr == gain1_tg))
    {
        TRACE(3,"[%s] cur gain: %d %d", __func__, gain0_curr, gain1_curr);
        return 0;
    }

    return 1;
}

int __anc_usb_app_fadeout(enum ANC_TYPE_T type)
{
    int32_t gain0_curr, gain1_curr;

    anc_get_gain(&gain0_curr, &gain1_curr, type);

    if (gain0_curr > 0)
    {
        gain0_curr--;
    }
    else if (gain0_curr < 0)
    {
        gain0_curr++;
    }

    if (gain1_curr > 0)
    {
        gain1_curr--;
    }
    else if (gain1_curr < 0)
    {
        gain1_curr++;
    }

    //  gain0_curr = gain1_curr = 0 ;
    hal_sys_timer_delay_us(25);
    anc_set_gain(gain0_curr, gain1_curr, type);

    if ((gain0_curr == 0) && (gain1_curr == 0))
    {
        TRACE(3,"[%s] gain: %d, %d", __func__, gain0_curr, gain1_curr);
        return 0;
    }

	return 1;
}

#if defined(ANC_FF_ENABLED) && defined(ANC_FB_ENABLED)
int __anc_usb_app_fadein_ff_fb(void)
{
    int32_t gain0_curr, gain1_curr;
    int32_t gain2_curr, gain3_curr;

    int32_t gain0_tg, gain1_tg;
    int32_t gain2_tg, gain3_tg;


    anc_get_gain(&gain0_curr, &gain1_curr, ANC_FEEDFORWARD);
	anc_get_gain(&gain2_curr, &gain3_curr, ANC_FEEDBACK);
    anc_get_cfg_gain(&gain0_tg, &gain1_tg, ANC_FEEDFORWARD);
	anc_get_cfg_gain(&gain2_tg, &gain3_tg, ANC_FEEDBACK);

    uint32_t random_factor = rand();
    random_factor %= 5;
    if(random_factor==0) random_factor++;


    uint32_t random_factor_inverse = 5-random_factor;
    if(random_factor_inverse==0) random_factor_inverse++;

    random_factor_inverse =1;

	if(is_sbc_mode() || is_sco_mode()){
	    if (gain0_tg > 0)
	    {
	        if (gain0_curr < gain0_tg)
	        {
	            gain0_curr = MIN(gain0_curr+random_factor_inverse, gain0_tg);
	        }
	    }
	    else
	    {
	        if (gain0_curr > gain0_tg)
	        {
				gain0_curr = MAX(gain0_curr-random_factor_inverse, gain0_tg);
	        }
	    }

	    if (gain1_tg > 0)
	    {
	        if (gain1_curr < gain1_tg)
	        {
	            gain1_curr = MIN(gain1_curr+random_factor_inverse, gain1_tg);
	        }
	    }
	    else
	    {
	        if (gain1_curr > gain1_tg)
	        {
	            gain1_curr = MAX(gain1_curr-random_factor_inverse, gain1_tg);
	        }
	    }

		if (gain2_tg > 0)
		{
			if (gain2_curr < gain2_tg)
			{
				gain2_curr = MIN(gain2_curr+random_factor_inverse, gain2_tg);
			}
		}
		else
		{
			if (gain2_curr > gain2_tg)
			{
	            gain2_curr = MAX(gain2_curr-random_factor_inverse, gain2_tg);
			}
		}

		if (gain3_tg > 0)
		{
			if (gain3_curr < gain3_tg)
			{
				gain3_curr = MIN(gain3_curr+random_factor_inverse, gain3_tg);
			}
		}
		else
		{
			if (gain3_curr > gain3_tg)
			{
	            gain3_curr = MAX(gain3_curr-random_factor_inverse, gain3_tg);
			}
		}
	}else{
	    if (gain0_tg > 0)
	    {
	        if (gain0_curr < gain0_tg)
	        {
	            gain0_curr = MIN(gain0_curr+random_factor_inverse, gain0_tg);
	        }
	    }
	    else
	    {
	        if (gain0_curr > gain0_tg)
	        {
				gain0_curr = MAX(gain0_curr-random_factor_inverse, gain0_tg);
	        }
	    }

	    if (gain1_tg > 0)
	    {
	        if (gain1_curr < gain1_tg)
	        {
	            gain1_curr = MIN(gain1_curr+random_factor_inverse, gain1_tg);
	        }
	    }
	    else
	    {
	        if (gain1_curr > gain1_tg)
	        {
	            gain1_curr = MAX(gain1_curr-random_factor_inverse, gain1_tg);
	        }
	    }

		if (gain2_tg > 0)
		{
			if (gain2_curr < gain2_tg)
			{
				gain2_curr = MIN(gain2_curr+random_factor_inverse, gain2_tg);
			}
		}
		else
		{
			if (gain2_curr > gain2_tg)
			{
	            gain2_curr = MAX(gain2_curr-random_factor_inverse, gain2_tg);
			}
		}

		if (gain3_tg > 0)
		{
			if (gain3_curr < gain3_tg)
			{
				gain3_curr = MIN(gain3_curr+random_factor_inverse, gain3_tg);
			}
		}
		else
		{
			if (gain3_curr > gain3_tg)
			{
	            gain3_curr = MAX(gain3_curr-random_factor_inverse, gain3_tg);
			}
		}
	}


    anc_set_gain(gain0_curr, gain1_curr, ANC_FEEDFORWARD);
	anc_set_gain(gain2_curr, gain3_curr, ANC_FEEDBACK);


	//TRACE(5,"[%s] cur gain: %d %d %d %d", __func__, gain0_curr, gain1_curr, gain2_curr, gain3_curr);

    //osDelay(random_factor);

    hal_sys_timer_delay_us(40);
    if(gain0_curr%40==0)
    {
        //osDelay(random_factor);
    }

    if ((gain0_curr == gain0_tg) && (gain1_curr == gain1_tg) && (gain2_curr == gain2_tg) && (gain3_curr == gain3_tg))
    {
        anc_disable_gain_updated_when_pass0(1);

        anc_set_gain(gain0_curr, gain1_curr, ANC_FEEDFORWARD);
	    anc_set_gain(gain2_curr, gain3_curr, ANC_FEEDBACK);
        TRACE(5,"[%s] end cur gain: %d, %d, %d, %d", __func__, gain0_curr, gain1_curr,gain2_curr,gain3_curr);
        return 0;
    }
    return 1;
}

int __anc_usb_app_fadeout_ff_fb(void)
{
    int32_t gain0_curr, gain1_curr;
	int32_t gain2_curr, gain3_curr;
    anc_get_gain(&gain0_curr, &gain1_curr, ANC_FEEDFORWARD);
	anc_get_gain(&gain2_curr, &gain3_curr, ANC_FEEDBACK);

    uint32_t random_factor = rand();
    random_factor = random_factor%5;
    if(random_factor==0) random_factor++;

    uint32_t random_factor_inverse = 5-random_factor;
    if(random_factor_inverse==0) random_factor_inverse++;

    random_factor_inverse =1;


	if(is_sbc_mode() || is_sco_mode()){
	    if (gain0_curr > 0)
	    {
			gain0_curr = MAX(gain0_curr-random_factor_inverse,0);
	    }
	    else if (gain0_curr < 0)
	    {
			gain0_curr = MIN(gain0_curr+random_factor_inverse,0);
	    }

	    if (gain1_curr > 0)
	    {
			gain1_curr = MAX(gain1_curr-random_factor_inverse,0);
	    }
	    else if (gain1_curr < 0)
	    {
			gain1_curr = MIN(gain1_curr+random_factor_inverse,0);
	    }

	    if (gain2_curr > 0)
	    {
			gain2_curr = MAX(gain2_curr-random_factor_inverse,0);
	    }
	    else if (gain2_curr < 0)
	    {
			gain2_curr = MIN(gain2_curr+random_factor_inverse,0);
	    }

	    if (gain3_curr > 0)
	    {
			gain3_curr = MAX(gain3_curr-random_factor_inverse,0);
	    }
	    else if (gain3_curr < 0)
	    {
			gain3_curr = MIN(gain3_curr+random_factor_inverse,0);
	    }
	}else{
	    if (gain0_curr > 0)
	    {
			gain0_curr = MAX(gain0_curr-random_factor_inverse,0);
	    }
	    else if (gain0_curr < 0)
	    {
			gain0_curr = MIN(gain0_curr+random_factor_inverse,0);
	    }

	    if (gain1_curr > 0)
	    {
			gain1_curr = MAX(gain1_curr-random_factor_inverse,0);
	    }
	    else if (gain1_curr < 0)
	    {
			gain1_curr = MIN(gain1_curr+random_factor_inverse,0);
	    }

	    if (gain2_curr > 0)
	    {
			gain2_curr = MAX(gain2_curr-random_factor_inverse,0);
	    }
	    else if (gain2_curr < 0)
	    {
			gain2_curr = MIN(gain2_curr+random_factor_inverse,0);
	    }

	    if (gain3_curr > 0)
	    {
			gain3_curr = MAX(gain3_curr-random_factor_inverse,0);
	    }
	    else if (gain3_curr < 0)
	    {
			gain3_curr = MIN(gain3_curr+random_factor_inverse,0);
	    }

	}


    anc_set_gain(gain0_curr, gain1_curr, ANC_FEEDFORWARD);
	anc_set_gain(gain2_curr, gain3_curr, ANC_FEEDBACK);
    //TRACE(5,"[%s] cur gain: %d %d %d %d", __func__, gain0_curr, gain1_curr, gain2_curr, gain3_curr);


	//osDelay(random_factor);

    hal_sys_timer_delay_us(40);
    if(gain0_curr%40==0)
    {
        //osDelay(random_factor);
    }

    if ((gain0_curr == 0) && (gain1_curr == 0) && (gain2_curr == 0) && (gain3_curr == 0))
    {
        anc_disable_gain_updated_when_pass0(1);

        anc_set_gain(gain0_curr, gain1_curr, ANC_FEEDFORWARD);
	    anc_set_gain(gain2_curr, gain3_curr, ANC_FEEDBACK);
        TRACE(5,"[%s] end cur gain: %d, %d, %d, %d", __func__, gain0_curr, gain1_curr,gain2_curr,gain3_curr);
        return 0;
    }

    return 1;
}
#endif

void anc_gain_fade_handle(void)
{
    TRACE(2," %s %d ", __func__, app_anc_fade_status);
    if (app_anc_fade_status == APP_ANC_FADE_OUT)
    {
        osSignalSet(anc_fade_thread_tid,FADE_OUT);
    }

    if (app_anc_fade_status == APP_ANC_FADE_IN)
    {
#ifdef ANC_LED_PIN
        //app_anc_switch_turnled(true);
#endif

        osSignalSet(anc_fade_thread_tid,FADE_IN);

        if (anc_set_dac_pa_delay)
        {
            anc_set_dac_pa_delay = false;
//          osDelay(600);
//          analog_aud_enable_dac_pa(48);
        }
    }
}
void app_anc_gain_fadein(void)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(1," %s ", __func__);
    if (app_poweroff_flag)
        return;

    app_anc_fade_status = APP_ANC_FADE_IN;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_FADE_IN;
    app_mailbox_put(&msg);
}

void anc_fade_thread(void const *argument)
{

    osEvent evt;
    evt.status = 0;
    uint32_t singles = 0;

    while(1)
    {
        evt = osSignalWait(0,osWaitForever);
        
        singles = evt.value.signals;
        TRACE(2,"anc_fade_thread. %d",singles);

        if(evt.status == osEventSignal)
        {

            switch (singles)
            {

                case FADE_IN:
#if defined(ANC_FF_ENABLED) && defined(ANC_FB_ENABLED)
                    anc_disable_gain_updated_when_pass0(0);
                    while(__anc_usb_app_fadein_ff_fb());
#else
#ifdef ANC_FF_ENABLED
                    while (__anc_usb_app_fadein(ANC_FEEDFORWARD))
                    ;
#endif
#ifdef ANC_FB_ENABLED
                    while (__anc_usb_app_fadein(ANC_FEEDBACK))
                    ;
#endif
#endif
                    app_anc_fade_status = APP_ANC_IDLE;

                    break;
                case FADE_OUT:

#if defined(ANC_FF_ENABLED) && defined(ANC_FB_ENABLED)
                    anc_disable_gain_updated_when_pass0(0);
                    while(__anc_usb_app_fadeout_ff_fb());
#else
#ifdef ANC_FF_ENABLED
                    while (__anc_usb_app_fadeout(ANC_FEEDFORWARD))
                        ;
#endif
#ifdef ANC_FB_ENABLED
                    while (__anc_usb_app_fadeout(ANC_FEEDBACK))
                        ;
#endif
#endif

                    app_anc_fade_status = APP_ANC_IDLE;


                    break;
                 case CHANGE_FROM_ANC_TO_TT_DIRECTLY:
#if defined(ANC_FF_ENABLED) && defined(ANC_FB_ENABLED)
                    anc_disable_gain_updated_when_pass0(0);
                    while(__anc_usb_app_fadeout_ff_fb());
#else
#ifdef ANC_FF_ENABLED
                    while (__anc_usb_app_fadeout(ANC_FEEDFORWARD));              
#endif

#ifdef ANC_FB_ENABLED
                    while (__anc_usb_app_fadeout(ANC_FEEDBACK));             
#endif
#endif

#if defined(ANC_FF_ENABLED) && defined(ANC_FB_ENABLED)
                    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDFORWARD,ANC_GAIN_DELAY);
                    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDBACK,ANC_GAIN_DELAY);
#ifdef AUDIO_ANC_FB_MC_HW
                    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_MUSICCANCLE,ANC_GAIN_DELAY);
#endif
#else
#ifdef ANC_FF_ENABLED
                    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDFORWARD,ANC_GAIN_DELAY);
#endif
     
#ifdef ANC_FB_ENABLED
                    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDBACK,ANC_GAIN_DELAY);
#endif
#endif

#if defined(ANC_FF_ENABLED) && defined(ANC_FB_ENABLED)
                    anc_disable_gain_updated_when_pass0(0);
                    while(__anc_usb_app_fadein_ff_fb());
#else
#ifdef ANC_FF_ENABLED
                    while (__anc_usb_app_fadein(ANC_FEEDFORWARD));
#endif
     
#ifdef ANC_FB_ENABLED
                    while (__anc_usb_app_fadein(ANC_FEEDBACK));
#endif
#endif

                    app_anc_fade_status = APP_ANC_IDLE;

                    //recommand to play "ANC SWITCH" prompt here...

                    break;
                default:
                    break;

            }

        }
        else
        {
            TRACE(2,"anc fade thread evt error");
            continue;
        }
    }
}


void app_anc_post_anc_codec_close(void)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(1," %s ", __func__);
    if (app_poweroff_flag)
        return;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = SIMPLE_PLAYER_CLOSE_CODEC_EVT;
    app_mailbox_put(&msg);
}

void app_anc_gain_fadeout(void)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(1," %s ", __func__);
    if (app_poweroff_flag)
        return;

    app_anc_fade_status = APP_ANC_FADE_OUT;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_FADE_OUT;
    app_mailbox_put(&msg);
}

void app_anc_status_post(uint8_t status)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(2," %s status %d", __func__, status);
    if (app_poweroff_flag)
        return;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_CHANGE_STATUS;
    msg.msg_body.message_Param0 = status;
    app_mailbox_put(&msg);
}

void app_anc_status_post_extend(uint32_t param0, uint32_t param1, uint32_t param2)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(4," %s param0:0x%04X param1:%d param2:%d", __func__, param0, param1, param2);
    if (app_poweroff_flag)
        return;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_CHANGE_STATUS;
    msg.msg_body.message_Param0 = param0;
    msg.msg_body.message_Param1 = param1;
    msg.msg_body.message_Param2 = param2;
    app_mailbox_put(&msg);
}

void app_anc_status_sync(uint8_t status)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(2," %s status %d", __func__, status);
    if (app_poweroff_flag)
        return;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_SYNC_STATUS;
    msg.msg_body.message_Param0 = status;
    app_mailbox_put(&msg);
}

void app_anc_status_sync_extend(uint32_t param0, uint32_t param1, uint32_t param2)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(4," %s param0:0x%04X param1:%d param2:%d", __func__, param0, param1, param2);
    if (app_poweroff_flag)
        return;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_SYNC_STATUS;
    msg.msg_body.message_Param0 = param0;
    msg.msg_body.message_Param1 = param1;
    msg.msg_body.message_Param2 = param2;
    app_mailbox_put(&msg);
}


void app_anc_status_sync_init(void)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(1," %s ", __func__);
    if (app_poweroff_flag)
        return;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_SYNC_INIT;
    app_mailbox_put(&msg);
}

void app_anc_do_init(void)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(1," %s DO INIT", __func__);
    if (app_poweroff_flag)
        return;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_INIT;
    app_mailbox_put(&msg);
}

void app_anc_send_howl_evt(uint32_t howl)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(2," %s %d", __func__, howl);
    if (app_poweroff_flag)
        return;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_HOWL_PROCESS;
    msg.msg_body.message_Param0 = howl;
    app_mailbox_put(&msg);
}

void app_anc_send_pwr_key_monitor_evt(uint8_t level)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(2," %s %d", __func__, level);
    if (app_poweroff_flag)
        return;

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = ANC_EVENT_PWR_KEY_MONITOR;
    msg.msg_body.message_Param0 = level;
    app_mailbox_put(&msg);
}

extern void simple_player_delay_stop(void);

void app_anc_post_simplayer_stop_evt(void)
{
    APP_MESSAGE_BLOCK msg;
    TRACE(1," %s d", __func__);

    msg.mod_id = APP_MODUAL_ANC;
    msg.msg_body.message_id = SIMPLE_PLAYER_DELAY_STOP_EVT;
    msg.msg_body.message_Param0 = 0;
    app_mailbox_put(&msg);
}

bool anc_enabled(void)
{
    return (anc_work_status == ANC_STATUS_ON);
}
//extern void anc_active_codec();

void app_anc_resample(uint32_t res_ratio, uint32_t *in, uint32_t *out, uint32_t samples)
{
    uint32_t flag = int_lock();
    for (int i = samples; i > 0; i--)
    {
        for (int j = 0; j < res_ratio; j++)
        {
            *(out + (i - 1) * res_ratio + j) = *(in + i - 1);
        }
    }
    int_unlock(flag);
}

void app_anc_select_coef(void)
{
    TRACE(2,"enter %s %d\n", __FUNCTION__, __LINE__);

#ifdef ANC_FF_ENABLED
    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx, ANC_FEEDFORWARD, ANC_GAIN_DELAY);
#endif

#ifdef ANC_FB_ENABLED
    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx, ANC_FEEDBACK, ANC_GAIN_DELAY);
#endif

#ifdef AUDIO_ANC_FB_MC_HW
    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx, ANC_MUSICCANCLE, ANC_GAIN_NO_DELAY);
#endif

    TRACE(2,"exit %s %d\n", __FUNCTION__, __LINE__);
}

void app_anc_enable(void)
{
    TRACE(2,"enter %s %d\n", __FUNCTION__, __LINE__);
//anc_active_codec();
#ifndef __SIMPLE_INTERNAL_PLAYER_SUPPORT__
    analog_aud_codec_speaker_enable(true);
#endif
#ifdef ANC_FF_ENABLED
    anc_open(ANC_FEEDFORWARD);
    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx, ANC_FEEDFORWARD, ANC_GAIN_DELAY);
#endif

#ifdef ANC_FB_ENABLED
    anc_open(ANC_FEEDBACK);
    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx, ANC_FEEDBACK, ANC_GAIN_DELAY);
#endif

#ifdef AUDIO_ANC_FB_MC_HW
    anc_open(ANC_MUSICCANCLE);
    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx, ANC_MUSICCANCLE, ANC_GAIN_NO_DELAY);
#endif

#if defined(ANC_WNR_ENABLED)
    if (is_sco_mode()) {
        anc_wnr_open(ANC_WNR_OPEN_MODE_CONFIGURE);
    } else {
        anc_wnr_open(ANC_WNR_OPEN_MODE_STANDALONE);
    }
#endif

    TRACE(2,"exit %s %d\n", __FUNCTION__, __LINE__);
}

void app_anc_disable(void)
{
    TRACE(2,"enter %s %d\n", __FUNCTION__, __LINE__);

#if defined(ANC_WNR_ENABLED)
    anc_wnr_close();
#endif

#ifdef ANC_FF_ENABLED
    anc_close(ANC_FEEDFORWARD);
#endif
#ifdef ANC_FB_ENABLED
    anc_close(ANC_FEEDBACK);
#endif

    TRACE(2,"exit %s %d\n", __FUNCTION__, __LINE__);
}

static void anc_sample_rate_change(enum AUD_STREAM_T stream, enum AUD_SAMPRATE_T rate, enum AUD_SAMPRATE_T *new_play, enum AUD_SAMPRATE_T *new_cap)
{
    enum AUD_SAMPRATE_T play_rate, cap_rate;

    if (anc_sample_rate[stream] != rate) {
#ifdef CHIP_BEST1000
        if (stream == AUD_STREAM_PLAYBACK) {
            play_rate = rate;
            cap_rate = rate * (anc_sample_rate[AUD_STREAM_CAPTURE] / anc_sample_rate[AUD_STREAM_PLAYBACK]);
        } else {
            play_rate = rate / (anc_sample_rate[AUD_STREAM_CAPTURE] / anc_sample_rate[AUD_STREAM_PLAYBACK]);
            cap_rate = rate;
        }
#else
        play_rate = rate;
        cap_rate = rate;
#ifdef ANC_FF_ENABLED
        anc_select_coef(play_rate,anc_coef_idx, ANC_FEEDFORWARD, ANC_GAIN_NO_DELAY);
#endif

#ifdef ANC_FB_ENABLED
        anc_select_coef(play_rate,anc_coef_idx, ANC_FEEDBACK, ANC_GAIN_NO_DELAY);
#endif

#ifdef AUDIO_ANC_FB_MC_HW
        anc_select_coef(play_rate,anc_coef_idx, ANC_MUSICCANCLE, ANC_GAIN_NO_DELAY);
#endif


#endif //CHIP_BEST1000

        TRACE(5,"%s: Update anc sample rate from %u/%u to %u/%u", __func__,
            anc_sample_rate[AUD_STREAM_PLAYBACK], anc_sample_rate[AUD_STREAM_CAPTURE], play_rate, cap_rate);

        if (new_play) {
            *new_play= play_rate;
        }
        if (new_cap) {
            *new_cap = cap_rate;
        }

        anc_sample_rate[AUD_STREAM_PLAYBACK] = play_rate;
        anc_sample_rate[AUD_STREAM_CAPTURE] = cap_rate;
    }
}

void app_anc_open_anc(void)
{
    enum AUD_SAMPRATE_T playback_rate;
    enum AUD_SAMPRATE_T capture_rate;
    AF_ANC_HANDLER handler;

    TRACE(2,"enter %s %d\n", __FUNCTION__, __LINE__);

#if 0// defined(__USE_AMP_MUTE_CTR__)//add by pang
    static bool init_open_anc=0;
    if(init_open_anc){   //relove the power on pop noise
	hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin); 
	osDelay(100);
    }
#endif

    handler = anc_sample_rate_change;

#ifdef __SIMPLE_INTERNAL_PLAYER_SUPPORT__
    app_start_player_by_anc();
#endif

    if (anc_sample_rate[AUD_STREAM_PLAYBACK] == AUD_SAMPRATE_NULL) {
#ifdef CHIP_BEST1000
        anc_sample_rate[AUD_STREAM_PLAYBACK] = AUD_SAMPRATE_96000;
        anc_sample_rate[AUD_STREAM_CAPTURE] = AUD_SAMPRATE_384000;
#else // !CHIP_BEST1000
        anc_sample_rate[AUD_STREAM_PLAYBACK] = AUD_SAMPRATE_44100;//AUD_SAMPRATE_48000;//m by pang for pop noise
        anc_sample_rate[AUD_STREAM_CAPTURE] = AUD_SAMPRATE_44100;//AUD_SAMPRATE_48000;
#endif
        anc_sample_rate[AUD_STREAM_PLAYBACK] = hal_codec_anc_convert_rate(anc_sample_rate[AUD_STREAM_PLAYBACK]);
        anc_sample_rate[AUD_STREAM_CAPTURE] = hal_codec_anc_convert_rate(anc_sample_rate[AUD_STREAM_CAPTURE]);
    }

    playback_rate = anc_sample_rate[AUD_STREAM_PLAYBACK];
    capture_rate = anc_sample_rate[AUD_STREAM_CAPTURE];

    pmu_anc_config(1);

#ifdef ANC_FF_ENABLED
    af_anc_open(ANC_FEEDFORWARD, playback_rate, capture_rate, handler);
    anc_open(ANC_FEEDFORWARD);
    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx, ANC_FEEDFORWARD, ANC_GAIN_DELAY);
#endif

#ifdef ANC_FB_ENABLED
    af_anc_open(ANC_FEEDBACK, playback_rate, capture_rate, handler);
    anc_open(ANC_FEEDBACK);
    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx, ANC_FEEDBACK, ANC_GAIN_DELAY);
#endif

#ifdef AUDIO_ANC_FB_MC_HW
    anc_open(ANC_MUSICCANCLE);
    anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx, ANC_MUSICCANCLE, ANC_GAIN_NO_DELAY);
#endif

#if defined(ANC_WNR_ENABLED)
    if (is_sco_mode()) {
        anc_wnr_open(ANC_WNR_OPEN_MODE_CONFIGURE);
    } else {
        anc_wnr_open(ANC_WNR_OPEN_MODE_STANDALONE);
    }
 #endif

 #if 0//defined(__USE_AMP_MUTE_CTR__)//add by pang  
 if(init_open_anc){
	hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin);  
 }
 init_open_anc=1;
#endif

    TRACE(2,"exit %s %d\n", __FUNCTION__, __LINE__);
}

void app_anc_close_anc(void)
{
    TRACE(2,"enter %s %d\n", __FUNCTION__, __LINE__);
    anc_set_dac_pa_delay = false;
#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
    if (anc_single_mode_is_on())
    {
        analog_aud_codec_speaker_enable(false);
    }
#endif

#ifdef ANC_WNR_ENABLED
    anc_wnr_close();
#endif

#ifdef ANC_FF_ENABLED
    anc_close(ANC_FEEDFORWARD);
    af_anc_close(ANC_FEEDFORWARD);
#endif

#ifdef ANC_FB_ENABLED
    anc_close(ANC_FEEDBACK);
    af_anc_close(ANC_FEEDBACK);
#endif

    pmu_anc_config(0);
#ifdef __SIMPLE_INTERNAL_PLAYER_SUPPORT__
    app_stop_player_by_anc();
#endif
    TRACE(2,"exit %s %d\n", __FUNCTION__, __LINE__);
}

void app_anc_bitrate_reopen(void)
{
    TRACE(2," %s status %d", __func__, anc_work_status);
    if (anc_work_status == ANC_STATUS_INIT_ON)
    {
//        hal_codec_reconfig_pll_freq(playback_rate, capture_rate);
    }
    else if (anc_work_status == ANC_STATUS_ON)
    {
    //    hal_codec_reconfig_pll_freq(playback_rate, capture_rate);
    }
    else if (anc_work_status == ANC_STATUS_WAITING_ON)
    {
    }
    else
    {
        TRACE(0,"no deal situation.");
    }
}

void app_anc_status_change(void)
{
    TRACE(2," %s status: %d ", __func__, anc_work_status);
    switch (anc_work_status)
    {
        case ANC_STATUS_OFF:
#if defined(IBRT) && defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
            if((app_anc_cmd_from_phone == OP_TOTA_ANC_ONOFF_WRITE_CMD))
            {
                if(app_anc_param_from_phone == 0)
                {
                    TRACE(1," %s onoff command invaild\n", __func__);
                    return;
                }
            }
#endif
            anc_work_status = ANC_STATUS_WAITING_ON;
            app_anc_timer_set(ANC_EVENT_OPEN, anc_switch_on_time);
            app_anc_open_anc();
            break;
        case ANC_STATUS_ON:
		case ANC_STATUS_WAITING_ON://add by pang

#if (ANC_COEF_LIST_NUM > 1)

#if defined(IBRT) && defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
            if((app_anc_cmd_from_phone == OP_TOTA_ANC_ONOFF_WRITE_CMD))
            {
                if(app_anc_param_from_phone == 0)
                {
                    app_anc_timer_set(ANC_EVENT_CLOSE, anc_close_delay_time);
                    app_anc_gain_fadeout();
                    anc_work_status = ANC_STATUS_WAITING_OFF;
                    return;
                }
                else
                {
                    TRACE(1," %s onoff command invaild\n", __func__);
                    return;
                }
            }

            if(app_anc_cmd_from_phone == OP_TOTA_ANC_LEVEL_WRITE_CMD)
            {
                if((app_anc_param_from_phone >= 0) && (app_anc_param_from_phone <= 7))
                {
                    anc_coef_idx = (app_anc_param_from_phone/2);
                }
                else
                {
                    TRACE(1," %s coef parameter invaild\n", __func__);
                    return;
                }
            }
#endif


#if defined(IBRT) && defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
            if (anc_coef_idx < ANC_COEF_LIST_NUM)
#else
            if (++anc_coef_idx < ANC_COEF_LIST_NUM)
#endif
            {
                TRACE(2," %s set coef idx: %d ", __func__, anc_coef_idx);

#ifdef ANC_MODE_SWITCH_WITHOUT_FADE

#ifdef ANC_FF_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
#endif
#ifdef ANC_FB_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);
#endif
#ifdef AUDIO_ANC_FB_MC_HW
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_MUSICCANCLE,ANC_GAIN_NO_DELAY);
#endif

                //recommand to play "ANC SWITCH" prompt here...

#else
                osSignalSet(anc_fade_thread_tid,CHANGE_FROM_ANC_TO_TT_DIRECTLY);
#endif
            }
            else
            {
                anc_coef_idx = 0;
                app_anc_timer_set(ANC_EVENT_CLOSE, anc_close_delay_time);
                app_anc_gain_fadeout();
                anc_work_status = ANC_STATUS_WAITING_OFF;
            }
#else
            anc_coef_idx = 0;
            app_anc_timer_set(ANC_EVENT_CLOSE, anc_close_delay_time);
            app_anc_gain_fadeout();
            anc_work_status = ANC_STATUS_WAITING_OFF;
#endif
            break;
        case ANC_STATUS_INIT_ON:
		case ANC_STATUS_WAITING_OFF://add by pang
#if defined(IBRT) && defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
            if((app_anc_cmd_from_phone == OP_TOTA_ANC_ONOFF_WRITE_CMD))
            {
                if(app_anc_param_from_phone == 0)
                {
                    TRACE(1," %s onoff command invaild\n", __func__);
                    return;
                }
            }
#endif
            app_anc_select_coef();
            app_anc_gain_fadein();
            anc_work_status = ANC_STATUS_WAITING_ON;
            app_anc_timer_close();
            break;
        default:
            break;
    };
}


#if defined(IBRT) && defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
static void app_anc_report_anc_param_to_phone(void)
{
    uint8_t buf[2] = {0};

    switch(anc_work_status)
    {
        case ANC_STATUS_WAITING_ON:
        case ANC_STATUS_ON:
            buf[0] = 1;
        break;
        case ANC_STATUS_WAITING_OFF:
        case ANC_STATUS_INIT_ON:
        case ANC_STATUS_OFF:
            buf[0] = 0;
        break;
        default:break;
    }

    buf[1] = (uint8_t)anc_coef_idx;

    TRACE(3,"[%s] onoff:%d level:%d\n", __func__, buf[0], buf[1]);
    app_tota_send_command(OP_TOTA_ANC_STATUS_REPORT_CMD, buf, 2, APP_TOTA_VIA_SPP);
    osDelay(50);
}
#endif

static int app_anc_handle_process(APP_MESSAGE_BODY *msg_body)
{
    uint32_t evt = msg_body->message_id;
    uint32_t arg0 = msg_body->message_Param0;

    TRACE(4," %s evt: %d, arg0: %d , anc status :%d", __func__, evt, arg0, anc_work_status);

    switch (evt)
    {
        case ANC_EVENT_INIT:
            //init anc_timer
            app_anc_init_timer();
            //init anc&open codec
#ifdef __AUDIO_SECTION_SUPPT__
            anc_load_cfg();
#endif

#ifdef __ANC_INIT_ON__
            app_anc_open_anc();
            anc_work_status = ANC_STATUS_INIT_ON;
            app_anc_timer_set(ANC_EVENT_CLOSE, anc_init_switch_off_time);
#else
            anc_work_status = ANC_STATUS_OFF;
#endif
#ifdef __ANC_STICK_SWITCH_USE_GPIO__
            if (app_anc_switch_get_val() == ANC_SWITCH_ON_LEVEL)
            {
                app_anc_switch_set_edge(ANC_SWITCH_ON_LEVEL);
                app_anc_status_post(ANC_SWITCH_ON_LEVEL);
            }
            else
            {
                app_anc_switch_set_edge(!(bool)ANC_SWITCH_ON_LEVEL);
            }
#endif

            anc_fade_thread_tid = osThreadCreate(osThread(anc_fade_thread),NULL);
            break;
        case ANC_EVENT_FADE_IN:
        case ANC_EVENT_FADE_OUT:
            anc_gain_fade_handle();
            if (evt == ANC_EVENT_FADE_IN)
            {
                anc_work_status = ANC_STATUS_ON;
                //recommand to play "ANC ON" prompt here...
            }
            if (evt == ANC_EVENT_FADE_OUT)
            {
                anc_work_status = ANC_STATUS_INIT_ON;
                //recommand to play "ANC OFF" prompt here...
            }
            break;
        case ANC_EVENT_CHANGE_SAMPLERATE:
            app_anc_bitrate_reopen();
            break;
        case ANC_EVENT_CHANGE_STATUS:

#ifdef __ANC_STICK_SWITCH_USE_GPIO__
            osDelay(10);
            if (app_anc_switch_get_val() != (bool)arg0)
            { //debonce
                TRACE(0,"io level not equeue exit");
                break;
            }

#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
            if (anc_single_mode_is_on())
            {
                if (arg0 != ANC_SWITCH_ON_LEVEL)
                {
                    TRACE(1," Anc ON but exit %d ", app_init_done);
                    if (app_init_done)
                    {
                        app_anc_shutdown = true;
                        app_shutdown();
                    }
                    else
                    {
                        app_anc_timer_set(ANC_EVENT_SWITCH_KEY_DEBONCE, anc_switch_key_debonce_time);
                    }
                    return 0;
                }
            }
#endif
            if (arg0 != ANC_SWITCH_ON_LEVEL)
                app_anc_switch_set_edge((!(bool)ANC_SWITCH_ON_LEVEL));
            else
                app_anc_switch_set_edge(ANC_SWITCH_ON_LEVEL);

            if (((arg0 == ANC_SWITCH_ON_LEVEL) && (anc_work_status == ANC_STATUS_ON)) ||
                ((arg0 != ANC_SWITCH_ON_LEVEL) && (anc_work_status == ANC_STATUS_OFF)))
            {
                //status same, no handle
                TRACE(0," Anc NOT ON, exit");
                return 0;
            }
#endif

#if defined(IBRT)
            if((BOOL)arg0 == app_anc_work_status())
                return 0;

#if defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
            app_anc_cmd_from_phone = msg_body->message_Param1;
            app_anc_param_from_phone = msg_body->message_Param2;
#endif
#endif
            app_anc_status_change();

            break;
        case ANC_EVENT_SYNC_STATUS:
#if defined(IBRT)

#if defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
            if(msg_body->message_Param1 == OP_TOTA_ANC_STATUS_SYNC_CMD)
            {
                app_anc_report_anc_param_to_phone();
                return 0;
            }
#endif

            TRACE(3," %s status %d ---> %d ", __func__, anc_work_status, arg0);
            if(ANC_STATUS_ON==arg0)
                arg0 = 1;
            else
                arg0 = 0;
            tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_ANC_STATUS,(uint8_t *)&arg0, sizeof(uint32_t));
#endif
            break;
        case ANC_EVENT_SYNC_INIT:
#if defined(IBRT)
            app_anc_status_sync_init();
#endif
            break;
        case ANC_EVENT_HOWL_PROCESS:
            //disable anc,set max gain,enable anc
            {
                //uint32_t howl = arg0;
                if (anc_work_status == ANC_STATUS_ON)
                {
                    //__anc_usb_app_howl_set_gain(howl);
                }
            }
            break;
#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
        case ANC_EVENT_PWR_KEY_MONITOR:
            osDelay(10);

            if (app_pwr_key_monitor_get_val() != (bool)arg0) //debonce
                return 0;

            if ((bool)arg0)
            {
                app_anc_timer_set(ANC_EVENT_PWR_KEY_MONITOR_REBOOT, anc_pwr_key_monitor_time);
                app_pwr_key_monitor_set_int_edge(1);
            }
            else
            {
                hwtimer_stop(anc_timerid);
                app_pwr_key_monitor_set_int_edge(0);
            }

            break;
#endif
        case SIMPLE_PLAYER_DELAY_STOP_EVT:
#ifdef __SIMPLE_INTERNAL_PLAYER_SUPPORT__
            simple_player_delay_stop();
#endif
            break;
        case SIMPLE_PLAYER_CLOSE_CODEC_EVT:
            app_anc_close_anc();
            anc_work_status = ANC_STATUS_OFF;
            break;
        default:break;
    }
    return 0;
}

static void anc_gain_fade_timer_handler(void *p)
{
    TRACE(2," %s , request %d ", __func__, anc_timer_request);
    switch (anc_timer_request)
    {
    case ANC_EVENT_OPEN:
        app_anc_gain_fadein();
        break;
    case ANC_EVENT_CLOSE:
        //real close
        //app_anc_close_anc();
        //anc_work_status = ANC_STATUS_OFF;
        app_anc_post_anc_codec_close();

        break;
    case ANC_EVENT_PWR_KEY_MONITOR_REBOOT:
        TRACE(0," reboot !");
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT_ANC_ON);
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT_BT_ON);
        app_reset();
        break;
    case ANC_EVENT_SWITCH_KEY_DEBONCE:
        TRACE(0," ANC SWITCH KEY ");
#ifdef __ANC_STICK_SWITCH_USE_GPIO__
        if (app_switch_callback)
            app_switch_callback(hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)ANC_SWITCH_PIN));
#endif
        break;
    default:
        break;
    }
    anc_timer_request = ANC_EVENT_NONE;
    hwtimer_stop(anc_timerid);
}

void app_anc_init_timer(void)
{
    if (anc_timerid == NULL)
        anc_timerid = hwtimer_alloc((HWTIMER_CALLBACK_T)anc_gain_fade_timer_handler, &anc_timer_request);
}

void app_anc_timer_set(uint32_t request, uint32_t delay)
{
    TRACE(3," %s request %d , delay %ds ", __func__, request, (TICKS_TO_MS(delay) / 1000));
    if (anc_timerid == NULL)
        return;
    anc_timer_request = request;
    hwtimer_stop(anc_timerid);
    hwtimer_start(anc_timerid, delay);
}

void app_anc_timer_close(void)
{
    TRACE(3," %s enter...", __func__);
    if (anc_timerid == NULL)
    {
        return;
    }
    
    anc_timer_request = ANC_EVENT_NONE;
    hwtimer_stop(anc_timerid);
}

void app_anc_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(3," %s anc_work_st: %d , timer %d", __func__, anc_work_status, anc_timer_request);

    bool temp = app_anc_work_status();

#if defined(IBRT)
    app_anc_status_sync(!temp);
#endif
    app_anc_status_post(!temp);
}

/** add by pang **/
void app_anc_Key_Pro(void)
{
    static bool power_anc_init=0;
	bool anc_open_flag=0;
	//TRACE(2," %s anc_current_mode: %d", __func__, anc_current_mode);
	if(anc_current_mode == monitor)
	{	
		anc_current_mode = anc_on; 
		anc_coef_idx = anc_on_mode;
		
		anc_open_flag=1;

		if(power_anc_init){
			//power_anc_init=0;
		}
		else{
#ifdef ANC_LED_PIN
		app_anc_switch_turnled(true);
		app_monitor_switch_turnled(false);
#endif
#ifdef MEDIA_PLAYER_SUPPORT		
		app_voice_report(APP_STATUS_INDICATION_ANC_ON, 0);
#endif
		}
	}
	else if(anc_current_mode == anc_off)
	{
		anc_current_mode = monitor;
		anc_coef_idx = monitor_mode;
		anc_open_flag=1;
#ifdef ANC_LED_PIN
		app_monitor_switch_turnled(true);
		app_anc_switch_turnled(false);
#endif
#ifdef MEDIA_PLAYER_SUPPORT
		app_voice_report(APP_STATUS_INDICATION_AWARENESS_ON, 0);
#endif
	}
	else
	{
		anc_current_mode = anc_off;
#ifdef ANC_LED_PIN
		app_anc_switch_turnled(false);
#endif		
#ifdef MEDIA_PLAYER_SUPPORT
		app_voice_report(APP_STATUS_INDICATION_ANC_OFF, 0);
#endif
	}

	switch (anc_work_status)
    {
        case ANC_STATUS_OFF:
			if(anc_open_flag){				
				TRACE(1," %s ANC_STATUS_OFF--ON", __func__);
				
				if(power_anc_init){
            		anc_work_status = ANC_STATUS_WAITING_ON;
            		app_anc_timer_set(ANC_EVENT_OPEN, MS_TO_TICKS(4000));
				}
				else{
					anc_work_status = ANC_STATUS_WAITING_ON;
            	    app_anc_timer_set(ANC_EVENT_OPEN, anc_switch_on_time);
				}
            	app_anc_open_anc();
			}
            break;
        case ANC_STATUS_ON: 
		case ANC_STATUS_WAITING_ON:
			if(anc_open_flag){			
				TRACE(2," %s ANC_STATUS_ON:--set coef idx: %d ", __func__, anc_coef_idx);
#ifdef ANC_MODE_SWITCH_WITHOUT_FADE

#ifdef ANC_FF_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
#endif
#ifdef ANC_FB_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);
#endif
#ifdef AUDIO_ANC_FB_MC_HW
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_MUSICCANCLE,ANC_GAIN_NO_DELAY);
#endif
#else
                osSignalSet(anc_fade_thread_tid,CHANGE_FROM_ANC_TO_TT_DIRECTLY);
#endif
            }
            else
            {    
				TRACE(1," %s ANC_STATUS_ON:--fadout", __func__);
                app_anc_timer_set(ANC_EVENT_CLOSE, anc_close_delay_time);
                app_anc_gain_fadeout();
                anc_work_status = ANC_STATUS_WAITING_OFF;
            }
            break;
        case ANC_STATUS_INIT_ON:
		case ANC_STATUS_WAITING_OFF:
			if(anc_open_flag){		
				TRACE(1," %s ANC_STATUS_INIT_ON:--fadin", __func__);
            	app_anc_select_coef();
            	app_anc_gain_fadein();
            	anc_work_status = ANC_STATUS_WAITING_ON;
            	app_anc_timer_close();
			}
            break;
        default:
            break;
    };
	if(power_anc_init){
		power_anc_init=0;
	}
	#if defined(__TPV_APP__)
	Notification_Nosie_Cancelling_Change();
	#endif
}

#if 0
void app_anc_Key_Pro(APP_KEY_STATUS *status, void *param)
{
    static bool power_anc_init=1;
	bool anc_open_flag=0;
	
	if(anc_current_mode != anc_on1)
	{	
		anc_current_mode = anc_on1; 
		anc_coef_idx = 0;
	    anc_open_flag=1;

		if(power_anc_init){
			power_anc_init=0;
		}
		else{
#ifdef ANC_LED_PIN
		app_anc_switch_turnled(true);
		app_monitor_switch_turnled(false);
#endif
#ifdef MEDIA_PLAYER_SUPPORT		
		app_voice_report(APP_STATUS_INDICATION_ANC_ON, 0);
#endif
		}
	}
	else
	{
		anc_current_mode = anc_off;
#ifdef ANC_LED_PIN
		app_anc_switch_turnled(false);
#endif		
#ifdef MEDIA_PLAYER_SUPPORT
		app_voice_report(APP_STATUS_INDICATION_ANC_OFF, 0);
#endif
	}

	switch (anc_work_status)
    {
        case ANC_STATUS_OFF:
			if(anc_open_flag){				
				TRACE(1," %s ANC_STATUS_OFF--ON", __func__);
            	anc_work_status = ANC_STATUS_WAITING_ON;
            	app_anc_timer_set(ANC_EVENT_OPEN, anc_switch_on_time);
            	app_anc_open_anc();
			}
            break;
        case ANC_STATUS_ON: 
		case ANC_STATUS_WAITING_ON:
			if(anc_open_flag){			
				TRACE(2," %s ANC_STATUS_ON:--set coef idx: %d ", __func__, anc_coef_idx);
#ifdef ANC_MODE_SWITCH_WITHOUT_FADE

#ifdef ANC_FF_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
#endif
#ifdef ANC_FB_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);
#endif
#ifdef AUDIO_ANC_FB_MC_HW
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_MUSICCANCLE,ANC_GAIN_NO_DELAY);
#endif
#else
                osSignalSet(anc_fade_thread_tid,CHANGE_FROM_ANC_TO_TT_DIRECTLY);
#endif
            }
            else
            {    
				TRACE(1," %s ANC_STATUS_ON:--fadout", __func__);
                app_anc_timer_set(ANC_EVENT_CLOSE, anc_close_delay_time);
                app_anc_gain_fadeout();
                anc_work_status = ANC_STATUS_WAITING_OFF;
            }
            break;
        case ANC_STATUS_INIT_ON:
		case ANC_STATUS_WAITING_OFF:
			if(anc_open_flag){		
				TRACE(1," %s ANC_STATUS_INIT_ON:--fadin", __func__);
            	app_anc_select_coef();
            	app_anc_gain_fadein();
            	anc_work_status = ANC_STATUS_WAITING_ON;
            	app_anc_timer_close();
			}
            break;
        default:
            break;
    };
}

void app_monitor_Key_Pro(APP_KEY_STATUS *status, void *param)
{
	bool anc_open_flag=0;

	if(anc_current_mode != anc_on2)
	{
		anc_current_mode = anc_on2;
		anc_coef_idx = 1;
		anc_open_flag=1;
#ifdef ANC_LED_PIN
		app_monitor_switch_turnled(true);
		app_anc_switch_turnled(false);
#endif
#ifdef MEDIA_PLAYER_SUPPORT
		app_voice_report(APP_STATUS_INDICATION_GSOUND_MIC_OPEN, 0);
#endif		
	}
	else
	{
		anc_current_mode = anc_off;	
#ifdef ANC_LED_PIN
		app_monitor_switch_turnled(false);
#endif
#ifdef MEDIA_PLAYER_SUPPORT
		app_voice_report(APP_STATUS_INDICATION_GSOUND_MIC_CLOSE, 0);
#endif
	}

	switch (anc_work_status)
    {
        case ANC_STATUS_OFF:
			if(anc_open_flag){				
				TRACE(1," %s ANC_STATUS_OFF--ON", __func__);
            	anc_work_status = ANC_STATUS_WAITING_ON;
            	app_anc_timer_set(ANC_EVENT_OPEN, anc_switch_on_time);
            	app_anc_open_anc();
			}
            break;
        case ANC_STATUS_ON: 
		case ANC_STATUS_WAITING_ON:
			if(anc_open_flag){			
				TRACE(2," %s ANC_STATUS_ON:--set coef idx: %d ", __func__, anc_coef_idx);
#ifdef ANC_MODE_SWITCH_WITHOUT_FADE

#ifdef ANC_FF_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
#endif
#ifdef ANC_FB_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);
#endif
#ifdef AUDIO_ANC_FB_MC_HW
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_MUSICCANCLE,ANC_GAIN_NO_DELAY);
#endif
#else
                osSignalSet(anc_fade_thread_tid,CHANGE_FROM_ANC_TO_TT_DIRECTLY);
#endif
            }
            else
            {    
				TRACE(1," %s ANC_STATUS_ON:--fadout", __func__);
                app_anc_timer_set(ANC_EVENT_CLOSE, anc_close_delay_time);
                app_anc_gain_fadeout();
                anc_work_status = ANC_STATUS_WAITING_OFF;
            }
            break;
        case ANC_STATUS_INIT_ON:
		case ANC_STATUS_WAITING_OFF:
			if(anc_open_flag){		
				TRACE(1," %s ANC_STATUS_INIT_ON:--fadin", __func__);
            	app_anc_select_coef();
            	app_anc_gain_fadein();
            	anc_work_status = ANC_STATUS_WAITING_ON;
            	app_anc_timer_close();
			}
            break;
        default:
            break;
    };
}
#endif

void set_anc_mode(uint8_t anc_new_mode)
{
	static bool power_anc_init=1;
	bool anc_open_flag=0;

	if(power_anc_init==0){
		//if(anc_current_mode==anc_new_mode)
			//return;
	}

	if(anc_new_mode == anc_on)
	{	
		anc_current_mode = anc_on; 
		anc_coef_idx = anc_on_mode;
	    anc_open_flag=1;
	}
	else if(anc_new_mode == monitor)
	{
		anc_current_mode = monitor;
		anc_coef_idx = monitor_mode;
		anc_open_flag=1;	
	}
	else
	{
		anc_current_mode = anc_off;
	}

	switch (anc_work_status)
    {
        case ANC_STATUS_OFF:
			if(anc_open_flag){				
				TRACE(1," %s ANC_STATUS_OFF--ON", __func__);
				
				anc_work_status = ANC_STATUS_WAITING_ON;
				if(power_anc_init){
            		app_anc_timer_set(ANC_EVENT_OPEN, MS_TO_TICKS(6000));
				}
				else{
            		app_anc_timer_set(ANC_EVENT_OPEN, anc_switch_on_time);
				}
            	app_anc_open_anc();
			}
            break;
        case ANC_STATUS_ON: 
		case ANC_STATUS_WAITING_ON:
			if(anc_open_flag){			
				TRACE(2," %s ANC_STATUS_ON:--set coef idx: %d ", __func__, anc_coef_idx);
#ifdef ANC_MODE_SWITCH_WITHOUT_FADE

#ifdef ANC_FF_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
#endif
#ifdef ANC_FB_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);
#endif
#ifdef AUDIO_ANC_FB_MC_HW
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_MUSICCANCLE,ANC_GAIN_NO_DELAY);
#endif
#else
                osSignalSet(anc_fade_thread_tid,CHANGE_FROM_ANC_TO_TT_DIRECTLY);
#endif
            }
            else
            {    
				TRACE(1," %s ANC_STATUS_ON:--fadout", __func__);
                app_anc_timer_set(ANC_EVENT_CLOSE, anc_close_delay_time);
                app_anc_gain_fadeout();
                anc_work_status = ANC_STATUS_WAITING_OFF;
            }
            break;
        case ANC_STATUS_INIT_ON:
		case ANC_STATUS_WAITING_OFF:
			if(anc_open_flag){		
				TRACE(1," %s ANC_STATUS_INIT_ON:--fadin", __func__);
            	app_anc_select_coef();
            	app_anc_gain_fadein();
            	anc_work_status = ANC_STATUS_WAITING_ON;
            	app_anc_timer_close();
			}
            break;
        default:
            break;
    }
	if(power_anc_init)
		power_anc_init=0;	
}


void app_monitor_moment(bool on)
{  	
    bool anc_open_flag=0;
	
	TRACE(2," %s status: %d ", __func__, anc_work_status);
	
	if(on){
		anc_coef_idx=12;
		anc_open_flag=1;		
	}
	else{
		TRACE(1,"******%s recover", __func__);		
		if(anc_current_mode==anc_on){	
			anc_coef_idx=anc_on_mode;
			anc_open_flag=1;
		}
		else if(anc_current_mode==monitor){
			anc_coef_idx=monitor_mode;
			anc_open_flag=1;
		}	
		else{
			anc_open_flag=0;
		}
	}
	
	switch (anc_work_status)
    {
        case ANC_STATUS_OFF:
			if(anc_open_flag){				
				TRACE(1," %s ANC_STATUS_OFF--ON", __func__);
            	anc_work_status = ANC_STATUS_WAITING_ON;
            	app_anc_timer_set(ANC_EVENT_OPEN, anc_switch_on_time);
            	app_anc_open_anc();
			}
            break;
        case ANC_STATUS_ON: 
		case ANC_STATUS_WAITING_ON:
			if(anc_open_flag){			
				TRACE(2," %s ANC_STATUS_ON:--set coef idx: %d ", __func__, anc_coef_idx);
#ifdef ANC_MODE_SWITCH_WITHOUT_FADE

#ifdef ANC_FF_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
#endif
#ifdef ANC_FB_ENABLED
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);
#endif
#ifdef AUDIO_ANC_FB_MC_HW
                anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_MUSICCANCLE,ANC_GAIN_NO_DELAY);
#endif

                //recommand to play "ANC SWITCH" prompt here...

#else
                osSignalSet(anc_fade_thread_tid,CHANGE_FROM_ANC_TO_TT_DIRECTLY);
#endif
            }
            else
            {    
				TRACE(1," %s ANC_STATUS_ON:--fadout", __func__);
                app_anc_timer_set(ANC_EVENT_CLOSE, anc_close_delay_time);
                app_anc_gain_fadeout();
                anc_work_status = ANC_STATUS_WAITING_OFF;
            }
            break;
        case ANC_STATUS_INIT_ON:
		case ANC_STATUS_WAITING_OFF:
			if(anc_open_flag){		
				TRACE(1," %s ANC_STATUS_INIT_ON:--fadin", __func__);
            	app_anc_select_coef();
            	app_anc_gain_fadein();
            	anc_work_status = ANC_STATUS_WAITING_ON;
            	app_anc_timer_close();
			}
            break;
        default:
            break;
    };
}

void app_anc_power_off(void)
{
	if((anc_work_status != ANC_STATUS_OFF)&&(anc_work_status != ANC_STATUS_WAITING_OFF)){
		anc_disable_gain_updated_when_pass0(0);
		while(__anc_usb_app_fadeout_ff_fb());
		app_anc_close_anc();
	}
}
/** end add **/

void app_anc_ios_init(void)
{
#ifdef __ANC_STICK_SWITCH_USE_GPIO__
    if (cfg_anc_switch.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_anc_switch, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_anc_switch.pin, HAL_GPIO_DIR_IN, 0);
    }
#endif
#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
    if (cfg_pwr_key_monitor.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_pwr_key_monitor, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_pwr_key_monitor.pin, HAL_GPIO_DIR_IN, 0);
    }
    if (cfg_anc_led.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_anc_led, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_anc_led.pin, HAL_GPIO_DIR_OUT, 0);
    }

#endif
#ifdef ANC_SWITCH_PIN
    uint8_t gpio_val = hal_gpio_pin_get_val(ANC_SWITCH_PIN);
    if (gpio_val)
    {
        app_anc_switch_turnled(true);
        TRACE(1," %s turn on led", __func__);
    }
    else
    {
        app_anc_switch_turnled(false);
        TRACE(1," %s turn off led", __func__);
    }
#endif
}

int app_anc_open_module(void)
{
    TRACE(1,"%s ", __func__);
    //set app module
    app_set_threadhandle(APP_MODUAL_ANC, app_anc_handle_process);
    app_anc_do_init();
#ifdef __ANC_STICK_SWITCH_USE_GPIO__
    app_anc_switch_init(app_anc_status_post);
    app_anc_switch_set_edge(app_anc_switch_get_val());
#endif
#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
    if (anc_single_mode_is_on())
    {
        // app_pwr_key_monitor_init(app_anc_send_pwr_key_monitor_evt);
        //  app_pwr_key_monitor_set_int_edge(app_pwr_key_monitor_get_val());
    }
#endif

    return 0;
}

int app_anc_close_module(void)
{
    TRACE(1," %s ", __func__);

    anc_timer_request = ANC_EVENT_NONE;
    if (anc_timerid)
    {
        hwtimer_stop(anc_timerid);
        hwtimer_free(anc_timerid);
        anc_timerid = NULL;
    }
    if (app_anc_get_anc_status() != ANC_STATUS_OFF)
    {
        anc_work_status = ANC_STATUS_OFF;
        app_anc_disable();
        app_anc_close_anc();
    }
    app_set_threadhandle(APP_MODUAL_ANC, NULL);
    return 0;
}

enum AUD_SAMPRATE_T app_anc_get_play_rate(void)
{
    return anc_sample_rate[AUD_STREAM_PLAYBACK];
}

bool app_anc_work_status(void)
{
   // TRACE(2," %s st %d", __func__, anc_work_status);
    return (anc_work_status == ANC_STATUS_ON || anc_work_status == ANC_STATUS_WAITING_ON);
}

uint32_t app_anc_get_anc_status(void)
{
    return anc_work_status;
}

void test_anc(void)
{
    app_anc_open_anc();
    app_anc_enable();
}

void test_anc_switch_coef(void)
{
    anc_coef_idx++;

    TRACE(2," %s set coef idx: %d ", __func__, anc_coef_idx);

    if (ANC_STATUS_OFF == anc_work_status || ANC_STATUS_INIT_ON == anc_work_status)
    {
        app_anc_status_change();
        anc_coef_idx = 0;
        return;
    }
    if (anc_coef_idx < (ANC_COEF_LIST_NUM))
    {
#ifdef ANC_FF_ENABLED
        while (__anc_usb_app_fadeout(ANC_FEEDFORWARD))
            ;
#endif
#ifdef ANC_FB_ENABLED
        while (__anc_usb_app_fadeout(ANC_FEEDBACK))
            ;
#endif

        app_anc_disable();
        anc_select_coef(anc_sample_rate[AUD_STREAM_PLAYBACK],anc_coef_idx,ANC_FEEDFORWARD,ANC_GAIN_DELAY);
        app_anc_enable();

#ifdef ANC_FF_ENABLED
        while (__anc_usb_app_fadein(ANC_FEEDFORWARD))
            ;
#endif
#ifdef ANC_FB_ENABLED
        while (__anc_usb_app_fadein(ANC_FEEDBACK))
            ;
#endif
    }
    else
    {
        anc_coef_idx = 0;
        app_anc_timer_set(ANC_EVENT_CLOSE, anc_close_delay_time);
        app_anc_gain_fadeout();
        anc_work_status = ANC_STATUS_WAITING_OFF;
    }
}

#if defined(IBRT)
void tws_anc_sync_status(void)
{
    app_anc_status_sync(app_anc_work_status());
}
#endif
