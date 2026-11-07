
.PHONY: docs doxygen pdf clean

docs: doxygen pdf

clean:
	rm -rf latex doxygen.err.log doxygen.out.log evalresp.pdf

doxygen:
	doxygen Doxyfile 2>doxygen.err.log >doxygen.out.log

pdf:
	cd latex; $(MAKE) pdf 2>latex.err.log >latex.out.log
	cp latex/refman.pdf evalresp.pdf
