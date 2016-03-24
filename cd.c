#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
void cd(char *path,char *home,char *olddir){
	char *token = (char *)malloc(1024*sizeof(char));
	char *error = (char *)malloc(1024*sizeof(char));
	if(path == NULL){     //If there is no argument change to home
		chdir(home);
		return;
	}
	if(path[0]=='/'){   //Change to root 
		chdir("/");
		path++;
	}
	else if(path[0]=='~'){   //Change to home 
		chdir(home);
		path++;
	}
	token = strtok(path,"/");   //Tokenize the path and change to directory accordingly
	while(token != NULL){
		int flag = chdir(token);
		error = strerror(errno);
		if(flag == -1){
			chdir(olddir);     //If there is an error revert back to old directory
			printf("%s\n",error);
			return;
		}
		token = strtok(NULL,"/");
	}
}
