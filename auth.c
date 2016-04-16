#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <openssl/sha.h>



#define SALTING 1
#define SALT_LEN 10

typedef struct {
char name[100];
char pw[100];
} user;

char* record(char* data,char* dest);
int verify(char* hash,char* salt,char* pw);
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




//return 0 if user,pw match
//return 1 if no match
//return 2 if username match
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
					if ((strcmp(usr->name,fields[0])==0)&&(verify(fields[1],fields[2],usr->pw)==1)){
						fclose(fp);
						return 0;
					}else if(strcmp(usr->name,fields[0])==0){
						fclose(fp);
						return 2;
					}
				}else{
					if ((strcmp(usr->name,fields[0])==0)&&(strcmp(usr->pw,fields[1])==0)){
						fclose(fp);
						return 0;
					}
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
//if mode =0, user creation mode
//if mode=1, login mode
//return 1 if EOF entered
//return 0 if valid user
int enterinfo(user* usr,int mode){
	int valid=0;
	printf("Enter name (ctrl+D to exit):\n");
	while (fgets(usr->name,sizeof(usr->name),stdin)){
		//fgets(usr->name,sizeof(usr->name),stdin);
		strtok(usr->name,"\n");
		//printf("Username length: %d\n",(int)strlen(usr->name));
		
		if(strcmp(usr->name,"\n")==0){
			printf("No input! try again\n");
		}else if(strlen(usr->name)>8){
			printf("Username length %d too long! try again\n",(int)strlen(usr->name));
		}else{
			if(mode==0){
				if(isUser(usr)==2){
					printf("Username '%s' already taken! try again\n",usr->name);
				}else{
					strcpy(usr->pw,getpass("Enter password:\n"));
					strtok(usr->pw,"\n");
					if(strlen(usr->pw)==0){
						printf("No input! try again\n");
					}else{
						valid=1;
						break;
					}
				}
			}else {
				strcpy(usr->pw,getpass("Enter password:\n"));
				strtok(usr->pw,"\n");
				if(strlen(usr->pw)==0){
					printf("No input! try again\n");
				}else{
					valid=1;
					break;
				}
			}
		}
		printf("Enter name (ctrl+D to exit):\n");
	}
	if(valid){
		return 0;
	}else{
		return 1;
	}
}


//if login fail, return null user
//if login success, return filled usr struct
user* login(user* usr){
	
	int tries=1;
	int auth = 0;
	while((tries <= 3)&&(auth==0)){	
		enterinfo(usr,1);
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
	printf("Enter file path (ctrl+D to exit):\n");
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
				printf("enter file path (ctrl+D to exit):\n");
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
		if(strlen(line)>1){ 
			nlflag = 0;
		}else{
			nlflag=1;
		}
		//printf("nlflag: %d\n",nlflag);
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
	if(newlined("users.txt")==0){
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

//return 0 if usr created successfully
//return 1 if exit char recieved
//duplicate usernames are not allowed
int mkUser(user* usr){
	if(enterinfo(usr,0)==1){
		return 1;
	}else{
		addUser(usr);
		printf("User added successfully!\n");
	}
	return 0;
}

//generate random string length n for salt
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

//generate SHA1 hash
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

//return 1 if hash is verified
//return 0 if no match
int verify(char* shash,char* salt,char* pw){
	//printf("verifying password: %s\n",pw);
	
	//printf("verifying hash: %s\n",shash);
	//printf("verifying salt: %s\n",salt);
	
	char buf[SHA_DIGEST_LENGTH*2];
	unsigned char temp[SHA_DIGEST_LENGTH];
	
	hash(buf,temp,strcat(salt,pw));
	//printf("produced hash: %s\n",buf);
	if(strcmp(buf,shash)==0){
		return 1;
	}else{
		return 0;
	}
}

//add hashed user to record.	
char* record(char* data,char* dest){
	char randl[256];
	int siz=SALT_LEN;
	randStr(randl,&siz);
	char salt[256];
	strcpy(salt,randl);
	//printf("salt: %s\n",salt);
	
	char buf[SHA_DIGEST_LENGTH*2];
	unsigned char temp[SHA_DIGEST_LENGTH];
	hash(buf,temp,strcat(randl,data));

	//printf("prepending to %s: %s\n",data,randl);
	//printf("hash of '%s': %s \n",randl,buf);
	strcat(buf,":");
	return strcpy(dest,strcat(buf,salt));
}

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
			if(mkUser(&myuser)!=0){
				break;
			}
		}else if(strcmp(instr,"2")==0){
			if(login(&myuser)!=NULL){
				readFile(&myuser);
			}else{
				break;
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






