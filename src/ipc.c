#include "dpdk_fast_switch/ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/dpdk_switch.sock"

static int server_fd = -1;
static int client_fd = -1;
static pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void* ipc_thread_func(void* arg) {
    (void)arg;
    struct sockaddr_un addr;

    if (access(SOCKET_PATH, F_OK) == 0) {
        unlink(SOCKET_PATH);
    }

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("IPC: socket");
        return NULL;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("IPC: bind");
        close(server_fd);
        return NULL;
    }

    if (listen(server_fd, 5) < 0) {
        perror("IPC: listen");
        close(server_fd);
        return NULL;
    }

    while (1) {
        int fd = accept(server_fd, NULL, NULL);
        if (fd < 0) {
            perror("accept");
            continue;
        }

        pthread_mutex_lock(&client_mutex);
        if (client_fd != -1) {
            close(client_fd);
        }
        client_fd = fd;
        pthread_mutex_unlock(&client_mutex);
        printf("IPC: Bridge connected\n");
    }
    return NULL;
}

void start_ipc_server(void) {
    pthread_t thread;
    pthread_create(&thread, NULL, ipc_thread_func, NULL);
    pthread_detach(thread);
}

void push_stats(const char *json_stats) {
    pthread_mutex_lock(&client_mutex);
    if (client_fd != -1) {
        ssize_t len = strlen(json_stats);
        if (send(client_fd, json_stats, len, MSG_NOSIGNAL) < 0) {
            if (errno == EPIPE || errno == ECONNRESET) {
                printf("IPC: Bridge disconnected\n");
                close(client_fd);
                client_fd = -1;
            }
        }
    }
    pthread_mutex_unlock(&client_mutex);
}
