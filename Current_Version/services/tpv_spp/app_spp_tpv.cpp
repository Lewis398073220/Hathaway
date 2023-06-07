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
#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_chipid.h"
#include "analog.h"
#include "app_audio.h"
#include "app_status_ind.h"
#include "app_bt_stream.h"
#include "nvrecord.h"
#include "nvrecord_env.h"

#include "bluetooth.h"
#include "cqueue.h"
#include "resources.h"
#include "app_spp_tpv.h"
#include "app_spp.h"
#include "plat_types.h"
#include "spp_api.h"
#include "sdp_api.h"

#define TPV_CONTROL_DEBUG

#ifdef TPV_CONTROL_DEBUG
#define TPV_LOG_DBG(num,str,...)   TRACE(num,str, ##__VA_ARGS__)             // DEBUG OUTPUT
#define TPV_LOG_DUMP		       DUMP8
#else
#define TPV_LOG_DBG(str,...)
#define TPV_LOG_DUMP
#endif


#if 0
static void tota_spp_read_thread(const void *arg);
osThreadDef(tota_spp_read_thread, osPriorityAboveNormal, 1, 3072, "tota_spp_read");
osThreadId  tota_spp_read_thread_id = NULL;
#endif

static bool isTpvSppConnected = false;
static struct spp_device  *tpv_spp_dev = NULL;
static struct spp_service  *tpvSppService = NULL;
osMutexDef(tpv_spp_mutex);

static btif_sdp_record_t *tpv_sdp_record = NULL;

static app_spp_tpv_tx_done_t app_spp_tpv_tx_done_func = NULL;

static uint8_t tpvSppTxBuf[TPV_SPP_TX_BUF_SIZE];

static uint32_t occupiedTpvSppTxBufSize;
static uint32_t offsetToFillTpvSppTxData;
static uint8_t* ptrTpvSppTxBuf;

uint16_t app_spp_tpv_tx_buf_size(void)
{
    return TPV_SPP_TX_BUF_SIZE;
}

void app_spp_tpv_init_tx_buf(uint8_t* ptr)
{
    ptrTpvSppTxBuf = ptr;
    occupiedTpvSppTxBufSize = 0;
    offsetToFillTpvSppTxData = 0;
}

static void app_spp_tpv_free_tx_buf(uint8_t* ptrData, uint32_t dataLen)
{
    if (occupiedTpvSppTxBufSize > 0)
    {
        occupiedTpvSppTxBufSize -= dataLen;
    }
    TPV_LOG_DBG(1,"occupiedTpvSppTxBufSize %d", occupiedTpvSppTxBufSize);
}

uint8_t* app_spp_tpv_fill_data_into_tx_buf(uint8_t* ptrData, uint32_t dataLen)
{
    ASSERT((occupiedTpvSppTxBufSize + dataLen) < TPV_SPP_TX_BUF_SIZE, 
        "Pending SPP tx data has exceeded the tx buffer size !");
        
    if ((offsetToFillTpvSppTxData + dataLen) > TPV_SPP_TX_BUF_SIZE)
    {
        offsetToFillTpvSppTxData = 0;
    }

    uint8_t* filledPtr = ptrTpvSppTxBuf + offsetToFillTpvSppTxData;
    memcpy(filledPtr, ptrData, dataLen);

    offsetToFillTpvSppTxData += dataLen;

    occupiedTpvSppTxBufSize += dataLen;

    TPV_LOG_DBG(3,"dataLen %d offsetToFillTpvSppTxData %d occupiedTpvSppTxBufSize %d",
        dataLen, offsetToFillTpvSppTxData, occupiedTpvSppTxBufSize);
    
    return filledPtr;
}

/****************************************************************************
 * TPV SPP SDP Entries
 ****************************************************************************/

/*---------------------------------------------------------------------------
 *
 * ServiceClassIDList 
 */
#if 0 
static const U8 TpvSppClassId[] = {
    SDP_ATTRIB_HEADER_8BIT(3),        /* Data Element Sequence, 6 bytes */ 
    SDP_UUID_16BIT(SC_SERIAL_PORT),     /* Hands-Free UUID in Big Endian */
};
#else
static const uint8_t TPV_SPP_UUID_128[16]={0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };

static const U8 TpvSppClassId[] = {
  	SDP_ATTRIB_HEADER_8BIT(17),        /* Data Element Sequence, 17 bytes */
  	SDP_UUID_128BIT(TPV_SPP_UUID_128),  /* 128 bit UUID in Big Endian */
};
#endif


static const U8 TpvSppProtoDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(12),  /* Data element sequence, 12 bytes */ 

    /* Each element of the list is a Protocol descriptor which is a 
     * data element sequence. The first element is L2CAP which only 
     * has a UUID element.  
     */ 
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for L2CAP, 3 
                                  * bytes 
                                  */ 

    SDP_UUID_16BIT(PROT_L2CAP),  /* Uuid16 L2CAP */ 

    /* Next protocol descriptor in the list is RFCOMM. It contains two 
     * elements which are the UUID and the channel. Ultimately this 
     * channel will need to filled in with value returned by RFCOMM.  
     */ 

    /* Data element sequence for RFCOMM, 5 bytes */
    SDP_ATTRIB_HEADER_8BIT(5),

    SDP_UUID_16BIT(PROT_RFCOMM), /* Uuid16 RFCOMM */

    /* Uint8 RFCOMM channel number - value can vary */
    SDP_UINT_8BIT(RFCOMM_CHANNEL_TPV)
};

/*
 * BluetoothProfileDescriptorList
 */
static const U8 TpvSppProfileDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(8),        /* Data element sequence, 8 bytes */

    /* Data element sequence for ProfileDescriptor, 6 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),

    SDP_UUID_16BIT(SC_SERIAL_PORT),   /* Uuid16 SPP */
    SDP_UINT_16BIT(0x0102)            /* As per errata 2239 */ 
};

/*
 * * OPTIONAL *  ServiceName
 */
static const U8 TpvSppServiceName1[] = {
    SDP_TEXT_8BIT(5),          /* Null terminated text string */ 
    'T', 'P', 'V', '1', '\0'
};

static const U8 TpvSppServiceName2[] = {
    SDP_TEXT_8BIT(5),          /* Null terminated text string */ 
    'T', 'P', 'V', '2', '\0'
};

/* SPP attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * SPP SDP record.
 */
//static const SdpAttribute TotaSppSdpAttributes1[] = {
static sdp_attribute_t TpvSppSdpAttributes1[] = {

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, TpvSppClassId), 

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, TpvSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, TpvSppProfileDescList),

    /* SPP service name*/
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), TpvSppServiceName1),
};

void tpv_bes_handle_received_data(uint8_t *Buf, uint16_t dataLenth)
{
    uint8_t typeCode = Buf[0];
    TPV_LOG_DBG(3,"[%s],case = 0x%x",__func__, typeCode);
}


int tpv_spp_handle_data_event_func(void *pDev, uint8_t process, uint8_t *pData, uint16_t dataLen)
{
    TPV_LOG_DBG(2,"[%s]data receive length = %d", __func__, dataLen);
    TPV_LOG_DUMP("[0x%x]", pData, dataLen);

    // the first two bytes of the data packet is the fixed value 0xFFFF
	tpv_bes_handle_received_data(pData, dataLen);

    return 0;
}

#if 0
static void app_spp_tota_create_read_thread(void)
{
    TOTA_LOG_DBG(2,"%s %d\n", __func__, __LINE__);
    tota_spp_read_thread_id = osThreadCreate(osThread(tota_spp_read_thread), NULL);
}

static void app_spp_tota_close_read_thread(void)
{
    TOTA_LOG_DBG(2,"%s %d\n", __func__, __LINE__);
    if(tota_spp_read_thread_id)
    {
        osThreadTerminate(tota_spp_read_thread_id);
        tota_spp_read_thread_id = NULL;
    }
}
#endif

static void spp_tpv_callback(struct spp_device *locDev, struct spp_callback_parms *Info)
{
    if (BTIF_SPP_EVENT_REMDEV_CONNECTED == Info->event)
    {
        TPV_LOG_DBG(1,"spp_tpv_callback::BTIF_SPP_EVENT_REMDEV_CONNECTED %d\n", Info->event);
        isTpvSppConnected = true;
        //app_spp_tota_create_read_thread();

    }
    else if (BTIF_SPP_EVENT_REMDEV_DISCONNECTED == Info->event)
    {
        TPV_LOG_DBG(1,"spp_tpv_callback::BTIF_SPP_EVENT_REMDEV_DISCONNECTED %d\n", Info->event);
        isTpvSppConnected = false;
        //app_spp_tota_close_read_thread();

        app_spp_tpv_tx_done_func = NULL;
    }
    else if (BTIF_SPP_EVENT_DATA_SENT == Info->event)
    {
        //app_spp_tota_free_tx_buf(Info->tx_buf, Info->tx_data_len);
        struct spp_tx_done *pTxDone = (struct spp_tx_done *)(Info->p.other);
        app_spp_tpv_free_tx_buf(pTxDone->tx_buf, pTxDone->tx_data_length);
        if (app_spp_tpv_tx_done_func)
        {
            app_spp_tpv_tx_done_func();
        }
    }
    else
    {
        TPV_LOG_DBG(1,"spp_tpv_callback::unknown event %d\n", Info->event);
    }
}

static void app_spp_tpv_send_data(uint8_t* ptrData, uint16_t length)
{
    if (!isTpvSppConnected)
    {
        return;
    }

    btif_spp_write(tpv_spp_dev, (char *)ptrData, &length);
}

void app_tpv_send_cmd_via_spp(uint8_t* ptrData, uint32_t length)
{
    uint8_t* ptrBuf = app_spp_tpv_fill_data_into_tx_buf(ptrData, length);
    app_spp_tpv_send_data(ptrBuf, (uint16_t)length);
}

void app_tpv_send_data_via_spp(uint8_t* ptrData, uint32_t length)
{
    TPV_LOG_DBG(2,"[%s]tpv send data length = %d",__func__,length);
    uint8_t* ptrBuf = app_spp_tpv_fill_data_into_tx_buf(ptrData, length);
    app_spp_tpv_send_data(ptrBuf, (uint16_t)length);
}


#if (SPP_SHARE_BUF==0)
static uint8_t tpv_spp_rx_buf[SPP_RECV_BUFFER_SIZE];
#endif

#ifdef __BT_ONE_BRING_TWO__
static bool app_spp_tpv_is_already_connected(int8_t server_channel)
{
	TRACE(0,"%s server_channel %d,isTotaSppConnected %d",__func__,server_channel,isTpvSppConnected);
    return ((server_channel == RFCOMM_CHANNEL_TPV) && isTpvSppConnected);
}
#endif

void app_spp_tpv_init(void)
{
    uint8_t *rx_buf;
    osMutexId mid;
    btif_sdp_record_param_t param;

#ifdef __BT_ONE_BRING_TWO__
	btif_spp_server_register_already_connected_callback(app_spp_tpv_is_already_connected);
#endif
	
    if(tpv_spp_dev == NULL)
        tpv_spp_dev = btif_create_spp_device();

    rx_buf = tpv_spp_dev->rx_buffer = &tpv_spp_rx_buf[0];
	btif_spp_init_rx_buf(tpv_spp_dev, rx_buf, SPP_RECV_BUFFER_SIZE);
    app_spp_tpv_init_tx_buf(tpvSppTxBuf);
    
    mid = osMutexCreate(osMutex(tpv_spp_mutex));
    if (!mid) {
        ASSERT(0, "cannot create mutex");
    }

    if (tpv_sdp_record == NULL)
        tpv_sdp_record = btif_sdp_create_record();

    param.attrs = &TpvSppSdpAttributes1[0],
    param.attr_count = ARRAY_SIZE(TpvSppSdpAttributes1);
    param.COD = BTIF_COD_MAJOR_PERIPHERAL;
    btif_sdp_record_setup(tpv_sdp_record, &param);

    if(tpvSppService == NULL)
        tpvSppService = btif_create_spp_service();

    tpvSppService->rf_service.serviceId = RFCOMM_CHANNEL_TPV;
    tpvSppService->numPorts = 0;
    btif_spp_service_setup(tpv_spp_dev, tpvSppService, tpv_sdp_record);

    tpv_spp_dev->portType = BTIF_SPP_SERVER_PORT;
    tpv_spp_dev->app_id = BTIF_APP_SPP_SERVER_TPV_RFCOMM_ID;
    tpv_spp_dev->spp_handle_data_event_func = tpv_spp_handle_data_event_func;
    btif_spp_init_device(tpv_spp_dev, 5, mid);

    btif_spp_open(tpv_spp_dev, NULL,  spp_tpv_callback);
}

