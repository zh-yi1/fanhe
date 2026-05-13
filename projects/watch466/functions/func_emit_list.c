#include "include.h"
#include "func.h"

#if BT_EMIT_EN

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define FUNC_BT_SEARCH_RESULT_CNT       10           //搜索耳机名称个数
#define EMIT_INFO_TARGET_IDX            4           //连接的蓝牙名耳机数据存储索引

enum {
    COMPO_ID_LISTBOX = 1,
};

typedef struct f_emit_list_info_t_ {
    char name[30];
    u8 addr[6];
    int8_t rssi;
    compo_listbox_move_cb_t mcb;
} f_emit_list_info_t;

typedef struct f_emit_list_t_ {
    f_emit_list_info_t emit_info[FUNC_BT_SEARCH_RESULT_CNT];
    u8 emit_info_cnt;
    bool flag_linkkey_auth_fail;

    compo_listbox_t *listbox;
    compo_listbox_item_t list[10];
    compo_listbox_custom_item_t tbl_txt_list[10];
} f_emit_list_t;

void bt_set_connect_addr(uint8_t *bd_addr);

void bt_drop_link_key_callback(void)
{
    f_emit_list_t *f_emit_list = (f_emit_list_t *)func_cb.f_cb;
    f_emit_list->flag_linkkey_auth_fail = true;
    printf("SET flag_linkkey_auth_fail\n");
}


void func_emit_list_sort(f_emit_list_info_t *info, int n)
{

    f_emit_list_info_t tmp_info;

    //由小到大排序
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (info[j].rssi < info[j + 1].rssi) {
                // 交换元素
                memset(&tmp_info, 0, sizeof(f_emit_list_info_t));
                memcpy(&tmp_info, &info[j], sizeof(f_emit_list_info_t));

                memcpy(&info[j], &info[j+1], sizeof(f_emit_list_info_t));
                memcpy(&info[j+1], &tmp_info, sizeof(f_emit_list_info_t));

            }
        }
    }
}


u8 func_emit_list_save(u8 *packet)
{
    f_emit_list_t *f_emit_list = (f_emit_list_t *)func_cb.f_cb;

//    printf("f_emit_list->emit_info_cnt:%d\n", f_emit_list->emit_info_cnt);

    if (func_cb.sta != FUNC_EMIT_LIST || f_emit_list->emit_info_cnt > FUNC_BT_SEARCH_RESULT_CNT-1) {
        return 0;
    }

    char *name = (char *)(packet + 2);
    u8 *addr = packet + packet[0] - 6;
    int8_t rssi = (int8_t)packet[1];


    for(u8 n=0;n<f_emit_list->emit_info_cnt;n++) {
        if(memcmp(name, f_emit_list->emit_info[n].name, strlen(name)) == 0) {
            printf("### the name name:%s\n", name);
            return 0;
        }
    }

    printf("result name:%s, rssi:%d, strlen(name):%d\n", name, rssi, strlen(name));
    print_r(addr, 6);

    memcpy(f_emit_list->emit_info[f_emit_list->emit_info_cnt].name, name, strlen(name));
    memcpy(f_emit_list->emit_info[f_emit_list->emit_info_cnt].addr, addr, 6);
    f_emit_list->emit_info[f_emit_list->emit_info_cnt].rssi = rssi;
    f_emit_list->emit_info_cnt++;

    func_emit_list_sort(f_emit_list->emit_info, f_emit_list->emit_info_cnt);

//    for(int i = 0; i < f_emit_list->emit_info_cnt; i++) {
//        printf("@@@ sort name[%s] rssi[%d]\n", f_emit_list->emit_info[i].name, f_emit_list->emit_info[i].rssi);
//    }

    return 0;
}

void bsp_emit_info_handle(u8 *packet)
{
    func_emit_list_save(packet);
    msg_enqueue(EVT_EMIT_LIST_REFRESH);
}

void func_emit_list_refresh(void)
{
    f_emit_list_t *f_emit_list = (f_emit_list_t *)func_cb.f_cb;
    compo_listbox_custom_item_t *tbl_txt_list = f_emit_list->tbl_txt_list;
    compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    u8 index;

    for(u8 i=0;i<f_emit_list->emit_info_cnt;i++) {
//        sprintf(str_txt, "%s rssi:%ddbm", f_map->emit_info[i].name, f_map->emit_info[i].rssi);
        memset(&tbl_txt_list[i], 0, sizeof(compo_listbox_custom_item_t));
        memcpy(tbl_txt_list[i].str_txt, f_emit_list->emit_info[i].name, strlen(f_emit_list->emit_info[i].name));
    }
    compo_listbox_set_text_modify(listbox, tbl_txt_list);
    index = f_emit_list->emit_info_cnt > 2 ? (f_emit_list->emit_info_cnt - 2) : 0;
    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, index));
    compo_listbox_update(listbox);
}


//创建地图窗体
compo_form_t *func_emit_list_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

//    printf("listbox->mcb:%x\n", listbox->mcb);
    if (/*listbox->mcb == NULL && */func_cb.sta == FUNC_EMIT_LIST) {
        f_emit_list_t *f_emit_list = (f_emit_list_t *)func_cb.f_cb;
        compo_listbox_custom_item_t *tbl_txt_list = f_emit_list->tbl_txt_list;
        compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
        memset(f_emit_list->list, 0, sizeof(compo_listbox_item_t) * 10);
        memset(tbl_txt_list, 0, sizeof(compo_listbox_custom_item_t) * 10);

        listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_MENU_NORMAL);
        compo_listbox_set(listbox, f_emit_list->list, 10);
        compo_setid(listbox, COMPO_ID_LISTBOX);
        compo_listbox_set_bgimg(listbox, UI_BUF_COMMON_BG_BIN);

        compo_listbox_set_text_modify(listbox, tbl_txt_list);
        compo_listbox_set_focus_byidx(listbox, 1);
        listbox->mcb = &f_emit_list->mcb;
        compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, 10 - 2));
        compo_listbox_update(listbox);
        bt_emit_refresh();
    }


    return frm;
}

//地图功能事件处理
static void func_emit_list_process(void)
{
    compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_move(listbox);

    func_process();
}

//地图功能消息处理
static void func_emit_list_message(size_msg_t msg)
{
    f_emit_list_t *f_emit_list = (f_emit_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    if (compo_listbox_message(listbox, msg)) {
        return;
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        {
            int num = compo_listbox_select(listbox, ctp_get_sxy());
            printf("click num:%d\n", num);
            bt_emit_refresh_cancel();
            delay_5ms(1);
//            bt_nor_delete_link_info();
            memset(xcfg_cb.bt_connect_name, 0, 32);
            memcpy(xcfg_cb.bt_connect_name, f_emit_list->emit_info[num].name, strlen(f_emit_list->emit_info[num].name));
//            printf("ready connect name:%s, addr:", xcfg_cb.bt_connect_name);
            print_r(f_emit_list->emit_info[num].addr, 6);
            bt_set_connect_addr(f_emit_list->emit_info[num].addr);
//            bt_put_ext_link_info(emit_info[idx].bd_addr, 0 ,6);
            bt_connect_address();
        }
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;


    case MSG_SYS_1S:
        if (bt_is_connected()) {
            bt_emit_refresh_cancel();
            func_cb.sta = FUNC_MUSIC;
        }
        break;

    case EVT_EMIT_LIST_REFRESH:
        func_emit_list_refresh();
        break;

    case EVT_EMIT_RECONNECT:
        //因为鉴权失败的回连
        if (f_emit_list->flag_linkkey_auth_fail) {
            f_emit_list->flag_linkkey_auth_fail = false;
            bt_connect_address();
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入地图功能
void func_emit_list_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_emit_list_t));
//    f_map_t *f_map = (f_map_t *)func_cb.f_cb;

    func_cb.frm_main = func_emit_list_form_create();
}

//退出地图功能
void func_emit_list_exit(void)
{
    func_cb.last = FUNC_EMIT_LIST;
}

//地图功能
void func_emit_list(void)
{
    printf("%s\n", __func__);
    func_emit_list_enter();
    while (func_cb.sta == FUNC_EMIT_LIST) {
        func_emit_list_process();
        func_emit_list_message(msg_dequeue());
    }
    func_emit_list_exit();
}
#endif

