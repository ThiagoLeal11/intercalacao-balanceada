#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err34-c"

#define true 1
#define false 0
#define bool int

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct Merge{
    int tapeIndex;
    int index;
    bool isEnd;
} Merge;

typedef struct TapeRange {
    int start;
    int end;
} TapeRange;

void bubbleSort(int* u, int n) {
    int aux;
    int i;
    int j;

    for (i=0; i<n; i++) {
        for (j=n-1; j>i; j--) {
            if (u[j-1] > u[j]) {
                aux = u[j-1];
                u[j-1] = u[j];
                u[j] = aux;
            }
        }
    }
}

int getMinIndex(const int* v, int n) {
    int i, min, index;

    // Init vars
    min = v[0];
    index = 0;

    // Get the min index from vector
    for (i = 0; i < n; i++) {
        if (v[i] != -1 && v[i] < min) {
            min = v[i];
            index = i;
        }
    }

    return index;
}

int externalMergeSort(char *filename, int memory, int tapes) {
    char name[20];
    int i, j;
    bool isEnd;
    int size, n, tapeSelector, tapeBlock, iter, count, tapesEnded, blockEnded;
    int aux[memory];
    char c;
    TapeRange readTapes, writeTapes;

    // Open the file
    FILE *input = fopen("teste.txt", "r+");

    // Cant open the file
    if (!input) {
        return -1;
    }

    // init tapes
    FILE *_tape[tapes];

    // Create the tapes
    for (i = 0; i < tapes; i++) {
        sprintf(name, "aux%d.txt", i);
        _tape[i] = fopen(name, "w");
    }

    /*
     * Read all the numbers from file and put every sort memory bloc on each tape.
     */
    isEnd = false;

    // Init control variables
    size = 0; // Counter of numbers in file.
    n = 0; // Actual read number.
    tapeSelector = 0; // The tape for save the memory.
    tapeBlock = (memory < tapes-2) ? memory : tapes-2; // Number of tapes blocks for each merge.

    // Iterate over the file
    while (!isEnd){
        // Read from the file.
        for (i = 0; i < memory & !isEnd; i++) {
            if (fscanf(input, "%d,", &n) == 1) {
                aux[i] = n;
                size++;
            } else {
                isEnd = true;
            }
        }

        // Sort the memory.
        bubbleSort(aux, i);

        // Save the numbers on tape.
        for (j = 0; j < i; j++) {
            fprintf(_tape[tapeSelector], "%d", aux[j]);
            if (j < i-1) {
                fprintf(_tape[tapeSelector], ",");
            } else {
                fprintf(_tape[tapeSelector], ";");
            }
        }

        // Increment or reset tapeSelector.
        if (tapeSelector < tapeBlock-1) {
            tapeSelector += 1;
        } else {
            tapeSelector = 0;
        }
    }

    // Calculate the number of iterations
    iter = (int) round(log10((double) size/memory)/log10(3));

    // Control variables
    readTapes.start = 0;
    readTapes.end = tapeBlock;
    writeTapes.start = tapeBlock;
    writeTapes.end = (writeTapes.start + tapeBlock < tapes) ? writeTapes.start + tapeBlock : tapes;

    int k; // Index of aux.
    int t; // Tape index.
    int w; // Write index.
    int min;

    // Initiate the merge.
    blockEnded = 0;
    int readTapeRange;
    for (j = 0; j < iter; j++) {
        // Open files to read.
        for (i = readTapes.start; i < readTapes.end; i++) {
            sprintf(name, "aux%d.txt", i);
            _tape[i] = fopen(name, "r");
        }

        // Open files to write.
        for (i = writeTapes.start; i < writeTapes.end; i++) {
            sprintf(name, "aux%d.txt", i);
            _tape[i] = fopen(name, "w");
        }

        t = readTapes.start;
        k = 0;
        readTapeRange = readTapes.end - readTapes.start;

        // Mount the aux vector
        while (k < memory && blockEnded < readTapeRange) {
            // Read the number
            if(fscanf(_tape[t], "%d", &n) == 1) {
                aux[k++] = n;

                fscanf(_tape[t], "%c", &c);
                if (c == ';') {
                    blockEnded++;
                }

                // Check for reset tape index;
                if (++t == readTapes.end) {
                    t = readTapes.start;
                }

            }
        }

        // Fill the rest of aux.
        while (k < memory) {
            aux[k++] = -1;
        }

        // Get the mim index
        min = getMinIndex(aux, memory);

        w = writeTapes.start;

        // Merge the block in the tape
        while (blockEnded < readTapeRange) {
//            fprintf(_tape[w], "%d")

            if(fscanf(_tape[t], "%d", &n) == 1) {
                aux[k++] = n;

                fscanf(_tape[t], "%c", &c);
                if (c == ';') {
                    blockEnded++;
                }

                // Check for reset tape index;
                if (++t == readTapes.end) {
                    t = readTapes.start;
                }

            }
        }

        // Go to next tape on the range or reset.



        // Copy to new tape



        // Calculate first and last tape.
        if (lastTape + tapeBlock >= tapes) {

        }
    }

    return 0;
}


// Leia o arquivo colocando cada par de numeros em uma fita

// Itere até o arquivo estar ordenado
  // Leio o bloco intercalado colocando na proxima sequencia de fita


int main() {
    externalMergeSort("teste.txt", 3, 6);

    return 0;
}