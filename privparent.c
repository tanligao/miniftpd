#include "privparent.h"
#include "common.h"

void handle_parent(session_t *sess)
{
	char cmd;
	while(1)
	{
		// 读取来自子进程的数据,解析命令(内部)
		read(sess->parent_fd,&cmd,1);
		
	}
}
