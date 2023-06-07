#ifdef DONGLE_SUPPORT

#include "cmsis_nvic.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "pmu.h"
#include "analog.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_chipid.h"
#include "hal_location.h"
#include "hal_cmu.h"
#include "hal_aud.h"
#include "plat_addr_map.h"
#include "tgt_hardware.h"

#include "reg_pmu_best3003.h"
#include "reg_analog_best3003.h"

#define DONGLE_TRACE    TRACE
#define DONGLE_ERROR    TRACE

typedef enum {
    DONGLE_MAIN_STATE_IDLE,
    DONGLE_MAIN_STATE_EARPHONE_CHECK,
    DONGLE_MAIN_STATE_KEY_CHECK,
    DONGLE_MAIN_STATE_ERROR,

    DONGLE_MAIN_STATE_NUM
} DONGLE_MAIN_STATE_E;

typedef enum {
    // sub states for EARPHONE_CHECKING
    DONGLE_SUB_STATE_PLUGIN_CHECKING = 0,
    DONGLE_SUB_STATE_GND_SENSING,
    DONGLE_SUB_STATE_LOAD_SENSING,

    // sub states for KEY_CHECKING
    DONGLE_SUB_STATE_KEY_DOWN_SENSING = 0,
    DONGLE_SUB_STATE_KEY_UP_SENSING,

    DONGLE_SUB_STATE_NUM
} DONGLE_SUB_STATE_E;

#define DONGLE_CHECK_FAULT_TYPE_NONE        0
#define DONGLE_CHECK_FAULT_TYPE_PLUGIN      (1 << 0)
#define DONGLE_CHECK_FAULT_TYPE_GND         (1 << 1)
#define DONGLE_CHECK_FAULT_TYPE_LOAD        (1 << 2)

#define DONGLE_CHECK_MAX_FAULT              (15)

typedef struct {
    uint8_t main_state:4;
    uint8_t earphone_check_sub_state:2;
    uint8_t key_check_sub_state:2;

    uint8_t fault_cnt:4;
    uint8_t fault_type:4;

    DONGLE_INFORM_PARAM_T dongle_info;
} DONGLE_CONTEXT_T;

static DONGLE_CONTEXT_T s_dongle_ctx;
static DONGLE_CONFIG_T s_dongle_cfg;

static uint16_t dongle_irq_status_get (void)
{
    uint16_t irq_status = 0;
    pmu_read(PMU_REG_7C, &irq_status);
    return irq_status;
}

static void dongle_irq_clear (void)
{
    pmu_write(PMU_REG_75, 0xFFFF);
    hal_sys_timer_delay_us(2);
    pmu_write(PMU_REG_75, 0);
}

static void dongle_plugin_volt_set (uint8_t high)
{
    uint16_t reg_val;

    pmu_read(PMU_REG_72, &reg_val);
    if (high) {
        reg_val |= EAR_PLUG_DETECT_INV;
    } else {
        reg_val &= ~EAR_PLUG_DETECT_INV;
    }
    pmu_write(PMU_REG_72, reg_val);
}

static void dongle_state_reset (void)
{
    s_dongle_ctx.earphone_check_sub_state = DONGLE_SUB_STATE_PLUGIN_CHECKING;
    s_dongle_ctx.key_check_sub_state = DONGLE_SUB_STATE_KEY_DOWN_SENSING;

    s_dongle_ctx.dongle_info.earphone_info.gnd_type = DONGLE_EARPHONE_GND_UNKNOWN;
    s_dongle_ctx.dongle_info.earphone_info.load_type = DONGLE_EARPHONE_LOAD_UNKNOWN;

    s_dongle_ctx.dongle_info.key_info = DONGLE_KEY_UNKNOWN;
}

static int dongle_earphone_connected_check (void)
{
    uint16_t status;
    int connected = 0;

    pmu_read(PMU_REG_78, &status);

    if (s_dongle_cfg.cfg_low_level_for_plugin != ((status&EAR_PLUG_DETECT_L)==EAR_PLUG_DETECT_L))
    {
        connected = 1;
    }

    return connected;
}

static DONGLE_MAIN_STATE_E dongle_load_sensing_start (void)
{
    uint16_t reg_val;

    if (s_dongle_cfg.cfg_load_sensing_mode == DONGLE_LOAD_SENSING_AUTO) {

        DONGLE_TRACE(0, "-> start load sensing");

        // to enable and mask the interrupt of load sensing
        pmu_read(PMU_REG_64, &reg_val);
        reg_val |= (EAR_RES_FLOW_DONE_INTR_EN | EAR_RES_FLOW_DONE_INTR_MSK);
        pmu_write(PMU_REG_64, reg_val);

        // to start load sensing
        pmu_read(PMU_REG_60, &reg_val);
        reg_val &= ~EAR_RES_ENABLE;
        pmu_write(PMU_REG_60, reg_val);

        hal_sys_timer_delay_us(10);

        reg_val |= EAR_RES_ENABLE;
        pmu_write(PMU_REG_60, reg_val);

        s_dongle_ctx.earphone_check_sub_state = DONGLE_SUB_STATE_LOAD_SENSING;
        return DONGLE_MAIN_STATE_EARPHONE_CHECK;
    }

    // to enable and mask the interrupt of load sensing
    pmu_read(PMU_REG_64, &reg_val);
    reg_val &= ~(EAR_RES_FLOW_DONE_INTR_EN | EAR_RES_FLOW_DONE_INTR_MSK);
    pmu_write(PMU_REG_64, reg_val);

    // to set earphone load as the configuration
    if (s_dongle_cfg.cfg_load_sensing_mode == DONGLE_LOAD_SENSING_16OHM_FIXED) {
        DONGLE_TRACE(0, "-> load is fixed as 16ohm");
        s_dongle_ctx.dongle_info.earphone_info.load_type = DONGLE_EARPHONE_LOAD_16_OHM;
    } else if (s_dongle_cfg.cfg_load_sensing_mode == DONGLE_LOAD_SENSING_32OHM_FIXED) {
        DONGLE_TRACE(0, "-> load is fixed as 32ohm");
        s_dongle_ctx.dongle_info.earphone_info.load_type = DONGLE_EARPHONE_LOAD_32_OHM;
    } else {
        DONGLE_TRACE(0, "-> load is fixed as 600ohm");
        s_dongle_ctx.dongle_info.earphone_info.load_type = DONGLE_EARPHONE_LOAD_600_OR_HIGHER_OHM;
    }

    // to notify the APP of the dongle information
    if (s_dongle_cfg.cb) {
        s_dongle_cfg.cb(DONGLE_NOTIFY_EVT_PLUGIN_LOAD_INFO, &s_dongle_ctx.dongle_info);
    }

    return DONGLE_MAIN_STATE_KEY_CHECK;
}

static DONGLE_MAIN_STATE_E dongle_gnd_sensing_start (void)
{
    uint16_t reg_val;

    if (s_dongle_cfg.cfg_gnd_sensing_mode == DONGLE_GND_SENSING_AUTO) {

        DONGLE_TRACE(0, "-> start GND sensing");

        // to enable VCM
        analog_read(ANA_REG_61, &reg_val);
        reg_val |= REG_CODEC_EN_VCM;
        analog_write(ANA_REG_61, reg_val);

        // to enable and mask gnd sensing interrupt
        pmu_read(PMU_REG_64, &reg_val);
        reg_val |= (EAR_DETECT_FLOW_DONE_INTR_EN | EAR_DETECT_FLOW_DONE_INTR_MSK);
        pmu_write(PMU_REG_64, reg_val);

        // to start gnd sensing
        pmu_read(PMU_REG_76, &reg_val);
        reg_val |= REG_AUTO_EAR_DETECT;
        pmu_write(PMU_REG_76, reg_val);

        s_dongle_ctx.earphone_check_sub_state = DONGLE_SUB_STATE_GND_SENSING;
        return DONGLE_MAIN_STATE_EARPHONE_CHECK;
    }

    // to disable and unmask gnd sensing interrupt
    pmu_read(PMU_REG_64, &reg_val);
    reg_val &= ~(EAR_DETECT_FLOW_DONE_INTR_EN | EAR_DETECT_FLOW_DONE_INTR_MSK);
    pmu_write(PMU_REG_64, reg_val);

    // to set gnd as the configuration
    pmu_read(PMU_REG_76, &reg_val);
    if (s_dongle_cfg.cfg_gnd_sensing_mode == DONGLE_GND_SENSING_POLE4_FIXED) {
        DONGLE_TRACE(0, "-> GND is fixed at POLE4");
        reg_val |= REG_DIRECT_OPEN_MIC_B;
        s_dongle_ctx.dongle_info.earphone_info.gnd_type = DONGLE_EARPHONE_GND_POLE4;
    } else if (s_dongle_cfg.cfg_gnd_sensing_mode == DONGLE_GND_SENSING_POLE3_FIXED) {
        DONGLE_TRACE(0, "-> GND is fixed at POLE3");
        reg_val |= REG_DIRECT_OPEN_MIC_A;
        s_dongle_ctx.dongle_info.earphone_info.gnd_type = DONGLE_EARPHONE_GND_POLE3;
    } else {
        DONGLE_TRACE(0, "-> GND is fixed at POLE3&POLE4");
        reg_val |= (REG_DIRECT_OPEN_GND_A | REG_DIRECT_OPEN_GND_B);
        s_dongle_ctx.dongle_info.earphone_info.gnd_type = DONGLE_EARPHONE_GND_POLE3_POLE4;
    }
    pmu_write(PMU_REG_76, reg_val);

    // to notify the APP of the dongle information
    if (s_dongle_cfg.cb) {
        s_dongle_cfg.cb(DONGLE_NOTIFY_EVT_PLUGIN_GND_INFO, &s_dongle_ctx.dongle_info);
    }

    return dongle_load_sensing_start();
}

static void dongle_key_check_start (void)
{
    if ((s_dongle_ctx.dongle_info.earphone_info.gnd_type == DONGLE_EARPHONE_GND_POLE3_POLE4)
            || (s_dongle_ctx.dongle_info.earphone_info.gnd_type == DONGLE_EARPHONE_GND_UNKNOWN))
    {
        return;
    }

    DONGLE_TRACE(0, "-> start key check");

    uint16_t reg_val;

    analog_aud_enable_vmic(ANA_CODEC_USER_DONGLE, AUD_VMIC_MAP_VMIC1);

    pmu_read(PMU_REG_55, &reg_val);

    pmu_read(PMU_REG_74, &reg_val);
    reg_val |= EAR_KEY_CHANGE_INTR_EN | EAR_KEY_CHANGE_INTR_MSK;
    pmu_write(PMU_REG_74, reg_val);

    pmu_read(PMU_REG_73, &reg_val);
    reg_val |= EAR_KEY_DETECT_EN;
    pmu_write(PMU_REG_73, reg_val);

}

static DONGLE_MAIN_STATE_E dongle_state_idle_process (void)
{
    uint16_t irq_status = dongle_irq_status_get();
    DONGLE_MAIN_STATE_E next_state = DONGLE_MAIN_STATE_IDLE;

    DONGLE_TRACE(0, "%s: irq[%x]", __func__, irq_status);

    if (irq_status & EAR_PLUG_CHANGE_INTR) {
        int connected = dongle_earphone_connected_check();
        if (connected) {
            if (s_dongle_cfg.cb) {
                s_dongle_cfg.cb(DONGLE_NOTIFY_EVT_WAKEUP, &s_dongle_ctx.dongle_info);
            }
            dongle_state_reset();
            next_state = DONGLE_MAIN_STATE_EARPHONE_CHECK;
        } else {
            DONGLE_TRACE(0, "-> no earphone connected");
            if (s_dongle_cfg.cb) {
                s_dongle_cfg.cb(DONGLE_NOTIFY_EVT_PLUGOUT, &s_dongle_ctx.dongle_info);
            }
        }
    }

    dongle_irq_clear();
    return next_state;
}

static DONGLE_MAIN_STATE_E dongle_earphone_plugin_check (int* err)
{
    *err = 0;
    dongle_irq_clear();

    return dongle_gnd_sensing_start();
}

static DONGLE_MAIN_STATE_E dongle_gnd_check (int* err)
{
    uint16_t reg_val;
    uint16_t ear_detect_a_data_l, ear_detect_a_data_r;
    uint16_t ear_detect_b_data_l, ear_detect_b_data_r;
    int sum = 0;
    uint32_t irq_status;
    static int gnd_check_cnt = 0;
    DONGLE_MAIN_STATE_E next_state = DONGLE_MAIN_STATE_EARPHONE_CHECK;

    *err = 0;
    pmu_read(PMU_REG_7D, &ear_detect_a_data_l);
    sum += (ear_detect_a_data_l & EAR_DETECT_A_DATA_L_MASK);
    pmu_read(PMU_REG_7E, &ear_detect_b_data_l);
    sum += (ear_detect_b_data_l & EAR_DETECT_B_DATA_L_MASK);
    pmu_read(PMU_REG_80, &ear_detect_a_data_r);
    sum += (ear_detect_a_data_r & EAR_DETECT_A_DATA_R_MASK);
    pmu_read(PMU_REG_81, &ear_detect_b_data_r);
    sum += (ear_detect_b_data_r & EAR_DETECT_B_DATA_R_MASK);

    irq_status = dongle_irq_status_get();
    dongle_irq_clear();

    DONGLE_TRACE(0, "%s: irq[%x], ear_det_data[%d, %d, %d, %d]", __func__, irq_status,
            ear_detect_a_data_l, ear_detect_b_data_l,
            ear_detect_a_data_r, ear_detect_b_data_r);

    if (sum == 0) {
        if (irq_status & EAR_DETECT_FLOW_DONE_INTR) {
            pmu_read(PMU_REG_84, &reg_val);
            DONGLE_TRACE(0, "-> ear_det_status: 01_[%x]", reg_val);
            if (reg_val & EAR_DETECT_THREE_PORT) {
                gnd_check_cnt++;
                if (gnd_check_cnt < 2) {
                    goto dongle_gnd_check_restart;
                }

                s_dongle_ctx.dongle_info.earphone_info.gnd_type = DONGLE_EARPHONE_GND_POLE3_POLE4;
                goto dongle_gnd_check_done;
            }
        }
        goto dongle_gnd_check_restart;
    }

    if (irq_status & EAR_DETECT_FLOW_DONE_INTR) {
        pmu_read(PMU_REG_84, &reg_val);
        if (reg_val & EAR_DETECT_THREE_PORT) {
            DONGLE_TRACE(0, "-> three-pole");
        } else {
            pmu_read(PMU_REG_7A, &reg_val);
            DONGLE_TRACE(0, "-> ear_det_status: 03_[%x]", reg_val);
            if (reg_val & EAR_DETECT_FOUR_PORT_A) {
                DONGLE_TRACE(0, "-> CTIA");
                s_dongle_ctx.dongle_info.earphone_info.gnd_type = DONGLE_EARPHONE_GND_POLE3;
            } else if (reg_val & EAR_DETECT_FOUR_PORT_B) {
                DONGLE_TRACE(0, "-> OMTP");
                s_dongle_ctx.dongle_info.earphone_info.gnd_type = DONGLE_EARPHONE_GND_POLE4;
            } else {
                goto dongle_gnd_check_restart;
            }
        }
    } else {
        next_state = DONGLE_MAIN_STATE_ERROR;
        *err = -1;
    }

dongle_gnd_check_done:
    if (*err == 0) {
        if (s_dongle_cfg.cb) {
            s_dongle_cfg.cb(DONGLE_NOTIFY_EVT_PLUGIN_GND_INFO, &s_dongle_ctx.dongle_info);
        }
        gnd_check_cnt = 0;

        pmu_read(PMU_REG_76, &reg_val);
        if (s_dongle_ctx.dongle_info.earphone_info.gnd_type == DONGLE_EARPHONE_GND_POLE3) {
            reg_val |= REG_DIRECT_OPEN_MIC_A;
        } else if (s_dongle_ctx.dongle_info.earphone_info.gnd_type == DONGLE_EARPHONE_GND_POLE4) {
            reg_val |= REG_DIRECT_OPEN_MIC_B;
        } else {
            reg_val |= (REG_DIRECT_OPEN_GND_A | REG_DIRECT_OPEN_GND_B);
        }
        pmu_write(PMU_REG_76, reg_val);

        pmu_read(PMU_REG_64, &reg_val);
        reg_val &= ~(EAR_DETECT_FLOW_DONE_INTR_EN | EAR_DETECT_FLOW_DONE_INTR_MSK);
        pmu_write(PMU_REG_64, reg_val);

        next_state = dongle_load_sensing_start();
    }
    return next_state;

dongle_gnd_check_restart:

    // to start gnd sensing
    pmu_read(PMU_REG_76, &reg_val);
    reg_val |= REG_AUTO_EAR_DETECT;
    pmu_write(PMU_REG_76, reg_val);
    return next_state;
}

static DONGLE_MAIN_STATE_E dongle_load_check (int* err)
{
    uint16_t reg_val, irq_status;
    DONGLE_MAIN_STATE_E next_state = DONGLE_MAIN_STATE_ERROR;

    irq_status = dongle_irq_status_get();
    dongle_irq_clear();

    if (!(irq_status & EAR_RES_FLOW_DONE_INTR)) {
        *err = -1;
        return next_state;
    }

    *err = 0;
    next_state = DONGLE_MAIN_STATE_KEY_CHECK;

    pmu_read(PMU_REG_78, &reg_val);
    DONGLE_TRACE(0, "-> load reg: 0x%x", reg_val);
    if (reg_val & EAR_RES_FIRST_RANGE1_MATCH) {
        DONGLE_TRACE(0, "-> 600ohm");
        s_dongle_ctx.dongle_info.earphone_info.load_type = DONGLE_EARPHONE_LOAD_600_OR_HIGHER_OHM;
    } else if (reg_val & EAR_RES_SECOND_RANGE1_MATCH) {
        DONGLE_TRACE(0, "-> 32ohm");
        s_dongle_ctx.dongle_info.earphone_info.load_type = DONGLE_EARPHONE_LOAD_32_OHM;
    } else if (reg_val & EAR_RES_SECOND_RANGE0_MATCH) {
        DONGLE_TRACE(0, "-> 16ohm");
        s_dongle_ctx.dongle_info.earphone_info.load_type = DONGLE_EARPHONE_LOAD_16_OHM;
    }  else {
        DONGLE_TRACE(0, "-> unknown load");
        s_dongle_ctx.dongle_info.earphone_info.load_type = DONGLE_EARPHONE_LOAD_UNKNOWN;
    }

    if (s_dongle_cfg.cb) {
        s_dongle_cfg.cb(DONGLE_NOTIFY_EVT_PLUGIN_LOAD_INFO, &s_dongle_ctx.dongle_info);
    }

    dongle_key_check_start();
    return next_state;
}

static DONGLE_MAIN_STATE_E dongle_state_earphone_checking_process (void)
{
    int connected = dongle_earphone_connected_check();

    DONGLE_TRACE(0, "%s: connected[%d], state[%d]", __func__, connected, s_dongle_ctx.earphone_check_sub_state);

    // if no earphone is connected to dongle, go back to idle state
    if (!connected) {
        return DONGLE_MAIN_STATE_IDLE;
    }

    int err = -1;
    DONGLE_MAIN_STATE_E next_state = DONGLE_MAIN_STATE_ERROR;

    switch (s_dongle_ctx.earphone_check_sub_state) {
        case DONGLE_SUB_STATE_PLUGIN_CHECKING:
            next_state = dongle_earphone_plugin_check(&err);
            if (err) {
                s_dongle_ctx.fault_type |= DONGLE_CHECK_FAULT_TYPE_PLUGIN;
            } else {
                s_dongle_ctx.fault_type &= ~DONGLE_CHECK_FAULT_TYPE_PLUGIN;
            }
            break;

        case DONGLE_SUB_STATE_GND_SENSING:
            next_state = dongle_gnd_check(&err);
            if (err) {
                s_dongle_ctx.fault_type |= DONGLE_CHECK_FAULT_TYPE_GND;
            } else {
                s_dongle_ctx.fault_type &= ~DONGLE_CHECK_FAULT_TYPE_GND;
            }
            break;

        case DONGLE_SUB_STATE_LOAD_SENSING:
            next_state = dongle_load_check(&err);
            if (err) {
                s_dongle_ctx.fault_type |= DONGLE_CHECK_FAULT_TYPE_LOAD;
            } else {
                s_dongle_ctx.fault_type &= ~DONGLE_CHECK_FAULT_TYPE_LOAD;
            }
            break;

        default:
            break;
    }

    if (err) {
        s_dongle_ctx.fault_cnt++;
        next_state = DONGLE_MAIN_STATE_ERROR;
    }

    return next_state;
}

static void dongle_key_dn_check (void)
{
    uint16_t reg_val = dongle_irq_status_get();

    DONGLE_TRACE(0, "%s: irq[%x]", __func__, reg_val);

    dongle_irq_clear();
    if (!(reg_val & EAR_KEY_CHANGE_INTR)) {
        return;
    }

    pmu_read(PMU_REG_78, &reg_val);
    DONGLE_TRACE(0, "-> key_det status[%x]", reg_val);
    if (reg_val & EAR_KEY_DETECT) {
        return;
    }

    if (s_dongle_cfg.cb) {
        s_dongle_cfg.cb(DONGLE_NOTIFY_EVT_WAKEUP, &s_dongle_ctx.dongle_info);
    }

    // reset GPADC
    pmu_read(PMU_REG_41, &reg_val);

    reg_val |= REG_GPADC_STATE_RESET;
    pmu_write(PMU_REG_41, reg_val);

    hal_sys_timer_delay_us(10);

    reg_val &= ~REG_GPADC_STATE_RESET;
    pmu_write(PMU_REG_41, reg_val);

    // start GPADC
    pmu_read(PMU_REG_76, &reg_val);
    reg_val |= REG_GPADC_START_DIRECT;
    pmu_write(PMU_REG_76, reg_val);

    // wait 50us
    hal_sys_timer_delay_us(50);

    // get GPADC value
    pmu_read(PMU_REG_58, &reg_val);
    reg_val &= DATA_CHAN7_MASK;
    reg_val = (reg_val * 1800) / 1024;

    DONGLE_TRACE(0, "-> GPADC value = [%d]", reg_val);

    if (reg_val >= DONGLE_DOWN_KEY_MIN_VOLT) {
        s_dongle_ctx.dongle_info.key_info = DONGLE_KEY_VOL_MINUS;
    } else if (reg_val >= DONGLE_UP_KEY_MIN_VOLT) {
        s_dongle_ctx.dongle_info.key_info = DONGLE_KEY_VOL_PLUS;
    } else {
        s_dongle_ctx.dongle_info.key_info = DONGLE_KEY_FUNC;
    }

    if (s_dongle_cfg.cb) {
        s_dongle_cfg.cb(DONGLE_NOTIFY_EVT_KEY_DN, &s_dongle_ctx.dongle_info);
    }

    s_dongle_ctx.key_check_sub_state = DONGLE_SUB_STATE_KEY_UP_SENSING;
}

static void dongle_key_up_check (void)
{
    uint16_t reg_val = dongle_irq_status_get();

    dongle_irq_clear();
    if (!(reg_val & EAR_KEY_CHANGE_INTR)) {
        return;
    }

    if (s_dongle_cfg.cb) {
        s_dongle_cfg.cb(DONGLE_NOTIFY_EVT_KEY_UP, &s_dongle_ctx.dongle_info);
    }

    s_dongle_ctx.key_check_sub_state = DONGLE_SUB_STATE_KEY_DOWN_SENSING;
}

static DONGLE_MAIN_STATE_E dongle_state_key_checking_process (void)
{
    int connected = dongle_earphone_connected_check();

    if (!connected) {
        return DONGLE_MAIN_STATE_IDLE;
    }

    if ((s_dongle_ctx.dongle_info.earphone_info.gnd_type != DONGLE_EARPHONE_GND_POLE3)
            && (s_dongle_ctx.dongle_info.earphone_info.gnd_type != DONGLE_EARPHONE_GND_POLE4))
    {
        dongle_irq_clear();
        return DONGLE_MAIN_STATE_KEY_CHECK;
    }

    switch (s_dongle_ctx.key_check_sub_state) {
        case DONGLE_SUB_STATE_KEY_DOWN_SENSING:
            dongle_key_dn_check();
            break;

        case DONGLE_SUB_STATE_KEY_UP_SENSING:
        default:
            dongle_key_up_check();
            break;
    }

    return DONGLE_MAIN_STATE_KEY_CHECK;
}

static DONGLE_MAIN_STATE_E dongle_state_error_process (void)
{
    if (s_dongle_ctx.fault_cnt >= DONGLE_CHECK_MAX_FAULT) {
        pmu_reboot();
    }

    return DONGLE_MAIN_STATE_IDLE;
}

static void dongle_irq_handler (void)
{
    DONGLE_MAIN_STATE_E next_state;

    do {
        switch (s_dongle_ctx.main_state) {
            case DONGLE_MAIN_STATE_IDLE:
                next_state = dongle_state_idle_process();
                break;

            case DONGLE_MAIN_STATE_EARPHONE_CHECK:
                next_state = dongle_state_earphone_checking_process();
                break;

            case DONGLE_MAIN_STATE_KEY_CHECK:
                next_state = dongle_state_key_checking_process();
                break;

            case DONGLE_MAIN_STATE_ERROR:
            default:
                next_state = dongle_state_error_process();
                break;
        }

        DONGLE_TRACE(0, "%s: state[%d -> %d]", __func__, s_dongle_ctx.main_state, next_state);

        if (next_state == s_dongle_ctx.main_state) {
            break;
        }
        s_dongle_ctx.main_state = next_state;
    } while(1);
}

static void dongle_irq_register (void)
{
    // to set dongle interrupt handler
    NVIC_SetVector(EARDET_IRQn, (uint32_t)dongle_irq_handler);
    // to set interrupt priority
    NVIC_SetPriority(EARDET_IRQn, IRQ_PRIORITY_NORMAL);
    // to clear penging dongle interrupts
    NVIC_ClearPendingIRQ(EARDET_IRQn);
    // to enale dongle interrupt vector
    NVIC_EnableIRQ(EARDET_IRQn);
}

void dongle_open(const DONGLE_CONFIG_T* cfg)
{
    uint16_t reg_val;

    DONGLE_TRACE(0, "%s: gnd[%u] load[%u] high_volt[%u]", __func__,
            cfg->cfg_gnd_sensing_mode,
            cfg->cfg_load_sensing_mode,
            cfg->cfg_low_level_for_plugin);

    //-------------------------------------------------------------------------------------------
    // dongle configuration
    //-------------------------------------------------------------------------------------------
    if ((cfg->cfg_gnd_sensing_mode >= DONGLE_GND_SENSING_MODE_NUM)
            || (cfg->cfg_load_sensing_mode >= DONGLE_LOAD_SENSING_MODE_NUM))
    {
        s_dongle_cfg.cfg_gnd_sensing_mode = DONGLE_GND_SENSING_AUTO;
        s_dongle_cfg.cfg_load_sensing_mode = DONGLE_LOAD_SENSING_AUTO;
        s_dongle_cfg.cfg_low_level_for_plugin = 0;
    } else {
        memcpy((uint8_t*)(&s_dongle_cfg), (uint8_t*)(cfg), sizeof(DONGLE_CONFIG_T));
    }

    //-------------------------------------------------------------------------------------------
    // earphone-plugin configuration
    //-------------------------------------------------------------------------------------------
    dongle_plugin_volt_set(s_dongle_cfg.cfg_low_level_for_plugin);

    //-------------------------------------------------------------------------------------------
    // GND-sensing configuration
    //-------------------------------------------------------------------------------------------
#ifndef DONGLE_GND_SAMPLE_RANGE0_MAX
#define DONGLE_GND_SAMPLE_RANGE0_MAX    0x60
#endif // DONGLE_GND_SAMPLE_RANGE0_MAX

#ifndef DONGLE_GND_SAMPLE_RANGE1_MIN
#define DONGLE_GND_SAMPLE_RANGE1_MIN    0x70
#endif // DONGLE_GND_SAMPLE_RANGE1_MIN

    pmu_read(PMU_REG_66, &reg_val);
    reg_val &= ~EAR_DETECT_SAMPLE_RANGE0_MAX_MASK;
    reg_val |= EAR_DETECT_SAMPLE_RANGE0_MAX(DONGLE_GND_SAMPLE_RANGE0_MAX);
    pmu_write(PMU_REG_66, reg_val);

    pmu_read(PMU_REG_67, &reg_val);
    reg_val &= ~EAR_DETECT_SAMPLE_RANGE1_MIN_MASK;
    reg_val |= EAR_DETECT_SAMPLE_RANGE1_MIN(DONGLE_GND_SAMPLE_RANGE1_MIN);
    pmu_write(PMU_REG_67, reg_val);

    //-------------------------------------------------------------------------------------------
    // load-sensing configuration
    //-------------------------------------------------------------------------------------------
    pmu_read(PMU_REG_41, &reg_val);
    reg_val &= ~SAR_VIN_GAIN_SEL_MASK;
    reg_val |= SAR_VIN_GAIN_SEL(2);
    pmu_write(PMU_REG_41, reg_val);

    //-------------------------------------------------------------------------------------------
    // key-checking parameters
    //-------------------------------------------------------------------------------------------
    pmu_read(PMU_REG_40, &reg_val);
    reg_val |= (REG_CLK_4M_PU_LDO | REG_CLK_4M_PU_LDO_DR);
    pmu_write(PMU_REG_40, reg_val);
    hal_sys_timer_delay_us(20);
    reg_val |= (REG_CLK_4M_EN | REG_CLK_4M_EN_DR);
    pmu_write(PMU_REG_40, reg_val);

    pmu_read(PMU_REG_8E, &reg_val);
    reg_val |= CLK_4M_EN_CLK_CP_OUT;
    pmu_write(PMU_REG_8E, reg_val);

    pmu_read(PMU_REG_6E, &reg_val);
    reg_val &= ~EAR_DET_CP_CKSEL;
    pmu_write(PMU_REG_6E, reg_val);

    pmu_read(PMU_REG_76, &reg_val);
    reg_val |= CLK_OSC_SEL;
    pmu_write(PMU_REG_76, CLK_OSC_SEL);

    //-------------------------------------------------------------------------------------------
    // dongle information callback
    //-------------------------------------------------------------------------------------------
    s_dongle_cfg.cb = cfg->cb;

    //-------------------------------------------------------------------------------------------
    // clear all interrupts
    //-------------------------------------------------------------------------------------------
    //dongle_irq_clear();

    //-------------------------------------------------------------------------------------------
    // set initiated state
    //-------------------------------------------------------------------------------------------
    dongle_state_reset();

    pmu_read(PMU_REG_71, &reg_val);
    reg_val |= (EAR_PLUG_CHANGE_INTR_EN|EAR_PLUG_CHANGE_INTR_MSK);
    pmu_write(PMU_REG_71, reg_val);

    //-------------------------------------------------------------------------------------------
    // interrupt handler
    //-------------------------------------------------------------------------------------------
    dongle_irq_register();

}

#endif // DONGLE_SUPPORT
