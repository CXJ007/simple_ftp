#include "servershell.h"

static void show_client(struct client_list *head);
static int get_terminal_cmd(struct server_cmd *cmd);
static void func_ls_pwd(struct server_cmd cmd);
static void func_cd(struct server_cmd cmd);
static void func_help(void);
static void func_show(struct client_list *head);
static void func_kill(struct server_cmd cmd);
static void func_quit(struct client_list *head);
///////////////////////////father

void *server_guard(void *argc)
{
    int ret;
    struct client_list *plisthead= (client_list *)argc;
    struct server_cmd cmd;

    while(1){
        ret = get_terminal_cmd(&cmd);
        if(ret == ERR){
            printf("CMD NO FIND\n");
        }else{
            switch(cmd.cmdswitch){
                case SERVER_CMD_NULL:
                    break;
                case SERVER_CMD_HELP:
                    func_help();break;
                case SERVER_CMD_SHOW:
                    func_show(plisthead);break;
                case SERVER_CMD_KILL:
                    func_kill(cmd);break;
                    case SERVER_CMD_QUITE:
                    func_quit(plisthead);break;
                case SERVER_CMD_LS:
                case SERVER_CMD_PWD:
                    //system(cmd.cmdbuf);break;//1;用破坏缓冲区2;在有client时会把标注输出定义到其他地方3：popen  r 也会出现2情况
                    func_ls_pwd(cmd);break;
                case SERVER_CMD_CD:
                    func_cd(cmd);break;
            }
        }

    }
}

//一般情况下服务器不会主动杀的更别说杀自己了

static void func_quit(struct client_list *head)
{
    //show_client(struct client_list *head)
    struct client_list *tmp = head;
    union sigval value;
    char buf[20];

    memset(buf, 0,sizeof(buf)); 
    while(tmp->next != NULL){
        tmp = tmp->next;
        sigqueue(tmp->pid,SIGRTMAX-2,value);      
    }
    printf("================END================\n");
    sprintf(buf, "kill -9 %d", getpid());
    system(buf);//TIME_WAIT close（bug）
    pthread_exit(NULL);
}

static void func_kill(struct server_cmd cmd)
{
    union sigval value;
    int i;
    for(i=1;i<cmd.cmdnum;i++){
        sigqueue(atoi(*(cmd.cmdargc+i)),SIGRTMAX-2,value);
    }
}

static void func_show(struct client_list *head)
{
    show_client(head);
}

static void func_help(void)
{
    printf("============SIMPLE TFT=============\n");
    printf("  CMD:\n\thelp\n\tquit:quit server(ending service is dangerous)\n\t\
kill:kill pid (ending the client is dangerous)\n\tshow:ip,port,pid,path\n\t\
ls:same as shell\n\tpwd\n\tcd: cd .. / cd path(bug)\n");
    printf("AUTHOR:CXJ  LICENSE:GPL\n");
    printf("===================================\n");
}

static void func_ls_pwd(struct server_cmd cmd)
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

//bug strstr 比如输入lss会成功（我懒了）但是linux会提示 (修复)
static int get_terminal_cmd(struct server_cmd *cmd)
{
    char buf[128];
    char (*tmp)[FILE_NAME_LEN] = cmd->cmdargc;

    memset(cmd,0,sizeof(struct server_cmd));
    // puts("==>");
    // gets(buf);//不安全
    fputs("==>", stdout);
    while(fgets(buf, sizeof(char)*128, stdin) == NULL);
    if(strcmp(buf, "\n") == 0){
        cmd->cmdswitch = SERVER_CMD_NULL;
        return OK;
    }else if((strstr(buf, "ls")==buf) && ((*(buf+2)=='\n')||(*(buf+2)==32))){ //32是空格
        strcpy(cmd->cmdbuf,buf);
        cmd->cmdswitch = SERVER_CMD_LS;
        return OK;
    }else if(strcmp(buf, "pwd\n") == 0){
        strcpy(cmd->cmdbuf,buf);
        cmd->cmdswitch = SERVER_CMD_PWD;
        return OK;
    }else if((strstr(buf, "cd")==buf) && (*(buf+2)==32)){
        char *p = NULL;
        cmd->cmdswitch = SERVER_CMD_CD;
        p = strtok(buf, " /");
        while(p){
            //printf("qweqw %s\n",p);
            strcpy(*(tmp+cmd->cmdnum), p); 
            cmd->cmdnum++;
            p = strtok(NULL, " /");
        }
        //printf("%s    %s\n", *cmd->cmdargc, *(cmd->cmdargc+1));
        return OK;
    }else if(strcmp(buf, "help\n") == 0){
        cmd->cmdswitch = SERVER_CMD_HELP;
        return OK;
    }else if(strcmp(buf, "show\n") == 0){
        cmd->cmdswitch = SERVER_CMD_SHOW;
        return OK;
    }else if((strstr(buf, "kill")==buf) && (*(buf+4)==32)){
        char *p = NULL;
        cmd->cmdswitch = SERVER_CMD_KILL;
        p = strtok(buf, " ");
        while(p){
            //printf("qweqw %s\n",p);
            strcpy(*(tmp+cmd->cmdnum), p); 
            cmd->cmdnum++;
            p = strtok(NULL, " ");
        }
        return OK;
    }else if(strcmp(buf, "quit\n") == 0){
        cmd->cmdswitch = SERVER_CMD_QUITE;
        return OK;
    }
    return ERR;
}




void add_client(struct client_list *head, int pid, char *ipbuf)
{
    client_list *node;
    if(head == NULL)
    {
        printf("head NULL\n");
    }
    node = malloc(sizeof(struct client_list));
    node->next = head->next;
    node->pid = pid;
    strcpy(node->ipbuf,ipbuf);
    head->next = node;
}

void rm_client(struct client_list *head, int pid, char *ipbuf)
{
    client_list *tmp;
    client_list *node = head;
    while(node->next != NULL){
        tmp = node;
        node = node->next;
        if(node->pid == pid){
            tmp->next = node->next;
            strcpy(ipbuf, node->ipbuf);
            free(node);
            break;
        }
   }
}

static void show_client(struct client_list *head)
{
    client_list *tmp = head;
    printf("======SERVER IP:%s port:%d\n",tmp->ipbuf,tmp->pid);
    while(tmp->next != NULL){
        tmp = tmp->next;
        printf("======CLIENT IP:%s pid:%d\n",tmp->ipbuf,tmp->pid);
    }
}

int get_local_addr(char *addrbuf)
{
    int ret;
    pid_t pid;
    struct ifaddrs *ifaddrs = NULL;
    struct in_addr tmpaddr;
    char *tmpbuf;


   ret = getifaddrs(&ifaddrs);
   if(ret == -1){
       perror("getifaddrs err:");
       exit(-1);
   }
   ret = -1;
   while(ifaddrs != NULL){
       if(ifaddrs->ifa_addr->sa_family == AF_INET){
            tmpaddr=((struct sockaddr_in *)ifaddrs->ifa_addr)->sin_addr;
            tmpbuf = inet_ntoa(tmpaddr);
            strcpy(addrbuf,tmpbuf);
            ret = 1;
       }
       ifaddrs = ifaddrs->ifa_next; 
   }
   freeifaddrs(ifaddrs);
   return ret;
}
