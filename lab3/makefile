CFLAGS = -g -Wall

CC = gcc
LIBS =  -lpthread  -lm -mnative
INCLUDES =
SRCS = sort.c


all: sort

# The variable $@ has the value of the target. In this case $@ = psort
sort: sort.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ sort.o ${LIBS}
	#./sort
	
merge: mergesort.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ mergesort.o ${LIBS}
	./merge


.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

clean:
	rm -f *.o sort *~

# DO NOT DELETE
