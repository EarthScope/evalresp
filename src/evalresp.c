/* evalresp.c:  Main module for 'evalresp'. */

/*
 10/19/2005 -- [ET]  Added parameters for List-blockette interpolation;
 added warnings for unrecognized parameters.
 11/3/2005 -- [ET]  Moved 'use_delay()' function from 'calc_fctns.c'
 to 'evalresp.c'.
 2/6/2006 -- [ET]  Moved 'use_delay()' function from 'evalresp.c'
 to 'evresp.c'.
 3/28/2006 -- [ET]  Added "free(freqs)" to end of 'main()' function.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include "evresp.h"
#include <getopt.h>
#include <log.h>

#define DATIMESIZE 32
#define TODAYSIZE 10

void usage (char *prgName)
{
    if (prgName == NULL)
    {
        prgName="evalresp";
    }
    printf("\nEVALRESP V%s\n", REVNUM);
    printf("\nUSAGE: %s STALST CHALST YYYY DAY MINFREQ", prgName);
    printf(" MAXFREQ NFREQ [options]\n\n");
    printf("  OPTIONS:\n\n");
    printf("    -f file              (directory-name|filename)\n");
    printf("    -u units             ('dis'|'vel'|'acc'|'def')\n");
    printf("    -t time-of-day       (HH:MM:SS)\n");
    printf("    -s type-of-spacing   (log|lin)\n");
    printf("    -n netid             ('II'|'IU'|'G'|'*'...)\n");
    printf("    -l locid             ('01'|'AA,AB,AC'|'A?'|'*'...)\n");
    printf("    -r resp_type         ('ap'=amp/pha | 'cs'=complex spectra |\n");
    printf("                          'fap'=freq/amp/pha)\n");
    printf("    -stage start [stop]  (start and stop are integer stage numbers)\n");
    printf("    -stdio               (take input from stdin, output to stdout)\n");
    printf("    -use-estimated-delay (use estimated delay instead of correction applied\n");
    printf("                          in computation of ASYM FIR response)\n");
    printf("    -il                  (interpolate List blockette output)\n");
    printf("    -ii                  (interpolate List blockette input)\n");
    printf("    -it tension          (tension for List blockette interpolation)\n");
    printf("    -unwrap              (unwrap phase if the output is AP) \n");
    printf("    -ts                  (use total sensitivity from stage 0 instead of\n");
    printf("                          computed)\n");
    printf("    -b62_x value         (sample value/volts where we compute response for\n");
    printf("                          B62)\n");
    printf("    -v                   (verbose; list parameters on stdout)\n");
    printf("    -x                   (xml; expect station.xml format)\n\n");
    printf("  NOTES:\n\n");
    printf("    (1) If the 'file' argument is a directory, that directory will be\n");
    printf("        searched for files of the form RESP.NETID.STA.CHA.  Files\n");
    printf("        of this type are created by rdseed when it is run with the\n");
    printf("        '-R' option or when the '-d' option is used and responses are\n");
    printf("        requested, but station.xml format files can also be read (if\n");
    printf("        the '-x' flag is given).\n");
    printf("    (2) If the 'file' argument is a file, that file is assumed to be\n");
    printf("        output from a call to rdseed with the '-R' option, or a file\n");
    printf("        in the station.xml format if '-x' is also given.\n");
    printf("    (3) If the 'file' argument is missing, the current directory\n");
    printf("        will be searched for files of the form RESP.NETID.STA.CHA.\n");
    printf("    (4) The directory indicated by the environment variable SEEDRESP\n");
    printf("        will also be searched for the requested files (if it is defined),\n");
    printf("        but if matching station-channel-network values are found in\n");
    printf("        both directories, then the local files take precedence.\n");
    printf("    (5) The NETID (above) indicates a one or two letter network code.\n");
    printf("        If no network code exists in the SEED volume, a value of '',\n");
    printf("         '\?\?', or '*' will match the 'no network' case.\n");
    printf("    (6) The '-stage' option allows the user to specify the range of\n");
    printf("        stages that will be included in the calculation.  If only one\n");
    printf("        stage number is given, a response will only be calculated for\n");
    printf("        that stage.  If both a start and stop stage number are given,\n");
    printf("        any stage between (and including) the start and stop stages\n");
    printf("        will be included in the calculation.\n");
    printf("    (7) -b62_x defines a value in counts or volts where response is\n");
    printf("        computed. This flag only is applied to responses with B62.\n\n");
    printf("  EXAMPLES:\n\n");
    printf("    evalresp AAK,ARU,TLY VHZ 1992 21 0.001 10 100 -f /EVRESP/NEW/rdseed.out\n");
    printf("    evalresp KONO BHN,BHE 1992 1 0.001 10 100 -f /EVRESP/NEW -t 12:31:04 -v\n");
    printf("    evalresp FRB BHE,BHZ 1994 31 0.001 10 100 -f resp.all_stations -n '*' -v\n\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

    char *sta_list, *cha_list, units[MAXFLDLEN], *file, *verbose;
    char *net_code, *locid, *type, rtype[MAXFLDLEN];
    int lin_typ = 0, nfreqs, i, fswidx, tmp_val, fldlen;
    int start_stage = -1, stop_stage = 0, stdio_flag = 0;
    int listinterp_out_flag = 0, listinterp_in_flag = 0;
    int unwrap_flag = 0, xml_flag = 0;
    double listinterp_tension = 1000.0;
    char *t_o_day;
    char *datime;
    double incr, freq_lims[2], temp_val, *freqs;
    double val;
    struct response *first;
    char *minfstr, *maxfstr, *numfstr;
    char param_err_msgstr[] = "%s: missing option to argument '%s'";
    int useTotalSensitivityFlag = 0; /* IGD 01/29/10 */
    double x_for_b62 = 0; /* IGD 10/03/13 */
    evalresp_log_t *log = NULL; /* set log to NULL for future updates but crurrently print to stderr */
    char *prog_name=argv[0];

    curr_seq_no = -1;

    myLabel[0] = '\0';
    int opt;
    struct option cmdline_flags[] = {
        {"file",                required_argument, 0, 'f'},
        {"units",               required_argument, 0, 'u'},
        {"time",                required_argument, 0, 't'},
        {"spacing",             required_argument, 0, 's'},
        {"network",             required_argument, 0, 'n'},
        {"location",            required_argument, 0, 'l'},
        {"response",            required_argument, 0, 'r'},
        {"stage",               required_argument, 0, 'S'},
        {"stdio",               no_argument,       &stdio_flag, 1},
        {"use-estimated-delay", no_argument,       0, 'U'},
        {"use-delay",           no_argument,       0, 'U'},
        {"il",                  no_argument,       &listinterp_out_flag, 1},
        {"ii",                  no_argument,       &listinterp_in_flag, 1},
        {"it",                  required_argument, 0, 'T'},
        {"unwrap",              no_argument,       &unwrap_flag, 1},
        {"ts",                  no_argument,       &useTotalSensitivityFlag, 1},
        {"b62_x",               required_argument, 0, 'b'},
        {"verbose",             no_argument,       0, 'v'},
        {"xml",                 no_argument,       &xml_flag, 1},
        {0,0,0,0}
    };
    int longindex;
    int flagc;
    char **flagv;
    char errflag[2]={0,0};
    char *err=NULL;

    if (argc < 5)
    {
        usage(prog_name);
    }

    /* find index of first switch parameter */
    fswidx = 0; /* loop until switch param found (if any) */
    while (++fswidx < argc && /*  (check if switch param or real #) */
    (strncmp(argv[fswidx], "-", 1) != 0 || is_real(argv[fswidx], log)))
        ;
    if (fswidx < 5) {
        evalresp_log(log, ERROR, 0, "Not all of the required inputs are \n\t"
                "present (%d missing), type '%s' for usage", 8 - fswidx,
                prog_name);
        exit(USAGE_ERROR);
        /*XXX error_exit(USAGE_ERROR, "Not all of the required inputs are \n\t"
                "present (%d missing), type '%s' for usage", 8 - fswidx,
                prog_name); */
    }

    /* setup min,max,num freq values; use defaults if missing */
    minfstr = (fswidx > 5) ? argv[5] : "1.0";
    maxfstr = (fswidx > 6) ? argv[6] : minfstr;
    numfstr = (fswidx > 7) ? argv[7] : "1";

    /* warn about any unexpected non-switch parameters */
    for (i = 8; i < fswidx; ++i) {
        if (argv[i] != NULL && argv[i][0] != '\0')
        {
            evalresp_log(log, WARN, 0, "Unrecognized parameter:  %s\n", argv[i]);
            /*XXX fprintf(stderr, "WARNING:  Unrecognized parameter:  %s\n", argv[i]); */
        }
    }

    /* then get the optional arguments (if any) */
    /* initialize the optional arguments */
    /* If user did not define -use-estimated-delay option by default it is FALSE */
    use_estimated_delay(FALSE);

    strncpy(units, "", MAXFLDLEN);
    strncpy(rtype, "", MAXFLDLEN);
    net_code = locid = file = verbose = type = (char *) NULL;
    if ((t_o_day = (char *) malloc(strlen("00:00:00") + 1)) == (char *) NULL) {
    }
    sprintf(t_o_day, "00:00:00");

    flagc = argc - fswidx + 1;
    if (flagc > 0)
    {
        flagv = argv + fswidx -1;

        while(-1 != (opt = getopt_long_only(flagc, flagv, ":f:u:t:s:n:l:r:S:UT:b:vx", cmdline_flags, &longindex)))
        {
            switch (opt)
            {
                case 0:/* This is to handle ones that get set automatically */
                    break;
                case 'f': /*file*/
                    file = strndup(optarg, MAXFLDLEN);
                    break;
                case 'u': /*units*/
                    strncpy(units, optarg, MAXFLDLEN);
                    break;
                case 't': /*time*/
                    fldlen = strlen(optarg);
                    if (fldlen >= TODAYSIZE) {
                        t_o_day = realloc(t_o_day, (fldlen + 1));
                    }
                    strcpy(t_o_day, optarg);
                    break;
                case 's': /*space*/
                    if (strcmp(optarg, "lin") & strcmp(optarg, "log")) {
                        evalresp_log(log, ERROR, 0, "%s: option '-s' illegal type '%s'",
                                prog_name, type);
                        exit(USAGE_ERROR);
                        /*XXX error_exit(USAGE_ERROR, "%s: option '-s' illegal type '%s'",
                                prog_name, type); */
                    }
                    type = strndup(optarg, MAXFLDLEN);
                    break;
                case 'n':/*network*/
                    net_code = strndup(optarg, MAXFLDLEN);
                    break;
                case 'l':/*location*/
                    locid = strndup(optarg, MAXFLDLEN);
                    break;
                case 'r':/*response*/
                    strncpy(rtype, optarg, MAXFLDLEN);
                    break;
                case 'S':/*stage*/
                    if (is_int(optarg, log))
                    {
                        start_stage = atoi(optarg);
                    } else {
                        evalresp_log(log, ERROR, 0, param_err_msgstr, prog_name, optarg);
                        exit(USAGE_ERROR);
                        /*XXX error_exit(USAGE_ERROR, param_err_msgstr, prog_name, optarg); */
                    }
                    if (optind < flagc && is_int(flagv[optind], log))
                    {
                        tmp_val = atoi(flagv[optind]);
                        if (tmp_val > start_stage)
                        {
                            stop_stage = tmp_val;
                        } else {
                            stop_stage = start_stage;
                            start_stage = tmp_val;
                        }
                        optind++;
                    }
                    break;
                case 'U':/*use-estimated-delay use-delay*/
                    use_estimated_delay(TRUE);
                    break;
                case 'T':/*it*/
                    if (is_real(optarg, log))
                    {
                        listinterp_tension = atof(optarg);
                    } else {
                        evalresp_log(log, ERROR, 0,
                                "%s:  illegal value for 'it' parameter:  %s",
                                prog_name, optarg);
                        exit(USAGE_ERROR);
                        /*XXX error_exit(USAGE_ERROR,
                                "%s:  illegal value for 'it' parameter:  %s",
                                prog_name, optarg); */
                    }
                    break;
                case 'b':/*b62_x*/
                    x_for_b62 = atof(optarg);
                    break;
                case 'v':/*verbose*/
                    verbose = "-v";
                    break;
                case 'x':
                    xml_flag = 1;
                    break;
                case ':':/* invalid argument for flag */
                    if (0 != cmdline_flags[longindex].name)
                    {
                        err = (char *)cmdline_flags[longindex].name;
                    } else {
                        errflag[0] = optopt;
                        err = errflag;
                    }
                    evalresp_log(log, ERROR, 0, param_err_msgstr, prog_name, err);
                    exit(USAGE_ERROR);
                    /*XXX error_exit(USAGE_ERROR, param_err_msgstr, prog_name, err); */
                    break;
                case '?': /* unrecognized flag */
                    evalresp_log(log, WARN, 0, "Unrecognized parameter:  %s\n", flagv[optind-1]);
                    /*XXX fprintf(stderr, "WARNING:  Unrecognized parameter:  %s\n", flagv[optind-1]); */
                    break;
            }
        }
    }

    /* Get arguments */
    sta_list = argv[1];
    cha_list = argv[2];

    if (!is_int(argv[3], log))
    {
        evalresp_log(log, ERROR, 0, "Year must be an integer, found '%s'", argv[3]);
        exit(USAGE_ERROR);
        //error_exit(USAGE_ERROR, "year must be an integer, found '%s'", argv[3]);
    }
    if (!is_int(argv[4], log))
    {
        evalresp_log(log, ERROR, 0, "Julian day must be an integer, found '%s'", argv[4]);
        exit(USAGE_ERROR);
        //error_exit(USAGE_ERROR, "julian day must be an integer, found '%s'",
        //        argv[4]);
    }
    if (!is_time(t_o_day, log))
    {
        evalresp_log(log, ERROR, 0, "'Time of day' must have format "
                                     "'hh[:mm[:ss[.#####]]]', found '%s'",
                                   t_o_day);
        exit(USAGE_ERROR);
        /*XXX error_exit(USAGE_ERROR,
                "'time of day' must have format 'hh[:mm[:ss[.#####]]]', found '%s'",
                t_o_day); */
    }

    fldlen = strlen(argv[3]) + strlen(argv[4]) + strlen(t_o_day) + 3;
    if ((datime = (char *) malloc(fldlen)) == (char *) NULL) {
    }
    sprintf(datime, "%s,%s,%s", argv[3], argv[4], t_o_day);

    if (!is_real(minfstr, log) || !is_real(maxfstr, log))
    {
        evalresp_log(log, ERROR, 0, "freq_lims must be real numbers, found (%s,%s)",
                minfstr, maxfstr);
        exit(USAGE_ERROR);
        /*XXX error_exit(USAGE_ERROR, "freq_lims must be real numbers, found (%s,%s)",
                minfstr, maxfstr); */
    }
    freq_lims[0] = atof(minfstr);
    freq_lims[1] = atof(maxfstr);
    if ((freq_lims[0] == 0.0 || freq_lims[1] == 0.0)
            && (type == NULL || strcmp(type, "lin") != 0)) {
        evalresp_log(log, ERROR, 0,
                " freq lims can't equal 0 if log spacing is used");
        exit(USAGE_ERROR);
      /*XXX error_exit(USAGE_ERROR,
                " freq lims can't equal 0 if log spacing is used");*/
    }
    if (!is_int(numfstr, log))
    {
        evalresp_log(log, ERROR, 0, "nfreqs must be an integer, found '%s'",
                numfstr);
        exit(USAGE_ERROR);
        /*XXX error_exit(USAGE_ERROR, "nfreqs must be an integer, found '%s'",
                numfstr); */
    }
    nfreqs = atoi(numfstr);

    /* check the frequency input values and range */

    if (freq_lims[1] < freq_lims[0]) {
        temp_val = freq_lims[1];
        freq_lims[1] = freq_lims[0];
        freq_lims[0] = temp_val;
    } else if (nfreqs <= 0) {
        evalresp_log(log, ERROR, 0,
                "nfreqs value entered must be positive, entered %d", nfreqs);
        exit(USAGE_ERROR);
        /*XXX error_exit(USAGE_ERROR,
                "nfreqs value entered must be positive, entered %d", nfreqs); */
    }

    /* depending on the type of spacing, set either a step size or a multiplication
     factor for the loop over frequencies */

    if (type != NULL && !strcmp(type, "lin")) {
        lin_typ = 1;
        if (nfreqs == 1)
        {
            incr = -1.0;
        } else {
            incr = (freq_lims[1] - freq_lims[0]) / (nfreqs - 1);
        }
    } else if (nfreqs == 1) {
        incr = -1.0;
    } else {
        incr = pow((freq_lims[1] / freq_lims[0]), (1. / (nfreqs - 1)));
    }

    /* if net_code not given, default to "*" value */

    if (net_code == (char *) NULL)
    {
        net_code = "*";
    }

    /* if locid not given, default to "*" value */

    if (locid == (char *) NULL)
    {
        locid = "*";
    }

    /* convert the units and response type arguments to lower case after
     checking to make sure they match one of the appropriate input values */

    if (strlen(rtype)) {
        for (i = 0; i < (int) strlen(rtype); i++)
        {
            *(rtype + i) = toupper(*(rtype + i));
        }
        if (strcmp(rtype, "CS") == 0) /* if complex-spectra output then */
        {
            listinterp_out_flag = 0; /* force List-out-interp flag clear */
        } else if ((strcmp(rtype, "AP") != 0)) { /* if invalid value then abort */
            if ((strcmp(rtype, "FAP") != 0))
            {
                evalresp_log(log, ERROR, 0,
                        "evalresp; rtype entered ('%s') not a recognized string (see usage)",
                        rtype);
                exit(USAGE_ERROR);
                /*XXX error_exit(USAGE_ERROR,
                        "evalresp; rtype entered ('%s') not a recognized string (see usage)",
                        rtype);*/
            }
        }
    } else {
        strncpy(rtype, "AP", MAXFLDLEN);
    }

    if (strlen(units)) {
        for (i = 0; i < (int) strlen(units); i++)
        {
            *(units + i) = toupper(*(units + i));
        }
        if (strcmp(units, "DIS") && strcmp(units, "VEL") && strcmp(units, "ACC")
                && strcmp(units, "DEF"))
        {
            evalresp_log(log, ERROR, 0,
                    "evalresp; units entered ('%s') not a recognized string (see usage)",
                    units);
            exit(USAGE_ERROR);
            /*XXX error_exit(USAGE_ERROR,
                    "evalresp; units entered ('%s') not a recognized string (see usage)",
                    units); */
        } else if (!strcmp(units, "DEF")) {
            def_units_flag = 1;
        } else {
            def_units_flag = 0;
        }
    } else {
        strncpy(units, "VEL", MAXFLDLEN);
    }

    /* allocate space for the frequencies and fill with appropriate values */

    freqs = alloc_double(nfreqs);
    for (i = 0, val = freq_lims[0]; i < nfreqs; i++) {
        freqs[i] = val;
        if (lin_typ)
            val += incr;
        else
            val *= incr;
    }

    /* then get the response for each of the requested station-channel pairs at
     each of the requested frequencies */

    first = evresp_itp(sta_list, cha_list, net_code, locid, datime, units, file,
            freqs, nfreqs, rtype, verbose, start_stage, stop_stage, stdio_flag,
            listinterp_out_flag, listinterp_in_flag, listinterp_tension,
            useTotalSensitivityFlag, x_for_b62, xml_flag, log);
    if (!first) {
        evalresp_log(log, WARN, 0, "EVRESP FAILED\n");
        /*XXX fprintf(stderr, "EVRESP FAILED\n"); */
        exit(EXIT_FAILURE);
    }

    /* and print the responses to a set of files */

    print_resp_itp(freqs, nfreqs, first, rtype, stdio_flag, listinterp_out_flag,
            listinterp_tension, unwrap_flag, log);

    free_response(first);
    free(freqs); /* added 3/28/2006 -- [ET] */

    exit(0);
    return 0; /* 'return' statement to avoid compiler warning */
}

