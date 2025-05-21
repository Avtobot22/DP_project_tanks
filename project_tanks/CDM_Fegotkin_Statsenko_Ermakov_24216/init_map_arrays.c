#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAP_SIZE 32
#define INF 32767
#define INVALID 255

char* mapData =
    "00000000000000000000000000000000"
    "01100000000001111100000000001110"
    "01110000000011111110000000011110"
    "01111111111111111111111111111110"
    "01100001000000000000000100001110"
    "00100001000000000000000100001100"
    "00100001000000000000000100001100"
    "00110001111111111111111100011100"
    "00110001111111111111111100011100"
    "00100000000000010000000100001100"
    "01100000000000010000000100001110"
    "01000000000000010000000100000110"
    "01000001000111111111000100000110"
    "01000001000100000001000100000110"
    "01000001000100000001000100000110"
    "01111111111100000001111111111110"
    "01000001000100000001000100000110"
    "01000001000100000001000100000110"
    "01000001000111111111000100000110"
    "01000001000000010000000000000110"
    "01100001000000010000000000001110"
    "00100001000000010000000000001100"
    "00110001111111111111111100011100"
    "00110001111111111111111100011100"
    "00100001000000000000000100001100"
    "00100001000000000000000100001100"
    "01100001000000000000000100001110"
    "01111111111111111111111111111110"
    "01110000000011111110000000011110"
    "01110000000011111110000000011110"
    "01100000000001111100000000001110"
    "00000000000000000000000000000000";


int nodeIndex[MAP_SIZE * MAP_SIZE];
int nodeRow[1024];
int nodeCol[1024];
int nodeCount = 0;

int dr[4] = {0, -1, 0, 1};
int dc[4] = {-1, 0, 1, 0};

void setDirection(int* matrix, int n, int i, int j, int direction) {
    int pairIndex = (i * n + j);
    int intIndex = pairIndex / 8;
    int bitPosition = (pairIndex % 8) * 2; 

    matrix[intIndex] &= ~(3 << bitPosition);
    matrix[intIndex] |= ((direction & 3) << bitPosition);
}

int getDirection(int* matrix, int n, int i, int j) {
    int pairIndex = (i * n + j);
    int intIndex = pairIndex / 8;
    int bitPosition = (pairIndex % 8) * 2;
    
    return (matrix[intIndex] >> bitPosition) & 3;
}

void initMap() {
    nodeCount = 0;
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            if (mapData[i * MAP_SIZE + j] == '1') {
                nodeIndex[i * MAP_SIZE + j] = nodeCount;
                nodeRow[nodeCount] = i;
                nodeCol[nodeCount] = j;
                nodeCount++;
            } else {
                nodeIndex[i * MAP_SIZE + j] = -1;
            }
        }
    }
}

void writeArrays(int* nextMatrix, int n, int intsNeeded) {
    FILE *file = fopen("map_arrays.c", "w");
    
    fprintf(file, "int pathNextMatrix[%d] = {\n    ", intsNeeded);
    for (int i = 0; i < intsNeeded; i++) {
        fprintf(file, "0x%04X", (unsigned int)(nextMatrix[i] & 0xFFFF));
        if (i < intsNeeded - 1) fprintf(file, ", ");
        if ((i + 1) % 8 == 0 && i < intsNeeded - 1) fprintf(file, "\n    ");
    }
    fprintf(file, "\n};\n\n");
    
    fprintf(file, "// Координаты узлов - строки\n");
    fprintf(file, "int nodeRow[%d] = {\n    ", n);
    for (int i = 0; i < n; i++) {
        fprintf(file, "%d", nodeRow[i]);
        if (i < n - 1) fprintf(file, ", ");
        if ((i + 1) % 16 == 0 && i < n - 1) fprintf(file, "\n    ");
    }
    fprintf(file, "\n};\n\n");
    
    fprintf(file, "// Координаты узлов - столбцы\n");
    fprintf(file, "int nodeCol[%d] = {\n    ", n);
    for (int i = 0; i < n; i++) {
        fprintf(file, "%d", nodeCol[i]);
        if (i < n - 1) fprintf(file, ", ");
        if ((i + 1) % 16 == 0 && i < n - 1) fprintf(file, "\n    ");
    }
    fprintf(file, "\n};\n\n");

    fprintf(file, "int nodeIndex[1024] = {\n    ");
    for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
        fprintf(file, "%d", nodeIndex[i]);
        if (i < MAP_SIZE * MAP_SIZE - 1) fprintf(file, ", ");
        if ((i + 1) % 16 == 0 && i < MAP_SIZE * MAP_SIZE - 1) fprintf(file, "\n    ");
    }
    fprintf(file, "\n};\n\n");
    fclose(file);
}

void initDist(int16_t* dist, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j)
                dist[i * n + j] = 0;
            else
                dist[i * n + j] = INF;
        }
    }
}

int main() {
    initMap();
    int n = nodeCount;
    int totalNodes = n * n;

    int16_t* dist = malloc(totalNodes * sizeof(int16_t));
    uint8_t* nextFull = malloc(totalNodes * sizeof(uint8_t));

    initDist(dist, n);

    memset(nextFull, INVALID, totalNodes * sizeof(uint8_t));

    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            if (mapData[i * MAP_SIZE + j] == '1') {
                int node = nodeIndex[i * MAP_SIZE + j];
                for (int d = 0; d < 4; d++) {
                    int ni = i + dr[d];
                    int nj = j + dc[d];
                    if (ni >= 0 && ni < MAP_SIZE && nj >= 0 && nj < MAP_SIZE && mapData[ni * MAP_SIZE + nj] == '1') {
                        int neighborNode = nodeIndex[ni * MAP_SIZE + nj];
                        dist[node * n + neighborNode] = 1;
                        nextFull[node * n + neighborNode] = d;
                    }
                }
            }
        }
    }
    
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            if (dist[i * n + k] == INF) continue;
            for (int j = 0; j < n; j++) {
                int new_dist = dist[i * n + k] + dist[k * n + j];
                if (dist[k * n + j] != INF && dist[i * n + j] > new_dist) {
                    dist[i * n + j] = new_dist;
                    nextFull[i * n + j] = nextFull[i * n + k];
                }
            }
        }
    }

    int totalDirections = n * n;
    int elementsPerInt = 8;
    int intsNeeded = (totalDirections + elementsPerInt - 1) / elementsPerInt;

    int* nextMatrix = calloc(intsNeeded, sizeof(int));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            uint8_t val = nextFull[i * n + j];
            if (val == INVALID) {
                val = 0;
            }
            setDirection(nextMatrix, n, i, j, val);
        }
    }

    writeArrays(nextMatrix, n, intsNeeded);

    free(nextFull);
    free(dist);
    free(nextMatrix);

    return 0;
}