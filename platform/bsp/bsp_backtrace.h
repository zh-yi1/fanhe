#ifndef __BSP_BACKTRACE_H
#define __BSP_BACKTRACE_H


#define CPU_USAGE_MONITOT_EN        0       //CPU占用率统计开关
#define SYS_EXCEPTION_MONITOR_EN    1       //系统异常时打印线程堆栈信息


//打印对应线程压栈的函数
os_err_t os_backtrace_dump_thread_stack(os_thread_t thread);

//CPU占用率统计初始化
void cpu_trace_monitor_init(void);

//CPU占用率统计监听
void cpu_trace_monitor(void);

//CPU占用率获取
float cpu_usage_rate_get(void);


#endif


