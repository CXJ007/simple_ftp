#include "client.h"

int main(int argc, char **argv)
{
    int flags;
    int ret;
    int sock_fd;
    struct sockaddr_in c_addr;
    struct client_cmd cmd;

    memset(&c_addr, 0, sizeof(c_addr));

    if(argc != 3){
        printf("need ip and port\n");
        exit(-1);
    }

    
    flags=fcntl(fileno(stdin),F_GETFL);
    flags|=O_NONBLOCK;
    fcntl(fileno(stdin),F_SETFL,flags);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    c_addr.sin_family = AF_INET;
    c_addr.sin_addr.s_addr = inet_addr(argv[1]);
    c_addr.sin_port = htons(atoi(argv[2]));
    ret = connect(sock_fd, (struct sockaddr*)&c_addr, sizeof(c_addr));
    if(ret == -1){
        perror("connect");
        exit(-1);
    }
    printf("==============CONNECT==============\n");
    while(1){
        ret = get_terminal_cmd(sock_fd, &cmd);
        if(ret == ERR){
            printf("CMD NO FIND\n");
        }else{
            ret = client_shell(sock_fd, cmd);
            if(ret == QUIT)
            {
                break;
            }
        }
    }

    close(sock_fd);
    return 0;
}