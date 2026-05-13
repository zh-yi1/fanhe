#ifndef _API_DB_H
#define _API_DB_H

// contacts 相关功能复用了通话RAM，不可在通话过程中使用

int contacts_kvdb_init(void);
int contacts_kvdb_set(const char *name, const char *number);
int contacts_kvdb_get_number(const char *name, char *number, u16 max_num_len);
int contacts_kvdb_del(const char *name);

/**
 * @param flash_addr 用于存储联系人名字的flash地址，用 \n 间隔联系人名
 * @param max_len flash字节数
 * @return u32 实际存储的字节数
 */
u32 contacts_kvdb_get_list(u32 flash_addr, u32 max_len);

void contacts_kvdb_iterate(bool (*cb)(char *name, void *arg), void *arg);

// msg 相关

#define MSG_CTRL_TYPE_STR 0
#define MSG_CTRL_TYPE_ARR 1

// u32 + msg_ctrl_part 1 + msg_ctrl_part 2 + ...
struct msg_ctrl_part {
    u8    type;
    u8    len;
    void *addr;
};

struct msg_ctrl {
    u8 num;

    struct msg_ctrl_part name;
    struct msg_ctrl_part title;
    struct msg_ctrl_part message;
};

void msg_tsdb_init(void);

void msg_tsdb_append_raw(void *header);
void msg_tsdb_append_raw_with_ts(void *header, u32 timestamp);

/**
 * @param buf 
 * @param num 需要的个数
 * @param need_msg false: timestamp; ture: timesamp + msg
 * @return real num
 */
u16 msg_tsdb_query(u32 *buf, u16 len, bool need_msg);

void msg_tsdb_append_user(const char *name, const char *title, const char *message);
int msg_tsdb_read_init(u32 addr, struct msg_ctrl *msg);
void msg_tsdb_read_name(struct msg_ctrl *msg, void *buf, u16 len);
void msg_tsdb_read_title(struct msg_ctrl *msg, void *buf, u16 len);
void msg_tsdb_read_message(struct msg_ctrl *msg, void *buf, u16 len);

#endif
