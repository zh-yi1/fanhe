#include "include.h"
#include "asr.h"

#include "fst_graph.h"
#include "fst_words.h"

#if (ASR_SELECT == ASR_WS)

void asr_prefetch_init(void);

extern volatile u8 asr_prefetch_kisck;
void asr_result_deal(const char *result);

#define TRACE_EN                    1

#if TRACE_EN
#define TRACE(...)                  printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

u32 asr_alg_process(short *ptr)
{
    const char *text;
    float score;
    if (sys_cb.mp3_res_playing) return 0;
    ///asr recog
    // GPIOBSET = BIT(3);
    int ret = Wanson_ASR_Recog(ptr, 640, &text, &score);
    // GPIOBCLR = BIT(3);

#if ASR_PREFETCH_EN
    if (asr_prefetch_kisck) {
        asr_prefetch_kisck --;
    }
#endif

    if (ret) {
#if ASR_PREFETCH_EN
        asr_prefetch_kisck = 5;
#endif
        if (ret == 1) {
            TRACE("ASR Result: %s\n", text);
            asr_result_deal(text);
        }
    }

    return ret;
}

void asr_alg_stop(void)
{
    Wanson_ASR_Release();
}

void asr_alg_start(void)
{
    //测量IO初始化 PB3 PB5
    // GPIOBFEN &= ~0x28;
    // GPIOBDE |= 0x28;
    // GPIOBDIR &= ~0x28;

    if (Wanson_ASR_Init() < 0) {
        TRACE("Wanson_ASR_Init Failed!\n");
        while (1) WDT_CLR();
    }
    Wanson_ASR_Reset();
    TRACE("Wanson_ASR_Init OK!\n");
#if ASR_PREFETCH_EN
    asr_prefetch_init();
#endif
}

void asr_alg_init(void)
{
#if 0
    GPIOBDE |= BIT(1);
    GPIOBCLR = BIT(1);
    GPIOBDIR &= ~BIT(1);

    GPIOBDE |= BIT(2);
    GPIOBCLR = BIT(2);
    GPIOBDIR &= ~BIT(2);

    GPIOEDE |= BIT(4);
    GPIOECLR = BIT(4);
    GPIOEDIR &= ~BIT(4);
#endif

//    extern u8 tdnn_weight_1[0x3c00];
//    extern u8 tdnn_weight_2[0x8000];
//    extern u8 tdnn_weight_3[0x8000];
//    extern u8 tdnn_weight_4[0x8000];
//    extern u8 tdnn_weight_5[0x8000];
//    extern u8 tdnn_weight_6[0x4000];
//    extern u8 tdnn_weight_7[0xe000];
//
//    print_r(tdnn_weight_1, 0x3c00);
//    print_r(tdnn_weight_2, 0x8000);
//    print_r(tdnn_weight_3, 0x8000);
//    print_r(tdnn_weight_4, 0x8000);
//    WDT_CLR();
//    print_r(tdnn_weight_5, 0x8000);
//    print_r(tdnn_weight_6, 0x4000);
//    print_r(tdnn_weight_7, 0xe000);
}

#endif //ASR_SELECT