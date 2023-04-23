#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define R_COUNT 25
sem_t r_sem;
sem_t m_sem;

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
    for (int i = 0; i < n; ++i) {
        sem_wait(&r_sem);
        if (r[i].hasClient == -1 && r[i].price <= mon) {
            sem_post(&r_sem);
            return i;
        }
        sem_post(&r_sem);
    }
    return -1;
}

void* client(void* arg) {
    client_arg* c = (client_arg*) arg;
    int r_price, r_num;
    sem_wait(&m_sem);
    r_num = findRoom(c->mon);

    if (r_num == -1) {
        printf("%d tried to book a room without money and bursted\n", c->id);
        sem_post(&m_sem);
        free(c);
        pthread_exit(NULL);
    }

    r_price = r[r_num].price;
    r[r_num].hasClient = c->id;
    c->mon -= r_price;
    printf("Room %d was booked by %d for %d\n", r_num + 1, c->id, r_price);
    sem_post(&m_sem);
    usleep((rand() % 10 + 1) * 400000);
    sem_wait(&m_sem);
    r[r_num].hasClient = -1;
    sem_post(&r_sem);
    sem_post(&m_sem);
    printf("%d has left the room %d\n", c->id, r_num);
    free(c);
    pthread_exit(NULL);
}

int main() {
    int i;
    srand(time(NULL));
    sem_init(&r_sem, 0, 1);
    sem_init(&m_sem, 0, 1);
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
        usleep(1000);
    }
    sem_destroy(&r_sem);
    sem_destroy(&m_sem);
    return 0;
}