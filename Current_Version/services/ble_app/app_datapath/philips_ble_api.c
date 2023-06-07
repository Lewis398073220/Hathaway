/***************************************************************************
 *
 * Copyright TPV.
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
#include "string.h"
#include "bluetooth.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "apps.h"
#include "stdbool.h"
//#include "app_ble_control_cmd_handler.h"
//#include "app_ble_control_custom_cmd.h"
#include "rwapp_config.h"
//#include "app_ble_control_server.h"

#include "philips_ble_api.h"
//#include "app_ble_control_server.h"

#include "app_datapath_server.h"
#include "app_ble_cmd_handler.h"
#include "app_ble_custom_cmd.h"

#include <stdio.h>
//Philips BLE
#include "../bt_if_enhanced/inc/avrcp_api.h"
#include "../bt_app/Btapp.h"
#include "app_spp_tota.h"

uint8_t title[150], title_len;
uint8_t artist[150], artist_len;
uint8_t album[150], album_len;
extern avrcp_media_status_t    media_status;

/** add by pang **/
#include "../../../apps/userapps/app_user.h"
#include "app_bt_stream.h"
#include "app.h"
#include "app_battery.h"
#include "../../../apps/anc/inc/app_anc.h"
#include "app_bt.h"

static uint8_t protocol_port=0;

void Philips_Api_protocol_port(uint8_t port)
{
	protocol_port=port;
}
/** end add **/


bool Philips_Headphone_Api_Entry(uint8_t *data, uint32_t size)
{	
   //Check Start head
   if (data[0] != (uint8_t) 0xff){
       TRACE(1,"Philips : fail! data[0]= %x\r\n ", data[0]);
	return false;
   }

   //Check API Version
    if (data[1] !=  (uint8_t)0x01){
        TRACE(1,"Philips : fail! data[1]= %x\r\n ", data[1]);
	return false;
   }

   //Check package Length
    if (data[2] != (uint8_t) size){
        TRACE(1,"Philips : fail! data[2]= %x\r\n ", data[2]);
	return false;
   }

   
   //Check Vendor ID
     if (data[3] !=  (uint8_t)0x04){
         TRACE(1,"Philips : fail! data[3]= %x\r\n ", data[3]);
	return false;
      }
     else{
	if (data[4] !=  (uint8_t)0x71){
          TRACE(1,"Philips : fail! data[4]= %x\r\n ", data[4]);
	   return false;
	 }
     }   


   //Check Command ID
    if (!CheckCommandID(data) ){
	TRACE(0,"Philips : Check Command ID Fail!\r\n");
	return false;
    }

   //Package Checksun
   if (!Command_CheckSum(data,(uint8_t) size) ){
	TRACE(0,"Philips : Checksun Fail!\r\n");
	return false;
   } 


   //Philips functions call
   if (!Philips_Functions_Call(data,(uint8_t) size) ){
	TRACE(0,"Philips : Philips_Functions_Call Fail!\r\n");
	return false;
   }


   TRACE(0,"Test_Philips_API OK");
   return true;
}

bool CheckCommandID(uint8_t *data)
{
	uint16_t command_id = ((uint16_t)data[5] << 8) | ((uint16_t)data[6]);
	   
	switch(command_id)
	{
		case GET_CONNECT_PHONE_MAC:
			//TRACE(0,"Philips : GET_CONNECT_PHONE_MAC!\r\n");
			return true;
		case GET_DEVICE_FEATURE_CONFIG:		
			//TRACE(0,"Philips : GET_DEVICE_FEATURE_CONFIG!\r\n");
			return true;
		case GET_API_VERSION:
			//TRACE(0,"Philips : GET_API_VERSION!\r\n");
			return true;	
		case GET_SOUND_QUALITY:
			//TRACE(0,"Philips : GET_SOUND_QUALITY!\r\n");
			return true;	
		case GET_RIGHT_EAR_MAC_ADDRESS:
			//TRACE(0,"Philips : GET_RIGHT_EAR_MAC_ADDRESS!\r\n");
			return true;
		case GET_LEFT_EAR_MAC_ADDRESS:
			//TRACE(0,"Philips : GET_LEFT_EAR_MAC_ADDRESS!\r\n");
			return true;		
		case NOTIFICATION_SOUND_QUALITY_CHANGE:
			//TRACE(0,"Philips : NOTIFICATION_SOUND_QUALITY_CHANGE!\r\n");
			return true;		
		case GET_BLE_MAC_ADDRESS:
			//TRACE(0,"Philips : GET_BLE_MAC_ADDRESS!\r\n");
			return true;			
		case GET_SALES_REGION:
			//TRACE(0,"Philips : GET_SALES_REGION!\r\n");
			return true;					
		case GET_CHIPSET_VENDOR:		
			//TRACE(0,"Philips : GET_CHIPSET_VENDOR!\r\n");
			return true;
		case GET_CHIPSET_SOLUTION:		
			//TRACE(0,"Philips : GET_CHIPSET_SOLUTION!\r\n");
			return true;
		case GET_FW_VERSION:		
			//TRACE(0,"Philips : GET_FW_VERSION!\r\n");
			return true;	
		case GET_DEVICE_TYPE:		
			//TRACE(0,"Philips : GET_DEVICE_TYPE!\r\n");
			return true;	
		case GET_FOTA_FINISH_FLAG:		
			//TRACE(0,"Philips : GET_FOTA_FINISH_FLAG!\r\n");
			return true;
		case SET_FOTA_FINISH_FLAG:		
			//TRACE(0,"Philips : SET_FOTA_FINISH_FLAG!\r\n");
			return true;		
		case GET_PCBA_VERSION:		
			//TRACE(0,"Philips : GET_PCBA_VERSION!\r\n");
			return true;		
			
		case GET_ANC_MODE_SUPPORT_LIST:		
			//TRACE(0,"Philips : GET_ANC_MODE_SUPPORT_LIST!\r\n");
			return true;
		case GET_ANC_MODE_STATUS:		
			//TRACE(0,"Philips : GET_ANC_MODE_STATUS!\r\n");
			return true;
		case SET_ANC_MODE_VALUE:		
			//TRACE(0,"Philips : SET_ANC_MODE_VALUE!\r\n");
			return true;	
		case GET_AWARENESS_VALUE:		
			//TRACE(0,"Philips : GET_AWARENESS_VALUE!\r\n");
			return true;
		case SET_AWARENESS_VALUE:		
			//TRACE(0,"Philips : SET_AWARENESS_VALUE!\r\n");
			return true;
		case GET_ENHANCE_VOICE_STATUS:		
			//TRACE(0,"Philips : GET_ENHANCE_VOICE_STATUS!\r\n");
			return true;
		case SET_ENHANCE_VOICE_STATUS:		
			//TRACE(0,"Philips : GET_ANC_TABLE_VALUE!\r\n");
			return true;
		case GET_ANC_TABLE_VALUE:		
			//TRACE(0,"Philips : SET_ENHANCE_VOICE_STATUS!\r\n");
			return true;
/*
		case GET_AMBIENT_SOUND_ITEM_STATUS:		
			//TRACE(0,"Philips : GET_AMBIENT_SOUND_ITEM_STATUS!\r\n");
			return true;	
		case SET_AMBIENT_SOUND_ITEM_VALUE:		
			//TRACE(0,"Philips : SET_AMBIENT_SOUND_ITEM_VALUE!\r\n");
			return true;	
		case GET_AMBIENT_RANGE_SUPPORT:		
			//TRACE(0,"Philips : GET_AMBIENT_RANGE_SUPPORT!\r\n");
			return true;	
		case GET_AMBIENT_RANGE_TOTAL:		
			//TRACE(0,"Philips : GET_AMBIENT_RANGE_TOTAL!\r\n");
			return true;	
		case GET_AMBIENT_FOCUS_ON_VOICE_SUPPORT:		
			//TRACE(0,"Philips : GET_AMBIENT_FOCUS_ON_VOICE_SUPPORT!\r\n");
			return true;	
		case GET_ADAPTIVE_STAND_D4_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_STAND_D4_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_WALKING_D4_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_WALKING_D4_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_RUNNING_D4_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_RUNNING_D4_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_TRAFFIC_D4_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_TRAFFIC_D4_VALUE!\r\n");
			return true;	
		case GET_AMBIENT_D4_VALUE:		
			//TRACE(0,"Philips : GET_AMBIENT_D4_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_D4_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_D4_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_D4_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_D4_VALUE!\r\n");
			return true;	
		case GET_AMBIENT_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : GET_AMBIENT_FOCUS_ON_VOICE_D4_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_STAND_CURRENT_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_STAND_CURRENT_VALUE!\r\n");
			return true;	
		case SET_ADAPTIVE_STAND_CURRENT_VALUE:		
			//TRACE(0,"Philips : SET_ADAPTIVE_STAND_CURRENT_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE!\r\n");
			return true;	
		case SET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : SET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_WALKING_CURRENT_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_WALKING_CURRENT_VALUE!\r\n");
			return true;	
		case SET_ADAPTIVE_WALKING_CURRENT_VALUE:		
			//TRACE(0,"Philips : SET_ADAPTIVE_WALKING_CURRENT_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE!\r\n");
			return true;	
		case SET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : SET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_RUNNING_CURRENT_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_RUNNING_CURRENT_VALUE!\r\n");
			return true;	
		case SET_ADAPTIVE_RUNNING_CURRENT_VALUE:		
			//TRACE(0,"Philips : SET_ADAPTIVE_RUNNING_CURRENT_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE!\r\n");
			return true;	
		case SET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : SET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE!\r\n");
			return true;	
		case GET_ADAPTIVE_TRAFFIC_CURRENT_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_TRAFFIC_CURRENT_VALUE!\r\n");
			return true;	
		case SET_ADAPTIVE_TRAFFIC_CURRENT_VALUE:		
			//TRACE(0,"Philips : SET_ADAPTIVE_TRAFFIC_CURRENT_VALUE!\r\n");
			return true;			
		case GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE!\r\n");
			return true;	
		case SET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : SET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE!\r\n");
			return true;	
		case GET_AMBIENT_CURRENT_VALUE:		
			//TRACE(0,"Philips : GET_AMBIENT_CURRENT_VALUE!\r\n");
			return true;	
		case SET_AMBIENT_CURRENT_VALUE:		
			//TRACE(0,"Philips : SET_AMBIENT_CURRENT_VALUE!\r\n");
			return true;	
		case GET_AMBIENT_FOCUS_ON_VOICE_STATUS:		
			//TRACE(0,"Philips : GET_AMBIENT_FOCUS_ON_VOICE_STATUS!\r\n");
			return true;	
		case SET_AMBIENT_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : GET_AMBIENT_FOCUS_ON_VOICE_STATUS!\r\n");
			return true;
*/
/*
		case GET_HW_NOSIE_CANCELLING_STATUS:		
			//TRACE(0,"Philips : GET_HW_NOSIE_CANCELLING_STATUS!\r\n");
			return true;
		case SET_HW_ANC_VALUE:		
			//TRACE(0,"Philips : SET_HW_ANC_VALUE!\r\n");
			return true;
		case GET_HW_AMBIENT_VALUE:		
			//TRACE(0,"Philips : GET_HW_AMBIENT_VALUE!\r\n");
			return true;			
		case SET_HW_AMBIENT_VALUE:		
			//TRACE(0,"Philips : SET_HW_AMBIENT_VALUE!\r\n");
			return true;	
		case GET_HW_FOCUS_ON_VOICE_STATUS:		
			//TRACE(0,"Philips : GET_HW_FOCUS_ON_VOICE_STATUS!\r\n");
			return true;	
		case SET_HW_FOCUS_ON_VOICE_STATUS:		
			//TRACE(0,"Philips : SET_HW_FOCUS_ON_VOICE_STATUS!\r\n");
			return true;
*/
		case NOTIFICATIION_NOSIE_CANCELLING_CHANGE:		
			//TRACE(0,"Philips : NOTIFICATIION_NOSIE_CANCELLING_CHANGE!\r\n");
			return true;					
		case GET_EQUALIZER_SUPPORT_LIST:		
			//TRACE(0,"Philips : GET_EQUALIZER_SUPPORT_LIST!\r\n");
			return true;	
		case GET_EQUALIZER_STATUS:		
			//TRACE(0,"Philips : GET_EQUALIZER_STATUS!\r\n");
			return true;	
		case SET_EQUALIZER_STATUS_VALUE:		
			//TRACE(0,"Philips : SET_EQUALIZER_STATUS_VALUE!\r\n");
			return true;	
		case GET_EQUALIZER_STATUS_D4_VALUE:
			//TRACE(0,"Philips : GET_EQUALIZER_STATUS_D4_VALUE!\r\n");
			return true;		
		case GET_LAST_EQ_STATUS:
			//TRACE(0,"Philips : GET_LAST_EQ_STATUS!\r\n");
			return true;
		case NOTIFICATIION_EQ_CHANGE:
			//TRACE(0,"Philips : NOTIFICATIION_EQ_CHANGE!\r\n");
			return true;			
		case GET_CUSTOMIZATION_EQ_SUPPORT_LIST:		
			//TRACE(0,"Philips : GET_CUSTOMIZATION_EQ_SUPPORT_LIST!\r\n");
			return true;	
		case GET_CUSTOMIZATION_EQ_BAND_VALUE:		
			//TRACE(0,"Philips : GET_CUSTOMIZATION_EQ_BAND_STATUS!\r\n");
			return true;	
		case SET_CUSTOMIZATION_EQ_BAND_VALUE:		
			//TRACE(0,"Philips : SET_CUSTOMIZATION_EQ_BAND_VALUE!\r\n");
			return true;	
		case GET_CUSTOMIZATION_EQ_BAND_RANGE_VALUE:		
			//TRACE(0,"Philips : GET_CUSTOMIZATION_EQ_BAND_RANGE_VALUE!\r\n");
			return true;	
		case GET_CUSTOMIZATION_EQ_WITH_LIB:		
			//TRACE(0,"Philips : GET_CUSTOMIZATION_EQ_WITH_LIB!\r\n");
			return true;	
		case GET_NOWPLAYING_PLAYBACK_STATUS:		
			//TRACE(0,"Philips : GET_NOWPLAYING_PLAYBACK_STATUS!\r\n");
			return true;		
		case SET_NOWPLAYING_PLAYBACK_STATUS_VALUE:		
			//TRACE(0,"Philips : SET_NOWPLAYING_PLAYBACK_STATUS_VALUE!\r\n");
			return true;	
		case SET_NOWPLAYING_PLAYBACK_NEXT:		
			//TRACE(0,"Philips : SET_NOWPLAYING_PLAYBACK_NEXT!\r\n");
			return true;	
		case SET_NOWPLAYING_PLAYBACK_PREVIOUS:		
			//TRACE(0,"Philips : SET_NOWPLAYING_PLAYBACK_PREVIOUS!\r\n");
			return true;	
		case NOTIFICATION_MEDIA_CHANGE:		
			//TRACE(0,"Philips : NOTIFICATION_MEDIA_CHANGE!\r\n");
			return true;				
		case GET_MEDIA_TITLE:		
			//TRACE(0,"Philips : GET_MEDIA_TITLE!\r\n");
			return true;	
		case GET_MEDIA_ARTIST:		
			//TRACE(0,"Philips : GET_MEDIA_ARTIST!\r\n");
			return true;	
		case GET_MEDIA_ALBUM:		
			//TRACE(0,"Philips : GET_MEDIA_ALBUM!\r\n");
			return true;			
		case GET_MEDIA_FUNTION_SUPPORT:		
			//TRACE(0,"Philips : GET_MEDIA_FUNTION_SUPPORT!\r\n");
			return true;		
/*						
		case GET_HEART_RATE_STATUS:		
			//TRACE(0,"Philips : GET_HEART_RATE_STATUS!\r\n");
			return true;	
		case SET_HEART_RATE_STATUS_VALUE:		
			//TRACE(0,"Philips : SET_HEART_RATE_STATUS_VALUE!\r\n");
			return true;	
		case GET_HEART_RATE_CURRENT_VALUE:		
			//TRACE(0,"Philips : GET_HEART_RATE_CURRENT_VALUE!\r\n");
			return true;		
		case GET_HEART_RATE_AUTO_NODIFY_STATUS:		
			//TRACE(0,"Philips : GET_HEART_RATE_AUTO_NODIFY_STATUS!\r\n");
			return true;	
		case SET_HEART_RATE_AUTO_NODIFY_VALUE:		
			//TRACE(0,"Philips : SET_HEART_RATE_AUTO_NODIFY_VALUE!\r\n");
			return true;		
*/	
/*
		case GET_EAR_DETECTION_STATUS:		
			//TRACE(0,"Philips : GET_EAR_DETECTION_STATUS!\r\n");
			return true;	
		case SET_EAR_DETECTION_STATUS_VALUE:		
			//TRACE(0,"Philips : SET_EAR_DETECTION_STATUS_VALUE!\r\n");
			return true;	
		case GET_EAR_DETECTION_CURRENT_VALUE:		
			//TRACE(0,"Philips : GET_EAR_DETECTION_CURRENT_VALUE!\r\n");
			return true;		
		case NOTIFICATION_EAR_DETECTION_CHANGE:		
			//TRACE(0,"Philips : NOTIFICATION_EAR_DETECTION_CHANGE!\r\n");
			return true;
*/
/*
		case P_SENSOR_CALIBRATION:		
			//TRACE(0,"Philips : P_SENSOR_CALIBRATION!\r\n");
			return true;	
		case P_SENSOR_RESET:		
			//TRACE(0,"Philips : P_SENSOR_RESET!\r\n");
			return true;	
		case SET_PAUSES_MUSIC_WHEN_TAKEOFF:		
			//TRACE(0,"Philips : SET_PAUSES_MUSIC_WHEN_TAKEOFF!\r\n");
			return true;		
		case GET_PAUSES_MUSIC_WHEN_TAKEOFF:		
			//TRACE(0,"Philips : GET_PAUSES_MUSIC_WHEN_TAKEOFF!\r\n");
			return true;
*/
		case GET_BATTERY_CHARGE_STATUS:		
			//TRACE(0,"Philips : GET_BATTERY_CHARGE_STATUS!\r\n");
			return true;	
		case GET_RIGHT_EAR_BATTERY_LEVEL_VALUE:		
			//TRACE(0,"Philips : GET_RIGHT_EAR_BATTERY_LEVEL_VALUE!\r\n");
			return true;	
		case GET_LEFT_EAR_BATTERY_LEVEL_VALUE:		
			//TRACE(0,"Philips : GET_LEFT_EAR_BATTERY_LEVEL_VALUE!\r\n");
			return true;	
		case GET_CHARGE_BOX_BATTERY_LEVEL_VALUE:		
			//TRACE(0,"Philips : GET_CHARGE_BOX_BATTERY_LEVEL_VALUE!\r\n");
			return true;	
		case NOTIFICATION_BATTERY_LEVEL_CHANGE:		
			//TRACE(0,"Philips : NOTIFICATION_BATTERY_LEVEL_CHANGE!\r\n");
			return true;	
		case GET_BATTERY_NOTIFY_STATUS:		
			//TRACE(0,"Philips : GET_BATTERY_NOTIFY_STATUS!\r\n");
			return true;				
		case SET_BATTERY_NOTIFY_STATUS:		
			//TRACE(0,"Philips : SET_BATTERY_NOTIFY_STATUS!\r\n");
			return true;	
		case NOTIFICATION_BATTERY_CHARGE_CHANGE:		
			//TRACE(0,"Philips : NOTIFICATION_BATTERY_CHARGE_CHANGE!\r\n");
			return true;				
		case SET_SLEEP_MODE_TIMER:		
			//TRACE(0,"Philips : SET_SLEEP_MODE_TIMER!\r\n");
			return true;				
		case GET_SLEEP_MODE_TIMER:		
			//TRACE(0,"Philips : GET_SLEEP_MODE_TIMER!\r\n");
			return true;		
		case GET_SMART_CHARGEBOX_SUPPORT:		
			//TRACE(0,"Philips : GET_SMART_CHARGEBOX_SUPPORT!\r\n");
			return true;	
		case GET_AUTO_POWER_OFF_SUPPORT:		
			//TRACE(0,"Philips : GET_AUTO_POWER_OFF_SUPPORT!\r\n");
			return true;	
		case GET_AUTO_POWER_OFF_TIMER:		
			//TRACE(0,"Philips : GET_AUTO_POWER_OFF_TIMER!\r\n");
			return true;
		case SET_AUTO_POWER_OFF_TIMER:		
			//TRACE(0,"Philips : SET_AUTO_POWER_OFF_TIMER!\r\n");
			return true;					
		/*		
		case GET_MULTI_LANGUAGE_VOICE_PROMPT_SUPPORT_LIST:		
			//TRACE(0,"Philips : GET_MULTI_LANGUAGE_VOICE_PROMPT_SUPPORT_LIST!\r\n");
			return true;		
		case GET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS:		
			//TRACE(0,"Philips : GET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS!\r\n");
			return true;	
		case SET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS_VALUE:		
			//TRACE(0,"Philips : SET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS_VALUE!\r\n");
			return true;
		case GET_MULTI_LANGUAGE_VOICE_PROMPT_D4_VALUE:		
			//TRACE(0,"Philips : GET_MULTI_LANGUAGE_VOICE_PROMPT_D4_VALUE!\r\n");
			return true;

		case GET_VOICE_ASSISTANT_SUPPORT_LIST:		
			//TRACE(0,"Philips : GET_VOICE_ASSISTANT_SUPPORT_LIST!\r\n");
			return true;
		case GET_VOICE_WAKEUP_SUPPORT_LIST:		
			//TRACE(0,"Philips : GET_VOICE_WAKEUP_SUPPORT_LIST!\r\n");
			return true;
		case GET_VOICE_ASSISTANT_STATUS:		
			//TRACE(0,"Philips : GET_VOICE_ASSISTANT_STATUS!\r\n");
			return true;
		case SET_VOICE_ASSISTANT_STATUS:		
			//TRACE(0,"Philips : SET_VOICE_ASSISTANT_STATUS!\r\n");
			return true;
		case GET_PUSH_AND_TALK_STATUS:		
			//TRACE(0,"Philips : GET_PUSH_AND_TALK_STATUS!\r\n");
			return true;
		case SET_PUSH_AND_TALK_STATUS:		
			//TRACE(0,"Philips : SET_PUSH_AND_TALK_STATUS!\r\n");
			return true;
		case GET_VOICE_WAKEUP_STATUS:		
			//TRACE(0,"Philips : GET_VOICE_WAKEUP_STATUS!\r\n");
			return true;
		case SET_VOICE_WAKEUP_STATUS:		
			//TRACE(0,"Philips : SET_VOICE_WAKEUP_STATUS!\r\n");
			return true;	
*/				
		case GET_SPECIAL_FUNCTION1_SUPPORT_LIST:
			//TRACE(0,"Philips : GET_SPECIAL_FUNCTION1_SUPPORT_LIST!\r\n");
		      return true;	
		case GET_TOUCH_STATUS:		
			//TRACE(0,"Philips : GET_TOUCH_STATUS!\r\n");
			return true;				
		case SET_TOUCH_STATUS:		
			//TRACE(0,"Philips : SET_TOUCH_STATUS!\r\n");
			return true;
/*
		case GET_RUNNING_LIGHT_STATUS:		
			//TRACE(0,"Philips : GET_RUNNING_LIGHT_STATUS!\r\n");
			return true;				
		case SET_RUNNING_LIGHT_STATUS:		
			//TRACE(0,"Philips : SET_RUNNING_LIGHT_STATUS!\r\n");
			return true;
*/
		case GET_SIDE_TONE_CONTROL_STATUS:		
			//TRACE(0,"Philips : GET_SIDE_TONE_CONTROL_STATUS!\r\n");
			return true;				
		case SET_SIDE_TONE_CONTROL_STATUS:		
			//TRACE(0,"Philips : SET_SIDE_TONE_CONTROL_STATUS!\r\n");
			return true;
/*
		case GET_LOW_LATENCY_STATUS:		
			//TRACE(0,"Philips : GET_LOW_LATENCY_STATUS!\r\n");
			return true;				
		case SET_LOW_LATENCY_STATUS:		
			//TRACE(0,"Philips : SET_LOW_LATENCY_STATUS!\r\n");
			return true;				
		case GET_VIBRATION_STATUS:		
			//TRACE(0,"Philips : GET_VIBRATION_STATUS!\r\n");
			return true;				
		case SET_VIBRATION_STATUS:		
			//TRACE(0,"Philips : SET_VIBRATION_STATUS!\r\n");
			return true;
*/		
		case GET_MULTIPOINT_STATUS:		
			//TRACE(0,"Philips : GET_MULTIPOINT_STATUS!\r\n");
			return true;				
		case SET_MULTIPOINT_STATUS:		
			//TRACE(0,"Philips : SET_MULTIPOINT_STATUS!\r\n");
			return true;

		case GET_DEVICE_COLOUR_STATUS:		
			//TRACE(0,"Philips : SET_MULTIPOINT_STATUS!\r\n");
			return true;

		case GET_SPECIAL_FUNCTION2_SUPPORT_LIST:
		case GET_TALK_MIC_LED_STATUS:					
		case SET_TALK_MIC_LED_STATUS:					
			return true;

		default:
			TRACE(0,"Philips : Command error!\r\n");
      break;
	}
	
	return false;
}

bool Command_CheckSum(uint8_t *data, uint8_t size)
{
	uint8_t i = 0;
	uint8_t checksum = data[0];
  
	for ( i=1; i < (size - 1); i++){
		checksum ^= data[i];
	}

	if (data[size -1 ] == checksum){
		return true;
	}
	
	return false;
}

static uint8_t g_valueLen = 0;
static uint8_t g_valuePtr[18] = {0};

uint8_t Do_CheckSum(uint8_t *data, uint8_t size)
{
	uint8_t i = 0;
	uint8_t checksum = data[0];
	  
	for ( i=1; i < (size - 1); i++){
		checksum ^= data[i];
	}
	
	return checksum;
}

void Philips_Send_Notify(uint8_t *data, uint32_t size)
{
    //app_control_app_server_send_data_via_notification(data,size);
    if(protocol_port==1)
		app_datapath_server_send_data_via_notification(data,size);
	else if(protocol_port==2)
		app_tota_send_data_via_spp(data, size);
}

void Get_Connect_Phone_Mac(void)
{
    uint8_t phoneAddr[6] ={0};
    app_get_curr_remDev(phoneAddr);
    g_valueLen = 14;
    uint8_t i =0;
    uint8_t data[14] = {0xff,0x01,0x00,0x04,0x71,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
     //Data length
     data[2] = 0x0e;
     //Phone Mac address 6 byte
     for (i = 0 ; i < 6 ; i++){
	      data[7 + i ] = phoneAddr[i];
     } 	
	 
     //Do checksum
     data[g_valueLen - 1]=Do_CheckSum(data,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	      g_valuePtr[i] = data[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
    
}


void Get_Device_Feature_Config(void)
{
	uint8_t device_feature[3] = {0};
	device_feature[0] = FOTA_SUPPORT | ANC_MODE_SUPPORT | EQUALIZER_SUPPORT | CUSTOMIZATION_EQ_SUPPORT | NOWPLAYING_SUPPORT; //| EAR_DETECTION_SUPPORT;//  | HEART_RATE_SUPPORT;
    device_feature[1] = BATTERY_STATUS_SUPPORT  | SPECIAL_FUNCTION1_SUPPORT |SPECIAL_FUNCTION2_SUPPORT;//| MULTI_LANGAUAGE_VOICE_PROMPT_SUPPORT
    device_feature[2] = NOT_SUPPORT ;
    
    g_valueLen = 11;
    uint8_t i =0;
    uint8_t data[11] = {0xff,0x01,0x00,0x04,0x71,0x80,0x01,0x00,0x00,0x00,0x00};
     //Data length
     data[2] = 0x0b;
     //Device features list 2 byte
     
     data[7] = device_feature[0];
     data[8] = device_feature[1];
     data[9] = device_feature[2] & 0x01;	
	 
     //Do checksum
     data[g_valueLen - 1]=Do_CheckSum(data,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = data[i];
     }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

unsigned char Version[] = "API V2.0";
void Get_Api_Version()
{
	  uint8_t Api_Version[10]= {0};
    uint8_t i =0;
    uint8_t head[7] = {0xff,0x01,0x00,0x04,0x71,0x80,0x02};
     
     uint8_t charsize = sizeof(Version);
     
     //TRACE(1,"charsize %d : ", charsize);
     
     memcpy(Api_Version, Version, charsize);
	 
     g_valueLen = 8 + charsize;

     uint8_t version[g_valueLen];	 
	 
     for (i = 0 ; i < 7 ; i++){
	 	version[i] = head[i];
     }

     //Data length
     version[2] = g_valueLen;

	 
     for (i = 7 ; i < (7 + charsize) ; i++){
	 	   version[i] = (uint8_t) Api_Version[i - 7];
     }    
	
	 
     //Do checksum
     version[g_valueLen - 1]=Do_CheckSum(version,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = version[i];
    }	 

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Sound_Quality(void)
{
	uint8_t sound_quality[]= {0x00}; //0 is SBC ,1 is AAC

	if(0x02==bt_sbc_player_get_codec_type())
		sound_quality[0]=0x01;
	else
		sound_quality[0]=0x00;
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x03,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //sound quality Item status 1 byte  
     head[7] = sound_quality[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
     }	   
	
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Right_Ear_MAC_Address(void)
{
	uint8_t MacAddr[6] ={1};
	memcpy(MacAddr, bt_addr, 6);
    g_valueLen = 14;
    uint8_t i =0;
    uint8_t data[14] = {0xff,0x01,0x00,0x04,0x71,0x80,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
     //Data length
     data[2] = 0x0e;
     //Mac address 6 byte
     for (i = 0 ; i < 6 ; i++){
	 		data[7 + i ] = MacAddr[i];  //Need fill it.
     } 	
	 
     //Do checksum
     data[g_valueLen - 1]=Do_CheckSum(data,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = data[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Left_Ear_MAC_Address(void)
{
	uint8_t MacAddr[6] ={2};
	memcpy(MacAddr, bt_addr, 6);
    g_valueLen = 14;
    uint8_t i =0;
    uint8_t data[14] = {0xff,0x01,0x00,0x04,0x71,0x80,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
     //Data length
     data[2] = 0x0e;
     //Mac address 6 byte
     for (i = 0 ; i < 6 ; i++){
	 		data[7 + i ] = MacAddr[i];  //Need fill it.
     } 	
	 
     //Do checksum
     data[g_valueLen - 1]=Do_CheckSum(data,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = data[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Notification_Sound_Quality_Change(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x06,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Notification_EQ_Change 1 byte  
     head[7] = 0x01; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
   
}

void Get_BLE_MAC_Address(void)
{
	uint8_t MacAddr[6] ={3};
	memcpy(MacAddr, ble_addr, 6);
    g_valueLen = 14;
    uint8_t i =0;
    uint8_t data[14] = {0xff,0x01,0x00,0x04,0x71,0x80,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
     //Data length
     data[2] = 0x0e;
     //Mac address 6 byte
     for (i = 0 ; i < 6 ; i++){
	 		data[7 + i ] = MacAddr[i];  //Need fill it.
     } 	
	 
     //Do checksum
     data[g_valueLen - 1]=Do_CheckSum(data,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = data[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Sales_Region(void)
{
	  uint8_t sound_quality[]= {0x00}; //00 is Global region , /93 is China
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x08,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //sound quality Item status 1 byte  
     head[7] = sound_quality[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
     }	   
	
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Chipset_Vendor(void)
{
	  unsigned char Vendor[] = "BES";
	  uint8_t Chipset_Vendor[10]= {0};
    uint8_t i =0;
    uint8_t head[7] = {0xff,0x01,0x00,0x04,0x71,0x80,0x10};
     
     uint8_t charsize = sizeof(Vendor);
     
     //TRACE(1,"charsize %d : ", charsize);
     
     memcpy(Chipset_Vendor, Vendor, charsize);
	 
     g_valueLen = 8 + charsize;

     uint8_t vendor[g_valueLen];	 
	 
     for (i = 0 ; i < 7 ; i++){
	 			vendor[i] = head[i];
     }

     //Data length
     vendor[2] = g_valueLen;

	 
     for (i = 7 ; i < (7 + charsize) ; i++){
	 	   vendor[i] = (uint8_t) Chipset_Vendor[i - 7];
     }    
	
	 
     //Do checksum
     vendor[g_valueLen - 1]=Do_CheckSum(vendor,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = vendor[i];
    }	 

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Chipset_Solution(void)
{
	  unsigned char Solution[] = "BES2300YP";
	  uint8_t Chipset_Solution[10]= {0};
    uint8_t i =0;
    uint8_t head[7] = {0xff,0x01,0x00,0x04,0x71,0x80,0x11};
     
     uint8_t charsize = sizeof(Solution);
     
     //TRACE(1,"charsize %d : ", charsize);
     
     memcpy(Chipset_Solution, Solution, charsize);
	 
     g_valueLen = 8 + charsize;

     uint8_t solution[g_valueLen];	 
	 
     for (i = 0 ; i < 7 ; i++){
	 			solution[i] = head[i];
     }

     //Data length
     solution[2] = g_valueLen;

	 
     for (i = 7 ; i < (7 + charsize) ; i++){
	 	   solution[i] = (uint8_t) Chipset_Solution[i - 7];
     }    
	
	 
     //Do checksum
     solution[g_valueLen - 1]=Do_CheckSum(solution,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = solution[i];
    }	 

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Chipset_Version(void)
{
	unsigned char Version[] = "V2.4.9.0";
	uint8_t FW_Version[10]= {0};
    uint8_t i =0;
    uint8_t head[7] = {0xff,0x01,0x00,0x04,0x71,0x80,0x12};
     
     uint8_t charsize = sizeof(Version);
     
     //TRACE(1,"charsize %d : ", charsize);
     
     memcpy(FW_Version, Version, charsize);
	 
     g_valueLen = 8 + charsize;

     uint8_t version[g_valueLen];	 
	 
     for (i = 0 ; i < 7 ; i++){
	 			version[i] = head[i];
     }

     //Data length
     version[2] = g_valueLen;

	 
     for (i = 7 ; i < (7 + charsize) ; i++){
	 	   version[i] = (uint8_t) FW_Version[i - 7];
     }    
	
	 
     //Do checksum
     version[g_valueLen - 1]=Do_CheckSum(version,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = version[i];
    }	 

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Device_Type(void)
{
	  uint8_t device_type[]= {0x00}; //0 is Headset 1 is TWS
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x13,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //device type Item status 1 byte  
     head[7] = device_type[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

static uint8_t g_FOTA_Flage[]= {0x00};
void Get_FOTA_Finish_Flag(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x14,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //device type Item status 1 byte 
     g_FOTA_Flage[0]=app_get_fota_flag();
     head[7] = g_FOTA_Flage[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	g_valuePtr[i] = head[i];
    }	   
   
     Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

void  Set_FOTA_Finish_Flag(uint8_t *valuePtr)
{
   g_FOTA_Flage[0] =  valuePtr[0] & 0x01;
   app_nvrecord_fotaflag_set(g_FOTA_Flage[0]); 
}

void Get_PCBA_Version(void)
{
	  unsigned char Version[] = "V1.0";
	  uint8_t PCBA_Version[10]= {0};
    uint8_t i =0;
    uint8_t head[7] = {0xff,0x01,0x00,0x04,0x71,0x80,0x16};
     
     uint8_t charsize = sizeof(Version);
     
     //TRACE(1,"charsize %d : ", charsize);
     
     memcpy(PCBA_Version, Version, charsize);
	 
     g_valueLen = 8 + charsize;

     uint8_t version[g_valueLen];	 
	 
     for (i = 0 ; i < 7 ; i++){
	 			version[i] = head[i];
     }

     //Data length
     version[2] = g_valueLen;

	 
     for (i = 7 ; i < (7 + charsize) ; i++){
	 	   version[i] = (uint8_t) PCBA_Version[i - 7];
     }    
	
	 
     //Do checksum
     version[g_valueLen - 1]=Do_CheckSum(version,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = version[i];
    }	 

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

/*
static uint8_t set_ambisound_item_value[]= {0x00};
void  Get_Ambient_Sound_Item_Status(void)
{

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x30,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Ambient Sound Item status 1 byte  
     head[7] =  set_ambisound_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void  Set_Ambient_Sound_Item_Value(uint8_t *valuePtr)
{
   set_ambisound_item_value[0] =  valuePtr[0] & 0x03;
   
}

void Get_Ambient_Range_Support(void)
{
	  uint8_t AmbientRange_Support[]= {0x01}; //0 is not support, 1 is support
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x32,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Ambient Sound Item status 1 byte  
     head[7] = AmbientRange_Support[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Ambient_Range_Total(void)
{
    uint8_t AmbientRange_Total[]= {0x14};  // is support 20 level
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x33,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Ambient Sound Item status 1 byte  
     head[7] = AmbientRange_Total[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

void Get_Ambient_Focus_On_Voice_Support(void)
{
	  uint8_t AmbientFocusOn_Support[]= {0x01}; //0 is not support, 1 is support
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x34,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Ambient focus on voice support 1 byte  
     head[7] = AmbientFocusOn_Support[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
	
}

void Get_Adaptive_Stand_D4_Value(void)
{
	  uint8_t AdaptiveStandD4Value[]= {0x14};  //level 20
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x35,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive stand D4 value 1 byte  
     head[7] = AdaptiveStandD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Adaptive_Walking_D4_Value(void)
{
	  uint8_t AdaptiveWalkingD4Value[]= {0x0c};  //level 12
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x36,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive walking D4 value 1 byte  
     head[7] = AdaptiveWalkingD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
	
}

void Get_Adaptive_Running_D4_Value(void)
{
	  uint8_t AdaptiveRunningD4Value[]= {0x14}; //level 20
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x37,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive walking D4 value 1 byte  
     head[7] = AdaptiveRunningD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
	
}

void Get_Adaptive_Traffic_D4_Value(void)
{
	  uint8_t AdaptiveTrafficD4Value[]= {0x00}; //level 0
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x38,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive walking D4 value 1 byte  
     head[7] = AdaptiveTrafficD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);     
	
}

void Get_Ambient_D4_Value(void)
{
	  uint8_t AmbientD4Value[]= {0x00}; //level 0
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x39,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive walking D4 value 1 byte  
     head[7] = AmbientD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Adaptive_Stand_Focus_On_Voice_D4_Value(void)
{
	  uint8_t StandFocusOnD4Value[]= {0x01}; //on
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x3a,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive walking D4 value 1 byte  
     head[7] = StandFocusOnD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	 
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

void Get_Adaptive_Walking_Focus_On_Voice_D4_Value(void)
{
	  uint8_t WalkingFocusOnD4Value[]= {0x00};  //off
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x3b,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive walking D4 value 1 byte  
     head[7] = WalkingFocusOnD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Adaptive_Running_Focus_On_Voice_D4_Value(void)
{
	  uint8_t RunningFocusOnD4Value[]= {0x00}; //off
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x3c,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive Running D4 value 1 byte  
     head[7] = RunningFocusOnD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Get_Adaptive_Traffic_Focus_On_Voice_D4_Value(void)
{
	  uint8_t TrafficFocusOnD4Value[]= {0x00}; //off
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x3d,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive Traffic D4 value 1 byte  
     head[7] = TrafficFocusOnD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

void Get_Adaptive_Ambient_Focus_On_Voice_D4_Value(void)
{
	  uint8_t AmbientFocusOnD4Value[]= {0x00}; //off
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x3e,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive Ambient D4 value 1 byte  
     head[7] = AmbientFocusOnD4Value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

static uint8_t g_set_adapstand_current_item_value[]= {0x00};
void Get_Adaptive_Stand_Current_Value(void)
{

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x3f,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive Stand Current Value value 1 byte  
     head[7] =  g_set_adapstand_current_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
	
}

void Set_Adaptive_Stand_Current_Value(uint8_t set_stand_current_value[1])
{
    g_set_adapstand_current_item_value[0] =  set_stand_current_value[0];
}

static uint8_t g_set_adapstand_focuson_item_value[]= {0x00};
void Get_Adaptive_Stand_Focus_On_Voice_Value(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x41,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive Stand Focus On Voice Value 1 byte  
     head[7] =  g_set_adapstand_focuson_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
	
}

void Set_Adaptive_Stand_Focus_On_Voice_Value(uint8_t set_adaptive_standfocuson_value[1])
{
    g_set_adapstand_focuson_item_value[0] =  set_adaptive_standfocuson_value[0]  & 0x01;
}

static uint8_t g_set_adapwalk_current_item_value[]= {0x00};
void Get_Adaptive_Walking_Current_Value(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x43,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //NV byte 4 is save Adaptive Walking Current Value 1 byte  
     head[7] =  g_set_adapwalk_current_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
	
}

void Set_Adaptive_Walking_Current_Value(uint8_t set_walking_current_value[1])
{
    g_set_adapwalk_current_item_value[0] =  set_walking_current_value[0];
    
}

static uint8_t g_set_adapwalk_focus_item_value[]= {0x00};
void Get_Adaptive_Walking_Focus_On_Voice_Value(void)
{
    printf("Philips: Get_Adaptive_Stand_Focus_On_Voice_Value:");
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x45,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive Walking Focus On Voice Value 1 byte  
     head[7] =  g_set_adapwalk_focus_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);     
	
}

void Set_Adaptive_Walking_Focus_On_Voice_Value(uint8_t set_adaptive_walkingfocuson_value[1])
{
    g_set_adapwalk_focus_item_value[0] =  set_adaptive_walkingfocuson_value[0]  & 0x01;;
	
}

static uint8_t g_set_adaprun_current_item_value[]= {0x00};
void Get_Adaptive_Running_Current_Value(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x47,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive Running Current Value 1 byte  
     head[7] =  g_set_adaprun_current_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Set_Adaptive_Running_Current_Value(uint8_t set_running_current_value[1])
{
    g_set_adaprun_current_item_value[0] =  set_running_current_value[0];
}

static uint8_t g_set_adaprun_focus_item_value[]= {0x00};
void Get_Adaptive_Running_Focus_On_Voice_Value(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x49,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive Running Focus On Voice Value 1 byte  
     head[7] =  g_set_adaprun_focus_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
	
}

void Set_Adaptive_Running_Focus_On_Voice_Value(uint8_t set_adaptive_runningfocuson_value[1])
{
    g_set_adaprun_focus_item_value[0] =  set_adaptive_runningfocuson_value[0]  & 0x01;;   
}

static uint8_t g_set_adaptra_current_item_value[]= {0x00};
void Get_Adaptive_Traffic_Current_Value(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x4b,0x00,0x00};
     //Data length
     head[2] = 0x09;
     ////NV byte 8 is save Adaptive Traffic Current Value 1 byte  
     head[7] =  g_set_adaptra_current_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

void Set_Adaptive_Traffic_Current_Value(uint8_t set_traffic_current_value[1])
{
    g_set_adaptra_current_item_value[0] =  set_traffic_current_value[0];
 	
}

static uint8_t g_set_adaptra_focus_item_value[]= {0x00};
void Get_Adaptive_Traffic_Focus_On_Voice_Value(void)
{

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x4d,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Adaptive Traffic Focus On Voice Value 1 byte  
     head[7] =  g_set_adaptra_focus_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
}

void Set_Adaptive_Traffic_Focus_On_Voice_Value(uint8_t set_adaptive_trafficfocuson_value[1])
{
    g_set_adaptra_focus_item_value[0] =  set_adaptive_trafficfocuson_value[0]  & 0x01;;
	
}

static uint8_t g_set_ambient_current_item_value[]= {0x00};
void Get_Ambient_Current_Value(void)
{

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x4f,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Ambient Current Value 1 byte  
     head[7] =  g_set_ambient_current_item_value[0];
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
}

void Set_Ambient_Current_Value(uint8_t set_ambient_current_value[1])
{
    g_set_ambient_current_item_value[0] =  set_ambient_current_value[0];
  		
}

static uint8_t g_set_ambient_focus_item_value[]= {0x00};
void Get_Ambient_Focus_On_Voice_Value(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x51,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Ambient Focus On Voice Value 1 byte  
     head[7] =  g_set_ambient_focus_item_value[0];     
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
}

void Set_Ambient_Focus_On_Voice_Value(uint8_t set_ambient_focuson_value[1])
{
    g_set_ambient_focus_item_value[0] =  set_ambient_focuson_value[0]  & 0x01;     
    
}
*/
/*
static uint8_t g_hw_nosie_cancelling_status[]= {0x00};
static uint8_t g_hw_ambient_value[]= {0x00};
static uint8_t g_hw_focus_on_voice_status[]= {0x00};

void  Get_HW_Nosie_Cancelling_Status(void )
{
    g_hw_nosie_cancelling_status[0]=app_get_anc_mode();
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x53,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Nosie Cancelling status 1 byte  
     head[7] =  g_hw_nosie_cancelling_status[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}


void Set_HW_ANC_Value(uint8_t set_hw_anc_value[1])
{
    g_hw_nosie_cancelling_status[0] = set_hw_anc_value[0] & 0x01;
    //Go to Set HW ANC functiion.

	if(g_hw_nosie_cancelling_status[0]>0x01)
		return;

    if(g_hw_nosie_cancelling_status[0]==0x00){
	  	app_nc_mode_change(NC_OFF,0);
		app_nvrecord_anc_set(NC_OFF);
    }
	else{
		app_nc_mode_change(ANC_ON,0);
		app_nvrecord_anc_set(ANC_ON);
	}
}

void  Get_HW_Ambient_value(void)
{
    g_hw_ambient_value[0]=app_get_monitor_level();
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x55,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //HW Ambient Value 1 byte  
     head[7] =  g_hw_ambient_value[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
		g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}


void  Set_HW_Ambient_value(uint8_t set_hw_ambient_value[2] )
{
		 //TRACE(1,"set_hw_ambient_value[0] %2x : ", set_hw_ambient_value[0]);
	   //TRACE(1,"set_hw_ambient_value[1] %2x : ", set_hw_ambient_value[1]);

	 if((set_hw_ambient_value[0]>20)||(set_hw_ambient_value[1] > 0x01))
		return;

	  g_hw_ambient_value[0] =  set_hw_ambient_value[0];
	  if (set_hw_ambient_value[1] == 0x00){  //save	  	
      	g_hw_nosie_cancelling_status[0] = 0x02;
	  	app_nvrecord_monitor_level_set(g_hw_ambient_value[0]);
	  	app_nvrecord_anc_set(MONITOR_ON);
	  }else{   //no save
	  	
	  }

	  //Go to Set HW Ambient functiion.
	  if(g_hw_ambient_value[0]==0)
	  	app_nc_mode_change(ANC_ON,0); 
	  else{
	  	if(g_hw_focus_on_voice_status[0])
	  		app_nc_mode_change(MONITOR_ON,21+g_hw_ambient_value[0]);
		else
			app_nc_mode_change(MONITOR_ON,g_hw_ambient_value[0]);
	  }
}

void  Get_HW_Focus_On_Voice_Status(void )
{
    g_hw_focus_on_voice_status[0]=app_get_focus();
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x57,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //HW Focus On Voice Status 1 byte  
     head[7] =  g_hw_focus_on_voice_status[0];
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
				g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}


void  Set_HW_Focus_On_Voice_Status(uint8_t set_hw_focus_on_voice_status[1] )
{
	   //TRACE(1,"set_hw_focus_on_voice_status[0] %2x : ", set_hw_focus_on_voice_status[0]);
	   //TRACE(1,"set_hw_focus_on_voice_status[1] %2x : ", set_hw_focus_on_voice_status[1]);

	   if ((set_hw_focus_on_voice_status[0] > 0x01)||(set_hw_focus_on_voice_status[1]>0x01))
	   	return;

	    g_hw_focus_on_voice_status[0] =  set_hw_focus_on_voice_status[0];
		if (set_hw_focus_on_voice_status[1] == 0x00){  //save    	
    	g_hw_nosie_cancelling_status[0] = 0x02;
		app_nvrecord_focus_set(g_hw_focus_on_voice_status[0]);
		app_nvrecord_anc_set(MONITOR_ON);
	  }else{   //no save
	  	  	
	  }

      //Go to Set Focus On Voice functiion.
      if(g_hw_focus_on_voice_status[0]==0x00)
      	app_nc_mode_change(MONITOR_ON,g_hw_ambient_value[0]); 
	  else
	  	app_nc_mode_change(MONITOR_ON,(21+g_hw_ambient_value[0])); 
}
*/
void Notification_Nosie_Cancelling_Change(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x59,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Notification_Nosie_Cancelling_Change 1 byte  
     head[7] = 0x01; 
     //LOG_MSGID_I(AIR, "Philips : anc_change_status = %x\r\n", 1, (uint8_t) anc_change_status);
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
}


void Get_ANC_Mode_Support_List(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x20,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //ANC_Mode Support List 1 byte  
     head[7] = ANC_OFF_SUPPORT | ANC_MODE1_SUPPORT | ANC_MODE2_SUPPORT | ANC_MODE4_SUPPORT | AWARENESS_SUPPORT | ENHANCE_VOICE_SUPPORT |AWARENESS_SLIDE_SUPPORT;
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
}

static uint8_t g_set_anc_mode_value[]= {0x00};
void Get_ANC_Mode_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x21,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //ANC mode Status 1 byte  
     g_set_anc_mode_value[0]=api_get_anc_mode();
     head[7] =  g_set_anc_mode_value[0];    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}

void Set_ANC_Mode_Status(uint8_t set_anc_mode_status_value[1])
{
   uint8_t anc_mode_status_value;
   
   g_set_anc_mode_value[0] =  set_anc_mode_status_value[0];

   if(g_set_anc_mode_value[0]==0x00){
     anc_mode_status_value=0x00;
   }
   else if(g_set_anc_mode_value[0]==0x01){
     anc_mode_status_value=0x01;
   }
   else if(g_set_anc_mode_value[0]==0x02){
     anc_mode_status_value=0x02;
   }
   else if(g_set_anc_mode_value[0]==0x04){
     anc_mode_status_value=0x03;
   }
   else 
   	 return;
   
   app_nvrecord_anc_set(anc_mode_status_value);
   if(anc_mode_status_value==0){
		set_anc_mode(anc_off);
   }
   else{
	 	app_set_anc_on_mode(anc_mode_status_value);
	 	set_anc_mode(anc_on);
   }
}

static uint8_t g_set_awareness_value[2]= {0x00};
static uint8_t g_set_enhance_voice_value[2]= {0x00};
void Get_Awareness_Value(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x23,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Awareness value 1 byte  
     g_set_awareness_value[0]=app_get_monitor_level();
     head[7] =  g_set_awareness_value[0];  
     
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	 g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}

void Set_Awareness_Value(uint8_t set_awareness_value[2])
{
      TRACE(1,"set_awareness_value [0]=%x",set_awareness_value[0]);
	  TRACE(1,"set_awareness_value [1]=%x",set_awareness_value[1]);	

	  g_set_awareness_value[0] =  set_awareness_value[0];
   	  if (set_awareness_value[1] == 0x00){  //save
        g_set_anc_mode_value[0] = 0x05;  //ANC mode status is Awareness on
        app_nvrecord_monitor_level_set(set_awareness_value[0]);
	  	app_nvrecord_anc_set(MONITOR_ON);
	  }else{   //no save
		g_set_anc_mode_value[0] = 0x05;  //ANC mode status is Awareness on
	  }

      TRACE(1,"g_set_awareness_value [0]=%x",g_set_awareness_value[0]);
	  TRACE(1,"g_set_anc_mode_value [0]=%x",g_set_anc_mode_value[0]);

	  //Go to Set awareness function.
	  if(g_set_awareness_value[0]==0){
	    set_anc_mode(anc_on);
	  }
	  else{
		app_set_monitor_mode(g_set_awareness_value[0]);
		set_anc_mode(monitor);
	  }   
}

void Get_Enhance_Voice_Value(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x25,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Awareness value 1 byte  
     g_set_enhance_voice_value[0]=app_get_focus();
     head[7] =  g_set_enhance_voice_value[0];    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
		g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}

void Set_Enhance_Voice_Value(uint8_t set_enhance_voice_value[2])
{
	  if ((set_enhance_voice_value[0] > 0x01)||(set_enhance_voice_value[1]>0x01))
			  return;
	  
	  g_set_enhance_voice_value[0] =  set_enhance_voice_value[0];
   	  if (set_enhance_voice_value[1] == 0x00){  //save	  	
        g_set_anc_mode_value[0] = 0x05;  //ANC mode status is Awareness on
        app_nvrecord_focus_set(g_set_enhance_voice_value[0]);
		app_nvrecord_anc_set(MONITOR_ON);
	  }else{   //no save
	  	g_set_anc_mode_value[0] = 0x05;  //ANC mode status is Awareness on
	  }

	  app_set_clearvoice_mode(g_set_awareness_value[0]);
	  set_anc_mode(MONITOR_ON);	  
}
	 
void Get_ANC_Table_Value(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x27,0x00,0x00};
     //Data length
     head[2] = 0x09;

     uint8_t anctable =app_nvrecord_anc_table_get();
	 if(anctable>0)
	 	anctable-=1;
	 
     head[7] =anctable;    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
		g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}

void Get_Equalizer_Support_List(void)
{
    g_valueLen = 11;
    uint8_t i =0;
    uint8_t head[11] = {0xff,0x01,0x00,0x04,0x71,0x80,0x90,0x00,0x00,0x00,0x00};
     //Data length
     head[2] = 0x0b;
     //Equalizer Support List 3 byte  
     head[7] = EQUALIZER_CLASSICAL | EQUALIZER_BASS_BOOSTER | EQUALIZER_JAZZ | EQUALIZER_HIP_HOP;
     head[8] = EQUALIZER_DISABLE;
     head[9] = EQUALIZER_DISABLE;
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
}

static uint8_t g_set_eq_item_value[]= {0x00};
void Get_Equalizer_Status(void)
{
    g_set_eq_item_value[0]=app_eq_index_get();
	
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x91,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Equalizer Status 1 byte  
     head[7] =  g_set_eq_item_value[0];    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}


void Set_Equalizer_Status_Value(uint8_t set_equalizer_status_value[1])
{
   g_set_eq_item_value[0] =  set_equalizer_status_value[0];

   if((g_set_eq_item_value[0]>0x04)&&(g_set_eq_item_value[0]!=0x3f))
   	return;
   	
   change_eq_from_ble_api(g_set_eq_item_value[0]);
   app_nvrecord_eq_set(g_set_eq_item_value[0]);
}


void Get_Equalizer_Status_D4_Value(void)
 {

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x93,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Equalizer Status D4 Value 1 byte  
     head[7] = 0x00;   //D4 is Off
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
 }
/* 
void Get_Last_EQ_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x94,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Last EQ Status 1 byte  	 
     head[7] =  0x00;  //don't care  
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	      g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
   
}
 
void Notification_EQ_Change()
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0x95,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Notification_EQ_Change 1 byte  
     head[7] = 0x01; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
   
}
*/
void Get_Customization_Eq_Support_List(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xa0,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Customization Eq List 1 byte  
     head[7] = NOT_SUPPORT | BRAND0_SUPPORT | BRAND1_SUPPORT | BRAND2_SUPPORT | BRAND3_SUPPORT | BRAND4_SUPPORT| BRAND5_SUPPORT;
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
				g_valuePtr[i] = head[i];
      }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
   
}

static uint8_t g_set_custeq_brand_item_value[6]= {0};
void Get_Customization_Eq_Brand_Current_Value(void)
{
    app_eq_custom_para_get(g_set_custeq_brand_item_value);
/*
	TRACE(1,"Get_Customization_Eq_Brand_Current_Value0=%x",g_set_custeq_brand_item_value[0]);	
	TRACE(1,"Get_Customization_Eq_Brand_Current_Value1=%x",g_set_custeq_brand_item_value[1]);
	TRACE(1,"Get_Customization_Eq_Brand_Current_Value2=%x",g_set_custeq_brand_item_value[2]);
	TRACE(1,"Get_Customization_Eq_Brand_Current_Value3=%x",g_set_custeq_brand_item_value[3]);
	TRACE(1,"Get_Customization_Eq_Brand_Current_Value4=%x",g_set_custeq_brand_item_value[4]);
	TRACE(1,"Get_Customization_Eq_Brand_Current_Value5=%x",g_set_custeq_brand_item_value[5]);
*/	
    g_valueLen = 14;
    uint8_t i =0;
    uint8_t head[14] = {0xff,0x01,0x00,0x04,0x71,0x80,0xa1,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
     //Data length
     head[2] = 0x0e;
     //Customization Eq List 5 byte  
     head[7] =  g_set_custeq_brand_item_value[0]; 
     head[8] =  g_set_custeq_brand_item_value[1]; 
     head[9] =  g_set_custeq_brand_item_value[2]; 
     head[10] = g_set_custeq_brand_item_value[3]; 
     head[11] = g_set_custeq_brand_item_value[4]; 
     head[12] = g_set_custeq_brand_item_value[5]; 
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
   
}

void Set_Customization_Eq_Brand_Current_Value(uint8_t set_customization_eq_value[6])
{
   g_set_custeq_brand_item_value[0] =  set_customization_eq_value[0];
   g_set_custeq_brand_item_value[1] =  set_customization_eq_value[1];
   g_set_custeq_brand_item_value[2] =  set_customization_eq_value[2];
   g_set_custeq_brand_item_value[3] =  set_customization_eq_value[3];
   g_set_custeq_brand_item_value[4] =  set_customization_eq_value[4]; 
   g_set_custeq_brand_item_value[5] =  set_customization_eq_value[5];

   app_nvrecord_eq_param_set(set_customization_eq_value);
   change_eq_from_ble_api(0x3f);   
}

void Get_Customization_Eq_Brand_Range_Value(void)
{

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xa3,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Customization Eq Range 1 byte  
     head[7] = 0x0a;  //support 10 level
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
   
}

void Get_Customization_Eq_With_Lib(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xa4,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Customization Eq Range 1 byte  
     head[7] = 0x00;  //not using lib
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
   
}

static uint8_t g_Nowplay_Playback_Status[]= {0x00};
void Get_Nowplaying_Playback_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xb0,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Nowplaying Playback Status 1 byte  
     g_Nowplay_Playback_Status[0] = media_status;
     head[7] = g_Nowplay_Playback_Status[0]; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
				g_valuePtr[i] = head[i];
    	}	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
   
}

void Set_Nowplaying_Playback_Status_Value(uint8_t set_nowplaying_palyback_status_value[1])
{
    uint8_t playstatus = 0;
    playstatus = set_nowplaying_palyback_status_value[0] & 0x03;
	
    if ((playstatus >=0) && (playstatus < 3)){
    //set_AVRCP_Media_PlayStatus(playstatus);	
    g_Nowplay_Playback_Status[0] = set_nowplaying_palyback_status_value[0] & 0x03;
      if(!playstatus){  
		a2dp_handleKey(AVRCP_KEY_PAUSE); 
		//Notification_Media_Change();
      	}	
	else if(1==playstatus){  
		a2dp_handleKey(AVRCP_KEY_PLAY);  
	}	
	else  {           
		if(1==media_status){      
			a2dp_handleKey(AVRCP_KEY_PAUSE);    
			//Notification_Media_Change();
		}	
		else if(2==media_status)      
			a2dp_handleKey(AVRCP_KEY_PLAY);  
		} 
    }	
}

void Set_Nowplaying_Playback_Next(void)
{
   TRACE(0,"Philips : Set_Nowplaying_Playback_Next!\r\n");
   a2dp_handleKey(AVRCP_KEY_FORWARD);
   //osDelay(100);
   //Notification_Media_Change();
}

void Set_Nowplaying_Playback_Previous(void)
{
  TRACE(0,"Philips : Set_Nowplaying_Playback_Previous!\r\n");
  a2dp_handleKey(AVRCP_KEY_BACKWARD);
  //osDelay(100);
  //Notification_Media_Change();
}


void Notification_Media_Title_Artist_Album(uint8_t type,char* media_buffer, uint8_t valueLen)
{

    //First Data
    uint8_t g_valueLen = 12;
    uint8_t head[12] = {0xff,0x01,0x00,0x04,0x71,0x80,0xb5,0x00,0x00,0x00,0x00,0x00};  //Title
    if (type == (uint8_t) 0x02){
	head[6] = (uint8_t) 0xb6; 	//Artist
    }
    else if (type == (uint8_t) 0x03){
	head[6] = (uint8_t) 0xb7; 		 //Album
    }
     //Data length
     head[2] = 0x0c;
     //Notification_Media_Change 4 byte
     head[7] = (uint8_t) 0x00;
     head[8] = (uint8_t) valueLen;
     head[9] = (uint8_t) 0;
     head[10] = (uint8_t) 0x12;
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);

    uint8_t i =0;
    uint8_t j =0;

   //printf("[sink][svrcp]SONGANME data: %s\r\n", media_buffer);

   uint8_t multi_send = 0;
   multi_send = valueLen / 18;
   uint8_t single_send = 0;
   single_send = valueLen % 18;
   uint8_t count = 0;

   //uint8_t end_send = multi_send + single_send;
   //printf("[sink][svrcp]SONGANME multi_send: %d\r\n", multi_send);
   //printf("[sink][svrcp]SONGANME single_send: %d\r\n", single_send);
   //printf("[sink][svrcp]SONGANME end_send: %d\r\n", end_send);
   //printf("[sink][svrcp]SONGANME valueLen: %d\r\n", valueLen);


  if (multi_send > 0){
  	  uint8_t data[18];
	   //printf("[sink][svrcp]SONGANME Have Two Data:\r\n");
   for (i =0; i<multi_send; i++){
      for (j=0; j< 18; j++){
	  	data[j] = media_buffer[(18*i) + j];
		count ++;
      	}
	head[2] = 0x0c;
	head[7] = (uint8_t) 0x01;
	head[8] = (uint8_t) valueLen;
	head[9] = (uint8_t) count - 18;
	head[10] = (uint8_t) count -1;
	//Do checksum
      head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);

	//Philips_Send_Notify(g_conn_handle,head, g_valueLen);
	//delay_5ms(10);
	osDelay(10);//50 m by pang
	//le_tx_notify(BLE_IDX_PHILIPS_APP_TX, head, g_valueLen);
	Philips_Send_Notify(head, (uint32_t)g_valueLen);
	//delay_5ms(10);
	osDelay(10);//50 m by pang
	//Philips_Send_Notify(g_conn_handle,data, 18);
	Philips_Send_Notify(data, 18);
	//delay_5ms(10);
	osDelay(10);//50 m by pang
   }
  }

  if (multi_send == 0){
  	//printf("[sink][svrcp]SONGANME Only One Data:\r\n");
	head[2] = 0x0c;
	head[7] = (uint8_t) 0x01;
	head[8] = (uint8_t) valueLen;
	head[9] = (uint8_t) 0;
	head[10] = (uint8_t) single_send;
	//Do checksum
      head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);

	//Philips_Send_Notify(g_conn_handle,head, g_valueLen);
	//delay_5ms(10);
	osDelay(10);//50 m by pang
	//le_tx_notify(BLE_IDX_PHILIPS_APP_TX, head, g_valueLen);
	Philips_Send_Notify(head, (uint32_t)g_valueLen);
	//delay_5ms(10);
	osDelay(10);//50 m by pang
  }
  else{
	 single_send = single_send + 1;
  }

  //printf("[sink][svrcp]SONGANME count: %d\r\n", count);
  if (single_send > 0){
  	 uint8_t data1[single_send];
	 for (i =0; i<single_send; i++){

		if (multi_send == 0){
		   data1[i] = media_buffer[ i];
		   //printf("[sink][svrcp]SONGANMEi: %d\r\n", i);
		}
		else{
	         data1[i] = media_buffer[count -1 + i];
		   //printf("[sink][svrcp]SONGANME count -1 + i: %d\r\n", (count -1 + i));
		}
		//printf("[sink][svrcp]SONGANME data1[i]: %x\r\n", data1[i]);
	 }

	head[2] = 0x0c;
	head[7] = (uint8_t) 0x01;
	head[8] = (uint8_t) valueLen;
	if (count == 0){
	   head[9] = (uint8_t) count;
	   head[10] = (uint8_t) single_send -1;
	}
	else{
	   head[9] = (uint8_t) count -1;
	   head[10] = (uint8_t) count + single_send -1 ;
	}

	//Do checksum
      head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);

	//Philips_Send_Notify(g_conn_handle,head, g_valueLen);
	//delay_5ms(10);
	osDelay(10);//50 m by pang
	//le_tx_notify(BLE_IDX_PHILIPS_APP_TX, head, g_valueLen);
	Philips_Send_Notify(head, (uint32_t)g_valueLen);
	//delay_5ms(10);
	osDelay(10);//50 m by pang
	//Philips_Send_Notify(g_conn_handle,data1, single_send);
	//le_tx_notify(BLE_IDX_PHILIPS_APP_TX, data1, single_send);
	Philips_Send_Notify(data1, (uint32_t)single_send);
	//delay_5ms(10);
	osDelay(10);//50 m by pang
  }

	head[2] = 0x0c;
	head[7] = (uint8_t) 0x00;
	head[8] = (uint8_t) 0;
	head[9] = (uint8_t) 0;
	head[10] = (uint8_t) 0;
	//Do checksum
      head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
     //delay_5ms(10);
	 osDelay(10);//50 m by pang
	//Philips_Send_Notify(g_conn_handle,head, g_valueLen);
	//le_tx_notify(BLE_IDX_PHILIPS_APP_TX, head, g_valueLen);
	Philips_Send_Notify(head, (uint32_t)g_valueLen);
	//delay_5ms(10);
	osDelay(10);//50 m by pang

}


bool get_AVRCP_Media_Title(){
	bool ret = true;
	uint8_t type = 0x01;

      Notification_Media_Title_Artist_Album(type,(char*)title,title_len);
	return ret;
}

bool get_AVRCP_Media_Artist(){
	bool ret = true;
	uint8_t type = 0x02;

       Notification_Media_Title_Artist_Album(type,(char*)artist,artist_len);
	return ret;
}

bool get_AVRCP_Media_Album(){
	bool ret = true;
	uint8_t type = 0x03;

       Notification_Media_Title_Artist_Album(type,(char*)album,album_len);
	return ret;
}

void Notification_Media_Change()
{
    uint8_t valueLen = 9;
    uint8_t valuePtr[18] = {0};
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xb4,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Notification_Media_Change 1 byte
     head[7] = 0x01;
     //Do checksum
     head[valueLen - 1]=Do_CheckSum(head,valueLen);

     for (i =0;i <  valueLen; i++){
	     valuePtr[i] = head[i];
    }
    //delay_5ms(30);
    //osDelay(10);//150 c m by pang
    //le_tx_notify(BLE_IDX_PHILIPS_APP_TX, valuePtr, valueLen);
    Philips_Send_Notify(valuePtr, (uint32_t)valueLen);
	//TRACE(0,"Notification_Media_Changer\n ");

}

void Get_Media_Function_Support(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xb8,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Customization Eq Range 1 byte  
     head[7] = 0x01;  //support media title
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
   
}
/*
static uint8_t g_set_ear_detection_value[]= {0x00};
void Get_Ear_Detection_Status(void)
{

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xd0,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Ear_Detection_Status 1 byte  
     head[7] =  g_set_ear_detection_value[0];    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}


void Set_Ear_Detection_Status_Value(uint8_t set_ear_detection_value[1])
{
   g_set_ear_detection_value[0] =  set_ear_detection_value[0];
   
}

static uint8_t g_set_ear_detection_current_value[]= {0x00};
void Get_Ear_Detection_Current_Value(void)
{

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xd2,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Ear_Detection_Status 1 byte  
     head[7] =  g_set_ear_detection_current_value[0];    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}

void Notification_Ear_Detection_Change()
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xd3,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Notification_Ear_Detection_Change 1 byte  
     head[7] = 0x01; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	      g_valuePtr[i] = head[i];
    }	   
	 
     Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
   
}


static uint8_t g_set_p_sensor_calibration_value[]= {0x00};	
void Set_P_Sensor_Calibration(uint8_t set_p_sensor_calibration_value[1])
{
   g_set_p_sensor_calibration_value[0] =  set_p_sensor_calibration_value[0];
   
}	

static uint8_t g_set_p_sensor_reset_value[]= {0x00};	
void Set_P_Sensor_Reset(uint8_t set_p_sensor_reset_value[1])
{
   g_set_p_sensor_reset_value[0] =  set_p_sensor_reset_value[0];
   
}		

static uint8_t g_set_pauses_music_takeoff_value[]= {0x00};
void Get_Pauses_Music_When_TakeOff_Status(void)
{
	g_set_pauses_music_takeoff_value[0]=app_get_focus();

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xd7,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Pauses_Music_When_TakeOff_Status 1 byte  
     head[7] =  g_set_pauses_music_takeoff_value[0];    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}


void Set_Pauses_Music_When_TakeOff(uint8_t set_pauses_music_takeoff_value[1])
{
   g_set_pauses_music_takeoff_value[0] =  set_pauses_music_takeoff_value[0];

   if(g_set_pauses_music_takeoff_value[0]>0x01)
   		return;

   app_nvrecord_sensor_set(g_set_pauses_music_takeoff_value[0]);   
}	
*/
void Get_Battery_Charge_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xe0,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Battery Charge Status 1 byte  

     head[7] = 0xff; //not support charge function.
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);	
}

void Get_Right_Battery_Level_Value(void)
{
	  uint8_t Right_Battery_Level_Value[]= {0x0a};
	Right_Battery_Level_Value[0]=app_battery_current_level();  
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xe1,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Battery Level Status 1 byte  
     //g_Battery_Level_Value[0] = DRV_BAT_GetLevelInPercent();
     head[7] = Right_Battery_Level_Value[0]; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
    }	   
	 
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

void Get_Left_Battery_Level_Value(void)
{
	  uint8_t Left_Battery_Level_Value[]= {0x0a};
	Left_Battery_Level_Value[0]=app_battery_current_level();  
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xe2,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Battery Level Status 1 byte  
     //g_Battery_Level_Value[0] = DRV_BAT_GetLevelInPercent();
     head[7] = Left_Battery_Level_Value[0]; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	    g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);	
}

void Get_ChargeBox_Battery_Level_Value(void)
{
	  uint8_t ChargeBox_Battery_Level_Value[]= {0xff};  //not support
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xe3,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Battery Level Status 1 byte  
     //g_Battery_Level_Value[0] = DRV_BAT_GetLevelInPercent();
     head[7] = ChargeBox_Battery_Level_Value[0]; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	    g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

void Notification_Battery_Level_Change()
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xe4,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Notification_Media_Change 1 byte  
     head[7] = 0x01; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	      g_valuePtr[i] = head[i];
    }	   
	 
     Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

void Notification_Battery_Charge_Change()
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xe7,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Notification_Media_Change 1 byte  
     head[7] = 0x01; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	      g_valuePtr[i] = head[i];
    }	   
	 
     Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}


static uint8_t g_set_sleep_timer_value[]= {0x00};
void Get_Sleep_Mode_Timer(void)
{
    g_set_sleep_timer_value[0]=app_get_sleep_time();
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xe9,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Equalizer Status 1 byte  
     head[7] =  g_set_sleep_timer_value[0];    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}


void Set_Sleep_Mode_Timer(uint8_t set_sleep_timer_value[1])
{
   g_set_sleep_timer_value[0] =  set_sleep_timer_value[0];
   if(g_set_sleep_timer_value[0]>0x03)
   	return;
   
   app_nvrecord_sleep_time_set(g_set_sleep_timer_value[0]);
}

void Get_Smart_ChargeBox_support(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xea,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Equalizer Status 1 byte  
     head[7] =  0x00;  //not support    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}


void Get_Auto_Power_Off_Support(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xeb,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Auto_Power 1 byte  
     head[7] =  0x01;  //support    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}

static uint8_t g_auto_power_off_timer_value[]= {0x00};
void Get_Auto_Power_Off_Timer(void)
{

    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x80,0xec,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Auto_Power_Off 1 byte  
     //head[7] =  g_auto_power_off_timer_value[0];    
	 head[7] = app_get_auto_poweroff();
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen); 
}

void Set_Auto_Power_Off_Timer(uint8_t auto_power_off_timer_value[1])
{
   g_auto_power_off_timer_value[0] =  auto_power_off_timer_value[0];
   app_auto_poweroff_set(g_auto_power_off_timer_value[0]);  
}


/*
void Get_Voice_Assistant_Support_List(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x00,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Voice Assistant Support List 1 byte  
     head[7] = Google_SUPPORT ;// Tencent_SUPPORT;    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
}

void Get_Voice_Wakeup_Support_List(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x01,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Voice Assistant Support List 1 byte  
     head[7] = NOT_SUPPORT;    
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	    g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
}

static uint8_t g_set_voice_assistant_value[]= {0x00};
void Get_Voice_Assistant_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x02,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Voice_Assistant_Status 1 byte  
     head[7] =  g_set_voice_assistant_value[0];
	 
     
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	      g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
   
}

void Set_Voice_Assistant_Status(uint8_t set_voice_assistant_value[1])
{

    g_set_voice_assistant_value[0] =  set_voice_assistant_value[0];   
   	
}

uint8_t g_set_push_and_talk_value[]= {0x00};
void Get_Push_And_Talk_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x04,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Push_And_Talk_Status 1 byte  
     head[7] =  g_set_push_and_talk_value[0];
	 
     
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
   
}

void Set_Push_And_Talk_Status(uint8_t set_push_and_talk_value[1])
{

    g_set_push_and_talk_value[0] =  set_push_and_talk_value[0]; 
   	
}

void Get_Voice_Wakeup_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x06,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Push_And_Talk_Status 1 byte  
     head[7] =  NOT_SUPPORT; 
     
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
    }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
   
}

//void Set_Voice_Wakeup_Status(uint8_t set_voice_wakeup_value[1])
//{
//    uint8_t set_voice_wakeup_item =  set_voice_wakeup_value[0];
//
//   	
//}
*/

void Get_Special_Function1_Support_List(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x10,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Special Function1 Support List 1 byte  
     head[7] = TOUCH_LOCK_SUPPORT | SIDE_TONE_SUPPORT;// |DEVICE_COLOUR_SUPPORT;// | LOW_LATENCY_SUPPORT| VIBRATION_SUPPORT;  
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
}


uint8_t g_set_touch_status_value[]= {0x00};
void Get_Touch_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x11,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Touch_Status 1 byte  
     g_set_touch_status_value[0]=app_get_touchlock();
     head[7] =  g_set_touch_status_value[0];

     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
   
}

void Set_Touch_Status(uint8_t set_touch_status_value[1])
{

    g_set_touch_status_value[0] =  set_touch_status_value[0];
	
	app_nvrecord_touchlock_set(g_set_touch_status_value[0]);   	
}

/*
uint8_t g_set_running_light_status_value[]= {0x00};
void Get_Running_Light_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x13,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Running_Light_Status 1 byte  
     head[7] =  g_set_running_light_status_value[0];
	 
     
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
   
}


void Set_Running_Light_Status(uint8_t set_running_light_status_value[1])
{

    g_set_running_light_status_value[0] =  set_running_light_status_value[0]; 
   	
}
*/
	
uint8_t g_set_side_tone_value[]= {0x00};
void Get_Side_Tone_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x15,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Side_Tone_Status 1 byte  
     g_set_side_tone_value[0]=app_get_sidetone();
     head[7] =  g_set_side_tone_value[0];
 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
   
}


void Set_Side_Tone_Status(uint8_t set_side_tone_value[1])
{

    g_set_side_tone_value[0] =  set_side_tone_value[0];
	app_nvrecord_sidetone_set(g_set_side_tone_value[0]);
   	
}
/*
uint8_t g_set_low_latency_value[]= {0x00};
void Get_Low_Latency_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x17,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Side_Tone_Status 1 byte  
     head[7] =  g_set_low_latency_value[0];
	 
     
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
   
}


void Set_Low_Latency_Status(uint8_t set_low_latency_value[1])
{

    g_set_low_latency_value[0] =  set_low_latency_value[0]; 
   	
}


uint8_t g_set_vibration_status_value[]= {0x00};
void Get_Vibration_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x19,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Get_Vibration_Status 1 byte  
      g_set_vibration_status_value[0]=app_get_vibrate_mode();
     head[7] =  g_set_vibration_status_value[0];
   
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);   
   
}

void Set_Vibration_Status(uint8_t set_vibration_status_value[1])
{
    g_set_vibration_status_value[0] =  set_vibration_status_value[0]; 

	if(g_set_vibration_status_value[0]>0x01)
		return;
	
   	app_nvrecord_vibrate_mode_set(g_set_vibration_status_value[0]);
}
*/
void Notification_Touch_Key_OnOff_Change(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x1f,0x00,0x00};
     //Data length
     head[2] = 0x09;
     head[7] = app_get_touchlock();
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
			g_valuePtr[i] = head[i];
    }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
}

static uint8_t g_set_multipoint_status_value[]= {0x00};
void Get_Multipoint_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x70,0x00,0x00};
     //Data length
     head[2] = 0x09;
 
     g_set_multipoint_status_value[0]=app_get_new_multipoint_flag();
     head[7] =  g_set_multipoint_status_value[0];
   
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);      
}

void Set_Multipoint_Status(uint8_t set_multipoint_status_value[1])
{
    g_set_multipoint_status_value[0] =  set_multipoint_status_value[0]; 

	if(g_set_multipoint_status_value[0]>0x01)
		return;
	
	app_nvrecord_multipoint_set(g_set_multipoint_status_value[0]);
}

void Get_Device_Colour(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x1B,0x00,0x00};
     //Data length
     head[2] = 0x09;
 
     head[7] =  get_custom_bin_config(0);
   
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);      
}

void Get_Special_Function2_Support_List(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x60,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Special Function1 Support List 1 byte  
     head[7] = TALK_MIC_LED_CONTROL_SUPPORT;// | LOW_LATENCY_SUPPORT| VIBRATION_SUPPORT;  
	 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   
    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);  
}

void Get_TalkMicLed_Status(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x63,0x00,0x00};
     //Data length
     head[2] = 0x09;
 
     head[7] = app_get_TalkMicLed_flag(); 
   
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	     g_valuePtr[i] = head[i];
      }	   

    Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);      
}

void Set_TalkMicLed_Status(uint8_t set_status_value[1])
{
    uint8_t g_set_value= set_status_value[0];
	
	if(g_set_value>0x01)
		return;

	app_nvrecord_TalkMicLed_set(g_set_value);
}

void Notification_TalkMicLed_Change(void)
{
    g_valueLen = 9;
    uint8_t i =0;
    uint8_t head[9] = {0xff,0x01,0x00,0x04,0x71,0x81,0x65,0x00,0x00};
     //Data length
     head[2] = 0x09;
     //Notification_Media_Change 1 byte  
     head[7] = 0x01; 
     //Do checksum
     head[g_valueLen - 1]=Do_CheckSum(head,g_valueLen);
	 
     for (i =0;i <  g_valueLen; i++){
	      g_valuePtr[i] = head[i];
    }	   
	 
     Philips_Send_Notify(g_valuePtr, (uint32_t)g_valueLen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Philips_Functions_Call(uint8_t *data, uint8_t size)
{
	uint16_t command_id = ((uint16_t)data[5] << 8) | ((uint16_t)data[6]);
	   
	switch(command_id)
	{
		case GET_CONNECT_PHONE_MAC:
			//TRACE(0,"Philips : Philips_Functions_Call GET_CONNECT_PHONE_MAC!\r\n");
			Get_Connect_Phone_Mac();
			return true;
		case GET_DEVICE_FEATURE_CONFIG:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_DEVICE_FEATURE_CONFIG!\r\n");
			Get_Device_Feature_Config();
			return true;
		
		case GET_API_VERSION:
			//TRACE(0,"Philips : Philips_Functions_Call GET_API_VERSION!\r\n");
			Get_Api_Version();
			return true;	
				
		case GET_SOUND_QUALITY:
			//TRACE(0,"Philips : Philips_Functions_Call GET_SOUND_QUALITY!\r\n");
			Get_Sound_Quality();
			return true;		
			
		case GET_RIGHT_EAR_MAC_ADDRESS:
			//TRACE(0,"Philips : Philips_Functions_Call GET_RIGHT_EAR_MAC_ADDRESS!\r\n");
			Get_Right_Ear_MAC_Address();
			return true;
			
		case GET_LEFT_EAR_MAC_ADDRESS:
			//TRACE(0,"Philips : Philips_Functions_Call GET_LEFT_EAR_MAC_ADDRESS!\r\n");
			Get_Left_Ear_MAC_Address();
			return true;	
				
		case NOTIFICATION_SOUND_QUALITY_CHANGE:
			//TRACE(0,"Philips : Philips_Functions_Call NOTIFICATION_SOUND_QUALITY_CHANGE!\r\n");
			return true;		
			
		case GET_BLE_MAC_ADDRESS:
			//TRACE(0,"Philips : Philips_Functions_Call GET_BLE_MAC_ADDRESS!\r\n");
			Get_BLE_MAC_Address();
			return true;			
			
		case GET_SALES_REGION:
			//TRACE(0,"Philips : Philips_Functions_Call GET_SALES_REGION!\r\n");
			Get_Sales_Region();
			return true;		
										
		case GET_CHIPSET_VENDOR:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_CHIPSET_VENDOR!\r\n");
			Get_Chipset_Vendor();
			return true;
			
		case GET_CHIPSET_SOLUTION:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_CHIPSET_SOLUTION!\r\n");
			Get_Chipset_Solution();
			return true;
			
		case GET_FW_VERSION:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_FW_VERSION!\r\n");
			Get_Chipset_Version();
			return true;	
			
		case GET_DEVICE_TYPE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_DEVICE_TYPE!\r\n");
			Get_Device_Type();			
			return true;			
			
		case GET_FOTA_FINISH_FLAG:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_FOTA_FINISH_FLAG!\r\n");
			Get_FOTA_Finish_Flag();
			return true;
		case SET_FOTA_FINISH_FLAG:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_FOTA_FINISH_FLAG!\r\n");
			 if (size != 9){
			    return false;
			 }			
			 uint8_t fota_flag[1] = {0};
	              fota_flag[0] = data[7];		
			 Set_FOTA_Finish_Flag(fota_flag);			
			return true;
		case GET_PCBA_VERSION:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_PCBA_VERSION!\r\n");
			Get_PCBA_Version();
			return true;	
			
		case GET_ANC_MODE_SUPPORT_LIST:		
			//TRACE(0,"Philips : GET_ANC_MODE_SUPPORT_LIST!\r\n");
			Get_ANC_Mode_Support_List();
			return true;
		case GET_ANC_MODE_STATUS:		
			//TRACE(0,"Philips : GET_ANC_MODE_STATUS!\r\n");
			Get_ANC_Mode_Status();
			return true;
		case SET_ANC_MODE_VALUE:		
			//TRACE(0,"Philips : SET_ANC_MODE_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_anc_mode_status_value[1] = {0};
	             set_anc_mode_status_value[0] = data[7];
			Set_ANC_Mode_Status(set_anc_mode_status_value);		
			return true;	
		case GET_AWARENESS_VALUE:		
			//TRACE(0,"Philips : GET_AWARENESS_VALUE!\r\n");
			Get_Awareness_Value();
			return true;
		case SET_AWARENESS_VALUE:		
			TRACE(0,"Philips : SET_AWARENESS_VALUE!\r\n");
			if (size != 10){
			    return false;
			 }		
				 
			uint8_t set_awareness_value[2] = {0};
	             set_awareness_value[0] = data[7];
			set_awareness_value[1] = data[8];	 
			Set_Awareness_Value(set_awareness_value);			
			return true;
		case GET_ENHANCE_VOICE_STATUS:		
			//TRACE(0,"Philips : GET_ENHANCE_VOICE_STATUS!\r\n");
			Get_Enhance_Voice_Value();
			return true;
		case SET_ENHANCE_VOICE_STATUS:		
			//TRACE(0,"Philips : SET_ENHANCE_VOICE_STATUS!\r\n");
			if (size != 10){
			    return false;
			 }		
			
			uint8_t set_enhance_voice_value[2] = {0};
	             set_enhance_voice_value[0] = data[7];
			set_enhance_voice_value[1] = data[8];	 
			Set_Enhance_Voice_Value(set_enhance_voice_value);				
			return true;

		case GET_ANC_TABLE_VALUE:		
			//TRACE(0,"Philips : GET_ANC_TABLE_VALUE!\r\n");
			Get_ANC_Table_Value();
		return true;	
/*					
		case GET_AMBIENT_SOUND_ITEM_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_AMBIENT_SOUND_ITEM_STATUS!\r\n");
			Get_Ambient_Sound_Item_Status();
			return true;	
			
		case SET_AMBIENT_SOUND_ITEM_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_AMBIENT_SOUND_ITEM_VALUE!\r\n");
			 if (size != 9){
			    return false;
			 }
			 uint8_t ambient_item_value[1] = {0};
	              ambient_item_value[0] = data[7];
			 Set_Ambient_Sound_Item_Value(ambient_item_value);			
			return true;	
			
		case GET_AMBIENT_RANGE_SUPPORT:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_AMBIENT_RANGE_SUPPORT!\r\n");
			Get_Ambient_Range_Support();
			return true;	
			
		case GET_AMBIENT_RANGE_TOTAL:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_AMBIENT_RANGE_TOTAL!\r\n");
			Get_Ambient_Range_Total();
			return true;	
			
		case GET_AMBIENT_FOCUS_ON_VOICE_SUPPORT:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_AMBIENT_FOCUS_ON_VOICE_SUPPORT!\r\n");
			Get_Ambient_Focus_On_Voice_Support();
			return true;	
			
		case GET_ADAPTIVE_STAND_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_STAND_D4_VALUE!\r\n");
			Get_Adaptive_Stand_D4_Value();
			return true;	
			
		case GET_ADAPTIVE_WALKING_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_WALKING_D4_VALUE!\r\n");
			Get_Adaptive_Walking_D4_Value();
			return true;	
			
		case GET_ADAPTIVE_RUNNING_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_RUNNING_D4_VALUE!\r\n");
			Get_Adaptive_Running_D4_Value();
			return true;	
			
		case GET_ADAPTIVE_TRAFFIC_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_RUNNING_D4_VALUE!\r\n");
			Get_Adaptive_Traffic_D4_Value();
			return true;	
			
		case GET_AMBIENT_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_AMBIENT_D4_VALUE!\r\n");
			Get_Ambient_D4_Value();
			return true;	
			
		case GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_D4_VALUE!\r\n");
			Get_Adaptive_Stand_Focus_On_Voice_D4_Value();
			return true;	
			
		case GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_D4_VALUE!\r\n");
			Get_Adaptive_Walking_Focus_On_Voice_D4_Value();
			return true;	
			
		case GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_D4_VALUE!\r\n");
			Get_Adaptive_Running_Focus_On_Voice_D4_Value();
			return true;	
			
		case GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_D4_VALUE!\r\n");
			Get_Adaptive_Traffic_Focus_On_Voice_D4_Value();
			return true;	
			
		case GET_AMBIENT_FOCUS_ON_VOICE_D4_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_AMBIENT_FOCUS_ON_VOICE_D4_VALUE\r\n");
			Get_Adaptive_Ambient_Focus_On_Voice_D4_Value();
			return true;	
			
		case GET_ADAPTIVE_STAND_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_STAND_CURRENT_VALUE!\r\n");
			Get_Adaptive_Stand_Current_Value();
			return true;	
			
		case SET_ADAPTIVE_STAND_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_ADAPTIVE_STAND_CURRENT_VALUE!\r\n");
			 if (size != 9){
			    return false;
			 }
			 uint8_t set_stand_current_value[1] = {0};
	              set_stand_current_value[0] = data[7];	
			Set_Adaptive_Stand_Current_Value(set_stand_current_value);			
			return true;	
			
		case GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_ADAPTIVE_STAND_CURRENT_VALUE!\r\n");
			Get_Adaptive_Stand_Focus_On_Voice_Value();
			return true;
				
		case SET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			 uint8_t set_adaptive_standfocuson_value[1] = {0};
	              set_adaptive_standfocuson_value[0] = data[7];	
			Set_Adaptive_Stand_Focus_On_Voice_Value(set_adaptive_standfocuson_value);			
			return true;	
			
		case GET_ADAPTIVE_WALKING_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_WALKING_CURRENT_VALUE!\r\n");
			Get_Adaptive_Walking_Current_Value();
			return true;	
			
		case SET_ADAPTIVE_WALKING_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_ADAPTIVE_WALKING_CURRENT_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_walking_current_value[1] = {0};
	             set_walking_current_value[0] = data[7];	
			Set_Adaptive_Walking_Current_Value(set_walking_current_value);			
			return true;	
			
		case GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE!\r\n");
			Get_Adaptive_Walking_Focus_On_Voice_Value();
			return true;	
			
		case SET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_adaptive_walkingfocuson_value[1] = {0};
	             set_adaptive_walkingfocuson_value[0] = data[7];	
			Set_Adaptive_Walking_Focus_On_Voice_Value(set_adaptive_walkingfocuson_value);			
			return true;	
			
		case GET_ADAPTIVE_RUNNING_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_RUNNING_CURRENT_VALUE!\r\n");
			Get_Adaptive_Running_Current_Value();
			return true;	
			
		case SET_ADAPTIVE_RUNNING_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_ADAPTIVE_RUNNING_CURRENT_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_running_current_value[1] = {0};
	             set_running_current_value[0] = data[7];				
			Set_Adaptive_Running_Current_Value(set_running_current_value);			
			return true;	
			
		case GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE!\r\n");
			Get_Adaptive_Running_Focus_On_Voice_Value();
			return true;	
			
		case SET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_adaptive_runningfocuson_value[1] = {0};
	             set_adaptive_runningfocuson_value[0] = data[7];				
			Set_Adaptive_Running_Focus_On_Voice_Value(set_adaptive_runningfocuson_value);			
			return true;	

		case GET_ADAPTIVE_TRAFFIC_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_TRAFFIC_CURRENT_VALUE!\r\n");
			Get_Adaptive_Traffic_Current_Value();
			return true;	
			
		case SET_ADAPTIVE_TRAFFIC_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_ADAPTIVE_TRAFFIC_CURRENT_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_traffic_current_value[1] = {0};
	             set_traffic_current_value[0] = data[7];				
			Set_Adaptive_Traffic_Current_Value(set_traffic_current_value);			
			return true;		
			
		case GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE!\r\n");
			Get_Adaptive_Traffic_Focus_On_Voice_Value();
			return true;	
			
		case SET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_adaptive_trafficfocuson_value[1] = {0};
	             set_adaptive_trafficfocuson_value[0] = data[7];			
			Set_Adaptive_Traffic_Focus_On_Voice_Value(set_adaptive_trafficfocuson_value);			
			return true;	
			
		case GET_AMBIENT_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_AMBIENT_CURRENT_VALUE!\r\n");
			Get_Ambient_Current_Value();
			return true;	
			
		case SET_AMBIENT_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_AMBIENT_CURRENT_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_ambient_current_value[1] = {0};
	             set_ambient_current_value[0] = data[7];				
			Set_Ambient_Current_Value(set_ambient_current_value);			
			return true;	
			
		case GET_AMBIENT_FOCUS_ON_VOICE_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_AMBIENT_FOCUS_ON_VOICE_STATUS!\r\n");
			Get_Ambient_Focus_On_Voice_Value();
			return true;	
			
		case SET_AMBIENT_FOCUS_ON_VOICE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_AMBIENT_FOCUS_ON_VOICE_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_ambient_focuson_value[1] = {0};
	             set_ambient_focuson_value[0] = data[7];				
			Set_Ambient_Focus_On_Voice_Value(set_ambient_focuson_value);			
			return true;	
*/
/*
		case GET_HW_NOSIE_CANCELLING_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_HW_NOSIE_CANCELLING_STATUS!\r\n");
			Get_HW_Nosie_Cancelling_Status();
			return true;
		case SET_HW_ANC_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_HW_ANC_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_hw_anc_value[1] = {0};
			set_hw_anc_value[0] = data[7];					
			Set_HW_ANC_Value(set_hw_anc_value);
			return true;
		case GET_HW_AMBIENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_HW_AMBIENT_VALUE!\r\n");
			Get_HW_Ambient_value();
			return true;			
		case SET_HW_AMBIENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_HW_AMBIENT_VALUE!\r\n");
			if (size != 10){
			    return false;
			 }
			uint8_t set_hw_ambient_value[2] = {0};
	             set_hw_ambient_value[0] = data[7];		
	             set_hw_ambient_value[1] = data[8];	
			Set_HW_Ambient_value(set_hw_ambient_value);
			return true;	
		case GET_HW_FOCUS_ON_VOICE_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_HW_FOCUS_ON_VOICE_STATUS!\r\n");
			Get_HW_Focus_On_Voice_Status();
			return true;	
		case SET_HW_FOCUS_ON_VOICE_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_HW_FOCUS_ON_VOICE_STATUS!\r\n");
			if (size != 10){
			    return false;
			 }
			uint8_t set_hw_focus_on_voice_status[2] = {0};
	             set_hw_focus_on_voice_status[0] = data[7];		
	             set_hw_focus_on_voice_status[1] = data[8];				
			Set_HW_Focus_On_Voice_Status(set_hw_focus_on_voice_status);
			return true;
*/			
		case NOTIFICATIION_NOSIE_CANCELLING_CHANGE:		
			//TRACE(0,"Philips : Philips_Functions_Call NOTIFICATIION_NOSIE_CANCELLING_CHANGE!\r\n");
                     //Notification_Nosie_Cancelling_Change();			
			return true;	
					
		case GET_EQUALIZER_SUPPORT_LIST:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_EQUALIZER_SUPPORT_LIST!\r\n");
			Get_Equalizer_Support_List();
			return true;	
					
		case GET_EQUALIZER_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_EQUALIZER_STATUS!\r\n");
			Get_Equalizer_Status();
			return true;	
		case SET_EQUALIZER_STATUS_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_EQUALIZER_STATUS_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_equalizer_status_value[1] = {0};
	             set_equalizer_status_value[0] = data[7];
			Set_Equalizer_Status_Value(set_equalizer_status_value);			
			return true;	

		case GET_EQUALIZER_STATUS_D4_VALUE:
			//TRACE(0,"Philips : Philips_Functions_Call GET_EQUALIZER_STATUS_D4_VALUE!\r\n");
		       Get_Equalizer_Status_D4_Value();			
			return true;			
/*			
		case GET_LAST_EQ_STATUS:
			//TRACE(0,"Philips : Philips_Functions_Call GET_LAST_EQ_STATUS!\r\n");
			Get_Last_EQ_Status();
			return true;
			
		case NOTIFICATIION_EQ_CHANGE:
			//TRACE(0,"Philips : Philips_Functions_Call NOTIFICATIION_EQ_CHANGE!\r\n");
			//Notification_EQ_Change();			
			return true;
*/									
		case GET_CUSTOMIZATION_EQ_SUPPORT_LIST:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_CUSTOMIZATION_EQ_SUPPORT_LIST!\r\n");
			Get_Customization_Eq_Support_List();			
			return true;	
			
		case GET_CUSTOMIZATION_EQ_BAND_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_CUSTOMIZATION_EQ_BAND_STATUS!\r\n");
			Get_Customization_Eq_Brand_Current_Value();			
			return true;	
			
		case SET_CUSTOMIZATION_EQ_BAND_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_CUSTOMIZATION_EQ_BAND_VALUE!\r\n");
			if (size != 14){
			    return false;
			 }
			uint8_t set_customization_eq_value[6] = {0};
	        set_customization_eq_value[0] = data[7];		
			set_customization_eq_value[1] = data[8];
			set_customization_eq_value[2] = data[9];
			set_customization_eq_value[3] = data[10];
			set_customization_eq_value[4] = data[11];
			set_customization_eq_value[5] = data[12];
			Set_Customization_Eq_Brand_Current_Value(set_customization_eq_value);			
			return true;	
			
			
		case GET_CUSTOMIZATION_EQ_BAND_RANGE_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_CUSTOMIZATION_EQ_BAND_RANGE_VALUE!\r\n");
			Get_Customization_Eq_Brand_Range_Value();			
			return true;	
			
		case GET_CUSTOMIZATION_EQ_WITH_LIB:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_CUSTOMIZATION_EQ_WITH_LIB!\r\n");
			Get_Customization_Eq_With_Lib();
			return true;	
												
		case GET_NOWPLAYING_PLAYBACK_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_NOWPLAYING_PLAYBACK_STATUS!\r\n");
			Get_Nowplaying_Playback_Status();			
			return true;		
			
		case SET_NOWPLAYING_PLAYBACK_STATUS_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_NOWPLAYING_PLAYBACK_STATUS_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_nowplaying_palyback_status_value[1] = {0};
	             set_nowplaying_palyback_status_value[0] = data[7];					
			Set_Nowplaying_Playback_Status_Value(set_nowplaying_palyback_status_value);			
			return true;	
					
		case SET_NOWPLAYING_PLAYBACK_NEXT:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_NOWPLAYING_PLAYBACK_NEXT!\r\n");
			Set_Nowplaying_Playback_Next();			
			return true;	
		case SET_NOWPLAYING_PLAYBACK_PREVIOUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_NOWPLAYING_PLAYBACK_PREVIOUS!\r\n");
			Set_Nowplaying_Playback_Previous();			
			return true;		
			
		case NOTIFICATION_MEDIA_CHANGE:		
			//TRACE(0,"Philips : Philips_Functions_Call NOTIFICATION_MEDIA_CHANGE!\r\n");
			//Notification_Media_Change();
			return true;	
						
		case GET_MEDIA_TITLE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_MEDIA_TITLE!\r\n");
			get_AVRCP_Media_Title();
			return true;	
		case GET_MEDIA_ARTIST:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_MEDIA_ARTIST!\r\n");
			get_AVRCP_Media_Artist();
			return true;	
		case GET_MEDIA_ALBUM:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_MEDIA_ALBUM!\r\n");
			get_AVRCP_Media_Album();
			return true;	
		case GET_MEDIA_FUNTION_SUPPORT:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_MEDIA_FUNTION_SUPPORT!\r\n");
			Get_Media_Function_Support();
			return true;					
	
/*					
		case GET_HEART_RATE_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_HEART_RATE_STATUS!\r\n");
			Get_Heart_Rate_Status();			
			return true;	
		case SET_HEART_RATE_STATUS_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_HEART_RATE_STATUS_VALUE!\r\n");

			if (size != 9){
			    return false;
			 }
			uint8_t set_heart_rate_status_value[1] = {0};
	             set_heart_rate_status_value[0] = data[7];					
			Set_Heart_Rate_Status_Value(set_heart_rate_status_value);			
			return true;	
		case GET_HEART_RATE_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_HEART_RATE_CURRENT_VALUE!\r\n");
			Get_Heart_Rate_Current_Value();			
			return true;		
		case GET_HEART_RATE_AUTO_NODIFY_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_HEART_RATE_AUTO_NODIFY_STATUS!\r\n");
			Get_Heart_Rate_Auto_Nodify_Status();			
			return true;	
		case SET_HEART_RATE_AUTO_NODIFY_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_HEART_RATE_AUTO_NODIFY_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_heart_rate_auto_nodify_value[1] = {0};
	             set_heart_rate_auto_nodify_value[0] = data[7];				
                   Set_Heart_Rate_Auto_Nodify_Value(set_heart_rate_auto_nodify_value);					
			return true;	
*/
/*
		case GET_EAR_DETECTION_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_EAR_DETECTION_STATUS!\r\n");
			Get_Ear_Detection_Status();			
			return true;	
		case SET_EAR_DETECTION_STATUS_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_EAR_DETECTION_STATUS_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_ear_detection_status_value[1] = {0};
	             set_ear_detection_status_value[0] = data[7];				
			Set_Ear_Detection_Status_Value(set_ear_detection_status_value);			
			return true;	
		case GET_EAR_DETECTION_CURRENT_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_EAR_DETECTION_CURRENT_VALUE!\r\n");
			Get_Ear_Detection_Current_Value();			
			return true;		
		case NOTIFICATION_EAR_DETECTION_CHANGE:		
			//TRACE(0,"Philips : Philips_Functions_Call NOTIFICATION_EAR_DETECTION_CHANGE!\r\n");
			//Notification_Ear_Detection_Change();
			return true;	
		case P_SENSOR_CALIBRATION:		
			//TRACE(0,"Philips : Philips_Functions_Call P_SENSOR_CALIBRATION!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_p_sensor_calibration_value[1] = {0};
	             set_p_sensor_calibration_value[0] = data[7];		
			Set_P_Sensor_Calibration(set_p_sensor_calibration_value);
			return true;	
		case P_SENSOR_RESET:		
			//TRACE(0,"Philips : Philips_Functions_Call P_SENSOR_RESET!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_p_sensor_reset_value[1] = {0};
	             set_p_sensor_reset_value[0] = data[7];		
			Set_P_Sensor_Reset(set_p_sensor_reset_value);			
			return true;	
		case SET_PAUSES_MUSIC_WHEN_TAKEOFF:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_PAUSES_MUSIC_WHEN_TAKEOFF!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_pauses_music_when_takeoff_value[1] = {0};
	             set_pauses_music_when_takeoff_value[0] = data[7];		
			Set_Pauses_Music_When_TakeOff(set_pauses_music_when_takeoff_value);	
			return true;		
		case GET_PAUSES_MUSIC_WHEN_TAKEOFF:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_PAUSES_MUSIC_WHEN_TAKEOFF!\r\n");
			Get_Pauses_Music_When_TakeOff_Status();
			return true;			
*/	
		case GET_BATTERY_CHARGE_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_BATTERY_CHARGE_STATUS!\r\n");
			Get_Battery_Charge_Status();
			return true;	

		case GET_RIGHT_EAR_BATTERY_LEVEL_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_BATTERY_AUTO_RELTURN_LEVEL_VALUE!\r\n");
			Get_Right_Battery_Level_Value();
			return true;	
			
		case GET_LEFT_EAR_BATTERY_LEVEL_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_BATTERY_AUTO_RELTURN_LEVEL_VALUE!\r\n");
			Get_Left_Battery_Level_Value();
			return true;	
			
		case GET_CHARGE_BOX_BATTERY_LEVEL_VALUE:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_BATTERY_AUTO_RELTURN_LEVEL_VALUE!\r\n");
			Get_ChargeBox_Battery_Level_Value();
			return true;	
			
		case NOTIFICATION_BATTERY_LEVEL_CHANGE:		
			//Notification_Battery_Level_Change();			
			return true;	
			
		case GET_BATTERY_NOTIFY_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_BATTERY_NOTIFY_STATUS!\r\n");
      //Don't care
			//Get_Battery_Notify_Status();
			return true;				
			
		case SET_BATTERY_NOTIFY_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_BATTERY_NOTIFY_STATUS!\r\n");
			//Don't care
			//if (size != 9){
			//    return false;
			// }	
			//uint8_t set_status[1] = {0};
	    //         set_status[0] = data[7];				
			//Set_Battery_Notify_Status(set_status);
			return true;	
		case NOTIFICATION_BATTERY_CHARGE_CHANGE:		
			//TRACE(0,"Philips : Philips_Functions_Call NOTIFICATION_BATTERY_CHARGE_CHANGE!\r\n");
			//Notification_Battery_Charge_Change();					
			return true;	
		case SET_SLEEP_MODE_TIMER:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_SLEEP_MODE_TIMER!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_sleep_timer_value[1] = {0};
	             set_sleep_timer_value[0] = data[7];						
			Set_Sleep_Mode_Timer(set_sleep_timer_value);
			return true;				
		case GET_SLEEP_MODE_TIMER:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_SLEEP_MODE_TIMER!\r\n");
			Get_Sleep_Mode_Timer();
			return true;		
		case GET_SMART_CHARGEBOX_SUPPORT:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_SMART_CHARGEBOX_SUPPORT!\r\n");
                   Get_Smart_ChargeBox_support();
			return true;	
		case GET_AUTO_POWER_OFF_SUPPORT:		
			//TRACE(0,"Philips : GET_AUTO_POWER_OFF_SUPPORT!\r\n");
			Get_Auto_Power_Off_Support();
			return true;	
		case GET_AUTO_POWER_OFF_TIMER:		
			//TRACE(0,"Philips : GET_AUTO_POWER_OFF_TIMER!\r\n");
			Get_Auto_Power_Off_Timer();
			return true;
		case SET_AUTO_POWER_OFF_TIMER:		
			//TRACE(0,"Philips : SET_AUTO_POWER_OFF_TIMER!\r\n");
			if (size != 9){
			    return false;
			 }
			uint8_t set_auto_power_off_value[1] = {0};
	             set_auto_power_off_value[0] = data[7];		
			Set_Auto_Power_Off_Timer(set_auto_power_off_value);
			return true;				
		/*	
		case GET_MULTI_LANGUAGE_VOICE_PROMPT_SUPPORT_LIST:		
			//TRACE(0,"Philips : GET_MULTI_LANGUAGE_VOICE_PROMPT_SUPPORT_LIST!\r\n");
			Get_Multi_Language_Voice_Prompt_Support_List();			
			return true;		
		case GET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS:		
			//TRACE(0,"Philips : GET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS!\r\n");
			Get_Multi_Language_Voice_Prompt_Value();			
			return true;	
		case SET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS_VALUE:		
			//TRACE(0,"Philips : SET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS_VALUE!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_Multi_language_voice_prompt_value[1] = {0};
	             set_Multi_language_voice_prompt_value[0] = data[7];		
			Set_Multi_Language_Voice_Prompt_Value(set_Multi_language_voice_prompt_value);			
			return true;
		case GET_MULTI_LANGUAGE_VOICE_PROMPT_D4_VALUE:		
			//TRACE(0,"Philips : GET_MULTI_LANGUAGE_VOICE_PROMPT_D4_VALUE!\r\n");
			Get_Multi_Language_Voice_Prompt_D4_Value();			
			return true;

		case GET_VOICE_ASSISTANT_SUPPORT_LIST:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_VOICE_ASSISTANT_SUPPORT_LIST!\r\n");
			Get_Voice_Assistant_Support_List();			
			return true;
		case GET_VOICE_WAKEUP_SUPPORT_LIST:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_VOICE_WAKEUP_SUPPORT_LIST!\r\n");
			Get_Voice_Wakeup_Support_List();
			return true;
		case GET_VOICE_ASSISTANT_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_VOICE_ASSISTANT_STATUS!\r\n");
			Get_Voice_Assistant_Status();
			return true;
		case SET_VOICE_ASSISTANT_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_VOICE_ASSISTANT_STATUS!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_voice_assistant_value[1] = {0};
	             set_voice_assistant_value[0] = data[7];		
			Set_Voice_Assistant_Status(set_voice_assistant_value);
			return true;
		case GET_PUSH_AND_TALK_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_PUSH_AND_TALK_STATUS!\r\n");
			Get_Push_And_Talk_Status();
			return true;
		case SET_PUSH_AND_TALK_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_PUSH_AND_TALK_STATUS!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_push_and_talk_value[1] = {0};
	             set_push_and_talk_value[0] = data[7];				
			Set_Push_And_Talk_Status(set_push_and_talk_value);
			return true;
		case GET_VOICE_WAKEUP_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_VOICE_WAKEUP_STATUS!\r\n");
			Get_Voice_Wakeup_Status();
			return true;
		case SET_VOICE_WAKEUP_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_VOICE_WAKEUP_STATUS!\r\n");
			//if (size != 9){
			//    return false;
			// }		
			//uint8_t set_voice_wakeup_value[1] = {0};
	    //         set_voice_wakeup_value[0] = data[7];		
			//Set_Voice_Wakeup_Status(set_voice_wakeup_value);
			return true;	
*/				
		case GET_SPECIAL_FUNCTION1_SUPPORT_LIST:
			//TRACE(0,"Philips : Philips_Functions_Call GET_SPECIAL_FUNCTION1_SUPPORT_LIST!\r\n");
			Get_Special_Function1_Support_List();
		      return true;
			  
		case GET_TOUCH_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_TOUCH_STATUS!\r\n");
			Get_Touch_Status();
			return true;				
		case SET_TOUCH_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_TOUCH_STATUS!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_touch_status_value[1] = {0};
	             set_touch_status_value[0] = data[7];		
			Set_Touch_Status(set_touch_status_value);
			return true;	
/*			
		case GET_RUNNING_LIGHT_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_RUNNING_LIGHT_STATUS!\r\n");
			Get_Running_Light_Status();
			return true;				
		case SET_RUNNING_LIGHT_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_RUNNING_LIGHT_STATUS!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_running_light_status_value[1] = {0};
	             set_running_light_status_value[0] = data[7];		
			Set_Running_Light_Status(set_running_light_status_value);
			return true;
*/
		case GET_SIDE_TONE_CONTROL_STATUS:		
			//TRACE(0,"Philips :Philips_Functions_Call GET_SIDE_TONE_CONTROL_STATUS!\r\n");
			Get_Side_Tone_Status();
			return true;				
		case SET_SIDE_TONE_CONTROL_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_SIDE_TONE_CONTROL_STATUS!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_side_tone_value[1] = {0};
	             set_side_tone_value[0] = data[7];		
			Set_Side_Tone_Status(set_side_tone_value);			
			return true;
/*
		case GET_LOW_LATENCY_STATUS:		
			//TRACE(0,"Philips :Philips_Functions_Call GET_LOW_LATENCY_STATUS!\r\n");
			Get_Low_Latency_Status();
			return true;				
		case SET_LOW_LATENCY_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_LOW_LATENCY_STATUS!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_low_latency_value[1] = {0};
	             set_low_latency_value[0] = data[7];		
			Set_Low_Latency_Status(set_low_latency_value);			
			return true;			
		case GET_VIBRATION_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call GET_VIBRATION_STATUS!\r\n");
			Get_Vibration_Status();		
			return true;				
		case SET_VIBRATION_STATUS:		
			//TRACE(0,"Philips : Philips_Functions_Call SET_VIBRATION_STATUS!\r\n");
			if (size != 9){
			    return false;
			 }		
			uint8_t set_vibration_status_value[1] = {0};
	             set_vibration_status_value[0] = data[7];		
			Set_Vibration_Status(set_vibration_status_value);				
			return true;
*/
		case GET_MULTIPOINT_STATUS:
			Get_Multipoint_Status();
		return true;

		case SET_MULTIPOINT_STATUS:
			if (size != 9){
			    return false;
			}		
			uint8_t set_multipoint_value[1] = {0};
	             set_multipoint_value[0] = data[7];		
			Set_Multipoint_Status(set_multipoint_value);
			return true;

		case GET_DEVICE_COLOUR_STATUS:
			Get_Device_Colour();
		return true;

		case GET_SPECIAL_FUNCTION2_SUPPORT_LIST:
			//TRACE(0,"Philips : Philips_Functions_Call GET_SPECIAL_FUNCTION1_SUPPORT_LIST!\r\n");
			Get_Special_Function2_Support_List();
		      return true;
		
		case GET_TALK_MIC_LED_STATUS:
			Get_TalkMicLed_Status();
			return true;

		case SET_TALK_MIC_LED_STATUS:
			if (size != 9){
			    return false;
			}	
			uint8_t set_value[] = {0};
	                set_value[0] = data[7];		
			Set_TalkMicLed_Status(set_value);
			return true;

		default:
			TRACE(0,"Philips : Philips_Functions_Call Command error!\r\n");
			break;
	}
	
	return false;
}
