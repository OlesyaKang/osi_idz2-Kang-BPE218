#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

#define R_COUNT 25
#define SEM_MUTEX "rooms_mutex"

typedef struct {
    int id;
    int mon;
} client_arg;

typedef struct {
    int hasClient;
    int price;
} room_arg;

room_arg r[R_COUNT];

bool freeCount(int mon, int n) {
    for (int i = 0; i < n; ++i) {
        if (r[i].hasClient == -1 && r[i].price <= mon) {
            return true;
        }
    }
    return false;
}
int findRoom(int mon) {
    int n;
    if (mon < 2000) {
        return -1;
    } else if (mon < 4000) {
        n = 10;
    } else if (mon < 6000) {
        n = 20;
    } else {
        n = 25;
    }
    if (freeCount(mon, n)) {
        int tmp = rand() % n;
        while (r[tmp].hasClient != -1 || r[tmp].price > mon) {
            tmp = rand() % n;
        }
        return tmp;
    }
    return -1;
}

void* client(void* arg) {
    client_arg* c = (client_arg*) arg;
    int r_price, r_num;
    sem_t* sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_wait(sem_mutex);
    r_num = findRoom(c->mon);

    if (r_num == -1) {
        printf("%d tried to book a room without money and bursted\n", c->id);
        sem_post(sem_mutex);
        free(c);
        sem_close(sem_mutex);
        pthread_exit(NULL);
    }

    r_price = r[r_num].price;
    r[r_num].hasClient = c->id;
    c->mon -= r_price;
    printf("Room %d was booked by %d for %d\n", r_num + 1, c->id, r_price);
    sem_post(sem_mutex);
    usleep((rand() % 10 + 1) * 400000);
    sem_wait(sem_mutex);
    r[r_num].hasClient = -1;
    sem_post(sem_mutex);
    printf("%d has left the room %d\n", c->id, r_num);
    free(c);
    sem_close(sem_mutex);
    pthread_exit(NULL);
}

int main() {
    int i;
    srand(time(NULL));
    sem_t* sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    for (i = 0; i < R_COUNT; ++i) {
        r[i].hasClient = -1;
        if (i < 10) {
            r[i].price = 2000;
        } else if (i < 20) {
            r[i].price = 4000;
        } else {
            r[i].price = 6000;
        }
    }
    for (i = 0; findRoom(6000) != -1; i++) {
        client_arg* arg = malloc(sizeof(client_arg));
        arg->id = i;
        arg->mon = (rand() % 10 + 1) * 1000;
        pthread_t pthread;
        pthread_create(&pthread, NULL, client, arg);
        usleep(8000);
    }
    sem_close(sem_mutex);
    sem_unlink(SEM_MUTEX);
    return 0;
}