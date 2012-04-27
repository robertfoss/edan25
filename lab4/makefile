CFLAGS = -g -Wall -O2 -ffast-math -std=c99
CC = gcc
LIBS =  -lpthread -lm
INCLUDES =
SRCS = dataflow.c

NSYM	= 100
NVERTEX	= 8
MAXSUCC	= 4
NACTIVE	= 10
NTHREADS = 4
PRINT_OUTPUT=false
PRINT_INPUT=true
PRINT_DEBUG=false

all: dataflow

# The variable $@ has the value of the target. In this case $@ = psort
dataflow: dataflow.o bitset.o list.o rand.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ dataflow.o bitset.o list.o rand.o ${LIBS}
	#./dataflow $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) ${PRINT_OUTPUT} ${PRINT_INPUT} ${PRINT_DEBUG} > c_output.txt

	#javac DataflowCorrect.java && java -Xmx1024M Dataflow $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(NTHREADS) ${PRINT_OUTPUT} ${PRINT_INPUT} ${PRINT_DEBUG} > java_output.txt
	#rm -f *.class
	#diff c_output.txt java_output.txt > diff.txt

bitset_test: bitset_test.o bitset.o list.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ bitset_test.o bitset.o list.o ${LIBS}

list_test: list_test.o list.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ list_test.o list.o ${LIBS}

test:   bitset_test list_test
	./bitset_test
	./list_test

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

clean:
	rm -f *.o dataflow bitset_test list_test *~


.PHONY: all test
# DO NOT DELETE
