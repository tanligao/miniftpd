#ifndef  __COMMON_H_
#define __COMMON_H_

// header file
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <ctype.h>
#include <shadow.h>
#include <crypt.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <wchar.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <linux/capability.h>
#include <sys/syscall.h>

#define ERR_EXIT(err_str) do { perror(err_str);  \
		exit(EXIT_FAILURE); } while(0)

#define LISTENQ 	 1024

#define MAX_COMMAND_LINE 1024
#define MAX_COMMAND 	 32
#define MAX_ARG		 1024

#define MAX_LINE 		 1024
#define MAX_SET_NAME_LEN	 128
#define MAX_SET_VALUE_LEN	 128

#define MINIFTPD_CONF	"miniftpd.conf"

#define HALF_YEAR_SEC	182*24*3600

#define DATA_PORT		20

#endif /* __COMMON_H_ */
