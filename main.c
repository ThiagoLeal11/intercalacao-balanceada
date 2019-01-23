#define true 1
#define false 0
#define bool int

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "libs/tapes.h"

// Todo Use a better sort algorithm.
// Worsted code ever to sort a vector, but it is quick to write. Hehehehe
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


// Sort the batches in memory and save them on the tapes in interleaved mode.
int firstMergePass(FILE *input, FILE *auxTapes[], TapeRange range, int memorySize, const char *divider) {
    bool isFileEnd = false;
    int i, j, n, size=0;
    int memory[memorySize];
    int tapeSelector = range.start;

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
        bubbleSort(memory, i);

        // Save the numbers from memory on tape.
        for (j = 0; j < i; j++) {
            writeOnAuxTape(auxTapes[tapeSelector], memory[j], (j == i-1)); // Pass if this number is the last on memory.
        }

        // Increment or reset tapeSelector.
        if (tapeSelector < range.end) {
            tapeSelector += 1;
        } else {
            tapeSelector = 0;
        }
    }

    // Close the input.
    fclose(input);

    return size;
}


// Merge each block on a given tape range. Return false if nothing was done.
bool mergeBlock(FILE *auxTapes[], TapeRange read, int writeIdx, int memorySize) {
    int i;

    int memory[memorySize];
    int mIdx = 0;
    int auxNumber;

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

    while (!isBlockEnd(memory, memorySize)) {
        // Get the mim index from read tapes.
        mIdx = getMinIndex(memory, memorySize);

        // Save the mim number.
        auxNumber = memory[mIdx];

        // Get the next number from tape.
        memory[mIdx] = readNumberFromAuxTape(auxTapes[read.start + mIdx]);

        // Write the mim number on specific tape and if memory is empty,
        // tell this is the last number of the block.
        writeOnAuxTape(auxTapes[writeIdx], auxNumber, isBlockEnd(memory, memorySize));
    }

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
    int rangeSize, size, i, iter;
    TapeRange readRange, writeRange;

    if (memorySize <= 1) {
        printf("Impossible to run the sort, insufficient memory");
        return -1;
    }

    // Define the max range size.
    rangeSize = (memorySize < auxTapesQnt - 1) ? memorySize : auxTapesQnt - 1;

    // Open the file for input.
    FILE *input = fopen("teste.txt", "r+");

    // Cant open the file.
    if (!input) {
        printf("Can't open the %s file for input.", inputFilename);
        return -1;
    }

    // init aux tapes.
    FILE *auxTapes[auxTapesQnt];

    // Init tapes range.
    readRange.start = 0;
    readRange.end = rangeSize-1;
    writeRange.start = rangeSize;
    writeRange.end = (writeRange.start + rangeSize < auxTapesQnt) ? writeRange.start + rangeSize - 1 : auxTapesQnt - 1;

    // Open tapes for read.
    openAuxTapesForWrite(auxTapes, readRange);

    // Read all the numbers from file and put every sort memory bloc on each tape.
    size = firstMergePass(input, auxTapes, readRange, memorySize, divider);

    // Open the files for read
    closeAuxTapes(auxTapes, readRange);

    // Calc the number of times to merge.
    iter = (int) ceil(log10((double) size/memorySize)/log10(rangeSize));

    for (i = 0; i < iter; i++) {
        // Merge the blocks on read tapes and put on write tapes.
        mergeTapesBlocks(auxTapes, auxTapesQnt, readRange, writeRange, memorySize);

        // Update the ranges
        readRange = writeRange;
        writeRange = getNewRange(auxTapesQnt, rangeSize, readRange);
    }

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

// Todo, make a user interface.
int main() {
    externalMergeSort("teste.txt", "outro0.txt", ",", 1, 6);

    return 0;
}