#include <stdio.h>
#include <sys/msg.h>
#include <stddef.h>
#include <string.h>

struct msgbuf {
	long mtype;
	char mtext[20];
};

int main(void)
{
	key_t key = ftok("sender.c", 'b');
	int msqid = msgget(key, 0666 | IPC_CREAT);

	struct msgbuf msg;
	int size = sizeof(msg.mtext);
	msg.mtype=1; // tipo 1.

	while(1)
	{
		fgets(msg.mtext,20,stdin);
		if(strncmp(msg.mtext,"salir",5)==0)
			break;
		printf("msg:%s\r\n",msg.mtext);
		msgsnd(msqid,&msg,size,0);
	}

	// Eliminamos la queue
	msgctl(msqid, IPC_RMID, NULL);

	return 0;
}

