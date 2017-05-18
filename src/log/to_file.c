#include "../log.h"
#include <stdlib.h>
#include <stdio.h>
#include "to_file.h"

int evalresp_log_to_file(evalresp_log_msg_t *msg, void *data)
{
    FILE *fd=data;
    char date_str[256];/*TODO this is tomany bytes*/
    if (!fd)
    {
        return EXIT_FAILURE;
    }
    /* turn the timestamp into locale std date string */
    strftime(date_str, 256, "%c", localtime(&(msg->timestamp)));
    /* just log the msg to stderr*/
    fprintf(fd, "%s [%s] %s\n", date_str,
            (msg->log_level < 4 && msg->log_level >= 0) ?
                log_level_strs[msg->log_level] :
                "unknown",
            msg->msg);
    return EXIT_SUCCESS;
}
