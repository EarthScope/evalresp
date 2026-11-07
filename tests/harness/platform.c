#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#define MKDIR(p) _mkdir (p)
#define RMDIR(p) _rmdir (p)
#define CHDIR(p) _chdir (p)
#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#else
#define MKDIR(p) mkdir ((p), 0777)
#define RMDIR(p) rmdir (p)
#define CHDIR(p) chdir (p)
#endif

int
er_chdir (const char *path)
{
  return CHDIR (path);
}

char *
er_getcwd (char *buf, size_t size)
{
#ifdef _WIN32
  return _getcwd (buf, (int)size);
#else
  return getcwd (buf, size);
#endif
}

int
er_is_file (const char *path)
{
  struct stat st;
  return stat (path, &st) == 0 && S_ISREG (st.st_mode);
}

int
er_is_dir (const char *path)
{
  struct stat st;
  return stat (path, &st) == 0 && S_ISDIR (st.st_mode);
}

static int
is_absolute (const char *path)
{
  if (path[0] == '/')
    return 1;
#ifdef _WIN32
  if (path[0] == '\\' || (path[0] && path[1] == ':'))
    return 1;
#endif
  return 0;
}

int
er_abspath (const char *path, char *buf, size_t size)
{
  if (is_absolute (path))
  {
    if (strlen (path) >= size)
      return -1;
    strcpy (buf, path);
    return 0;
  }
  if (!er_getcwd (buf, size) || strlen (buf) + strlen (path) + 2 > size)
    return -1;
  strcat (buf, "/");
  strcat (buf, path);
  return 0;
}

int
er_mkdir_p (const char *path)
{
  char buf[ER_PATH_MAX];
  size_t i, n = strlen (path);
  if (n == 0 || n >= sizeof (buf))
    return -1;
  strcpy (buf, path);
  for (i = 1; i <= n; ++i)
  {
    if (buf[i] == '/' || buf[i] == '\\' || buf[i] == '\0')
    {
      char c = buf[i];
      buf[i] = '\0';
      /* skip a drive prefix such as "C:" */
      if (!(i == 2 && buf[1] == ':') && !er_is_dir (buf) && MKDIR (buf) != 0 && !er_is_dir (buf))
        return -1;
      buf[i] = c;
    }
  }
  return 0;
}

int
er_copy_file (const char *src, const char *dst)
{
  char buf[65536];
  size_t n;
  int rc = 0;
  FILE *in = fopen (src, "rb"), *out = NULL;
  if (!in)
    return -1;
  if (!(out = fopen (dst, "wb")))
  {
    fclose (in);
    return -1;
  }
  while ((n = fread (buf, 1, sizeof (buf), in)) > 0)
  {
    if (fwrite (buf, 1, n, out) != n)
    {
      rc = -1;
      break;
    }
  }
  if (ferror (in))
    rc = -1;
  fclose (in);
  if (fclose (out) != 0)
    rc = -1;
  return rc;
}

static int
name_cmp (const void *a, const void *b)
{
  return strcmp (*(char *const *)a, *(char *const *)b);
}

static int
add_name (char ***names, int *n, int *cap, const char *name)
{
  if (*n == *cap)
  {
    char **tmp = realloc (*names, (*cap ? 2 * *cap : 32) * sizeof (**names));
    if (!tmp)
      return -1;
    *names = tmp;
    *cap = *cap ? 2 * *cap : 32;
  }
  if (!((*names)[*n] = malloc (strlen (name) + 1)))
    return -1;
  strcpy ((*names)[(*n)++], name);
  return 0;
}

void
er_free_names (char **names, int n)
{
  int i;
  for (i = 0; i < n; ++i)
    free (names[i]);
  free (names);
}

/* all entries of dir except "." and ".."; sorted */
static int
list_entries (const char *dir, char ***names)
{
  int n = 0, cap = 0;
  *names = NULL;
#ifdef _WIN32
  {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char pattern[ER_PATH_MAX];
    if (strlen (dir) + 3 > sizeof (pattern))
      return -1;
    sprintf (pattern, "%s\\*", dir);
    if ((h = FindFirstFileA (pattern, &fd)) == INVALID_HANDLE_VALUE)
      return -1;
    do
    {
      if (strcmp (fd.cFileName, ".") && strcmp (fd.cFileName, ".."))
      {
        if (add_name (names, &n, &cap, fd.cFileName))
        {
          FindClose (h);
          er_free_names (*names, n);
          return -1;
        }
      }
    } while (FindNextFileA (h, &fd));
    FindClose (h);
  }
#else
  {
    struct dirent *e;
    DIR *d = opendir (dir);
    if (!d)
      return -1;
    while ((e = readdir (d)))
    {
      if (strcmp (e->d_name, ".") && strcmp (e->d_name, ".."))
      {
        if (add_name (names, &n, &cap, e->d_name))
        {
          closedir (d);
          er_free_names (*names, n);
          return -1;
        }
      }
    }
    closedir (d);
  }
#endif
  if (n > 1)
    qsort (*names, n, sizeof (**names), name_cmp);
  return n;
}

int
er_list_files (const char *dir, char ***names)
{
  char **all;
  int i, n, m = 0;
  if ((n = list_entries (dir, &all)) < 0)
    return -1;
  for (i = 0; i < n; ++i)
  {
    char path[ER_PATH_MAX];
    if ((size_t)snprintf (path, sizeof (path), "%s/%s", dir, all[i]) < sizeof (path))
    {
      if (er_is_file (path))
        all[m++] = all[i];
      else
        free (all[i]);
    }
    else
      free (all[i]);
  }
  *names = all;
  return m;
}

/* a directory itself, not a symbolic link to one */
static int
is_real_dir (const char *path)
{
#ifdef _WIN32
  return er_is_dir (path);
#else
  struct stat st;
  return lstat (path, &st) == 0 && S_ISDIR (st.st_mode);
#endif
}

int
er_remove_contents (const char *dir)
{
  char **all;
  int i, n, rc = 0;
  if ((n = list_entries (dir, &all)) < 0)
    return -1;
  for (i = 0; i < n; ++i)
  {
    char path[ER_PATH_MAX];
    if ((size_t)snprintf (path, sizeof (path), "%s/%s", dir, all[i]) >= sizeof (path))
    {
      rc = -1;
      continue;
    }
    if (is_real_dir (path))
    {
      if (er_remove_contents (path) || RMDIR (path))
        rc = -1;
    }
    else if (remove (path))
      rc = -1;
  }
  er_free_names (all, n);
  return rc;
}

void
er_exe_name (const char *prog, char *buf, size_t size)
{
#ifdef _WIN32
  _snprintf (buf, size, "%s.exe", prog);
#else
  snprintf (buf, size, "%s", prog);
#endif
  buf[size - 1] = '\0';
}

/* growable string */
typedef struct
{
  char *s;
  size_t len, cap;
} strbuf;

static int
sb_add (strbuf *b, const char *s, size_t n)
{
  if (b->len + n + 1 > b->cap)
  {
    size_t cap = (b->len + n + 1) * 2;
    char *tmp = realloc (b->s, cap);
    if (!tmp)
      return -1;
    b->s = tmp;
    b->cap = cap;
  }
  memcpy (b->s + b->len, s, n);
  b->len += n;
  b->s[b->len] = '\0';
  return 0;
}

static int
sb_str (strbuf *b, const char *s)
{
  return sb_add (b, s, strlen (s));
}

/* append s so that the shell sees it as a single literal word */
static int
sb_quoted (strbuf *b, const char *s, int is_path)
{
  size_t i;
  int rc = 0;
#ifdef _WIN32
  rc |= sb_str (b, "\"");
  for (i = 0; s[i]; ++i)
  {
    char c = (is_path && s[i] == '/') ? '\\' : s[i];
    rc |= sb_add (b, &c, 1);
  }
  rc |= sb_str (b, "\"");
#else
  (void)is_path;
  rc |= sb_str (b, "'");
  for (i = 0; s[i]; ++i)
  {
    if (s[i] == '\'')
      rc |= sb_str (b, "'\\''");
    else
      rc |= sb_add (b, s + i, 1);
  }
  rc |= sb_str (b, "'");
#endif
  return rc;
}

int
er_run (const char *const *argv, const char *in, const char *out, const char *err)
{
  strbuf b = {NULL, 0, 0};
  int i, rc = 0, status;

#ifdef _WIN32
  /* cmd.exe strips one pair of enclosing quotes */
  rc |= sb_str (&b, "\"");
#endif
  for (i = 0; argv[i]; ++i)
  {
    if (i)
      rc |= sb_str (&b, " ");
    rc |= sb_quoted (&b, argv[i], i == 0);
  }
  if (in)
  {
    rc |= sb_str (&b, " < ");
    rc |= sb_quoted (&b, in, 1);
  }
  if (out)
  {
    rc |= sb_str (&b, " > ");
    rc |= sb_quoted (&b, out, 1);
  }
  if (err)
  {
    rc |= sb_str (&b, " 2> ");
    rc |= sb_quoted (&b, err, 1);
  }
#ifdef _WIN32
  rc |= sb_str (&b, "\"");
#endif
  if (rc)
  {
    free (b.s);
    return -1;
  }

  fflush (NULL);
  status = system (b.s);
  free (b.s);
#ifdef _WIN32
  return status;
#else
  if (status == -1 || !WIFEXITED (status))
    return -1;
  return WEXITSTATUS (status);
#endif
}

int
er_setenv (const char *name, const char *value)
{
#ifdef _WIN32
  return _putenv_s (name, value ? value : "");
#else
  return value ? setenv (name, value, 1) : unsetenv (name);
#endif
}
