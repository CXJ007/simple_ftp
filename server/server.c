#include "server.h"

///////////////////////////son

static int get_client_cmd(struct server_cmd *cmd, char *buf);
static void func_ls_pwd(int fd, struct server_cmd cmd);
static void func_cd(struct server_cmd cmd);
static void func_get(int s_fd, struct server_cmd cmd);
static void func_put(int s_fd, struct server_cmd cmd);

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
        //printf("%s cmd", buf);
        get_client_cmd(&recv_cmd, buf);
        switch(recv_cmd.cmdswitch){
            case CLIENT_CMD_LS:
            case CLIENT_CMD_PWD:
                func_ls_pwd(fd, recv_cmd);break;
            case CLIENT_CMD_CD:
                func_cd(recv_cmd);break;
            case CLIENT_CMD_GET:
                func_get(fd, recv_cmd);break;
            case CLIENT_CMD_PUT:
                func_put(fd, recv_cmd);break;
        }
    }
    return DISCONNECT;
}



static int get_client_cmd(struct server_cmd *cmd, char *buf)
{
    char *p;
    char (*tmp)[FILE_NAME_LEN] = cmd->cmdargc;

    p = strtok(buf, "\n");
    strcpy(buf,p);

    memset(cmd,0,sizeof(struct server_cmd));

    if((strstr(buf, "ls")==buf)){
        strcpy(cmd->cmdbuf,p);
        cmd->cmdswitch = CLIENT_CMD_LS;
        return OK;
    }else if(strcmp(buf, "pwd") == 0){
        strcpy(cmd->cmdbuf,p);
        cmd->cmdswitch = CLIENT_CMD_PWD;
        return OK;
    }else if((strstr(buf, "cd")==buf) && (*(buf+2)==32)){
        char *p = NULL;
        cmd->cmdswitch = CLIENT_CMD_CD;
        p = strtok(buf, " /");
        while(p){
            strcpy(*(tmp+cmd->cmdnum), p); 
            cmd->cmdnum++;
            p = strtok(NULL, " /");
        }
        //printf("%s    %s\n", *cmd->cmdargc, *(cmd->cmdargc+1));
        return OK;
    }else if((strstr(buf, "get")==buf) && (*(buf+3)==32)){
        cmd->cmdswitch = CLIENT_CMD_GET;
        p = strtok(buf, " ");
        while(p){
            strcpy(*(tmp+cmd->cmdnum), p); 
            cmd->cmdnum++;
            p = strtok(NULL, " ");
        }
        //printf("get   %s    %s\n", *cmd->cmdargc, *(cmd->cmdargc+1));
        return OK;
    }else if((strstr(buf, "put")==buf) && (*(buf+3)==32)){
        cmd->cmdswitch = CLIENT_CMD_PUT;
        p = strtok(buf, " ");
        while(p){
            strcpy(*(tmp+cmd->cmdnum), p); 
            cmd->cmdnum++;
            p = strtok(NULL, " ");
        }
        //printf("get   %s    %s\n", *cmd->cmdargc, *(cmd->cmdargc+1));
        return OK;
    }

    return ERR;
}

static void func_ls_pwd(int s_fd, struct server_cmd cmd)
{
    FILE *fd;
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    //printf("cmd:%s  len:%d\n",cmd.cmdbuf,strlen(cmd.cmdbuf));
    fd = popen(cmd.cmdbuf, "r");
    fread(buf, sizeof(buf), 1, fd);
    //fwrite(buf, sizeof(buf), 1, stdout);
    write(s_fd, buf, sizeof(buf));
    //write(STDOUT_FILENO, buf, sizeof(buf));
    pclose(fd);
}

//bug strstr是从前往后，应从后往前不过我懒了
//int readlink(const char * path ,char * buf,size_t bufsiz);后面发现这个
static void func_cd(struct server_cmd cmd)
{
    int ret;
    char tmpbuf[FILE_NAME_LEN];
    char path[FILE_NAME_LEN];
    char *p = NULL;
    char tmpp[FILE_NAME_LEN];
    memset(path, 0 ,sizeof(path));
    memset(tmpbuf, 0 ,sizeof(tmpbuf));

    strncpy(path,*(cmd.cmdargc+1),strlen(*(cmd.cmdargc+1)));
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
        ret = chdir(path);
        if(ret == -1)
        {
            perror("chdir");
        }
    }
    //同步信息
    memset(path, 0 ,sizeof(path));
    getcwd(path, sizeof(path));
    union sigval value;                                          
    sigqueue(getppid(),SIGRTMAX-3,value);//sigqueue发送字符串只能在 共享内存或者 同一进程下 才可以发送奇怪的知识
    int fd,len; 
    fd = open(FIFO_PATH,O_WRONLY);//小心工作目录切换了
    if(fd < 0){
        perror("open1");
    }
    write(fd, path, sizeof(path));
    close(fd);
}

static void func_get(int s_fd, struct server_cmd cmd)
{
    int ret;
    int fd;
    char buf[1024];
    char path[FILE_NAME_LEN];
    memset(buf, 0, sizeof(buf));
    memset(path, 0, sizeof(path));

    strcpy(path, *(cmd.cmdargc+1));
    if(access(path, F_OK) == -1){
        strcpy(path, "file does not exist");
        write(s_fd,path, sizeof(path));
    }else{
        fd = open(path,O_RDONLY);
        read(fd, buf,sizeof(buf));
        write(s_fd,buf, sizeof(path));
        close(fd);
    }
}

static void func_put(int s_fd, struct server_cmd cmd)
{
    int ret;
    int fd;
    char buf[1024];
    char path[FILE_NAME_LEN];
    memset(buf, 0, sizeof(buf));
    memset(path, 0, sizeof(path));

    strcpy(path, *(cmd.cmdargc+1));
    if(access(path, F_OK) == 0){
        strcpy(path, "file exist");
        write(s_fd,path, sizeof(path));
    }else{
        strcpy(path, "file no exist");
        write(s_fd,path, sizeof(path));
        read(s_fd, buf, sizeof(buf));
        strcpy(path, *(cmd.cmdargc+1));
        fd = open(path,O_CREAT|O_WRONLY);
        write(fd, buf, sizeof(buf));
        fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        close(fd);
    }

}