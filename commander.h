/*
char const* CommandNames[]=
{
    "getcommands",
    "areyoualive"
    "getsysinfo",
    "dissconnect",
}; 
*/

#include "include.h"

void function_a(void);
void function_b(void);
void function_c(void);
void function_d(void);
void function_e(void);

const static struct {
  const char *name;
  void (*func)(void);
} function_map [5] = {
  { "function_a", function_a },
  { "function_b", function_b },
  { "function_c", function_c },
  { "function_d", function_d },
  { "function_e", function_e },
};

char *get_basic_info();

int call_function(const char *name);