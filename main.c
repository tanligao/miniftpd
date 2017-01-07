#include "common.h"
#include "session.h"
#include "sysutil.h"
#include "str.h"
#include "parseconf.h"
#include "tunable.h"
#include "ftpcodes.h"
#include "ftpproto.h"
#include "hash.h"

extern session_t *p_sess;
static unsigned int s_children;

static hash_t *s_ip_count_hash;
static hash_t *s_pid_ip_hash;

void check_limits(session_t *sess);
void handle_sigchld(int sig);
unsigned int hash_func(unsigned int,void *);
unsigned int handle_ip_count(unsigned int *ip);
void drop_ip_count(unsigned int *ip);

int main(int argc,char *argv[])
{
	if( getuid() != 0 )
	{
		printf("run miniftpd must be root\n");
		exit(EXIT_FAILURE);
	}
	// test for list
	/*
	list_common();
	exit(EXIT_SUCCESS);
	*/

	// test for str.h
	/* test	str_all_space
	char *str1 = "            a         e";
	char *str2 = "                    ";
	if( str_all_space(str1) )
	{
		printf("str1 is all space\n");
	}
	else
	{
		printf("str1 is not all space\n");
	}

	if( str_all_space(str2) )
	{
		printf("str2 is all space\n");
	}
	else
	{
		printf("str2 is not all space\n");
	}
	*/

	/* test str_upper
	char str3[] = "testTest";
	str_upper(str3);
	printf("str3:%s\n", str3);
	*/

	/* test str_to_longlong
	long long result = str_to_longlong("1234567890123");
	printf("%lld\n", result);
	*/

	/* test str_octal_to_uint
	unsigned int result = str_octal_to_uint("0755");
	printf("%d\n", result);
	*/

	// test for parseconf
	/*
	parseconf_load_file(MINIFTPD_CONF);
	printf("tunable_pasv_enable: %d\n", tunable_pasv_enable);
	printf("tunable_port_enable: %d\n", tunable_port_enable);
	printf("tunable_listen_port: %d\n", tunable_listen_port);
	printf("tunable_max_clients: %d\n", tunable_max_clients);
	printf("tunable_max_per_ip: %d\n", tunable_max_per_ip);
	printf("tunable_accept_timeout: %d\n", tunable_accept_timeout);
	printf("tunable_connect_timeout: %d\n", tunable_connect_timeout);
	printf("tunable_idle_session_timeout: %d\n", tunable_idle_session_timeout);
	printf("tunable_data_connection_timeout: %d\n", tunable_data_connection_timeout);
	printf("tunable_local_umask: 0%o\n", tunable_local_umask);
	printf("tunable_upload_max_rate: %d\n", tunable_upload_max_rate);
	printf("tunable_download_max_rate: %d\n", tunable_download_max_rate);		

	if( tunable_listen_adress == NULL )
		printf("tunable_listen_addrss = NULL\n");
	else
		printf("tunable_listen_addrss: %s\n", tunable_listen_adress);
	*/

	parseconf_load_file(MINIFTPD_CONF);

	daemon(0,0);

	s_children = 0;
	s_ip_count_hash =  hash_alloc(IP_COUNT_BUCKETS,hash_func);
	s_pid_ip_hash = hash_alloc(PID_IP_COUNT,hash_func);
	signal(SIGCHLD,handle_sigchld);
	
	int listenfd = tcp_server(tunable_listen_adress,tunable_listen_port);
	
	session_t sess = {-1,-1,"","","",-1,-1,0,NULL,-1,
		-1,0,0,NULL,0,0,0,0,0,0,0};
	
	p_sess = &sess;
	
	sess.bw_upload_rate_max = tunable_upload_max_rate;
	sess.bw_download_rate_max = tunable_download_max_rate;

	pid_t pid;
	for( ; ; )
	{
		struct sockaddr_in client_addr;
		bzero(&client_addr,sizeof(struct sockaddr_in));

		// 时间设置为0,阻塞接收连接
		int connfd = accept_timeout(listenfd,&client_addr,0);
		
		unsigned int client_ip = client_addr.sin_addr.s_addr;

		 sess.num_this_ip = handle_ip_count(&client_ip);

		if( connfd == -1 )
			continue;

		++s_children;
		sess.num_clients = s_children;

		// 创建子进程
		pid = fork();
		switch(pid)
		{
			case 0:
				// 子进程关闭listenfd，避免出现“惊群效应”
				close(listenfd);
				sess.ctrl_fd = connfd;
				check_limits(&sess);
				signal(SIGCHLD,SIG_IGN);
				begin_session(&sess);
				break;
			case -1:
				--s_children;
				ERR_EXIT("fork");
				break;
			default:
				hash_add_entry(s_pid_ip_hash,&pid,sizeof(pid),&client_ip,sizeof(client_ip));
				// 父进程关闭connfd
				close(connfd);
				break;
		}
	}

	return EXIT_SUCCESS;
}

void check_limits(session_t *sess)
{
	if( tunable_max_clients > 0 && sess->num_clients > tunable_max_clients )
	{	
		ftp_relply(sess,FTP_TOO_MANY_USERS,"There are too many connected users,please try later.");

		exit(EXIT_FAILURE);
	}

	if( tunable_max_per_ip > 0 && sess->num_this_ip > tunable_max_per_ip )
	{
		ftp_relply(sess,FTP_IP_LIMIT,"There are too many connections,from your internet address");

		exit(EXIT_FAILURE);
	}
}

void handle_sigchld(int sig)
{
	pid_t pid;
	while( (pid = waitpid(-1,NULL,WNOHANG)) > 0 )
	{
		unsigned int *ip = hash_lookup_entry(s_pid_ip_hash,&pid,sizeof(pid));
		if( ip == NULL )
			continue;

		drop_ip_count(ip);
		hash_free_entry(s_pid_ip_hash,&pid,sizeof(pid))	;
	}	

	--s_children;
}

unsigned int hash_func(unsigned int buckets,void *key)
{
	unsigned int *number = (unsigned int *)key;

	return (*number) % buckets;
}

unsigned int handle_ip_count(unsigned int *ip)
{
	// 当一个客户登录时，先在s_ip_count_hash更新这个表中的对应的
	// 表项，即该ip对应的连接数+1,如果这个表项不存在，则在表中添加
	// 一条记录，并且将ip对应的连接数置为1
	unsigned int count;
	unsigned int *p_count = (unsigned int *)hash_lookup_entry(s_ip_count_hash,ip,sizeof(unsigned int));

	if( p_count == NULL )
	{
		count = 1;
		hash_add_entry(s_ip_count_hash,ip,sizeof(unsigned int),&count,sizeof(unsigned int));
	}
	else
	{
		count = *p_count;
		++count;
		*p_count = count;
	}
	return count;
}

void drop_ip_count(unsigned int *ip)
{
	unsigned int count;
	unsigned int *p_count = (unsigned int *)hash_lookup_entry(s_ip_count_hash,ip,sizeof(unsigned int));

	if( p_count == NULL )
	{
		return;
	}
	count = *p_count;
	if( count <= 0 )
	{
		return;
	}
	--count;
	*p_count = count;
	
	if( count == 0 )
	{
		hash_free_entry(s_ip_count_hash,ip,sizeof(unsigned int));
	}
}