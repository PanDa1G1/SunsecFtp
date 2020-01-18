#ifndef _feed_dog_h
#define _feed_dog_h
#include "mysql.h" 
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h> 
 
#define HOST "localhost" 
#define USERNAME "root" 
#define PASSWORD "root" 
#define DATABASE "ftp"

#ifdef __cplusplus
extern "C" {
#endif
	int connect_db(MYSQL * my_conn);
    extern int get_password(char* username,char* password);
    extern int select_data(MYSQL * my_conn,char* username,MYSQL_RES ** res_ptr);
    extern int close_db(MYSQL * my_conn);
#ifdef __cplusplus
}
#endif
#endif