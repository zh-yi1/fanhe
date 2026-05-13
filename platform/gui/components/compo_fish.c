#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#include "fixmath.h"

#define TRIGO_DIV                       1000
#define FISH_BEND_AMP_OMIGA             300                                     //
#define FISH_BEND_AMP_OMIGA_STEP        FISH_BEND_AMP_OMIGA/6
#define FISH_TURN_ANGLE                 600
#define FISH_TURN_ANGLE_STEP            FISH_TURN_ANGLE/6
#define FISH_SWIM_FAST_TIME             40
#define FISH_SWIM_FAST_STEP             -10
#define FISH_SWIM_NORMAL_STEP           -2
#define FISH_FRIST_ROTATE               900
#define FISH_FRIST_POS                  GUI_SCREEN_CENTER_Y
#define FISH_TURN_CLICK_AREA            50
#define FISH_SWIM_RETARD_AREA           20
#define FISH_UPDATE_RATE                50
#define FISH_PATH_AMP                   15
#define FISH_PATH_ANGLE                 15
#define FISH_BODY2BODY_LEN              40

#define FISH_APPER_RAND_ROTATE          0


#define WORLD_LIGHT_X                   20
#define WORLD_LIGHT_Y                   15


//使用角度计算COS
static int cos_from_deg(float deg)
{
    float cos = fix16_to_float(fix16_cos(fix16_deg_to_rad(fix16_from_float(deg))));
    return (int)(cos*TRIGO_DIV);
}

static int sin_from_deg(float deg)
{
    float sin = fix16_to_float(fix16_sin(fix16_deg_to_rad(fix16_from_float(deg))));
    return (int)(sin*TRIGO_DIV);
}

static int tan_from_deg(float deg)
{
    float tan = fix16_to_float(fix16_tan(fix16_deg_to_rad(fix16_from_float(deg))));
    return (int)(tan*TRIGO_DIV);
}

static int cos_from_rad(float rad)
{
    float cos = fix16_to_float(fix16_cos(fix16_from_float(rad)));
    return (int)(cos*TRIGO_DIV);
}

static int sin_from_rad(float rad)
{
    float sin = fix16_to_float(fix16_sin(fix16_from_float(rad)));
    return (int)(sin*TRIGO_DIV);
}

//static int tan_from_rad(float rad)
//{
//    float tan = fix16_to_float(fix16_tan(fix16_from_float(rad)));
//    return (int)(tan*TRIGO_DIV);
//}

static int atan_from_rad(float rad)
{
    float deg = fix16_to_float(fix16_rad_to_deg(fix16_atan(fix16_from_float(rad))));
    return (int)(deg*TRIGO_DIV);
}

//static float sqrt_to_float(float value)
//{
//    float sqrtV = fix16_to_float(fix16_sqrt(fix16_from_float(value)));
//    return sqrtV;
//}

compo_fish_t* compo_fish_create(compo_form_t* frm, const u32 *res_body, const u32 *res_shadow, u8 res_cnt)
{

    compo_fish_t *fish = compo_create(frm, COMPO_TYPE_FINSH);
    if (res_cnt != FINSH_BODY_FIXED_CNT) {
        printf("%s [param]in res_cnt err\n");
        return NULL;
    }

    //创建组件page
//    widget_page_t* page = frm->page_body;
    widget_page_t* page = widget_page_create(frm->page_body);
    widget_page_set_client(page, 0, 0);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    fish->page = page;

    //创建鱼身体影子
    for (int i=0; i<res_cnt; i++) {
        if (res_shadow) {
            fish->shadow[i].img = widget_image_create(page, res_shadow[i]);
            fish->shadow[i].raw_res_area = gui_image_get_size(res_shadow[i]);
            fish->shadow[i].len = FISH_BODY2BODY_LEN;
        } else {
            fish->shadow[i].img = NULL;
        }
    }


    //创建鱼身体
    for (int i=0; i<res_cnt; i++) {
        fish->body[i].img = widget_image_create(page, res_body[i]);
        fish->body[i].raw_res_area = gui_image_get_size(res_body[i]);
        fish->body[i].len = FISH_BODY2BODY_LEN;
    }




    fish->offset.x = GUI_SCREEN_CENTER_X;
    fish->offset.y = GUI_SCREEN_CENTER_Y;

    fish->mcb.head_location = FISH_FRIST_POS;

    //设置默认鱼游动路径
    compo_fish_set_path(fish, FISH_PATH_AMP, FISH_PATH_ANGLE);

    //根据路劲更新鱼控制器位置
    compo_fish_mcb_location_update(fish);

    //渲染更新
    compo_fish_render_update(fish);

    //设置图片误差
    //图片大小的误差微调
    //由于路劲跟随直接取等间距的
    //所以图片就没有办法合并起来
    //直接把图片拉伸或压缩
    widget_set_size(fish->body[0].img, fish->body[0].raw_res_area.wid, fish->body[0].raw_res_area.hei);
    widget_set_size(fish->body[1].img, fish->body[1].raw_res_area.wid, fish->body[1].raw_res_area.hei+15);
    widget_set_size(fish->body[2].img, fish->body[2].raw_res_area.wid, fish->body[2].raw_res_area.hei+15);
    widget_set_size(fish->body[3].img, fish->body[3].raw_res_area.wid, fish->body[3].raw_res_area.hei+12);
    widget_set_size(fish->body[4].img, fish->body[4].raw_res_area.wid+5, fish->body[4].raw_res_area.hei+2);

    if (res_shadow) {
        widget_set_size(fish->shadow[0].img, fish->shadow[0].raw_res_area.wid, fish->shadow[0].raw_res_area.hei);
        widget_set_size(fish->shadow[1].img, fish->shadow[1].raw_res_area.wid, fish->shadow[1].raw_res_area.hei+5);
        widget_set_size(fish->shadow[2].img, fish->shadow[2].raw_res_area.wid, fish->shadow[2].raw_res_area.hei+10);
        widget_set_size(fish->shadow[3].img, fish->shadow[3].raw_res_area.wid, fish->shadow[3].raw_res_area.hei+13);
        widget_set_size(fish->shadow[4].img, fish->shadow[4].raw_res_area.wid+5, fish->shadow[4].raw_res_area.hei+2);
    }

    return fish;
}

void compo_fish_set_path(compo_fish_t* fish, float amp, float zita)
{
    if (fish == NULL) {
        return;
    }
    float slope = (float)tan_from_deg(zita); //计算斜率
    fish->mcb.swim_path.AMP = amp;
    if (amp == 0) {
        fish->mcb.swim_path.K = 0;
    } else {
        fish->mcb.swim_path.K = slope / amp /TRIGO_DIV;
    }

//    printe("%s [OUT] --> AMP[%f], K[%f], func[y= %f * sin(%f * x)]\n", __func__, fish->mcb.swim_path.AMP, (fish->mcb.swim_path.K), fish->mcb.swim_path.AMP, (fish->mcb.swim_path.K));
}

s16 compo_fish_get_total_len(compo_fish_t* fish)
{
    if (fish == NULL) {
        return 0;
    }
    s16 ret = 0;
    for (int i=0; i<FINSH_BODY_FIXED_CNT; i++) {
        ret += fish->body[i].len;
    }
    return ret;
}

void compo_fish_mcb_location_update(compo_fish_t* fish)
{
    if (fish == NULL) {
        return;
    }
    //使用鱼头部位置迭代计算整条鱼的位置
    float x, slope;


//    if (fish->mcb.head_location > GUI_SCREEN_CENTER_Y + compo_fish_get_total_len(fish)) {
//        fish->mcb.head_location = -GUI_SCREEN_CENTER_Y;
//    } else
    if (fish->mcb.head_location < - GUI_SCREEN_CENTER_Y - compo_fish_get_total_len(fish)) {
        fish->mcb.head_location = GUI_SCREEN_CENTER_Y;
#if FISH_APPER_RAND_ROTATE
            fish->mcb.rotate += get_random(FISH_APPER_RAND_ROTATE);
            fish->mcb.rotate = fish->mcb.rotate % 3600;
#endif // FISH_APPER_RAND_DELTA_ROTATE
    }

    fish->mcb.body_location[0].pos.x = fish->mcb.head_location;


    for (int i=0; i<FINSH_BODY_FIXED_CNT; i++) {
        if (i != 0) {     //获取下一个点迭代坐标
            fish->mcb.body_location[i].pos.x = fish->mcb.body_location[i-1].pos.x + fish->body[i-1].len;
        }
        x = (float)fish->mcb.body_location[i].pos.x;
        slope = fish->mcb.swim_path.AMP * fish->mcb.swim_path.K * cos_from_rad(fish->mcb.swim_path.K * x) / TRIGO_DIV;

        fish->mcb.body_location[i].pos.x = x;
        fish->mcb.body_location[i].pos.y = fish->mcb.swim_path.AMP * sin_from_rad(fish->mcb.swim_path.K * x) / TRIGO_DIV;
        fish->mcb.body_location[i].rotate = atan_from_rad(slope) / 100;

    }

    fish->mcb.flag_need_render = true;
}


void compo_fish_render_update(compo_fish_t* fish)
{
    if (fish == NULL) {
        return;
    }
    //开始渲染
//    if (!fish->mcb.flag_need_render) {
//        return;
//    }


    //平移渲染
//    for (int i=0; i<FINSH_BODY_FIXED_CNT; i++) {
//        fish->body[i].location.pos.x= fish->mcb.body_location[i].pos.x;
//        fish->body[i].location.pos.y= fish->mcb.body_location[i].pos.y;
//        fish->body[i].location.rotate= fish->mcb.body_location[i].rotate;
//    }

    //旋转渲染
    if (fish->mcb.rotate_center.x != fish->mcb.rotate_center_last.x ||
        fish->mcb.rotate_center.y != fish->mcb.rotate_center_last.y ||
        fish->mcb.rotate != fish->mcb.rotate_last) {

        int cos = cos_from_deg((fish->mcb.rotate - fish->mcb.rotate_last) / 10.);
        int sin = sin_from_deg((fish->mcb.rotate - fish->mcb.rotate_last) / 10.);
        fish->mcb.itera.x += ((fish->mcb.rotate_center.x * (TRIGO_DIV - cos)) + (fish->mcb.rotate_center.y * sin)) / TRIGO_DIV;
        fish->mcb.itera.y += ((fish->mcb.rotate_center.x * sin) + (fish->mcb.rotate_center.y * (TRIGO_DIV - cos))) / TRIGO_DIV;
        fish->mcb.rotate_center_last = fish->mcb.rotate_center;
        fish->mcb.rotate_last = fish->mcb.rotate;
    }

    for (int i=0; i<FINSH_BODY_FIXED_CNT; i++) {
        //平移渲染
        fish->body[i].location.pos.x= fish->mcb.body_location[i].pos.x;
        fish->body[i].location.pos.y= fish->mcb.body_location[i].pos.y;
        fish->body[i].location.rotate= fish->mcb.body_location[i].rotate;

        //旋转渲染
        int cos = cos_from_deg(fish->mcb.rotate / 10.);
        int sin = sin_from_deg(fish->mcb.rotate / 10.);
        int x = (fish->mcb.body_location[i].pos.x * cos - fish->mcb.body_location[i].pos.y * sin) / TRIGO_DIV + fish->mcb.itera.x;
        int y = (fish->mcb.body_location[i].pos.x * sin + fish->mcb.body_location[i].pos.y * cos) / TRIGO_DIV + fish->mcb.itera.y;
        fish->body[i].location.pos.x = x;
        fish->body[i].location.pos.y = y;

//            printf("%s [%d,%d,%d]\n",  __func__, fish->body[i].location.pos.x, fish->body[i].location.pos.y, fish->body[i].location.rotate);

        if (fish->shadow[i].img) {
            fish->shadow[i].location.pos.x = fish->body[i].location.pos.x + WORLD_LIGHT_Y;
            fish->shadow[i].location.pos.y = fish->body[i].location.pos.y + WORLD_LIGHT_X;
            fish->shadow[i].location.rotate = fish->body[i].location.rotate;
            widget_set_pos(fish->shadow[i].img, fish->shadow[i].location.pos.y + fish->offset.x, fish->shadow[i].location.pos.x + fish->offset.y);
            widget_image_set_rotation(fish->shadow[i].img, -(fish->shadow[i].location.rotate + fish->mcb.rotate));
        }

        //图片渲染
        widget_set_pos(fish->body[i].img, fish->body[i].location.pos.y + fish->offset.x, fish->body[i].location.pos.x + fish->offset.y);
        widget_image_set_rotation(fish->body[i].img, -(fish->body[i].location.rotate + fish->mcb.rotate));

    }



    //图片渲染
//    for(int i=0; i<FINSH_BODY_FIXED_CNT; i++) {
//        widget_set_pos(fish->body[i].img, fish->body[i].location.pos.y + fish->offset.x, fish->body[i].location.pos.x + fish->offset.y);
//        widget_image_set_rotation(fish->body[i].img, -(fish->body[i].location.rotate + fish->mcb.rotate));
//    }

    //渲染结束
    fish->mcb.flag_need_render = false;
}

void compo_fish_set_visable(compo_fish_t* fish, bool visable)
{
    if (fish == NULL) {
        return;
    }
    widget_set_visible(fish->page, visable);
}

bool compo_fish_get_visable(compo_fish_t* fish)
{
    if (fish == NULL) {
        return false;
    }
    return widget_get_visble(fish->page);
}

void compo_fish_mcb_message(compo_fish_t* fish, point_t pt, u8 sta)
{
    if (fish == NULL || compo_fish_get_visable(fish) == false) {
        return;
    }
    switch (sta) {
    case COMPO_FINSH_MCB_CLICK: {
        if ((abs(pt.x - (widget_get_absolute(fish->body[0].img).x + widget_get_absolute(fish->body[1].img).x) / 2) * 2) < (fish->body[0].raw_res_area.hei + fish->body[1].raw_res_area.hei) &&
            (abs(pt.y - (widget_get_absolute(fish->body[0].img).y + widget_get_absolute(fish->body[1].img).y) / 2) * 2) < (fish->body[0].raw_res_area.hei + fish->body[1].raw_res_area.hei)) {
            fish->mcb.flag_click = COMPO_FINSH_CLICK_HEAD;
        } else if ((abs(pt.x - (widget_get_absolute(fish->body[2].img).x + widget_get_absolute(fish->body[3].img).x) / 2) * 2) < (fish->body[2].raw_res_area.hei + fish->body[3].raw_res_area.hei) &&
            (abs(pt.y - (widget_get_absolute(fish->body[2].img).y + widget_get_absolute(fish->body[3].img).y) / 2) * 2) < (fish->body[2].raw_res_area.hei + fish->body[3].raw_res_area.hei)) {
            fish->mcb.flag_click = COMPO_FINSH_CLICK_TAIL;
        }
    } break;

    case COMPO_FINSH_MCB_NONE:
        break;
    }
}

void compo_fish_set_swim_delta_x2path(compo_fish_t* fish, s16 delta)
{
    if (fish == NULL) {
        return;
    }
    fish->mcb.head_location += delta;
//    printf("head =%d\n", fish->mcb.head_location);
}

void compo_fish_bend_update(compo_fish_t* fish, bool lr)
{
    if (fish == NULL) {
        return;
    }
    if (!lr) {
        //向左弯曲
        fish->mcb.body_location[1].rotate = fish->mcb.body_location[2].rotate + fish->mcb.turn_omiga;
        fish->mcb.body_location[1].pos.x  = fish->mcb.body_location[2].pos.x - fish->body[2].len * cos_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[2].rotate)/10.) / TRIGO_DIV;
        fish->mcb.body_location[1].pos.y  = fish->mcb.body_location[2].pos.y - fish->body[2].len * sin_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[2].rotate)/10.) / TRIGO_DIV;

        fish->mcb.body_location[0].rotate = fish->mcb.body_location[1].rotate + fish->mcb.turn_omiga;
        fish->mcb.body_location[0].pos.x  = fish->mcb.body_location[1].pos.x - fish->body[1].len * cos_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[1].rotate)/10.) / TRIGO_DIV;
        fish->mcb.body_location[0].pos.y  = fish->mcb.body_location[1].pos.y - fish->body[1].len * sin_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[1].rotate)/10.) / TRIGO_DIV;

        fish->mcb.body_location[3].rotate = fish->mcb.body_location[2].rotate - fish->mcb.turn_omiga;
        fish->mcb.body_location[3].pos.x  = fish->mcb.body_location[2].pos.x + fish->body[2].len * cos_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[2].rotate)/10.) / TRIGO_DIV;
        fish->mcb.body_location[3].pos.y  = fish->mcb.body_location[2].pos.y - fish->body[2].len * sin_from_deg((fish->mcb.turn_omiga/2-fish->mcb.body_location[2].rotate)/10.) / TRIGO_DIV;

        fish->mcb.body_location[4].rotate = fish->mcb.body_location[3].rotate - fish->mcb.turn_omiga;
        fish->mcb.body_location[4].pos.x  = fish->mcb.body_location[3].pos.x + fish->body[3].len * cos_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[3].rotate)/10.) / TRIGO_DIV;
        fish->mcb.body_location[4].pos.y  = fish->mcb.body_location[3].pos.y - fish->body[3].len * sin_from_deg((fish->mcb.turn_omiga/2-fish->mcb.body_location[3].rotate)/10.) / TRIGO_DIV;
    } else {
        //向右转动
        fish->mcb.body_location[1].rotate = fish->mcb.body_location[2].rotate - fish->mcb.turn_omiga;
        fish->mcb.body_location[1].pos.x  = fish->mcb.body_location[2].pos.x - fish->body[2].len * cos_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[2].rotate)/10.) / TRIGO_DIV;
        fish->mcb.body_location[1].pos.y  = fish->mcb.body_location[2].pos.y + fish->body[2].len * sin_from_deg((fish->mcb.turn_omiga/2-fish->mcb.body_location[2].rotate)/10.) / TRIGO_DIV;

        fish->mcb.body_location[0].rotate = fish->mcb.body_location[1].rotate - fish->mcb.turn_omiga;
        fish->mcb.body_location[0].pos.x  = fish->mcb.body_location[1].pos.x - fish->body[1].len * cos_from_deg((fish->mcb.turn_omiga/2-fish->mcb.body_location[1].rotate)/10.) / TRIGO_DIV;
        fish->mcb.body_location[0].pos.y  = fish->mcb.body_location[1].pos.y + fish->body[1].len * sin_from_deg((fish->mcb.turn_omiga/2-fish->mcb.body_location[1].rotate)/10.) / TRIGO_DIV;
//
        fish->mcb.body_location[3].rotate = fish->mcb.body_location[2].rotate + fish->mcb.turn_omiga;
        fish->mcb.body_location[3].pos.x  = fish->mcb.body_location[2].pos.x + fish->body[2].len * cos_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[2].rotate)/10.) / TRIGO_DIV;
        fish->mcb.body_location[3].pos.y  = fish->mcb.body_location[2].pos.y + fish->body[2].len * sin_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[2].rotate)/10.) / TRIGO_DIV;
//
        fish->mcb.body_location[4].rotate = fish->mcb.body_location[3].rotate + fish->mcb.turn_omiga;
        fish->mcb.body_location[4].pos.x  = fish->mcb.body_location[3].pos.x + fish->body[3].len * cos_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[3].rotate)/10.) / TRIGO_DIV;
        fish->mcb.body_location[4].pos.y  = fish->mcb.body_location[3].pos.y + fish->body[3].len * sin_from_deg((fish->mcb.turn_omiga/2+fish->mcb.body_location[3].rotate)/10.) / TRIGO_DIV;
    }
}

void compo_fish_set_frist_rotate(compo_fish_t* fish, s16 rotate)
{
    fish->mcb.frist_rotate = rotate;
}

void compo_fish_set_frist_rotate_rand(compo_fish_t* fish, bool yn)
{
    fish->mcb.flag_rotate_rand = yn;
}

void compo_fish_mcb_process(compo_fish_t* fish)
{
    if (fish == NULL || compo_fish_get_visable(fish) == false) {
        return;
    }

    memset(fish->mcb.body_location, 0, sizeof(fish->mcb.body_location));

    if (tick_check_expire(fish->mcb.ticks, FISH_UPDATE_RATE)) {
        fish->mcb.ticks = tick_get();

        //更新位置
        compo_fish_mcb_location_update(fish);

        switch (fish->mcb.swim_sta) {
        case COMPO_FINSH_MCB_SWIM_NONE: {
            fish->mcb.ticks_cnt = 0;
            fish->mcb.turn_sta = 0;
            fish->mcb.turn_omiga = 0;
            fish->mcb.turning = false;
            fish->mcb.head_location = FISH_FRIST_POS;
            if (fish->mcb.flag_rotate_rand) {
                fish->mcb.rotate = get_random(3600);//FISH_FRIST_ROTATE
            } else {
                fish->mcb.rotate = fish->mcb.frist_rotate;
            }
            fish->mcb.delta_rotate = fish->mcb.rotate;
            fish->mcb.swim_sta = COMPO_FINSH_MCB_SWIM_NORMAL;
        } break;

        case COMPO_FINSH_MCB_SWIM_NORMAL: {
            if (fish->mcb.flag_click == COMPO_FINSH_CLICK_HEAD) {
                fish->mcb.delta_rotate = fish->mcb.rotate;
                fish->mcb.swim_sta = COMPO_FINSH_MCB_SWIM_TURN;
                fish->mcb.flag_click = COMPO_FINSH_CLICK_NONE;
            } else if (fish->mcb.flag_click == COMPO_FINSH_CLICK_TAIL) {
                fish->mcb.delta_rotate = fish->mcb.rotate;
                fish->mcb.swim_sta = COMPO_FINSH_MCB_SWIM_FAST_TIME;
                fish->mcb.flag_click = COMPO_FINSH_CLICK_NONE;
            } else {
                compo_fish_set_swim_delta_x2path(fish, FISH_SWIM_NORMAL_STEP);
            }
        } break;

        case COMPO_FINSH_MCB_SWIM_FAST: {
            fish->mcb.flag_click = COMPO_FINSH_CLICK_NONE;
            if (abs(fish->body[2].location.pos.x) < FISH_SWIM_RETARD_AREA && abs(fish->body[2].location.pos.y) < FISH_SWIM_RETARD_AREA) {
                fish->mcb.swim_sta = COMPO_FINSH_MCB_SWIM_NORMAL;
            } else {
                compo_fish_set_swim_delta_x2path(fish, FISH_SWIM_FAST_STEP);
            }
        } break;

        case COMPO_FINSH_MCB_SWIM_TURN: {
            if (fish->mcb.turning == false && (abs(fish->body[2].location.pos.x) > FISH_TURN_CLICK_AREA || abs(fish->body[2].location.pos.y) > FISH_TURN_CLICK_AREA)) {
                fish->mcb.swim_sta = COMPO_FINSH_MCB_SWIM_FAST_TIME;
                break;
            }
            fish->mcb.turning = true;
            fish->mcb.flag_click = COMPO_FINSH_CLICK_NONE;
            fish->mcb.rotate_center = fish->body[2].location.pos;
            if (fish->mcb.turn_sta == 0) {
                fish->mcb.turn_omiga += FISH_BEND_AMP_OMIGA_STEP;
                if (fish->mcb.turn_omiga >= FISH_BEND_AMP_OMIGA) {
                    fish->mcb.turn_sta = 1;
                }
            } else if (fish->mcb.turn_sta == 1) {
                if (fish->mcb.turn_omiga <= 0) {
                    fish->mcb.turn_omiga = 0;
                } else {
                    fish->mcb.turn_omiga -= FISH_BEND_AMP_OMIGA_STEP;
                }
            }

            if (abs(fish->mcb.rotate - fish->mcb.delta_rotate) >= FISH_TURN_ANGLE) {
                fish->mcb.delta_rotate = fish->mcb.rotate;
                fish->mcb.swim_sta = COMPO_FINSH_MCB_SWIM_FAST_TIME;
                fish->mcb.turn_sta = 0;
                fish->mcb.itera.x = 0;
                fish->mcb.itera.y = 0;
                fish->mcb.turning = false;
            } else {
                if (fish->mcb.body_location[0].rotate > 0) {
                    fish->mcb.rotate += FISH_TURN_ANGLE_STEP;
                } else {
                    fish->mcb.rotate -= FISH_TURN_ANGLE_STEP;
                }
//                fish->mcb.itera.x = 0;
//                fish->mcb.itera.y = 0;
                fish->mcb.rotate = fish->mcb.rotate % 3600;
            }

            if (fish->mcb.body_location[0].rotate > 0) {
                compo_fish_bend_update(fish, false);
            } else {
                compo_fish_bend_update(fish, true);
            }
        } break;

        case COMPO_FINSH_MCB_SWIM_FAST_TIME: {
            fish->mcb.ticks_cnt++;
            if (fish->mcb.ticks_cnt > FISH_SWIM_FAST_TIME) {
                fish->mcb.swim_sta = COMPO_FINSH_MCB_SWIM_FAST;
                fish->mcb.ticks_cnt = 0;
            }
            compo_fish_set_swim_delta_x2path(fish, FISH_SWIM_FAST_STEP);
        } break;

        default:
            break;

        }

        compo_fish_render_update(fish);
    }
}

