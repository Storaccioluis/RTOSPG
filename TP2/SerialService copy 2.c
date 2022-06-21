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
int n;
char serial_buff[MAX];
int buff_size;
int flag_send = 0;

pthread_mutex_t mutexData = PTHREAD_MUTEX_INITIALIZER;

void bloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    // sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void desbloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    // sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void sigint_handler(int sig)
{
    write(0, "Ahhh! SIGINT!\n", 14);
    exit(1);
}

void send_trama_serial(char *trama)
{
}

void *start_thread(void *arg)
{
    int *connfd = (int *)arg;
    printf("Thread socket\n");

    while (1)
    {
        bzero(buff, MAX);
        if (read(*connfd, buff, (sizeof(buff))) != EOF)
        {
            printf("From client: %s\t To client : ", buff);
            bzero(buff, MAX);
            n = 0;
        }
        else
        {
            printf("cliente desconectado\n\r");
        }
        if (flag_send == 1)
        {
            printf("Trama enviada: %s\t", serial_buff);
            pthread_mutex_lock(&mutexData);
            write(*connfd, serial_buff, (sizeof(serial_buff) - 1));
            pthread_mutex_unlock(&mutexData);
            bzero(serial_buff, MAX);
            flag_send = 0;
        }
        sleep(2);

        printf("Thread_run\n");
        // sleep(1);
    }
}

int main(void)
{

    pthread_t thread;
    int ret;

    printf("Asigno handler\n");

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    if (serial_open(1, 115200) != 0)
    {
        printf("Error abriendo puerto serie\r\n");
        exit(1);
    }

    //**SOCKET SERVER//
    int sockfd, *connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    *connfd = accept(sockfd, (SA *)&cli, &len);
    if (*connfd < 0)
    {
        printf("server accept failed...\n");
        for (uint8_t index = 0; index < 10; index++)
        {
            *connfd = accept(sockfd, (SA *)&cli, &len);
            sleep(1);
        }
        if (*connfd < 0)
        {
            printf("server accept failed...TIMEOUT\n");
            exit(0);
        }
    }
    else
    {
        printf("server accept the client...\n");
        printf("Bloqueo signal\n");
        bloquearSign();
        printf("Creo thread 'start_thread'\n");
        ret = pthread_create(&thread,
                             NULL,
                             start_thread,
                             (void *)connfd);
        if (!ret)
        {
            errno = ret;
            perror("pthread_create");
            return -1;
        }
        printf("Desbloqueo signal\n");
        desbloquearSign();
    }

    // Function for chatting between client and server

    while (1)
    {
        printf("receive data..\r\n");
        pthread_mutex_lock(&mutexData);
        if (flag_send == 0)
        {
            if (serial_receive(serial_buff, buff_size) != 0)
            {
                serial_buff[sizeof(serial_buff)] = '\r';
                serial_buff[sizeof(serial_buff) + 1] = '\n';
                flag_send = 1;
                pthread_mutex_unlock(&mutexData);
                printf("serial_buff: %s\r\n", serial_buff);
            }
        }
        usleep(500000);
    }
    printf("Espero.\n");
    sleep(1);
    printf("Fin proceso.\n");
    close(sockfd);
    return 0;
}