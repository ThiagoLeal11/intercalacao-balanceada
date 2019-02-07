#define true 1
#define false 0
#define bool int
#define maxPrintSize 20

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "libs/tapes.h"
#include "libs/render.h"

void quickSort(int *array, int start, int end) {
    int aux;
    int i = start;
    int j = end - 1;
    int pivot = array[ (start + end) / 2 ];
    while (i <= j) {
        while (i<end && array[i] < pivot)
            i++;
        while (j>start && array[j] > pivot)
            j--;
        if (i <= j) {
            aux = array[i];
            array[i] = array[j];
            array[j] = aux;
            i++;
            j--;
        }
    }
    if (i < end)
        quickSort (array, i, end);
    if (j > start)
        quickSort (array, start, j+1);
}


// Returns the index of the lowest present values in the vector ignoring the -1 values.
int getMinIndex(const int* v, int n) {
    int i, min, index;

    // Init mim and index with the first array value.
    // If this value is empty, init index with -2.
    min = v[0];
    index = (v[0] == -1) ? -2 : 0;

    // Get the min valid index from vector.
    for (i = 0; i < n; i++) {
        if ( v[i] != -1 && (min == -1 || v[i] < min) ) {
            min = v[i];
            index = i;
        }
    }

    return index;
}


// Check if every position in memory is empty.
// This indicates that all the block numbers have already been read.
bool isBlockEnd(const int memory[], int memorySize) {
    int i;
    int count = 0;

    for (i = 0; i < memorySize; i++) {
        if (memory[i] == -1) {
            count++;
        }
    }

    return (count == memorySize);
}


// Return the next valid index for an specific range.
int getNextIndexOfRange(int totalSize, TapeRange range, int idx) {
    // Adjust the block size for index range.
    totalSize -= 1;

    // Increment index.
    idx++;

    // Check for the range.
    if (range.start <= range.end) {
        return (idx <= range.end) ? idx : range.start;
    } else {
        if (idx >= range.start) {
            return (idx <= totalSize) ? idx : 0;
        } else {
            return (idx <= range.end) ? idx : range.start;
        }
    }
}


// Return the size of a range based on o max range given by 0 and totalSize.
int calcRangeSize(int totalSize, TapeRange range) {

    if (range.start <= range.end) {
        return range.end - range.start + 1;
    } else {
        return totalSize - range.end + range.start + 1;
    }
}


// Get a new valid range based on a super range given by 0 and totalSize excluding the protect range.
TapeRange getNewRange(int totalSize, int rangeSize, TapeRange blockedRange) {
    int size = 1;
    int idx;
    int freeSize = totalSize - calcRangeSize(totalSize, blockedRange);
    TapeRange newRange;

    // Get the start of range.
    newRange.start = (blockedRange.end + 1 <= totalSize-1) ? blockedRange.end + 1 : 0;
    idx = newRange.start;
//    newRange.end = (blockedRange.start - 1 >= 0) ? blockedRange.start - 1 : totalSize - 1;

    while (size < rangeSize && size < freeSize) {
        // Increment or reset idx.
        if (idx < totalSize - 1) {
            idx += 1;
        } else {
            idx = 0;
        }
        size++;
    }

    newRange.end = idx;

    return newRange;
}

bool isIndexOnRange(TapeRange range, int idx) {
    if (range.start <= range.end) {
        return (idx >= range.start && idx <= range.end);
    } else {
        return (idx <= range.end && idx >= range.start);
    }
}


// Sort the batches in memory and save them on the tapes in interleaved mode.
int firstMergePass(FILE *input, FILE *auxTapes[], TapeRange range, int memorySize, const char *divider) {
    bool isFileEnd = false;
    int i, j, n, size=0;
    int memory[memorySize];
    int tapeSelector = range.start;

    // Open tapes for write.
    openAuxTapesForWrite(auxTapes, range);

    // Read all the numbers from input.
    while (!isFileEnd) {
        // Read numbers from the file and fill the memory
        for (i = 0; i < memorySize & !isFileEnd; i++) {
            if (fscanf(input, "%d", &n) == 1) {
                memory[i] = n;
                size++;
                fscanf(input, divider);
            } else {
                isFileEnd = true;
                i--;
            }
        }

        // Sort the memory.
        quickSort(memory, 0, i);

        // Save the numbers from memory on tape.
        for (j = 0; j < i; j++) {
            writeOnAuxTape(auxTapes[tapeSelector], memory[j]);
        }

        // Save -1 indicating that block ended.
        writeOnAuxTape(auxTapes[tapeSelector], -1);

        // Increment or reset tapeSelector.
        if (tapeSelector < range.end) {
            tapeSelector += 1;
        } else {
            tapeSelector = 0;
        }
    }

    // Close tapes before write
    closeAuxTapes(auxTapes, range);

    return size;
}


// Merge each block on a given tape range. Return false if nothing was done.
bool mergeBlock(FILE *auxTapes[], TapeRange read, int writeIdx, int memorySize) {
    int i;

    int memory[memorySize];
    int mIdx = 0;
    // Read the first block number of each tape.
    for (i = read.start; i <= read.end; i++) {
        memory[mIdx++] = readNumberFromAuxTape(auxTapes[i]);
    }

    // Fill the unused memory.
    for (i = mIdx; i < memorySize; i++) {
        memory[i] = -1;
    }

    // If range is empty, return false
    if (isBlockEnd(memory, memorySize)) {
        return false;
    }

    // Merge the entirely read range and write into write tape
    while (!isBlockEnd(memory, memorySize)) {
        // Get the mim index from read tapes.
        mIdx = getMinIndex(memory, memorySize);

        // Write the mim number on specific tape.
        writeOnAuxTape(auxTapes[writeIdx], memory[mIdx]);

        // Get the next number from tape.
        memory[mIdx] = readNumberFromAuxTape(auxTapes[read.start + mIdx]);
    }

    // Write -1 to indicate that block ended.
    writeOnAuxTape(auxTapes[writeIdx], -1);

    return true;
}


// Merge each blocks in read tapes range to put on write tapes range.
void mergeTapesBlocks(FILE *auxTapes[], int auxTapesQnt, TapeRange readRange, TapeRange writeRange, int memorySize) {
    int idx;
    bool canMerge;

    // Open the tapes for files operations.
    openAuxTapesForRead(auxTapes, readRange);
    openAuxTapesForWrite(auxTapes, writeRange);

    // Merge each block contained in read range and put it in order on write tapes.
    idx = writeRange.start;
    do {
        // Merge the blocks.
        canMerge = mergeBlock(auxTapes, readRange, idx, memorySize);

        // Update index of write tape.
        idx = getNextIndexOfRange(auxTapesQnt, writeRange, idx);
    } while (canMerge);

    // Close the tapes utilized.
    closeAuxTapes(auxTapes, readRange);
    closeAuxTapes(auxTapes, writeRange);
}


// Make the external merge based on 3 steps:
// 1. Read the file sorting the batch based on memory size and put it in a tape ranges.
// 2. Merge each block and put in another tape range. Do it while has to blocks in the same tape.
// 3. Merge the tapes and copy it to the final tape.
int externalMergeSort(const char *inputFilename, const char *outputFilename, const char *divider, int memorySize, int auxTapesQnt) {
    int rangeSize, size, i, iter, count, maxPrint;
    TapeRange readRange, writeRange;
    int readPrint = 0;


    if (memorySize <= 1) {
        printf("Impossible to run the sort, insufficient memory");
        return -1;
    }

    /*
     * Tapes.
     */

    // Open the file for input.
    FILE *input = fopen(inputFilename, "r");

    // Cant open the file.
    if (!input) {
        printf("Can't open the %s file for input.", inputFilename);
        return -1;
    }

    // init aux tapes.
    FILE *auxTapes[auxTapesQnt];

    /*
     * First merge.
     */

    // Define the max tape range size.
    rangeSize = (memorySize < auxTapesQnt - 2) ? memorySize : auxTapesQnt - 2;

    // Init tapes range.
    readRange.start = auxTapesQnt;
    readRange.end = auxTapesQnt;
    writeRange = getNewRange(auxTapesQnt, rangeSize, readRange);

    // Read all the numbers from file and put every sort memory bloc on each tape.
    size = firstMergePass(input, auxTapes, writeRange, memorySize, divider);

    // Close the input.
    fclose(input);

    /*
     * Additional merges.
     */

    // Calc the number of times to merge.
    int minRangeSize = (rangeSize < auxTapesQnt - rangeSize) ? rangeSize : auxTapesQnt - rangeSize;
    iter = (int) ceil(log10((double) size / memorySize) / log10(minRangeSize));

    for (i = 0; i < iter; i++) {
        // Update the ranges
        readRange = writeRange;
        writeRange = getNewRange(auxTapesQnt, rangeSize, readRange);

        // Merge the blocks on read tapes and put on write tapes.
        mergeTapesBlocks(auxTapes, auxTapesQnt, readRange, writeRange, memorySize);

        /*
         * Print
         */

        // Print header
        printf("\n\n|  Tape  |  Read  |  Write  |  Numbers\n");
        printDivision("+", "-", 4, 8, 8, 9, 139);

        // Print the tapes state
        for (int j = 0; j < auxTapesQnt; j++) {
            printf("|   %d    |   %c    |    %c    | " ,
                    j,
                    (isIndexOnRange(readRange, j) ? '*' : ' '),
                    (isIndexOnRange(writeRange, j)) ? '*' : ' ');

            TapeRange auxRange = {j, j};
            openAuxTapesForRead(auxTapes, auxRange);

            count = 0;
            maxPrint = 0;

            do {
                readPrint = readNumberFromAuxTape(auxTapes[j]);
                if (readPrint != -1) {
                    if (maxPrint < maxPrintSize) {
                        printf("%5d ", readPrint);
                        maxPrint++;
                    } else {
                        printf("\n|%8s|%8s|%9s| %5d ", "", "", "", readPrint);
                        maxPrint = 1;
                    }
                    count = 0;
                } else {
                    count++;
                    if (count == 1) {
                        printf("  | ");
                    }
                }

            } while (count < 2);
            printf("\n");
            printDivision("+", "-", 4, 8, 8, 9, 139);
        }

    }

    /*
     * Last Merge.
     */

    // Update ReadRange.
    readRange = writeRange;

    // Get the final range of length 1.
    writeRange = getNewRange(auxTapesQnt, 1, readRange);

    // Make the last merge to output file.
    mergeTapesBlocks(auxTapes, auxTapesQnt, readRange, writeRange, memorySize);

    // Open the file for output.
    FILE *output = fopen(outputFilename, "w");

    // Cant open the file
    if (!output) {
        printf("Can't open the %s file for output.", inputFilename);
        return -1;
    }

    // Update and open the readRange.
    readRange = writeRange;
    openAuxTapesForRead(auxTapes, readRange);

    // Copy the sort vector for the final file.
    copyForOutputFile(auxTapes[readRange.start], output, divider);

    fclose(output);

    return 0;
}

void createRandonTape(const char *divider, int size, char *fileName){

    // Open input file.
    FILE *input;
    input = fopen(fileName, "w");

    if (input) {
        for (int i = 0; i < size; i++){
            fprintf(input, "%d%s", rand() % 999, divider);
        }
    } else {
        printf("Can't create the file! ");
    }

    fclose(input);
}

int main() {

    int option = 1;
    int memSize = 4;
    int auxTapes = 6;
    int elementsNumber = 100;
    char inputFile[30];
    char outputFile[30];

    strcpy(inputFile, "unsorted.txt");
    strcpy(outputFile, "sorted.txt");

    createRandonTape(",", elementsNumber, inputFile);

    while (option) {

        printMenu();
        scanf("%d", &option);

        switch (option) {
            case 1:
                printHeader("General Settings");
                printf("\nMemory Size: %d", memSize);
                printf("\nAux Tapes: %d", auxTapes);
                printf("\nInput File: %s", inputFile);
                printf("\nOutput File: %s", outputFile);
                printf("\nNumber of Elements: %d", elementsNumber);
                printf("\nSorting Algorithm: QuickSort");

                waitForChar("\n\nPress any key to continue...");
                break;
            case 2:
                printHeader("Change Memory Size");
                printf("\nDefault: %d\n", memSize);
                printf("\nNew Memory Size: ");
                scanf("%d", &memSize);
                printf("\nMemory set to: %d", memSize);
                waitForChar("\n\nPress any key to continue...");
                break;
            case 3:
                printHeader("Change Input File");
                printf("\nDefault: %s\n", inputFile);
                printf("\nNew Input File: ");
                scanf("%s", inputFile);
                printf("\nInput File set to: %s", inputFile);
                createRandonTape(",", elementsNumber, inputFile);
                printf("\n\n%s Created Sucessfully with %d elements", inputFile, elementsNumber);
                waitForChar("\n\nPress any key to continue...");
                break;
            case 4:
                printHeader("Change Output File");
                printf("\nDefault: %s\n", outputFile);
                printf("\nNew Output File: ");
                scanf("%s", outputFile);
                printf("\nOutput File set to: %s", outputFile);
                waitForChar("\n\nPress any key to continue...");
                break;
            case 5:
                printHeader("Change Elements Number");
                printf("\nDefault: %d\n", elementsNumber);
                printf("\nNew Quantity: ");
                scanf("%d", &elementsNumber);
                printf("\nElements Number set to: %d", elementsNumber);
                createRandonTape(",", elementsNumber, inputFile);
                printf("\n\n%s Created Sucessfully with %d elements", inputFile, elementsNumber);
                waitForChar("\n\nPress any key to continue...");
                break;
            case 6:
                printHeader("Sorting File");
                externalMergeSort(inputFile, outputFile, ",", memSize, auxTapes);
                waitForChar("\n\nPress any key to continue...");
                break;
        }
    }



    return 0;
}