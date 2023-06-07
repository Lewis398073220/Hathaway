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
#ifndef __APP_ANC_H__
#define __APP_ANC_H__

#include "hal_aud.h"
#include "app_key.h"

#ifdef __cplusplus
extern "C" {
#endif

/** add by pang **/
enum
{
	NC_OFF = 0,
	ANC_HIGH,
	ANC_LOW,
	ANC_ADAPTIVE,
	ANC_WIND,
	MONITOR_ON,
	NC_INVALID
};

enum
{
	anc_off = 0,
	anc_on,
	monitor,
};



uint8_t app_get_anc_mode(void);
uint8_t app_get_anc_on_mode(void);
void app_set_anc_on_mode(uint8_t anc_on_new_mode);
uint8_t app_get_monitor_mode(void);
void app_set_monitor_mode(uint8_t monitor_new_level);
void app_set_clearvoice_mode(uint8_t clear_mode);
void set_anc_mode(uint8_t anc_new_mode);
void app_monitor_moment(bool on);
void app_anc_power_off(void);
void poweron_set_anc(void);
uint8_t api_get_anc_mode(void);
void app_anc_Key_Pro(void);
/** end add **/

void app_anc_set_coef(uint8_t index);
uint8_t app_anc_get_coef(void);
void app_anc_set_playback_samplerate(enum AUD_SAMPRATE_T sample_rate);
void app_anc_init(enum AUD_IO_PATH_T input_path, enum AUD_SAMPRATE_T playback_rate, enum AUD_SAMPRATE_T capture_rate);
void app_anc_close(void);
void app_anc_enable(void);
void app_anc_disable(void);
bool anc_enabled(void);
void test_anc(void);
void app_anc_resample(uint32_t res_ratio, uint32_t *in, uint32_t *out, uint32_t samples);
void app_anc_key(APP_KEY_STATUS *status, void *param);
int app_anc_open_module(void);
int app_anc_close_module(void);
enum AUD_SAMPRATE_T app_anc_get_play_rate(void);
bool app_anc_work_status(void);
void app_anc_send_howl_evt(uint32_t howl);
uint32_t app_anc_get_anc_status(void);
bool app_pwr_key_monitor_get_val(void);
bool app_anc_switch_get_val(void);
void app_anc_ios_init(void);
void app_anc_set_init_done(void);
bool app_anc_set_reboot(void);
void app_anc_status_post(uint8_t status);
bool app_anc_is_on(void);
uint32_t app_anc_get_sample_rate(void);

#ifdef __cplusplus
}
#endif

#endif
