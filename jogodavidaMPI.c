#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define N 2048
#define MAX_ITER 2000

// struct para os valores dos vizinhos de uma célula
typedef struct viz_t {
    float media;
    int vivos;
} viz_t;

void alocarMatriz(float ***matriz);
void desalocarMatriz(float **matriz);
void vizinhos(viz_t *viz, float **grid, int x, int y);
void printSubgrid(float **grid);
void comunicaVizinhos(float **grid, int rank, int size, int local_N);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    struct timeval start_time, end_time;
    double elapsed_time;

    int local_N = N / size;
    int celulas_vivas = 0;

    float **grid, **new_grid;
    alocarMatriz(&grid);
    alocarMatriz(&new_grid);

    // Inicializar grid
    if(rank == 0){
        int lin = 1, col = 1;
        grid[lin][col+1] = 1.0;
        grid[lin+1][col+2] = 1.0;
        grid[lin+2][col] = 1.0;
        grid[lin+2][col+1] = 1.0;
        grid[lin+2][col+2] = 1.0;
        lin = 10, col = 30;
        grid[lin][col+1] = 1.0;
        grid[lin][col+2] = 1.0;
        grid[lin+1][col] = 1.0;
        grid[lin+1][col+1] = 1.0;
        grid[lin+2][col+1] = 1.0;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    gettimeofday(&start_time, NULL);

    for (int iter = 1; iter <= MAX_ITER; iter++) {
        celulas_vivas = 0;

        // Comunicar bordas com os vizinhos
        comunicaVizinhos(grid, rank, size, local_N);

        for (int i = rank * local_N; i < (rank + 1) * local_N; i++) {
            for (int j = 0; j < N; j++) {
                viz_t viz;
                viz.media = 0.0;
                viz.vivos = 0;
                vizinhos(&viz, grid, i, j);

                if (grid[i][j] != 0.0) { // cell is alive
                    if (viz.vivos < 2 || viz.vivos > 3)
                        new_grid[i][j] = 0.0;
                    else {
                        new_grid[i][j] = 1.0;
                        celulas_vivas++;
                    }
                } else { // cell is dead
                    if (viz.vivos == 3) {
                        new_grid[i][j] = viz.media;
                        celulas_vivas++;
                    } else
                        new_grid[i][j] = 0.0;
                }
            }
        }

        float **temp = grid;
        grid = new_grid;
        new_grid = temp;

        MPI_Barrier(MPI_COMM_WORLD);

        int total_celulas_vivas = 0;
        MPI_Reduce(&celulas_vivas, &total_celulas_vivas, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0) printf("Geração %d: %d\n", iter, total_celulas_vivas);
    }

    gettimeofday(&end_time, NULL);

    if (rank == 0) {
        printf("-------Execução Finalizada-------\n");

        elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                       (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

        printf("Tempo de execução: %lf segundos\n", elapsed_time);
    }

    desalocarMatriz(grid);
    desalocarMatriz(new_grid);

    MPI_Finalize();

    return 0;
}

void comunicaVizinhos(float **grid, int rank, int size, int local_N) {
    MPI_Status status;

    // Comunicar com vizinho anterior
    MPI_Sendrecv(
        grid[rank * local_N], N, MPI_FLOAT, (rank - 1 + size) % size, 0,
        grid[rank * local_N], N, MPI_FLOAT, (rank + 1) % size, 0, 
        MPI_COMM_WORLD, &status);

    // Comunicar com vizinho posterior
    MPI_Sendrecv(
        grid[(rank + 1) * local_N - 1], N, MPI_FLOAT, (rank + 1) % size, 0,
        grid[(rank + 1) * local_N - 1], N, MPI_FLOAT, (rank - 1 + size) % size, 0, 
        MPI_COMM_WORLD, &status);
}



void vizinhos(viz_t *viz, float** grid, int x, int y){
    int aux_i, aux_j;

    for(int i = x - 1; i <= x + 1; i++){
        for(int j = y - 1; j <= y + 1; j++){
            if(i == x && j == y) continue;
            
            aux_i = i;
            aux_j = j;
            // simular borda infinita
            if(i < 0) i = 2047;
            else if(i >= N) i = 0;
            if(j < 0) j = 2047;
            else if(j >= N) j = 0;
            
            if(grid[i][j] != 0.0) viz->vivos++;
            viz->media += grid[i][j];
            i = aux_i;
            j = aux_j;      
        }
    }
    viz->media /= 8.0;
}


void alocarMatriz(float ***matriz){
    *matriz = (float **)malloc(N * sizeof(float *));
    for (int i = 0; i < N; i++) (*matriz)[i] = (float *)malloc(N * sizeof(float));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            (*matriz)[i][j] = 0.0;
        }
    }
}


void desalocarMatriz(float **matriz){
    for(int i = 0; i < N; i++) free(matriz[i]);
    free(matriz);
}