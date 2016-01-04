#include "mmqueue.h"
#include "base/stdinc.h"
#include "base/logger.h"

using namespace std;

void  _work(void * ctx, int idx, char * omsg, size_t * omsg_szp, const char * inmsg, size_t inmsg_sz){
    GLOG_IFO("convert msg sz:%zd msg:%s by worker idx:%d", inmsg_sz, inmsg, idx);
    memcpy(omsg, inmsg, inmsg_sz);
    *omsg_szp = inmsg_sz;
}


int main(int argc, char ** argv){
    mmqueue_config_t conf;
    conf.key = "/tmp";
    conf.max_worker = 12;
    mmqueue_t * q = mmqueue_create(&conf, _work, nullptr);
    if (!q) return -1;
    char * msg = new char [1024];
    size_t msgsz = 1024;
    const char * send_msg = "hello,world !!";
    int ret = 0;
    while (true){
        usleep(10000);
        if (argc > 1){
            ret = mmqueue_put(q, send_msg, strlen(send_msg) + 1);
        }
        else {
            ret = mmqueue_take(q, msg, &msgsz);
        }
    }
    return 0;
}