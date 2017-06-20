

#ifndef EVALRESP_PUBLIC_H
#define EVALRESP_PUBLIC_H

/* IGD 10/16/04 This is for Windows which does not use Makefile.am 
   IGD TODO 06/20/2017: Should we come up with more automatic way fo setting version in Windows?*/
#ifdef VERSION
#define REVNUM VERSION
#else
#define REVNUM "5.0.0"
#endif

#include "./public_api.h"
#include "./public_channels.h"
#include "./public_compat.h"
#include "./public_responses.h"

#endif
