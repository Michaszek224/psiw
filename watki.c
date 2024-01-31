#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_PASSENGERS 25
#define WAGON_CAPACITY 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wagon_full_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t wagon_empty_cond = PTHREAD_COND_INITIALIZER;

int pasażerowie_czekający = 0;
int pasażerowie_na_pokładzie = 0;

void *wątek_pasażera(void *arg) {
    int id_pasażera = *(int *)arg;

    pthread_mutex_lock(&mutex);

    while (pasażerowie_na_pokładzie == WAGON_CAPACITY) {
        printf("Pasażer %d czeka na zwolnienie wagonika.\n", id_pasażera);
        pthread_cond_wait(&wagon_empty_cond, &mutex);
    }

    pasażerowie_czekający--;
    pasażerowie_na_pokładzie++;

    printf("Pasażer %d wszedł na pokład. Pasażerowie na pokładzie: %d\n", id_pasażera, pasażerowie_na_pokładzie);

    if (pasażerowie_na_pokładzie == WAGON_CAPACITY) {
        printf("Wagonik jest pełny. Sygnalizacja wyjazdu wagonika.\n");
        pthread_cond_signal(&wagon_full_cond);
    }

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *wątek_wagonika(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        while (pasażerowie_na_pokładzie < WAGON_CAPACITY) {
            printf("Wagonik czeka na wejście pasażerów.\n");
            pthread_cond_wait(&wagon_full_cond, &mutex);
        }

        printf("Wagonik odjeżdża z %d pasażerami na pokładzie.\n", pasażerowie_na_pokładzie);

        pasażerowie_na_pokładzie = 0;

        pthread_cond_broadcast(&wagon_empty_cond);

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    pthread_t wagonik, pasażerowie[MAX_PASSENGERS];
    int id_pasażera[MAX_PASSENGERS];

    pthread_create(&wagonik, NULL, wątek_wagonika, NULL);

    for (int i = 0; i < MAX_PASSENGERS; i++) {
        id_pasażera[i] = i + 1;
        pthread_create(&pasażerowie[i], NULL, wątek_pasażera, &id_pasażera[i]);
    }

    pthread_join(wagonik, NULL);

    for (int i = 0; i < MAX_PASSENGERS; i++) {
        pthread_join(pasażerowie[i], NULL);
    }

    return 0;
}
