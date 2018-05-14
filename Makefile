.phony all:
all: Assig1

Assig1: Assig1.c
	gcc Assig1.c -lreadline -lhistory -ltermcap -o Assig1

.PHONY clean:
clean:
	-rm -rf *.o *.exe
