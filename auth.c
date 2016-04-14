#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <openssl/sha.h>



#define SALTING 0
#define SALT_LEN 10

typedef struct {
char name[100];
char pw[100];
} user;

char* record(char* data,char* dest);
/*
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
*/

//parse string by delim.
//if there are multiple sequential delims, empty string fields are created for each delim.
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
int isUser(user* usr){
	FILE* fp;
	fp = fopen("users.txt","r");
	if(fp==NULL){
		fprintf(stderr, "Can't open input file users.txt!\n");
		exit(1);
	}
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
				if(SALTING){
				char dest[256];
				strcat(line,record(usr->pw,dest));
				strcat(line,"\n");
				}else{
					strcat(line,usr->pw);
					strcat(line,"\n");
				}

			if ((strcmp(usr->name,fields[0])==0)&&(strcmp(usr->pw,fields[1])==0)){
				fclose(fp);
				return 0;
			}
		}
	}
	if(feof(fp)){
			fclose(fp);
			return 1;
	}else{
		fprintf(stderr,"read error\n");
		fclose(fp);
		exit(1);
	}
	
}

//fill usr struct with usr info
void enterinfo(user* usr){
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


//if login fail, return null user
//if login success, return filled usr struct
user* login(user* usr){
	
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

		if (isUser(usr)==0){
			auth=1;
			break;
		} else{
			printf("Login incorrect!\n");
		}
		//printf("You've used up try #%d\n\n",tries);
		tries++;
	}
	if(auth==0){
		usr = NULL; 
		printf("3 tries used: exiting\n");
	}
	return usr;
}

//return 0 if authorized by ACL
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

//print specified file
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

//enter readFile user interface
//if user is authorized, print file.
//if not, deny access.
//return 1 to exit outer loop.
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
	printf("EOF read: exiting to main menu\n");
	fclose(fp);
}

//return 1 if file ends with a newline.
//return 0 if not.
//return 2 if error.
int newlined(char* fname){
	FILE* fp;
	fp = fopen(fname,"r");
	if(fp==NULL){
		fprintf(stderr, "Can't open input file users.txt!\n");
		exit(1);
	}
	int nlflag;
	char line[256];
	while(fgets(line,sizeof(line),fp)){
		if(sizeof(line)>1){ 
			nlflag = 0;
		}else{
			nlflag=1;
		}
	}
	if(feof(fp)){
		fclose(fp);
		if(nlflag==0){
			return 0;
		}else if(nlflag==1){
			return 1;
		}else{
			printf("error!\n");
			return 2;
		}
	}else{
		fprintf(stderr,"read error\n");
		fclose(fp);
		return 2;
		exit(1);
	}
}





//given a user struct, add users to users.txt.
//assumes that the user struct is not already a user.
int addUser(user* usr){
	FILE* fp;
	fp = fopen("users.txt","a");
	if(fp==NULL){
		fprintf(stderr, "Can't open input file users.txt!\n");
		exit(1);
	}
	char line[256];
	line[0] = '\0';
	if(newlined("users.txt")){
		strcat(line,"\n");
	}
	strcat(line,usr->name);
	strcat(line,":");

	if(SALTING){
		char dest[256];
		strcat(line,record(usr->pw,dest));
		strcat(line,"\n");
	}else{
		strcat(line,usr->pw);
		strcat(line,"\n");
	}
	
	
	fputs(line,fp);
	fclose(fp);
	return 0;
}


//make usr.
//return 1 if usr already exists
//return 0 if usr created successfully
//return 2 if error
int mkUser(user* usr){
	enterinfo(usr);	
	int test = isUser(usr);
	if(test==0){
		printf("This user already exists! Exiting.\n");
		return 1;
	}else if(test==1){
		addUser(usr);
		printf("User added successfully!\n");
		return 0;
	}else{
		printf("Error: we should never get here!\n");
		return 2;
		exit(1);
	}
}


char* randStr(char* str,int* len){
	srand(time(NULL));
	int min = 33;
	int max = 126;
	int i;
	for(i=0;i<*len;i++){
		str[i] = (rand() % (max-min)) + min;
	}
	str[i] = '\0';
	return str;
}


char* hash(char buf[SHA_DIGEST_LENGTH*2],unsigned char temp[SHA_DIGEST_LENGTH],char* data){
	memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
    memset(temp, 0x0, SHA_DIGEST_LENGTH);
    SHA1((unsigned char *)data, strlen(data), temp);
    int i;
    for (i=0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf((char*)&(buf[i*2]), "%02x", temp[i]);
    }
    return buf;
}


char* record(char* data,char* dest){
	char randl[256];
	int siz=SALT_LEN;
	randStr(randl,&siz);
	char salt[256];
	strcpy(salt,randl);
	printf("salt: %s\n",salt);
	char buf[SHA_DIGEST_LENGTH*2];
	unsigned char temp[SHA_DIGEST_LENGTH];
	hash(buf,temp,strcat(randl,data));
	printf("prepending to %s: %s\n",data,randl);
	printf("hash of '%s': %s \n",randl,buf);
	return strcpy(dest,strcat(buf,salt));
}

/*
char* getSalt(char* line){

}
*/

int main(){
	char data[] = "Hello wordl";
	char dest[256];

	record(data,dest);

	printf("%s\n",dest );

	//char decode[] = "bc67a1096e92a9556bea72b6e182314c32a8ca5cbc67a1096e92a9556bea72b6e182314c32a8ca5c!1\\*{00wKL";

	printf("sizeof decode: %d\n",(int)strlen(dest));

	int len = sizeof(dest);
	int i;
	for(i=len-1;i>=len-1-SALT_LEN;i--){
		printf("%c\n",dest[i]);
	}


	return 0;
}


/*

int main(){
	printf("Welcome to Some Files, Inc!\n");
	printf("Press 1 to make an account.\n");
	printf("Press 2 to Sign in to access Some Files.\n");
	printf("Press ctrl+D to exit.\n");
	char instr[256];
	user myuser;
	while(fgets(instr,sizeof(instr),stdin)){
		strtok(instr,"\n");
		//printf("Username length: %d\n",(int)strlen(instr));
		if(strcmp(instr,"\n")==0){
			printf("No input! try again\n");
		}else if(strcmp(instr,"1")==0){
			mkUser(&myuser);	
		}else if(strcmp(instr,"2")==0){
			if(login(&myuser)!=NULL){
				readFile(&myuser);
			}
		}else{
			printf("invalid input! try again\n");
		}
		printf("\n\nPress 1 to make an account.\n");
		printf("Press 2 to Sign in to access Some Files.\n");
		printf("Press ctrl+D to exit.\n");
	}
	printf("EOF entered, exiting\n");
	return 0;
} 



*/



