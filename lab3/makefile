CFLAGS = -g -Wall -O2
CC = gcc
LIBS =  -lpthread  -lm
INCLUDES =
SRCS = sort.c


all: sort

# The variable $@ has the value of the target. In this case $@ = psort
sort: sort.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ sort.o ${LIBS}
	./sort 1000000 5

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

clean:
	rm -f *.o sort *~

# DO NOT DELETE
