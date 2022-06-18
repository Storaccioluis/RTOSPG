
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

void sigint_handler(int sig)
{
    write(1, "Ahhh! SIGUSR1!\n", 14);
}
int main(void)
{
    char s[200];
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0; // SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
    printf("Enter a string:\n");
    if (fgets(s, sizeof s, stdin) == NULL)
        perror("fgets");
    else
        printf("You entered: %s\n", s);
    return 0;
}