#ifndef __FTPPROTO_H__
#define __FTPPROTO_H__

#include "session.h"

typedef struct ftpcmd
{
	const char *cmd;
	void (*cmd_func)(session_t *sess);
} ftpcmd_t ;

void handle_child(session_t *sess);
int list_common(session_t *sess);

#endif /*__FTPPROTO_H__ */
