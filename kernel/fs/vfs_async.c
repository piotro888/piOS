#include "vfs_async.h"
#include <libk/con/semaphore.h>

void vfs_async_finalize(struct vfs_async_req_t* req, ssize_t ret) {
    req->res = ret;
    
    if (req->callback)
        req->callback(req);

    if (req->fin_sema)
        semaphore_up(req->fin_sema);
}