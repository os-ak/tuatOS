#include "a.h"
#include <stdio.h>
#include <string.h>

void HariMain(void)
{
	char* s;
	int pid;
	pid=api_getpid();
	sprintf(s,"pid : %d\n",pid);
	api_putstr0(s);
	api_sleep(1000);
	api_putstr0("\nuser wakeup");
	api_end();
}
