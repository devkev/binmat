
CPPFLAGS = -DHAVE_UNSIGNED_LONG_LONG_INT
CPPFLAGS += -DDO_TRAD
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

