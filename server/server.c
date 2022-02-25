#include "server.h"

///////////////////////////son

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

    act.sa_flags = 0;
    act.sa_sigaction = handler;
    sigaction(SIGRTMAX-2, &act, NULL);
    sw = 1;
    while(sw){
        len = recv(fd, buf, sizeof(buf), 0);
        if(len<=0 && errno!=EINTR){
            return DISCONNECT;
        }
    }
    return DISCONNECT;
}