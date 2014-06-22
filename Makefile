# ---
#
# Main Makefile
#
# Copyright (c) 2014 Mattias Mattsson. All rights reserved.
# This source is distributed under the license in LICENSE.txt in the top
# InstaWorks directory.
#
# ---

.PHONY: clean selftest examples

all: instaworks selftest examples dox

instaworks:
	$(MAKE) -f Makefile.instaworks

selftest:
	$(MAKE) -f Makefile.selftest

examples:
	$(MAKE) -C examples -f Makefile

dox:
	doxygen InstaWorks.doxygen

clean:
	$(MAKE) -f Makefile.instaworks clean
	$(MAKE) -f Makefile.selftest clean
	$(MAKE) -C examples -f Makefile clean
	rm -rf html

# ---

