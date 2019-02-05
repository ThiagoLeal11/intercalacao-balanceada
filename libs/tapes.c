
#include <stdio.h>
#include "tapes.h"

void openAuxTapesForRead(FILE **tape, TapeRange range) {
    char name[20];
    int i;

    for (i = range.start; i <= range.end; i++) {
        sprintf(name, "aux%d.txt", i);
        tape[i] = fopen(name, "r");
    }
}


void openAuxTapesForWrite(FILE **tape, TapeRange range) {
    char name[20];
    int i;

    for (i = range.start; i <= range.end; i++) {
        sprintf(name, "aux%d.txt", i);
        tape[i] = fopen(name, "w");
    }
}


void closeAuxTapes(FILE **tapes, TapeRange range) {
    int i;

    for (i = range.start; i <= range.end; i++) {
        fclose(tapes[i]);
    }
}


int readNumberFromAuxTape(FILE *tape) {
    char control;
    int number;

    // Try to read the control char.
    if (fscanf(tape, "%c", &control) == 1) {  // File not ended
        if (control == ';') {
            return -1;
        } else {
            fscanf(tape, "%d", &number);
            return number;
        }

    } else {  // File ended, return -1.
        return -1;
    }
}


void writeOnAuxTape(FILE *tape, int number, bool lastOfBlock) {
    // Write the number on aux tape with an block identifier.
    if (!lastOfBlock) {
        fprintf(tape, ",%d", number);
    } else {
        fprintf(tape, ".%d;", number);
    }
}

void copyForOutputFile(FILE *tape, FILE *output, const char *divider) {
    int n;

    // Read the first number.
    n = readNumberFromAuxTape(tape);

    while (n != -1) {
        // write the number.
        fprintf(output, "%d", n);

        // Get the next number.
        n = readNumberFromAuxTape(tape);

        // Check if need to print the divider
        if (n != -1) {
            fprintf(output,"%s", divider);
        }
    }
}