#include "include.h"
#include "home_top_time.h"
#include "home_ui_shared.h"
#include "home_ui_gpu_detach.h"

#if ELUNCHBOX_PANEL_EN
/* ELUNCHBOX 模式：TE block 标志声明 */
extern volatile u8 elunchbox_te_block_flag;
#endif

#ifndef UI_BUF_HOME_0M_BIN
#error "Missing 0m.bin: add ui/home/0m.png..9m.png, colonm.png, AMm.png, PMm.png and run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_COLONM_BIN
#error "Missing colonm.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_AMM_BIN
#error "Missing AMm.bin: run gen_home_icons.py + prebuild.bat"
#endif

#ifndef UI_BUF_HOME_PMM_BIN
#error "Missing PMm.bin: run gen_home_icons.py + prebuild.bat"
#endif

static const u32 tbl_top_time_digit_addr[10] = {
    UI_BUF_HOME_0M_BIN, UI_BUF_HOME_1M_BIN, UI_BUF_HOME_2M_BIN, UI_BUF_HOME_3M_BIN,
    UI_BUF_HOME_4M_BIN, UI_BUF_HOME_5M_BIN, UI_BUF_HOME_6M_BIN, UI_BUF_HOME_7M_BIN,
    UI_BUF_HOME_8M_BIN, UI_BUF_HOME_9M_BIN,
};

static const u16 tbl_top_time_digit_len[10] = {
    UI_LEN_HOME_0M_BIN, UI_LEN_HOME_1M_BIN, UI_LEN_HOME_2M_BIN, UI_LEN_HOME_3M_BIN,
    UI_LEN_HOME_4M_BIN, UI_LEN_HOME_5M_BIN, UI_LEN_HOME_6M_BIN, UI_LEN_HOME_7M_BIN,
    UI_LEN_HOME_8M_BIN, UI_LEN_HOME_9M_BIN,
};

static const u16 tbl_top_time_digit_w[10] = {
    HOME_TOP_TIME_0M_W, HOME_TOP_TIME_1M_W, HOME_TOP_TIME_2M_W, HOME_TOP_TIME_3M_W,
    HOME_TOP_TIME_4M_W, HOME_TOP_TIME_5M_W, HOME_TOP_TIME_6M_W, HOME_TOP_TIME_7M_W,
    HOME_TOP_TIME_8M_W, HOME_TOP_TIME_9M_W,
};

static const u16 tbl_top_time_digit_h[10] = {
    HOME_TOP_TIME_0M_H, HOME_TOP_TIME_1M_H, HOME_TOP_TIME_2M_H, HOME_TOP_TIME_3M_H,
    HOME_TOP_TIME_4M_H, HOME_TOP_TIME_5M_H, HOME_TOP_TIME_6M_H, HOME_TOP_TIME_7M_H,
    HOME_TOP_TIME_8M_H, HOME_TOP_TIME_9M_H,
};

static bool home_top_time_gpu_flash_to_ram(u8 *ram, u16 buf_size, u32 flash_addr, u16 flash_len,
                                           compo_picturebox_t *pic)
{
    u16 need;

    if (ram == NULL || flash_len == 0 || flash_len > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    os_spiflash_read(ram, flash_addr, flash_len);
    if (!gui_set_ram_check(ram, __func__)) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    need = (u16)(8 + (u32)GET_LE16(&ram[4]) * GET_LE16(&ram[6]) * 2);
    if (need > flash_len || need > buf_size) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (pic != NULL) {
        compo_picturebox_set_ram(pic, ram);
        compo_picturebox_set_visible(pic, true);
    }
    return true;
}

static void home_top_time_parse(tm_t *tm, u8 *hour12, u8 *min, bool *is_pm)
{
    u8 hour = tm->hour;

    *is_pm = false;
    if (hour >= 12) {
        *is_pm = true;
        if (hour > 12) {
            hour -= 12;
        }
    }
    if (hour == 0) {
        hour = 12;
    }
    *hour12 = hour;
    *min = tm->min;
}

static bool home_top_time_load_digit(u8 slot, u8 digit, compo_picturebox_t *pic)
{
    if (digit > 9 || slot >= HOME_TOP_TIME_DIGIT_SLOTS) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    if (home_top_time_gpu_flash_to_ram(home_ui_shared_top_time_digit_ram[slot],
                                       HOME_TOP_TIME_DIGIT_RAM_MAX_SIZE,
                                       tbl_top_time_digit_addr[digit],
                                       tbl_top_time_digit_len[digit], pic)) {
        compo_picturebox_set_size(pic, tbl_top_time_digit_w[digit], tbl_top_time_digit_h[digit]);
        return true;
    }
    return false;
}

static void home_top_time_layout(home_top_time_ui_t *ui, u8 hour12, u8 min, bool is_pm)
{
    u8 h10 = (u8)(hour12 / 10);
    u8 h1 = (u8)(hour12 % 10);
    u8 m10 = (u8)(min / 10);
    u8 m1 = (u8)(min % 10);
    bool show_h10 = (hour12 >= 10);
    s16 x = HOME_TOP_TIME_X;
    u16 ampm_w = is_pm ? HOME_TOP_TIME_PMM_W : HOME_TOP_TIME_AMM_W;
    u16 ampm_h = is_pm ? HOME_TOP_TIME_PMM_H : HOME_TOP_TIME_AMM_H;

    if (show_h10) {
        s16 cx = x + (s16)(tbl_top_time_digit_w[h10] / 2);

        compo_picturebox_set_pos(ui->pic_h10, cx, HOME_TOP_TIME_Y);
        compo_picturebox_set_size(ui->pic_h10, tbl_top_time_digit_w[h10], tbl_top_time_digit_h[h10]);
        x += tbl_top_time_digit_w[h10] + HOME_TOP_TIME_ELEM_GAP;
    } else if (ui->pic_h10 != NULL) {
        compo_picturebox_set_visible(ui->pic_h10, false);
    }

    {
        s16 cx = x + (s16)(tbl_top_time_digit_w[h1] / 2);

        compo_picturebox_set_pos(ui->pic_h1, cx, HOME_TOP_TIME_Y);
        compo_picturebox_set_size(ui->pic_h1, tbl_top_time_digit_w[h1], tbl_top_time_digit_h[h1]);
        x += tbl_top_time_digit_w[h1] + HOME_TOP_TIME_ELEM_GAP;
    }

    {
        s16 cx = x + (s16)(HOME_TOP_TIME_COLONM_W / 2);

        compo_picturebox_set_pos(ui->pic_colon, cx, HOME_TOP_TIME_Y);
        compo_picturebox_set_size(ui->pic_colon, HOME_TOP_TIME_COLONM_W, HOME_TOP_TIME_COLONM_H);
        x += HOME_TOP_TIME_COLONM_W + HOME_TOP_TIME_ELEM_GAP;
    }

    {
        s16 cx = x + (s16)(tbl_top_time_digit_w[m10] / 2);

        compo_picturebox_set_pos(ui->pic_m10, cx, HOME_TOP_TIME_Y);
        compo_picturebox_set_size(ui->pic_m10, tbl_top_time_digit_w[m10], tbl_top_time_digit_h[m10]);
        x += tbl_top_time_digit_w[m10] + HOME_TOP_TIME_ELEM_GAP;
    }

    {
        s16 cx = x + (s16)(tbl_top_time_digit_w[m1] / 2);

        compo_picturebox_set_pos(ui->pic_m1, cx, HOME_TOP_TIME_Y);
        compo_picturebox_set_size(ui->pic_m1, tbl_top_time_digit_w[m1], tbl_top_time_digit_h[m1]);
        x += tbl_top_time_digit_w[m1] + HOME_TOP_TIME_AMPM_GAP;
    }

    {
        s16 cx = x + (s16)(ampm_w / 2);

        compo_picturebox_set_pos(ui->pic_ampm, cx, HOME_TOP_TIME_Y);
        compo_picturebox_set_size(ui->pic_ampm, ampm_w, ampm_h);
    }
}

void home_top_time_create(compo_form_t *frm, u32 placeholder, const u16 id_h10,
                          u16 id_h1, u16 id_colon, u16 id_m10, u16 id_m1, u16 id_ampm)
{
    compo_picturebox_t *pic;
    const u16 ids[6] = { id_h10, id_h1, id_colon, id_m10, id_m1, id_ampm };
    u8 i;

    for (i = 0; i < 6; i++) {
        pic = compo_picturebox_create(frm, placeholder);
        compo_setid(pic, ids[i]);
        compo_picturebox_set_visible(pic, false);
        compo_picturebox_set_pos(pic, HOME_TOP_TIME_X, HOME_TOP_TIME_Y);
    }
}

void home_top_time_bind(home_top_time_ui_t *ui, u16 id_h10, u16 id_h1, u16 id_colon,
                        u16 id_m10, u16 id_m1, u16 id_ampm)
{
    ui->pic_h10 = compo_getobj_byid(id_h10);
    ui->pic_h1 = compo_getobj_byid(id_h1);
    ui->pic_colon = compo_getobj_byid(id_colon);
    ui->pic_m10 = compo_getobj_byid(id_m10);
    ui->pic_m1 = compo_getobj_byid(id_m1);
    ui->pic_ampm = compo_getobj_byid(id_ampm);
    ui->last_key = 0xffff;
}

bool home_top_time_refresh(home_top_time_ui_t *ui, tm_t *tm)
{
    u8 hour12;
    u8 min;
    bool is_pm;
    u16 key;
    u8 h10;
    u8 h1;
    u8 m10;
    u8 m1;
    bool show_h10;

    if (ui == NULL || tm == NULL) {
        return false;
    }

    home_top_time_parse(tm, &hour12, &min, &is_pm);
    key = (u16)hour12 | ((u16)min << 8) | (is_pm ? 0x8000 : 0);
    if (ui->last_key == key) {
        return false;
    }

    home_gpu_wait_idle();
    ui->last_key = key;

    h10 = (u8)(hour12 / 10);
    h1 = (u8)(hour12 % 10);
    m10 = (u8)(min / 10);
    m1 = (u8)(min % 10);
    show_h10 = (hour12 >= 10);

    if (show_h10) {
        home_top_time_load_digit(0, h10, ui->pic_h10);
    } else if (ui->pic_h10 != NULL) {
        compo_picturebox_set_visible(ui->pic_h10, false);
    }

    home_top_time_load_digit(1, h1, ui->pic_h1);

    home_top_time_gpu_flash_to_ram(home_ui_shared_top_time_colon_ram, HOME_TOP_TIME_COLONM_RAM_SIZE,
                                   UI_BUF_HOME_COLONM_BIN, UI_LEN_HOME_COLONM_BIN, ui->pic_colon);
    if (ui->pic_colon != NULL) {
        compo_picturebox_set_size(ui->pic_colon, HOME_TOP_TIME_COLONM_W, HOME_TOP_TIME_COLONM_H);
    }

    home_top_time_load_digit(2, m10, ui->pic_m10);
    home_top_time_load_digit(3, m1, ui->pic_m1);

    if (is_pm) {
        home_top_time_gpu_flash_to_ram(home_ui_shared_top_time_ampm_ram, HOME_TOP_TIME_AMPM_RAM_MAX_SIZE,
                                       UI_BUF_HOME_PMM_BIN, UI_LEN_HOME_PMM_BIN, ui->pic_ampm);
        if (ui->pic_ampm != NULL) {
            compo_picturebox_set_size(ui->pic_ampm, HOME_TOP_TIME_PMM_W, HOME_TOP_TIME_PMM_H);
        }
    } else {
        home_top_time_gpu_flash_to_ram(home_ui_shared_top_time_ampm_ram, HOME_TOP_TIME_AMPM_RAM_MAX_SIZE,
                                       UI_BUF_HOME_AMM_BIN, UI_LEN_HOME_AMM_BIN, ui->pic_ampm);
        if (ui->pic_ampm != NULL) {
            compo_picturebox_set_size(ui->pic_ampm, HOME_TOP_TIME_AMM_W, HOME_TOP_TIME_AMM_H);
        }
    }

    home_top_time_layout(ui, hour12, min, is_pm);
    return true;
}

static bool home_top_time_load_digit_flash(u8 digit, compo_picturebox_t *pic)
{
    if (digit > 9 || pic == NULL) {
        if (pic != NULL) {
            compo_picturebox_set_visible(pic, false);
        }
        return false;
    }

    home_ui_pic_set_flash(pic, tbl_top_time_digit_addr[digit],
                          tbl_top_time_digit_w[digit], tbl_top_time_digit_h[digit]);
    return true;
}

static bool home_top_time_refresh_flash_impl(home_top_time_ui_t *ui, tm_t *tm)
{
    u8 hour12;
    u8 min;
    bool is_pm;
    u16 key;
    u8 h10;
    u8 h1;
    u8 m10;
    u8 m1;
    bool show_h10;

    if (ui == NULL || tm == NULL) {
        return false;
    }

#if ELUNCHBOX_PANEL_EN
    /* ELUNCHBOX 模式：设置 TE block 标志，保护整个刷新操作 */
    u8 was_blocked = elunchbox_te_block_flag;
    if (!was_blocked) {
        elunchbox_te_block_flag = 1;
    }
#endif

    home_top_time_parse(tm, &hour12, &min, &is_pm);
    key = (u16)hour12 | ((u16)min << 8) | (is_pm ? 0x8000 : 0);
    if (ui->last_key == key) {
#if ELUNCHBOX_PANEL_EN
        if (!was_blocked) {
            elunchbox_te_block_flag = 0;
        }
#endif
        return false;
    }

    ui->last_key = key;

    h10 = (u8)(hour12 / 10);
    h1 = (u8)(hour12 % 10);
    m10 = (u8)(min / 10);
    m1 = (u8)(min % 10);
    show_h10 = (hour12 >= 10);

    if (show_h10) {
        home_top_time_load_digit_flash(h10, ui->pic_h10);
    } else if (ui->pic_h10 != NULL) {
        compo_picturebox_set_visible(ui->pic_h10, false);
    }

    home_top_time_load_digit_flash(h1, ui->pic_h1);
    home_ui_pic_set_flash(ui->pic_colon, UI_BUF_HOME_COLONM_BIN,
                          HOME_TOP_TIME_COLONM_W, HOME_TOP_TIME_COLONM_H);
    home_top_time_load_digit_flash(m10, ui->pic_m10);
    home_top_time_load_digit_flash(m1, ui->pic_m1);

    if (is_pm) {
        home_ui_pic_set_flash(ui->pic_ampm, UI_BUF_HOME_PMM_BIN,
                              HOME_TOP_TIME_PMM_W, HOME_TOP_TIME_PMM_H);
    } else {
        home_ui_pic_set_flash(ui->pic_ampm, UI_BUF_HOME_AMM_BIN,
                              HOME_TOP_TIME_AMM_W, HOME_TOP_TIME_AMM_H);
    }

    home_top_time_layout(ui, hour12, min, is_pm);

#if ELUNCHBOX_PANEL_EN
    if (!was_blocked) {
        elunchbox_te_block_flag = 0;
    }
#endif
    return true;
}

bool home_top_time_refresh_flash(home_top_time_ui_t *ui, tm_t *tm)
{
    return home_top_time_refresh_flash_impl(ui, tm);
}

void home_top_time_gpu_detach_light(home_top_time_ui_t *ui)
{
    if (ui == NULL) {
        return;
    }
    home_ui_gpu_pic_detach_light(ui->pic_h10);
    home_ui_gpu_pic_detach_light(ui->pic_h1);
    home_ui_gpu_pic_detach_light(ui->pic_colon);
    home_ui_gpu_pic_detach_light(ui->pic_m10);
    home_ui_gpu_pic_detach_light(ui->pic_m1);
    home_ui_gpu_pic_detach_light(ui->pic_ampm);
    ui->last_key = 0xffff;
}

void home_top_time_gpu_detach(home_top_time_ui_t *ui)
{
    compo_picturebox_t *pics[6];
    u8 i;

    if (ui == NULL) {
        return;
    }

    pics[0] = ui->pic_h10;
    pics[1] = ui->pic_h1;
    pics[2] = ui->pic_colon;
    pics[3] = ui->pic_m10;
    pics[4] = ui->pic_m1;
    pics[5] = ui->pic_ampm;

    home_gpu_wait_idle();
    for (i = 0; i < 6; i++) {
        home_ui_gpu_pic_detach(pics[i]);
    }
    ui->last_key = 0xffff;
    home_gpu_wait_idle();
}
