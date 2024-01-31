#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

static struct sembuf buf;

void podnies(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;

    if (semop(semid, &buf, 1) == -1){
        exit(1);
    }
}

void opusc(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;

    if (semop(semid, &buf, 1) == -1){
        perror("Opuszczenie semafora");
        exit(1);
    }
}

int main(){

    // segment pamieci inicjowanie
    int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Utworzenie segmentu pamieci wspoldzielonej");
        exit(1);
    }

    int *kierunek = (int *)shmat(shmid, NULL, 0);
    if (kierunek == (int *)(-1)) {
        perror("Przylaczenie pamieci wspoldzielonej");
        exit(1);
    }
    *kierunek=0;

    // Segment pamieci dla zmiennej i
    int shmid_i = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shmid_i == -1) {
        perror("Utworzenie segmentu pamieci wspoldzielonej dla i");
        exit(1);
    }


    int *i = (int *)shmat(shmid_i, NULL, 0);
    if (i == (int *)(-1)) {
        perror("Przylaczenie pamieci wspoldzielonej dla i");
        exit(1);
    }
    *i=0;


    int semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("Utworzenie semafora");
        exit(1);
    }
    // Ustawienie początkowych wartości semaforów
    semctl(semid, 0, SETVAL, 1); //lokalny semafor
    semctl(semid, 1, SETVAL, 1); //kontrolujacy kierunki
    

    
    srand(time(NULL));
    fork();
    fork();
    while (1) {
        sleep(1);
        opusc(semid, 1);
        int nowyKierunek = rand() % 2;

        if (nowyKierunek == 0){
            printf("Pojazd %d jedzie na południe\n", *i);
        } else{
            printf("Pojazd %d jedzie na północ\n", *i);
        }

        opusc(semid, 0);
        if (nowyKierunek != *kierunek){
            printf("Czekanie na przejazd przeciwnego kierunku\n");
            sleep(3);
        }

        printf("Pojazd %d przejechał\n", *i);
        (*i)++;
        *kierunek = nowyKierunek;

        podnies(semid, 0);
        podnies(semid, 1);
    }
    // sposob dzialania to na 2 semafory jeden kontroluje wewnatrz petli a drugi polnoc poludnie 
    return 0;
}
