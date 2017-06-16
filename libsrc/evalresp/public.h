
#ifndef EVALRESP_PUBLIC_H
#define EVALRESP_PUBLIC_H

/* IGD 10/16/04 This is for Windows which does not use Makefile.am */
#ifdef VERSION
#define REVNUM VERSION
#else
#define REVNUM "4.0.6"
#endif

#include "./public_api.h"
#include "./public_channels.h"
#include "./public_responses.h"
#include "./public_compat.h"

#endif
