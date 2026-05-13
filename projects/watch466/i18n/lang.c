#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

extern const char * const i18n_zh[];
extern const char * const i18n_en[];
const char * const *i18n;

void lang_select(int lang_id)
{
    switch (lang_id) {
    case LANG_ZH:
        i18n = i18n_zh;
        break;

    case LANG_EN:
        i18n = i18n_en;
        break;

    default:
        TRACE("Language Select Error!\n");
        halt(HALT_I18N_INIT);
        break;
    }
}
