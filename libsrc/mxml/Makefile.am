#(2.1) define a list of libraries we create
lib_LTLIBRARIES = libmxmlev.la

#(2.2) define a list of library source files
libmxmlev_la_SOURCES = mxml-attr.c    mxml-file.c   mxml-node.c     mxml-set.c \
     mxml-get.c    mxml-private.c  mxml-string.c \
     mxml-entity.c  mxml-index.c  mxml-search.c   

#(2.3) define library FLAGS/OPTIONS
#libmxmlev_la_LDFLAGS = 


#=========(3) - GENERAL SETTING ==========
noinst_HEADERS = config.h  install-sh  mxml.h  mxml-private.h 
#man_MANS = 
EXTRA_DIST = ANNOUNCEMENT CHANGES COPYING README README.evalresp Makefile.nmake Makefile config.h.in  configure.ac mxml.xml mxml.list.in mxml.pc.in  mxml.spec test.xml Makefile.in.orig testmxml.c config.h.unix config.h.ms mxmldoc.c
AM_CPPFLAGS = $(all_includes)
#SUBDIRS = doc
LIBS = 
CFLAGS = -Wall -g -O2 -O0 -Wall -Wunused-but-set-variable
