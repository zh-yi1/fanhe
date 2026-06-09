#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_reservation_t_ {

} f_reservation_t;

compo_form_t *func_reservation_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    return frm;
}

static void func_reservation_process(void)
{
    func_process();
}

static void func_reservation_message(size_msg_t msg)
{
    switch (msg) {
    default:
        func_message(msg);
        break;
    }
}

void func_reservation_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_reservation_t));
    func_cb.frm_main = func_reservation_form_create();
}

void func_reservation_exit(void)
{
    func_cb.last = FUNC_RESERVATION;
}

void func_reservation(void)
{
    printf("%s\n", __func__);
    func_reservation_enter();
    while (func_cb.sta == FUNC_RESERVATION) {
        func_reservation_process();
        func_reservation_message(msg_dequeue());
    }
    func_reservation_exit();
}
