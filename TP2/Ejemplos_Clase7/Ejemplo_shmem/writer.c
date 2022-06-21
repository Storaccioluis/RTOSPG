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
	shmid = shmget(key, 1024, 0644 | IPC_CREAT); //1k segment

	char* pData = shmat(shmid, (void *)0, 0);
	if (pData == (char *)(-1))
		perror("shmat");

    while(1)
    {
        fgets(pData,1024,stdin);
        if(strncmp(pData,"salir",5)==0)
            break;
    }

	printf("detach y eliminacion...\r\n");
	shmdt(pData);
	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}
