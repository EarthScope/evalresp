
#include <stdlib.h>
#include <stdarg.h>

#include "x2r.h"
#include "x2r_log.h"


static char *labels[] = {"ERROR", "WARN", "INFO", "DEBUG"};
static int max_level = sizeof(labels) / sizeof(char*);


// A simple logging library.


/** Create a log, with output at the given level or above to the given stream. */
int x2r_alloc_log(int level, FILE *file, x2r_log **log) {

    int status = X2R_OK;

    if (!(*log = calloc(1, sizeof(**log)))) {status = X2R_ERR_MEMORY; goto exit;}
    (*log)->log = file;
    (*log)->level = level;

    exit:
    return status;
}


/** Release resources associated with the log. */
int x2r_free_log(x2r_log *log, int status) {
    free(log);
    return status;
}


/** Underlying log implementation. */
static void X2R_vfprintf(x2r_log *log, int level, char *template, va_list argp) {
    if (level <= log->level) {
        level--;
        fprintf(log->log, "%5s: ", labels[level > max_level ? max_level : level]);
        vfprintf(log->log, template, argp);
        fprintf(log->log, "\n");
    }
}


/** Log a debug message. */
void x2r_debug(x2r_log *log, char *template, ...) {
    va_list argp;
    va_start(argp, template);
    X2R_vfprintf(log, X2R_DEBUG, template, argp);
    va_end(argp);
}


/** Log an info message. */
void x2r_info(x2r_log *log, char *template, ...) {
    va_list argp;
    va_start(argp, template);
    X2R_vfprintf(log, X2R_INFO, template, argp);
    va_end(argp);
}


/** Log a warn message. */
void x2r_warn(x2r_log *log, char *template, ...) {
    va_list argp;
    va_start(argp, template);
    X2R_vfprintf(log, X2R_WARN, template, argp);
    va_end(argp);
}


/**
 * Log an error message.
 *
 * The status is returned to simplify got exit code.
 */
int x2r_error(x2r_log *log, int status, char *template, ...) {
    va_list argp;
    va_start(argp, template);
    X2R_vfprintf(log, X2R_ERROR, template, argp);
    va_end(argp);
    return status;
}

