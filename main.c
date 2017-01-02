#include "common.h"
#include "session.h"
#include "sysutil.h"
#include "str.h"
#include "parseconf.h"
#include "tunable.h"


int main(int argc,char *argv[])
{
	if( getuid() != 0 )
	{
		printf("run miniftpd must be root\n");
		exit(EXIT_FAILURE);
	}
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

	int listenfd = tcp_server(tunable_listen_adress,tunable_listen_port);
	
	session_t sess = {-1,-1,"","","",-1,-1};
	pid_t pid;
	for( ; ; )
	{
		// 时间设置为0,阻塞接收连接
		int connfd = accept_timeout(listenfd,NULL,0);
		if( connfd == -1 )
			continue;
		// 创建子进程
		pid = fork();
		switch(pid)
		{
			case 0:
				// 子进程关闭listenfd，避免出现“惊群效应”
				close(listenfd);
				sess.ctrl_fd = connfd;
				begin_session(&sess);
				break;
			case -1:
				ERR_EXIT("fork");
				break;
			default:
				// 父进程关闭connfd
				close(connfd);
				break;
		}
	}

	return EXIT_SUCCESS;
}
