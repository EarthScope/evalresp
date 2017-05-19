
# the following could be used so that all tests run with 'make check'
# but imho it's better to keep them separate
# (if you do this, it breaks run-c-tests.sh which instead runs everything)
#
# note that only base tests are cleared below - that lets the user
# configure extended tests before running 'macke check'
#
#check:
#	cd ../..; \
#		tests/jenkins/clean-base-test-dirs.sh; \
#		tests/jenkins/run-robot-tests.sh

clean:
	rm -fr run/*
	rm -fr lib/build
	find . -name "*~" -exec rm \{} \;
