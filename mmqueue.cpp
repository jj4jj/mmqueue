
#include "mmqueue.h"
#include "base/stdinc.h"
#include "base/dcsmq.h"
#include "base/logger.h"

//void *mmap(void *addr, size_t length, int prot, int flags,
//    int fd, off_t offset);
//int munmap(void *addr, size_t length);
//int msync(void *addr, size_t length, int flags);

using std::vector;
using std::string;
using std::thread;
using std::mutex;
using std::condition_variable;

struct mmqueue_t {
    mmqueue_config_t conf;
    void *              work_ctx;
    mmqueue_work_t      work;
    vector<thread>      workers;
    mutex               lock;
    dcsmq_t             *mq;
};
#define MAX_WORKER_NUM 128
static inline int 
_check_conf(const mmqueue_config_t * conf){
    if (!conf ||
        !conf->key ||
        conf->max_queue_size <= 0 ||
        conf->max_worker <= 0 ||
        conf->max_worker >= MAX_WORKER_NUM ||
        conf->max_input_msg_size <= 0 ||
        conf->max_output_msg_size <= 0){
        return -1;
    }
}

struct worker_param {
    void *  ctx;
    int     idx;
};

static void *
_worker(void * p){
    worker_param * param = (worker_param*)p;
    //fetch msg
    //...
}


mmqueue_t *    
mmqueue_create(const mmqueue_config_t * conf, mmqueue_work_t work, void * ctx){
    if (_check_conf(conf)){
        GLOG_ERR("error mmqueue config !");
        return nullptr;
    }
    mmqueue_t * q = new mmqueue_t();
    q->conf = *conf;
    q->conf.key = new char[strlen(conf->key)+1];
    strcpy(q->conf.key, conf->key);
    q->work = nullptr;
    q->work_ctx = nullptr;
    dcsmq_config_t  mconf;
    mconf.keypath = conf->key;
    //to optimal
    mconf.max_queue_buff_size = conf->max_queue_size * std::max(conf->max_output_msg_size, conf->max_input_msg_size);
    mconf.msg_buffsz = std::max(conf->max_output_msg_size, conf->max_input_msg_size);
    mconf.passive = false;
    q->mq = dcsmq_create(mconf);
    if (!q->mq){
        GLOG_ERR("error create dcmsg queue !");
        mmqueue_destroy(q);
        return nullptr;
    }
    q->work = work;
    q->work_ctx = ctx;
    static worker_param param[MAX_WORKER_NUM];
    for (int i = 0; i < conf->max_worker; ++i){
        param[i].ctx = ctx;
        param[i].idx = i;
        q->workers[i] = thread(_worker, &param[i]);
        q->workers[i].detach();
    }
    return q;
}
void            mmqueue_destroy(mmqueue_t *){

}
int             mmqueue_put(mmqueue_t *, const void * msg, size_t msg_sz);//input
int             mmqueue_take(mmqueue_t *, void * msg, size_t * msg_szp)