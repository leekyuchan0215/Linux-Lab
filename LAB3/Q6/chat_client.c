#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 9002
#define IP "127.0.0.1"
#define BUF_SIZE 1024
#define NAME_SIZE 20

char name[NAME_SIZE] = "[Unknown]";
char msg[BUF_SIZE];

// 메시지 송신 담당 쓰레드
void *send_msg(void *arg) {
    int sock = *((int *)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    
    while (1) {
        fgets(msg, BUF_SIZE, stdin);
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

// 메시지 수신 담당 쓰레드
void *recv_msg(void *arg) {
    int sock = *((int *)arg);
    char name_msg[BUF_SIZE + NAME_SIZE];
    int str_len;
    
    while (1) {
        str_len = read(sock, name_msg, BUF_SIZE + NAME_SIZE - 1);
        if (str_len == -1) return (void *)-1;
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;

    if (argc != 2) {
        printf("Usage : %s <Name>\n", argv[0]);
        exit(1);
    }

    sprintf(name, "[%s]", argv[1]);

    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Connect Error");
        exit(1);
    }

    printf("--- Chat Connected (Name: %s) ---\n", name);

    // 송신 쓰레드와 수신 쓰레드 생성
    pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);

    // 쓰레드 종료 대기
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(sock);
    return 0;
}