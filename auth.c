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


//void readIn()

//clean newline
void clean(char* str) {
	int i;
	for(i=0;i<sizeof str;i++){
		if(str[i]=='\n'){
			str[i] = '\0';	
		}
	}
}

//return 0 if user
//return 1 if no match
int isUser(char* name, char* pw,FILE* fp){
	char readName[9];
	char readPw[100];
	char line[256];
	int j;
	int k;
	while(fgets(line,sizeof(line),fp)){
		if(sizeof(line)>1){ //not just a newline 
			j=0;
			k=0;
			while(line[j]!=':'){
				readName[j] = line[j]; 
				j++;
			}
			readName[j] = '\0';
			j++;
			while(line[j]!='\0'){
				readPw[k] = line[j]; 
				j++;
				k++;
			}
			readPw[k]='\0';
			
			clean(readName);
			clean(readPw);
			/* 
				printf("Does it match?\n");
				printf("name: %s\n",readName);
				printf("password: %s\n",readPw);
				printf("vs\n");
				printf("name: %s\n",name);
				printf("password: %s\n",pw);
			 */
			if ((strcmp(name,readName)==0)&&(strcmp(pw,readPw)==0)){
				return 0;
			}
		}
	}
	if(feof(fp)){
			printf("\n\n");
			printf("end of file reached successsfuly\n");
			return 1;
	}else{
		fprintf(stderr,"read error\n");
		exit(1);
	}
	
}


int main(){
	
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
	while((tries < 3)&&(auth==0)){	
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
			break;
		} else{
			printf("Login incorrect!\n");
			rewind(fp);
		}
		printf("You've used up try #%d\n\n",tries);
		tries++;
	}
	printf("Login success!\n");	
	fclose(fp);
	
	
	
	
	
	
	
	
	
	
	
	return 0;
} 







