
#ifndef EVALRESP_PUBLIC_H
#define EVALRESP_PUBLIC_H

/* IGD 10/16/04 This is for Windows which does not use Makefile.am */
#ifdef VERSION
#define REVNUM VERSION
#else
#define REVNUM "4.0.6"
#endif

#include "public_api.h"
#include "evalresp/public_compat.h"
#include "evalresp/evalresp_public_seed.h"
#include "evalresp_log/log.h"

#endif
