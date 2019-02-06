#ifndef SGBD_ARVORE_B_MAIS_RENDER_H
#define SGBD_ARVORE_B_MAIS_RENDER_H

void clearScreen();
void cleanBuffer();
void waitForChar(const char *message);
void printHeader(const char *str);
void printSequence(const char *str, int times);
void printDivision(const char *divider, const char *sequence, int size, ...);
void printStudentRow(int id, const char *name, const char *email, int age, char status);
void printStudentBlock(int id, const char *name, const char *email, int age, char status);
void printStudentDivision();
void printStudentRowHeader();
void printMenu();

#endif //SGBD_ARVORE_B_MAIS_RENDER_H
