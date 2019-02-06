#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "render.h"

#define stdoutSize = 80;

// Clean buffer after a text input
void cleanBuffer() {
    int i;
    while ((i = getchar()) != '\n');
}

void waitForChar(const char *message) {
    cleanBuffer();
    printf("%s", message);
    getchar();
}


// Clear screen both windows and unix
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printSequence(const char *str, int times) {
    while ( times > 0 ) {
        printf("%s", str);
        times--;
    }
}

void printDivision(const char *divider, const char *sequence, int size, ...) {
    int i;

    va_list list;
    va_start(list, size);

    for (i = 0; i < size; i ++) {
        printf("%s", divider);
        printSequence(sequence, va_arg(list, int));
    }

    printf("%s\n", divider);
}

// Todo better header.
void printHeader(const char *str) {
    clearScreen();
    unsigned size = (unsigned) strlen(str);
    printf("\n");
    printSequence("-", size + 8);
    printf("\n-   %s   -\n", str);
    printSequence("-", size + 8);
    printf("\n");
}

void printMenu() {
    clearScreen();

    printf("\n");
    printHeader("Balanced Interleaving");

    printf("\n\nChoose an option entering the corresponding number:\n\n");
    printf("[1] - Show Running Settings\n");
    printf("[2] - Change Memory Size\n");
    printf("[3] - Change Input File\n");
    printf("[4] - Change Output File\n");
    printf("[5] - Change Elements Number\n");
    printf("[6] - Start Sorting\n");
    printf("[0] - Exit\n\n");

    printf("Option: ");
}