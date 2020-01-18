#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "mysql_f.h"

#define SERVER_PORT 8088
#define LISTEN_SIZE 10
#define BUFF_SIZE 100
#define N 25
#define FILE_SIZE 102400

int check_user(int client_fd);
void title(void);
int command_ls(int sockfd,char* dir_path);
int  command_get(int sockfd,char* filename);
int command_put(int sockfd,char* filename);
void get_fileName(char* filename);
static void sig_child(int singo);

void banner(void)
{
    FILE* fp;
    char banner[1000],temp_content[1000];

    fp = fopen("banner.txt","r");
    bzero(banner,1000);
    bzero(temp_content,1000);
    while((fgets(temp_content,1000,fp))!=NULL){
		strncat(banner,temp_content,strlen(temp_content));
        bzero(temp_content,1000);
	}
    printf("\n%s\n",banner);
}

int check_user(int client_fd){

    char buff[BUFF_SIZE],acc_buff[BUFF_SIZE],re_password[100],username[100],db_password[100],data[20];
    char* temp_ptr,success_,failed_;
    int result,i,if_login;
    char delims[] = ",";

    if_login=0;
    bzero(buff,BUFF_SIZE);
    bzero(acc_buff,BUFF_SIZE);
    bzero(re_password,sizeof(re_password));
    bzero(username,sizeof(username));
    bzero(db_password,sizeof(db_password));
    bzero(data,sizeof(data));

    while(recv(client_fd,acc_buff,BUFF_SIZE,0) >= 0)
    {
        strncat(buff,acc_buff,BUFF_SIZE);
        if(if_login ==0)
        {   
            //printf("%s\n",acc_buff);
            temp_ptr = strtok(buff,delims);
            strncat(username,temp_ptr,strlen(temp_ptr));
            for(i = 0;i<2;i++)
            {
                if(i==1)
                {   
                    strncat(re_password,temp_ptr,strlen(temp_ptr));
                    break;
                }
                temp_ptr = strtok(NULL, delims);
            }
            //printf("[1]%s\n",re_password);
            result = get_password(username,db_password);
            //printf("[2]%d\n",result);
            if(result==-1){
                snprintf(buff,sizeof(buff),"username error!");
                continue;
            }
            if(result ==1)
            {
                //printf("[5]%s\n",db_password);
                if(strncmp(db_password,re_password,strlen(db_password)) == 0)
                {
                    //printf("[6] mid\n");
                    if_login = 1;
                    snprintf(data,sizeof(data),"success");
                    //printf("[7]data %s\n",data);
                    if(send(client_fd,data,sizeof(data),0) < 0)
                    {
                        printf("[-]send error!exiting....\n");
                        exit(1);
                    }
                    return 1;
                }  
                else
                {
                    snprintf(data,sizeof(data),"failed");
                    if(send(client_fd,data,sizeof(data),0) < 0)
                    {
                        printf("[-]send error!exiting....\n");
                        exit(1);
                    }
                    continue;
                }
            }  
        }
    }

}

int command_ls(int sockfd,char* dir_path)
{
    FILE* fp;
    char content[1024],tmp_content[100],command[100];

    bzero(content,1024);
    bzero(tmp_content,100);
    bzero(command,100);
    snprintf(command,100,"ls %s > files.txt ",dir_path);
    system("rm -rf files.txt");
    system(command);

    if((fp=fopen("files.txt","r"))==NULL){
		printf("[-]failed to open file\n");
		return -1;
	}
 
    while((fgets(tmp_content,90,fp))!=NULL){
		strncat(content,tmp_content,strlen(tmp_content));
        bzero(tmp_content,100);
	}
    content[strlen(content)-1]='\0';
    //printf("[9]%s\n",content);
    if(send(sockfd,content,strlen(content),0) < 0){
        printf("[-]send error!\n");
        return -1;
    }
	fclose(fp);

}

int command_get(int sockfd,char* filename)
{
    FILE* fp;
    int filesize;
    char* file_content;
    char temp_content[1000],file_path[30];

    bzero(file_path,30);
    
    //snprintf(file_path,30,"files/%s",filename);
    if((fp=fopen(filename,"r"))==NULL){
		printf("[-]failed to open file\n");
		return -1;
	}

    fseek(fp,0,SEEK_END);
	filesize = ftell(fp);
	file_content=(char *)malloc(filesize);
    bzero(file_content,filesize);
    bzero(temp_content,1000);
	rewind(fp);
    while((fgets(temp_content,1000,fp))!=NULL){
		strncat(file_content,temp_content,strlen(temp_content));
        bzero(temp_content,1000);
	}
    //printf("[12]%s\n",file_content);
    if(send(sockfd,file_content,strlen(file_content),0) < 0){
        printf("[-]send error!exiting....\n");
        return -1;
    }
    printf("[*]send the %s successfully!\n",filename);
	fclose(fp);
    return 1;
}

void get_fileName(char* filename){
    char fin_fileName[50];
    bzero(fin_fileName,50);
    char* ret;
   
    ret = strrchr(filename,'/');
    strncpy(fin_fileName,&ret[1],strlen(ret)-1);
    //printf("[19]filename: %s\n",fin_fileName);
    bzero(filename,50);
    strncpy(filename,fin_fileName,50);
}

int command_put(int sockfd,char* filename){
    int fd;
    FILE* fp;
    char file_content[FILE_SIZE],file_path[30],put_file_name[50];

    bzero(put_file_name,50);
    strncpy(put_file_name,filename,50);
    get_fileName(filename);
    bzero(file_content,FILE_SIZE);
    while(recv(sockfd,file_content,FILE_SIZE,0)>=0)
    {
        snprintf(file_path,30,"files/%s",filename);
        if((fd=open(file_path,O_WRONLY|O_CREAT,222))<0)
        {
		    printf("[-]failed to open file\n");
		    return -1;
	    }
        //printf("[15]%s\n",put_file_name);
        //printf("[16]%s\n",file_content);
        if(write(fd,file_content,strlen(file_content)) == strlen(file_content)){
            printf("[*]get the %s successfully\n",put_file_name);
            return 1;
        }
    }
}

static void sig_child(int singo){
    pid_t pid;
    int stat;
    while ((pid=waitpid(-1,&stat,WNOHANG))>0)
    {
        printf("[*]%d exit success!\n",pid);
        printf("[*]waiting a client....\n");
    }
    
}

int main(){

    struct  sockaddr_in client_sock,server_sock;
    int sockfd,pid,sock_len,client_len,client_fd,result;
    char command[100],file_name[20],cliend_ip[20],file_path[100];

    sock_len = sizeof(server_sock);
    client_len = sizeof(client_sock);
    bzero(&server_sock,sock_len);
    bzero(command,100);
    bzero(file_name,20);
    server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sock.sin_port = htons(SERVER_PORT);
    server_sock.sin_family = AF_INET;
    volatile int true = 1;
    
    sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    bind(sockfd,(struct sockaddr *)&server_sock,sock_len);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(void*)&true,sizeof(true));
    listen(sockfd,LISTEN_SIZE);
    signal(SIGCHLD,sig_child);
    banner();
    printf("[*]waiting a client....\n");
    while(1){
        client_fd = accept(sockfd,(struct sockaddr *)&client_sock,&client_len);
        if((pid = fork()) == 0){
            if(check_user(client_fd) == 1){
                inet_ntop(AF_INET,&client_sock,cliend_ip,20);
                printf("[*]client ip is %s\n",cliend_ip);
                while(recv(client_fd,command,sizeof(command),0) >=0)
                {
        
                    if(strncmp("ls",command,2) == 0){
                        printf("[*]executing %s\n",command);
                        //printf("[8] command %s\n",command);
                        bzero(file_path,100);
                        strncpy(file_path,&command[3],strlen(command)-3);
                        //printf("[17] file_path %s\n",file_path);
                        result = command_ls(client_fd,file_path);
                        bzero(command,100);
                        if(result == -1)
                            continue;
                    }
                    if(strncmp("get",command,3) == 0)
                    {
                        printf("[*]executing %s\n",command);
                        bzero(file_name,20);
                        //printf("[11]%s\n",command);
                        strncpy(file_name,&command[4],strlen(command)-4);
                        //printf("[10]%s\n",file_name);
                        result = command_get(client_fd,file_name);
                        bzero(command,100);
                        if(result == -1)
                            continue;
                    }
                    if(strncmp("put",command,3) == 0)
                    {
                        printf("[*]executing %s\n",command);
                        //printf("[13]%s\n",command);
                        bzero(file_name,20);
                        strncpy(file_name,&command[4],strlen(command)-4);
                        //printf("[14]%s\n",file_name);
                        result = command_put(client_fd,file_name);
                        bzero(command,100);
                        if(result == -1)
                            continue;
                    }
                    if(strncmp("exit",command,4) == 0)
                    {
                        //printf("[18] %s\n",command);
                        exit(0);
                    }
                }
            }
        }
        close(client_fd);
    }

}

