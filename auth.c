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

		strtok(usr->name,"\n");

		
		if(strcmp(usr->name,"\n")==0){
			printf("No input! try again\n");
		}else if(strlen(usr->name)>8){
			printf("Username length %d too long! try again\n",(int)strlen(usr->name));
		}else{
			if(mode==0){
				int test = isUser(usr);
				if((test==2)||(test==0)){
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
			//no permission specified -> deny permission
			if (strlen(fields[2])==0){
			}else if(strlen(fields[1])==0){

			}
			if ((strcmp(usr->name,fields[1])==0)||(strlen(fields[1])==0)){
				if ((strcmp(fname,fields[2])==0)||(strlen(fields[2])==0)){
					if(strcmp("PERMIT",fields[0])==0){
						return 0;
					}else if(strcmp("DENY",fields[0])==0){
						return 1;
					}else{
						return 1;
					}
				}
			}
			
		}
	}
	if(feof(fp)){
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
	int nlflag=0;
	int n=0;
	char line[256];
	while(fgets(line,sizeof(line),fp)){
		n++;
		if((strcmp(line,"\n")==0)||(strlen(line)==0)){ 
			nlflag = 0;
		}else{
			nlflag=1;
		}
		//printf("nlflag: %d\n",nlflag);
	}
	if(feof(fp)){
		fclose(fp);
		if(n>0){
			if(nlflag==0){
			return 0;
			}else if(nlflag==1){
				return 1;
			}else{
				printf("error!\n");
				return 2;
			}
		}else{
			return 1; //empty file
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
		printf("EOF read: exiting to main menu\n");
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
	char buf[SHA_DIGEST_LENGTH*2];
	unsigned char temp[SHA_DIGEST_LENGTH];
	hash(buf,temp,strcat(salt,pw));
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
	char buf[SHA_DIGEST_LENGTH*2];
	unsigned char temp[SHA_DIGEST_LENGTH];
	hash(buf,temp,strcat(randl,data));
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
	int tries=0;
	while(fgets(instr,sizeof(instr),stdin)){
		strtok(instr,"\n");
		if(strcmp(instr,"\n")==0){
			printf("No input! try again\n");
		}else if(strcmp(instr,"1")==0){
			if(mkUser(&myuser)!=0);
		}else if(strcmp(instr,"2")==0){
			if(login(&myuser)!=NULL){
				readFile(&myuser);
			}else{
				tries++;
				break;
			}
		}else{
			printf("invalid input! try again\n");
		}
		printf("\n\nPress 1 to make an account.\n");
		printf("Press 2 to Sign in to access Some Files.\n");
		printf("Press ctrl+D to exit.\n");
	}
	if(tries==0){
		printf("EOF entered, exiting\n");
	}else{
		printf("3 tries used: exiting\n");
	}
	

	
	return 0;
} 






