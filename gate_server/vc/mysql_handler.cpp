#include "mysql_handler.h"


int mysql_connect(const char *host,const char *user,const char *passwd,const char *db,
				  unsigned int port,const char *unix_socket, unsigned long clientflag);
{
	try
	{
		mysql_init(&g_conn);
		int ret = mysql_real_connect(&g_conn, host, user, passwd, db, port, unix_socket, clientflag);
		if (!ret)
		{

		}
	}
	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	
}