#include <iostream>
#include <pthread.h>
#include <unistd.h>

const int pictureCapacity = 10;
const int numberOfVisitors = 50;
const int gallerySize = 50;
int countOfVisitors[5];
bool isClosing;
bool isClosed;
pthread_cond_t picture_not_full[5];
pthread_cond_t picture_not_empty[5];
bool isIn[numberOfVisitors];
bool isSeeing[numberOfVisitors];
int choosedPicture[numberOfVisitors];
int count = 0;
unsigned int seed = 101;
std::string names[] = {"Leonardo da Vinci. Madonna Litta", "Raphael. Conestabile Madonna",
                       "Titian. Penitent Magdalene", "Caravaggio. The Lute Player",
                       "Rembrandt. Return of the Prodigal Son"};
//visitor *visitors[numberOfVisitors];
pthread_mutex_t mutexCaretaker;

pthread_cond_t not_full;

pthread_cond_t not_empty;

void *seePicture(void *param) {
    int vis = *((int *) param);
    choosedPicture[vis - 1] = rand() % 5;

    if (!isClosing && isIn[vis - 1] && !isSeeing[vis - 1]) {
        pthread_mutex_lock(&mutexCaretaker);
        while (countOfVisitors[choosedPicture[vis - 1]] == pictureCapacity) {
            printf("Visitor %d is waiting for the picture '%s'\n", vis, names[choosedPicture[vis - 1]].c_str());
            pthread_cond_wait(&picture_not_full[choosedPicture[vis - 1]], &mutexCaretaker);
        }
        countOfVisitors[choosedPicture[vis - 1]]++;
        isSeeing[vis - 1] = true;
        printf("Visitor %d is seeing the picture '%s'\n", vis, names[choosedPicture[vis - 1]].c_str());
        pthread_mutex_unlock(&mutexCaretaker);
        pthread_cond_broadcast(&picture_not_empty[choosedPicture[vis - 1]]);
        sleep(rand() % 4 + 1);
    }


    return NULL;
}

void *freePicture(void *param) {
    int vis = *((int *) param);
    if (isSeeing[vis - 1] && isIn[vis - 1]) {
        pthread_mutex_lock(&mutexCaretaker);
        while (countOfVisitors[choosedPicture[vis - 1]] == 0) {
            pthread_cond_wait(&picture_not_empty[choosedPicture[vis - 1]], &mutexCaretaker);
        }
        if (!isClosed) {
            countOfVisitors[choosedPicture[vis - 1]]--;
            isSeeing[vis - 1] = false;
            printf("Visitor %d moves away from the picture '%s'\n", vis, names[choosedPicture[vis - 1]].c_str());
        }
        pthread_mutex_unlock(&mutexCaretaker);

        pthread_cond_broadcast(&picture_not_full[choosedPicture[vis - 1]]);
    }
    return NULL;
}

void *comeIn(void *param) {
    //visitor vis = *((visitor *) param);
    int vis = *((int *) param);
    sleep(rand() % 4);
    while (count == gallerySize) {
        pthread_cond_wait(&not_full, &mutexCaretaker);
    }
    pthread_mutex_lock(&mutexCaretaker);
    if (!isClosing) {
        isIn[vis - 1] = true;
        ++count;
        printf("Visitior %d came in to Hermitage\n", vis);
    }
    pthread_mutex_unlock(&mutexCaretaker);

    pthread_cond_broadcast(&not_empty);
    return NULL;
}

void *comeOut(void *param) {
    int vis = *((int *) param);
    if (!isClosing) {
        sleep(rand() % 2);
    }
    if (isIn[vis - 1]) {
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutexCaretaker);
        }
        pthread_mutex_lock(&mutexCaretaker);
        if (!isClosed) {
            isIn[vis - 1] = false;
            printf("Visitor %d: came out from Hermitage\n", vis);
            --count;
        }
        pthread_mutex_unlock(&mutexCaretaker);

        pthread_cond_broadcast(&not_full);
    }
    return NULL;
}

void *routine(void *param) {
    if (!isClosing) {
        comeIn(param);
        while (!isClosing && rand() % 101 < 76) {
            seePicture(param);
            freePicture(param);
        }
        comeOut(param);
    }
    return NULL;
}

int main() {
    srand(seed);
    int i;
    for (int j = 0; j < 5; ++j) {
        pthread_cond_init(&picture_not_full[j], NULL);
        pthread_cond_init(&picture_not_empty[j], NULL);
    }
    pthread_mutex_init(&mutexCaretaker, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_t threadP[numberOfVisitors];
    int producers[numberOfVisitors];
    for (i = 0; i < numberOfVisitors; ++i) {
        producers[i] = i + 1;
        pthread_create(&threadP[i], NULL, routine, (void *) (producers + i));
    }
    sleep(5);
    isClosing = true;
    printf("\nIT IS CLOSING TIME!\n\n");
    sleep(2);
    isClosed = true;
    printf("\nCARETAKER CLOSED THE MUSEUM!\n\n");
    for (int j = 0; j < numberOfVisitors; ++j) {
        if (isIn[j]) {
            printf("Visitor %d spends the night at the museum!\n", j);
        }
    }
    return 0;
}