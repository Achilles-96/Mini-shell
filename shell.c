#include <stdio.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <whoami.h>
#include <string.h>
#include <prompt.h>
#include <cd.h>
#include <echo.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX 1024
#define BACKED 1
#define STOPPED 2
struct tempid{
	pid_t pid;
	int type;
	char name[MAX];
};
char proclist[10000][10];
pid_t pid,pid1,rootpid;
struct tempid stack[MAX];
int top=-1;
char temp[MAX];
char home[MAX],command[MAX],*username,delim[2]=" ",*token,current[MAX],*a[MAX],command_main[MAX],buf[MAX],ch;

void executer(int *m,int *j,int in,int out,int *sig,int *background){

	(*m)++;
				command[(*j)++]='\n';			//Because fgets scans the last newline character also	
				command[(*j)++]='\0';
				int l,instate=0,outstate=0;
				char filein[MAX],fileout[MAX];;
				for(l=0;command[l]!='\n'&&l<strlen(command);l++){
					if(command[l]=='>'){
						outstate = 1;
						if(command[l+1]=='>'){
							command[l]=' ';
							outstate=2;
							l++;
						}
						command[l]=' ';
						l++;
						while(command[l]==' ')l++;
						int f=0;
						for(;command[l]!=' '&&command[l]!='<'&&command[l]!='\n';l++){
							fileout[f++]=command[l];
							command[l]=' ';
						}
						fileout[f]='\0';
					}
					if(command[l]=='<'){
						instate = 1;
						command[l]=' ';
						l++;
						while(command[l]==' ')l++;
						int f=0;
						for(;command[l]!=' '&&command[l]!='>'&&command[l]!='\n';l++){
							filein[f++]=command[l];
							command[l]=' ';
						}
						if(command[l]=='>') l--;
						filein[f]='\0';
					}
				}
				
				if(filein[strlen(filein)-1]=='\n')
					filein[strlen(filein)-1]='\0';
				if(fileout[strlen(fileout)-1]=='\n')
					fileout[strlen(fileout)-1]='\0';
				if(instate==1){
					in = open(filein, O_RDONLY);
					dup2(in,0);
				}
				if(outstate==1){
					out = open(fileout,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					dup2(out,1);
				}
				if(outstate==2){
					//printf("Boomboom\n");
					out = open(fileout, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					dup2(out,1);
				}
				int command_len = strlen(command);      
				token = (char *)strtok(command,delim);
				if(token[strlen(token)-1] == '\n')
					token[strlen(token)-1] = '\0';
				if(strcmp(token,"echo")==0)             //Call echo() function in echo.c
					echo(command+strlen(token)+1);
				else if(strcmp(token,"pwd")==0)         //print the current working directory
					printf("%s\n",current);
				else if(strcmp(token,"cd")==0){         //Extract the path to be changed to and send it to cd() in cd.c
					command[command_len-1]='\0';
					token = strtok(NULL," ");
					cd(token,home,current);
				}
				else if(strcmp(token,"quit")==0)        //Break the while(1) loop to exit out of shell
					exit(0);
				else if(strcmp(token,"fg") == 0){
					pid = fork();
					if(pid!=0){
						pid_t p = stack[top].pid;
						token = strtok(NULL,"\n ");
						int num = 0,k;
						if(token!=NULL){
							for(k=0;token[k]!='\0';k++){
								num = num*10 + token[k]-'0';
							}
							num--;
							if(num > top){
								printf("Sorry no such job\n");
								return;
							}
							p=stack[num].pid;
						}
						for(k=num;k<top;k++)
							stack[k]=stack[k+1];
					//	printf("Here buddy%d %d\n",p,rootpid);
						kill(p,SIGCONT);
					//	setpgid(p,rootpid);
					//	tcsetpgrp(STDIN_FILENO,rootpid);
					//	tcsetpgrp(STDOUT_FILENO,getpid());
						top--;
						while(1){
							pid_t pid_check = waitpid(p,sig,WNOHANG|WUNTRACED);
							//printf("Here huh\n");
							if(pid_check == p){
								if(WIFSTOPPED(*sig)){
									top++;
									stack[top].pid = pid_check;
									stack[top].type = STOPPED;
									break;
								}
								else if(WIFEXITED(*sig))
									break;
								else if(WIFSIGNALED(*sig))
									break;
							}
						}
					}
					else
						_exit(0);
				}
				else if(strcmp(token,"bg")==0){
					kill(stack[top].pid,SIGCONT);
					stack[top].type = BACKED;
				}
				else if(strcmp(token,"jobs")==0){
					int k;
					//printf("Here %d\n",top);
					for(k=0;k<=top;k++){
						printf("[%d]\t",k+1 );
						if(stack[k].type==BACKED)
							printf("Running\t");
						else
							printf("Stopped\t");
						printf("\t\t%s\n",stack[k].name);
					}
				}
				else if(strcmp(token,"kjob")==0){
					token = strtok(NULL," ");
					int jobno=0,k,sign=0;
					pid_t p;
					if(token!=NULL){
							for(k=0;token[k]!='\0';k++){
								jobno = jobno*10 + token[k]-'0';
							}
							jobno--;
							if(jobno > top){
								printf("Sorry no such job\n");
								return;
							}
							p=stack[jobno].pid;
						}
					else{
						printf("Too few arguments\n");
						return;
					}
					token = strtok(NULL," ");
					if(token!=NULL){
						for(k=0;token[k]!='\0';k++){
								sign = sign*10 + token[k]-'0';
							}
					}
					else{
						printf("Too few arguments\n");
						return;
					}
					kill(p,sign);

				}
				else if(strcmp(token,"overkill")==0){
					int k;
					for(k=0;k<=top;k++)
						kill(stack[k].pid,9);
					top=-1;
				}
				else{					//Handle functions other than cd,echo,pwd and exit	
					if(command[command_len-2]=='&'){
						*background=1;		//Setting the backgroung flag 	
						command[command_len-2]='\0';
					}
					int killstate=0;
					if(strcmp(token,"kill")==0)
						killstate=1;
					pid = fork();//Creating a new process	
					if(pid==0){			//Child process
						int i=0;
						char ppp[MAX];
						while(token!=NULL){	//Tokenizing the given command for giving it to execvp
							a[i]=(char*)malloc(strlen(token)*sizeof(char));
							strncpy(a[i],token,strlen(token));
							token = strtok(NULL," \n");
							i++;
						}
						a[i]=NULL;
						char xx[MAX],yy[MAX];
						int retstatus = execvp(a[0],a);		//Execute the command
						
						if(retstatus == -1)
							printf("%s\n",strerror(errno));
							
						_exit(0);
					}

					else{	
					//	printf("%d %d\n",getpgid(0),getpid());
						if(*background!=0){
							printf("[%d]\n",pid);
					//		setpgid(pid,0);
					//		tcsetpgrp(STDIN_FILENO,getpid());
					//		tcsetpgrp(STDOUT_FILENO,getpid());
						}
						//Parent process
						sprintf(buf,"/proc/%d/cmdline",pid);		//Getting the process name from from process
						FILE *fp = fopen(buf,"r");
						usleep(10000);
						int m=0;		
						while( ( ch = fgetc(fp) ) != EOF )  temp[m++]=ch;
						strncpy(proclist[pid],temp,strlen(temp));
						if(*background==0){
							while(1){
								pid_t pid_check = waitpid(pid,sig,WNOHANG|WUNTRACED);
								if(pid_check == pid){
									if(WIFSTOPPED(*sig)){
										top++;
										stack[top].pid = pid;
										stack[top].type = STOPPED;
										strncpy(stack[top].name,temp,strlen(temp));
										break;
									}
									else if(WIFEXITED(*sig))
										break;
									else if(WIFSIGNALED(*sig))
										break;
								}
							}
						}
						else{
						//	printf("Hello\n");
							top++;
							stack[top].pid=pid;
							stack[top].type=BACKED;
							strncpy(stack[top].name,temp,strlen(temp));
						//	printf("%d\n",top );
						}
					}
				}
}
void stop_handler(int signo){
	if( signo == SIGTSTP )	
		if(pid == 0)
			kill(getpid(),SIGSTOP);
}
void sigint_handler(int signo){
	if( signo == SIGINT )	
		if(getpid() != rootpid)
			kill(getpid(),SIGKILL);
}
int main(){
	rootpid=getpid();
	signal(SIGTSTP,stop_handler);
	signal(SIGINT,sigint_handler);
	username = (char *)malloc(MAX*sizeof(char));
	token = (char *)malloc(MAX*sizeof(char));
	getcwd(home,sizeof(home));          //getting launch directory and setting it as home directory
	username = (char *)getusername();   //getting the username from whoami.c
	int sig;
	while(1){
		int background = 0,i;
		for(i=0;i<MAX;i++) command[i]='\0';
		getcwd(current,MAX);
		while((pid=waitpid(-1,&sig,WNOHANG|WUNTRACED))>0) {
			int state = 0,k,trr;
			for(k=0;k<=top;k++)
				if(stack[k].pid==pid){
					state=1;
					break;
				}

			if((WIFEXITED(sig)&&state == 1)||kill(stack[k].pid,0)==-1){
				printf("[%d] %s exited\n",pid,proclist[pid]);
				for(trr=k;trr<top;trr++)
						stack[trr]=stack[trr+1];
					top--;
					}   //checking if any background processes exited
		}
		prompt(current,home,username);   //Calling prompt() in prompt.c
		fgets(command_main,MAX,stdin);   //Taking command
		int m=0;
		int fd[2],in=0,out;
		int stdintemp=dup(0);
		int stdouttemp=dup(1);
		while(command_main[m]!='\0'){                                                   
			int j=0;
			if(command_main[0]=='\n')
				break;
			while(command_main[m]!=';'&&command_main[m]!='\n'&&command_main[m]!='|'){              //Splitting the command
				command[j]=command_main[m];                              //based on semicolons
				j++;
				m++;
			}
			char c = command_main[m];
			if(c=='|'){
				pipe(fd);
				//out = fd[1];
				if((pid1=fork())==0){
					if(in!=0){
						dup2(in,0);
						close(in);
					}
					if(fd[1]!=1){
						//		printf("Here\n");
						dup2(fd[1],1);
						close(fd[1]);
					}
					executer(&m,&j,in,out,&sig,&background);	
					_exit(0);
				}
				else{
					m++;
					wait(NULL);
					close(fd[1]);
					in = fd[0];
				}

			}
			else{
				if(in!=0){
					//	printf("here%s\n",command);
					dup2(in,0);
					close(in);
				}
				executer(&m,&j,in,out,&sig,&background);
				in=0;
				dup2(stdintemp,0);
				dup2(stdouttemp,1);
				close(stdintemp);
				close(stdouttemp);
			}
		}
	}
	return 0;
}
