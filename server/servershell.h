#ifndef SERVERSHELL_H
#define SERVERSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>


#define FIFO_PATH "/home/cxj/Desktop/FTP/server/fifo"
//cmd
#define SERVER_CMD_NULL 0
#define SERVER_CMD_HELP 1 
#define SERVER_CMD_SHOW 2 
#define SERVER_CMD_LS   3
#define SERVER_CMD_PWD  4
#define SERVER_CMD_CD   5
#define SERVER_CMD_KILL   6
#define SERVER_CMD_QUITE   7


#define DISCONNECT 0
#define OK 0
#define ERR -1

#define FILE_NAME_LEN 40

typedef struct client_list{
    struct client_list *next;
    int pid;
    int fd;
    char ipbuf[16];
    char path[100];
}client_list;


typedef struct server_cmd{
    int cmdswitch;
    char cmdbuf[10];
    int cmdnum;
    char cmdargc[10][FILE_NAME_LEN];
}server_cmd;

int get_local_addr(char *addrbuf);
void *server_guard(void *argc);
int server_hand(int fd);
void add_client(struct client_list *head, int pid, int fd, char *ipbuf, char *path);
void rm_client(struct client_list *head, int pid, char *ipbuf);

#endif