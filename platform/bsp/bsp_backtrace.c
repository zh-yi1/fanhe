#include "include.h"

/*****************************************************************************************************
 *      系统异常时线程堆栈信息打印
 *******************************************************************************************************/
#define OS_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))
#define DEADWORD                        0x23232323

extern u32 __comm_start, __comm_end;
extern u32 __code_start_stream, __code_end_all;


AT(.com_rodata.exception)
const char dump_str[] = "\n<<< task: %s, stack: %p--%p, sp: %p >>>\n";
AT(.com_rodata.exception)
const char backtrace_str[] = "riscv32-elf-addr2line -e app.rv32 -a -f ";
AT(.com_rodata.exception)
const char num_str[] = "%08x ";
AT(.com_rodata.exception)
const char ln_str[] = "\n";


AT(.com_text.exception)
os_err_t os_backtrace_dump_thread_stack(os_thread_t thread)
{
    uint32_t start_addr = (uint32_t)thread->sp;
    uint32_t end_addr = (uint32_t)thread->stack_addr + thread->stack_size - 28;

    printk(dump_str, thread->name, thread->stack_addr, thread->stack_addr + thread->stack_size, thread->sp);
    printk(backtrace_str);

    //inverse search
    for(int i = 0; i < thread->stack_size / 4; i++) {
        if ((end_addr - i * 4) < start_addr) {
            break;
        }
        uint32_t addr = *(uint32_t*)(end_addr - i * 4);
        bool is_valid = (((addr >= (uint32_t)(&__comm_start)) && (addr <= (uint32_t)(&__comm_end))) || ((addr >= (uint32_t)(&__code_start_stream))  && (addr <= (uint32_t)(&__code_end_all))));
        if(is_valid) {
            printk(num_str,  addr);
        }
    }

    printk(ln_str);
    printk(ln_str);

    for(int i = 0; i < thread->stack_size/4; i++) {
        if (((uint32_t)thread->sp + i * 4) > ((uint32_t)thread->stack_addr + thread->stack_size)) {
            break;
        }
        if(i % 8 == 0) {
            printk(ln_str);
        }
        uint32_t addr = *(uint32_t*)((uint32_t)thread->sp + i * 4);
        printk(num_str,  addr);
    }

    printk(ln_str);
    printk(ln_str);

    return OS_EOK;
}

#if SYS_EXCEPTION_MONITOR_EN

AT(.com_text.exception)
void exception_isr_callback(void)
{
    os_backtrace_dump_thread_stack(os_thread_self());
}

#endif

/*****************************************************************************************************
 *      CPU占用率统计
 *******************************************************************************************************/
#if CPU_USAGE_MONITOT_EN

#define CPU_TRACE_PERIOD            1000         //统计周期1000ms
#define THREAD_COUNT_MAX            16          //最大线程数量
#define THREAD_SCHEDULE_COUNT_MAX   4000        //统计周期内线程调度最大次数
#define MAIN_IDLE_BASE_CNT          102000      //统计周期系统空闲计数基准值(1000ms周期 - 102000，100ms周期8500)
#define rt_list_entry(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))
#define HW_TIMERX_USE               HW_TIMER1   //使用硬件定时器1
#define HW_TIMERX_SFR               TMR1CON
#define HW_TIMERX_CNT               TMR1CNT

typedef struct {
    os_thread_t thread;
    u32 usage;
}task_info_t;

static task_info_t thread_info_tbl[THREAD_SCHEDULE_COUNT_MAX]; //统计周期内线程切换最大次数，需要根据实际情况调整
static u32 thread_schedule_cnt = 0;
static u32 last_timer_cnt = 0;
static volatile u32 main_idle_count = 0;
static float per_cpu_usage = 0.0;
static bool cpu_monitor_en = false;
static u32 timer_sfr_bk[4];

AT(.com_rodata.task_str)
const char thread_shecdule_err_str[] = "[trace]thread_shecdule full\n";

AT(.com_rodata.task_str)
const char thread_info[] = "%s:%.2f%%\n";

AT(.com_rodata.task_str)
const char cpu_info[] = "cpu:%.2f%%\n";

AT(.com_rodata.task_str)
const char main_name_str[] = "mai";

AT(.com_rodata.task_str)
const char main_idle_str[] = "max:%d, idle:%d\n";

AT(.com_text.task)
void shecher_hook(os_thread_t from, os_thread_t to)
{
    if (bsp_system_is_sleep()) {
        if (cpu_monitor_en) {
            cpu_monitor_en = false;
        }
        return ;
    } else {
        if (false == cpu_monitor_en) {
            cpu_monitor_en = true;
            psfr_t timer_sfr = &HW_TIMERX_SFR;
            for (u8 i = 0; i < 4; i++) {
                timer_sfr[i] = timer_sfr_bk[i];
            }
        }
    }
    if (thread_schedule_cnt >= THREAD_SCHEDULE_COUNT_MAX) {
        printf(thread_shecdule_err_str);
        memset(thread_info_tbl, 0, sizeof(thread_info_tbl));
        thread_schedule_cnt = 0;
    }
    GLOBAL_INT_DISABLE();
    u32 cur_timer_cnt = HW_TIMERX_CNT;
    thread_info_tbl[thread_schedule_cnt].thread = from;
    thread_info_tbl[thread_schedule_cnt].usage = cur_timer_cnt < last_timer_cnt ? cur_timer_cnt : cur_timer_cnt - last_timer_cnt;
    last_timer_cnt = cur_timer_cnt;
    thread_schedule_cnt ++;
    GLOBAL_INT_RESTORE();
}

AT(.com_text.isr)
void hw_task_cal_timer(void)
{
    os_thread_t sys_pthd_tbl[THREAD_COUNT_MAX] = {0};
    u32 pthd_usage[THREAD_COUNT_MAX] = {0};
    u32 total_usage = 0;
    u8 pthd_count = 0;
    
    // 遍历线程列表
    os_thread_t thread = os_thread_self();      
    os_list_t *thread_list = &(thread->list);  
    bool head_read = false;
    for (os_list_t *node = thread_list; ; node = node->next) {
        if (node == thread_list) {             
            if (head_read == false) {
                head_read = true;
            } else {                            
                break;
            }
        }
        os_thread_t pthd = rt_list_entry(node, struct os_thread, list);
        if (pthd && pthd_count < THREAD_COUNT_MAX) {
            sys_pthd_tbl[pthd_count++] = pthd;
        }
    }
    
    // 计算CPU占用率 
    //printf(main_idle_str, MAIN_IDLE_BASE_CNT, main_idle_count);
    if (main_idle_count >= MAIN_IDLE_BASE_CNT) {
        per_cpu_usage = 0.0;
    } else {
        per_cpu_usage = 100 - (float)(main_idle_count) * 100 / MAIN_IDLE_BASE_CNT;
    }
    main_idle_count = 0;

    // 计算各个线程占有率
    for (u32 i = 0; i < thread_schedule_cnt; i++) {
        for (u8 j = 0; j < pthd_count; j++) {
            if (thread_info_tbl[i].thread == sys_pthd_tbl[j]) {
                total_usage += thread_info_tbl[i].usage;  
                pthd_usage[j] += thread_info_tbl[i].usage;
            }
        }
    }
    memset(thread_info_tbl, 0, sizeof(thread_info_tbl));
    thread_schedule_cnt = 0;
    for (u8 j = 0; j < pthd_count; j++) { 
        if (pthd_usage[j]) {
            float usage_rate = (float)(pthd_usage[j]) * 100 / (float)total_usage;
            //printf(thread_info, sys_pthd_tbl[j]->name, usage_rate);
            if (!strcmp(main_name_str, sys_pthd_tbl[j]->name)) {
                if (per_cpu_usage == 0.0) {
                    per_cpu_usage = 100.0 - usage_rate; 
                }
            }
        }
    }

    printf(cpu_info, per_cpu_usage);
    printf(ln_str);

    last_timer_cnt = HW_TIMERX_CNT; //reset timer cnt
}

void cpu_trace_monitor(void)
{
    main_idle_count ++;
}

float cpu_usage_rate_get(void)
{   
    return per_cpu_usage;
}

void cpu_trace_monitor_init(void)
{
    bsp_hw_timer_set(HW_TIMERX_USE, CPU_TRACE_PERIOD * 1000, hw_task_cal_timer); 
    os_scheduler_sethook(shecher_hook);
    cpu_monitor_en = true;

    psfr_t timer_sfr = &HW_TIMERX_SFR;
    for (u8 i = 0; i < 4; i++) {
        timer_sfr_bk[i] = timer_sfr[i];
    }
}

#endif //CPU_USAGE_MONITOT_EN