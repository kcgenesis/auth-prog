#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

typedef struct {
char name[100];
char pw[100];
} user;


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
	
	int n;
	for(n=0;n<10;n++){
		fields[n][0] = '\0';
	}
	strtok(str,"\r\n");


	int i; //str char
	int j=0;//word char
	int k=0;//wordnum
	int STATE=0;
	//printf("the length is %d\n",strlen(str));

	for(i=0;i<strlen(str);i++){
		if(str[i]==*delim){	
			if(STATE==1){
				STATE=0;
				fields[k][j]='\0';
			}
			k++;
		}else{
			if(STATE==0){
				STATE=1;
				j=0;
			}
			fields[k][j] = str[i];
			j++;
			if(str[i+1]=='\0'){
				fields[k][j] = '\0';
			}
		}
	}
	/*
		int m;
		for(m=0;m<3;m++){
			printf("field %d: %s\n",m+1,fields[m]);	
		}
	*/
}




//return 0 if user
//return 1 if no match
int isUser(user* usr,FILE* fp){
	char line[256];
	while(fgets(line,sizeof(line),fp)){
		if(sizeof(line)>1){ 
			const char d = ':';
			char fields[10][100];
			parse(line,&d,fields);
			
			/* 
				printf("Does it match?\n");
				printf("name: %s\n",readName);
				printf("password: %s\n",readPw);
				printf("vs\n");
				printf("name: %s\n",name);
				printf("password: %s\n",pw);
			 */
			if ((strcmp(usr->name,fields[0])==0)&&(strcmp(usr->pw,fields[1])==0)){
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





void enterinfo(user* usr){
	//fields[0]=name
	//fields[1]=pw
	

	//printf("length is %d\n",(int)strlen(usr->name));
	int valid=0;
	while(valid==0){
		printf("Enter name!\n");
		fgets(usr->name,sizeof(usr->name),stdin);
		strtok(usr->name,"\n");
		//printf("Username length: %d\n",(int)strlen(usr->name));
		if(strcmp(usr->name,"\n")==0){
			printf("No input! try again\n");
		}else if(strlen(usr->name)>8){
			printf("Username length %d too long! try again\n",(int)strlen(usr->name));
		}else {
			//printf("Enter password:\n");
   			strcpy(usr->pw,getpass("Enter password:\n"));
   			//usr->pw[0] = mypw;
			//fgets(usr->pw,sizeof(usr->pw),stdin);
			strtok(usr->pw,"\n");
			//printf("Password length: %d\n",(int)strlen(usr->pw));
			if(strlen(usr->pw)==0){
				printf("No input! try again\n");
			}else{
				valid++;
			}
		}
	}
}







//return user* 
user* login(user* usr){
	FILE* fp;
	fp = fopen("users.txt","r");
	if(fp==NULL){
		fprintf(stderr, "Can't open input file users.txt!\n");
		exit(1);
	}
	int tries=1;
	int auth = 0;
	while((tries <= 3)&&(auth==0)){	
		
		/*
			puts(name);
			turn echo off if possible
			ncurses?
			http://forums.fedoraforum.org/showthread.php?t=270335
			examples
			http://www.cplusplus.com/articles/E6vU7k9E/		
		*/
		
		
		enterinfo(usr);

		if (isUser(usr,fp)==0){
			auth=1;
			break;
		} else{
			printf("Login incorrect!\n");
			rewind(fp);
		}
		//printf("You've used up try #%d\n\n",tries);
		tries++;
	}
	if(auth==0){
		usr = NULL; 
		printf("3 tries used: exiting\n");
	}
	fclose(fp);
	return usr;
}

//return 0 if authorized
//return 1 if not	
int isAuthorized(user* usr,char* fname,FILE* fp){
	char line[256];
	
	while(fgets(line,sizeof(line),fp)){
		if(strlen(line)>1){ 
			const char d[] = ":\r\n";
			char fields[10][100];
			parse(line,d,fields);
			/*
			for(i=0;i<3;i++){
				printf("field[%d]: %s\n",i,fields[i]);
			}
			*/
			//no permission specified -> deny permission
			//first rule parsed is the rule which will be applied
			
			if (strlen(fields[2])==0){
				//printf("USER wildcard detected\n");
			}else if(strlen(fields[1])==0){

			}



			if ((strcmp(usr->name,fields[1])==0)||(strlen(fields[1])==0)){
				//printf("matched username/wildcard\n");
				if ((strcmp(fname,fields[2])==0)||(strlen(fields[2])==0)){
					//printf("matched filename/wildcard\n");
					if(strcmp("PERMIT",fields[0])==0){
						//printf("permitted\n");
						return 0;
					}else if(strcmp("DENY",fields[0])==0){
						//printf("denied\n");
						return 1;
					}else{
						//printf("No permission found: denied\n");
						return 1;
					}
				}
			}
			
		}
	}
	if(feof(fp)){
			//printf("end of file reached successsfuly\n");
			//printf("not found in ACL: denied\n");
			return 1;
	}else{
		fprintf(stderr,"read error\n");
		exit(1);
	}
}

void printFile(char* fname){
	FILE* fp;
	const char* name = fname;
	fp = fopen(name,"r");
	if(fp==NULL){
		fprintf(stderr, "Can't open input file %s!\n",fname);
		exit(1);
	}
	printf("--------BEGIN '%s'--------\n",fname);
	char line[256];
	while(fgets(line,sizeof(line),fp)){
		printf("%s",line);
	}
	if(!feof(fp)){
		fprintf(stderr,"read error\n");
		exit(1);
	}
	printf("\n--------END '%s'--------\n",fname);
	fclose(fp);
}


void readFile(user* usr){
	FILE* fp;
	fp = fopen("auth.txt","r");
	if(fp==NULL){
		fprintf(stderr, "Can't open input file auth.txt!\n");
		exit(1);
	}
	char line[256];
	printf("enter file path:\n");
	while(fgets(line,sizeof(line),stdin)){
		if(strlen(line)>1){
			strtok(line,"\n");
			if(fopen(line,"r")==NULL){
				printf("the file '%s' does not exist! try again\n",line);
			}else{
				if(isAuthorized(usr,line,fp)==0){
					printFile(line);
				}else{
					printf("Access to file %s denied\n",line);
				}
				printf("enter file path:\n");
				rewind(fp);
			}
		}else {
			printf("no input! try again\n");
		}
	}
	printf("EOF read: exiting\n");
	fclose(fp);
}




int main(){
	user myuser;
	if(login(&myuser)!=NULL){
		readFile(&myuser);
	}
	return 0;
} 







