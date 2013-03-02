
# Copyright (C) 2013 Kevin Pulo <kev@pulo.com.au>.
#
# This file is part of binmat.
#
# binmat is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# binmat is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with binmat.  If not, see <http://www.gnu.org/licenses/>.


CPPFLAGS = -DHAVE_UNSIGNED_LONG_LONG_INT
CPPFLAGS += -DBINMAT_TRAD
#CPPFLAGS += -DBINMAT_DEBUG
CFLAGS = -g -Wall
CFLAGS += -fPIC -DPIC
CFLAGS += -O2
#CFLAGS += -O3
#CFLAGS += -O4
#LDLIBS = -lefence


all: libbinmat.so libbinmat.a binmat-test

clean:
	rm -f libbinmat.o libbinmat.so libbinmat.a binmat-test

libbinmat.o: libbinmat.c
libbinmat.so: libbinmat.o
libbinmat.a: libbinmat.o

#binmat-test: binmat-test.c libbinmat.so
binmat-test: binmat-test.c libbinmat.a


%.so: %.o
	$(LINK.o) $^ -shared $(LOADLIBES) $(LDLIBS) -o $@

%.a: %.o
	$(AR) $(ARFLAGS) $@ $^

