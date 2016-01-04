#pragma once
#include "stdinc.h"

struct dcsmq_t;

//initiative							passive
//											s
//	c1/2/3/4/.. = msgq.type
//	c1.send(c1, msg)		=>		s.recive(any, msg) , any = c1
//	c1.receive(c1, msg)		<=		s.send(c1, msg)


//	c1				----------->					
//					<-----------

//	c2				----------->			
//					<-----------

//	c3				----------->			
//					<-----------

//	c4				----------->			
//					<-----------


struct dcsmq_config_t {
    string			keypath;
	bool			passive;//if true, receive all type msg, else receive 
	int				msg_buffsz;
	int				max_queue_buff_size;
	bool			attach;
	dcsmq_config_t(){
		msg_buffsz = 1024 * 1024;
		max_queue_buff_size = 10 * 1024 * 1024; //10MB
		passive = false;
		attach = false;
	}
};

struct dcsmq_msg_t {
    char *       buffer;
    int			 sz;
	dcsmq_msg_t(char * buf = nullptr, int s = 0) :buffer(buf), sz(s){}
};

struct dcsmq_stat_t {
	int					sender_key;
	int					receiver_key;
	size_t				send_size;
    size_t				recv_size;
    size_t              send_num;
    size_t              recv_num;
    size_t              send_error;
    size_t              recv_error;
    uint64_t            send_last;
    uint64_t            recv_last;
    uint64_t            msg_stime;//time us
    uint64_t            msg_rtime;//time us
};

typedef int (*dcsmq_msg_cb_t)(dcsmq_t * , uint64_t src, const dcsmq_msg_t & msg, void * ud);
dcsmq_t *	dcsmq_create(const dcsmq_config_t & conf);
void		dcsmq_destroy(dcsmq_t*);
void		dcsmq_msg_cb(dcsmq_t *, dcsmq_msg_cb_t cb, void * ud);
int			dcsmq_poll(dcsmq_t*, int max_time_us );
int			dcsmq_send(dcsmq_t*,uint64_t dst, const dcsmq_msg_t & msg);
int			dcsmq_push(dcsmq_t*, uint64_t dst, const dcsmq_msg_t & msg);//send to peer like himself
uint64_t    dcsmq_recv(dcsmq_t*, dcsmq_msg_t & msg);
uint64_t	dcsmq_pop(dcsmq_t*, dcsmq_msg_t & msg);//send to peer like himself
bool		dcsmq_server_mode(dcsmq_t *);
void		dcsmq_set_session(dcsmq_t *, uint64_t session); //send or recv type
uint64_t	dcsmq_session(dcsmq_t *);
const dcsmq_stat_t *	dcsmq_stat(dcsmq_t *);

