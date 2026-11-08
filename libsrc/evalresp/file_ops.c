#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* file_ops.c

    8/28/2001 -- [ET]  Added 'WIN32' directives for Windows compiler
                       compatibility; added code to use 'findfirst()' and
                       'findnext()' (instead of 'ls') when using a Windows
                       compiler.
   10/21/2005 -- [ET]  Modified 'get_names()' function to work with
                       Microsoft compiler (under Windows).
    3/28/2006 -- [ET]  Fixed description of "mode" in header comment for
                       'find_files()' function (changed second "zero" to
                       "one").
    2/25/2010 -- [ET]  Convert 'get_names()' used under non-Windows to use
                       to use the system glob() to find files instead of
               forking a child to run 'ls' in a sub-process.
    5/30/203 -- [IGD] Modified get_names() a bit more to properly process
                      cases with environmental variable SEEDRESP

 */

#include <sys/stat.h>
#include <sys/types.h>

#include <stdlib.h>
#ifndef _WIN32 /* if not Windows compiler then */
#include <errno.h>
#include <glob.h>
#include <sys/param.h> /* include header files */
#include <sys/time.h>
#include <unistd.h>
#else             /* if Windows compiler then */
#include <time.h> /* 'time.h' is not in 'sys/' */
#endif
#include <evalresp/private.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h> /* define macro used below: */
#include <io.h>     /* include header files for directory functions */
#define S_ISDIR(m) ((m) & S_IFDIR)
#endif

#define SEARCH_NAME_LEN 4096

/* Format a search pattern; an empty result (after logging an error) if it does not fit. */
static void
format_name (evalresp_logger *log, char *dest, const char *fmt, ...)
{
  va_list args;
  int n;

  va_start (args, fmt);
  n = vsnprintf (dest, SEARCH_NAME_LEN, fmt, args);
  va_end (args);
  if (n < 0 || n >= SEARCH_NAME_LEN)
  {
    dest[0] = '\0';
    evalresp_log (log, EV_ERROR, EV_ERROR, "file name pattern is longer than %d characters",
                  SEARCH_NAME_LEN - 1);
  }
}

/* Files matching name, or else those matching alt (if not empty). */
static int
get_names_in (char *name, char *alt, struct matched_files *files, evalresp_logger *log)
{
  int n = get_names (name, files, log);

  if (!n && *alt)
    n = get_names (alt, files, log);
  return n;
}

static void
warn_no_files (evalresp_logger *log, const char *name, const char *alt, const char *note)
{
  if (*alt)
    evalresp_log (log, EV_WARN, EV_WARN, "no files match '%s' or '%s'%s", name, alt, note);
  else
    evalresp_log (log, EV_WARN, EV_WARN, "no files match '%s'%s", name, note);
}

// TODO - change mode to enum
struct matched_files *
find_files (char *file, evalresp_sncls *scn_lst,
            int *mode, evalresp_logger *log)
{
  char *basedir, testdir[SEARCH_NAME_LEN];
  char comp_name[SEARCH_NAME_LEN], new_name[SEARCH_NAME_LEN];
  int i, nscn, nfiles, loc_wild;
  struct matched_files *flst_head, *flst_ptr, *tmp_ptr;
  evalresp_sncl *scn_ptr;
  struct stat buf;

  /* first determine the number of station-channel-networks to look at */

  nscn = scn_lst->nscn;

  /* allocate space for the first element of the file pointer linked list */

  flst_head = alloc_matched_files (log);

  /* and set an 'iterator' variable to be moved through the linked list */

  flst_ptr = flst_head;

  /* set the value of the mode to 1 (indicating that a filename was
     not specified or a directory was specified) */

  *mode = 1;

  /* if a filename was given, check to see if is a directory name, if not
     treat it as a filename */

  if (file != NULL && strlen (file) != 0)
  {
    if (!stat (file, &buf) && S_ISDIR (buf.st_mode))
    {
      for (i = 0; i < nscn; i++)
      {
        scn_ptr = scn_lst->scn_vec[i];
        loc_wild = !strcmp (scn_ptr->locid, "*") || !strcmp (scn_ptr->locid, "??");
        comp_name[0] = '\0';
        format_name (log, comp_name, "%s/RESP.%s.%s.%s.%s", file,
                     scn_ptr->network, scn_ptr->station,
                     loc_wild ? "*" : scn_ptr->locid,
                     scn_ptr->channel);
        nfiles = get_names (comp_name, flst_ptr, log);
        if (!nfiles && !loc_wild)
        {
          warn_no_files (log, comp_name, "", "");
        }
        else if (!nfiles && loc_wild)
        {
          comp_name[0] = '\0';
          format_name (log, comp_name, "%s/RESP.%s.%s.%s", file,
                       scn_ptr->network, scn_ptr->station,
                       scn_ptr->channel);
          nfiles = get_names (comp_name, flst_ptr, log);
          if (!nfiles)
          {
            warn_no_files (log, comp_name, "", " (or globbed location)");
          }
        }
        tmp_ptr = alloc_matched_files (log);
        flst_ptr->ptr_next = tmp_ptr;
        flst_ptr = tmp_ptr;
      }
    }
    else
      /* file was specified and is not a directory, treat as filename */
      *mode = 0;
  }
  else
  {
    for (i = 0; i < nscn; i++)
    { /* for each station-channel-net in list */
      scn_ptr = scn_lst->scn_vec[i];
      comp_name[0] = '\0';
      format_name (log, comp_name, "./RESP.%s.%s.%s.%s", scn_ptr->network,
                   scn_ptr->station, scn_ptr->locid, scn_ptr->channel);
      new_name[0] = '\0';
      if ((basedir = (char *)getenv ("SEEDRESP")) != NULL)
      {
        /* if the current directory is not the same as the SEEDRESP
                 directory (and the SEEDRESP directory exists) add it to the
                 search path */
        if (!getcwd (testdir, SEARCH_NAME_LEN))
          testdir[0] = '\0';
        if (!stat (basedir, &buf) && S_ISDIR (buf.st_mode) && strcmp (testdir, basedir))
        {
          format_name (log, new_name, "%s/RESP.%s.%s.%s.%s", basedir,
                       scn_ptr->network, scn_ptr->station, scn_ptr->locid,
                       scn_ptr->channel);
        }
      }
      nfiles = get_names_in (comp_name, new_name, flst_ptr, log);
      if (!nfiles && strcmp (scn_ptr->locid, "*"))
      {
        warn_no_files (log, comp_name, new_name, "");
      }
      else if (!nfiles && !strcmp (scn_ptr->locid, "*"))
      {
        comp_name[0] = '\0';
        format_name (log, comp_name, "./RESP.%s.%s.%s", scn_ptr->network,
                     scn_ptr->station, scn_ptr->channel);
        new_name[0] = '\0';
        if (basedir != NULL)
        {
          if (!getcwd (testdir, SEARCH_NAME_LEN))
            testdir[0] = '\0';
          if (!stat (basedir, &buf) && S_ISDIR (buf.st_mode) && strcmp (testdir, basedir))
          {
            format_name (log, new_name, "%s/RESP.%s.%s.%s", basedir,
                         scn_ptr->network, scn_ptr->station,
                         scn_ptr->channel);
          }
        }
        nfiles = get_names_in (comp_name, new_name, flst_ptr, log);
        if (!nfiles)
        {
          warn_no_files (log, comp_name, new_name, "");
        }
      }
      tmp_ptr = alloc_matched_files (log);
      flst_ptr->ptr_next = tmp_ptr;
      flst_ptr = tmp_ptr;
    }
  }

  /* return the pointer to the head of the linked list, which is null
     if no files were found that match request */

  return (flst_head);
}

#ifndef _WIN32 /* if not Windows then use original 'get_names()' */

/* get_names:  uses system glob() to get filenames matching the
 expression in 'in_file'. */

int
get_names (char *in_file, struct matched_files *files, evalresp_logger *log)
{
  struct file_list *lst_ptr, *tmp_ptr;
  glob_t globs;
  int count;
  int rv;

  /* Search for matching file names */
  if ((rv = glob (in_file, 0, NULL, &globs)))
  {
    if (GLOB_NOMATCH != rv)
      perror ("glob");
    return 0;
  }

  /* set the head of the 'files' linked list to a pointer to a newly allocated
     'matched_files' structure */

  files->first_list = alloc_file_list (log);
  tmp_ptr = lst_ptr = files->first_list;

  /* retrieve the files from the glob list and build up a linked
     list of matching files */

  count = globs.gl_pathc;
  while (count)
  {
    count--;
    files->nfiles++;
    lst_ptr->name = alloc_char (strlen (globs.gl_pathv[count]) + 1, log);
    strcpy (lst_ptr->name, globs.gl_pathv[count]);
    lst_ptr->next_file = alloc_file_list (log);
    tmp_ptr = lst_ptr;
    lst_ptr = lst_ptr->next_file;
  }

  /* allocated one too many files in the linked list */

  if (lst_ptr != (struct file_list *)NULL)
  {
    free_file_list (lst_ptr);
    free (lst_ptr);
    if (tmp_ptr != lst_ptr)
      tmp_ptr->next_file = (struct file_list *)NULL;
  }

  globfree (&globs);

  return (files->nfiles);
}

#else /* if Windows compiler then use new 'get_names()' */

/* get_names:  uses 'findfirst()' and 'findnext()' to get filenames
 matching the expression in 'in_file'. */

int
get_names (char *in_file, struct matched_files *files, evalresp_logger *log)
{
  struct file_list *lst_ptr, *tmp_ptr;
  struct _finddata_t fblk; /* define block for 'findfirst()' fn */
  /* setup things for Microsoft compiler compatibility: */
  intptr_t fhandval;
  size_t dirlen = 0;
  const char *sep;
#define ff_name name
#define findfirst(name, blk, attrib) (fhandval = _findfirst (name, blk))
#define findnext(blk) _findnext (fhandval, blk)
#define findclose() _findclose (fhandval)

  if (findfirst (in_file, &fblk, 0) < 0)
  {               /* no matching files found */
    findclose (); /* release resources for findfirst/findnext */
    return 0;
  }

  /* matches are qualified with the directory part of the pattern */
  for (sep = in_file; *sep; ++sep)
  {
    if (*sep == '/' || *sep == '\\')
      dirlen = (size_t)(sep - in_file) + 1;
  }

  files->first_list = alloc_file_list (log);
  lst_ptr = files->first_list;

  /* retrieve the files and build up a linked
     list of matching files */
  do
  {
    files->nfiles++;
    lst_ptr->name = alloc_char (dirlen + strlen (fblk.ff_name) + 1, log);
    memcpy (lst_ptr->name, in_file, dirlen);
    strcpy (lst_ptr->name + dirlen, fblk.ff_name);
    lst_ptr->next_file = alloc_file_list (log);
    tmp_ptr = lst_ptr;
    lst_ptr = lst_ptr->next_file;
  } while (findnext (&fblk) >= 0);
  findclose (); /* release resources for findfirst/findnext */

  /* allocated one too many files in the linked list */
  if (lst_ptr != (struct file_list *)NULL)
  {
    free_file_list (lst_ptr);
    free (lst_ptr);
    if (tmp_ptr != lst_ptr)
      tmp_ptr->next_file = (struct file_list *)NULL;
  }

  return (files->nfiles);
}

#endif
