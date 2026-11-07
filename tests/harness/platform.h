#ifndef ER_PLATFORM_H
#define ER_PLATFORM_H

/*
 * Filesystem and process helpers.  All platform-specific code in the test
 * suite lives in platform.c.
 */

#include <stddef.h>

#define ER_PATH_MAX 1024

int er_mkdir_p (const char *path);
int er_chdir (const char *path);
char *er_getcwd (char *buf, size_t size);

/* set an environment variable; a NULL value unsets it */
int er_setenv (const char *name, const char *value);

int er_is_file (const char *path);
int er_is_dir (const char *path);

/* make path absolute (relative to the current directory) */
int er_abspath (const char *path, char *buf, size_t size);

/* binary copy; returns 0 on success */
int er_copy_file (const char *src, const char *dst);

/* delete everything inside dir (recursively), keeping dir itself */
int er_remove_contents (const char *dir);

/* names of the regular files in dir, sorted; returns the count or -1.
   Release with er_free_names. */
int er_list_files (const char *dir, char ***names);
void er_free_names (char **names, int n);

/* name of an installed program, with any platform suffix */
void er_exe_name (const char *prog, char *buf, size_t size);

/*
 * Run argv[0] with the remaining NULL-terminated arguments, each passed
 * literally (no globbing or expansion).  in/out/err, when not NULL, are
 * files for stdin, stdout and stderr.  Returns the exit status, or -1 if
 * the program could not be run or was killed by a signal.
 */
int er_run (const char *const *argv, const char *in, const char *out, const char *err);

#endif
