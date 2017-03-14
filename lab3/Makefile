
CC=gcc
CFLAGS=-ansi -o
EXE_FILE=simulate
SOURCE_FILE=lc3bsim3.c
CONTROL_STORE=ucode3
INPUT_PROG=isaprogram
INPUT_CMD=in.txt
OUTPUT_FILE=out.txt
CLEANER_FILE=cleaner.sh
all: ${EXE_FILE} run


simulate: ${SOURCE_FILE}
	$(CC) $(CFLAGS) ${EXE_FILE} ${SOURCE_FILE}


run: ucode3 isaprogram
	echo quit | ./${EXE_FILE} ${CONTROL_STORE} ${INPUT_PROG}
	# ./${EXE_FILE} ${CONTROL_STORE} ${INPUT_PROG}  < ${INPUT_CMD} > ${OUTPUT_FILE}
	# ./${CLEANER_FILE} ${OUTPUT_FILE}


clean:
ifeq ($(wildcard ${EXE_FILE}), )
else
	cp -rf ./${EXE_FILE} ./${OUTPUT_FILE} /tmp/
	rm ./${EXE_FILE} ./${OUTPUT_FILE}
	touch ./${OUTPUT_FILE}
endif

c:
ifeq ($(wildcard ${EXE_FILE}), )
else
	cp -rf ./${EXE_FILE} ./${OUTPUT_FILE} /tmp/
	rm ./${EXE_FILE} ./${OUTPUT_FILE}
	touch ./${OUTPUT_FILE}
endif

cout:
ifeq ($(wildcard ${OUTPUT_FILE}), )
else
	cp -rf ./${OUTPUT_FILE} /tmp/
	rm ./${OUTPUT_FILE}
	touch ./${OUTPUT_FILE}
endif


cleanAll:
ifeq ($(wildcard ${EXE_FILE}), )
else
	rm ./${EXE_FILE}
endif

ifneq ($(wildcard /tmp/${EXE_FILE}), )
	rm /tmp/${EXE_FILE}
endif
	rm ./${OUTPUT_FILE}
	touch ./${OUTPUT_FILE}

call:
ifeq ($(wildcard ${EXE_FILE}), )
else
	rm ./${EXE_FILE}
endif

ifneq ($(wildcard /tmp/${EXE_FILE}), )
	rm /tmp/${EXE_FILE}
endif
	rm ./${OUTPUT_FILE}
	touch ./${OUTPUT_FILE}


restore:
ifneq ($(wildcard /tmp/${EXE_FILE}), )
	cp /tmp/${EXE_FILE} ./
else
	echo "ERROR: no backups were found."
endif

restoreOutput:
ifneq ($(wildcard /tmp/${OUTPUT_FILE}), )
	cp /tmp/${OUTPUT_FILE} ./
else
	echo "ERROR: no backups were found."
endif

r:
ifneq ($(wildcard /tmp/${EXE_FILE}), )
	cp /tmp/${EXE_FILE} ./
else
	echo "ERROR: no backups were found."
endif

rout:
ifneq ($(wildcard /tmp/${OUTPUT_FILE}), )
	cp /tmp/${OUTPUT_FILE} ./
else
	echo "ERROR: no backups were found."
endif
