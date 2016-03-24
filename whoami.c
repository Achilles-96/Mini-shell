#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
char* getusername(int argc, char *argv[])
{
  register struct passwd *pw;
  register uid_t uid;
  
  uid = geteuid ();		//get id of the user
  pw = getpwuid (uid);		//get name from uid
  if (pw)
      return pw->pw_name;
  exit (EXIT_FAILURE);
  
}
