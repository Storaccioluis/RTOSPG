#include <stdio.h>
#include <sys/msg.h>
#include <stddef.h>

struct msgbuf {
	long mtype;
	char mtext[20];
};

int main(void)
{
	key_t key = ftok("sender.c", 'b');
	int msqid = msgget(key,0);

	struct msgbuf msg;
	int size = sizeof(msg.mtext);

	while(1)
	{
		printf("Leo proximo mensaje...\r\n");
		if(msgrcv(msqid,&msg,size,0,0)==-1)
		{
			perror("msgrcv");
			exit(1);
		}
		printf("lei de la queue: %s\r\n",msg.mtext);
	}

	return 0;
}

