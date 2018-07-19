
EXTRA_DIST = README.rst

# the following could be used so that all tests run with 'make check'
# but imho it's better to keep them separate
# (if you do this, it breaks run-c-tests.sh which instead runs everything)
#
# note that only base tests are cleared below - that lets the user
# configure extended tests before running 'make check'
#
#check:
#	cd ../..; \
#		tests/jenkins/clean-base-test-dirs.sh; \
#		tests/jenkins/run-robot-tests.sh

clean-local:
	rm -fr run/*
	rm -fr all/extended
	rm -fr data/extended
	rm -fr target/extended
	rm -fr lib/build
	find . -name "*~" -exec rm \{} \;

distclean-local:
	rm -fr data/*.zip
	rm -fr targets/*.zip
