#ifndef _FUNC_MANAGE_H
#define _FUNC_MANAGE_H

#define TASK_STACK_MAX                      8         //任务堆栈最大入栈个数
#define LATEST_TASK_MAX                     8         //最近任务最大记录个数

/**
 * @brief 获取任务图标资源地址
 * @param[in] func_sta : 任务id
 * @return 资源地址
 **/
u32 func_get_icon_addr(u8 func_sta);

/**
 * @brief 获取任务标题字符串序号
 * @param[in] func_sta : 任务id
 * @return 标题字符串序号
 **/
u32 func_get_str_idx(u8 func_sta);


///----------------------任务堆栈----------------------
/**
 * @brief 堆栈初始化
 **/
void task_stack_init(void);

/**
 * @brief 入栈
          目前首先入栈的是时钟表盘
 * @param[in] val : 入栈的值
 **/
void task_stack_push(u8 val);

/**
 * @brief 出栈
 * @return 出栈后，处于栈顶的值
 **/
u8 task_stack_pop(void);

/**
 * @brief 获取栈顶
 * @return 栈顶的值
 **/
u8 gui_get_stack_top(void);


///----------------------最近任务列表----------------------
/**
 * @brief 列表初始化
 **/
void latest_task_init(void);

/**
 * @brief 获取列表记录个数
 * @return 最近任务个数
 **/
u8 latest_task_count(void);

/**
 * @brief 插入最近任务到列表头部，尾部元素先入先出
          注意：idx=0为最新
 * @param[in] func_sta : 任务id
 **/
void latest_task_add(u8 func_sta);

/**
 * @brief 通过下标（0最新）删除列表中的任务
 * @param[in] idx : 任务在列表中的序号
 **/
void latest_task_del(u8 idx);

/**
 * @brief 通过下标（0最新）获取列表中的任务
 * @param[in] idx : 任务在列表中的序号
 * @return 任务id
 **/
u8 latest_task_get(u8 idx);

/**
 * @brief 最近任务列表中查找任务
 * @param[in] func_sta : 任务id
 * @return 任务在列表中的序号（没有则返回-1）
 **/
s8 latest_task_find(u8 func_sta);

#endif
