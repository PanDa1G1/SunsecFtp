


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>

#define COMMAND_SIZE 100
#define SERVER_PORT 8088
#define FILE_SIZE 1024

void banner(void);
int ftp_login(char* ip);
void get_command(int sockfd);
int command_get(int sockfd,char* filename);
int command_put(int sockfd,char* filename);
void get_fileName(char* filename);

void banner(void)
{
    FILE* fp;
    char banner[1000],temp_content[1000];

    bzero(banner,1000);
    bzero(temp_content,1000);
    fp = fopen("banner.txt","r");
    while((fgets(temp_content,1000,fp))!=NULL){
		strncat(banner,temp_content,strlen(temp_content));
        bzero(temp_content,1000);
	}
    printf("\n%s\n",banner);


    printf("\n===============usage:===============\n");
    printf("\n     hint:必须先进行登陆\n");
    printf("\n     login <ip> :登陆服务器\n");
    printf("\n     exit:离开FTP服务器\n");
    printf("\n     ls: 显示FTP服务器的文件列表\n");
    printf("\n     get <file>：从FTP服务器下载文件\n");
    printf("\n     put <file>:上传文件到FTP服务器\n");
    printf("\n===============end:=================\n");


}

int ftp_login(char* ip){

    struct  sockaddr_in server_sock;
    int sockfd,sock_len,if_login;
    char username[10],password[100],data[200],reply[100],data_[20];

    if_login=0;
    sock_len = sizeof(server_sock);
    bzero(&server_sock,sock_len);
    bzero(username,10);
    bzero(password,10);
    bzero(data_,20);
    server_sock.sin_family = AF_INET;
    server_sock.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET,ip,&server_sock.sin_addr);
    sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
login:
    if(if_login == 0)
    {
        printf("ftp_client> login name:");
        if(fgets(username,COMMAND_SIZE,stdin) == NULL)
        {
            printf("[-]Input error! exiting.....\n");
            exit(1);
        }
        username[strlen(username)-1]='\0';
        printf("ftp_client> password:");
        if(fgets(password,100,stdin) == NULL)
        {
            printf("[-]Input error! exiting.....\n");
            exit(1);
        }
        password[strlen(password)-1]='\0';
        if(connect(sockfd,(struct sockaddr *)&server_sock,sock_len)<0)
        {
            printf("[-] connect error! exiting ......\n");
            exit(1);
        }

        snprintf(data,sizeof(data),"%s,%s",username,password);
        //printf("%s\n",data);
        if(send(sockfd,data,strlen(data),0)<0)
        {
            printf("[-] send error! exiting.....\n");
            exit(1);
        }
        if(recv(sockfd,data_,sizeof(data_),0) >=0){
            //printf("%s\n",data_);
            if(strncmp("success",data_,7) == 0){
                printf("[*]login successfully!\n");
                if_login=1;
                get_command(sockfd);
            }
            else{
                printf("[-]login failed!\n");
                goto login;
            }
        }

    }

}
void get_fileName(char* filename){
    char fin_fileName[50];
    bzero(fin_fileName,50);
    char* ret;
   
    ret = strrchr(filename,'/');
    strncpy(fin_fileName,&ret[1],strlen(ret)-1);
    //printf("[6]filename: %s\n",fin_fileName);
    bzero(filename,50);
    strncpy(filename,fin_fileName,50);
}
int command_get(int sockfd,char* filename){
    
    int fd;
    FILE* fp;
    char file_content[FILE_SIZE],fin_path[100];
    char get_file_name[50];

    bzero(get_file_name,50);
    strncpy(get_file_name,filename,50);
    get_fileName(filename);
    //printf("[7]filename: %s\n",filename);
    bzero(file_content,FILE_SIZE);
    bzero(fin_path,100);
    snprintf(fin_path,100,"download_files/%s",filename);
    while(recv(sockfd,file_content,FILE_SIZE,0)>=0)
    {
        if((fd=open(fin_path,O_WRONLY|O_CREAT,222))<0)
        {
		    printf("[-]failed to open file\n");
		    return -1;
	    }
        //printf("[2]%s\n",get_file_name);
        //printf("[3]%s\n",file_content);
        if(write(fd,file_content,strlen(file_content)) == strlen(file_content)){
            printf("[*]get the %s success\n",get_file_name);
            bzero(file_content,FILE_SIZE);
            return 1;
        }
    }

}

int command_put(int sockfd,char* filename){
    FILE* fp;
    int filesize;
    char* file_content;
    char temp_content[1000],file_path[30];

    bzero(file_path,30);
    //printf("[8]filename: %s\n",filename);
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

    if((fgets(file_content,filesize,fp))==NULL){
        printf("[-]get file content failed!\n");
        return -1;
	}
    //printf("[4]%s\n",file_content);
    if(send(sockfd,file_content,strlen(file_content),0) < 0){
        printf("[-]send error!\n");
        return -1;
    }
    printf("[*]send the %s successfully!\n",filename);
	fclose(fp);
    return 1;
}

void get_command(int sockfd)
{
    char command[100],send_command[100],content_[FILE_SIZE],file_name[50],send_content[20];
    int result;

    bzero(send_command,100);
    bzero(content_,FILE_SIZE);
    bzero(file_name,50);
    bzero(send_content,20);

    while(1)
    {
        bzero(command,100);
        printf("ftp_client>");
        if(fgets(command,COMMAND_SIZE,stdin) == NULL)
        {
            printf("[-]Input error!\n");
            continue;
        }
        command[strlen(command)-1]='\0';
        if(strncmp("ls",command,2) == 0){
            if(send(sockfd,command,strlen(command),0)<0){
                printf("[-] send error!\n");
                continue;
            }
            bzero(content_,FILE_SIZE);
            if(recv(sockfd,content_,sizeof(content_),0)>=0)
            {
                printf("%s\n",content_);
            }

        }
        if(strncmp("get",command,3) == 0)
        {
            bzero(file_name,50);
            strncpy(file_name,&command[4],strlen(command)-4);
            
            //printf("[1]%s\n",file_name);
            if(send(sockfd,command,strlen(command),0)<0){
                printf("[-] send error!\n");
                continue;
            }
            result = command_get(sockfd,file_name);
            if(result == -1)
                continue;
        }
        if(strncmp("put",command,3) == 0)
        {   
            //printf("[5]%s\n",command);
            bzero(file_name,50);
            strncpy(file_name,&command[4],strlen(command)-4);
            //printf("[3]%s\n",file_name);
            if(send(sockfd,command,strlen(command),MSG_DONTWAIT)<0){
                printf("[-] send error!\n");
                continue;
            }
            result = command_put(sockfd,file_name);
            if(result == -1)
                continue;
        }

        if(strncmp("exit",command,4) == 0)
        {
            if(send(sockfd,command,strlen(command),0)<0){
                printf("[-] send error! exiting.....\n");
                exit(1);
            }
            exit(0);
            
        }

    }
}

int main(){

    char command[COMMAND_SIZE],ip[20];
    int temp_len;

    banner();
    bzero(command,COMMAND_SIZE);
    bzero(ip,20);
    printf("ftp_client>");
    if(fgets(command,COMMAND_SIZE,stdin) == NULL){
        printf("[-]Input error! exiting.....\n");
        exit(1);
    }
    command[strlen(command)-1]='\0';

    if(strncmp(command,"login",5) == 0){
        temp_len = strlen(command);
        strncat(ip,&command[6],temp_len - 6);
        //printf("%s\n",ip);
        ftp_login(ip);
    }

    return 0;
}
