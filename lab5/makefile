NSYM	= 100
NVERTEX	= 8
MAXSUCC	= 4
NACTIVE	= 10
NSPUS = 1
PRINT_OUTPUT=true
PRINT_INPUT=true


DIRS		:= spu
PROGRAM_ppu	:= dataflow
IMPORTS         = spu/lib_dataflow_spu.a -lspe2 -lpthread 

INSTALL_DIR		= $(EXP_SDKBIN)/tutorial
INSTALL_DIR		= ..
INSTALL_FILES		= $(PROGRAM_ppu)

CELL_TOP	= /opt/cell/sdk

include $(CELL_TOP)/buildutils/make.footer

#CFLAGS		= -O4 -Wall -pedantic -std=c99 -DTEST
#CFLAGS		= -O4 -Wall -pedantic -std=c99
CFLAGS		= -maltivec -O0 -Wall -pedantic -std=c99 -Werror -D_XOPEN_SOURCE=600 -g

OBJS	= driver.o error.o vertex.o list.o rand.o

all: $(OBJS)
#	./dataflow $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(NSPUS) ${PRINT_OUTPUT} ${PRINT_INPUT}# > cell_output.txt
#	(cd ../lab4; make; ./dataflow $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(NSPUS) ${PRINT_OUTPUT} ${PRINT_INPUT} > c_output.txt)
#	diff cell_output.txt ../lab4/c_output.txt > diff.txt

clean:
	rm -f $(PROGRAM_ppu) $(OBJS)
	(cd spu; make clean)

#.PHONY: all spu
