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
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

int unblock = 1;

//sygnaly wysylane do matki
void handler1(int sig){
	if(sig == SIGHUP)
	{
		printf("-------P1 otrzymalem SIGHUP\n");
		kill(getppid(), SIGHUP);
	}
	if(sig == SIGINT)
	{
		printf("-------P1 otrzymalem SIGINT\n");
		kill(getppid(), SIGINT);
	}
	if(sig == SIGQUIT)
	{
		printf("-------P3 otrzymalem SIGQUIT\n");
		kill(getppid(), SIGQUIT);
	}
	if(sig == SIGILL)
	{
		printf("-------P1 otrzymalem SIGILL\n");		
		kill(getppid(), SIGILL);
	}
}

//syugnaly przysylane od matki
void handler2(int sig){
	if(sig == SIGABRT)
	{
		printf("-------P1 otrzymalem SIGABRT\n");			
		if(unblock ==1) {unblock = 0; printf("Wstrzymanie dzialania P1\n");}
	}
	if(sig == SIGBUS)
	{
		printf("-------P1 otrzymalem SIGBUS\n");			
		if(unblock ==0) {unblock = 1; printf("Wznowienie dzialania P1\n");}	
	}
	if(sig == SIGIO)
	{
		printf("-------P1 konczy dzialanie\n");			
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

int main(){
	int shmid, semid;
	char *buf;
	char path[256];

	//odwolanie sie do semaforow
	semid = semget(45230, 2, IPC_CREAT|0600);
	if(semid == -1)
	{
		perror("Blad tworzenia tablicy semaforow p1\n");
		exit(1);
	}
	
	//utworzenie segmentu pamieci wspoldzielonej
	shmid = shmget(45286, sizeof(char)*10, IPC_CREAT|0600);
	if(shmid == -1)
	{
		perror("Blad utworzenia segmentu pamieci wspoldzielonej p2\n");
		exit(1);
	}
	// Pobranie wskaznika na adres pamieci wspoldzielonej
	buf = shmat(shmid, NULL, 0);
	if(buf == NULL)
	{
		perror("Blad przypisania segmentu pamieci wspoldzielonej p3");
		exit(1);
	}
	
				signal(SIGHUP, handler1); //kodowanie do matki
				signal(SIGINT, handler1); //wstrzymanie do matki
				signal(SIGQUIT, handler1); //wznowienie do matki
				signal(SIGILL, handler1); //zakonczenie do matki
				signal(SIGABRT, handler2); //wstrzymanie od matki
				signal(SIGBUS, handler2); //wznowienie od matki
				signal(SIGIO, handler2); //zakonczenie od matki	
	
	//pobranie sciezki i zapisa do pliku
	while(1) {
		//pobranie sciezki
		printf("Podaj sciezke do katalogu: ");
	    if (fgets(path, sizeof(path), stdin) == NULL) {
		fprintf(stderr, "Błąd odczytu ścieżki\n");
		return 1;
		}
		size_t len = strlen(path);
		if (len > 0 && path[len - 1] == '\n') {
		path[len - 1] = '\0';
		}
		DIR *wsk_katalog = opendir(path); //wskaznik do katalogu
		if (wsk_katalog == NULL) {
		perror("Błąd otwarcia katalogu!");
		return 1;
		}
		struct dirent *wejscie;
		//zapisywanie sciezek do pamieci wspoldzielonej
		while((wejscie = readdir(wsk_katalog)) != NULL)
		{
			if(unblock == 0)
			{
				while(unblock == 0) {;}
			}
			char message[100];
			wait(semid, 0);
			sleep(1);
			if(unblock == 0)
			{
				while(unblock == 0) {;}
			}
			//zapis sciezki do pamieci wspoldzielonej
			sprintf(message, "%s/%s", path, wejscie->d_name);
			strcpy(buf, message);
			printf("P1: %s\n", buf);
			sig(semid, 1);
		}
		closedir(wsk_katalog);	
		wait(semid, 0);
		sleep(1);
		printf("Mozna wpisywac dalej!\n");
		sig(semid, 0);
	}
	
	return 0;
}
