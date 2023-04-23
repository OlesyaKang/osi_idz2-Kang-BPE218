#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/sem.h>

#define R_COUNT 25

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
    struct seminfo* __buf;
};

typedef struct {
    int id;
    int mon;
} client_arg;

typedef struct {
    int hasClient;
    int price;
} room_arg;

room_arg r[R_COUNT];

int sem_m_id;

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
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_flg = 0;
    sem_op.sem_op = -1;
    semop(sem_m_id, &sem_op, 1);

    r_num = findRoom(c->mon);

    if (r_num == -1) {
        printf("%d tried to book a room without money and bursted\n", c->id);
        sem_op.sem_op = 1;
        semop(sem_m_id, &sem_op, 1);
        free(c);
        pthread_exit(NULL);
    }

    r_price = r[r_num].price;
    r[r_num].hasClient = c->id;
    c->mon -= r_price;
    printf("Room %d was booked by %d for %d\n", r_num + 1, c->id, r_price);
    sem_op.sem_op = 1;
    semop(sem_m_id, &sem_op, 1);
    usleep((rand() % 10 + 1) * 400000);
    sem_op.sem_op = -1;
    semop(sem_m_id, &sem_op, 1);
    r[r_num].hasClient = -1;
    sem_op.sem_op = 1;
    semop(sem_m_id, &sem_op, 1);
    printf("%d has left the room %d\n", c->id, r_num);
    free(c);
    pthread_exit(NULL);
}

int main() {
    int i;
    srand(time(NULL));
    sem_m_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    union semun sem_arg;
    sem_arg.val = 1;
    semctl(sem_m_id, 0, SETVAL, sem_arg);
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
        pthread_t c_thread;
        pthread_create(&c_thread, NULL, client, (void*) arg);
        pthread_detach(c_thread);
        usleep(4000);
    }
    semctl(sem_m_id, 0, IPC_RMID, 0);
    return 0;
}