/**
 * @file main.c
 * @author Storaccio Luis (storaccioluis@gmail.com)
 * @brief Trabajo práctico II de sistemas operativos de propósitos generales
 * @version 0.1
 * @date 2022-06-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "SerialManager.h"
#include "stdbool.h"
#define MAX 80
#define PORT 10000 // 10000
#define SA struct sockaddr

#include <sys/socket.h>

const char *msg_thread = "Thread 1";
char buff[MAX];
char buff_send[10];
int n;
char serial_buff[MAX];
int buff_size = MAX;
int flag_send = 0;

pthread_mutex_t mutexData = PTHREAD_MUTEX_INITIALIZER;

void bloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT | SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void desbloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT | SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void sigint_handler(int sig)
{
    write(0, "Ahhh! SIGINT!\n", 14);
    exit(1);
}

void send_to_emulator(char *buffer, int size)
{
    serial_send(buffer, size);
}

void *start_socket_thread(void *arg)
{
    printf("Thread_socket..\n");
    int *connfd = (void *)arg;
    bzero(serial_buff, MAX);
    while (1)
    {

        bzero(buff, MAX);
        if (read(*connfd, buff, (sizeof(buff))) != EOF)
        {
            printf("From client: %s\t To client : ", buff);
            send_to_emulator(buff, sizeof(buff));
            bzero(buff, MAX);
            n = 0;
        }
        else
        {
            printf("cliente desconectado\n\r");
        }
        if (flag_send == 1)
        {
            printf("Trama enviada: %s\t", buff_send);
            // write(*connfd, buff_send, 9 - 1);
            flag_send = 0;
            bzero(buff_send, 10);
        }
        sleep(1);

        printf("Thread_socket_run..\n");
    }
}

int main(void)
{
    pthread_t thread;
    int ret;
    int data_receive = 0;

    printf("Asigno handler\n");

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT | SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
    printf("Thread socket\n");
    int sockfd, *connfd, len;
    struct sockaddr_in servaddr, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    *connfd = accept(sockfd, (SA *)&cli, &len);
    if (*connfd < 0)
    {
        printf("server accept failed...\n");
        exit(0);
    }
    else
    {
        printf("server accept the client...\n");
        printf("Bloqueo signal\n");
        bloquearSign();
        printf("Creo thread 'start_socket_thread'\n");
        ret = pthread_create(&thread,
                             NULL,
                             start_socket_thread,
                             (void *)connfd);
        if (ret != 0)
        {
            errno = ret;
            perror("pthread_error");
            exit(1);
        }
        printf("Desbloqueo signal\n");
        desbloquearSign();
    }
    if (serial_open(1, 115200) != 0)
    {
        printf("Error abriendo puerto serie\r\n");
        exit(1);
    }

    while (1)
    {
        printf("receive data..\r\n");
        data_receive = serial_receive(serial_buff, buff_size);

        if (data_receive > 0)
        {
            for (int i = 0; i < data_receive; i++)
            {
                buff_send[i] = serial_buff[i];
            }
            bzero(serial_buff, MAX);
            printf("sizeof: %d\r\n", data_receive);
            printf("serial_buff: %s\r\n", buff_send);
            write(*connfd, buff_send, data_receive - 1);
        }

        usleep(500000);
    }
    printf("Espero.\n");
    sleep(1);
    printf("Fin proceso.\n");
    close(sockfd);
    return 0;
}