#include "include.h"
#include "commander.h"

void function_a(void) { printf("Function A\n"); }
void function_b(void) { printf("Function B\n"); }
void function_c(void) { printf("Function C\n"); }
void function_d(void) { printf("Function D\n"); }
void function_e(void) { printf("Function E\n"); }

int call_function(const char *name)
{

  int i;
  for (i = 0; i < (sizeof(function_map) / sizeof(function_map[0])); i++) {
    if (strcmp(function_map[i].name, name)==0  && function_map[i].func) {
        function_map[i].func();

      return 0;
    }
    else
    {

    }
  }

  return -1;
}


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

