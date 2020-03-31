CC:=gcc
CFLAGS:=-g -Wall -Werror 

TESTFILES := $(wildcard apps/test*.c) 
$(info TESTFILES are $(TESTFILES))

TESTS := $(TESTFILES:apps/%.c=%)
$(info TESTS are $(TESTS))

all: $(TESTS) disk.o file.o clean

test%: apps/test%.c file.o
	$(CC) $(CFLAGS) -o apps/$@ $^

disk.o: disk/disk.c disk/disk.h
	$(CC) $(CFLAGS) -c -o $@ $<

file.o: io/File.c disk/disk.c io/File.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm *.o
