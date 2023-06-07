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
#include "cmsis_os.h"
#include "stdbool.h"
#include "hal_trace.h"
#include "app_pwl.h"
#include "app_status_ind.h"
#include "string.h"

static APP_STATUS_INDICATION_T app_status = APP_STATUS_INDICATION_NUM;
static APP_STATUS_INDICATION_T app_status_ind_filter = APP_STATUS_INDICATION_NUM;

const char *app_status_indication_str[] =
{
    "[POWERON]",
    "[INITIAL]",
    "[PAGESCAN]",
    "[POWEROFF]",
    "[CHARGENEED]",
    "[CHARGING]",
    "[FULLCHARGE]",
    /* repeatable status: */
    "[BOTHSCAN]",
    "[CONNECTING]",
    "[CONNECTED]",
    "[DISCONNECTED]",
    "[CALLNUMBER]",
    "[INCOMINGCALL]",
    "[PAIRSUCCEED]",
    "[PAIRFAIL]",
    "[HANGUPCALL]",
    "[REFUSECALL]",
    "[ANSWERCALL]",
    "[CLEARSUCCEED]",
    "[CLEARFAIL]",
    "[WARNING]",
    "[ALEXA_START]",
    "[ALEXA_STOP]",
    "[GSOUND_MIC_OPEN]",
    "[GSOUND_MIC_CLOSE]",
    "[GSOUND_NC]",
    "[INVALID]",
    "[MUTE]",
    "[TESTMODE]",
    "[TESTMODE1]",
    "[RING_WARNING]",
#ifdef __INTERACTION__	
    "[FINDME]",
#endif	
    "[TILE_FIND]",
/** add by pang **/
    "[BTOFF]",
	"[FACTORYRESET]",
	"[BEEP_21]",
	"[BEEP_22]",
	"[BEEP_24]",
	"[BEEP_24S]",
	"[BUTTON_49]",         
	"[BUTTON_50]",
	"[ANC_OFF]",
	"[ANC_ON]",
	"[AWARENESS_ON]",
	"[POWEROFF_LOWBATTERY]",
	"[DEMO_MODE]",
	"[RING_DIGITAL]",
	"[TOUCHPAD_DISABLED]",
	"[BLUETOOTH_ONE_CONNECTED]",
	"[BLUETOOTH_ONE_DISCONNECTED]",
	"[BLUETOOTH_TWO_CONNECTED]",
	"[BLUETOOTH_TWO_DISCONNECTED]",
	"[BLUE_BLINK_1X]",
	"[VOL_MINMAX]",
/** end add **/

//add by cai
	"[A2DP]",
	"[CALLING_MUTE]",
	"[CALLING_UNMUTE]",
//end add
};

/** add by pang **/
static APP_STATUS_INDICATION_T app_status_ind_recover = APP_STATUS_INDICATION_NUM;
static bool app_status_recover_on=0;

void app_status_indication_recover(void)
{
	TRACE(1,"********%s",__func__);

	if(app_status_ind_recover!=APP_STATUS_INDICATION_NUM){
		app_status_recover_on=0;
		app_status_indication_set(app_status);
	    app_status_ind_recover = APP_STATUS_INDICATION_NUM;	    
	}
}

void app_status_indication_recover_set(APP_STATUS_INDICATION_T status)
{
	TRACE(2,"********%s %d",__func__, status);

    app_status_recover_on=0;
    app_status_ind_recover = status;
	app_status_indication_set(status);
	app_status_recover_on=1;
}

void app_status_indication_set_next(APP_STATUS_INDICATION_T curr_status,APP_STATUS_INDICATION_T next_status)
{
	TRACE(3,"********%s %d %d",__func__, curr_status, next_status);

    app_status_recover_on=0;
    app_status_ind_recover = curr_status;
	app_status_indication_set(curr_status);
	app_status_recover_on=1;
	app_status_indication_set(next_status);
}
/** end add **/

const char *status2str(uint16_t status)
{
    const char *str = NULL;

    if (status >= 0 && status < APP_STATUS_INDICATION_NUM)
    {
        str = app_status_indication_str[status];
    }
    else
    {
        str = "[UNKNOWN]";
    }

    return str;
}

int app_status_indication_filter_set(APP_STATUS_INDICATION_T status)
{
    app_status_ind_filter = status;
    return 0;
}

APP_STATUS_INDICATION_T app_status_indication_get(void)
{
    return app_status;
}

int app_status_indication_set(APP_STATUS_INDICATION_T status)
{
    struct APP_PWL_CFG_T cfg0;
    struct APP_PWL_CFG_T cfg1;

    TRACE(2,"%s %d",__func__, status);

#if 1 // m by pang
	if(app_status_recover_on){
    	app_status = status;
	    return 0;
	}
	
    if ((app_status == status)&&(app_status_ind_recover==APP_STATUS_INDICATION_NUM))
        return 0;

    if (app_status_ind_filter == status)
        return 0;

    if(app_status_ind_recover==APP_STATUS_INDICATION_NUM)
       app_status = status;
	
	TRACE(2,"********enter->%s %d",__func__, status);//add by cai for look at LED
#else
    if (app_status == status)
        return 0;

    if (app_status_ind_filter == status)
        return 0;

    app_status = status;
#endif
    memset(&cfg0, 0, sizeof(struct APP_PWL_CFG_T));//white led
    memset(&cfg1, 0, sizeof(struct APP_PWL_CFG_T));//blue led
    app_pwl_stop(APP_PWL_ID_0);
    app_pwl_stop(APP_PWL_ID_1);
    switch (status) {
        case APP_STATUS_INDICATION_POWERON:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (2000); 
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (500);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        case APP_STATUS_INDICATION_INITIAL:
            break;
        case APP_STATUS_INDICATION_PAGESCAN:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (8000);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;
			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        case APP_STATUS_INDICATION_BOTHSCAN:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;

			cfg1.part[0].level = 0;
            cfg1.part[0].time = (300);
			cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 0;
            cfg1.periodic = true;
			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
			app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_CONNECTING:
			cfg1.part[0].level = 1;
            cfg1.part[0].time = (2000);
			cfg1.part[1].level = 0;
            cfg1.part[1].time = (500);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = false;

			app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
			
        case APP_STATUS_INDICATION_CONNECTED:
			cfg1.part[0].level = 0;
            cfg1.part[0].time = (5000);
			cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 0;
            cfg1.periodic = true;

			app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
			
        case APP_STATUS_INDICATION_DISCONNECTED://add by pang
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (2000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;
			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
/*
		case APP_STATUS_INDICATION_BAT_VERYLOW:		
			cfg1.part[0].level = 1;
			cfg1.part[0].time = (300);
			cfg1.part[1].level = 0;
			cfg1.part[1].time = (300);
			cfg1.parttotal = 2;
			cfg1.startlevel = 1;
			cfg1.periodic = true;
			
			app_pwl_setup(APP_PWL_ID_1, &cfg1);
			app_pwl_start(APP_PWL_ID_1);
			break;
*/			
        case APP_STATUS_INDICATION_CHARGING:
			cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.parttotal = 1;
            cfg0.startlevel = 1;
            cfg0.periodic = false;

            cfg1.part[0].level = 0;
            cfg1.part[0].time = (300);
            cfg1.parttotal = 1;
            cfg1.startlevel = 0;
            cfg1.periodic = false;

			app_pwl_setup(APP_PWL_ID_0, &cfg0);
			app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
			
        case APP_STATUS_INDICATION_FULLCHARGE:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (300);
            cfg0.parttotal = 1;
            cfg0.startlevel = 0;
            cfg0.periodic = false;

            cfg1.part[0].level = 0;
            cfg1.part[0].time = (300);
            cfg1.parttotal = 1;
            cfg1.startlevel = 0;
            cfg1.periodic = false;

			app_pwl_setup(APP_PWL_ID_0, &cfg0);
			app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
			
        case APP_STATUS_INDICATION_POWEROFF:		
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (3000); 
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (500);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        case APP_STATUS_INDICATION_CHARGENEED:			
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (5000);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;
			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
			
        case APP_STATUS_INDICATION_TESTMODE:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;

            cfg1.part[0].level = 0;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_TESTMODE1:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (1000);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (1000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;

            cfg1.part[0].level = 0;
            cfg1.part[0].time = (1000);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (1000);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);

            break;
	//add by cai
		case APP_STATUS_INDICATION_DEMO_MODE:
			cfg1.part[0].level = 1;
            cfg1.part[0].time = (2000);
			cfg1.part[1].level = 0;
            cfg1.part[1].time = (500);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = false;

			app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
			break;	
	//end add
	/** add by pang **/
		 case APP_STATUS_INDICATION_INCOMINGCALL:
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (1000);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
		case APP_STATUS_INDICATION_BTOFF:
			cfg1.part[0].level = 0;
			cfg1.part[0].time = (300);
			cfg1.part[1].level = 1;
			cfg1.part[1].time = (8300);
			cfg1.parttotal = 2;
			cfg1.startlevel = 0;
			cfg1.periodic = true;

			app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
			break;

		case APP_STATUS_INDICATION_FACTORYRESET:			
			cfg1.part[0].level = 1;
            cfg1.part[0].time = (2000);
			cfg1.part[1].level = 0;
            cfg1.part[1].time = (500);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = false;

			app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
			break;

		case APP_STATUS_INDICATION_A2DP:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (2000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
	/** end add **/
        default:
            break;
    }
    return 0;
}
