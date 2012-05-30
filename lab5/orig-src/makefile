C		= 3
S		= 10000
V		= 1000
U		= 4
A		= 1000

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
CFLAGS		= -maltivec -O4 -Wall -pedantic -std=c99 -Werror

OBJS	= driver.o error.o

all: $(OBJS) 
	./dataflow $(S) $(V) $(U) $(A) $(C)

clean:
	rm -f $(PROGRAM_ppu) $(OBJS)
	(cd spu; make clean)
