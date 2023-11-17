/*
Daniel Ferreira Martins 156369
Juan Marcos Martins 156470
Savio Augusto Machado Araujo 156584
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define N 2048
#define MAX_ITER 2000
#define MAX_THREADS 2
#define STEP (N / MAX_THREADS)
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_WHITE "\x1b[37m"

typedef struct viz_t{
    float media;
    int vivos;
} viz_t;

typedef struct args_t{
    int start;
    float **grid;
    float **new_grid;
} args_t;

void alocarMatriz(float ***matriz);
void desalocarMatriz(float **matriz);
void copiarMatriz(float **m1, float **m2);
void vizinhos(viz_t *viz, float **grid, int x, int y);
void *threadFunc(void *arg);
int celulasVivas(float **grid);
void printSubgrid(float **grid);

int main() {

    struct timeval start_time, end_time;
    double elapsed_time;

    float **grid, **new_grid;
    alocarMatriz(&grid);
    alocarMatriz(&new_grid);

    int lin = 1, col = 1;
    grid[lin][col + 1] = 1.0;
    grid[lin + 1][col + 2] = 1.0;
    grid[lin + 2][col] = 1.0;
    grid[lin + 2][col + 1] = 1.0;
    grid[lin + 2][col + 2] = 1.0;
    lin = 10, col = 30;
    grid[lin][col + 1] = 1.0;
    grid[lin][col + 2] = 1.0;
    grid[lin + 1][col] = 1.0;
    grid[lin + 1][col + 1] = 1.0;
    grid[lin + 2][col + 1] = 1.0;

    gettimeofday(&start_time, NULL);

    printf("Condicao Inicial: %d\n", celulasVivas(grid));
    pthread_t t[MAX_THREADS];
    args_t args[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        alocarMatriz(&(args[i].grid));
        alocarMatriz(&(args[i].new_grid));
        copiarMatriz(args[i].grid, grid);
        copiarMatriz(args[i].new_grid, new_grid);
        
        args[i].start = i * STEP;
    }

    for (int k = 1; k <= MAX_ITER; k++) {
        for (int i = 0; i < MAX_THREADS; i++) pthread_create(&t[i], NULL, threadFunc, (void *)&args[i]);
        for (int i = 0; i < MAX_THREADS; i++) pthread_join(t[i], NULL);

        for(int k = 0; k < MAX_THREADS; k++){
            for (int i = 0; i < N; i++) {
                for (int j = k * STEP; j < (k * STEP) + STEP; j++) {
                    grid[j][i] = args[k].new_grid[j][i];
                }
            }
        }
        for(int k = 0; k < MAX_THREADS; k++) copiarMatriz(args[k].grid, grid);
        printf("Geracao %d: %d\n", k, celulasVivas(grid));
        printSubgrid(grid);
        system("cls");
    }

    gettimeofday(&end_time, NULL);

    printf("-------Execução Pthread finalizada(%d Threads)-------\n", MAX_THREADS);

    elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                   (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    printf("Tempo total de execução: %lf segundos\n", elapsed_time);

    desalocarMatriz(grid);
    desalocarMatriz(new_grid);

    pthread_exit(NULL);
}


int celulasVivas(float **grid) {
    int celulas_vivas = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] != 0.0) {
                celulas_vivas++;
            }
        }
    }
    return celulas_vivas;
}


void *threadFunc(void *arg) {
    args_t *args = (args_t *)arg;

    for (int j = args->start; j < args->start + STEP; j++) {
        for (int k = 0; k < N; k++) {
            viz_t viz;
            viz.media = 0.0;
            viz.vivos = 0;
            vizinhos(&viz, args->grid, j, k);

            if (args->grid[j][k] != 0.0) {
                if (viz.vivos < 2 || viz.vivos > 3)
                    args->new_grid[j][k] = 0.0;
                else
                    args->new_grid[j][k] = 1.0;
            } else {
                if (viz.vivos == 3)
                    args->new_grid[j][k] = viz.media;
                else
                    args->new_grid[j][k] = 0.0;
            }
        }
    }

    pthread_exit(NULL);
}


void vizinhos(viz_t *viz, float **grid, int x, int y) {
    int aux_i, aux_j;

    for (int i = x - 1; i <= x + 1; i++) {
        for (int j = y - 1; j <= y + 1; j++) {
            if (i == x && j == y) continue;

            aux_i = i;
            aux_j = j;
            if (i < 0) i = 2047;
            else if (i >= N) i = 0;
            if (j < 0) j = 2047;
            else if (j >= N) j = 0;

            if (grid[i][j] != 0.0) viz->vivos++;
            viz->media += grid[i][j];
            i = aux_i;
            j = aux_j;
        }
    }
    viz->media /= 8.0;
}


void printSubgrid(float **grid) {
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++){
            int simb = (int)(grid[i][j] * 10);
            if(simb == 0) printf(".  ");
            else if(simb < 10) printf(ANSI_YELLOW "o  " ANSI_WHITE);
            else if(simb == 10) printf(ANSI_GREEN "o  " ANSI_WHITE);
        }
        printf("\n");
    }
}


void alocarMatriz(float ***matriz) {
    *matriz = (float **)malloc(N * sizeof(float *));
    for (int i = 0; i < N; i++) (*matriz)[i] = (float *)malloc(N * sizeof(float));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            (*matriz)[i][j] = 0.0;
        }
    }
}


void desalocarMatriz(float **matriz) {
    for (int i = 0; i < N; i++) free(matriz[i]);
    free(matriz);
}


void copiarMatriz(float **m1, float **m2){
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            m1[i][j] = m2[i][j];
        }
    }
}

