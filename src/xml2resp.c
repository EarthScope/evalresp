
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "x2r.h"
#include "x2r_log.h"
#include "x2r_ws.h"
#include "x2r_xml.h"


/**
 * Handle command-line options.
 *
 * We don't use getopt because Windows.
 */
static int parse_opts(int argc, char *argv[], x2r_log **log, FILE **in, FILE **out) {

    int status = X2R_OK, level = 0, index = 0;
    char *input = NULL, *output = NULL;

    *in = stdin;
    *out = stdout;

    while (++index < argc) {
    	if (argv[index][0] == '-') {
    		switch (argv[index][1]) {
            case 'v':
                level += strlen(argv[index]) - 1;
                break;
            case 'o':
                output = strdup(argv[++index]);
                break;
            default:
                status = X2R_ERR_USER;
                goto exit;
    		}
    	} else if (input) {
    		status = X2R_ERR_USER;
    		goto exit;
    	} else {
    		input = strdup(argv[index]);
    	}
    }

    if ((status = x2r_alloc_log(level, stderr, log))) goto exit;
    x2r_info(*log, "Logging to stderr");

    if (input) {
        if (!(*in = fopen(input, "r"))) {
            status = x2r_error(*log, X2R_ERR_IO, "Cannot open %s to read", input);
            goto exit;
        }
        x2r_info(*log, "Input from %s", input);
    } else {
        x2r_info(*log, "Input from stdin");
    }

    if (output) {
        if (!(*out = fopen(output, "w"))) {
            status = x2r_error(*log, X2R_ERR_IO, "Cannot open %s to write", output);
            goto exit;
        }
        x2r_info(*log, "Output to %s", output);
    } else {
        x2r_info(*log, "Output to stdout");
    }

exit:
    free(input);
    free(output);
    return status;
}


/**
 * A command-line interface to the library.  Converts a single file from station.xml
 * to response format.
 */
int main(int argc, char *argv[]) {

    int status = X2R_OK;
    x2r_log *log = NULL;
    x2r_fdsn_station_xml *root = NULL;
    FILE *in = stdin, *out = stdout;

    if ((status = parse_opts(argc, argv, &log, &in, &out))) goto exit;
    if ((status = x2r_station_service_load(log, in, &root))) goto exit;
    if ((status = x2r_resp_util_write(log, out, root))) goto exit;

exit:
    if (status == X2R_ERR_USER) {
        fprintf(stderr, "\nUsage:\n");
        fprintf(stderr, "\n  Specifying input and output files\n");
        fprintf(stderr, "  %s [-v] -o OUT.resp IN.xml\n", argv[0]);
        fprintf(stderr, "\n  Using stdin and stdout\n");
        fprintf(stderr, "  %s [-v] < IN.xml > OUT.resp\n", argv[0]);
        fprintf(stderr, "\n  Logging goes to stderr (multiple -v gives more detail)\n");
        fprintf(stderr, "  %s -vvvv -o OUT.resp < IN.xml 2> LOG\n\n", argv[0]);
    }
    if (in && in != stdin) fclose(in);
    if (out && out != stdout) fclose(out);
    status = x2r_free_fdsn_station_xml(root, status);
    status = x2r_free_log(log, status);
    return status;
}
