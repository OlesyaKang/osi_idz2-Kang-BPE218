# osi_idz2-Kang-BPE218
## Ваирант 13
## Кан Олеся БПИ 218
### Условие:
В гостинице 10 номеров с ценой 2000 рублей, 10 номеров с ценой 4000 рублей и 5 номеров с ценой 6000 руб. Клиент, зашедший в гостиницу, обладает некоторой (случайной) суммой и получает номер по своим финансовым возможностям, если тот свободен. При наличии денег и разных по стоимости номеров он выбирает случайный номер. Если доступных по цене свободных номеров нет, клиент уходит искать ночлег в другое место. Клиенты порождаются динамически и уничтожаются при освобождении номера или уходе из гостиницы при невозможности оплаты. Создать приложение, моделирующее работу гостиницы. Каждого клиента реализовать в виде отдельного процесса.

Результаты предоставлены в приложенных файлах (на 4 - main, на 5 - main2  и на 6 - main3, соответственно).

### Схема работы:
Создается массив комнат (10 по 2000 денег, 10 по 4000 денег и 5 по 6000 денег). Затем пока есть свободные комнаты по одному вызываются клиенты каждый в отдельный поток. Каждому клиенту случайным образом содзается бюджет и, если этого бюджета хватает, чтобы заселиться в свободную комнату, то в массиве случайная комната соотвтетствующая бюджету бронируется на id данного клиента, затем процесс этого клиента засыпает (якобы клиент там что-то делает и не выходит). Далее комната освобождается. Если подходящей комнаты не нашлось, то клиент "взрывается" от возмущения...

#### Работа на 4 (Множество процессов взаимодействуют с использованием именованных POSIX семафоров. Обмен данными ведется через разделяемую память в стандарте POSIX.)
```c
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
```
#### Программа в действии:
```
Room 17 was booked by 0 for 4000
Room 18 was booked by 1 for 4000
Room 2 was booked by 2 for 2000
Room 10 was booked by 3 for 2000
Room 1 was booked by 4 for 2000
Room 20 was booked by 5 for 4000
Room 8 was booked by 6 for 2000
Room 15 was booked by 7 for 4000
Room 5 was booked by 8 for 2000
Room 25 was booked by 9 for 6000
Room 9 was booked by 10 for 2000
Room 3 was booked by 11 for 2000
Room 19 was booked by 12 for 4000
13 tried to book a room without money and bursted
3 has left the room 9
Room 10 was booked by 14 for 2000
Room 4 was booked by 15 for 2000
Room 13 was booked by 16 for 4000
7 has left the room 14
8 has left the room 4
Room 5 was booked by 17 for 2000
Room 15 was booked by 18 for 4000
Room 6 was booked by 19 for 2000
Room 21 was booked by 20 for 6000
Room 7 was booked by 21 for 2000
Room 12 was booked by 22 for 4000
23 tried to book a room without money and bursted
24 tried to book a room without money and bursted
Room 22 was booked by 25 for 6000
26 tried to book a room without money and bursted
Room 16 was booked by 27 for 4000
Room 14 was booked by 28 for 4000
Room 23 was booked by 29 for 6000
Room 11 was booked by 30 for 4000
Room 24 was booked by 31 for 6000

Process finished with exit code 0
```

#### Работа на 5 (Множество процессов взаимодействуют с использованием неименованных POSIX семафоров расположенных в разделяемой памяти. Обмен данными также ведется через разделяемую память в стандарте POSIX.)
```c
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
```
#### Программа в действии:
```
Room 1 was booked by 0 for 2000
1 tried to book a room without money and bursted
Room 2 was booked by 2 for 2000
Room 3 was booked by 3 for 2000
Room 4 was booked by 4 for 2000
Room 5 was booked by 5 for 2000
Room 6 was booked by 6 for 2000
Room 7 was booked by 7 for 2000
Room 8 was booked by 8 for 2000
Room 9 was booked by 9 for 2000
Room 10 was booked by 10 for 2000
11 tried to book a room without money and bursted
Room 11 was booked by 12 for 4000
Room 12 was booked by 13 for 4000
14 tried to book a room without money and bursted
Room 13 was booked by 15 for 4000
Room 14 was booked by 16 for 4000
Room 15 was booked by 17 for 4000
18 tried to book a room without money and bursted
Room 16 was booked by 19 for 4000
Room 17 was booked by 20 for 4000
Room 18 was booked by 21 for 4000
Room 19 was booked by 22 for 4000
Room 20 was booked by 23 for 4000
24 tried to book a room without money and bursted
25 tried to book a room without money and bursted
26 tried to book a room without money and bursted
Room 21 was booked by 27 for 6000
28 tried to book a room without money and bursted
Room 22 was booked by 29 for 6000
Room 23 was booked by 30 for 6000
0 has left the room 0
Room 1 was booked by 31 for 2000
Room 24 was booked by 32 for 6000
33 tried to book a room without money and bursted
2 has left the room 1
Room 2 was booked by 34 for 2000
3 has left the room 2
35 tried to book a room without money and bursted
4 has left the room 3
Room 3 was booked by 36 for 2000
5 has left the room 4
Room 4 was booked by 37 for 2000
Room 5 was booked by 38 for 2000
39 tried to book a room without money and bursted
40 tried to book a room without money and bursted
Room 25 was booked by 41 for 6000

Process finished with exit code 0
```
#### Работа на 6 (Множество процессов взаимодействуют с использованием семафоров в стандарте UNIX SYSTEM V. Обмен данными ведется через разделяемую память в стандарте UNIX SYSTEM V.)
```c
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
```
#### Программа в действии:
```
Room 17 was booked by 0 for 4000
Room 18 was booked by 1 for 4000
Room 8 was booked by 2 for 2000
Room 2 was booked by 3 for 2000
4 tried to book a room without money and bursted
Room 13 was booked by 5 for 4000
Room 9 was booked by 6 for 2000
Room 10 was booked by 7 for 2000
Room 12 was booked by 8 for 4000
Room 11 was booked by 9 for 4000
Room 5 was booked by 10 for 2000
Room 7 was booked by 11 for 2000
Room 1 was booked by 12 for 2000
Room 3 was booked by 13 for 2000
Room 14 was booked by 14 for 4000
Room 16 was booked by 15 for 4000
Room 4 was booked by 16 for 2000
Room 6 was booked by 17 for 2000
18 tried to book a room without money and bursted
Room 20 was booked by 19 for 4000
Room 22 was booked by 20 for 6000
21 tried to book a room without money and bursted
Room 21 was booked by 22 for 6000
Room 25 was booked by 23 for 6000
Room 15 was booked by 24 for 4000
25 tried to book a room without money and bursted
Room 19 was booked by 26 for 4000
Room 23 was booked by 27 for 6000
Room 24 was booked by 28 for 6000

Process finished with exit code 0
```
