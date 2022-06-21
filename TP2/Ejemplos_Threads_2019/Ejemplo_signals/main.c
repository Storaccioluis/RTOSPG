#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>



void bloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    //sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void desbloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    //sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void sigint_handler(int sig)
{
	write(0, "Ahhh! SIGINT!\n", 14);
}

int main(void)
{
	printf("Asigno handler\n");

	// signal ctrl+c
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Bloqueo signal\n");
	bloquearSign();

	printf("Espero. Presionar ctrl+c para probar\n");
	int c=0;
	while(c<10)
	{
		sleep(1);
		printf("%d \n",c);
		c++;
	}	
	printf("Desbloqueo signal\n");

	desbloquearSign();

	printf("Espero.\n");
	sleep(1);
	printf("Fin proceso.\n");
	return 0;
}
