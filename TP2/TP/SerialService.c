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
#include "SerialManager.h"
#include "stdbool.h"
#define MAX 80
#define PORT 10000
#define SA struct sockaddr

#include <sys/socket.h>

const char *msg_thread = "Thread 1";
char buff[MAX];
char buff_send[10];
int n;
char serial_buff[MAX];
int buff_size = MAX;

int connfd, len;

int sockfd;
bool clientConnected;
bool signalExit;

pthread_mutex_t mutexData = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief sign_block: función que bloquea las señales para evitar que en el momento que
 * se crea el thread llegue una señal y no haya quien la atienda.
 *
 */
void sign_block(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT | SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

/**
 * @brief sign_unblock: función que desbloquea las señales para que una vez creado el thread, pueda
 * recibir las mismas.
 *
 */
void sign_unblock(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT | SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

/**
 * @brief signal_handler: función llamada al recibir una señal SIGINT o SIGTERM.
 *
 * @param sig: señal recibida.
 */
void signal_handler(int sig)
{
    write(0, "SIGNAL RECEIVE!'\r\n", 17);
    signalExit = true;
}

/**
 * @brief send_to_emulator: función que envía datos al emulador del puerto serial.
 *
 * @param buffer: dirección del buffer a enviar.
 * @param size: tamaño del buffer a enviar.
 */
void send_to_emulator(char *buffer, int size)
{
    serial_send(buffer, size);
}

/**
 * @brief Thread Socket: una vez creado el socket, se crea el thread para el
 * manejo del envío de los datos del cliente al emulador con la función send_to_emulator
 *
 * @param arg: file descriptor del socket creado
 * @return void*
 */
void *start_socket_thread(void *arg)
{
    printf("Thread_socket..\n");

    bool connClient = false;

    clientConnected = false;

    bzero(serial_buff, MAX);

    struct sockaddr_in servaddr, cli;

    /* Create socket */

    printf("Thread socket\n");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    /* Ip and port */

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("Socket bind failed...\n");
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

    /* accep socket*/
    while (1)
    {
        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0)
        {
            printf("Server accept failed...\n");
            exit(0);
        }
        else
        {
            printf("Server accept the client...\n");
            pthread_mutex_lock(&mutexData);
            clientConnected = true;
            connClient = clientConnected;
            pthread_mutex_unlock(&mutexData);
        }

        while (connClient)
        {
            if (read(connfd, buff, (sizeof(buff))) != 0)
            {
                printf("From client: %s\t To client : ", buff);
                send_to_emulator(buff, sizeof(buff));
                bzero(buff, MAX);
                n = 0;
            }
            else
            {
                printf("Client disconnected\n\r");
                pthread_mutex_lock(&mutexData);
                clientConnected = false;
                connClient = clientConnected;
                pthread_mutex_unlock(&mutexData);
            }
            sleep(1);
        }
        printf("Reconnected client...\n\r");
        sleep(1);
    }
}

int main(void)
{

    int ret;
    int data_receive = 0;
    /* signal handler create */

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    signalExit = false; // Flag para terminar thread y cerrar programa

    pthread_t thread;

    bool connClient = false;

    /* sigactiont SIGINT */

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    /* sigactiont SIGTERM */

    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    if (serial_open(1, 115200) != 0)
    {
        printf("Error abriendo puerto serie\r\n");
        exit(1);
    }
    printf("Signal lock\r\n");
    sign_block();
    printf("Creo thread 'start_socket_thread'\n");
    ret = pthread_create(&thread,
                         NULL,
                         start_socket_thread,
                         NULL);
    if (ret != 0)
    {
        errno = ret;
        perror("pthread_error");
        exit(1);
    }
    printf("Signal unlock\r\n");
    sign_unblock();

    printf("receive data..\r\n");
    do
    {
        pthread_mutex_lock(&mutexData);
        connClient = clientConnected;
        pthread_mutex_unlock(&mutexData);
        if (connClient)
        {
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
                write(connfd, buff_send, data_receive - 1);
                bzero(buff_send, data_receive);
            }
        }
        else
        {
            printf("Client disconnected\r\n");
        }

        usleep(500000);
    } while (!signalExit);

    printf("End process.\n");
    pthread_cancel(thread);
    pthread_join(thread,NULL);
    close(sockfd);
    return 0;
}