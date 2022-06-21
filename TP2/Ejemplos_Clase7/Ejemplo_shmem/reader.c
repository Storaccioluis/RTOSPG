#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main()
{
	key_t key;
	int shmid;
	key = ftok("writer.c", 'R');
	shmid = shmget(key, 1024, 0); //1k segment

	char* pData = shmat(shmid, (void *)0, 0);
	if (pData == (char *)(-1))
		perror("shmat");

    while(1)
    {
		printf("%s\r\n",pData);
		usleep(1000000);
    }



	return 0;
}
