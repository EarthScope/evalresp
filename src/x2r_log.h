
#ifndef X2R_LOG_H
#define X2R_LOG_H

#include <stdio.h>

#define X2R_SILENT 0
#define X2R_ERROR 1
#define X2R_WARN 2
#define X2R_INFO 3
#define X2R_DEBUG 4

typedef struct {
    FILE *log;
    int level;
} x2r_log;

int x2r_alloc_log(int level, FILE *file, x2r_log **log);
int x2r_free_log(x2r_log *log, int status);

void x2r_debug(x2r_log *log, char *template, ...);
void x2r_info(x2r_log *log, char *template, ...);
void x2r_warn(x2r_log *log, char *template, ...);
int x2r_error(x2r_log *log, int status, char *template, ...);

#endif
