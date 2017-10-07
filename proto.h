#ifndef _LS_PROTO_H
#define _LS_PROTO_H


int main(int argc, char **argv);

int do_hello(message *m);
int do_start_log(message *m);
int do_set_logger_level(message *m);
int do_write_log(message *m);
int do_close_log(message *m);
int do_clear_logs(message *m);

int init_loggers();
int sef_cb_init_fresh(int type, sef_init_info_t *info);

#endif
