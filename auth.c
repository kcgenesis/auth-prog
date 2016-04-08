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


void clean(char* str) {
	int i;
	for(i=0;i<sizeof str;i++){
		if(str[i]=='\n'){
			printf("replacing newline!\n");
			str[i] = '\0';	
		}
	}	
}




int main(){
	
	FILE* fp;
	fp = fopen("users.txt","r");
	if(fp==NULL){
		fprintf(stderr, "Can't open input file users.txt!\n");
		exit(1);
	}
	
	int j,k = 0;
	char line[256];
	char readName[9];
	char readPw[100];
	char name[9];
	char pw[100];
	int tries=0;
	int auth = 0;
	while((tries < 3)&&(auth==0)){
		tries++;	
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
		//puts(pw);
		
		clean(name);
		clean(pw);
		
		
		while(fgets(line,sizeof(line),fp)){
			if(sizeof(line)>1){
				
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
					auth=1;
					break;
				} else{
					printf("no match\n");
				}
			}
		}
		if (auth==1){
			break;
		} else{
			printf("Login incorrect!\n");
			rewind(fp);
		}
		/*
			if(feof(fp)){
				printf("\n\n");
				printf("end of file reached successsfuly\n");
			}else{
				fprintf(stderr,"read error\n");
				exit(1);
			}
		*/
		//printf("You've used up try #%d\n\n",tries);
	}
	printf("Login success!\n");
	
	
	
	
	
	
	fclose(fp);
	return 0;
} 







