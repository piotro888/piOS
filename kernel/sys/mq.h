#ifndef SYS_MQ_H
#define SYS_MQ_H

#include <libk/types.h>

struct mq_msg {
    int id;
    int type;
    size_t size;
    void* data;
};

void mqs_init();

int mqs_create();
int mq_push(unsigned mq_id, struct mq_msg msg);
struct mq_msg* mq_pop(unsigned mq_id);
struct mq_msg* mq_pop_nonblock(unsigned mq_id);

#endif