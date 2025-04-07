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
#include <signal.h>

int flag = 1;
int unblock = 1;

void handler1(int sig){
	if(sig == SIGHUP)
	{
		printf("-------P2 otrzymalem SIGHUP\n");		
		kill(getppid(), SIGHUP);
	}
	if(sig == SIGINT)
	{
		printf("-------P2 otrzymalem SIGINT\n");
		kill(getppid(), SIGINT);
	}
	if(sig == SIGQUIT)
	{
		printf("-------P2 otrzymalem SIGQUIT\n");
		kill(getppid(), SIGQUIT);
	}
	if(sig == SIGILL)
	{
		printf("-------P2 otrzymalem SIGILL\n");		
		kill(getppid(), SIGILL);
	}
}

void handler2(int sig){
	if(sig == SIGUSR2)
	{
		printf("-------P2 otrzymalem rozkaz zmieniajacy kodowanie\n");
		if(flag ==1) {flag = 0;}
		else {flag = 1;}
	}
	if(sig == SIGABRT)
	{
		printf("-------P2 otrzymalem SIGABRT\n");	
		if(unblock ==1) {unblock = 0; printf("Wstrzymanie dzialania P2\n");}
	}
	if(sig == SIGBUS)
	{
		printf("-------P2 otrzymalem SIGBUS\n");			
		if(unblock ==0) {unblock = 1; printf("Wznowienie dzialania P2\n");}	
	}
	if(sig == SIGIO)
	{
		printf("-------P2 konczy dzialanie\n");			
		kill(getpid(), SIGKILL);
	}
}


    
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

struct mymsgbuf {
	long mtype;
	char msg[300];
}queue;


int main(int argc, char **argv){
	int qid;
	key_t msgkey;
	int semid, shmid;
	char * buf;
	
	msgkey = ftok(".", 'm');
	if((qid = msgget( msgkey, IPC_CREAT | 0660 )) == -1) {
		perror("Blad otwierania kolejki komunikatow");
		exit(1);
	}
	
	semid = semget(45230, 2, IPC_CREAT|0600);
	if(semid == -1)
	{
		perror("Blad tworzenia tablicy semaforow p2\n");
		exit(1);
	}
	shmid = shmget(45286, sizeof(char)*10, IPC_CREAT|0600);
	if(shmid == -1)
	{
		perror("Blad utworzenia segmentu pamieci wspoldzielonej k11\n");
		exit(1);
	}
	// Pobranie wskaznika na adres pamieci wspoldzielonej
	buf = shmat(shmid, NULL, 0);
	if(buf == NULL)
	{
		perror("Błąd przypisania segmentu pamięci wspoldzielonej k12");
		exit(1);
	}
	

				signal(SIGHUP, handler1); //kodowanie do matki
				signal(SIGINT, handler1); //wstrzymanie do matki
				signal(SIGQUIT, handler1); //wznowienie do matki
				signal(SIGILL, handler1); //zakonczenie do matki
				signal(SIGUSR2, handler2); //koniec/start kodowania od matki
				signal(SIGABRT, handler2); //wstrzymanie od matki
				signal(SIGBUS, handler2); //wznowienie od matki
				signal(SIGIO, handler2); //zakonczenie od matki	
	
	char coded[300];
	while(1)
	{
		if(unblock == 0)
		{
			while(unblock == 0) {;}
		}
		wait(semid, 1);
		sleep(1);
		if(unblock == 0)
		{
			while(unblock == 0) {;}
		}
		strcpy(coded, buf);
		if(flag == 1)
		{
			int i;
			int j = 0; //indeks tablicy heks
			for(i = 0; i<100; i++){
				if(buf[i] != '\0') {
				sprintf(coded + j, "%02X", buf[i]);
				j = j + 2;
				}
			}
		}
		printf("P2: %s -:- %s\n", buf, coded);
		strcpy(queue.msg, coded);
		queue.mtype = 1;
		if((msgsnd( qid, &queue, sizeof(struct mymsgbuf), 0)) == -1)
		{
			perror("Blad wysylania komunikatu");
			exit(1);
		}		
		sleep(1);
		sig(semid, 0);	
	}
	
	return 0;
}
