#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <spu_intrinsics.h>
#include <stdio.h>
#include <spu_mfcio.h>
#include <libsync.h>
#include <stdbool.h>
#include <string.h>
#include <libmisc.h>

#include "driver.h"
#include "vertex.h"


//#define DEBUG_WORK
#ifdef DEBUG_WORK
#define TRACE_WORK if(1)
#else
#define TRACE_WORK if(0)
#endif

//#define DEBUG_FETCH_BITSET
#ifdef DEBUG_FETCH_BITSET
#define TRACE_FETCH_BITSET if(1)
#else
#define TRACE_FETCH_BITSET if(0)
#endif


param_t		    param A16;
int             spu_num;
int 		    tag[5] = {0 };
unsigned int    bitset_size;
unsigned int    bitset_subsets;
int i = 0;


void bitset_mega(unsigned int* in, unsigned int* out, unsigned int* use, unsigned int* def){
    //printf("SPU megaop, bitset_subsets = %u %d\n", param.bitset_subsets, i++);
    for(unsigned int i = 0; i < param.bitset_subsets; ++i) {
        in[i]  = out[i];
		in[i]  = in[i] & (~(in[i] & def[i]));
		in[i] |= use[i];
    }
}
//035-158109

unsigned int* fetch_bitset(unsigned int* addr, unsigned int offset, int tag) {

	TRACE_FETCH_BITSET printf("SPU[%u] malloc_align(sz=%u, align_exp=%u)\n", spu_num, param.bitset_size, ALIGN_EXP);
    unsigned int* tmp = malloc_align(param.bitset_size, ALIGN_EXP);
	unsigned int host_addr = (unsigned int) (addr + offset);
	TRACE_FETCH_BITSET printf("SPU[%u] Reading %u bytes from %u mod %u = %u to %u mod %u = %u \n",
                              spu_num, param.bitset_size, host_addr, ALIGN_CONSTANT, host_addr % ALIGN_CONSTANT,
                              (unsigned int)tmp, ALIGN_CONSTANT, ((unsigned int)tmp) % ALIGN_CONSTANT);

    printf("before mfc_get\n");
    mfc_get(tmp, host_addr, param.bitset_size, tag, 0, 0);
    printf("after mfc_get\n");
	TRACE_FETCH_BITSET printf("SPU[%u] mfc_get returned\n", spu_num);

    return tmp;
}


void work() {

    spu_send_mail_t send_mail;
    unsigned int index;
    unsigned int* in;
    unsigned int* out;
    unsigned int* use;
    unsigned int* def;
//	printf("SPU[%u] IN from %u mod %u = %u..\n", spu_num, (unsigned int)param.bs_in_addr, ALIGN_CONSTANT, ((unsigned int) param.bs_in_addr) % ALIGN_CONSTANT);
//	printf("SPU[%u] OUT from %u mod %u = %u..\n", spu_num, (unsigned int)param.bs_out_addr, ALIGN_CONSTANT, ((unsigned int) param.bs_out_addr) % ALIGN_CONSTANT);
//	printf("SPU[%u] USE from %u mod %u = %u..\n", spu_num, (unsigned int)param.bs_use_addr, ALIGN_CONSTANT, ((unsigned int) param.bs_use_addr) % ALIGN_CONSTANT);
//	printf("SPU[%u] DEF from %u mod %u = %u..\n", spu_num, (unsigned int)param.bs_def_addr, ALIGN_CONSTANT, ((unsigned int) param.bs_def_addr) % ALIGN_CONSTANT);

    while(1){
        index = spu_read_in_mbox();
        printf("SPU received #%u\n", index);
		TRACE_WORK printf("SPU[%u] received #%u\n", spu_num, index);
		
		// No more work to be done signalled
		/*if(index == UINT_MAX){
			return;
		}*/

		unsigned int offset = index * param.bitset_subsets;
        //printf("\nindex = %d\toffset = %d\n\n", index, offset);
		TRACE_WORK printf("SPU[%u] DMA#%d reading..\n", spu_num, tag[0]);
        in  = fetch_bitset(param.bs_in_addr, offset, tag[0]);
		TRACE_WORK printf("SPU[%u] DMA#%d reading 2..\n", spu_num, tag[1]);
        out = fetch_bitset(param.bs_out_addr, offset, tag[1]);
		TRACE_WORK printf("SPU[%u] DMA#%d reading 3..\n", spu_num, tag[2]);
        use = fetch_bitset(param.bs_use_addr, offset, tag[2]);
		TRACE_WORK printf("SPU[%d] DMA#%d reading 4..\n", spu_num, tag[3]);
        def = fetch_bitset(param.bs_def_addr, offset, tag[3]);
        TRACE_WORK printf("fetched 4 bitsets\n");

        mfc_write_tag_mask(1<<tag[0] | 1<<tag[1] | 1<<tag[2] | 1<<tag[3]);
        mfc_read_tag_status_all();
		TRACE_WORK printf("SPU[%u] calculating megaop\n", spu_num);
        bitset_mega(in, out, use, def);

		TRACE_WORK printf("SPU[%u] DMA#%d writing..\n", spu_num, tag[1]);
        mfc_put(in, (unsigned int) (param.bs_in_addr + offset), param.bitset_size, tag[1], 0 , 0);
        mfc_write_tag_mask(1<<tag[1]);
        mfc_read_tag_status_all();

		TRACE_WORK printf("SPU[%u] sending msg #%u\n", spu_num, index);
        send_mail.op_completed = index;
        printf("SPU sending %u\n", index);
        spu_write_out_intr_mbox(send_mail.op_completed);

		free(in);
		free(out);
		free(use);
		free(def);
    }
}


int main(unsigned long long id, unsigned long long parm) 
{

    id = id;
    printf("SPU parm: %llu\n", parm);
    mfc_get((void *) &param, (unsigned int) parm, sizeof(param_t), 1, 0, 0);
    mfc_write_tag_mask(1);
    mfc_read_tag_status_all();

    int j;
    int tag_num = param.proc * 5;
    for(j = 0; j < param.proc*5+5; ++j){
    	tag[j] = tag_num++;
    }

	spu_num = param.proc;
    bitset_size = param.bitset_size;
    bitset_subsets = param.bitset_subsets;

	printf("SPU[%d] initiated, with tags{%d, %d, %d, %d, %d}\n", spu_num, tag[0], tag[1], tag[2], tag[3], tag[4]);

    work();
	
    return 0;
}
