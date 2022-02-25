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

typedef struct client_list{
    struct client_list *next;
    int pid;
    char ipbuf[16];
}client_list;

typedef struct server_cmd{
    int cmdswitch;
    char cmdbuf[10];
    int cmdnum;
    char cmdargc[10][20];
}server_cmd;

int get_local_addr(char *addrbuf);
void *server_guard(void *argc);
int server_hand(int fd);
void add_client(struct client_list *head, int pid, char *ipbuf);
void rm_client(struct client_list *head, int pid, char *ipbuf);
void show_client(struct client_list *head);
int get_terminal_cmd(struct server_cmd *cmd);
void func_ls_pwd(struct server_cmd cmd);
void func_cd(struct server_cmd cmd);
void func_help(void);
void func_show(struct client_list *head);
void func_kill(struct server_cmd cmd);
void func_quit(struct client_list *head);

#endif