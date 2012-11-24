#include <iostream>
#include "sm_server.h"

#include <event2/thread.h>


int main(int argc, char **argv)
{
	runServer(8001, 1);

	system("pause");

	return 0;
}
