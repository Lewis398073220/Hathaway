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
#include "hal_i2c.h"
#include "hal_uart.h"
#include "bt_drv.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "cmsis.h"
#include "hal_cmu.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "tgt_hardware.h"
#include "bt_drv_internal.h"
#include "bt_drv_1402_internal.h"
#include "hal_chipid.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "iqcorrect.h"

#define AUTO_CAL                                0

#ifndef BT_RF_MAX_XTAL_TUNE_PPB
// Default 10 ppm/bit or 10000 ppb/bit
#define BT_RF_MAX_XTAL_TUNE_PPB                 10000
#endif

#ifndef BT_RF_XTAL_TUNE_FACTOR
// Default 0.2 ppm/bit or 200 ppb/bit
#define BT_RF_XTAL_TUNE_FACTOR                  200
#endif

#undef DEFAULT_XTAL_FCAP
#define DEFAULT_XTAL_FCAP                       0x8100

#define XTAL_FCAP_NORMAL_SHIFT                  0
#define XTAL_FCAP_NORMAL_MASK                   (0x1FF << XTAL_FCAP_NORMAL_SHIFT)
#define XTAL_FCAP_NORMAL(n)                     BITFIELD_VAL(XTAL_FCAP_NORMAL, n)

#define BT_XTAL_CMOM_DR                         (1 << 13)

#define RF_REG_XTAL_FCAP                        0x59
#define RF_REG_XTAL_CMOM_DR                     0x58

/* ljh add for sync modify rf RF_REG_XTAL_FCAP value*/
#define SPI_TRIG_RX_NEG_TIMEOUT                 MS_TO_TICKS(3)

#define SPI_TRIG_NEG_TIMEOUT                    SPI_TRIG_RX_NEG_TIMEOUT


enum BIT_OFFSET_CMD_TYPE_T
{
    BIT_OFFSET_CMD_STOP = 0,
    BIT_OFFSET_CMD_START,
    BIT_OFFSET_CMD_ACK,
};

static uint16_t xtal_fcap = DEFAULT_XTAL_FCAP;
static uint16_t init_xtal_fcap = DEFAULT_XTAL_FCAP;

static uint16_t read_value_imf_reg01;
static uint8_t rf_vco_div = BT_RF_VCO_DIV_1P5;

//rampup start     1--->2->3 rampup ok
//rampdown start 3-->2-->1-->0  rampdown ok


//3a06=3b3f
//for pa@1.8v
struct bt_drv_tx_table_t
{
    uint16_t tbl[16][3];
};

const struct bt_drv_tx_table_t rf_tx_pwr_tbl[] =
{
    {{
        }
    },
    {{
        }
    },
    {{
        }
    },
    {{
        }
    },
    {{
        }
    }
};

struct RF_SYS_INIT_ITEM
{
    uint16_t reg;
    uint16_t set;
    uint16_t mask;
    uint16_t delay;
};

//align with 1400
const uint16_t rf_init_tbl_1[][3] =
{
    {0x01,0xfdbb,0},
    {0x02,0x213c,0}, //reg_lna_rmx_bias=4 modify by langwei optimize BLE sensitivity
    {0x03,0x2a01,0},
    {0x04,0x1238,0},
    {0x05,0xa275,0}, //for optimize 1400 IQ mismatch by xulicheng
    {0x06,0x59b3,0}, //improve 1402 rx sensitivity according to jiangpeng
    {0x07,0x4124,0},
    {0x08,0x00df,0}, // improve Tx out-of-band spurious
#if VCO_DIV2
    {0x09,0x5a24,0}, // 0x3e62
#else
    {0x09,0x1a24,0}, // 0x3e62
#endif
    {0x0a,0x3fc0,0},
    {0x0b,0x7ff8,0}, // tx inband flatness
    {0x0c,0x22fe,0}, // vco current
                     // 22fe by walker 2020.5.8 14:30 mail
    {0x0d,0x0724,0},
    {0x0e,0x38bf,0}, //jiangpeng, 0x38bb -> 0x38bf 3M_DEVM fail
#if VCO_DIV2
    {0x11,0x0312,0},
#else
    {0x11,0x0302,0}, // 0402-->0x0382 vco current improve rx
#endif
    {0x12,0x16ff,0},
    {0x14,0x06c8,0},
    {0x15,0x300c,0},
    {0x1b,0x1f0a,0}, // 0f0a by walker 2020.5.8 14:30 mail
                     // 0f0a-->1f0a by walker 2020.05.0915:09 mail
    {0x1c,0x2d10,0}, // 0x3010->0x2d10 by luobin
    {0x1d,0x2d10,0}, // 0x3010->0x2d10 by luobin
    {0x21,0x1c0b,0}, //0c0b by walker 2020.5.8 14:30 mail
                     //0c0b-->1c0b by walker 2020.05.0915:09 mail
    {0x2e,0x00ea,0}, // max tx flt current
    {0x2c,0x0210,0},
#if VCO_DIV2
    {0x2f,0x13ff,0},
#else
    {0x2f,0x03ff,0},
#endif
    {0x36,0x008c,0},
    {0x37,0x001f,0},
    {0x38,0x0dff,0},
    {0x41,0x50c7,0},
    {0x42,0x6c09,0},
    {0x43,0xdc73,0},
    {0x46,0x0974,0},
    {0x66,0xffff,0},
    {0x88,0xba00,0},
    {0x89,0x0600,0},
    {0xaa,0xfde0,0},
    {0xc0,0x0063,0},
    {0xc2,0x0039,0},
    {0xc3,0x001c,0},
    {0xc5,0x04bc,0},
    {0xcc,0x001f,0},
    {0xd3,0x0202,0},
    {0xd5,0x01c7,0},
    {0xd6,0x0205,0},
    {0xd7,0x0400,0}, //1402 VCO/1.5 only
    {0xd8,0x0240,0},
};

///tws triggle
void btdrv_rf_trig_patch_enable(bool enable)
{
}

void btdrv_tws_trig_role(uint8_t role)
{
}


uint32_t btdrv_rf_get_max_xtal_tune_ppb(void)
{
    return BT_RF_MAX_XTAL_TUNE_PPB;
}


uint32_t btdrv_rf_get_xtal_tune_factor(void)
{
    return BT_RF_XTAL_TUNE_FACTOR;
}

void btdrv_rf_init_xtal_fcap(uint32_t fcap)
{
    xtal_fcap = SET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL, fcap);
    btdrv_write_rf_reg(RF_REG_XTAL_FCAP, xtal_fcap);
    init_xtal_fcap = xtal_fcap;
}

uint32_t btdrv_rf_get_init_xtal_fcap(void)
{
    return GET_BITFIELD(init_xtal_fcap, XTAL_FCAP_NORMAL);
}

uint32_t btdrv_rf_get_xtal_fcap(void)
{
    return GET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL);
}

void btdrv_rf_set_xtal_fcap(uint32_t fcap, uint8_t is_direct)
{
    xtal_fcap = SET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL, fcap);
    btdrv_write_rf_reg(RF_REG_XTAL_FCAP, xtal_fcap);
}
int btdrv_rf_xtal_fcap_busy(uint8_t is_direct)
{
    return 0;
}

void btdrv_rf_bit_offset_track_enable(bool enable)
{
}

void btdrv_rf_bit_offset_track_force_disable(void)
{
}


uint32_t btdrv_rf_bit_offset_get(void)
{
    return 0;
}

void btdrv_xtal_rf_init(void)
{
}

void btdrv_rf_tx_init(int idx)
{
}


uint16_t btdrv_rf_bitoffset_get(uint8_t conidx)
{
    uint16_t localVal = BTDIGITAL_BT_EM(EM_BT_BITOFF_ADDR + conidx * BT_EM_SIZE);
    return (localVal >> 0);
}

void btdrv_rf_log_delay_cal(void)
{
    unsigned short read_value;
    unsigned short read_value_tmp;
    unsigned short read_value_tmp_reg11;
    unsigned short read_value_tmp_reg21;
    unsigned short write_value_reg43 = 0;
    unsigned short tmp_value;

    //unsigned short read_value_tmp3;
    uint32_t val_record;

    BT_DRV_TRACE(0,"btdrv_rf_log_delay_cal\n");
    btdrv_read_rf_reg(0x11,&read_value_tmp_reg11);
    btdrv_write_rf_reg(0x11, (0x0382));

    btdrv_read_rf_reg(0x21,&read_value_tmp_reg21);
    btdrv_write_rf_reg(0x21, (0x1c0b));
    ///improve CI perfomance

    btdrv_write_rf_reg(0x43, 0x0000);

    //btdrv_read_rf_reg(0xd7, &read_value_tmp2);
    //BT_DRV_TRACE(1,"0xd7 read=%x\n",read_value_tmp2);
    //btdrv_write_rf_reg(0xd7, 0x0f00);

    btdrv_read_rf_reg(0x42, &read_value_tmp);
    BT_DRV_TRACE(1,"0x42 read=%x\n",read_value_tmp);
    read_value_tmp = ((read_value_tmp & 0xfe1f) | 0x01e0);
    BT_DRV_TRACE(1,"0x42 write=%x\n",read_value_tmp);
    btdrv_write_rf_reg(0x42, read_value_tmp);

    //btdrv_read_rf_reg(0xa7, &read_value_tmp3);
    //BT_DRV_TRACE(1,"0xa7 read=%x\n",read_value_tmp3);
    //btdrv_write_rf_reg(0xa7, 0x0027);

    //val_record = *(volatile uint32_t*)(0xd0340020);
    //BT_DRV_TRACE(1,"0xd0340020 = %x\n",val_record);
    //*(volatile uint32_t*)(0xd0340020) = 0x030e01c1;

    val_record = *(volatile uint32_t*)(0xd02201e4);
    *(volatile uint32_t*)(0xd02201e4) = 0x000a00a7;

    btdrv_delay(1);

    btdrv_read_rf_reg(0xb4, &read_value);
    BT_DRV_TRACE(1,"0xb4 read=%x\n",read_value);
    //write_value = (1<<14) | (1<<15) | (1<<1) | ((read_value & 0x1000)>>12) | ((read_value & 0x0fc0)<<2) | ((read_value & 0x003f)<<2);
    //write_value = (1<<14) | (1<<15) | (1<<1) | ((read_value & 0x1000)>>12) | ((((read_value & 0x0fc0)>>6) + 2)<<8) | (((read_value & 0x003f) + 2)<<2);////update by xulicheng 2019.10.11

    write_value_reg43 |= (read_value & 1<<12)>>12;

    tmp_value = ((read_value & (0x3f<<6)) >> 6) + 2;
    BT_DRV_TRACE(1,"0xb4[11:6] %04x\n",tmp_value);
    tmp_value = tmp_value > 0x3f ? 0x3f : tmp_value;
    BT_DRV_TRACE(1,"0xb4[11:6] reorder %04x\n",tmp_value);
    write_value_reg43 |= tmp_value << 8;

    tmp_value = (read_value & 0x3f) + 2;
    BT_DRV_TRACE(1,"0xb4[5:0] %04x\n",tmp_value);
    tmp_value = tmp_value > 0x3f ? 0x3f : tmp_value;
    BT_DRV_TRACE(1,"0xb4[5:0] reorder %04x\n",tmp_value);
    write_value_reg43 |= tmp_value <<2;

    write_value_reg43 |= (1<<14) | (1<<15) | (1<<1);
    BT_DRV_TRACE(1,"0x43 write=%x\n",write_value_reg43);
    btdrv_write_rf_reg(0x43, write_value_reg43);

    btdrv_delay(1);

    btdrv_write_rf_reg(0x11, (read_value_tmp_reg11));
    btdrv_write_rf_reg(0x21, (read_value_tmp_reg21));

    BT_DRV_TRACE(1,"0x42 write=%x\n",(read_value_tmp & 0xfe1f));
    btdrv_write_rf_reg(0x42, (read_value_tmp & 0xfe1f));

    //btdrv_write_rf_reg(0xd7, read_value_tmp2);

    //btdrv_write_rf_reg(0xa7, read_value_tmp3);

    //*(volatile uint32_t*)(0xd0340020) = val_record;
    *(volatile uint32_t*)(0xd02201e4) = val_record;

}

void btdrv_rf_rx_gain_adjust_regdump(void)
{
}

void btdrv_rf_rx_gain_adjust_default(void)
{
}

void btdrv_rf_rx_gain_adjust_lowgain(void)
{
}

void btdrv_rf_rx_gain_adjust_req(uint32_t user, bool lowgain)
{
}

bool btdrv_rf_rx_gain_adjust_getstatus(void)
{
    return false;
}

void btdrv_rf_rx_gain_adjust_init(void)
{
}

bool btdrv_rf_blerx_gain_adjust_getstatus(void)
{
    return false;
}

void btdrv_rf_blerx_gain_adjust_default(void)
{
}

void btdrv_rf_blerx_gain_adjust_lowgain(void)
{
}

uint8_t btdrv_rf_init(void)
{
    const uint16_t (*p_rf_tx_init_tbl)[3];
    uint32_t tbl_size;
    uint8_t i = 0;
    uint16_t val = 0;

    p_rf_tx_init_tbl = &rf_init_tbl_1[0];
    tbl_size = sizeof(rf_init_tbl_1)/sizeof(rf_init_tbl_1[0]);

    for(; i<tbl_size; i++)
    {
        btdrv_write_rf_reg(p_rf_tx_init_tbl[i][0],p_rf_tx_init_tbl[i][1]);
        if(p_rf_tx_init_tbl[i][2] !=0)
        {
            btdrv_delay(p_rf_tx_init_tbl[i][2]);//delay
        }
        //btdrv_read_rf_reg(p_rf_tx_init_tbl[i][0],&rf_value);
        //BT_DRV_TRACE(2,"reg=%x,v=%x",p_rf_tx_init_tbl[i][0],rf_value);
    }

    ///improve CI perfomance
    btdrv_read_rf_reg(0x01,&read_value_imf_reg01);

    read_value_imf_reg01 |= 1<<10;
    btdrv_write_rf_reg(0x01, read_value_imf_reg01);

    //Make sure that before rf_log_delay_cal, reg_log_cal_en is in the open state, to avoid reboot, this value is 0
    btdrv_read_rf_reg(0x44,&val);
    val |= (1<<10);
    btdrv_write_rf_reg(0x44, val);

    btdrv_rf_log_delay_cal();
    btdrv_spi_trig_init();
    bt_drv_tx_pwr_init();
#ifdef __HW_AGC__
    read_value_imf_reg01 |= 1<<10;
#else
    read_value_imf_reg01 &= ~(1<<10);
#endif
    btdrv_write_rf_reg(0x01, read_value_imf_reg01);

    return 1;
}

void btdrv_rf_txcal(void)
{
    //unsigned short rfreg_18 = 0;
    unsigned short rfreg_28 = 0;
    unsigned short rfreg_04 = 0;
    unsigned short rfreg_d3 = 0;
    unsigned short rfreg_d4 = 0;
    unsigned short rfreg_27 = 0;
    unsigned short rfreg_05 = 0;
    unsigned short rfreg_2a = 0;
    unsigned short rfreg_42 = 0;
    unsigned short rfreg_06 = 0;
    unsigned short rfreg_aa = 0;
    unsigned short rfreg_03 = 0;

    uint32_t dig_d0350208 = 0;
    uint32_t dig_d0350228 = 0;
    uint32_t dig_d0330038 = 0;
    uint32_t dig_d0350364 = 0;
    uint32_t dig_d0350360 = 0;
    uint32_t dig_d035037c = 0;
    uint32_t dig_d02201e4 = 0;
    uint32_t dig_d0350218 = 0;
    uint32_t dig_d0350240 = 0;
    uint32_t dig_d0330034 = 0;

    // Backup registers
    //btdrv_read_rf_reg(0x18, &rfreg_18);
    btdrv_read_rf_reg(0x28, &rfreg_28);
    btdrv_read_rf_reg(0x04, &rfreg_04);
    btdrv_read_rf_reg(0xd3, &rfreg_d3);
    btdrv_read_rf_reg(0xd4, &rfreg_d4);
    btdrv_read_rf_reg(0x27, &rfreg_27);
    btdrv_read_rf_reg(0x05, &rfreg_05);
    btdrv_read_rf_reg(0x2a, &rfreg_2a);
    btdrv_read_rf_reg(0x42, &rfreg_42);
    btdrv_read_rf_reg(0x06, &rfreg_06);
    btdrv_read_rf_reg(0xaa, &rfreg_aa);
    btdrv_read_rf_reg(0x03, &rfreg_03);

    dig_d0350208 = BTDIGITAL_REG(0xd0350208);
    dig_d0350228 = BTDIGITAL_REG(0xd0350228);
    dig_d0330038 = BTDIGITAL_REG(0xd0330038);
    dig_d0350364 = BTDIGITAL_REG(0xd0350364);
    dig_d0350360 = BTDIGITAL_REG(0xd0350360);
    dig_d035037c = BTDIGITAL_REG(0xd035037c);
    dig_d02201e4 = BTDIGITAL_REG(0xd02201e4);
    dig_d0350218 = BTDIGITAL_REG(0xd0350218);
    dig_d0350240 = BTDIGITAL_REG(0xd0350240);
    dig_d0330034 = BTDIGITAL_REG(0xd0330034);

    // Write registers
    //btdrv_write_rf_reg(0x18,0x29f8);
    btdrv_write_rf_reg(0x28,0x8023);
    btdrv_write_rf_reg(0x04,0x1278);
    btdrv_write_rf_reg(0xd3,0x0cc1);
    btdrv_write_rf_reg(0xd4,0x0c34);
    btdrv_write_rf_reg(0x27,0x3003);//adc en
    btdrv_write_rf_reg(0x05,0x9fc5);
    btdrv_write_rf_reg(0x2a,0x3000);
    btdrv_write_rf_reg(0x42,0xec09);
    btdrv_write_rf_reg(0x06,0x5937);
    btdrv_write_rf_reg(0xaa,0xfea0);
    btdrv_write_rf_reg(0x03,0x0b01);

    BTDIGITAL_REG(0xd0350208)=0xffffffff;//set dump clock 52M
    BTDIGITAL_REG(0xd0350228)=0xffffffff;//set dump clock 52M
    BTDIGITAL_REG(0xd0330038)=0x0030010d;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    BTDIGITAL_REG(0xd0350364)=0x002eb948;
    BTDIGITAL_REG(0xd0350360)=0x007fc240;
    BTDIGITAL_REG(0xd035037c)=0x20415;
    BTDIGITAL_REG(0xd02201e4)=0x000a002b;
    BTDIGITAL_REG(0xd0350218)=0x404;
    BTDIGITAL_REG(0xd0350240)=0x2007;
    BTDIGITAL_REG(0xd0330034)=0x6da3a3aa;
    hal_sys_timer_delay(MS_TO_TICKS(10));

    bt_iqimb_ini();
    bt_iqimb_test_ex(0);

    // Restore registers
    //btdrv_write_rf_reg(0x18,rfreg_18);
    btdrv_write_rf_reg(0x28,rfreg_28);
    btdrv_write_rf_reg(0x04,rfreg_04);
    btdrv_write_rf_reg(0xd3,rfreg_d3);
    btdrv_write_rf_reg(0xd4,rfreg_d4);
    btdrv_write_rf_reg(0x27,rfreg_27);
    btdrv_write_rf_reg(0x05,rfreg_05);
    btdrv_write_rf_reg(0x2a,rfreg_2a);
    btdrv_write_rf_reg(0x42,rfreg_42);
    btdrv_write_rf_reg(0x06,rfreg_06);
    btdrv_write_rf_reg(0xaa,rfreg_aa);
    btdrv_write_rf_reg(0x03,rfreg_03);

    BTDIGITAL_REG(0xd0350208)=dig_d0350208;
    BTDIGITAL_REG(0xd0350228)=dig_d0350228;
    BTDIGITAL_REG(0xd0330038)=dig_d0330038;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    BTDIGITAL_REG(0xd0350364)=dig_d0350364;
    BTDIGITAL_REG(0xd0350360)=dig_d0350360;
    BTDIGITAL_REG(0xd035037c)=dig_d035037c;
    BTDIGITAL_REG(0xd02201e4)=dig_d02201e4;
    BTDIGITAL_REG(0xd0350218)=dig_d0350218;
    BTDIGITAL_REG(0xd0350240)=dig_d0350240;
    BTDIGITAL_REG(0xd0330034)=dig_d0330034;
    hal_sys_timer_delay(MS_TO_TICKS(10));

    {
        unsigned short rfreg_42 = 0;
        unsigned short rfreg_a7 = 0;
        btdrv_read_rf_reg(0x42, &rfreg_42);
        btdrv_write_rf_reg(0x42, rfreg_42&(~1));   // clear bit 0
        btdrv_read_rf_reg(0xa7, &rfreg_a7);
        btdrv_write_rf_reg(0xa7, rfreg_a7|(2<<8)); // a7[10:8] tx gain index
    }
}

const uint16_t swagc_[][3] =
{
#ifdef __SWAGC_2_LEVEL__
    {0x5a,0x001C,0},  //Rx gain section_1
    {0x5b,0x001C,0},
    {0x6e,0x003f,0},  //Rx gain section_2
    {0x6f,0x0034,0},
#else
    {0x5a,0x001c,0},  //Rx gain section_1
    {0x5b,0x001d,0},
    {0x5c,0x001f,0},
    {0x5d,0x000f,0},
    {0x5e,0x0007,0},
    {0x5f,0x0003,0},
    {0x60,0x0003,0},
    {0x61,0x0003,0},

    {0x6e,0x00Bf,0},  //Rx gain section_2
    {0x6f,0x00bf,0},
    {0x70,0x00bf,0},
    {0x71,0x00bf,0},
    {0x72,0x00bf,0},
    {0x73,0x00bf,0},
    {0x74,0x00af,0},
    {0x75,0x009f,0},
#endif
};

const uint16_t hwagc_[][3] =
{
#ifdef __SWAGC_2_LEVEL__
    {0x5a,0x001c,0},  //Rx gain section_1
    {0x5b,0x000f,0},
    {0x6e,0x003f,0},  //Rx gain section_2
    {0x6f,0x003e,0},
#else
    {0x5a,0x0007,0},  //Rx gain section_1
    {0x5b,0x000f,0},
    {0x5c,0x000d,0},
    {0x5d,0x000c,0},
    {0x5e,0x001c,0},
    {0x5f,0x0000,0},
    {0x60,0x0000,0},
    {0x61,0x0000,0},

    {0x6e,0x003f,0},  //Rx gain section_2
    {0x6f,0x003f,0},
    {0x70,0x003e,0},
    {0x71,0x003d,0},
    {0x72,0x003a,0},
    {0x73,0x0030,0},
    {0x74,0x0020,0},
    {0x75,0x0010,0},
#endif
};

void btdrv_switch_agc_mode(enum BT_AGC_MODE_T mode)
{
#ifdef __HYBIRD_AGC__
    const uint16_t (*p_rf_tx_init_tbl)[3];
    uint32_t tbl_size;
    static enum BT_AGC_MODE_T mode_bak = BT_AGC_MODE_NONE;
    uint16_t i = 0;

    if(mode_bak != mode)
    {
        mode_bak = mode;
        uint32_t lock = int_lock_global();
        if(mode == BT_AGC_MODE_HW)
        {
            bt_drv_hw_sw_agc_select(BT_AGC_MODE_HW);
            p_rf_tx_init_tbl = (const uint16_t (*)[3])&hwagc_[0];
            tbl_size = sizeof(hwagc_)/sizeof(hwagc_[0]);
            read_value_imf_reg01 |= 1<<10;
            btdrv_write_rf_reg(0x01, read_value_imf_reg01);
        }
        else
        {
            bt_drv_hw_sw_agc_select(BT_AGC_MODE_SW);
            p_rf_tx_init_tbl = (const uint16_t (*)[3])&swagc_[0];
            tbl_size = sizeof(swagc_)/sizeof(swagc_[0]);
            read_value_imf_reg01 &= ~(1<<10);
            btdrv_write_rf_reg(0x01, read_value_imf_reg01);
        }
        for( i=0; i< tbl_size; i++ )
        {
            btdrv_write_rf_reg(p_rf_tx_init_tbl[i][0],p_rf_tx_init_tbl[i][1]);
        }
        int_unlock_global(lock);
    }
#endif
}

void btdrv_rf_vco_div_switch(uint8_t vco_div)
{
    unsigned short read_value_tmp_reg09;
    unsigned short read_value_tmp_reg11;
    unsigned short read_value_tmp_reg2f;

    rf_vco_div = vco_div;

    btdrv_read_rf_reg(0x09,&read_value_tmp_reg09);
    btdrv_read_rf_reg(0x11,&read_value_tmp_reg11);
    btdrv_read_rf_reg(0x2f,&read_value_tmp_reg2f);

    switch (rf_vco_div){
        case BT_RF_VCO_DIV_2:
            read_value_tmp_reg09 |= (1<<14);
            read_value_tmp_reg11 |= (1<<4);
            read_value_tmp_reg2f |= (1<<12);
            break;
        case BT_RF_VCO_DIV_1P5:
            read_value_tmp_reg09 &= ~(1<<14);
            read_value_tmp_reg11 &= ~(1<<4);
            read_value_tmp_reg2f &= ~(1<<12);
        default:
            break;
    }
    btdrv_write_rf_reg(0x09, read_value_tmp_reg09);
    btdrv_write_rf_reg(0x11, read_value_tmp_reg11);
    btdrv_write_rf_reg(0x2f, read_value_tmp_reg2f);
}

void bt_drv_tx_pwr_init(void)
{
    switch (rf_vco_div){
        case BT_RF_VCO_DIV_2:
            btdrv_write_rf_reg(0x76, 0x0004); // min tx gain  2019.02.26
            btdrv_write_rf_reg(0x77, 0x0004); // mid tx gain  2019.02.26
            btdrv_write_rf_reg(0x78, 0x0004); // max tx gain 2019.02.26
            //ble txpower need modify ble tx idx @ bt_drv_config.c
            btdrv_write_rf_reg(0x79, 0x000d);
            break;
        case BT_RF_VCO_DIV_1P5:
            btdrv_write_rf_reg(0x76, 0x0002); // min tx gain  2019.02.26
            btdrv_write_rf_reg(0x77, 0x0002); // mid tx gain  2019.02.26
            btdrv_write_rf_reg(0x78, 0x0002); // max tx gain 2019.02.26
            //ble txpower need modify ble tx idx @ bt_drv_config.c
            btdrv_write_rf_reg(0x79, 0x000d);
        default:
            break;
    }
}

void bt_drv_tx_pwr_init_for_testmode(void)
{
    switch (rf_vco_div){
        case BT_RF_VCO_DIV_2:
            btdrv_write_rf_reg(0x76, 0x000d); // min tx gain by walker 2019.02.26
            btdrv_write_rf_reg(0x77, 0x000a); // mid tx gain by walker 2019.02.26
            btdrv_write_rf_reg(0x78, 0x0007); // max tx gain by walker 2019.02.26
            //ble txpower need modify ble tx idx @ bt_drv_config.c
            btdrv_write_rf_reg(0x79, 0x000d);
            break;
        case BT_RF_VCO_DIV_1P5:
            btdrv_write_rf_reg(0x76, 0x000c); // min tx gain by walker 2019.02.26
            btdrv_write_rf_reg(0x77, 0x0009); // mid tx gain by walker 2019.02.26
            btdrv_write_rf_reg(0x78, 0x0002); // max tx gain by walker 2019.02.26
            //ble txpower need modify ble tx idx @ bt_drv_config.c
            btdrv_write_rf_reg(0x79, 0x000b);
        default:
            break;
    }
}
