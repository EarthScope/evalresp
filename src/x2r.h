
#ifndef X2R_H
#define X2R_H

#define X2R_OK 0
#define X2R_ERR_USER 1
#define X2R_ERR_MEMORY 2
#define X2R_ERR_XML2 3
#define X2R_ERR_BUFFER 4
#define X2R_ERR_XML 5
#define X2R_ERR_IO 6
#define X2R_ERR_DATE 7

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#endif
