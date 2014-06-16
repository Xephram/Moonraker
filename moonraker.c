#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/tcp.h>

//inet
#include <net/inet_connection_sock.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daymion Reynolds <daymion@gmail.com>");
MODULE_DESCRIPTION("In-kernel Cache");
//module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
//MODULE_PARM_DESC(buffer_size, "Internal buffer size");

#define CONFIG_TUX_NUMTHREADS 16
#define NR_IO_THREADS 64
#define CONFIG_TUX_NUMSOCKETS 16
#define MOONRAKER_LOGGING 5

typedef struct moonraker_error_s{
	int level;
	char* msg;
}moonraker_error_t;

typedef struct moonraker_req_s{
	char *url;
	int size;
	int duration;
	int time;
	moonraker_error_t *error;
}moonraker_req_t;

typedef struct moonraker_proto_s
{
        unsigned int defer_accept;
        unsigned int can_redirect;
        //atom_func_t *illegal_request;
        //atom_func_t *request_timeout;
        char *name;
} moonraker_proto_t;

typedef struct moonraker_socket_s{
	moonraker_proto_t *proto;
	unsigned int ip;
	unsigned short port;
	struct proc_dir_entry *entry;
	int keepalive_timeout;
	unsigned int ack_pingpong;
	unsigned int defer_accept;
	unsigned int max_backlog;
} moonraker_socket_t;



void moon_log(moonraker_req_t *req);
moonraker_req_t *moon_req_alloc(void);
void moon_req_free(moonraker_req_t *req);
void moon_req_free_error(moonraker_req_t *req);
void moon_req_add_error(moonraker_req_t *req,int level,char* msg);
void moon_req_add_url(moonraker_req_t *req,char *url);

void moon_listen(void);


static int __init moonraker_init(void)
{
	printk(KERN_INFO "Moonraker device has been registered\n");
	return 0;
}
 
static void __exit moonraker_exit(void)
{
	printk(KERN_INFO "Moonraker device has been unregistered\n");
}

//Moonraker loger
void moon_log(moonraker_req_t *req)
{
	if(req->error->level<MOONRAKER_LOGGING)
               return;

	printk(KERN_INFO "information received");
	//Log it
}

//Moonraker Request memory managment
//long term this will not work due to the copy of the request
//will need to minimize this.
moonraker_req_t *moon_req_alloc(void)
{
	moonraker_req_t *req = kmalloc(sizeof(moonraker_req_t),GFP_KERNEL);
	req->url=NULL;
	req->error=NULL;
	return req;	

}
//start_listening
void moon_listen(void)
{
	//For test purposes
	moonraker_socket_t *listen;
	listen = kmalloc(sizeof(moonraker_socket_t),GFP_KERNEL);
	listen->proto = kmalloc(sizeof(moonraker_proto_t),GFP_KERNEL);
	listen->proto->name = kmalloc(5,GFP_KERNEL);
	listen->ip = 2130706433;
	listen->port = 80;
	listen->proto->defer_accept=0;
	listen->keepalive_timeout=0;
	listen->ack_pingpong=1;
	listen->max_backlog=2048;
	listen->defer_accept=1;

	strcpy(listen->proto->name,"http");
	//end for test purpose
	
       	struct sockaddr_in sin;
       	struct socket *sock = NULL;
       	struct sock *sk;
       	struct tcp_sock *tp;
       	struct inet_connection_sock *icsk;
	moonraker_proto_t *proto = listen->proto;

	u16 port = listen->port;
	u32 addr = listen->ip; //127.0.0.1

	int err = 0;

	err = sock_create_kern(PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
       	if (err) {
               printk(KERN_ERR "Moonraker: error %d creating socket.\n", err);
               goto error;
	}



	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(addr);
	sin.sin_port = htons(port);


	sk = sock->sk;
	icsk = inet_csk(sk);
	sk->sk_reuse = 1;
	sock_set_flag(sk, SOCK_URGINLINE);
	err = sock->ops->bind(sock, (struct sockaddr*)&sin, sizeof(sin));

	if (err){
		printk(KERN_ERR "Moonraker: error %d binding socket. This means that probably some other process is (or was a short time ago) using addr %d\n",err,sin.sin_addr.s_addr);
		goto error;
	}

	tp = tcp_sk(sk);

	printk("listen sk accept_queue: %d.\n",!reqsk_queue_empty(&icsk->icsk_accept_queue));
	icsk->icsk_ack.pingpong = listen->ack_pingpong;

	sock_reset_flag(sk, SOCK_LINGER);
	sk->sk_lingertime = 0;
	tp->linger2 = listen->keepalive_timeout * HZ;

	if (proto->defer_accept && !listen->keepalive_timeout && listen->defer_accept)
		icsk->icsk_accept_queue.rskq_defer_accept = 1;


	err = sock->ops->listen(sock, listen->max_backlog);
	if (err) {
		printk(KERN_ERR "Moonraker: error %d listening on socket.\n", err);
		goto error;
	}

	printk(KERN_NOTICE "Moonraker: thread %d listens on %s://%d.%d.%d.%d:%d.\n",
		1, proto->name, HIPQUAD(addr), port);
//       return sock;


	
	return;

error:

	if (sock)
		sock_release(sock);

	return;

	return NULL;	
}


void moon_req_add_url(moonraker_req_t *req, char *url)
{
	req->url = kmalloc(strlen(url)+1,GFP_KERNEL);
	strcpy(req->url,url);
}

void moon_req_free(moonraker_req_t *req)
{
	if(req->url!=NULL)
		kfree(req->url);

	moon_req_free_error(req);

	kfree(req);

}
void moon_req_free_error(moonraker_req_t *req)
{
        if(req->error!=NULL)
        {
                if(req->error->msg!=NULL)
                        kfree(req->error->msg);
                kfree(req->error);		
        }
	req->error=NULL;
}

void moon_req_add_error(moonraker_req_t *req, int level, char *msg)
{
	moonraker_error_t *error;
	moon_req_free_error(req);
	error = kmalloc(sizeof(moonraker_error_t),GFP_KERNEL);
	error->level = level;
	error->msg = kmalloc(strlen(msg)+1,GFP_KERNEL);
	strcpy(error->msg,msg);
	req->error = error;
}
//end Moonraker Request memory managment

module_init(moonraker_init);
module_exit(moonraker_exit);
