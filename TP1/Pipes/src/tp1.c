#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int main(void)
{
    int pfds[2];
    pipe(pfds);
    if (!fork()) // Hijo
    {
        dup2(pfds[1], 1);
        close(pfds[0]);
        execlp("ls", "ls", NULL);
    }
    else
    {
        dup2(pfds[0], 0);
        close(pfds[1]);
        execlp("wc", "wc", "-l", NULL);
    }

} // Fin main