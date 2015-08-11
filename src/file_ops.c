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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#ifndef _WIN32               /* if not Windows compiler then */
#include <sys/param.h>       /* include header files */
#include <unistd.h>
#include <glob.h>
#include <errno.h>
#include <sys/time.h>
#else                        /* if Windows compiler then */
#include <time.h>            /* 'time.h' is not in 'sys/' */
#endif
#include <string.h>
#include "evresp.h"

#ifdef _WIN32
#include <io.h>         /* include header files for directory functions */
#include <direct.h>          /* define macro used below: */
#define S_ISDIR(m) ((m) & S_IFDIR)
#endif

/* find_files:

   creates a linked list of files to search based on the filename and
   scn_lst input arguments, i.e. based on the filename (if it is non-NULL)
   and the list of stations and channels.

   If the filename exists as a directory, then that directory is searched
   for a list of files that look like 'RESP.NETCODE.STA.CHA'.  The names of
   any matching files will be placed in the linked list of file names to
   search for matching response information.  If no match is found for a
   requested 'RESP.NETCODE.STA.CHA' file in that directory, then the search
   for a matching file will stop (see discussion of SEEDRESP case below).

   If the filename is a file in the current directory, then a (matched_files *)NULL
   will be returned

   if the filename is NULL the current directory and the directory indicated
   by the environment variable 'SEEDRESP' will be searched for files of
   the form 'RESP.NETCODE.STA.CHA'.  Files in the current directory will
   override matching files in the directory pointed to by 'SEEDRESP'.  The
   routine will behave exactly as if the filenames contained in these two
   directories had been specified

   the mode is set to zero if the user named a specific filename and
   to one if the user named a directory containing RESP files (or if the
   SEEDRESP environment variable was used to find RESP files

   if a pattern cannot be found, then a value of NULL is set for the
   'names' pointer of the linked list element representing that station-
   channel-network pattern.

   */

struct matched_files *find_files(char *file, struct scn_list *scn_lst,
        int *mode) {
    char *basedir, testdir[MAXLINELEN];
    char comp_name[MAXLINELEN], new_name[MAXLINELEN];
    int i, nscn, nfiles, loc_wild;
    struct matched_files *flst_head, *flst_ptr, *tmp_ptr;
    struct scn *scn_ptr;
    struct stat buf;

    /* first determine the number of station-channel-networks to look at */

    nscn = scn_lst->nscn;

    /* allocate space for the first element of the file pointer linked list */

    flst_head = alloc_matched_files();

    /* and set an 'iterator' variable to be moved through the linked list */

    flst_ptr = flst_head;

    /* set the value of the mode to 1 (indicating that a filename was
     not specified or a directory was specified) */

    *mode = 1;

    /* if a filename was given, check to see if is a directory name, if not
     treat it as a filename */

    if (file != NULL && strlen(file) != 0) {
        stat(file, &buf);
        if (S_ISDIR(buf.st_mode)) {
            for (i = 0; i < nscn; i++) {
                scn_ptr = scn_lst->scn_vec[i];
                loc_wild = !strcmp(scn_ptr->locid, "*") || !strcmp(scn_ptr->locid, "??");
                memset(comp_name, 0, MAXLINELEN);
                sprintf(comp_name, "%s/RESP.%s.%s.%s.%s", file,
                        scn_ptr->network, scn_ptr->station,
						loc_wild ? "*" : scn_ptr->locid,
                        scn_ptr->channel);
                nfiles = get_names(comp_name, flst_ptr);
                if (!nfiles && !loc_wild) {
                    fprintf(stderr, "WARNING: evresp_; no files match '%s'\n",
                            comp_name);
                    fflush(stderr);
                } else if (!nfiles && loc_wild) {
                    memset(comp_name, 0, MAXLINELEN);
                    sprintf(comp_name, "%s/RESP.%s.%s.%s", file,
                            scn_ptr->network, scn_ptr->station,
                            scn_ptr->channel);
                    nfiles = get_names(comp_name, flst_ptr);
                    if (!nfiles) {
                        fprintf(stderr,
                                "WARNING: evresp_; no files match '%s' (or globbed location)\n",
                                comp_name);
                        fflush(stderr);
                    }
                }
                tmp_ptr = alloc_matched_files();
                flst_ptr->ptr_next = tmp_ptr;
                flst_ptr = tmp_ptr;
            }
        } else
            /* file was specified and is not a directory, treat as filename */
            *mode = 0;
    } else {
        for (i = 0; i < nscn; i++) { /* for each station-channel-net in list */
            scn_ptr = scn_lst->scn_vec[i];
            memset(comp_name, 0, MAXLINELEN);
            sprintf(comp_name, "./RESP.%s.%s.%s.%s", scn_ptr->network,
                    scn_ptr->station, scn_ptr->locid, scn_ptr->channel);
            if ((basedir = (char *) getenv("SEEDRESP")) != NULL) {
                /* if the current directory is not the same as the SEEDRESP
                 directory (and the SEEDRESP directory exists) add it to the
                 search path */
                stat(basedir, &buf);
                (void) getcwd(testdir, MAXLINELEN);
                if (S_ISDIR(buf.st_mode) && strcmp(testdir, basedir)) {
                    memset(new_name, 0, MAXLINELEN);
                    sprintf(new_name, " %s/RESP.%s.%s.%s.%s", basedir,
                            scn_ptr->network, scn_ptr->station, scn_ptr->locid,
                            scn_ptr->channel);
                    strcat(comp_name, new_name);
                }
            }
            nfiles = get_names(comp_name, flst_ptr);
            if (!nfiles && strcmp(scn_ptr->locid, "*")) {
                fprintf(stderr, "WARNING: evresp_; no files match '%s'\n",
                        comp_name);
                fflush(stderr);
            } else if (!nfiles && !strcmp(scn_ptr->locid, "*")) {
                memset(comp_name, 0, MAXLINELEN);
                sprintf(comp_name, "./RESP.%s.%s.%s", scn_ptr->network,
                        scn_ptr->station, scn_ptr->channel);
                if (basedir != NULL) {
                    stat(basedir, &buf);
                    (void) getcwd(testdir, MAXLINELEN);
                    if (S_ISDIR(buf.st_mode) && strcmp(testdir, basedir)) {
                        memset(new_name, 0, MAXLINELEN);
                        sprintf(new_name, " %s/RESP.%s.%s.%s", basedir,
                                scn_ptr->network, scn_ptr->station,
                                scn_ptr->channel);
                        strcat(comp_name, new_name);
                    }
                }
                nfiles = get_names(comp_name, flst_ptr);
                if (!nfiles) {
                    fprintf(stderr, "WARNING: evresp_; no files match '%s'\n",
                            comp_name);
                    fflush(stderr);
                }
            }
            tmp_ptr = alloc_matched_files();
            flst_ptr->ptr_next = tmp_ptr;
            flst_ptr = tmp_ptr;
        }
    }

    /* return the pointer to the head of the linked list, which is null
     if no files were found that match request */

    return (flst_head);

}

#ifndef _WIN32      /* if not Windows then use original 'get_names()' */

/* get_names:  uses system glob() to get filenames matching the
 expression in 'in_file'. */

int get_names(char *in_file, struct matched_files *files) {
    struct file_list *lst_ptr, *tmp_ptr;
    glob_t globs;
    int count;
    int rv;
    char *first_infile = NULL;
    char *second_infile = NULL;

    /* IGD 05/30/2013: in_file can contain one token (pathname with possible widlcards)
     * or two tokens pathname + default pathname pointed by SEEDRESP environmental variable.
     * When ET modified this function SEEDRESP case was forgoten. I am restoring it adding strtok
     * function
     */

    first_infile = strtok(in_file, " "); /*IGD 05/30/2013 Assumed to be always present */

    /* Search for matching file names */
    if ((rv = glob(first_infile, 0, NULL, &globs))) {
        second_infile = strtok(NULL, " ");
        if (!second_infile) {
            if (GLOB_NOMATCH != rv)
                perror("glob");
            return 0;
        }
        if ((rv = glob(second_infile, 0, NULL, &globs))) {
            if (GLOB_NOMATCH != rv)
                perror("glob");
            return 0;
        }

    }

    /* set the head of the 'files' linked list to a pointer to a newly allocated
     'matched_files' structure */

    files->first_list = alloc_file_list();
    tmp_ptr = lst_ptr = files->first_list;

    /* retrieve the files from the glob list and build up a linked
     list of matching files */

    count = globs.gl_pathc;
    while (count) {
        count--;
        files->nfiles++;
        lst_ptr->name = alloc_char(strlen(globs.gl_pathv[count]) + 1);
        strcpy(lst_ptr->name, globs.gl_pathv[count]);
        lst_ptr->next_file = alloc_file_list();
        tmp_ptr = lst_ptr;
        lst_ptr = lst_ptr->next_file;
    }

    /* allocated one too many files in the linked list */

    if (lst_ptr != (struct file_list *) NULL) {
        free_file_list(lst_ptr);
        free(lst_ptr);
        if (tmp_ptr != lst_ptr)
            tmp_ptr->next_file = (struct file_list *) NULL;
    }

    globfree(&globs);

    return (files->nfiles);
}

#else              /* if Windows compiler then use new 'get_names()' */

/* get_names:  uses 'findfirst()' and 'findnext()' to get filenames
 matching the expression in 'in_file'. */

int get_names(char *in_file, struct matched_files *files)
{
    struct file_list *lst_ptr, *tmp_ptr;
    struct _finddata_t fblk; /* define block for 'findfirst()' fn */
    /* setup things for Microsoft compiler compatibility: */
    int fhandval;
#define ff_name name
#define findfirst(name,blk,attrib) (fhandval=_findfirst(name,blk))
#define findnext(blk) _findnext(fhandval,blk)
#define findclose() _findclose(fhandval)

    if(findfirst(in_file,&fblk,0) < 0)
    { /* no matching files found */
        findclose(); /* release resources for findfirst/findnext */
        return 0;
    }

    files->first_list = alloc_file_list();
    lst_ptr = files->first_list;

    /* retrieve the files and build up a linked
     list of matching files */
    do
    {
        files->nfiles++;
        lst_ptr->name = alloc_char(strlen(fblk.ff_name)+1);
        strcpy(lst_ptr->name,fblk.ff_name);
        lst_ptr->next_file = alloc_file_list();
        tmp_ptr = lst_ptr;
        lst_ptr = lst_ptr->next_file;
    }
    while(findnext(&fblk) >= 0);
    findclose(); /* release resources for findfirst/findnext */

    /* allocated one too many files in the linked list */
    if(lst_ptr != (struct file_list *)NULL) {
        free_file_list(lst_ptr);
        free(lst_ptr);
        if(tmp_ptr != lst_ptr)
        tmp_ptr->next_file = (struct file_list *)NULL;
    }

    return(files->nfiles);
}

#endif

