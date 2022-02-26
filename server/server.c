#include "server.h"

///////////////////////////son

static int get_client_cmd(struct server_cmd *cmd, char *buf);
static void func_ls_pwd(int fd, struct server_cmd cmd);



int sw;

static void handler(int signum, siginfo_t *info, void *context)
{
    sw = 0;  
}

int server_hand(int fd)
{
    char buf[128];
    int len;
    struct sigaction act;
    struct server_cmd recv_cmd;

    act.sa_flags = 0;
    act.sa_sigaction = handler;
    sigaction(SIGRTMAX-2, &act, NULL);
    sw = 1;
    while(sw){
        len = read(fd, buf, sizeof(buf));
        if(len<=0 && errno!=EINTR){
            return DISCONNECT;
        }
        get_client_cmd(&recv_cmd, buf);
        switch(recv_cmd.cmdswitch){
            case CLIENT_CMD_LS:
            case CLIENT_CMD_PWD:
                func_ls_pwd(fd, recv_cmd);break;
        }
    }
    return DISCONNECT;
}



static int get_client_cmd(struct server_cmd *cmd, char *buf)
{
    char (*tmp)[FILE_NAME_LEN] = cmd->cmdargc;

    memset(cmd,0,sizeof(struct server_cmd));

    if((strstr(buf, "ls")==buf) && ((*(buf+2)=='\n')||(*(buf+2)==32))){ //32是空格
        strcpy(cmd->cmdbuf,buf);
        cmd->cmdswitch = CLIENT_CMD_LS;
        return OK;
    }else if(strcmp(buf, "pwd\n") == 0){
        strcpy(cmd->cmdbuf,buf);
        cmd->cmdswitch = CLIENT_CMD_PWD;
        return OK;
    }else if((strstr(buf, "cd")==buf) && (*(buf+2)==32)){
        char *p = NULL;
        cmd->cmdswitch = CLIENT_CMD_CD;
        p = strtok(buf, " /");
        while(p){
            //printf("qweqw %s\n",p);
            strcpy(*(tmp+cmd->cmdnum), p); 
            cmd->cmdnum++;
            p = strtok(NULL, " /");
        }
        //printf("%s    %s\n", *cmd->cmdargc, *(cmd->cmdargc+1));
        return OK;
    }
    return ERR;
}

static void func_ls_pwd(int s_fd, struct server_cmd cmd)
{
    FILE *fd;
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    fd = popen(cmd.cmdbuf, "r");
    fread(buf, sizeof(buf), 1, fd);
    //fwrite(buf, sizeof(buf), 1, stdout);
    write(s_fd, buf, sizeof(buf));
    //write(STDOUT_FILENO, buf, sizeof(buf));
    pclose(fd);
}