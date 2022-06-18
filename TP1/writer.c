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
#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300


const uint8_t *sign1= "SIGN:1\n\r";
const uint8_t *sign2= "SIGN:2\n\r";
uint32_t bytesWrote;

int32_t returnCode, fd;

void recibiSignal_us_1(int sig)
{
    if ((bytesWrote = write(fd, sign1, strlen(sign1) + 1)) == -1)
    {
        perror("write");
    }
}

void recibiSignal_us_2(int sig)
{
    if ((bytesWrote = write(fd, sign2, strlen(sign2) + 1)) == -1)
    {
        perror("write");
    }
}
int main(void)
{
    char inputBuffer[BUFFER_SIZE];
    char outputBuffer[BUFFER_SIZE + 6];

    struct sigaction sa_us_1;
    struct sigaction sa_us_2;

    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    printf("waiting for readers...\n");
    if ((fd = open(FIFO_NAME, O_WRONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }

    sa_us_1.sa_handler = recibiSignal_us_1;
    sa_us_2.sa_handler = recibiSignal_us_2;
    sa_us_1.sa_flags = SA_RESTART;
    sa_us_2.sa_flags = SA_RESTART;
    sigemptyset(&sa_us_1.sa_mask);
    sigemptyset(&sa_us_2.sa_mask);

    if (sigaction(SIGUSR1, &sa_us_1, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGUSR2, &sa_us_2, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("got a reader--type some stuff\n");

    while (1)
    {
        /* Get some text from console */
        if (fgets(inputBuffer, BUFFER_SIZE, stdin) != NULL)
        {
            sprintf(outputBuffer, "DATA:%s\n\r", inputBuffer);
            if ((bytesWrote = write(fd, outputBuffer, strlen(outputBuffer) - 1)) == -1)
            {
                perror("write");
            }
            else
            {
                printf("writer: wrote %d bytes\n", bytesWrote);
            }
        }
    }
    return 0;
}
