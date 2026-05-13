
#if 0
#ifndef __FREEOS_H__
#define __FREEOS_H__

/* FREE-OS error code definitions */
#define OS_EOK                          0               /**< There is no error */
#define OS_ERROR                        1               /**< A generic error happens */
#define OS_ETIMEOUT                     2               /**< Timed out */
#define OS_EFULL                        3               /**< The resource is full */
#define OS_EEMPTY                       4               /**< The resource is empty */
#define OS_ENOMEM                       5               /**< No memory */
#define OS_ENOSYS                       6               /**< No system */
#define OS_EBUSY                        7               /**< Busy */
#define OS_EIO                          8               /**< IO error */

#define OS_IPC_FLAG_FIFO                0x00            /**< FIFOed IPC. @ref IPC. */
#define OS_IPC_FLAG_PRIO                0x01            /**< PRIOed IPC. @ref IPC. */

#define OS_IPC_CMD_UNKNOWN              0x00            /**< unknown IPC command */
#define OS_IPC_CMD_RESET                0x01            /**< reset IPC object */

#define OS_WAITING_FOREVER              -1              /**< Block forever until get resource. */
#define OS_WAITING_NO                   0               /**< Non-block. */

#define OS_TIMER_FLAG_DEACTIVATED       0x0             /**< timer is deactive */
#define OS_TIMER_FLAG_ACTIVATED         0x1             /**< timer is active */
#define OS_TIMER_FLAG_ONE_SHOT          0x0             /**< one shot timer */
#define OS_TIMER_FLAG_PERIODIC          0x2             /**< periodic timer */

#define OS_TIMER_FLAG_HARD_TIMER        0x0             /**< hard timer,the timer's callback function will be called in tick isr. */
#define OS_TIMER_FLAG_SOFT_TIMER        0x4             /**< soft timer,the timer's callback function will be called in timer thread. */

#define OS_TIMER_CTRL_SET_TIME          0x0             /**< set timer control command */
#define OS_TIMER_CTRL_GET_TIME          0x1             /**< get timer control command */
#define OS_TIMER_CTRL_SET_ONESHOT       0x2             /**< change timer to one shot */
#define OS_TIMER_CTRL_SET_PERIODIC      0x3             /**< change timer to periodic */

#ifndef OS_TIMER_SKIP_LIST_LEVEL
#define OS_TIMER_SKIP_LIST_LEVEL          1
#endif

/* 1 or 3 */
#ifndef OS_TIMER_SKIP_LIST_MASK
#define OS_TIMER_SKIP_LIST_MASK         0x3
#endif

/* FREE-OS basic data type definitions */
typedef signed   char                   os_int8_t;      /**<  8bit integer type */
typedef signed   short                  os_int16_t;     /**< 16bit integer type */
typedef signed   long                   os_int32_t;     /**< 32bit integer type */
typedef unsigned char                   os_uint8_t;     /**<  8bit unsigned integer type */
typedef unsigned short                  os_uint16_t;    /**< 16bit unsigned integer type */
typedef unsigned long                   os_uint32_t;    /**< 32bit unsigned integer type */
typedef int                             os_bool_t;      /**< boolean type */

/* 32bit CPU */
typedef long                            os_base_t;      /**< Nbit CPU related date type */
typedef unsigned long                   os_ubase_t;     /**< Nbit unsigned CPU related data type */

typedef os_base_t                       os_err_t;       /**< Type for error number */
typedef os_uint32_t                     os_time_t;      /**< Type for time stamp */
typedef os_uint32_t                     os_tick_t;      /**< Type for tick count */
typedef os_base_t                       os_flag_t;      /**< Type for flags */
typedef os_ubase_t                      os_size_t;      /**< Type for size number */
typedef os_ubase_t                      os_dev_t;       /**< Type for device */
typedef os_base_t                       os_off_t;       /**< Type for offset */

#define OS_IPC_FLAG_FIFO                0x00            /**< FIFOed IPC. @ref IPC. */
#define OS_IPC_FLAG_PRIO                0x01            /**< PRIOed IPC. @ref IPC. */


#ifndef OS_TIMER_SKIP_LIST_LEVEL
#define OS_TIMER_SKIP_LIST_LEVEL          1
#endif

#define OS_NAME_MAX	                    4



struct os_list_node
{
    struct os_list_node *next;                          /**< point to next node. */
    struct os_list_node *prev;                          /**< point to prev node. */
};
typedef struct os_list_node os_list_t;                  /**< Type for lists. */


/**
 * Base structure of Kernel object
 */
struct os_object
{
    char       name[OS_NAME_MAX];                       /**< name of kernel object */
    os_uint8_t type;                                    /**< type of kernel object */
    os_uint8_t flag;                                    /**< flag of kernel object */

#ifdef OS_USING_MODULE
    void      *module_id;                               /**< id of application module */
#endif
    os_list_t  list;                                    /**< list node of kernel object */
};
typedef struct os_object *os_object_t;                  /**< Type for kernel objects. */


/**
 * timer structure
 */
struct os_timer
{
    struct os_object parent;                            /**< inherit from os_object */

    os_list_t        row[OS_TIMER_SKIP_LIST_LEVEL];

    void (*timeout_func)(void *parameter);              /**< timeout function */
    void            *parameter;                         /**< timeout function's parameter */

    os_tick_t        init_tick;                         /**< timer timeout tick */
    os_tick_t        timeout_tick;                      /**< timeout tick */
};
typedef struct os_timer *os_timer_t;


/**
 * Thread structure
 */
struct os_thread
{
    /* rt object */
    char        name[OS_NAME_MAX];                      /**< the name of thread */
    os_uint8_t  type;                                   /**< type of object */
    os_uint8_t  flags;                                  /**< thread's flags */

#ifdef OS_USING_MODULE
    void       *module_id;                              /**< id of application module */
#endif

    os_list_t   list;                                   /**< the object list */
    os_list_t   tlist;                                  /**< the thread list */

    /* stack point and entry */
    void       *sp;                                     /**< stack point */
    void       *entry;                                  /**< entry */
    void       *parameter;                              /**< parameter */
    void       *stack_addr;                             /**< stack address */
    os_uint16_t stack_size;                             /**< stack size */

    /* error code */
    os_err_t    error;                                  /**< error code */

    os_uint8_t  stat;                                   /**< thread stat */

    /* priority */
    os_uint8_t  current_priority;                       /**< current priority */
    os_uint8_t  init_priority;                          /**< initialized priority */
#if OS_THREAD_PRIORITY_MAX > 32
    os_uint8_t  number;
    os_uint8_t  high_mask;
#endif
    os_uint32_t number_mask;

#if defined(OS_USING_EVENT)
    /* thread event */
    os_uint32_t event_set;
    os_uint8_t  event_info;
#endif

    os_ubase_t  init_tick;                              /**< thread's initialized tick */
    os_ubase_t  remaining_tick;                         /**< remaining tick */

    struct os_timer thread_timer;                       /**< built-in thread timer */

    void (*cleanup)(struct os_thread *tid);             /**< cleanup function when thread exit */

//    os_uint32_t user_data;                              /**< private user data beyond this thread */
};
typedef struct os_thread *os_thread_t;



/**
 * Base structure of IPC object
 */
struct os_ipc_object
{
    struct os_object parent;                            /**< inherit from os_object */

    os_list_t        suspend_thread;                    /**< threads pended on this resource */
};

struct os_semaphore
{
    struct os_ipc_object parent;                        /**< inherit from ipc_object */

    os_uint16_t          value;                         /**< value of semaphore. */
};
typedef struct os_semaphore *os_sem_t;


struct os_mutex
{
    struct os_ipc_object parent;                        /**< inherit from ipc_object */

    os_uint16_t          value;                         /**< value of mutex */

    os_uint8_t           original_priority;             /**< priority of last thread hold the mutex */
    os_uint8_t           hold;                          /**< numbers of thread hold the mutex */

    struct os_thread    *owner;                         /**< current owner of mutex */
};
typedef struct os_mutex *os_mutex_t;


/**
 * message queue structure
 */
struct os_messagequeue
{
    struct os_ipc_object parent;                        /**< inherit from ipc_object */

    void                *msg_pool;                      /**< start address of message queue */

    os_uint16_t          msg_size;                      /**< message size of each message */
    os_uint16_t          max_msgs;                      /**< max number of messages */

    os_uint16_t          entry;                         /**< index of messages in the queue */

    void                *msg_queue_head;                /**< list head */
    void                *msg_queue_tail;                /**< list tail */
    void                *msg_queue_free;                /**< pointer indicated the free node of queue */
};
typedef struct os_messagequeue *os_mq_t;





//----------------------------------------------------------------------------------
//
//
typedef os_tick_t (*os_tick_get_func)(void);

typedef os_err_t (*os_thread_init_func)(struct os_thread *thread,
                        const char       *name,
                        void (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        os_uint32_t       stack_size,
                        os_uint8_t        priority,
                        os_uint32_t       tick);
typedef os_err_t (*os_thread_detach_func)(os_thread_t thread);
typedef os_thread_t (*os_thread_create_func)(const char *name,
                             void (*entry)(void *parameter),
                             void       *parameter,
                             os_uint32_t stack_size,
                             os_uint8_t  priority,
                             os_uint32_t tick);
typedef os_err_t (*os_thread_startup_func)(os_thread_t thread);
typedef os_err_t (*os_thread_suspend_func)(os_thread_t thread);
typedef os_err_t (*os_thread_resume_func)(os_thread_t thread);
typedef os_thread_t (*os_thread_self_func)(void);
typedef void (*os_thread_exit_func)(void);

typedef os_err_t (*os_sem_init_func)(os_sem_t sem, const char *name, os_uint32_t value, os_uint8_t  flag);
typedef os_err_t (*os_sem_detach_func)(os_sem_t sem);
typedef os_err_t (*os_sem_detach_func)(os_sem_t sem);
typedef os_sem_t (*os_sem_create_func)(const char *name, os_uint32_t value, os_uint8_t flag);
typedef os_err_t (*os_sem_delete_func)(os_sem_t sem);
typedef os_err_t (*os_sem_take_func)(os_sem_t sem, os_int32_t time);
typedef os_err_t (*os_sem_trytake_func)(os_sem_t sem);
typedef os_err_t (*os_sem_control_func)(os_sem_t sem, os_uint8_t cmd, void *arg);
typedef os_err_t (*os_sem_release_func)(os_sem_t sem);

typedef os_err_t (*os_mutex_init_func)(os_mutex_t mutex, const char *name, os_uint8_t flag);
typedef os_err_t (*os_mutex_detach_func)(os_mutex_t mutex);
typedef os_mutex_t (*os_mutex_create_func)(const char *name, os_uint8_t flag);
typedef os_err_t (*os_mutex_delete_func)(os_mutex_t mutex);
typedef os_err_t (*os_mutex_take_func)(os_mutex_t mutex, os_int32_t time);
typedef os_err_t (*os_mutex_release_func)(os_mutex_t mutex);


typedef os_err_t (*os_mq_init_func)(os_mq_t mq, const char *name, void *msgpool, os_size_t msg_size, os_size_t pool_size, os_uint8_t flag);
typedef os_mq_t (*os_mq_create_func)(const char *name, os_size_t msg_size, os_size_t max_msgs, os_uint8_t flag);
typedef os_err_t (*os_mq_delete_func)(os_mq_t mq);
typedef os_err_t (*os_mq_send_func)(os_mq_t mq, void *buffer, os_size_t size);
typedef os_err_t (*os_mq_recv_func)(os_mq_t mq, void *buffer, os_size_t size, os_int32_t timeout);
typedef os_err_t (*os_mq_detach_func)(os_mq_t mq);


void os_task_sleep(uint32_t ms);
uint32_t tick_per_second_config(void);

void os_system_tick_init(void);
os_tick_t os_tick_get(void);
void os_tick_set(os_tick_t tick);
void os_tick_increase(void);
os_tick_t os_tick_from_millisecond(os_uint32_t ms);

void os_system_timer_init(void);
void os_system_timer_thread_init(void  *stack_start, os_uint32_t stack_size, os_uint8_t priority, os_uint32_t tick);

void os_timer_init(os_timer_t  timer,
                   const char *name,
                   void (*timeout)(void *parameter),
                   void       *parameter,
                   os_tick_t   time,
                   os_uint8_t  flag);
os_err_t os_timer_detach(os_timer_t timer);
os_timer_t os_timer_create(const char *name,
                           void (*timeout)(void *parameter),
                           void       *parameter,
                           os_tick_t   time,
                           os_uint8_t  flag);
os_err_t os_timer_delete(os_timer_t timer);
os_err_t os_timer_start(os_timer_t timer);
os_err_t os_timer_stop(os_timer_t timer);
os_err_t os_timer_control(os_timer_t timer, os_uint8_t cmd, void *arg);

// os_err_t os_mq_init(os_mq_t mq, const char *name, void *msgpool, os_size_t msg_size, os_size_t pool_size, os_uint8_t flag);

os_tick_t os_timer_next_timeout_tick(void);
uint32_t os_tick_from_ms(uint32_t ms);
typedef void *(*os_malloc_func)(os_size_t size);
typedef void *(*os_free_func)(void *rmem);
typedef os_err_t (*os_sleep_func)(os_tick_t tick);
typedef os_base_t (*os_hw_interrupt_disable_func)(void);
typedef void (*os_hw_interrupt_enable_func)(os_base_t level);

/**
 * @brief initialize a list
 *
 * @param l list to be initialized
 */
inline void os_list_init(os_list_t *l)
{
    l->next = l->prev = l;
}

/**
 * @brief insert a node after a list
 *
 * @param l list to insert it
 * @param n new node to be inserted
 */
inline void os_list_insert_after(os_list_t *l, os_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

/**
 * @brief insert a node before a list
 *
 * @param n new node to be inserted
 * @param l list to insert it
 */
inline void os_list_insert_before(os_list_t *l, os_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * @brief remove node from list.
 * @param n the node to remove from the list.
 */
inline void os_list_remove(os_list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief tests whether a list is empty
 * @param l the list to test.
 */
inline int os_list_isempty(const os_list_t *l)
{
    return l->next == l;
}

/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define os_list_entry(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))

/**
 * os_list_for_each - iterate over a list
 * @pos:	the os_list_t * to use as a loop cursor.
 * @head:	the head for your list.
 */
#define os_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * os_list_for_each_safe - iterate over a list safe against removal of list entry
 * @param pos the os_list_t * to use as a loop cursor.
 * @param n another os_list_t * to use as temporary storage
 * @param head the head for your list.
 */
#define os_list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

// 跟芯片相关
#define     OS_FUNC_BASE                    0xfff90100
#define     OS_FUNC_BASE2                   (OS_FUNC_BASE + 0x40)
#define     OS_FUNC_BASE5                   (OS_FUNC_BASE + 0x100)
#define     OS_FUNC_BASE6                   (OS_FUNC_BASE + 0x140)
#define     OS_FUNC_BASE8                   (OS_FUNC_BASE + 0x1C0)

#define     os_hw_interrupt_disable         ((os_hw_interrupt_disable_func)(OS_FUNC_BASE2 + 24))
#define     os_hw_interrupt_enable          ((os_hw_interrupt_enable_func)(OS_FUNC_BASE2 + 28))

#define     os_thread_init                  ((os_thread_init_func)(OS_FUNC_BASE5 + 0))
#define     os_thread_detach                ((os_thread_detach_func)(OS_FUNC_BASE5 + 4))
#define     os_thread_create                ((os_thread_create_func)(OS_FUNC_BASE5 + 8))
#define     os_thread_suspend               ((os_thread_suspend_func)(OS_FUNC_BASE5 + 16))
#define     os_thread_resume                ((os_thread_resume_func)(OS_FUNC_BASE5 + 20))
#define     os_thread_self                  ((os_thread_self_func)(OS_FUNC_BASE5 + 28))
#define     os_thread_startup               ((os_thread_startup_func)(OS_FUNC_BASE5 + 36))

#define     os_sem_init                     ((os_sem_init_func)(OS_FUNC_BASE6 + 0))
#define     os_sem_detach                   ((os_sem_detach_func)(OS_FUNC_BASE6 + 4))
#define     os_sem_create                   ((os_sem_create_func)(OS_FUNC_BASE6 + 8))
#define     os_sem_delete                   ((os_sem_delete_func)(OS_FUNC_BASE6 + 12))
#define     os_sem_take                     ((os_sem_take_func)(OS_FUNC_BASE6 + 16))
#define     os_sem_release                  ((os_sem_release_func)(OS_FUNC_BASE6 + 20))
#define     os_sem_trytake                  ((os_sem_trytake_func)(OS_FUNC_BASE6 + 28))

#define     os_mutex_init                   ((os_mutex_init_func)(OS_FUNC_BASE6 + 32))
#define     os_mutex_detach                 ((os_mutex_detach_func)(OS_FUNC_BASE6 + 36))
#define     os_mutex_create                 ((os_mutex_create_func)(OS_FUNC_BASE6 + 40))
#define     os_mutex_delete                 ((os_mutex_delete_func)(OS_FUNC_BASE6 + 44))
#define     os_mutex_take                   ((os_mutex_take_func)(OS_FUNC_BASE6 + 48))
#define     os_mutex_release                ((os_mutex_release_func)(OS_FUNC_BASE6 + 52))

#define     os_mq_init                      ((os_mq_init_func)(OS_FUNC_BASE8 + 0))
#define     os_mq_detach                    ((os_mq_detach_func)(OS_FUNC_BASE8 + 4))
#define     os_mq_create                    ((os_mq_create_func)(OS_FUNC_BASE8 + 8))
#define     os_mq_delete                    ((os_mq_delete_func)(OS_FUNC_BASE8 + 12))
#define     os_mq_send                      ((os_mq_send_func)(OS_FUNC_BASE8 + 16))
#define     os_mq_recv                      ((os_mq_recv_func)(OS_FUNC_BASE8 + 20))
#endif
#else
#include "../../../rt_thread.h"
#endif
