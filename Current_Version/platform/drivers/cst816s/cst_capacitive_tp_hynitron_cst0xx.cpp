/**
 *Name        : capacitive_tp_hynitron_cst0xx.c
 *Author      : gary
 *Version     : V1.0
 *Create      : 2017-11-9
 *Copyright   : zxzz
 */

/*****************************************************************/
#include "plat_types.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "hal_trace.h" 
#include "hwtimer_list.h"
#include "hal_timer.h" 
#include "hal_iomux.h" 
#include "hal_gpio.h" 
#include "hal_i2c.h"
#include "string.h"
#include "btapp.h"
#include "app_thread.h"
#include "app_status_ind.h"
#include "hal_codec.h"
#include "app_anc.h"
#include "apps.h"
#include "app_user.h"
#include "app_bt_stream.h"


#include "cst_capacitive_tp_hynitron_cst0xx.h"
#include "cst_ctp_hynitron_ext.h"

extern struct BT_DEVICE_T  app_bt_device;

/*****************************************************************/
#if defined(__CST816S_TOUCH__)

#define CTP_DBG
#ifdef CTP_DBG
#define CTP_DBG_TRACE(n,str, ...)	TRACE(n,str, ##__VA_ARGS__)
#else
#define CTP_DBG_TRACE(n,str, ...)  	TRACE_DUMMY(n,str, ##__VA_ARGS__)
#endif

enum{
	TOUCH_EVENT_INIT=0,
	TOUCH_EVENT_PRO,
};

#define CST816S_RST_PIN     	HAL_IOMUX_PIN_P0_2
#define CST816S_INTR_DET_PIN    HAL_IOMUX_PIN_P1_4

uint8_t gesture_id=0;

void user_delay_ms(uint16_t Nms)
{
    hal_sys_timer_delay(MS_TO_TICKS(Nms));
}

//#define i2c_gpio

#ifdef i2c_gpio
struct HAL_GPIO_I2C_CONFIG_T cfg_gpio_i2c_cmt={
	HAL_GPIO_PIN_P2_0,HAL_GPIO_PIN_P2_1,100000
};
#else
static struct HAL_I2C_CONFIG_T _codec_i2c_cfg;
#endif

//#define i2c_mode_task

void user_i2c_init(uint32_t speed)
{
#ifdef i2c_gpio
		hal_gpio_i2c_open(&cfg_gpio_i2c_cmt);
#else
		hal_iomux_set_i2c0();

#ifdef i2c_mode_task
		_codec_i2c_cfg.mode = HAL_I2C_API_MODE_TASK;
#else
		_codec_i2c_cfg.mode = HAL_I2C_API_MODE_SIMPLE;
#endif
		_codec_i2c_cfg.use_dma	= 0;
		_codec_i2c_cfg.use_sync = 1;
		_codec_i2c_cfg.speed = speed;
		_codec_i2c_cfg.as_master = 1;
		hal_i2c_open(HAL_I2C_ID_0, &_codec_i2c_cfg);
#endif
}

uint32_t user_i2c_write(uint8_t uchDeviceId, const uint8_t uchWriteBytesArr[], uint16_t usWriteLen)
{

#ifdef i2c_gpio	
	return(hal_gpio_i2c_simple_send(uchDeviceId, uchWriteBytesArr, usWriteLen));
#else
	#ifdef i2c_mode_task
		return (hal_i2c_task_send(HAL_I2C_ID_0, uchDeviceId, uchWriteBytesArr, usWriteLen,0,0));
	#else
		return (hal_i2c_simple_send(HAL_I2C_ID_0, uchDeviceId, uchWriteBytesArr, usWriteLen));
	#endif
#endif
}

uint32_t user_i2c_read(uint8_t uchDeviceId, const uint8_t uchCmddBytesArr[], uint16_t usCmddLen, uint8_t uchReadBytesArr[], uint16_t usMaxReadLen)
{
#ifdef i2c_gpio	
	return (hal_gpio_i2c_simple_recv(uchDeviceId, uchCmddBytesArr, usCmddLen, uchReadBytesArr, usMaxReadLen));	
#else
	#ifdef i2c_mode_task
		return (hal_i2c_task_recv(HAL_I2C_ID_0, uchDeviceId, uchCmddBytesArr, usCmddLen, uchReadBytesArr, usMaxReadLen,0,0));	
	#else
		return (hal_i2c_simple_recv_cst(HAL_I2C_ID_0, uchDeviceId, uchCmddBytesArr, usCmddLen, uchReadBytesArr, usMaxReadLen));	
	#endif
#endif
}

static void cst816s_rst_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pin;
	
	pin.pin = CST816S_RST_PIN;
	pin.function = HAL_IOMUX_FUNC_GPIO;
	pin.pull_sel = HAL_IOMUX_PIN_NOPULL;
	pin.volt = HAL_IOMUX_PIN_VOLTAGE_VIO;
	hal_iomux_init(&pin, 1);
	hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)CST816S_RST_PIN, HAL_GPIO_DIR_OUT, 0);
}

void hal_set_cst816s_rst_high(void)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)CST816S_RST_PIN);
}

void hal_set_cst816s_rst_low(void)
{
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)CST816S_RST_PIN);
}

static void cst816s_int_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pin;
	
	pin.pin = CST816S_INTR_DET_PIN;
	pin.function = HAL_IOMUX_FUNC_GPIO;
	pin.pull_sel = HAL_IOMUX_PIN_NOPULL;
	pin.volt = HAL_IOMUX_PIN_VOLTAGE_VIO;
	hal_iomux_init(&pin, 1);
	hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)CST816S_INTR_DET_PIN, HAL_GPIO_DIR_IN, 1);
}

static void cst816s_int_handler(enum HAL_GPIO_PIN_T pin);
static void cst816s_int_irq(void)
{
	struct HAL_GPIO_IRQ_CFG_T cfg;
		
	cfg.irq_debounce = 1;
	cfg.irq_enable = 1;
	cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
	cfg.irq_handler = cst816s_int_handler;
	cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
		
	hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)CST816S_INTR_DET_PIN, &cfg);
}

static void cst816s_int_disable_irq(void)
{
	struct HAL_GPIO_IRQ_CFG_T cfg;
		
	cfg.irq_debounce = 1;
	cfg.irq_enable = 0;
	cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
	cfg.irq_handler = NULL;
	cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
		
	hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)CST816S_INTR_DET_PIN, &cfg);
}

static uint8_t hal_get_cst816s_int_level(void)
{
    return (hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)CST816S_INTR_DET_PIN));
}

static int touch_event_post(uint32_t id);
static void cst816s_int_handler(enum HAL_GPIO_PIN_T pin) 
{   	
	uint8_t temp[1];

	//CTP_DBG_TRACE(1,"******%s",__func__);	

	if (pin != (enum HAL_GPIO_PIN_T)CST816S_INTR_DET_PIN)
		return;
	
	cst816s_int_disable_irq();
	if(!hal_get_cst816s_int_level())
	{   
		gesture_id=0;

		temp[0]=0x01;
		user_i2c_read(CTP_SLAVER_ADDR, temp, 1, &gesture_id, 1);

		CTP_DBG_TRACE(2,"******%s: 0x%2x",__func__,gesture_id);		
				
		touch_event_post(TOUCH_EVENT_PRO);		
	}
	
	cst816s_int_irq();
}

bool ctp_hynitron_cst0_init(void)
{	 
	user_i2c_init(400000);//400000
	cst816s_rst_init();
	hal_set_cst816s_rst_low();
    user_delay_ms(10);
	hal_set_cst816s_rst_high();
	 
	cst816s_int_init();
    cst816s_int_irq();
	 
	CTP_DBG_TRACE(1,"******%s,init succeed!", __func__);
    return CTP_TRUE;
}

bool ctp_hynitron_cst0_power_on(bool enable)
{
	if (enable){
        hal_set_cst816s_rst_low();
		user_delay_ms(10);
        hal_set_cst816s_rst_high();
    }else{
        uint8_t enterSleep[2];
		
		enterSleep[0]=0xA5;
		enterSleep[1]=0x03;
		user_i2c_write(CTP_SLAVER_ADDR, enterSleep, 2);
    }
    return CTP_TRUE;
}

static void Key_pro(void)
{
	//if(app_get_touchlock())
		//return;

	if(app_poweroff_flag)
		return;

	if(app_apps_3p5jack_plugin_flag(0))
		return;

	switch(gesture_id)
	{
		case CTP_GESTURE_COVER_PRESS:
		CTP_DBG_TRACE(1,"******%s---COVER_PRESS",__func__);
		TOUCH_gesture_cover_press();
		break;
	case CTP_GESTURE_COVER_LEAVE:
		CTP_DBG_TRACE(1,"******%s---COVER_LEAVE",__func__);
		TOUCH_gesture_cover_leave();
		break;
	case CTP_GESTURE_UP:
	case CTP_GESTURE_UP_AND_HOLD:
		CTP_DBG_TRACE(1,"******%s---UP",__func__);
	    Touch_gesture_up();
		break;
	case CTP_GESTURE_DOWN:
	case CTP_GESTURE_DOWN_AND_HOLD:
		CTP_DBG_TRACE(1,"******%s---DOWN",__func__);
	    TOUCH_gesture_down();
		break;
	default:
		break;
	}

	switch(gesture_id)
	{
	case CTP_GESTURE_LEFT:
		CTP_DBG_TRACE(1,"******%s---LEFT",__func__);
		TOUCH_gesture_left();
		break;
	case CTP_GESTURE_RIGHT:
		CTP_DBG_TRACE(1,"******%s---RIGHT",__func__);
		TOUCH_gesture_right();
		break;
	case CTP_GESTURE_CLICK:
		CTP_DBG_TRACE(1,"******%s---CLICK",__func__);
		TOUCH_gesture_click();
		break;
	case CTP_GESTURE_DOUBLE:
		CTP_DBG_TRACE(1,"******%s---DOUBLE",__func__);
		TOUCH_gesture_double_click();	
		break;
	case CTP_GESTURE_LONG:
		CTP_DBG_TRACE(1,"******%s---LONG",__func__);
		TOUCH_gesture_long_press();
		break;
	case CTP_GESTURE_TRIPLE:
		CTP_DBG_TRACE(1,"******%s---TRIPLE",__func__);
		TOUCH_gesture_triple_click();
		break;
	
	default:
		break;
	}
}

static int touch_event_post(uint32_t id)
{
    APP_MESSAGE_BLOCK msg;

    msg.mod_id = APP_MODUAL_TOUCH;
    msg.msg_body.message_id = id;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_mailbox_put(&msg);
	
    return 0;
}

static int app_touch_handle_process(APP_MESSAGE_BODY *msg_body)
{   
	uint32_t evt = msg_body->message_id;

    switch (evt)
    {
        case TOUCH_EVENT_INIT:
			break;
		case TOUCH_EVENT_PRO:
			Key_pro();
			break;
	    default:
			break;
    }
	
	return 0;
}

void cst816s_update_fw(void)
{
	cst816s_rst_init();
	
    //ctp_hynitron_update();
}

void cst816s_open_module(void)
{	
	CTP_DBG_TRACE(1,"******%s",__func__);
	
	ctp_hynitron_cst0_init();
	
	app_set_threadhandle(APP_MODUAL_TOUCH, app_touch_handle_process);	
}


#if 0
#if !defined(IC_MODULE_TEST)
extern kal_uint32 /*lint -e(526)*/L1I_GetTimeStamp(void);
#endif

/*****************************************************************/

CTP_parameters_struct CTP_parameters;

/*
This struct recorded the basic infomation of the CTP vendor and chip id
*/
CTP_custom_information_struct  ctp_custom_information_def = 
{
	"HYNITRON",
	"cst0",
	"UNKNOWN ",	
};

/*****************************************************************/

/*
  *
  */
 kal_bool ctp_hynitron_cst0_init(void)
 {
     kal_uint8 lvalue;
     kal_uint8 write_data[2];
     kal_bool temp_result = CTP_TRUE;
 
     hctp_i2c_power_on();
#if CTP_HYNITRON_EXT==1
    ctp_hynitron_update();
#endif
     hctp_i2c_init(CTP_SLAVER_ADDR,300);
     CTP_SET_RESET_PIN_LOW;
     hctp_delay_ms(10);
     CTP_SET_RESET_PIN_HIGH;
 
     kal_sleep_task(1);

     /*
     config EINT debounce time and sensitivity
     MUST set 0 to EINT debounce
     */
     EINT_Set_HW_Debounce(custom_eint_get_channel(touch_panel_eint_chann), 0);
     EINT_SW_Debounce_Modify(custom_eint_get_channel(touch_panel_eint_chann),0);
     EINT_Set_Sensitivity(custom_eint_get_channel(touch_panel_eint_chann), EDGE_SENSITIVE);
 
     ctp_dbg_print(MOD_TP_TASK, "ctp_cst0_init succeed! ");
     return CTP_TRUE;
 }

/*
 *
 */
kal_bool ctp_hynitron_cst0_power_on(kal_bool enable)
{
	//_TODO:  Implement this funciton by customer
	if (enable){
        CTP_SET_RESET_PIN_LOW;
        hctp_delay_ms(10);
        CTP_SET_RESET_PIN_HIGH;
    }else{
        kal_uint8 enterSleep[] = {0x03};
        hctp_write_bytes(0xA5,enterSleep,1,1);
    }
    return CTP_TRUE;
}
/*
 *
 */
kal_bool ctp_hynitron_cst0_device_mode(ctp_device_mode_enum mode)
{

	//_TODO:  if needed, add this function to switch decive work mode
	//_TODO: Implement this function by customer

	if(mode == CTP_ACTIVE_MODE)
	{
	}
	else if(mode == CTP_IDLE_MODE)
	{
	}
	else if(mode == CTP_SLEEP_MODE)
	{
	}
	
	return CTP_TRUE;
}


/*
The following function is JUST used in timer trigger mode. 
Now we use interrupt mode to get data. So this function is not used.

ATTENTION: DO NOT delete this function!!
This function is one member of the CTP function pointer struct.
*/
Touch_Panel_PenState_enum ctp_hynitron_cst0_hisr(void)
{
}
/*
This function is a INTERNAL FUNCTION in CTP driver.
By now, it has not been used~
If needed, implement it by yourselves.
But NO implement is OK~
*/
kal_bool ctp_hynitron_cst0_get_version(CTP_custom_information_struct  *version)
{
	return CTP_TRUE;
}

/*
This function is used to get parameter from CTP IC or set parameter to CTP IC.

By now, Maybe this function has not been used by upper layer.
ATTENTION: If need to implement it, DO NOT get and set the same parameter in the same time
*/
kal_bool ctp_hynitron_cst0_parameters(CTP_parameters_struct *para, kal_uint32 get_para, kal_uint32 set_para)
{
	//_TODO: not implement. if needed,  please add them.
	//_TODO: Implement this function by customer


	kal_bool result = CTP_TRUE;
	
	/*this function can NOT	 get and set the same parameter in the same time*/
	if (get_para & set_para)
	{
		ASSERT(0);
	}
	
	if (set_para & CTP_PARA_RESOLUTION)
	{
		result &= CTP_FALSE; 
	}
	else if (set_para & CTP_PARA_THRESHOLD)
	{
		result &= CTP_FALSE; 
	}
	else if (set_para & CTP_PARA_REPORT_INTVAL)
	{
		result &= CTP_TRUE;
	}
	else if (set_para & CTP_PARA_IDLE_INTVAL)
	{
		result &= CTP_FALSE;
	}
	else if (set_para & CTP_PARA_SLEEP_INTVAL)
	{
		result &= CTP_FALSE;
	}
	
	if (get_para & CTP_PARA_RESOLUTION)
	{
		result &= CTP_FALSE;
	}
	else if (get_para & CTP_PARA_THRESHOLD)
	{
		result &= CTP_FALSE;
	}
	else if (get_para & CTP_PARA_REPORT_INTVAL)
	{
		result = CTP_TRUE;
	}
	else if (get_para & CTP_PARA_IDLE_INTVAL)
	{
		result &= CTP_FALSE;
	}
	else if (get_para & CTP_PARA_SLEEP_INTVAL)
	{
		result &= CTP_FALSE;
	}
			
	return result;
}

/*
This function is used to get the raw data of the fingures that are pressed.
When CTP IC send intterupt signal to BB chip, this function will be called in the interrupt handler function.

ATTENTION: Becasue this function is called in the interrupt handler function, it MUST NOT run too long.
That will block the entire system.
If blocking too long, it generally will cause system crash *....*
*/
kal_bool ctp_hynitron_cst0_get_data(TouchPanelMultipleEventStruct *tpes)
{
	kal_bool temp_result;
	kal_uint8 lvalue[5];
	kal_uint32 counter = 0;
	kal_uint32 model = 0;
	
	ASSERT(tpes);

	tpes->time_stamp = (kal_uint16)L1I_GetTimeStamp();
	tpes->padding = CTP_PATTERN;

	hctp_read_bytes(0x02,lvalue,5,1);
	
	model = lvalue[0];
	ctp_dbg_print(MOD_TP_TASK, "ctp_get_data finger_num=%d", model);
		
	tpes->model = (kal_uint16)model;

	/*
	0 fingure meas UP EVENT, so return FALSE;
	And now we only support FIVE fingures at most, so if more than 5 fingures also return FALSE 
	*/	
	if ((model == 0)||(model > 2))
	{
		return CTP_FALSE;
	}
	
    tpes->points[0].x = (((kal_uint16)(lvalue[1]&0x0f))<<8) | lvalue[2];;
    tpes->points[0].y = (((kal_uint16)(lvalue[3]&0x0f))<<8) | lvalue[4];
    ctp_dbg_print(MOD_TP_TASK, "piont[%d], x:%d, y:%d", 0, tpes->points[0].x, tpes->points[0].y);
	return CTP_TRUE;    
}

/*
JUST return CTP information.
Not Need to modify it!
*/
CTP_custom_information_struct *ctp_Get_Data(void) 
{
	return (&ctp_custom_information_def);
}

/*
This structure is to initialize function pointer to CTP driver.
NOT all function MUST BE implemented in this struct, 
JUST doing function declaration is OK!!

But the following TWO functions MUST BE implemented:
ctp_hynitron_cst0_init
ctp_hynitron_cst0_get_data

Other functions should be implemented by customer for better performance.
*/
CTP_customize_function_struct ctp_custom_func=
{
	ctp_hynitron_cst0_init,
	ctp_hynitron_cst0_device_mode,
	ctp_hynitron_cst0_hisr,
	ctp_hynitron_cst0_get_data,
	ctp_hynitron_cst0_parameters,
	ctp_hynitron_cst0_power_on
};

/*
Upper layer use this hook to get CTP driver function
*/
CTP_customize_function_struct *ctp_GetFunc(void)
{
	return (&ctp_custom_func);  
}
#endif
#endif //#if defined(__CST816S_TOUCH__)


