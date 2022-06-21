#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
	int a;
	int b;
}Data;

Data data;

void* start_thread (void* message)
{
	while(1)
	{
		data.a++;
		data.b++;
		usleep(100);
	}
	return NULL;
}


int main (void)
{
	pthread_t t1,t2;
	int a,b;

	pthread_create (&t1, NULL, start_thread,NULL);
	pthread_create (&t2, NULL, start_thread,NULL);

	while(1)
	{
		a = data.a;
		b = data.b;
		printf("%d %d\r\n",a,b);
		sleep(1);
	}

	return 0;
}


