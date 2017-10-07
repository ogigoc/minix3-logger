#include "inc.h"
#include <minix/endpoint.h>

static endpoint_t who_e;
static int callnr;

static void get_work(message *m_ptr);
static void reply(endpoint_t whom, message *m_ptr);

static void sef_local_startup(void);

int main(int argc, char **argv)
{
    message m;
    int result;                 

    env_setargs(argc, argv);
    sef_local_startup();

    while (TRUE) {              

        get_work(&m);

        if (is_notify(callnr)) {
          printf("LS: warning, got illegal notify from: %d\n", m.m_source);
          result = EINVAL;
          goto send_reply;
    }

    init_loggers();

    switch (callnr) {
        case LS_HELLO:
        result = do_hello(&m);
        break;
        case LS_START_LOG:
        result = do_start_log(&m);
        break;
        case LS_SET_LOGGER_LEVEL:
        result = do_set_logger_level(&m);
        break;
        case LS_WRITE_LOG:
        result = do_write_log(&m);
        break;
        case LS_CLOSE_LOG:
        result = do_close_log(&m);
        break;
        case LS_CLEAR_LOGS :
        result = do_clear_logs(&m);
        break;
        default: 
        printf("LS: warning, got illegal request from %d\n", m.m_source);
        result = EINVAL;
    }

    send_reply:
        if (result != EDONTREPLY) {
            m.m_type = result;
            reply(who_e, &m);
        }
    }
    return(OK);
}

static void sef_local_startup()
{
    sef_setcb_init_fresh(sef_cb_init_fresh);
    sef_setcb_init_restart(sef_cb_init_fail);

    sef_startup();
}

static void get_work(message *m_ptr)
{
    int status = sef_receive(ANY, m_ptr);
    if (OK != status)
        panic("failed to receive message!: %d", status);
    who_e = m_ptr->m_source;
    callnr = m_ptr->m_type;
}

static void reply(endpoint_t who_e, message *m_ptr)
{
    int s = ipc_send(who_e, m_ptr);
    if (OK != s)
        printf("LS: unable to send reply to %d: %d\n", who_e, s);
}

