

#ifndef EVALRESP_PUBLIC_H
#define EVALRESP_PUBLIC_H

/* VERSION is defined if using autotools */
#ifdef VERSION
#define REVNUM VERSION
#else
#define REVNUM "5.0.1"
#endif

#include "./public_api.h"
#include "./public_channels.h"
#include "./public_compat.h"
#include "./public_responses.h"

#endif
