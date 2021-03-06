#ifndef CLIENT_H
#define CLIENT_H

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
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>


#define DISCONNECT 0
#define OK 0
#define ERR -1
#define QUIT 1
//cmd
#define CLIENT_CMD_NULL 0
#define CLIENT_CMD_HELP 1 
#define CLIENT_CMD_LS   2
#define CLIENT_CMD_PWD  3
#define CLIENT_CMD_CD   4
#define CLIENT_CMD_LLS   5
#define CLIENT_CMD_LPWD  6
#define CLIENT_CMD_LCD   7
#define CLIENT_CMD_LRM   8
#define CLIENT_CMD_QUIT   9
#define CLIENT_CMD_GET   10
#define CLIENT_CMD_PUT   11


#define FILE_NAME_LEN 40

typedef struct client_cmd{
    int cmdswitch;
    char cmdbuf[10];
    int cmdnum;
    char cmdargc[10][FILE_NAME_LEN];
}client_cmd;

int get_terminal_cmd(int c_fd, struct client_cmd *cmd);
int client_shell(int fd, struct client_cmd cmd);


#endif