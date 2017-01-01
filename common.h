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

#define ERR_EXIT(err_str) do { perror(err_str);  \
		exit(EXIT_FAILURE); } while(0)

#define LISTENQ 1024

#endif /* __COMMON_H_ */
