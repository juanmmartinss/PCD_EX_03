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
void comunicaVizinhos(float **grid, int rank, int size);

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
    for (int i = rank * local_N; i < (rank + 1) * local_N; i++) {
        for (int j = 0; j < N; j++) {
            int offset = rank * local_N;

            int lin1 = offset + 1, col1 = 1;
            if (i == lin1 && j >= col1 && j < col1 + 5) grid[i][j] = 1.0;

            int lin2 = offset + 10, col2 = 30;
            if (i >= lin2 && i < lin2 + 3 && j >= col2 && j < col2 + 3) grid[i][j] = 1.0;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    gettimeofday(&start_time, NULL);

    for (int iter = 1; iter <= MAX_ITER; iter++) {
        celulas_vivas = 0;

        // Comunicar bordas com os vizinhos
        comunicaVizinhos(grid, rank, size);

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

        if (rank == 0) {
            printf("Generation %d: %d\n", iter, celulas_vivas);
        }
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

void comunicaVizinhos(float **grid, int rank, int size) {
    MPI_Status status;

    // Enviar borda superior rank - 1
    if (rank > 0) {
        MPI_Send(grid[rank * (N / size)], N, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD);
    }

    // Receber borda superior rank - 1
    if (rank > 0) {
        MPI_Recv(grid[(rank - 1) * (N / size) + N / size], N, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD, &status);
    }

    // Enviar borda inferior rank + 1
    if (rank < size - 1) {
        MPI_Send(grid[(rank + 1) * (N / size) - 1], N, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD);
    }

    // Receber borda inferior rank + 1
    if (rank < size - 1) {
        MPI_Recv(grid[rank * (N / size) + N / size], N, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD, &status);
    }
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