
.PHONY: docs doxygen pdf html clean

docs: doxygen pdf

clean:
	rm -rf latex html doxygen.err.log doxygen.out.log evalresp.pdf

doxygen:
	doxygen Doxyfile 2>doxygen.err.log >doxygen.out.log

pdf:
	cd latex; $(MAKE) pdf 2>latex.err.log >latex.out.log
	cp latex/refman.pdf evalresp.pdf

# Graphs are drawn only when Graphviz (dot) is installed
HAVE_DOT := $(if $(shell command -v dot 2>/dev/null),YES,NO)

html:
	( cat Doxyfile; \
	  echo "GENERATE_HTML = YES"; \
	  echo "GENERATE_LATEX = NO"; \
	  echo "HAVE_DOT = $(HAVE_DOT)" ) | doxygen - 2>doxygen.err.log >doxygen.out.log
