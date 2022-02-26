#include "client.h"


static void func_lls_lpwd(struct client_cmd cmd);
static void func_lcd(struct client_cmd cmd);
static void func_ls_pwd(int fd, struct client_cmd cmd);

void client_shell(int fd, struct client_cmd cmd)
{
    switch(cmd.cmdswitch){
                case CLIENT_CMD_NULL:
                    break;
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
            }
}


int get_terminal_cmd(struct client_cmd *cmd)
{
    char buf[128];
    char (*tmp)[FILE_NAME_LEN] = cmd->cmdargc;

    memset(cmd,0,sizeof(struct client_cmd));
    // puts("==>");
    // gets(buf);//不安全
    fputs("==>", stdout);
    while(fgets(buf, sizeof(char)*128, stdin) == NULL);
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
    }

    return ERR;
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

static void func_ls_pwd(int fd, struct client_cmd cmd)
{
    int ret;
    char buf[1024];

    memset(buf, 0, sizeof(buf));
    ret = write(fd, cmd.cmdbuf,strlen(cmd.cmdbuf));
    if(ret != strlen(cmd.cmdbuf)){
        printf("send cmd err\n");
    }
    ret = read(fd, buf, sizeof(buf));
    printf("%s", buf);
}