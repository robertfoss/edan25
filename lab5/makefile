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
CFLAGS		+= -maltivec -O0 -Wall -pedantic -gdwarf-2 -std=c99 -Werror -D_XOPEN_SOURCE=600 -g ${XTRACFLAGS}

OBJS	= driver.o error.o list.o rand.o

all: $(OBJS)


clean:
	rm -f $(PROGRAM_ppu) $(OBJS)
	(cd spu; make clean)

#.PHONY: all spu
