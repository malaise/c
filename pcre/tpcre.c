#include <stdio.h>
#include <unistd.h>
#include <pcre.h>

int main (void) {

  int res;
  int set;


  printf ("Pcre version is %s\n", pcre_version());

  printf ("Calling pcre_config\n");
  res = pcre_config (PCRE_CONFIG_UTF8, &set);
  printf ("Result is %d\n", res);
  printf ("Set is %d\n", set);

  exit (0);
}
  
