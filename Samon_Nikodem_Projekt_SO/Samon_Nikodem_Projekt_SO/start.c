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
#include <errno.h>
#include <signal.h>

int flag = 0;
static struct sembuf buf;
pid_t child_pid1, child_pid2, child_pid3;
	
void handler1(int sig){
	if(sig == SIGHUP)
	{
		printf("-------S otrzymalem SIGHUP\n");		
		kill(child_pid2, SIGUSR2);
	}
	if(sig == SIGINT)
	{
		printf("-------S otrzymalem SIGINT\n");	
		kill(child_pid1, SIGABRT);
		kill(child_pid2, SIGABRT);
		kill(child_pid3, SIGABRT);
	}
	if(sig == SIGQUIT)
	{
		printf("-------S otrzymalem SIGQUIT\n");	
		kill(child_pid1, SIGBUS);
		kill(child_pid2, SIGBUS);
		kill(child_pid3, SIGBUS);
	}
	if(sig == SIGILL)
	{
		printf("-------S otrzymalem SIGILL\n");		
		kill(child_pid1, SIGIO);
		kill(child_pid2, SIGIO);
		kill(child_pid3, SIGIO);
		flag = 1;
		sleep(1);
		printf("Zakonczenie dzialania\n");
		kill(getpid(), SIGKILL);
		
	}
}

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
	if (semop(semid, &buf, 1) == -1){
		perror("Opuszczenie semafora");
		exit(1);
	}
}

struct mymsgbuf {
	long mtype;
	char msg[300];
}queue;

int main(){
	int shmid, semid;
	int qid;
	key_t msgkey;
	
	//pamiec wspoldzielona
	shmid = shmget(45286, sizeof(char)*10, IPC_CREAT|0600);
	if(shmid == -1)
	{
		perror("Blad utworzenia segmentu pamieci wspoldzielonej k11\n");
		exit(1);
	}
	
	//kolejka komunikatow
	msgkey = ftok(".", 'm');
	if((qid = msgget( msgkey, IPC_CREAT | 0660 )) == -1) {
		perror("Blad otwierania kolejki komunikatow");
		exit(1);
	}
	
	// utworzenie tablicy 2 semaforow
	semid = semget(45230, 2, IPC_CREAT|0600);
	if(semid == -1)
	{
		perror("Blad tworzenia tablicy semaforow start\n");
		exit(1);
	}
	// Ustawienie poczatkowych wartosci semaforow
	if(semctl(semid, 0, SETVAL, (int)1) == -1)
	{
		perror("Blad tworzenia tablicy semaforow\n");
		exit(1);
	}
	if(semctl(semid, 1, SETVAL, (int)0) == -1)
	{
		perror("Blad tworzenia tablicy semaforow\n");
		exit(1);
	}

	//utworzenie 3 procesow ktore beda sie ze soba komunikowac
    child_pid1 = fork(); //utworzenie procesu 1 - pobierajacego sciezke
    if (child_pid1 == -1) {
        perror("Blad fork() dla procesu potomnego 1");
        exit(EXIT_FAILURE);
    } else if (child_pid1 == 0) {
		printf("P1 PID: %d\n", getpid());
        execl("./p1", "p1", NULL);  // Uruchamianie programu p1
        perror("Bladd execl() dla procesu producenta - 1");
        exit(EXIT_FAILURE);
    }

    child_pid2 = fork(); //utworzenie procesu 2 - odbierajacego i wysylajacego sciezke
    if (child_pid2 == -1) {
        perror("Blad fork() dla procesu potomnego 2");
        exit(EXIT_FAILURE);
    } else if (child_pid2 == 0) {
		printf("P2 PID: %d\n", getpid());
        execl("./p2", "p2", NULL);  // Uruchamianie programu k2
        perror("Blad execl() dla procesu posrednika - 2");
        exit(EXIT_FAILURE);
    }

	child_pid3 = fork(); //utworzenie procesu 3 - odbierajacego sciezke
    if (child_pid3 == -1) {
        perror("BĹ‚Ä…d fork() dla procesu potomnego 2");
        exit(EXIT_FAILURE);
    } else if (child_pid3 == 0) {
		printf("P3 PID: %d\n", getpid());
        execl("./p3", "p3", NULL);  // Uruchamianie programu k2
        perror("Blad execl() dla procesu konsumenckiego - 3");
        exit(EXIT_FAILURE);
    }
	sleep(1);
	
	signal(SIGHUP, handler1); //kodowanie do matki
	signal(SIGINT, handler1); //wstrzymanie do matki
	signal(SIGQUIT, handler1); //wznowienie do matki
	signal(SIGILL, handler1); //zakonczenie do matki
	while(1) 
	{
		if(flag == 1)
		{
			shmctl(shmid, IPC_RMID, NULL);
			shmctl(semid, 0, IPC_RMID);
			shmctl(semid, 1, IPC_RMID);
			if(msgctl(qid, IPC_RMID, 0) == -1){
				perror("Blad usuwania kolejki komunikatow");
				exit(1);
			}
			flag = 0;
		}
	}
	return 0;
}
