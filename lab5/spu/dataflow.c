#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <spu_intrinsics.h>
#include <stdio.h>
#include <spu_mfcio.h>
#include <libsync.h>
#include "driver.h"

int		tag;

param_t		param A16;

int main(unsigned long long id, unsigned long long parm)
{
	int		i;
	size_t		nvertex;
	unsigned int	x;
	
	printf("hello from an spu: parm = %llx\n", parm);

	id = id;

	tag = mfc_tag_reserve();

	spu_writech(MFC_WrTagMask, -1); 
	spu_mfcdma32((void *)&param, (unsigned int)parm, sizeof(param_t), tag, 
		MFC_GET_CMD);                        
	spu_mfcstat(MFC_TAG_UPDATE_ALL);

	i = param.proc;
	nvertex = param.nvertex;

        x = spu_read_in_mbox();
	spu_write_out_mbox(x * 10);


	printf("SPU %d read nvertex = %zu and says goodbye\n", i, nvertex);
	
	return 0;
}
