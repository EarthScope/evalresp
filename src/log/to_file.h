#ifndef __evalresp_log_to_file_h__
#define __evalresp_log_to_file_h__
#include "../log.h"

int evalresp_log_to_file(evalresp_log_msg_t *, void *);
int evalresp_log_intialize_log_for_file(evalresp_log_t *, FILE *);

/*typedef struct evalresp_file_data
{
    FILE *fd;
} evalresp_file_data_t;*/
#endif /* __evalresp_log_to_file_h__*/
