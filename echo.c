#include<string.h>
#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>

void echo(char *text){
	int i;
	int state = 0,state2 = 0;
	text[strlen(text)-1]='\0';    //Removing the last newline character
	if(text[0]=='$'){
		printf("%s\n",getenv(text+1));
		return;
	}
	do{
		for(i=0;text[i]!='\0';i++){
			if(text[i]=='\\'){      //checking if a character has to be escaped
				i++;
				printf("%c",text[i]);
			}
			else if((text[i]!='\"'||state2==1)&&(text[i]!='\''||state==1))			//Managing double quotes
				printf("%c",text[i]);
			else if(text[i]=='\"')
				if(state==0)
					state = 1;
				else
					state = 0;
			else 
				if(state2 == 0)
					state2 = 1;
				else
					state2 = 0;
		}
		printf("\n");
		if(state == 1 || state2 == 1){					//Asking for more input if quotes are not balanced
			printf(">");
			fgets(text,1024,stdin);
			text[strlen(text)-1]='\0';		//Again removing newline character	
		}
	}while(state == 1);
}
