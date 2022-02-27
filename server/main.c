#include "servershell.h"

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;
struct client_list listhead;



void sighand(int num)
{
    if(num == SIGCHLD){
        wait(NULL);
        //printf("wait..........\n");
    }
}

void handler(int signum, siginfo_t *info, void *context)
{
    if(signum == SIGRTMAX-1){
        char idbuf[INET_ADDRSTRLEN];
        pthread_mutex_lock(&list_mutex);
        rm_client(&listhead, info->si_pid,idbuf);
        pthread_mutex_unlock(&list_mutex);
    }else if(signum == SIGRTMAX-3){
        int fd;
        struct client_list *tmp = &listhead;
        char path[FILE_NAME_LEN];
        fd = open(FIFO_PATH,O_RDONLY);
        if(fd < 0){
            perror("open");
        }
        read(fd, path, sizeof(path));
        close(fd);
        while(tmp->next != NULL){
            tmp = tmp->next;
            if(tmp->pid == info->si_pid){
                strcpy(tmp->path, path);
                break;
            }
        }
    }
}


int main(int argc, char **argv)
{
    int ret;
    int sock_fd;
    int c_fd;
    pid_t pid;
    pthread_t p_id;
    pthread_attr_t attr;
    struct sockaddr_in s_addr;
    char addrbuf[INET_ADDRSTRLEN];
    struct sockaddr_in sock_addr;
    socklen_t len;
    struct sigaction act;
    memset(&s_addr, 0, sizeof(s_addr));
    memset(&sock_addr, 0, sizeof(sock_addr));
    memset(&listhead, 0, sizeof(listhead));
    
    signal(SIGCHLD, sighand);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    sigaction(SIGRTMAX-1, &act, NULL);
    sigaction(SIGRTMAX-3, &act, NULL);
    ret = mkfifo(FIFO_PATH,0777);
    if((ret==-1) && (errno!=EEXIST)){
        perror("mkfifo");
        exit(-1);
    }
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        perror("socket:");
        exit(-1);
    }
    ret = get_local_addr(addrbuf);
    if(ret == -1){
        printf("get_local_addr err\n");
        exit(-1);
    }
    s_addr.sin_family = AF_INET;
    if(argc == 1) s_addr.sin_port = 0;
    if(argc == 2) s_addr.sin_port = htons(atoi(argv[1]));
    s_addr.sin_addr.s_addr = inet_addr(addrbuf);
    ret = bind(sock_fd, (struct sockaddr *)&s_addr, sizeof(s_addr));
    if(ret == -1){
        perror("bind:");
        exit(-1);
    }
    len = sizeof(s_addr);
    getsockname(sock_fd, (struct sockaddr *)&s_addr, &len);
    printf("IP:%s  port:%d\n", addrbuf , ntohs(s_addr.sin_port));
    printf("===================================\n");
    strcpy(listhead.ipbuf,addrbuf);
    listhead.pid = ntohs(s_addr.sin_port);//port
    ret = listen(sock_fd, 20);
    len = sizeof(sock_addr);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&p_id, &attr, server_guard, (void *)&listhead);
    if(ret != 0){
        perror("pthread_creat:");
        exit(-1);
    }

    while(1){
        c_fd = accept(sock_fd, (struct sockaddr *)&sock_addr, &len);
        if(c_fd==-1){//会被信号中断
            if(errno!= EINTR)
                perror("accept:");
        }else{
            pid = fork();
            if(pid == 0){
                //printf("======CONNECT IP:%s PID:%d\n",inet_ntoa(sock_addr.sin_addr),getpid());
                ret = server_hand(c_fd);
                // char quitbuf[] = "quit";
                // write(c_fd,quitbuf,sizeof(quitbuf))这里奇怪的bug详细可以看static void func_kill(struct server_cmd cmd)
                union sigval value;
                sigqueue(getppid(),SIGRTMAX-1,value);
                //close(c_fd);
                _exit(ret);   
            }else if(pid > 0){
                char path[100];
                memset(path, 0, sizeof(path));
                getcwd(path,sizeof(path));
                pthread_mutex_lock(&list_mutex);
                add_client(&listhead, pid, c_fd, inet_ntoa(sock_addr.sin_addr),path);
                pthread_mutex_unlock(&list_mutex);
            }else{
                perror("fork:");
            }
        }
    }

    return 0;
}