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
#ifndef __APP_SPP_TPV_H__
#define __APP_SPP_TPV_H__

#define TPV_SPP_ID					 0
#define TPV_SPP_MAX_PACKET_SIZE     600
#define TPV_SPP_MAX_PACKET_NUM      5


#define TPV_SPP_TX_BUF_SIZE	    (TPV_SPP_MAX_PACKET_SIZE*TPV_SPP_MAX_PACKET_NUM)

#define APP_TOTA_DATA_CMD_TIME_OUT_IN_MS	5000

typedef void(*app_spp_tpv_tx_done_t)(void);
void app_spp_tpv_init(void);
void app_tpv_send_cmd_via_spp(uint8_t* ptrData, uint32_t length);
void app_tpv_send_data_via_spp(uint8_t* ptrData, uint32_t length);

#endif
