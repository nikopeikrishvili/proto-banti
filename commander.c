#include "include.h"
#include <sys/utsname.h>
char *get_basic_info()
{
	struct utsname buffer;
	char *data = malloc(1024); //i think its enough size
   	if (uname(&buffer) == 0)
    	{
	   	sprintf(data,"system name = %s\n", buffer.sysname);
   		sprintf(data,"%s; node name   = %s\n",data, buffer.nodename);
   		sprintf(data,"%s; release     = %s\n", data,buffer.release);
   		sprintf(data,"%s; version     = %s\n",data, buffer.version);
   		sprintf(data,"%s; machine     = %s\n",data, buffer.machine);

	   #ifdef _GNU_SOURCE
	      sprintf(data,"domain name = %s\n", buffer.domainname);
	   #endif
   }
   else
   {
   	strcpy(data, "no result");
   }
   
   return data;
}

