#include "mq.h"
#include <string.h>
#include <libk/list.h>
#include <libk/con/semaphore.h>
#include <libk/kmalloc.h>

static struct list mqs;

static unsigned mq_id = 0;
static unsigned mq_msg_id = 0;

struct mq_entry {
    unsigned id;
    struct list mq;
    unsigned len;
    struct semaphore lock;
    struct semaphore data_available;
};


void mqs_init() {
    list_init(&mqs);
}

int mqs_create() {
    struct mq_entry* entry = kmalloc(sizeof (struct mq_entry));
    entry->id = ++mq_id;
    list_init(&entry->mq);
    semaphore_init(&entry->lock);
    semaphore_init(&entry->data_available);
    semaphore_up(&entry->lock);
    entry->len = 0;
    list_append(&mqs, entry);
    return entry->id;
}

struct mq_entry* find_mq(unsigned id) {
    list_foreach(&mqs) {
        if(LIST_FOREACH_VAL(struct mq_entry*)->id == id)
            return  LIST_FOREACH_VAL(struct mq_entry*);
    }
    return NULL;
}

int mq_push(unsigned mq_id, struct mq_msg msg) {
    struct mq_entry* mqe = find_mq(mq_id);
    if(!mqe)
        return -1;

    msg.id = ++mq_msg_id;

    struct mq_msg* hmsg = kmalloc(sizeof (struct mq_msg));
    memcpy(hmsg, &msg, sizeof (struct mq_msg));
    
    semaphore_down(&mqe->lock);
    
    list_append(&mqe->mq, hmsg);
    semaphore_up(&mqe->data_available);

    mqe->len++;
    
    semaphore_up(&mqe->lock);
    return msg.id;
}

struct mq_msg* mq_pop(unsigned mq_id) {
    struct mq_entry* mqe = find_mq(mq_id);
    if(!mqe)
        return NULL;
    
    struct mq_msg* m;

    semaphore_down(&mqe->data_available);

    semaphore_down(&mqe->lock);

    m = (struct mq_msg*) mqe->mq.first->val;
    list_remove(&mqe->mq, mqe->mq.first);

    mqe->len--;

    semaphore_up(&mqe->lock);

    return m; // note: kfree in syshdlr
}

struct mq_msg* mq_pop_nonblock(unsigned mq_id) {
    struct mq_entry* mqe = find_mq(mq_id);
    if(!mqe)
        return NULL;
    
    struct mq_msg* m;
    semaphore_down(&mqe->lock);

    if (!mqe->len) {
        semaphore_up(&mqe->lock);
        return NULL;
    }
    
    m = (struct mq_msg*) mqe->mq.first->val;
    list_remove(&mqe->mq, mqe->mq.first);
    semaphore_down(&mqe->data_available);
    mqe->len--;
    
    semaphore_up(&mqe->lock);
    return m;
}

// TODO: generalize to FS! only create sycall would be needed

