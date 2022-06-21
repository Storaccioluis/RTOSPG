#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
	int a;
	int b;
}Data;

Data data;
pthread_mutex_t mutexData = PTHREAD_MUTEX_INITIALIZER;

void* start_thread (void* message)
{
	while(1)
	{
		pthread_mutex_lock (&mutexData);
		data.a++;
		data.b++;
		pthread_mutex_unlock (&mutexData);
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
		pthread_mutex_lock (&mutexData);
		a = data.a;
		b = data.b;
		pthread_mutex_unlock (&mutexData);
		printf("%d %d\r\n",a,b);
		sleep(1);
	}

	return 0;
}


