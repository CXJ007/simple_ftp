#include "client.h"

static int get_server_cmd(int c_fd, struct client_cmd *cmd);
static void func_lls_lpwd(struct client_cmd cmd);
static void func_lcd(struct client_cmd cmd);
static void func_ls_pwd(int fd, struct client_cmd cmd);
static int func_quit(int c_fd);
static void func_cd(int c_fd, struct client_cmd cmd);
static void func_get(int c_fd, struct client_cmd cmd);
static void func_put(int c_fd, struct client_cmd cmd);
static void func_help(void);

int client_shell(int fd, struct client_cmd cmd)
{
    int ret = OK;
    switch(cmd.cmdswitch){
                case CLIENT_CMD_NULL:
                    break;
                case CLIENT_CMD_QUIT:
                    func_quit(fd);
                    ret = QUIT;break;
                case CLIENT_CMD_HELP:
                    func_help();break;
                case CLIENT_CMD_LPWD:
                case CLIENT_CMD_LLS:
                    func_lls_lpwd(cmd);break;
                case CLIENT_CMD_LRM:
                    system(cmd.cmdbuf);break;
                case CLIENT_CMD_LCD:
                    func_lcd(cmd);break;
                case CLIENT_CMD_LS:
                case CLIENT_CMD_PWD:
                    func_ls_pwd(fd,cmd);break;
                case CLIENT_CMD_CD:
                    func_cd(fd,cmd);break;
                case CLIENT_CMD_GET:
                    func_get(fd,cmd);break;
                case CLIENT_CMD_PUT:
                    func_put(fd,cmd);break;
                    
            }
    return ret;
}


int get_terminal_cmd(int c_fd, struct client_cmd *cmd)
{
    char buf[128];
    char (*tmp)[FILE_NAME_LEN] = cmd->cmdargc;

    memset(cmd,0,sizeof(struct client_cmd));
    // puts("==>");
    // gets(buf);//不安全
    fputs("==>", stdout);
    while(fgets(buf, sizeof(char)*128, stdin) == NULL){
        if(get_server_cmd(c_fd,cmd) == OK){
            break;
        }
    }
    if(strcmp(buf, "\n") == 0){
        cmd->cmdswitch = CLIENT_CMD_NULL;
        return OK;
    }else if((strstr(buf, "lls")==buf) && ((*(buf+3)=='\n')||(*(buf+3)==32))){
        cmd->cmdswitch = CLIENT_CMD_LLS;
        strcpy(cmd->cmdbuf,buf+1);
        return OK;
    }else if(strstr(buf, "lpwd") == buf){
        cmd->cmdswitch = CLIENT_CMD_LPWD;
        strcpy(cmd->cmdbuf,buf+1);
        return OK;
    }else if((strstr(buf, "lrm")==buf) && (*(buf+3)==32)){
        cmd->cmdswitch = CLIENT_CMD_LRM;
        strcpy(cmd->cmdbuf,buf+1);
        return OK;
    }else if((strstr(buf, "lcd")==buf) && (*(buf+3)==32)){
        char *p = NULL;
        cmd->cmdswitch = CLIENT_CMD_LCD;
        p = strtok(buf, " /");
        while(p){
            //printf("qweqw %s\n",p);
            strcpy(*(tmp+cmd->cmdnum), p); 
            cmd->cmdnum++;
            p = strtok(NULL, " /");
        }
        //printf("%s    %s\n", *cmd->cmdargc, *(cmd->cmdargc+1));
        return OK;
    }else if((strstr(buf, "ls")==buf) && ((*(buf+2)=='\n')||(*(buf+2)==32))){
        cmd->cmdswitch = CLIENT_CMD_LS;
        strcpy(cmd->cmdbuf,buf);
        return OK;
    }else if(strstr(buf, "pwd") == buf){
        cmd->cmdswitch = CLIENT_CMD_LS;
        strcpy(cmd->cmdbuf,buf);
        return OK;
    }else if((strcmp(cmd->cmdbuf, "quit")==0) || (strcmp(buf, "quit\n")==0)){
        cmd->cmdswitch = CLIENT_CMD_QUIT;
        return OK;
    }else if((strstr(buf, "cd")==buf) && (*(buf+2)==32)){
        cmd->cmdswitch = CLIENT_CMD_CD;
        strcpy(cmd->cmdbuf,buf);
        return OK;
    }else if((strstr(buf, "get")==buf) && (*(buf+3)==32)){
        char *p = NULL;
        cmd->cmdswitch = CLIENT_CMD_GET;
        strcpy(cmd->cmdbuf,buf);
        p = strtok(buf, " ");
        while(p){
            //printf("qweqw %s\n",p);
            strcpy(*(tmp+cmd->cmdnum), p); 
            cmd->cmdnum++;
            p = strtok(NULL, " /");
        }
        return OK;
    }else if((strstr(buf, "put")==buf) && (*(buf+3)==32)){
        char *p = NULL;
        cmd->cmdswitch = CLIENT_CMD_PUT;
        strcpy(cmd->cmdbuf,buf);
        p = strtok(buf, " ");
        while(p){
            //printf("qweqw %s\n",p);
            strcpy(*(tmp+cmd->cmdnum), p); 
            cmd->cmdnum++;
            p = strtok(NULL, " /");
        }
        return OK;
    }else if(strcmp(buf, "help\n") == 0){
        cmd->cmdswitch = CLIENT_CMD_HELP;
        strcpy(cmd->cmdbuf,buf);
        return OK;
    }

    return ERR;
}

static int get_server_cmd(int c_fd, struct client_cmd *cmd)
{
    int ret;
    char buf[40];

    memset(buf, 0, sizeof(buf));

    ret = recv(c_fd, buf, sizeof(buf), MSG_DONTWAIT);//MSG_DONTWAIT
    if((ret==-1) && (errno!=EAGAIN)){
        perror("recv");
    }
    
    if(strcmp("quit", buf) == 0){
        ret = OK;
        strcpy(cmd->cmdbuf, "quit");
    }else{
        ret = ERR; 
    }
    
    return ret;
}



static int func_quit(int c_fd)
{
    char buf[10];
    // while(recv(c_fd, buf, sizeof(buf), MSG_PEEK) > 0)
    //     ;
    return QUIT;
}

static void func_lls_lpwd(struct client_cmd cmd)
{
    FILE *fd;
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    fd = popen(cmd.cmdbuf, "r");
    fread(buf, sizeof(buf), 1, fd);
    //fwrite(buf, sizeof(buf), 1, stdout);
    printf("%s",buf);
    //write(STDOUT_FILENO, buf, sizeof(buf));
    pclose(fd);
}

static void func_lcd(struct client_cmd cmd)
{
    int ret;
    char tmpbuf[FILE_NAME_LEN];
    char path[FILE_NAME_LEN];
    char *p = NULL;
    char tmpp[FILE_NAME_LEN];
    memset(path, 0 ,sizeof(path));
    memset(tmpbuf, 0 ,sizeof(tmpbuf));

    strncpy(path,*(cmd.cmdargc+1),strlen(*(cmd.cmdargc+1))-1);
    if(strcmp(path, "..")==0 ||strcmp(path, ".")==0){
        while(--cmd.cmdnum){
            //printf("%d\n",cmd.cmdnum);
            getcwd(tmpbuf, sizeof(tmpbuf));
            //printf("%s\n", tmpbuf);
            p = strtok(tmpbuf, "/");
            while(p){
                strcpy(tmpp, p);
                p = strtok(NULL, "/");
            }
            getcwd(tmpbuf, sizeof(tmpbuf));
            //printf("%s   %s\n",tmpbuf, tmpp);
            p = strstr(tmpbuf,tmpp);
            //if(p == NULL)printf("err\n");
            //printf("p  %s\n", p);
            strncpy(path, tmpbuf, strlen(tmpbuf)-strlen(p)-1);
            //printf("path %s\n", path);
            ret = chdir(path); //多一个尾巴
            memset(path, 0, sizeof(path));
            if(ret == -1)
            {
                perror("chdir");
            }
        }
    }else{
        ret = chdir(path); //多一个尾巴
        if(ret == -1)
        {
            perror("chdir");
        }
    }
    //printf("%s\n", lastpath);
    //printf("\n");

}

static void func_ls_pwd(int c_fd, struct client_cmd cmd)
{
    int ret;
    char buf[1024];

    memset(buf, 0, sizeof(buf));
    ret = send(c_fd, cmd.cmdbuf,strlen(cmd.cmdbuf), 0);
    if(ret != strlen(cmd.cmdbuf)){
        printf("send cmd err\n");
    }
    ret = recv(c_fd, buf, sizeof(buf),0);
    printf("%s", buf);
}

static void func_cd(int c_fd, struct client_cmd cmd)
{
    int ret;
    
    ret = send(c_fd, cmd.cmdbuf,strlen(cmd.cmdbuf), 0);
    if(ret != strlen(cmd.cmdbuf)){
        printf("send cmd err\n");
    }
}

static void func_get(int c_fd, struct client_cmd cmd)
{
    int ret;
    int fd;
    char buf[1024];
    char cmdbuf[40];
    char (*p)[FILE_NAME_LEN] = cmd.cmdargc;
    memset(buf, 0, sizeof(buf));
    memset(cmdbuf, 0, sizeof(cmdbuf));


    strncpy(cmdbuf, *(p+1),strlen(*(p+1))-1);
    if(access(cmdbuf, F_OK) == 0){
        printf("file exists\n");
    }else{
        sprintf(cmdbuf, "%s %s", *p,*(p+1));
        ret = send(c_fd, cmd.cmdbuf,strlen(cmd.cmdbuf), 0);
        if(ret != strlen(cmd.cmdbuf)){
            printf("send cmd err\n");
        }
        read(c_fd, buf, sizeof(buf));
        if(strcmp(buf, "file does not exist") == 0){
            printf("server file does not exist\n");
        }else{
            memset(cmdbuf, 0, sizeof(cmdbuf));
            strncpy(cmdbuf, *(p+1),strlen(*(p+1))-1);
            fd = open(cmdbuf,O_CREAT|O_WRONLY);
            write(fd, buf, sizeof(buf));
            fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
            close(fd);
        }
    }
}

static void func_put(int c_fd, struct client_cmd cmd)
{
    int ret;
    int fd;
    char buf[1024];
    char cmdbuf[40];
    char (*p)[FILE_NAME_LEN] = cmd.cmdargc;

    memset(buf, 0, sizeof(buf));
    memset(cmdbuf, 0, sizeof(cmdbuf));

    strncpy(cmdbuf, *(p+1),strlen(*(p+1))-1);
    if(access(cmdbuf, F_OK) == -1){
        printf("file does not exist\n");
    }else{
        ret = send(c_fd, cmd.cmdbuf,strlen(cmd.cmdbuf), 0);
        if(ret != strlen(cmd.cmdbuf)){
            printf("send cmd err\n");
        }
        read(c_fd, buf, sizeof(buf));
        if(strcmp(buf, "file exist") == 0){
            printf("server file exist\n");
        }else{
            memset(cmdbuf, 0, sizeof(cmdbuf));
            strncpy(cmdbuf, *(p+1),strlen(*(p+1))-1);
            fd = open(cmdbuf,O_RDONLY);
            read(fd, buf,sizeof(buf));
            write(c_fd,buf, sizeof(buf));
            close(fd);
        }

    }

}

static void func_help(void)
{
    printf("============SIMPLE FTP=============\n");
    printf("  CMD:\n\thelp\n\tls\n\tcd\n\tpwd\n\tlls\n\tlcd\n\tlpwd\n\tget\n\tput\n");
    printf("AUTHOR:CXJ  LICENSE:GPL  DATE:2022/2/27\n");
    printf("===================================\n");
}
