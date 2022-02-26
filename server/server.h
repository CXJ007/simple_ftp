#ifndef SERVER_H
#define SERVER_H

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

#include "servershell.h"

#define CLIENT_CMD_LS   0
#define CLIENT_CMD_PWD  1
#define CLIENT_CMD_CD   2

#endif