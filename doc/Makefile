
docs: doxygen pdf

doxygen:
	doxygen Doxyfile 2>doxygen.err.log >doxygen.out.log

pdf:
	cd latex; $(MAKE) pdf 2>latex.err.log >latex.out.log
	cp latex/refman.pdf evalresp.pdf
