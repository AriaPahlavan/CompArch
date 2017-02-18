
CC=gcc
CFLAGS=-ansi -o
EXE_FILE=simulate
SOURCE_FILE=lc3bsim2.c


all: ${EXE_FILE} run


simulate: ${SOURCE_FILE}
	$(CC) $(CFLAGS) ${EXE_FILE} ${SOURCE_FILE}


run: countOdd.asm output.obj
	./simulate output.obj


clean:
ifeq ($(wildcard ${EXE_FILE}), )
else
	cp ./${EXE_FILE} /tmp/
	rm ./${EXE_FILE}
endif

c:
ifeq ($(wildcard ${EXE_FILE}), )
else
	cp ./${EXE_FILE} /tmp/
	rm ./${EXE_FILE}
endif


cleanAll:
ifeq ($(wildcard ${EXE_FILE}), )
else
	rm ./${EXE_FILE}
endif

ifneq ($(wildcard /tmp/${EXE_FILE}), )
	rm /tmp/${EXE_FILE}
endif

call:
ifeq ($(wildcard ${EXE_FILE}), )
else
	rm ./${EXE_FILE}
endif

ifneq ($(wildcard /tmp/${EXE_FILE}), )
	rm /tmp/${EXE_FILE}
endif


restore:
ifneq ($(wildcard /tmp/${EXE_FILE}), )
	cp /tmp/${EXE_FILE} ./
else
	echo "ERROR: no backups were found."
endif

r:
ifneq ($(wildcard /tmp/${EXE_FILE}), )
	cp /tmp/${EXE_FILE} ./
else
	echo "ERROR: no backups were found."
endif
