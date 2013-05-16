PROGRAM_spu 	:= dataflow
LIBRARY_embed	:= lib_dataflow_spu.a


INCLUDE		:= -I ..  -I /opt/cell/sdk/src/lib/sync/spu_source

LDFLAGS		+= -lsync -L/opt/cell/sdk/src/lib/sync/spu
CFLAGS		+= -O4 -Wall -pedantic -std=c99 -Werror -g -Wno-unused-variable

IMPORTS     += -lmisc

include /opt/cell/sdk/buildutils/make.footer
