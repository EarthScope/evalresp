#include <stdio.h>
#include <stdlib.h>          /* for 'exit()'; added 8/28/2001 -- [ET] */

void
regerror(s)
char *s;
{
#ifdef ERRAVAIL
	error("regexp: %s", s);
#else
	fprintf(stderr, "regexp(3): %s", s);
	exit(1);
#endif
	/* NOTREACHED */
}
