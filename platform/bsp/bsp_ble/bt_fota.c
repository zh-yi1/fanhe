#include "include.h"
#include "bt_fota.h"

#if LE_AB_FOT_EN

#define FOT_DEBUG_EN                    0

#if FOT_DEBUG_EN
#define FOT_DEBUG(...)                  printf(__VA_ARGS__)
#define FOT_DEBUG_R(...)                print_r(__VA_ARGS__)
#else
#define FOT_DEBUG(...)
#define FOT_DEBUG_R(...)
#endif

/**********************************************/
#define FOT_STA_INIT            BIT(0)
#define FOT_STA_START           BIT(1)
#define FOT_STA_PAUSE           BIT(2)

/**********************************************/
#define FOT_FLAG_UPDATE_OK      BIT(0)
#define FOT_FLAG_SYS_RESET      BIT(1)
#define FOT_FLAG_ROLE_SWITCH    BIT(2)
#define FOT_FLAG_APP_CONNECT    BIT(3)
#define FOT_FLAG_CLK_SET        BIT(4)
#define FOT_FLAG_UPDATE_EXIT    BIT(5)

/**********************************************/
#define FOT_NOTIFY_STA          0x90
#define FOT_GET_INFO            0x91
#define FOT_GET_INFO_TLV        0x92
#define FOT_OUT_DATA_START      0xA0
#define FOT_OUT_DATA_CONTINUE   0x20

/**********************************************/
#define FOT_CMD_POS             0
#define FOT_SEQ_POS             1
#define FOT_ADDR_POS            2
#define FOT_DATA_LEN_POS        6
#define DATA_START_POS          10
#define DATA_CONTINUE_POS       2

/**********************************************/
#define FOT_BLOCK_LEN           1024        //压缩升级必须是512的倍数，目前只做了512倍数的处理
#define FOTA_UI_CFG_SIZE        512
#define FOTA_UI_AREA_NUM        10//最大10个UI块


extern bool ble_fot_send_packet(u8 *buf, u16 len);
extern u16 get_spp_mtu_size(void);
extern uint8_t sys_clk_get_cur(void);
u16 dev_version = 0x0001; //设备版本号

AT(.com_text.const)
static const u8 fot_auth_data[] = {0xCC, 0xAA, 0x55, 0xEE, 0x12, 0x19, 0xE4};

typedef enum{
    INFO_DEV_VER  = 1,
    INFO_UPDATE_REQ,
    INFO_DEV_FEATURE,
    INFO_DEV_CONNECT_STA,
    INFO_EDR_ADDR,
    INFO_DEV_CHANNEL,
}DEV_INFO_E;

typedef struct __attribute__((packed)){
    u32 addr;
    u32 size;
}ui_area;

typedef struct __attribute__((packed)){
    u16 remote_ver;
    u32 total_len;
    u32 remain_len;
    u32 hash;
    u16 data_pos;
    u8 fot_recv_ok;
    u32 addr;
    u32 tick;
    u8 type;
    u8 sys_clk;
    u32 ota_recv_size;      //ota接收大小
    u32 ui_res_len;         //UI资源大小
    u32 ui_updata_address;  //更新时使用的地址
    u32 ui_recv_remain;     //256对齐
    u8  is_ui_flag;         //是否是UI数据，1是，0不是
    u32 code_res_len;       //代码区间大小
    u8 fota_finish;         //升级完成
#if FOTA_DOG_EN
    u8  ui_mode_type;       //UI升级模式中，携带FOT，其他:UI+FOT/FOT, 1:UI
    ui_area area[FOTA_UI_AREA_NUM]; //UI升级块
    u8 aidx;                        //UI升级块序号
    u8 anum;                        //UI升级块数量
#endif
}fot_s;

fot_s fot_var;

typedef enum {
    FOT_FILE_TYPE_UI = 1,
    FOT_FILE_TYPE_FOT,
}FOT_FILE_TYPE;

typedef struct {
    FOT_FILE_TYPE  type;        //文件类型
    u8  percent;                //文件进度百分比
    u32 files_all_size;         //文件长度
    u32 files_recv_size;        //文件接收长度
}fot_progress_t;
static fot_progress_t fot_progress;

static u8 fot_data[FOT_BLOCK_LEN] AT(.fot_data.buf);

static u8 fot_seq = 0;
static u8 fot_remote_seq = 0;
static u8 fot_sta = 0;
static u8 fot_flag = 0;
static u16 fot_connect_sta = 0;

fot_progress_t *bsp_fot_progress_get(void);

static bool fot_err_seq = false;

u8 is_fot_update_en(void)
{
    return 1;
}

void app_fota_pack_write(void *buf, u32 len)
{
    u32 total_use_len = 0;

    while (len && (ota_pack_get_err() == FOT_ERR_OK)) {
        ota_pack_write(buf + total_use_len);
        len -= 512;
        total_use_len += 512;
    }
}

AT(.text.fot.cache)
void app_fota_write(void *buf, u32 addr, u32 len)
{
    app_fota_pack_write(buf, len);
}

AT(.text.fot.cache)
u8 app_fota_get_err(void)
{
    return ota_pack_get_err();
}

AT(.text.fot.cache)
void app_fota_verify(void)
{
    ota_pack_verify();
}

bool app_fota_breakpoint_info_read(void)
{
    return ota_pack_breakpoint_info_read();
}

void app_fota_init(void)
{
    ota_pack_init();
    load_code_fota();
}

u32 app_fota_get_curaddr(void)
{
    return ota_pack_get_curaddr();
}

void app_fota_update_done(void)
{
    ota_pack_done();
}

bool app_fota_is_write_done(void)
{
    return ota_pack_is_write_done();
}

AT(.text.fot.cache)
static void fot_sent_proc(u8 *buf,u8 len)
{
    if((fot_flag & FOT_FLAG_APP_CONNECT) == 0){
        return;
    }

    FOT_DEBUG("fot tx:");
    FOT_DEBUG_R(buf,len);

#if LE_AB_FOT_EN
    if(ble_is_connect()){
        ble_fot_send_packet(buf,len);
    }
#endif
}

AT(.text.fot.update)
static void fot_reply_info_tlv(u8 *buf,u8 len)
{
    u8 read_offset = 0;
    u8 write_offset = 0;
    u8 rsp[32];
    u8 val_len = 0;

    if((buf == NULL) || (len == 0)){
        return;
    }

    rsp[write_offset++] = FOT_GET_INFO_TLV;
    rsp[write_offset++] = fot_seq++;

    while(read_offset < len){
        switch(buf[read_offset]){
            case INFO_DEV_VER:
                FOT_DEBUG("INFO_DEV_VER\n");
                val_len = buf[read_offset + 1];
                rsp[write_offset++] = INFO_DEV_VER;
                rsp[write_offset++] = 2;
                rsp[write_offset++] = dev_version & 0xff;
                rsp[write_offset++] = (dev_version >> 8) & 0xff;
            break;

            case INFO_DEV_FEATURE:
                FOT_DEBUG("INFO_DEV_FEATURE\n");
                {
                    u16 dev_ability = 0;
                    val_len = buf[read_offset + 1];
                    rsp[write_offset++] = INFO_DEV_FEATURE;
                    rsp[write_offset++] = 2;
                    rsp[write_offset++] = dev_ability & 0xff;
                    rsp[write_offset++] = (dev_ability >> 8) & 0xff;
                }
            break;

            case INFO_DEV_CONNECT_STA:
                FOT_DEBUG("INFO_DEV_CONNECT_STA\n");
                val_len = buf[read_offset + 1];
                rsp[write_offset++] = INFO_DEV_CONNECT_STA;
                rsp[write_offset++] = 2;
                rsp[write_offset++] = fot_connect_sta & 0xff;
                rsp[write_offset++] = (fot_connect_sta >> 8) & 0xff;
                break;

            default:
                val_len = buf[read_offset + 1];
                break;
        }
        read_offset += (2 + val_len);
    }

    if(write_offset > sizeof(rsp)){
        FOT_DEBUG("fot:rsp buf overflow!!!\n");
        while(1);
    }

    fot_sent_proc(rsp,write_offset);
}


AT(.text.fot.update)
static void fot_reply_dev_version(void)
{
    u8 data[5];

    data[0] = FOT_GET_INFO;
    data[1] = fot_seq++;
    data[2] = INFO_DEV_VER;
    memcpy(&data[3],&dev_version,2);
    fot_sent_proc(data,5);
}

AT(.text.fot.cache)
static void fot_dev_notify_sta(u8 sta)
{
    u8 buf[3];

    buf[0] = FOT_NOTIFY_STA;
    buf[1] = fot_seq++;
    buf[2] = sta;

    fot_sent_proc(buf,3);
}

AT(.text.fot.update)
u8 is_fot_start(void)
{
    return (fot_sta & FOT_STA_START) ? 1:0;
}

AT(.text.fot.update)
void bsp_fot_init(void)
{
#if (ASR_SELECT && ASR_FULL_SCENE)
    bsp_asr_stop();
#endif

    u8  dev_version_str[] = SW_VERSION;
    u16 version_temp = 0;

    memset(&fot_var,0,sizeof(fot_var));
    memset(&fot_progress, 0, sizeof(fot_progress));

    dev_version = 0x00;
    version_temp = dev_version_str[1]-'0';
    version_temp <<= 12;
    dev_version |= version_temp;

    version_temp = dev_version_str[3]-'0';
    version_temp <<= 8;
    dev_version |= version_temp;

    version_temp = dev_version_str[5]-'0';
    version_temp <<= 4;
    dev_version |= version_temp;

    fot_sta = FOT_STA_INIT;
}

void bsp_fot_exit(void)
{
    fot_sta = 0;
    fot_seq = 0;
    fot_remote_seq = 0;
    fot_err_seq = 0;
    fot_flag &= ~FOT_FLAG_APP_CONNECT;

    if(fot_var.sys_clk){
        sys_clk_set(fot_var.sys_clk);
    }

#if (ASR_SELECT && ASR_FULL_SCENE)
    bsp_asr_start();
#endif

    unlock_code_fota();

#if FOTA_DOG_EN
    if (fot_var.fota_finish) {
        ble_adv_dis();                  //升级成功,ble关闭广播,防止重连二次升级
    } else {
                 						//升级失败,退出处理
    }
#endif
}

void fot_update_pause(void)
{
    if(fot_sta & FOT_STA_START){
        fot_sta = FOT_STA_PAUSE;
        fot_dev_notify_sta(FOT_UPDATE_PAUSE);
    }
}

void fot_update_continue(void)
{
    if(fot_sta & FOT_STA_PAUSE){
        bsp_fot_init();
        fot_dev_notify_sta(FOT_UPDATE_CONTINUE);
    }
}

u16 bsp_fot_mtu_get(void)
{
    u16 packet_len = 0;
#if LE_AB_FOT_EN
    if(ble_is_connect()){
        packet_len = ble_get_gatt_mtu();
    }
#endif
    return packet_len;
}

AT(.text.fot.update)
static void fot_reply_update_request(void)
{
    u16 flash_remote_ver;
    u32 hash;
    u32 addr = 0;
    u32 block_len = FOT_BLOCK_LEN;
    u16 packet_len = 0;
    u8 data[14];
    u8 need_update = 1;

    if(bt_get_status() >= BT_STA_INCOMING){
        need_update = 0;
        fot_flag |= FOT_FLAG_UPDATE_EXIT;
        goto fot_req_reply;
    }

#if LE_AB_FOT_EN
    if(ble_is_connect()){
        ble_update_conn_param(16,0,400);
    }
#endif

    packet_len = bsp_fot_mtu_get();

    FOT_DEBUG("fot_packet_len:%d\n",packet_len);

    app_fota_init();

    fot_flag |= FOT_FLAG_CLK_SET;
    fot_sta |= FOT_STA_START;

    FOT_DEBUG("hash_val:0x%x\n",fot_var.hash);

    param_fot_remote_ver_read((u8*)&flash_remote_ver);
    param_fot_hash_read((u8*)&hash);

    FOT_DEBUG("flash hash val:0x%x\n",hash);

#if !FOTA_DOG_EN
    //关闭断点升级
    if((fot_var.hash != 0xFFFFFFFF) && (flash_remote_ver == fot_var.remote_ver) && (hash == fot_var.hash)){
        param_fot_type_read(&fot_var.type);
        if(app_fota_breakpoint_info_read() == true){
            addr = app_fota_get_curaddr();
        }
    }
#endif

fot_req_reply:
    data[0] = FOT_GET_INFO;
    data[1] = fot_seq++;
    data[2] = INFO_UPDATE_REQ;
    memcpy(&data[3],&addr,4);
    memcpy(&data[7],&block_len,4);
    memcpy(&data[11],&packet_len,2);
    data[13] = need_update;
    fot_sent_proc(data,14);
}


#if LE_AB_FOT_EN
void fot_ble_disconnect_callback(void)
{
    if((fot_flag & FOT_FLAG_APP_CONNECT) == 0){
        return;
    }

    FOT_DEBUG("--->fot_ble_disconnect_callback\n");

    bsp_fot_exit();

}

void fot_ble_connect_callback(void)
{
    FOT_DEBUG("--->fot_ble_connect_callback\n");
}
#endif

void fot_spp_connect_callback(void)
{
    FOT_DEBUG("--->fot_spp_connect_callback\n");
}

void fot_spp_disconnect_callback(void)
{
    if((fot_flag & FOT_FLAG_APP_CONNECT) == 0){
        return;
    }

    FOT_DEBUG("--->fot_spp_disconnect_callback\n");

    bsp_fot_exit();

}

#if FOTA_DOG_EN
AT(.text.fot.cache)
static void fot_dev_notify_seq_err(u8 sta)
{
    u8 buf[3];

    buf[0] = FOT_NOTIFY_STA;
    buf[1] = fot_seq;
    buf[2] = sta;

    fot_sent_proc(buf,3);
}
#endif

AT(.text.fot.cache)
void fot_recv_proc(u8 *buf, u16 len)
{
    u32 addr;
    u32 recv_data_len;
    u8 cmd;

#if !FOTA_DOG_EN
    if(fot_remote_seq != buf[FOT_SEQ_POS]){
        if(memcmp(fot_auth_data, buf, 7)){      //接入码先过掉
            FOT_DEBUG("remote seq err:%d,%d\n",fot_remote_seq,buf[FOT_SEQ_POS]);
            fot_dev_notify_sta(FOT_ERR_SEQ);
            fot_flag |= FOT_FLAG_UPDATE_EXIT;
        }
        return;
    }
#else
    if(fot_err_seq){
        if(fot_remote_seq != buf[FOT_SEQ_POS]){
            return;
        }
    }
    if(fot_remote_seq != buf[FOT_SEQ_POS]){
        fot_dev_notify_seq_err(FOT_ERR_SEQ);
        fot_err_seq = true;
        return;
    }
    fot_err_seq = false;
#endif

    fot_remote_seq++;

    if((fot_sta & FOT_STA_INIT) == 0){
        return;
    }

    cmd = buf[FOT_CMD_POS];

    switch(cmd){
    case FOT_GET_INFO_TLV:
        fot_reply_info_tlv(&buf[2],len-2);
        break;

    case FOT_GET_INFO:
        if(buf[2] == INFO_DEV_VER){
            fot_reply_dev_version();
        }else if(buf[2] == INFO_UPDATE_REQ){
            memcpy(&fot_var.remote_ver,&buf[3],2);
            memcpy(&fot_var.hash,&buf[5],4);
            fot_reply_update_request();
        }
        break;

    case FOT_OUT_DATA_START:
        if(fot_var.remain_len){
            FOT_DEBUG("--->len err:%d\n",fot_var.remain_len);
            fot_dev_notify_sta(FOT_ERR_DATA_LEN);
            fot_flag |= FOT_FLAG_UPDATE_EXIT;
            return;
        }
        recv_data_len = len-DATA_START_POS;
        memcpy(&fot_var.total_len,&buf[FOT_DATA_LEN_POS],4);
        fot_var.remain_len = fot_var.total_len - recv_data_len;
        memcpy(&addr,&buf[FOT_ADDR_POS],4);
        memcpy(&fot_data[fot_var.data_pos],&buf[DATA_START_POS],recv_data_len);
        fot_var.data_pos += recv_data_len;
        if(fot_var.remain_len == 0){
            fot_var.fot_recv_ok = 1;
            fot_var.data_pos = 0;
        }
        break;

    case FOT_OUT_DATA_CONTINUE:
        recv_data_len = len-DATA_CONTINUE_POS;
        if(fot_var.remain_len < recv_data_len){
           recv_data_len =  fot_var.remain_len;
        }
        fot_var.remain_len -= recv_data_len;
        memcpy(&fot_data[fot_var.data_pos],&buf[DATA_CONTINUE_POS],recv_data_len);
        fot_var.data_pos += recv_data_len;
        if(fot_var.remain_len == 0){
            fot_var.fot_recv_ok = 1;
            fot_var.data_pos = 0;
        }
        break;
    }
}

#if FOTA_UI_EN

static const u8 ui_ota_res_sign[8] = {0x44, 0x55, 0x46, 0x2D, 0x55, 0x49, 0x41, 0x42};//DUF-UIAB
#if FOTA_DOG_EN
static const u8 ui_ota_end_sign[8] = {0x44, 0x55, 0x46, 0x2D, 0x45, 0x58, 0x49, 0x54};//DUF-EXIT
#endif // FOTA_DOG_EN
static u8 fot_data_buff[512] AT(.fot_data.buf);

//256对写入
AT(.text.fot.cache)
void bsp_fot_ui_write_page(void *buf)
{
#if FOTA_DOG_EN
    if (fot_var.ui_updata_address - fot_var.area[fot_var.aidx].addr == fot_var.area[fot_var.aidx].size) {   //addr-4k对齐,size-256对齐
        if (fot_var.aidx < (fot_var.anum - 1)) {
            fot_var.aidx++;
            fot_var.ui_updata_address = fot_var.area[fot_var.aidx].addr;
            printf("new_addr:%d size:%d idx:%d\n", fot_var.area[fot_var.aidx].addr, fot_var.area[fot_var.aidx].size, fot_var.aidx);
        }
    }
#endif
    if((fot_var.ui_updata_address & 0xFFF) == 0){
        os_spiflash_erase(fot_var.ui_updata_address);
    }
    os_spiflash_program(buf, fot_var.ui_updata_address, 256);
    fot_var.ui_updata_address += 256;
}

AT(.text.fot.cache)
void bsp_fot_ui_updata_do(u8 *buf, u32 len)
{
    uart_putchar('*');
    if(fot_var.ui_recv_remain){
        if((len + fot_var.ui_recv_remain) >= 256){
            memcpy(&fot_data_buff[fot_var.ui_recv_remain], buf, 256-fot_var.ui_recv_remain);
            bsp_fot_ui_write_page(fot_data_buff);
            len -= 256-fot_var.ui_recv_remain;
            buf += 256-fot_var.ui_recv_remain;
            fot_var.ui_recv_remain = 0;
        }else{
            memcpy(&fot_data_buff[fot_var.ui_recv_remain], buf, len);
            fot_var.ui_recv_remain += len;
            len = 0;
            return;
        }
    }

    while(len >= 256){
        bsp_fot_ui_write_page(buf);
        buf += 256;
        len -= 256;
    }

    if(len){
        memcpy(fot_data_buff, buf, len);
        fot_var.ui_recv_remain = len;
    }
}

AT(.text.fot.cache)
void bsp_fot_code_updata_do(u8 *buf, u32 len)
{
#if FOTA_DOG_EN
    if (fot_var.ui_mode_type == 1) {    //纯UI升级不执行此项
        printf("--->ui update success\n");
        fot_var.fota_finish = true;
        return;
    }
#endif
    uart_putchar('.');
    if(fot_var.ui_recv_remain){
        if((len + fot_var.ui_recv_remain) >= 512){
            memcpy(&fot_data_buff[fot_var.ui_recv_remain], buf, 512-fot_var.ui_recv_remain);
            app_fota_write(fot_data_buff, app_fota_get_curaddr(), 512);
            len -= 512-fot_var.ui_recv_remain;
            buf += 512-fot_var.ui_recv_remain;
            fot_var.ui_recv_remain = 0;
        }else{
            memcpy(&fot_data_buff[fot_var.ui_recv_remain], buf, len);
            fot_var.ui_recv_remain += len;
            len = 0;
            return;
        }
    }

    while(len >= 512){
        app_fota_write(buf, app_fota_get_curaddr(), 512);
        buf += 512;
        len -= 512;
    }

    if(len){
        memcpy(fot_data_buff, buf, len);
        fot_var.ui_recv_remain = len;
    }
}
#endif

#if FOTA_DOG_EN
AT(.text.fot.cache)
u8 bsp_ui_cfg_analysis(u8 **addr, u32 cfg_len) {
    u8 *buf = *addr;
    fot_var.ui_updata_address = FLASH_UI_BASE;
    fot_var.ui_recv_remain    = 0;
    fot_var.aidx = 0;
    fot_var.anum = 1;
    int i = 0;
    int j = 0;
    int k = 0;
    int num = 0;
    int error_flag = 0;
    u8 res_sign_len = sizeof(ui_ota_res_sign);
    u8 end_sign_len = sizeof(ui_ota_end_sign);
    if (memcmp(ui_ota_res_sign, buf, res_sign_len) == 0) {
        fot_var.is_ui_flag = 1;
        i+=(res_sign_len+2);
        while (i < cfg_len) {
            if (i <= (cfg_len - end_sign_len)) {
                if (memcmp(ui_ota_end_sign, buf+i, end_sign_len) == 0) {
                    i+=end_sign_len;
                    break;
                }
            } else {
                printf("error: file too long\n");
                error_flag = 1;
                break;
            }

            if (*(buf+i) == '#') {                             //参数解析
                i++;
                while (i < cfg_len) {
                    num = num * 10 + *(buf + i++) - '0';
                    if(*(buf+i) == 0x0d) {
                        if (k == 0) {
                            fot_var.ui_mode_type = num;
                        } else if (k == 1) {
                            fot_var.ui_res_len = num;
                        }
                        printf("#%d\n", num);
                        k++;
                        i+=2;
                        num = 0;
                        break;
                    }
                }
            } else if (*(buf+i) == '@') {                      //ui area 解析
                i++;
                if (fot_var.anum >= FOTA_UI_AREA_NUM) {
                    printf("error: too many areas\n");
                    error_flag = 2;
                    break;
                }
                while (i < cfg_len) {
                    num = num * 10 + *(buf + i++) - '0';
                    if (*(buf+i) == '|') {
                        fot_var.area[j].addr = num;
                        i++;
                        num = 0;
                        while (i < cfg_len) {
                            num = num * 10 + *(buf + i++) - '0';
                            if (*(buf+i) == 0x0d) {
                                printf("@%d|%d\n", fot_var.area[j].addr, num);
                                fot_var.area[j++].size = num;
                                i+=2;
                                num = 0;
                                fot_var.anum = j;
                                break;
                            }
                        }
                        break;
                    }
                }
            } else {
                i++;
            }
        }

        *addr += cfg_len;
        fot_var.total_len -= cfg_len;
        fot_var.ui_updata_address = fot_var.area[0].addr;
        printf("UI_ADDR:%d\n", fot_var.ui_updata_address);
        printf("UI_SIZE:%d\n", fot_var.ui_res_len);
        printf("UI_MODE:%d\n", fot_var.ui_mode_type);
        printf("UI_ANUM:%d\n", fot_var.anum);
        for (int t = 0; t < j; t++) {
            printf("UI_AREA[%d]-addr:%d, size:%d\n", t, fot_var.area[t].addr, fot_var.area[t].size);
        }
        printf("new_addr:%d size:%d idx:%d\n", fot_var.area[fot_var.aidx].addr, fot_var.area[fot_var.aidx].size, fot_var.aidx);

    }
    return error_flag;
}

u8 bsp_ui_mode_get(void)
{
    return fot_var.ui_mode_type;
}
#endif

AT(.text.fot.cache)
void bsp_fot_process(void)
{
    if(fot_sta & FOT_STA_START){
        if(fot_var.fot_recv_ok){
            fot_var.fot_recv_ok = 0;
        #if FOTA_UI_EN
            u8 *buff_ptr = fot_data;

            if(fot_var.ota_recv_size == 0){
                //print_r(fot_data, fot_var.total_len);
                #if !FOTA_DOG_EN
                fot_var.ui_updata_address = FLASH_UI_BASE;
                fot_var.ui_recv_remain    = 0;
                if(memcmp(ui_ota_res_sign, fot_data, 8) == 0){
                    u32 temp = 0;
                    u8 i = 0;
                    for(i=8; i<23; i++){
                        if(fot_data[i] == 0x0D){
                            break;
                        }else{
                            temp = temp *10 + fot_data[i]-'0';
                        }
                    }
                    fot_var.ui_res_len = temp;
                    fot_var.is_ui_flag = 1;
                    i += 2;

                    //增加代码大小 批处理需要新的
                    if((fot_data[i] != 'F') || (fot_data[i+1] != 'O') || (fot_data[i+2] != 'N')){
                        temp = 0;
                        for(; i<32; i++){
                            if(fot_data[i] == 0x0D){
                                break;
                            }else{
                                temp = temp *10 + fot_data[i]-'0';
                            }
                        }
                        fot_var.code_res_len = temp;
                        i += 2;
                    }
                    fot_var.total_len -= i;
                    buff_ptr += i;

                    func_cb.sta = FUNC_OTA_UI_MODE;         //进入UI OTA模式，避免UI刷死机
                    printf("UI SIZE:%d CODE SIZE:%d %d\n", fot_var.ui_res_len, fot_var.code_res_len, i+2);
                }
                #else
                    u8 error = bsp_ui_cfg_analysis(&buff_ptr, FOTA_UI_CFG_SIZE);
                    if (error) {
                        printf("error: info analyzing : id:%d\n", error);
                        fot_dev_notify_sta(FOT_ERR_NO_UPDATA);
                        fot_flag |= FOT_FLAG_UPDATE_EXIT;
                        printf("-->FOT_FLAG_UPDATE_EXIT\n");
                        bsp_fot_exit();
                        fot_flag &= ~FOT_FLAG_UPDATE_EXIT;
                        ble_disconnect();                                       //升级过程ERR OR EXIT 主动断开
                        return;

                    }
                    func_cb.sta = FUNC_OTA_UI_MODE;         //进入UI OTA模式，避免UI刷死机
                #endif
            }

            if(fot_var.is_ui_flag){
                if((fot_var.ota_recv_size + fot_var.total_len) <= fot_var.ui_res_len){
                    bsp_fot_ui_updata_do(buff_ptr, fot_var.total_len);
                }else{
                    if(fot_var.ota_recv_size < fot_var.ui_res_len){
                        bsp_fot_ui_updata_do(buff_ptr, fot_var.ui_res_len-fot_var.ota_recv_size);
                        if(fot_var.ui_recv_remain){
                            bsp_fot_ui_write_page(fot_data_buff);
                            fot_var.ui_recv_remain = 0;
                        }
                        bsp_fot_code_updata_do(fot_data+(fot_var.ui_res_len-fot_var.ota_recv_size), fot_var.total_len - (fot_var.ui_res_len-fot_var.ota_recv_size));
                    }else{
                        bsp_fot_code_updata_do(fot_data, fot_var.total_len);
                    }
                }
            }else{
                app_fota_write(fot_data, app_fota_get_curaddr(), fot_var.total_len);
            }
            fot_var.ota_recv_size += fot_var.total_len;
        #else
            app_fota_write(fot_data, app_fota_get_curaddr(), fot_var.total_len);
        #endif
            bsp_fot_progress_get();
            if(app_fota_is_write_done()){
                app_fota_verify();
                printf("fota check err:%d, done:%d\n", app_fota_get_err(), app_fota_is_write_done());
                if(app_fota_get_err()){
                    fot_dev_notify_sta(app_fota_get_err());
                    fot_flag |= FOT_FLAG_UPDATE_EXIT;
                }else{
                    printf("--->fot update success\n");
                    app_fota_update_done();
                    fot_var.fota_finish = true;
                }
            }else{
                fot_dev_notify_sta(app_fota_get_err());
                if(app_fota_get_err()){
                    printf("err:0x%x\n", app_fota_get_err());
                    fot_flag |= FOT_FLAG_UPDATE_EXIT;
                }
            }
            if (fot_var.fota_finish) {
                fot_flag |= (FOT_FLAG_UPDATE_OK | FOT_FLAG_SYS_RESET);
                fot_var.tick = tick_get();
                fot_dev_notify_sta(FOT_UPDATE_DONE);
            }
        }
    }

    if(fot_flag & FOT_FLAG_SYS_RESET){
        if(tick_check_expire(fot_var.tick,3000)){
            fot_flag &= ~FOT_FLAG_SYS_RESET;
            FOT_DEBUG("-->fota update ok,sys reset\n");
            WDT_RST();
        }
    }

    if(fot_flag & FOT_FLAG_CLK_SET){
        FOT_DEBUG("--->FOT_FLAG_CLK_SET\n");
        fot_var.sys_clk = sys_clk_get_cur();
        sys_clk_set(SYS_CLK_SEL);
        fot_flag &= ~FOT_FLAG_CLK_SET;
    }

    if(fot_flag & FOT_FLAG_UPDATE_EXIT){
        FOT_DEBUG("-->FOT_FLAG_UPDATE_EXIT");
        bsp_fot_exit();
        fot_flag &= ~FOT_FLAG_UPDATE_EXIT;
#if FOTA_DOG_EN
        ble_disconnect();                                       //升级过程ERR OR EXIT 主动断开
#endif
    }
}


AT(.com_text.fot)
u8 fot_app_connect_auth(uint8_t *packet, uint16_t size)
{
    if(!fot_sta){
        if(size == 7 && !memcmp(fot_auth_data, packet, 7)){     //sizeof(fot_auth_data)
            fot_ble_connect_callback();
            fot_flag |= FOT_FLAG_APP_CONNECT;
            bsp_fot_init();
        }
    }

    return fot_sta;
}

AT(.com_text.fot)
bool bsp_fot_is_connect(void)
{
    return (fot_sta>0);
}

AT(.text.fot.cache)
fot_progress_t *bsp_fot_progress_get(void)
{
    fot_progress_t *p = &fot_progress;

    if (fot_var.is_ui_flag) {
        if (fot_var.ota_recv_size < fot_var.ui_res_len) {
            p->type = FOT_FILE_TYPE_UI;
            p->files_all_size = fot_var.ui_res_len;
            p->files_recv_size = fot_var.ota_recv_size;
        } else {
            if (ota_pack_get_total_size() == 0 && ota_pack_get_curaddr() == 0) {
                p->type = FOT_FILE_TYPE_UI;
                p->files_all_size = fot_var.ui_res_len;
                p->files_recv_size = fot_var.ui_res_len;
            } else {
                p->type = FOT_FILE_TYPE_FOT;
                p->files_all_size = ota_pack_get_total_size();
                p->files_recv_size = ota_pack_get_curaddr();
            }
        }
    } else {
        p->type = FOT_FILE_TYPE_FOT;
        p->files_all_size = ota_pack_get_total_size();
        p->files_recv_size = ota_pack_get_curaddr();
    }

    if (p->files_all_size) {
        p->percent = p->files_recv_size * 100 / p->files_all_size;
    }

    return p;
}

#else
WEAK void fot_update_pause(void)
{

}

WEAK void fot_update_continue(void)
{

}

#endif  //FOT_EN
