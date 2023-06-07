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
#ifndef __PHILIPS_BLE_API_H__
#define __PHILIPS_BLE_API_H__

#ifdef __cplusplus
extern "C" {
#endif
//Philips BLE
#include "hal_timer.h"
extern u8 title[150], title_len;
extern u8 artist[150], artist_len;
extern u8 album[150], album_len;

	typedef enum PHILIPS_BASE_COMMAND
	{
		GET_CONNECT_PHONE_MAC = 0x8000,			//0x8000
		GET_DEVICE_FEATURE_CONFIG,				//0x8001
		GET_API_VERSION,				                   //0x8002
		GET_SOUND_QUALITY,						//0x8003
		GET_RIGHT_EAR_MAC_ADDRESS,				//0x8004
		GET_LEFT_EAR_MAC_ADDRESS,				//0x8005
		NOTIFICATION_SOUND_QUALITY_CHANGE,				//0x8006
		GET_BLE_MAC_ADDRESS,				//0x8007
		GET_SALES_REGION,				//0x8008
	}PHILIPS_BASE_COMMAND;

	typedef enum PHILIPS_FOTA_COMMAND
	{
		GET_CHIPSET_VENDOR = 0x8010,	             //0x8010
		GET_CHIPSET_SOLUTION,					//0x8011
		GET_FW_VERSION,						//0x8012
		GET_DEVICE_TYPE,						//0x8013
		GET_FOTA_FINISH_FLAG,                           //0X8014
		SET_FOTA_FINISH_FLAG,                           //0X8015	
		GET_PCBA_VERSION,                           //0X8016
	}PHILIPS_FOTA_COMMAND;

	typedef enum PHILIPS_ANC_COMMAND
	{
		GET_ANC_SUPPORT_LIST = 0x8020,	       //0x8020
		GET_ANC_STATUS,					//0x8021
		SET_ANC_VALUE,
		NOTIFICATION_ANC_CHANGE,
	}PHILIPS_ANC_COMMAND;

	typedef enum PHILIPS_ANC_MODE_COMMAND
	{
 	  GET_ANC_MODE_SUPPORT_LIST = 0x8020,	       //0x8020
 	  GET_ANC_MODE_STATUS = 0x8021,	             //0x8021
 	  SET_ANC_MODE_VALUE = 0x8022,	                   //0x8022
 	  GET_AWARENESS_VALUE = 0x8023,	            //0x8023
	  SET_AWARENESS_VALUE = 0x8024,	            //0x8024
	  GET_ENHANCE_VOICE_STATUS = 0x8025,        //0x8025
	  SET_ENHANCE_VOICE_STATUS = 0x8026,        //0x8026
	  GET_ANC_TABLE_VALUE = 0x8027,        //0x8027
	}PHILIPS_ANC_MODE_COMMAND;
	
	typedef enum PHILIPS_AMBIENT_COMMAND
	{
		GET_AMBIENT_SOUND_ITEM_STATUS = 0x8030,	       //0x8030
		SET_AMBIENT_SOUND_ITEM_VALUE,					//0x8031
		GET_AMBIENT_RANGE_SUPPORT,                             //0x8032
		GET_AMBIENT_RANGE_TOTAL,                                 //0x8033
		GET_AMBIENT_FOCUS_ON_VOICE_SUPPORT,           //0x8034
		GET_ADAPTIVE_STAND_D4_VALUE,                        //0x8035
		GET_ADAPTIVE_WALKING_D4_VALUE,                    //0x8036
		GET_ADAPTIVE_RUNNING_D4_VALUE,                    //0x8037
		GET_ADAPTIVE_TRAFFIC_D4_VALUE,                    //0x8038
		GET_AMBIENT_D4_VALUE,                                    //0x8039
		GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_D4_VALUE,               //0x803a
		GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_D4_VALUE,           //0x803b
		GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_D4_VALUE,           //0x803c
		GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_D4_VALUE,           //0x803d
		GET_AMBIENT_FOCUS_ON_VOICE_D4_VALUE,                          //0x803e
		GET_ADAPTIVE_STAND_CURRENT_VALUE,                                //0x803f
		SET_ADAPTIVE_STAND_CURRENT_VALUE,                                //0x8040
		GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE,                 //0x8041
		SET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE,                 //0x8042
		GET_ADAPTIVE_WALKING_CURRENT_VALUE,                            //0x8043
		SET_ADAPTIVE_WALKING_CURRENT_VALUE,                            //0x8044
		GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE,              //0x8045
             SET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE,              //0x8046
		GET_ADAPTIVE_RUNNING_CURRENT_VALUE,                            //0x8047
		SET_ADAPTIVE_RUNNING_CURRENT_VALUE,                            //0x8048
		GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE,              //0x8049
             SET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE,		 //0x804a
             GET_ADAPTIVE_TRAFFIC_CURRENT_VALUE,                            //0x804b
		SET_ADAPTIVE_TRAFFIC_CURRENT_VALUE,                            //0x804c
		GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE,              //0x804d
             SET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE,		        //0x804e	
		GET_AMBIENT_CURRENT_VALUE,                                            //0x804f
		SET_AMBIENT_CURRENT_VALUE,                                            //0x8050
		GET_AMBIENT_FOCUS_ON_VOICE_STATUS,                            //0x8051
		SET_AMBIENT_FOCUS_ON_VOICE_VALUE,                             //0x8052
		GET_HW_NOSIE_CANCELLING_STATUS,                                 //0x8053
             SET_HW_ANC_VALUE,                                                         //0x8054
             GET_HW_AMBIENT_VALUE,                                                  //0x8055
             SET_HW_AMBIENT_VALUE,                                                  //0x8056
             GET_HW_FOCUS_ON_VOICE_STATUS,                                  //0x8057
		SET_HW_FOCUS_ON_VOICE_STATUS,                                  //0x8058		
		NOTIFICATIION_NOSIE_CANCELLING_CHANGE= 0x8059,                    //0x8059
	}PHILIPS_AMBIENT_COMMAND;

	typedef enum PHILIPS_EQUALIZER_COMMAND
	{
		GET_EQUALIZER_SUPPORT_LIST = 0x8090,	       //0x8090
		GET_EQUALIZER_STATUS,					       //0x8091
		SET_EQUALIZER_STATUS_VALUE,						//0x8092
		GET_EQUALIZER_STATUS_D4_VALUE,				//0x8093
		GET_LAST_EQ_STATUS,										//0x8094
		NOTIFICATIION_EQ_CHANGE,							//0x8095
	}PHILIPS_EQUALIZER_COMMAND;

	typedef enum PHILIPS_CUSTOMIZATION_EQ_COMMAND
	{
		GET_CUSTOMIZATION_EQ_SUPPORT_LIST = 0x80a0,	       //0x80a0
		GET_CUSTOMIZATION_EQ_BAND_VALUE,				 //0x80a1
		SET_CUSTOMIZATION_EQ_BAND_VALUE,				//0x80a2
		GET_CUSTOMIZATION_EQ_BAND_RANGE_VALUE,		//0x80a3
		GET_CUSTOMIZATION_EQ_WITH_LIB,					//0x80a4
	}PHILIPS_CUSTOMIZATION_EQ_COMMAND;

	typedef enum PHILIPS_NOWPLAYING_COMMAND
	{
		GET_NOWPLAYING_PLAYBACK_STATUS = 0x80b0,	       //0x80B0
		SET_NOWPLAYING_PLAYBACK_STATUS_VALUE,			//0x80B1
		SET_NOWPLAYING_PLAYBACK_NEXT,							//0x80B2
		SET_NOWPLAYING_PLAYBACK_PREVIOUS,						//0x80B3
		NOTIFICATION_MEDIA_CHANGE,							//0x80B4
		GET_MEDIA_TITLE,													//0x80B5
		GET_MEDIA_ARTIST,													//0x80B6
		GET_MEDIA_ALBUM,													//0x80B7
		GET_MEDIA_FUNTION_SUPPORT,								//0x80B8
	}PHILIPS_NOWPLAYING_COMMAND;


	typedef enum PHILIPS_HEART_RATE_COMMAND
	{
		GET_HEART_RATE_STATUS = 0x80c0,	              //0x80C0
		SET_HEART_RATE_STATUS_VALUE,			       //0x80C1
		GET_HEART_RATE_CURRENT_VALUE,
		GET_HEART_RATE_AUTO_NODIFY_STATUS,
		SET_HEART_RATE_AUTO_NODIFY_VALUE,
	}PHILIPS_HEART_RATE_COMMAND;
	
	typedef enum PHILIPS_EAR_DETECTION_COMMAND
	{
		GET_EAR_DETECTION_STATUS = 0x80d0,	              //0x80d0
		SET_EAR_DETECTION_STATUS_VALUE,			       //0x80d1
		GET_EAR_DETECTION_CURRENT_VALUE,			//0x80d2
		NOTIFICATION_EAR_DETECTION_CHANGE,			//0x80d3
		P_SENSOR_CALIBRATION,                                       //0x80d4
		P_SENSOR_RESET,                                                  //0x80d5
             SET_PAUSES_MUSIC_WHEN_TAKEOFF,                    //0x80d6
             GET_PAUSES_MUSIC_WHEN_TAKEOFF,                   //0x80d7
	}PHILIPS_EAR_DETECTION_COMMAND;


	typedef enum PHILIPS_BATTERY_COMMAND
	{
		GET_BATTERY_CHARGE_STATUS = 0x80e0,	              //0x80e0
		GET_RIGHT_EAR_BATTERY_LEVEL_VALUE,			//0x80e1
		GET_LEFT_EAR_BATTERY_LEVEL_VALUE,			//0x80e2
		GET_CHARGE_BOX_BATTERY_LEVEL_VALUE,		//0x80e3	
		NOTIFICATION_BATTERY_LEVEL_CHANGE,			//0x80e4
		GET_BATTERY_NOTIFY_STATUS,					//0x80e5
		SET_BATTERY_NOTIFY_STATUS,					//0x80e6	
		NOTIFICATION_BATTERY_CHARGE_CHANGE,		//0x80e7
		SET_SLEEP_MODE_TIMER,							//0x80e8
		GET_SLEEP_MODE_TIMER,							//0x80e9
		GET_SMART_CHARGEBOX_SUPPORT,				//0x80ea
		GET_AUTO_POWER_OFF_SUPPORT,						//0x80eb
		GET_AUTO_POWER_OFF_TIMER,						//0x80ec
		SET_AUTO_POWER_OFF_TIMER,						//0x80eb
	}PHILIPS_BATTERY_COMMAND;

	typedef enum PHILIPS_LANGUAGE_VOICE_COMMAND
	{
		GET_MULTI_LANGUAGE_VOICE_PROMPT_SUPPORT_LIST = 0x80f0,	                           //0x80f0
		GET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS,			                    //0x80F1
		SET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS_VALUE,
		GET_MULTI_LANGUAGE_VOICE_PROMPT_D4_VALUE,
	}PHILIPS_LANGUAGE_VOICE_COMMAND;

	typedef enum PHILIPS_VOICE_ASSISTANT_COMMAND
	{
		GET_VOICE_ASSISTANT_SUPPORT_LIST = 0x8100,	                        //0x8100
		GET_VOICE_WAKEUP_SUPPORT_LIST,	                           			//0x8101
		GET_VOICE_ASSISTANT_STATUS,	                           				//0x8102
		SET_VOICE_ASSISTANT_STATUS,	                           				//0x8103
		GET_PUSH_AND_TALK_STATUS,	                           				//0x8104
		SET_PUSH_AND_TALK_STATUS,	                           				//0x8105
		GET_VOICE_WAKEUP_STATUS,	                           				//0x8106
		SET_VOICE_WAKEUP_STATUS,	                           				//0x8107				
	}PHILIPS_VOICE_ASSISTANT_COMMAND;
	
	typedef enum PHILIPS_SPECIAL_FUNCTION1_COMMAND
	{
		GET_SPECIAL_FUNCTION1_SUPPORT_LIST = 0x8110,	         //0x8110
		GET_TOUCH_STATUS,                                        //0x8111
		SET_TOUCH_STATUS,	                                     //0x8112		
		GET_RUNNING_LIGHT_STATUS,                                //0x8113
		SET_RUNNING_LIGHT_STATUS,	                             //0x8114	
		GET_SIDE_TONE_CONTROL_STATUS,                            //0x8115
		SET_SIDE_TONE_CONTROL_STATUS,	                         //0x8116		
		GET_LOW_LATENCY_STATUS,                                  //0x8117
		SET_LOW_LATENCY_STATUS,	                                 //0x8118	
		GET_VIBRATION_STATUS,                                    //0x8119
		SET_VIBRATION_STATUS,	                                 //0x811a
		GET_DEVICE_COLOUR_STATUS,
	}PHILIPS_SPECIAL_FUNCTION1_COMMAND;

	typedef enum PHILIPS_SPECIAL_FUNCTION2_COMMAND
	{
		GET_SPECIAL_FUNCTION2_SUPPORT_LIST=0x8160,         		//0x8160
		GET_SPECIAL_FUNCTION2_reserved1,
		SET_SPECIAL_FUNCTION2_reserved1,
		GET_TALK_MIC_LED_STATUS,                                                           
		SET_TALK_MIC_LED_STATUS,
		NOTIFICATIION_TALK_MIC_LED_CHANGE,
	}PHILIPS_SPECIAL_FUNCTION2_COMMAND;
	
	typedef enum PHILIPS_MULTIPOINT_COMMAND
	{
		GET_MULTIPOINT_STATUS=0x8170,                         //0x8170
		SET_MULTIPOINT_STATUS,	                                  //0x8171						
	}PHILIPS_MULTIPOINT_COMMAND;

//Byte 0
#define FOTA_SUPPORT 				  0x01
#define ANC_MODE_SUPPORT 			  0x02
#define SMART_ANC_SUPPORT 	               0x04
#define EQUALIZER_SUPPORT 			  0x08
#define CUSTOMIZATION_EQ_SUPPORT 	  0x10
#define NOWPLAYING_SUPPORT 		  0x20
#define HEART_RATE_SUPPORT 		  0x40
#define EAR_DETECTION_SUPPORT 		  0x80
//Byte 1
#define BATTERY_STATUS_SUPPORT 				       0x01
#define MULTI_LANGAUAGE_VOICE_PROMPT_SUPPORT 	 0x02
#define AI_VOICE_ASSISTANT_SUPPORT 	                    0x04
#define SPECIAL_FUNCTION1_SUPPORT 			              0x08
#define INTERNAL_RTC_SUPPORT 	        				0x10
#define KIDS_MODE_SUPPORT 		        				0x20
#define SPECIAL_FUNCTION2_SUPPORT 		  			0x40
#define MULTIPOINT_SUPPORT                				0x80
//Byte 2
#define GAME_MODE_SUPPORT 				       	0x01
#define KEY_DEFINE_CHANGE_SUPPORT 	 			0x02

//ANC
#define ANC_OFF_SUPPORT 			  0x01
#define ANC_MODE1_SUPPORT 	        0x02
#define ANC_MODE2_SUPPORT 	        0x04
#define ANC_MODE3_SUPPORT 	        0x08
#define ANC_MODE4_SUPPORT 	        0x10
#define AWARENESS_SUPPORT 	               0x20
#define ENHANCE_VOICE_SUPPORT 	  0x40
#define AWARENESS_SLIDE_SUPPORT 	  0x80


//Ambient
#define NOT_SUPPORT 			  0x00
#define SUPPORT 			               0x01
//Ambient Total Level
#define AMBIENT_TOTAL_5 		  0x05
#define AMBIENT_TOTAL_10 		  0x0a
#define AMBIENT_TOTAL_15 		  0x0f
#define AMBIENT_TOTAL_20 		  0x14

//Equalizer support list
#define EQUALIZER_DISABLE			  0x00
#define EQUALIZER_BASS_BOOSTER 	  0x01
#define EQUALIZER_CLASSICAL 		  0x02
#define EQUALIZER_JAZZ   			  0x04
#define EQUALIZER_HIP_HOP			  0x08
#define EQUALIZER_DANCE 	               0x10
#define EQUALIZER_POP 	             	  0x20
#define EQUALIZER_ELECTRONIC       	  0x40
#define EQUALIZER_TREBLE_BOOSTER     0x80

#define EQUALIZER_ACOUSTIC 		  0x01
#define EQUALIZER_BASS_REDUCER	  0x02
#define EQUALIZER_LATIN        		  0x04
#define EQUALIZER_LOUDNESS       	  0x08
#define EQUALIZER_LOUNGE	               0x10
#define EQUALIZER_PIANO	             	  0x20
#define EQUALIZER_DEEP               	  0x40
#define EQUALIZER_R_B                         0x80

#define EQUALIZER_SMALL_SPEAKERS     0x01
#define EQUALIZER_SPOKEN_WORD         0x02
#define EQUALIZER_FLAT        		         0x04
#define EQUALIZER_TREBLE_REDUCER      0x08
#define EQUALIZER_BOCAL_BOOSTER       0x10


//Equalizer value
#define DISABLE			  0x00
#define BASS_BOOSTER 	  0x01
#define CLASSICAL 		  0x02
#define JAZZ   			  0x03
#define HIP_HOP			  0x04
#define DANCE 	               0x05
#define POP 	             	  0x06
#define ELECTRONIC       	  0x07
#define TREBLE_BOOSTER    0x08
#define ACOUSTIC 		  0x09
#define BASS_REDUCER	  0x0a
#define LATIN        		  0x0b
#define LOUDNESS       	  0x0c
#define LOUNGE	               0x0d
#define PIANO	               0x0e
#define DEEP               	  0x0f
#define R_B                        0x10
#define SMALL_SPEAKERS    0x11
#define SPOKEN_WORD        0x12
#define FLAT                        0x13
#define TREBLE_REDUCER     0x14
#define BOCAL_BOOSTER      0x15

//Custom EQ support list
#define NOT_SUPPORT			                0x00
#define BRAND0_SUPPORT           		   0x01
#define BRAND1_SUPPORT                       0x02
#define BRAND2_SUPPORT                  	   0x04
#define BRAND3_SUPPORT          		   0x08
#define BRAND4_SUPPORT               	   0x10
#define BRAND5_SUPPORT               	   0x20

//Language voice prompt support
#define NOT_SUPPORT			               0x00
#define ENGLISH           		               0x01
#define CHINESE                            	  0x02
#define RUSSIAN                  			  0x04
#define INDONESIAN          			  0x08
#define TURKISH               	               0x10
#define GEMAN            	             	  0x20
#define ITALIAN                             	  0x40
#define JAPANESE                                  0x80

#define FRENCH                        		  0x01
#define THAI                                  	  0x02
#define PROTUGUESE                    		  0x04
#define SPANISH                            	  0x08
#define VIETNAMESE         	               0x10
#define ARABIC          	             	  0x20
#define KOREAN                             	  0x40
#define MALAY                                       0x80

#define HINDI                                        0x01
#define URDU                                         0x02
#define BENGALI                 		         0x04

//Language voice prompt select
#define TONE_SELECT			                            0x00
#define ENGLISH_SELECT           		               0x01
#define CHINESE_SELECT                            	  0x02
#define RUSSIAN_SELECT                  			  0x03
#define INDONESIAN_SELECT          			  0x04
#define TURKISH_SELECT               	                     0x05
#define GEMAN_SELECT            	             	  0x06
#define ITALIAN_SELECT                             	  0x07
#define JAPANESE_SELECT                                  0x08

#define FRENCH_SELECT                        		  0x09
#define THAI_SELECT                                  	  0x0a
#define PROTUGUESE_SELECT                    		  0x0b
#define SPANISH_SELECT                            	  0x0c
#define VIETNAMESE_SELECT         	                     0x0d
#define ARABIC_SELECT          	             	        0x0e
#define KOREAN_SELECT                             	  0x0f
#define MALAY_SELECT                                       0x10

#define HINDI_SELECT                                        0x11
#define URDU_SELECT                                         0x12
#define BENGALI_SELECT                 		        0x13

//Voice Assistant support list
#define PUSH_AND_TALK_SUPPORT 	  0x00
#define Google_SUPPORT 			 	  0x01
#define Alexa_SUPPORT 	  			  0x02
#define Tencent_SUPPORT 	               0x04

//Special function 1 support list
#define SIRIi_BISTO_SPECIAL_KEY_SUPPORT 	0x01
#define TOUCH_LOCK_SUPPORT 			 	  	0x02
#define RUNNING_LIGHT_SUPPORT 	  			0x04
#define SIDE_TONE_SUPPORT 	                0x08
#define LOW_LATENCY_SUPPORT 	            0x10
#define VIBRATION_SUPPORT 	                0x20
#define DEVICE_COLOUR_SUPPORT 	   			0x40

//Special function 2 support list
#define LDAC_CONTROL_SUPPORT 				0x01
#define TALK_MIC_LED_CONTROL_SUPPORT 		0x02

void Philips_Api_protocol_port(uint8_t port);
bool Philips_Headphone_Api_Entry(uint8_t *data, uint32_t size);
bool CheckCommandID(uint8_t *data);
bool Command_CheckSum(uint8_t *data, uint8_t size);
uint8_t Do_CheckSum(uint8_t *data, uint8_t size);
bool Philips_Functions_Call(uint8_t *data, uint8_t size);
void Philips_Send_Notify(uint8_t *data, uint32_t size);
void Get_Connect_Phone_Mac(void);
void Get_Device_Feature_Config(void);
void Get_Api_Version();
void Get_Sound_Quality(void);
void Get_Right_Ear_MAC_Address(void);
void Get_Left_Ear_MAC_Address(void);
void Notification_Sound_Quality_Change(void);
void Get_BLE_MAC_Address(void);
void Get_Sales_Region(void);
void Get_Chipset_Vendor(void);
void Get_Chipset_Solution(void);
void Get_Chipset_Version(void);
void Get_Device_Type(void);
void Get_FOTA_Finish_Flag(void);
void  Set_FOTA_Finish_Flag(uint8_t *valuePtr);
void Get_PCBA_Version(void);
void  Get_Ambient_Sound_Item_Status(void);
void  Set_Ambient_Sound_Item_Value(uint8_t *valuePtr);
void Get_Ambient_Range_Support(void);
void Get_Ambient_Range_Total(void);
void Get_Ambient_Focus_On_Voice_Support(void);
void Get_Adaptive_Stand_D4_Value(void);
void Get_Adaptive_Walking_D4_Value(void);
void Get_Adaptive_Running_D4_Value(void);
void Get_Adaptive_Traffic_D4_Value(void);
void Get_Ambient_D4_Value(void);
void Get_Adaptive_Stand_Focus_On_Voice_D4_Value(void);
void Get_Adaptive_Walking_Focus_On_Voice_D4_Value(void);
void Get_Adaptive_Running_Focus_On_Voice_D4_Value(void);
void Get_Adaptive_Traffic_Focus_On_Voice_D4_Value(void);
void Get_Adaptive_Ambient_Focus_On_Voice_D4_Value(void);
void Get_Adaptive_Stand_Current_Value(void);
void Set_Adaptive_Stand_Current_Value(uint8_t set_stand_current_value[1]);
void Get_Adaptive_Stand_Focus_On_Voice_Value(void);
void Set_Adaptive_Stand_Focus_On_Voice_Value(uint8_t set_adaptive_standfocuson_value[1]);
void Get_Adaptive_Walking_Current_Value(void);
void Set_Adaptive_Walking_Current_Value(uint8_t set_walking_current_value[1]);
void Get_Adaptive_Walking_Focus_On_Voice_Value(void);
void Set_Adaptive_Walking_Focus_On_Voice_Value(uint8_t set_adaptive_walkingfocuson_value[1]);
void Get_Adaptive_Running_Current_Value(void);
void Set_Adaptive_Running_Current_Value(uint8_t set_running_current_value[1]);
void Get_Adaptive_Running_Focus_On_Voice_Value(void);
void Set_Adaptive_Running_Focus_On_Voice_Value(uint8_t set_adaptive_runningfocuson_value[1]);
void Get_Adaptive_Traffic_Current_Value(void);
void Set_Adaptive_Traffic_Current_Value(uint8_t set_traffic_current_value[1]);
void Get_Adaptive_Traffic_Focus_On_Voice_Value(void);
void Set_Adaptive_Traffic_Focus_On_Voice_Value(uint8_t set_adaptive_trafficfocuson_value[1]);
void Get_Ambient_Current_Value(void);
void Set_Ambient_Current_Value(uint8_t set_ambient_current_value[1]);
void Get_Ambient_Focus_On_Voice_Value(void);
void Set_Ambient_Focus_On_Voice_Value(uint8_t set_ambient_focuson_value[1]);
void  Get_HW_Nosie_Cancelling_Status(void );
void Set_HW_ANC_Value(uint8_t set_hw_anc_value[1]);
void  Get_HW_Ambient_value(void);
void  Set_HW_Ambient_value(uint8_t set_hw_ambient_value[2] );
void  Get_HW_Focus_On_Voice_Status(void );
void  Set_HW_Focus_On_Voice_Status(uint8_t set_hw_focus_on_voice_status[2] );
void Notification_Nosie_Cancelling_Change(void);
void Get_Equalizer_Support_List(void);
void Get_Equalizer_Status(void);
void Set_Equalizer_Status_Value(uint8_t set_equalizer_status_value[1]);
void Get_Equalizer_Status_D4_Value(void);
void Get_Last_EQ_Status(void);
void Notification_EQ_Change();
void Get_Customization_Eq_Support_List(void);
void Get_Customization_Eq_Brand_Current_Value(void);
void Set_Customization_Eq_Brand_Current_Value(uint8_t set_customization_eq_value[6]);
void Get_Customization_Eq_Brand_Range_Value(void);
void Get_Customization_Eq_With_Lib(void);
void Get_Nowplaying_Playback_Status(void);
void Set_Nowplaying_Playback_Status_Value(uint8_t set_nowplaying_palyback_status_value[1]);
void Set_Nowplaying_Playback_Next(void);
void Set_Nowplaying_Playback_Previous(void);
void Notification_Media_Change();
void Notification_Media_Title_Artist_Album(uint8_t type,char* media_buffer, uint8_t valueLen);
void Get_Media_Function_Support(void);
void Get_Battery_Charge_Status(void);
void Get_Right_Battery_Level_Value(void);
void Get_Left_Battery_Level_Value(void);
void Get_ChargeBox_Battery_Level_Value(void);
void Notification_Battery_Level_Change();
void Notification_Battery_Charge_Change();
void Get_Sleep_Mode_Timer(void);
void Set_Sleep_Mode_Timer(uint8_t set_sleep_timer_value[1]);
void Get_Smart_ChargeBox_support(void);
void Get_Auto_Power_Off_Support(void);
void Get_Auto_Power_Off_Timer(void);
void Set_Auto_Power_Off_Timer(uint8_t auto_power_off_timer_value[1]);
void Get_Voice_Assistant_Support_List(void);
void Get_Voice_Wakeup_Support_List(void);
void Get_Voice_Assistant_Status(void);
void Set_Voice_Assistant_Status(uint8_t set_voice_assistant_value[1]);
void Get_Push_And_Talk_Status(void);
void Set_Push_And_Talk_Status(uint8_t set_push_and_talk_value[1]);
void Get_Voice_Wakeup_Status(void);
void Get_Touch_Status(void);
void Set_Touch_Status(uint8_t set_touch_status_value[1]);
void Set_P_Sensor_Calibration(uint8_t set_p_sensor_calibration_value[1]);
void Set_P_Sensor_Reset(uint8_t set_p_sensor_reset_value[1]);
void Get_Special_Function1_Support_List(void);
void Notification_Touch_Key_OnOff_Change(void);
void Notification_TalkMicLed_Change(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __PHILIPS_BLE_API_H__