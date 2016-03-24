#include<stdio.h>
#include<string.h>
void prompt(char *current,char *home,char *username){  //Check if current directory is behind home directory
	if(strlen(current)<strlen(home))		//and display prompt accordingly
		printf("%s@SHELL:%s>",username,current);
	else
		printf("%s@SHELL:~%s>",username,current+strlen(home));
}
