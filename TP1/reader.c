/*Este proceso leerá los datos del named fifo y según el encabezado “DATA” o “SIGN” escribirá en el
archivo log.txt o signals.txt.*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define FIFO_NAME "myfifo"
#define LOG_NAME "Log.txt"
#define SIGN_NAME "Sign.txt"
#define BUFFER_SIZE 300

const char data[] = {'D', 'A', 'T', 'A'};
const char sign[] = {'S', 'I', 'G', 'N'};

int buscar_DATA(char *buffer)
{
    int ret = -1;
    int acum = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        if (*(buffer + i) == data[i])
        {
            acum++;
        }
    }
    if (acum == 4)
    {
        ret = 1;
    }
    return ret;
}
int buscar_SIGN(char *buffer)
{
    int ret = -1;
    int acum = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        if (*(buffer + i) == sign[i])
        {
            acum++;
        }
    }
    if (acum == 4)
    {
        ret = 1;
    }
    return ret;
}

int main(void)
{
    uint8_t i;
    uint8_t inputBuffer[BUFFER_SIZE];
    int32_t bytesRead, returnCode, fd;
    uint32_t bytesWrote;
    FILE *fd_log;
    FILE *fd_sign;

    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    printf("waiting for writers...\n");
    if ((fd = open(FIFO_NAME, O_RDONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("got a writer\n");

    /* Loop until read syscall returns a value <= 0 */
    do
    {
        /* read data into local buffer */
        if ((bytesRead = read(fd, inputBuffer, BUFFER_SIZE)) == -1)
        {
            perror("read");
        }
       
        else
        {   
            inputBuffer[bytesRead] = '\0';
            printf("reader: read %d bytes: \"%s\"\n", bytesRead, inputBuffer);
            if (buscar_DATA(inputBuffer) == 1)
            {
                if ((fd_log = fopen(LOG_NAME, "w")) < 0)
                {
	        exit(1);
                }
                for (size_t i = 0; i < bytesRead; i++)
                    {fprintf(fd_log, "%c", inputBuffer[i]);}
                fclose(fd_log);
            }
            else {printf("DATA no encontrado\n\r");}
            if (buscar_SIGN(inputBuffer) == 1)
            {
                if ((fd_sign = fopen(SIGN_NAME, "w")) < 0)
                {
	         exit(1);
                }
                for (size_t i = 0; i < bytesRead; i++)
                    {fprintf(fd_sign, "%c", inputBuffer[i]);}
                fclose(fd_sign);
               
            }
            else {printf("SIGN no encontrado\n\r");}
        }
     }
        while (bytesRead > 0);

        return 0;
    }
