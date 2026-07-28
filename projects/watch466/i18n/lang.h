#ifndef _LANG_H
#define _LANG_H

//enum {
//    LANG_EN,
//    LANG_ZH,
//};

enum
{
    STR_HEAT,
    STR_MODE,
    STR_SETUP,
    STR_ORDER,
    STR_CHICKEN,
    STR_PASTA,
    STR_WARM,
    STR_SETUP_TIME,
    STR_LANGUAGE,
    STR_VER_INFO,
    STR_VER_INFO_TEXT,
    STR_HEAT_LAB,
    STR_TIME_LAB,
    STR_RESIDUE_LAB,
    STR_YUYUE_NAME,
    STR_CHICKEN_MODE,
    STR_PASTA_MODE,
    STR_WARM1,
    STR_WARM_LAB,
    STR_BTN_YES,
    STR_BTN_NO,
    STR_CONFIRM_LID_OPEN,
    STR_CONFIRM_CONTINUE_HEAT,

    
};

extern const char * const *i18n;

void lang_select(int lang_id);

#endif
