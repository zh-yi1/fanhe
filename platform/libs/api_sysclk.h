#ifndef _API_SYSCLK_H_
#define _API_SYSCLK_H_

enum {
    SYS_RC2M,
    SYS_24M,
    SYS_48M,
    SYS_96M,
    SYS_144M,
    SYS_192M,
};

enum {
    INDEX_VOICE,
    INDEX_DECODE,
    INDEX_STACK,
    INDEX_RES_PLAY,
    INDEX_DVP,
    INDEX_VIDEO,
    INDEX_GUI,
    INDEX_REC,
    INDEX_USER1,
    INDEX_USER2,
    INDEX_MAX_NB = 10,
};

uint8_t sys_clk_get(void);
void sys_clk_set_do(u32 sys_clk);                       //上电启动未启用OS时调用
bool sys_clk_set(uint8_t sys_clk);                      //main线程调用，一般给客户调用
bool sys_clk_req(uint8_t index, uint8_t sys_clk);       //其它任务调用，一般在库内调用
bool sys_clk_free(uint8_t index);                       //跟sys_clk_req成对使用
bool sys_clk_free_all(void);
void adda_clk_source_sel(u32 val);
u32 get_sysclk_nhz(void);
#endif
