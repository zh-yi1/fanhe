#ifndef __API_RT_THREAD__
#define __API_RT_THREAD__

#ifdef API_MAXROM

/**
 * @brief 初始化线程
 * 此函数将初始化一个线程，通常用于初始化一个静态线程对象。
 * @param[in] thread	线程句柄。线程句柄由用户提供出来，并指向对应的线程控制块内存地址。
 * @param[in] name	线程的名称；线程名称的最大长度由 rtconfig.h 中定义的 RT_NAME_MAX 宏指定，多余部分会被自动截掉。
 * @param[in] entry	线程的入口函数
 * @param[in] parameter	入口函数的传入参数
 * @param[in] stack_start	线程堆栈的起始地址
 * @param[in] stack_size	线程栈大小，单位是字节。在大多数系统中需要做栈空间地址对齐（例如 ARM 体系结构中需要向 4 字节地址对齐）。
 * @param[in] priority	线程的优先级。从 15～ 29可以用，其他的优先级被用了，数值越小优先级越高，0 代表最高优先级。
 * @param[in] tick	线程的时间片大小。当系统中存在相同优先级线程时，这个参数指定线程一次调度能够运行的最大时间长度。
 *
 * @return  成功返回 RT_EOK,
            失败则返回 -RT_ERROR
 **/
os_err_t os_thread_init(struct os_thread *thread,
                        const char       *name,
                        void (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        os_uint32_t       stack_size,
                        os_uint8_t        priority,     //15到29可以用
                        os_uint32_t       tick);

                        /**
 * @brief 初始化线程
 * 此函数将初始化一个线程，线程堆栈在heap里自动分配
 * @param[in] name	线程的名称；线程名称的最大长度由 rtconfig.h 中定义的 RT_NAME_MAX 宏指定，多余部分会被自动截掉。
 * @param[in] entry	线程的入口函数
 * @param[in] parameter	入口函数的传入参数
 * @param[in] stack_size	线程栈大小，单位是字节。在大多数系统中需要做栈空间地址对齐（例如 ARM 体系结构中需要向 4 字节地址对齐）。
 * @param[in] priority	线程的优先级。从 15～ 29可以用，其他的优先级被用了，数值越小优先级越高，0 代表最高优先级。
 * @param[in] tick	线程的时间片大小。当系统中存在相同优先级线程时，这个参数指定线程一次调度能够运行的最大时间长度。
 *
 * @return  成功返回 线程handld,
            失败则返回 NULL
 **/
os_thread_t os_thread_create(const char *name,
                             void (*entry)(void *parameter),
                             void       *parameter,
                             os_uint32_t stack_size,
                             os_uint8_t  priority,
                             os_uint32_t tick);

/**
 * @brief 启动线程
 * 此函数将启动一个线程并将其放入系统就绪队列
 * @param[in] thread	被要被启动的线程句柄
 * @param[in] enable
 *
 * @return  成功返回 RT_EOK
            失败则返回 -RT_ERROR
 **/
os_err_t os_thread_startup(os_thread_t thread);

/**
 * @brief 挂起线程
 * 该函数将挂起指定的线程。
 * @param[in] thread	要被挂起的线程
 * @param[in] enable
 *
 * @return  成功返回 RT_EOK
            失败则返回 -RT_ERROR
   注: 如果挂起的是当前线程，该函数运行之后必须调用rt_schedule()函数。 用户只需要了解该接口的作用，不推荐使用该接口。
 **/
os_err_t os_thread_suspend(os_thread_t thread);

/**
 * @brief 使线程恢复运行
 * 线程恢复就是让挂起的线程重新进入就绪状态，并将线程放入系统的就绪队列中；如果被恢复线程在所有就绪态线程中，位于最高优先级链表的第一位，那么系统
 * 做上下文切換
 * @param[in] thread	将要被恢复的线程
 *
 * @return  成功返回 RT_EOK
            失败则返回 -RT_ERROR
 **/
os_err_t os_thread_resume(os_thread_t thread);

/**
 * @brief 获得当前线程
 * 该函数将返回当前线程的线程对象句柄
 * @param[in] thread	将要被恢复的线程
 *
 * @return  成功返回 当前线程对象句柄
            失败则返回 RT_NULL
 **/
os_thread_t os_thread_self(void);


/**
 * @brief 初始化信号量
 * 该函数将初始化信号量并将其置于内核管理器的控制之下。
 * @param[in] sem	指定的信号量对象句柄
 * @param[in] name	信号量的名称
 * @param[in] value	信号量初始化时的值
 * @param[in] flag	信号量的标志位
 *
 * @return  成功返回 RT_EOK
 **/
os_err_t os_sem_init(os_sem_t sem, const char *name, os_uint32_t value, os_uint8_t  flag);

/**
 * @brief 初始化信号量
 * 该函数将初始化信号量并将其置于内核管理器的控制之下。
 * @param[in] name	信号量的名称
 * @param[in] value	信号量初始化时的值
 * @param[in] flag	信号量的标志位
 *
 * @return  成功返回 RT_EOK
 **/
os_sem_t os_sem_create(const char *name, os_uint32_t value, os_uint8_t flag);

/**
 * @brief 删除信号量
 * 系统不再使用信号量时，可通过删除信号量以释放系统资源，适用于动态创建的信号量。 调用该函数将删除一个信号量对象，并释放其占用的内存空间。
 * @param[in] sem	指定的信号量对象句柄
 * @return  成功返回 RT_EOK
 **/
os_err_t os_sem_delete(os_sem_t sem);

/**
 * @brief 获取信号量
 * 线程通过调用该函数获取信号量来获得信号量资源实例，当信号量值大于零时，线程将获得信号量， 并且相应的信号量值会减 1；
 * 如果信号量的值等于零，那么说明当前信号量资源实例不可用，申请 该信号量的线程将根据 time 参数的情况选择直接返回、或挂起等待一段时间、或永久等待，
 * 直到 其他线程或中断释放该信号量。

 * @param[in] sem	指定的信号量对象句柄
 * @param[in] time	指定的等待时间，单位是操作系统时钟节拍（OS Tick）
 * @return  成功获得信号量；-RT_ETIMEOUT 超时依然未获得信号量；-RT_ERROR 其他错误
 **/
os_err_t os_sem_take(os_sem_t sem, os_int32_t time);

/**
 * @brief 无等待获取信号量
 * 当用户不想在申请的信号量上挂起线程进行等待时，可以调用该函数以无等待方式获取信号量
 * @param[in] sem	指定的信号量对象句柄
 * @return  RT_EOK 成功获得信号量；-RT_ETIMEOUT 获取失败
 **/
os_err_t os_sem_trytake(os_sem_t sem);

/**
 * @brief 释放信号量
 * 该函数将释放一个信号量，当信号量的值等于零时，并且有线程等待这个信号量时，
 * 释放信号量将唤醒等待在该信号量线程队列中的第一个线程，由它获取信号量；否则 将把信号量的值加一。
 * @param[in] sem	指定的信号量对象句柄
 * @return  RT_EOK 成功释放信号量
 **/
os_err_t os_sem_release(os_sem_t sem);


/**
 * @brief 初始化互斥量
 * 该函数将初始化互斥锁并将其置于内核管理器的控制之下
 * @param[in] mutex	互斥量对象的句柄
 * @param[in] name	互斥量的名称
 * @param[in] flag	互斥量的标志位
 * @return  RT_EOK 初始化成功
 **/
os_err_t os_mutex_init(os_mutex_t mutex, const char *name, os_uint8_t flag);

/**
 * @brief 初始化互斥量
 * 该函数将初始化互斥锁并将其置于内核管理器的控制之下
 * @param[in] name	互斥量的名称
 * @param[in] flag	互斥量的标志位
 * @return  RT_EOK 初始化成功
 **/
os_mutex_t os_mutex_create(const char *name, os_uint8_t flag);

/**
 * @brief 删除互斥量
 * 当系统不再使用互斥量时，通过调用该函数删除互斥量以释放系统资源，适用于动态创建的互斥量。
 * @param[in] mutex	互斥量对象的句柄
 * @return  RT_EOK 删除成功
 **/
os_err_t os_mutex_delete(os_mutex_t mutex);

/**
 * @brief 获取互斥量
 * 该函数将获取一个互斥量，如果互斥量没有被其他线程控制，那么申请该互斥量的线程将成功获得该互斥量。
   如果互斥量已经被当前线程线程控制，则该互斥量的持有计数加1，
   当前线程也不会挂起等待。如果互斥量 已经被其他线程占有，则当前线程在该互斥量上挂起等待，直到其他线程释放它或者等待时间超过指定的 超时时间。
 * @param[in] mutex	互斥量对象的句柄
 * @param[in] time	指定的等待时间
 * @return  RT_EOK 成功获得互斥量；-RT_ETIMEOUT 超时；-RT_ERROR 获取失败
 **/
os_err_t os_mutex_take(os_mutex_t mutex, os_int32_t time);

/**
 * @brief 释放互斥量
 * 使用该函数接口时，只有已经拥有互斥量控制权的线程才能释放它，每释放一次该互斥量，它的持有计数就减 1.
   当该互斥量的持有计数为零时（即持有线程已经释放所有的持有操作）， 它变为可用，等待在该信号量上的线程将被唤醒。
   如果线程的运行优先级被互斥量提升， 那么当互斥量被释放后，线程恢复为持有互斥量前的优先级。
 * @param[in] mutex	互斥量对象的句柄
 * @return  RT_EOK 成功
 **/
os_err_t os_mutex_release(os_mutex_t mutex);


/**
 * @brief 初始化消息队列
 * 该函数将初始化消息队列并将其置于内核管理器的控制之下。 1.
 * @param[in] mq	消息队列对象句柄
 * @param[in] name	消息队列的名称
 * @param[in] msgpool	用于存放消息的缓冲区指针
 * @param[in] msg_size	消息队列中一条消息的最大长度，单位字
 * @param[in] pool_size	存放消息的缓冲区大小
 * @param[in] 消息队列采用的等待方式，它可以取值：OS_IPC_FLAG_FIFO或OS_IPC_FLAG_PRIO
 * @return  RT_EOK 成功
 **/
os_err_t os_mq_init(os_mq_t mq, const char *name, void *msgpool, os_size_t msg_size, os_size_t pool_size, os_uint8_t flag);

/**
 * @brief 初始化消息队列
 * 该函数将初始化消息队列并将其置于内核管理器的控制之下。 1.
 * @param[in] mq	消息队列对象句柄
 * @param[in] name	消息队列的名称
 * @param[in] msg_size	消息队列中一条消息的最大长度，单位字
 * @param[in] 消息队列采用的等待方式，它可以取值：OS_IPC_FLAG_FIFO或OS_IPC_FLAG_PRIO
 * @return  RT_EOK 成功
 **/
os_mq_t os_mq_create(const char *name, os_size_t msg_size, os_size_t max_msgs, os_uint8_t flag);

/**
 * @brief 删除消息队列
 *当消息队列不再被使用时，应该调用该函数接口删除它以释放系统资源，一旦操作完成， 消息队列将被永久性地删除。
  删除消息队列时，如果有线程被挂起在该消息队列等待队列上， 则内核先唤醒挂起在该消息等待队列上的所有线程（返回值是 -RT_ERROR），
  然后再释放 消息队列使用的内存，最后删除消息队列对象。
 * @param[in] mq	消息队列对象句柄
 * @return  RT_EOK 成功
 **/
os_err_t os_mq_delete(os_mq_t mq);

/**
 * @brief 发送消息
 *线程或者中断服务程序都可以调用该函数给消息队列发送消息。当发送消息时， 消息队列对象先从空闲消息链表上取下一个空闲消息块，
  把线程或者中断服务 程序发送的消息内容复制到消息块上，然后把该消息块挂到消息队列的尾部。 当且仅当空闲消息链表上有可用的空闲消息块时，
  发送者才能成功发送消息； 当空闲消息链表上无可用消息块，说明消息队列已满，此时，发送消息的的 线程或者中断程序会收到一个错误码（-RT_EFULL）。

 * @param[in] mq	消息队列对象句柄
 * @param[in] buffer	消息内容
 * @param[in] size	消息大小

 * @return  RT_EOK 成功 -RT_EFULL 消息队列已满；-RT_ERROR 失败，表示发送的 消息长度大于消息队列中消息的最大长度
 **/
os_err_t os_mq_send(os_mq_t mq, void *buffer, os_size_t size);

/**
 * @brief 接收消息
 * 当消息队列中有消息时，接收者才能接收消息，否则接收者会根据超时时间设置， 或挂起在消息队列的等待线程队列上，或直接返回。
 * @param[in] mq	消息队列对象句柄
 * @param[in] buffer	消息内容
 * @param[in] size	消息大小
 * @param[in] timeout	指定的超时时间
 *
 * @return  RT_EOK 成功；-RT_ETIMEOUT 超时；-RT_ERROR 失败
 **/
os_err_t os_mq_recv(os_mq_t mq, void *buffer, os_size_t size, os_int32_t timeout);
#endif


/**
 * @brief
 * 获取线程剩余栈空间
 * @param[in] pthd	线程
 *
 * @return  返回数量
 **/
u32 thread_get_stack_free(os_thread_t pthd);

/**
 * @brief
 * 获取线程已使用栈空间
 * @param[in] pthd	线程
 *
 * @return  返回数量
 **/
AT(.com_text.rtt)
u32 thread_get_stack_used(os_thread_t pthd);

/**
 * @brief
 * 获取线程最大栈深度
 * @param[in] pthd	线程
 *
 * @return  返回数量
 **/
AT(.com_text.rtt)
u32 thread_get_stack_max_used(os_thread_t pthd);

#endif // __API_RT_THREAD__
