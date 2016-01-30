#include "a.h"
#include <stdio.h>
#include <string.h>

void HariMain(void)
{
	FileDescriptor *fd;
	char* path = "/TESFILE";
	char src[16];
	char *result;
	api_crate(path);
	fd =(FileDescriptor*)api_open(path,FD_MODE_READ_WRITE);
	strcpy(src,"writing");
	api_write(fd,16,src);
	api_read(fd,16,result);
	api_putstr0(result);
	api_close(fd);
	api_end();
}
