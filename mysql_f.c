#include "mysql.h" 
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h> 
 

int connect_db(MYSQL * my_conn){
    mysql_init(my_conn);
    if (mysql_real_connect(my_conn, HOST, USERNAME, PASSWORD, DATABASE, 0, NULL, CLIENT_FOUND_ROWS))
        return 1;
    else
        return -1;
    
}

int select_data(MYSQL * my_conn,char* username,MYSQL_RES ** res_ptr){

    char sql_query[100];
    int res;

    snprintf(sql_query,sizeof(sql_query),"select password from ftp_user where username='%s'",username);
    res = mysql_query(my_conn,sql_query);
    //printf("%s\n",sql_query);
    if(res){
        return -1;
    }
    else{
        *res_ptr = mysql_store_result(my_conn);
        return 1; 
    }
}


int get_password(char* username,char* password){

    MYSQL my_conn;
    MYSQL_RES* res_ptr; 
    MYSQL_ROW result;
    //char* username = "sunsec";

    if(connect_db(&my_conn)){
        if(select_data(&my_conn,username,&res_ptr) > 0){
            if(res_ptr){
                result = mysql_fetch_row(res_ptr);
                strncat(password,result[0],strlen(result[0]));
                return 1;
            }
            else{
                printf("[-]null\n");
                return -1;
            }
        }
        else
        {
            printf("[-]select error!\n");
            return -2;
        }    
    }
    else
    {
        printf("[-]connect error\n");
        return -3;
    }
}


