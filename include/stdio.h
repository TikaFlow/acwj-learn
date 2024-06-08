//
// Created by tika on 24-6-3.
//

#ifndef ACWJ_LEARN_STDIO_H
#define ACWJ_LEARN_STDIO_H

#include <stddef.h>

// This FILE definition will do for now
typedef char * FILE;

FILE *fopen(char *pathname, char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
int fclose(FILE *stream);
int printf(char *format);
int fprintf(FILE *stream, char *format);
int fputc(int c, FILE *stream);
int fputs(char *s, FILE *stream);
int putc(int c, FILE *stream);
int putchar(int c);
int puts(char *s);

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#endif //ACWJ_LEARN_STDIO_H
