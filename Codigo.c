#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma comment(lib,"pthreadVC2.lib")
#define HAVE_STRUCT_TIMESPEC

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define N 15000           // Tamanho da matriz (N x N)
#define NUM_THREADS 4 // N�mero de threads
#define MACROBLOCK_ROWS 15000 // N�mero de linhas dos macroblocos
#define MACROBLOCK_COLS 15000  // N�mero de colunas dos macroblocos

int** matriz;
int count_primos_serial = 0;
int count_primos_paralelo = 0;
pthread_mutex_t mutex;

// Fun��o para verificar se um n�mero � primo
int ehPrimo(int n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

// Fun��o executada por cada thread para contar primos em sua parte da matriz
void* contarPrimosParalelo(void* arg) {
    int id = *(int*)arg;
    int local_count = 0;

    for (int i = id; i < N; i += NUM_THREADS) {
        for (int j = 0; j < N; ++j) {
            if (ehPrimo(matriz[i][j])) {
                local_count++;
            }
        }
    }

    pthread_mutex_lock(&mutex);
    count_primos_paralelo += local_count;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

// Fun��o para gerar a matriz de n�meros aleat�rios
void gerarMatriz() {
    matriz = (int**)malloc(N * sizeof(int*));
    if (matriz == NULL) {
        perror("Erro ao alocar matriz");
        exit(1);
    }

    for (int i = 0; i < N; ++i) {
        matriz[i] = (int*)malloc(N * sizeof(int));
        if (matriz[i] == NULL) {
            perror("Erro ao alocar linha da matriz");
            exit(1);
        }
        for (int j = 0; j < N; ++j) {
            matriz[i][j] = rand() % 32000;
        }
    }
}

// Fun��o para contar primos na matriz de forma serial
void contarPrimosSerial() {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (ehPrimo(matriz[i][j])) {
                count_primos_serial++;
            }
        }
    }
}

int main() {
    srand(0); // Semente fixa para a gera��o dos n�meros aleat�rios
    gerarMatriz();

    // Contagem Serial
    clock_t start_serial = clock();
    contarPrimosSerial();
    clock_t end_serial = clock();
    double time_serial = (double)(end_serial - start_serial) / CLOCKS_PER_SEC;

    // Contagem Paralela
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    pthread_mutex_init(&mutex, NULL);

    clock_t start_parallel = clock();
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, contarPrimosParalelo, (void*)&thread_ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    clock_t end_parallel = clock();
    double time_parallel = (double)(end_parallel - start_parallel) / CLOCKS_PER_SEC;

    pthread_mutex_destroy(&mutex);

    printf("Primos Serial: %d\n", count_primos_serial);
    printf("Tempo Serial: %f segundos\n", time_serial);
    printf("Primos Paralelo: %d\n", count_primos_paralelo);
    printf("Tempo Paralelo: %f segundos\n", time_parallel);
    printf("Speedup: %f", time_serial / time_parallel);

    // Libera��o da mem�ria
    for (int i = 0; i < N; ++i) {
        free(matriz[i]);
    }
    free(matriz);

    return 0;
}
