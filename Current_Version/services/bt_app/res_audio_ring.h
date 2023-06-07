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

const int16_t RES_AUD_RING_SAMPRATE_8000 [] = {
#include "res/ring/SOUND_RING_8000.txt"
};

/** add by pang **/
const int16_t RES_AUD_RING_SAMPRATE_16000 [] = {
#include "res/ring/SOUND_RING_16000.txt"
};

const int16_t RES_AUD_RING_MIC_MUTE [] = {
#include "res/ring/Mute.txt"
};

const int16_t RES_AUD_RING_MIC_UNMUTE [] = {
#include "res/ring/Unmute.txt"
};
const int16_t RES_AUD_RING_BEEP_21 [] = {
#include "res/ring/beep_21_16Km.txt"
};

const int16_t RES_AUD_RING_BEEP_22 [] = {
#include "res/ring/beep_22_16Km.txt"
};

const int16_t RES_AUD_RING_BATTERY_LOW [] = {
#include "res/ring/battery_low.txt"
};

const int16_t RES_AUD_RING_ANC [] = {
//#include "res/ring/ANC.txt"
#include "res/ring/ANC_ON.txt"
};

const int16_t RES_AUD_RING_AWARENESS [] = {
//#include "res/ring/AWARENESS.txt"
#include "res/ring/AWARENESS_ON.txt"
};

const int16_t RES_AUD_RING_OFF [] = {
//#include "res/ring/OFF.txt"
#include "res/ring/ANC_OFF.txt"
};

const int16_t RES_AUD_RING_VOLT [] = {
//#include "res/ring/OFF.txt"
#include "res/ring/HP_volminmax.txt"
};


const int16_t CN_RES_AUD_RING_MIC_MUTE [] = {
//#include "res/ring/29_CN_mute.txt"
};
const int16_t CN_RES_AUD_RING_MIC_UNMUTE [] = {
//#include "res/ring/CN_unmute.txt"
};
const int16_t CN_RES_AUD_RING_ANC [] = {
//#include "res/ring/89_CN_anc_on.txt"
};
const int16_t CN_RES_AUD_RING_AWARENESS [] = {
//#include "res/ring/59_CN_monitor.txt"
};
const int16_t CN_RES_AUD_RING_OFF [] = {
//#include "res/ring/90_CN_anc_off.txt"
};


const int16_t RES_AUD_RING_MIC_MUTE_8K [] = {
//#include "res/ring/Mute_8k.txt"
};

const int16_t RES_AUD_RING_MIC_UNMUTE_8K [] = {
//#include "res/ring/Unmute_8k.txt"
};
const int16_t RES_AUD_RING_BEEP_21_8K [] = {
#include "res/ring/beep_21_8k.txt"
};

const int16_t RES_AUD_RING_BEEP_22_8K [] = {
//#include "res/ring/beep_22_8k.txt"
};

const int16_t RES_AUD_RING_BATTERY_LOW_8K [] = {
//#include "res/ring/battery_low_8k.txt"
};

const int16_t RES_AUD_RING_ANC_8K [] = {
//#include "res/ring/ANC.txt"
//#include "res/ring/ANC_ON_8k.txt"
};

const int16_t RES_AUD_RING_AWARENESS_8K [] = {
//#include "res/ring/AWARENESS.txt"
//#include "res/ring/AWARENESS_ON_8k.txt"
};

const int16_t RES_AUD_RING_OFF_8K [] = {
//#include "res/ring/OFF.txt"
//#include "res/ring/ANC_OFF_8k.txt"
};
/** end add **/
#ifdef __BT_WARNING_TONE_MERGE_INTO_STREAM_SBC__

//const int16_t RES_AUD_RING_SAMPRATE_16000 [] = {
//#include "res/ring/SOUND_RING_16000.txt"
//};
const int16_t RES_AUD_RING_SAMPRATE_44100[] = {
#include "res/ring/SOUND_RING_44100.txt"
};

const int16_t RES_AUD_RING_SAMPRATE_48000 [] = {
#include "res/ring/SOUND_RING_48000.txt"
};
#endif
