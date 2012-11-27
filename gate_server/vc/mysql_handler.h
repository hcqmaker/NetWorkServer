#ifndef __MSYQL_HANDLER__
#define __MSYQL_HANDLER__

#include <mysql.h>
#include <WinSock2.h>

#include <string.h>


MYSQL g_conn;

int mysql_connect(const char *host,const char *user,const char *passwd,const char *db,
				  unsigned int port,const char *unix_socket, unsigned long clientflag);




#endif // __MSYQL_HANDLER__