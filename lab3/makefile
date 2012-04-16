CFLAGS = -g -Wall

CC = gcc
LIBS =  -lm 
INCLUDES =
SRCS = sort.c


all: sort

# The variable $@ has the value of the target. In this case $@ = psort
sort: sort.o ${OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ sort.o ${LIBS}
	./sort


.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

clean:
	rm -f *.o sort *~

# DO NOT DELETE
