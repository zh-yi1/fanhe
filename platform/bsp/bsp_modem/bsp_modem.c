#include "include.h"
#include "api.h"
#include "api_modem.h"
#include "api_iis.h"

void mp3_res_play_kick(u32 addr, u32 len);

void modem_call_init(void);
void modem_call_exit(void);
void sco_buffer_reset(void);

int contacts_kvdb_init(void);
int contacts_kvdb_set(const char *name, const char *number);
int contacts_kvdb_get_number(const char *name, char *number, u16 max_num_len);
u32 contacts_kvdb_get_list(u32 flash_addr, u32 max_len);

int  bsp_modem_get_contains(u16 from, u16 to);
int  bsp_modem_update_contains(void);
void bsp_modem_sta_init(void);

void bsp_modem_call_check_ok(bool is_ok);
void modem_call_notice_state(void *param);
void modem_call_notice_ring(void *param);
void modem_call_notice_current_call(void *param);

bsp_modem_t modem_cb;

#if MODEM_CAT1_EN

#define FLAG_MODEM_INIT BIT(0)
#define FLAG_MODEM_AUDIO BIT(1)

static u8 modem_flag = 0;

volatile bool modem_bypass_flag = true;

bool cat1_is_bypass(void)
{
    return modem_bypass_flag;
}

// static u8 state = 0;

// void modem_test_machine(void)
// {
//     printf("%s %d\n", __func__, state);
//     switch (state) {
//     case 0:
//         modem_net_read_plmn_select();
//         break;
//     case 1:
//         modem_net_read_eps_regist();
//         break;
//     case 2:
//         modem_mtcas_req_sq();
//         break;
//     case 3:
//         modem_general_req_psn_id(TE_PSN_IMEI);
//         break;
//     }
//     state++;
// }

AT(.com_text) NO_INLINE
static void bsp_modem_set_flag(u8 flag)
{
    GLOBAL_INT_DISABLE();
    modem_flag |= flag;
    GLOBAL_INT_RESTORE();
}

AT(.com_text) NO_INLINE
static void bsp_modem_clr_flag(u8 flag)
{
    GLOBAL_INT_DISABLE();
    modem_flag &= ~flag;
    GLOBAL_INT_RESTORE();
}

ALWAYS_INLINE
static bool bsp_modem_get_flag(u8 flag)
{
    return (modem_flag & flag);
}

void bsp_modem_init(void)
{
    if (!bsp_modem_get_flag(FLAG_MODEM_INIT)) {
        bsp_modem_set_flag(FLAG_MODEM_INIT);
        modem_bypass_flag = true;
        printf("%s\n", __func__);

        modem_platform_init();
        GPIOFFEN &= ~(BIT(3));
        GPIOFDE |= (BIT(3));
        GPIOFDIR &= ~(BIT(3));
        GPIOF |= (BIT(3));

        modem_cb.spr = SPR_16000;
        bsp_modem_sta_init();
        modem_platform_init_end();
        printf("init end\n");

        // TODO
        // SENDER_STA_UP();
#if 0
        bsp_modem_update_contains();
        contacts_kvdb_init();
        modem_at_test();
        delay_ms(10);
        bsp_modem_update_contains();
#endif
    }
}

void bsp_modem_exit(void)
{
    if (bsp_modem_get_flag(FLAG_MODEM_INIT)) {
        bsp_modem_clr_flag(FLAG_MODEM_INIT);
        SENDER_STA_DOWN();
        modem_platform_exit();
    }

    GPIOFFEN &= ~(BIT(3));
    GPIOFDE |= (BIT(3));
    GPIOFDIR &= ~(BIT(3));
    GPIOFCLR = (BIT(3));
}

int bsp_modem_get_init_flag(void)
{
    return bsp_modem_get_flag(FLAG_MODEM_INIT);
}

u8 bsp_modem_get_spr(void)
{
    return modem_cb.spr;
}

void modem_sms_notice(uint evt, void *param, u8 type)
{
    switch (evt) {
    case MODEM_SMS_NOTICE_DELIVER:
    {
        char *ud = ((struct sms_notice_deliver *)param)->ud;
        printf("recv sms: %s\n", ud);
#if FLASHDB_EN
        msg_tsdb_append_user("DuanXin",
                             ((struct sms_notice_deliver *)param)->oa,
                             ud);
#endif
        break;
    }

    default:
        break;
    }
}

void modem_mtcas_notice(uint evt, void *param, u8 type)
{
    switch (evt) {
    case MODME_MTCAS_NOTICE_READ_PB:
    {
        struct mtcas_notice_read_pb *notice = param;
        printf("(%d %s %d %s)\n",
               notice->index,
               notice->number,
               notice->type,
               notice->name);
#if FLASHDB_EN
        contacts_kvdb_set(notice->name, notice->number);
#endif
        break;
    }
    case MODME_MTCAS_NOTICE_REQ_SQ:
    {
        struct mtcas_notice_req_sq *notice = param;
        printf("signal quality %d\n", notice->rssi);
        break;
    }
    case MODME_MTCAS_NOTICE_HOT_PLUG:
    {
        struct mtcas_notice_hot_plug_report *notice = param;
        printf("sim plug: %d %d\n", notice->slot, notice->slotstate);
        break;
    }

    default:
        break;
    }
}

void modem_general_notice(uint evt, void *param, u8 type)
{
    switch (evt) {
    case MODEM_GENERAL_REQ_IMS_ID:
        printf("IMSI %s\n", param);
        break;
    case MODEM_GENERAL_REQ_PSN_ID:
    {
        printf("IMEI %s\n", param);
        break;
    }
    case MODEM_GENERAL_REQ_ICCID:
    {
        struct general_notice_req_iccid *notice = param;
        printf("ICCID %s\n", notice->iccid);
        break;
    }
    default:
        break;
    }
}

void modem_net_notice(uint evt, void *param, u8 type)
{
    printf("%s type:%d\n", __func__, type);
    switch (evt) {
    case MODEM_NET_STATUS_REPORT:
    {
        struct net_notice_status_report *notice = param;
        printf("STATUS_REPORT %d %d %s %s %s\n",
               notice->time_zone,
               notice->sav_time,
               notice->oper_long,
               notice->oper_short,
               notice->univer_time);
        break;
    }
    case MODEM_NET_EPS_REGISTRAION:
    {
        if (type == AT_TYPE_ACTIVE_REPORT) {
            struct net_notice_eps_registraion_report *notice = param;
            printf("EPS_REGISTRAION %d %s %s\n",
                   notice->stat,
                   notice->tac,
                   notice->ci);
        } else if (type == AT_TYPE_CMD_READ) {
            struct net_notice_eps_registraion_read *notice = param;
            printf("EPS_REGISTRAION %d %d %s %s\n",
                   notice->n,
                   notice->stat,
                   notice->tac,
                   notice->ci);
        }
        break;
    }
    case MODEM_NET_PLMN_SELECTION:
    {
        struct net_notice_plmn_selection *notice = param;
        printf("PLMN_SELECTION %d %d %s %d %d\n",
               notice->mode,
               notice->format,
               notice->oper,
               notice->stat,
               notice->act);
        break;
    }
    }
}

void modem_cmd_notice(bool is_ok, u32 error_code)
{
    printf("%s %d %d\n", __func__, is_ok, error_code);
    bsp_modem_call_check_ok(is_ok);
}

void modem_send_timeout_cb(u32 n_5ms)
{
    printf("%s %d\n", __func__, n_5ms);
}

// bool modem_check_can_send_callback(modem_err_t err, u32 err_code)
// {
//     switch (err)
//     {
//     case MODEM_ETIMEOUT:
//         // printf("Error: AT timeout\n");
//         // 1. reset
//         // modem_at_reset();
//         // return true;

//         // 2. 继续等
//         return false;
//     case MODEM_ENOINIT:
//         printf("Error: AT no initialized\n");
//         return false;
//     case MODEM_RECV_ERROR:
//         break;
//     default:
//         break;
//     }
//     return true;
// }

uint32_t iis_buf[60 * 4] AT(.sco_cache.buf);

static void bsp_iis_init(u8 spr)
{
    printf("%s\n", __func__);

    CLKGAT0 |= BIT(10);     //iis_clken
    RSTCON0 |= BIT(1);
    delay_us(5);

    iis_spr_set(spr);

    iis_base_init(1, true, false, false, true, true, false);
    IISCON0 |= BIT(18);
    IISCON0 |= BIT(19);

    iis_dma_init(iis_buf, iis_buf + 60 * 2, 60);

    sco_buffer_reset();
    aubuf0_gpdma_init(0);
}

static void bsp_iis_stop(void)
{
    iis_stop();
    RSTCON0 &= ~BIT(1);
    CLKGAT0 &= ~BIT(10);    //iis_clk dis
}

void bsp_modem_audio_start(void)
{
    if (!bsp_modem_get_flag(FLAG_MODEM_AUDIO)) {
        bsp_modem_set_flag(FLAG_MODEM_AUDIO);
        modem_bypass_flag = false;
        bt_audio_bypass();
        bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
        modem_call_init();
        bsp_iis_init(modem_cb.spr);
        iis_start();
    }
}

void bsp_modem_audio_stop(void)
{
    if (bsp_modem_get_flag(FLAG_MODEM_AUDIO)) {
        bsp_modem_clr_flag(FLAG_MODEM_AUDIO);
        bsp_iis_stop();
        modem_call_exit();
        bsp_change_volume(sys_cb.vol);
        bt_audio_enable();
        modem_bypass_flag = true;
    }
}

int bsp_modem_get_contains(u16 from, u16 to)
{
    contacts_kvdb_init();
    modem_mtcas_get_contains(from, to);
    return 0;
}

int bsp_modem_update_contains(void)
{
    contacts_kvdb_init();
    u32 addr = FLASH_UI_BASE + FLASH_UI_SIZE + 512;
    contacts_kvdb_get_list(addr, 1024);
    return 0;
}

AT(.com_text)
int get_modem_dig_gain(void)
{
    return AEC_DIG_P10DB;
}
#else
AT(.com_text)
void iis_isr(void) {}
void bsp_iis_stop(void) {}

int bsp_modem_get_init_flag(void)
{
    return 0;
}
void bsp_modem_init(void) {}
void bsp_modem_exit(void) {}
#endif
