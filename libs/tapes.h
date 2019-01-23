#ifndef INTERCALACAO_BALANCEADA_FILES_H
#define INTERCALACAO_BALANCEADA_FILES_H

#define bool int

typedef struct TapeRange {
    int start;
    int end;
} TapeRange;

void openAuxTapesForRead(FILE **tape, TapeRange range);
void openAuxTapesForWrite(FILE **tape, TapeRange range);
void closeAuxTapes(FILE **tapes, TapeRange range);
int readNumberFromAuxTape(FILE *tape);
void writeOnAuxTape(FILE *tape, int number, bool lastOfBlock);
void copyForOutputFile(FILE *tape, FILE *output, const char *divider);

#endif //INTERCALACAO_BALANCEADA_FILES_H
