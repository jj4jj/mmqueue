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
using std::unique_lock;
#define MAX_WORKER_NUM 128
#define MMQ_DISPATCHER_MSG_TYPE     (0XAA55AA)
struct mmqueue_t {
    mmqueue_config_t conf;
    void *              work_ctx;
    mmqueue_work_t      work;   
    std::thread         workers[MAX_WORKER_NUM];
    mutex               lock;
    dcsmq_t             *mq;
    bool                stop;
    condition_variable  signal;
};
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
    return 0;
}

struct worker_param {
    mmqueue_t * mq;
    int         idx;
};

static void *
_worker(void * p){
    worker_param * param = (worker_param*)p;
    char * msg_recv_buff = new char [param->mq->conf.max_input_msg_size];
    char * msg_send_buff = new char [param->mq->conf.max_output_msg_size];
    dcsmq_msg_t dcmsg;
    uint64_t    reciver;
    uint64_t    worker_id = param->idx;
    while (!param->mq->stop){
        dcmsg.buffer = msg_recv_buff;
        dcmsg.sz = param->mq->conf.max_input_msg_size;
        param->mq->lock.lock();
        reciver = dcsmq_pop(param->mq->mq, dcmsg);
        param->mq->lock.unlock();
        if (reciver == (uint64_t)-1){ //empty msg
            unique_lock<std::mutex>    ulck(param->mq->lock);
            param->mq->signal.wait_for(ulck, std::chrono::seconds(1));
        }
        else {
            size_t omsg_sz = param->mq->conf.max_output_msg_size;
            param->mq->work(param->mq->work_ctx, param->idx, msg_send_buff, &omsg_sz, dcmsg.buffer, dcmsg.sz);
            param->mq->lock.lock();
            dcsmq_push(param->mq->mq, MMQ_DISPATCHER_MSG_TYPE, dcsmq_msg_t(msg_send_buff, omsg_sz));
            param->mq->lock.unlock();
        }
    }
    delete msg_recv_buff;
    delete msg_send_buff;
    return nullptr;
}
mmqueue_config_t::mmqueue_config_t(){
    key = "/tmp";
    max_worker = 1;
    max_queue_size = 128;
    max_input_msg_size = 1024;
    max_output_msg_size = 1024;
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
    q->stop = false;
    q->mq = dcsmq_create(mconf);
    if (!q->mq){
        GLOG_ERR("error create dcmsg queue !");
        mmqueue_destroy(q);
        return nullptr;
    }
    q->work = work;
    q->work_ctx = ctx;
    if (q->conf.max_worker == 0){
        q->conf.max_worker = std::thread::hardware_concurrency();
    }
    static worker_param param[MAX_WORKER_NUM];
    for (int i = 0; i < q->conf.max_worker; ++i){
        param[i].mq = q;
        param[i].idx = i+1;
        q->workers[i] = std::thread(_worker, &param[i]);
        q->workers[i].detach();
    }
    return q;
}
void
mmqueue_destroy(mmqueue_t * mq){
    if (mq){
        delete mq;
    }
}
void
mmqueue_stop(mmqueue_t * mq){
    mq->stop = true;
}
int
mmqueue_put(mmqueue_t * mq, const char * msg, size_t msg_sz){
    int ret;
    mq->lock.lock();
    ret = dcsmq_send(mq->mq, MMQ_DISPATCHER_MSG_TYPE, dcsmq_msg_t((char*)msg, msg_sz));
    mq->lock.unlock();
    mq->signal.notify_one();
    return ret;
}
int
mmqueue_take(mmqueue_t * mq, char * msg, size_t * msg_szp){
    dcsmq_msg_t dcmsg((char*)msg, *msg_szp);
    uint64_t msgid =  dcsmq_recv(mq->mq, dcmsg);
    if (msgid != (uint64_t)-1){
        assert(msgid == MMQ_DISPATCHER_MSG_TYPE);
        memmove(msg, dcmsg.buffer, dcmsg.sz);
        *msg_szp = dcmsg.sz;
        return 0;
    }
    return -1;

}



