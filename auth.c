#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
/*
	typedef struct {
	char name[8];
	char pw[100];
	} user;

*/

//clean newline
void clean(char* str) {
	int i;
	for(i=0;i<sizeof str;i++){
		if(str[i]=='\n'){
			printf("removing newline\n");
			str[i] = '\0';	
		}
	}
}

void parse(char* str, const char* delim,char fields[10][100]){
	//printf("WE PARSING\n");	
	char* token = strtok(str,delim);
	
	int i=0;
	while(token != NULL){
		strcpy(fields[i],token);
		clean(fields[i]);
		printf("field %d: %s\n",i,fields[i]);
		token = strtok(NULL,delim);
		i++;
	}
}




//return 0 if user
//return 1 if no match
int isUser(char* name, char* pw,FILE* fp){
	char line[256];
	
	while(fgets(line,sizeof(line),fp)){
		if(sizeof(line)>1){ 
			const char d[] = ":\r\n";
			char fields[10][100];
			parse(line,d,fields);
			
			/* 
				printf("Does it match?\n");
				printf("name: %s\n",readName);
				printf("password: %s\n",readPw);
				printf("vs\n");
				printf("name: %s\n",name);
				printf("password: %s\n",pw);
			 */
			if ((strcmp(name,fields[0])==0)&&(strcmp(pw,fields[1])==0)){
				return 0;
			}
		}
	}
	if(feof(fp)){
			//printf("end of file reached successsfuly\n");
			return 1;
	}else{
		fprintf(stderr,"read error\n");
		exit(1);
	}
	
}

int login(){
	FILE* fp;
	fp = fopen("users.txt","r");
	if(fp==NULL){
		fprintf(stderr, "Can't open input file users.txt!\n");
		exit(1);
	}
	char name[9];
	char pw[100];
	int tries=1;
	int auth = 0;
	while((tries <= 3)&&(auth==0)){	
		printf("Enter name!\n");
		fgets(name,sizeof(name),stdin);
		//puts(name);
		//turn echo off if possible
		//ncurses?
		//http://forums.fedoraforum.org/showthread.php?t=270335
		//examples
		//http://www.cplusplus.com/articles/E6vU7k9E/
		printf("Enter password!\n");
		fgets(pw,sizeof(pw),stdin);
		clean(name);
		clean(pw);
		
		if (isUser(name,pw,fp)==0){
			auth =1;
			break;
		} else{
			printf("Login incorrect!\n");
			rewind(fp);
		}
		//printf("You've used up try #%d\n\n",tries);
		tries++;
	}
	fclose(fp);
	return auth;
}


int main(){
	
	if(login()==1){
		
	}
	
	
	
	
	
	
	return 0;
} 







