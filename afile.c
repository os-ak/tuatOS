#include "a.h"
#include <stdio.h>
#include <string.h>

void HariMain(void)
{
	FileDescriptor *fd;
	char* path = "/TESFILE";
	char src[16];
	char *result;
	if(0 != api_crate(path)) {
		api_putstr0("create error");
		api_end();
	}
	if((fd =(FileDescriptor*)api_open(path,FD_MODE_READ_WRITE))== NULL){
		api_putstr0("open error");
		api_end();
	}
	strcpy(src,"writing");
	if(0 != api_write(fd,16,src)){
		api_putstr0("write error");
		api_end();
	}
	if(0 != api_read(fd,16,result)){
		 api_putstr0("read error");
		 api_end();
 	}
	api_putstr0(result);
	if(0 != api_close(fd))
		api_putstr0("create error");
	api_end();
}
