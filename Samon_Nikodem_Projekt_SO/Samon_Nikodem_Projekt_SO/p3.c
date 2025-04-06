#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int unblock = 1;

void handler1(int sig){
	if(sig == SIGHUP)
	{
		printf("-------P3 otrzymalem SIGHUP\n");		
		kill(getppid(), SIGHUP);
	}
	if(sig == SIGINT)
	{
		printf("-------P3 otrzymalem SIGINT\n");
		kill(getppid(), SIGINT);
	}
	if(sig == SIGQUIT)
	{
		printf("-------P3 otrzymalem SIGQUIT\n");
		kill(getppid(), SIGQUIT);
	}
	if(sig == SIGILL)
	{
		printf("-------P3 otrzymalem SIGILL\n");		
		kill(getppid(), SIGILL);
	}
}

//syugnaly przysylane od matki
void handler2(int sig){
	if(sig == SIGABRT)
	{
		printf("-------P3 otrzymalem SIGABRT\n");			
		if(unblock ==1) {unblock = 0; printf("Wstrzymanie dzialania P3\n");}
	}
	if(sig == SIGBUS)
	{
		printf("-------P3 otrzymalem SIGBUS\n");			
		if(unblock ==0) {unblock = 1; printf("Wznowienie dzialania P3\n");}	
	}
	if(sig == SIGIO)
	{
		printf("-------P3 konczy dzialanie\n");			
		kill(getpid(), SIGKILL);
	}
}

struct mymsgbuf {
	long mtype;
	char msg[300];
}queue;

static struct sembuf buf;

void sig(int semid, int semnum){
	buf.sem_num = semnum;
	buf.sem_op = 1;
	buf.sem_flg = 0;
	if (semop(semid, &buf, 1) == -1){
	perror("Podnoszenie semafora");
	exit(1);
	}
}

void wait(int semid, int semnum){
	buf.sem_num = semnum;
	buf.sem_op = -1;
	buf.sem_flg = 0;
	while(semop(semid, &buf, 1) == -1){
	;
	}
}


struct mymsgbuf readbuffer;

int main(int argc, char **argv){
	int qid;
	int semid;
	key_t msgkey;
	msgkey = ftok(".", 'm');
	if((qid = msgget( msgkey, IPC_CREAT | 0660 )) == -1) {
		perror("Blad otwierania kolejki komunikatow");
		exit(1);
	}
	
	semid = semget(45230, 2, IPC_CREAT|0600);
	if(semid == -1)
	{
		perror("Blad tworzenia tablicy semaforow p3\n");
		exit(1);
	}
	
				signal(SIGHUP, handler1); //kodowanie do matki
				signal(SIGINT, handler1); //wstrzymanie do matki
				signal(SIGQUIT, handler1); //wznowienie do matki
				signal(SIGILL, handler1); //zakonczenie do matki
				signal(SIGABRT, handler2); //wstrzymanie od matki
				signal(SIGBUS, handler2); //wznowienie od matki
				signal(SIGIO, handler2); //zakonczenie od matki	
	
	while(1) {
				if(unblock == 0)
				{
					while(unblock == 0) {;}
				}
				while(msgrcv( qid, &readbuffer, sizeof(struct mymsgbuf), readbuffer.mtype, IPC_NOWAIT) != -1){
				sleep(1);
				if(unblock == 0)
				{
					while(unblock == 0) {;}
				}
				printf("P3: %s\n", readbuffer.msg);
			}
	}
	
	return 0;
}
