#pragma  once
#include <cstdlib>
#include <cstdint>

struct mmqueue_t;

struct mmqueue_config_t {
    int             max_worker;
    int             max_queue_size;
    int             max_input_msg_size;
    int             max_output_msg_size;
    char *          key;
    mmqueue_config_t();
};

typedef   void(*mmqueue_work_t)(void * ctx, int idx, char * omsg, size_t * omsg_szp, const char * inmsg, size_t inmsg_sz);
mmqueue_t *     mmqueue_create(const mmqueue_config_t * conf, mmqueue_work_t work, void * ctx);
void            mmqueue_destroy(mmqueue_t *);
void            mmqueue_stop(mmqueue_t *);
int             mmqueue_put(mmqueue_t *,const char * msg, size_t msg_sz);//input
int             mmqueue_take(mmqueue_t *, char * msg, size_t * msg_szp);

